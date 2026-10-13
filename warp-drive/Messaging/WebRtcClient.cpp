#include "warpr_includes.h"
#include "WebRtcClient.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Threading;
using namespace rtc;
using namespace std;
using namespace std::chrono;

namespace Warpr::Messaging
{
  const std::string_view WebRtcClient::_stateNames[] = {
    "New",
    "Connecting",
    "Connected",
    "Disconnected",
    "Failed",
    "Closed"
  };

  const std::string_view WebRtcClient::_iceStateNames[] = {
    "New",
    "Checking",
    "Connected",
    "Completed",
    "Failed",
    "Disconnected",
    "Closed"
  };

  const std::string_view WebRtcClient::_gatheringStateNames[] = {
    "New",
    "InProgress",
    "Complete"
  };

  WebRtcClient::WebRtcClient(Axodox::Infrastructure::dependency_container* container) :
    ControlMessageReceived(_events),
    AuxMessageReceived(_events),
    _containerRef(container->get_ref()),
    _settings(container->resolve<WarpConfiguration>()),
    _signaler(container->resolve<WebSocketClient>()),

    _lastReportingTime(steady_clock::now()),
    _dataSentSinceLastReport(0),

    _signalerMessageReceivedSubscription(_signaler->MessageReceived({ this, &WebRtcClient::OnSignalingMessageReceived }))
  { }

  bool WebRtcClient::IsConnected() const
  {
    return _peerConnection && _peerConnection->state() == PeerConnection::State::Connected;
  }

  void WebRtcClient::SendVideoPacket(const rtc::binary& packet)
  {
    lock_guard lock(_mutex);
    if (!IsConnected()) return;

    _streamChannel->send(packet);

    _dataSentSinceLastReport += packet.size();
    auto now = steady_clock::now();
    if (now - _lastReportingTime > 1s)
    {
      _logger.log(log_severity::debug, L"Output buffer size: {} bytes", _streamChannel->bufferedAmount());
      _logger.log(log_severity::debug, L"Output data rate: {} bytes/second", _dataSentSinceLastReport);
      _lastReportingTime = now;
      _dataSentSinceLastReport = 0;
    }
  }

  void WebRtcClient::SendControlMessage(const rtc::message_variant& message)
  {
    lock_guard lock(_mutex);
    if (!IsConnected()) return;
    _controlChannel->send(message);
  }

  void WebRtcClient::SendAuxMessage(const rtc::message_variant& message)
  {
    lock_guard lock(_mutex);
    if (!IsConnected()) return;

    _auxMessageSplitter.SplitMessage(message, [=](const binary& message) {
      _auxChannel->send(message);
      });
  }

  void WebRtcClient::OnSignalingMessageReceived(WebSocketClient* sender, const WarprSignalingMessage* message)
  {
    lock_guard lock(_mutex);

    switch (message->Type())
    {
    case WarprSignalingMessageType::PairingCompleteMessage:
      OnPairingComplete(static_cast<const PairingCompleteMessage*>(message));
      break;

    case WarprSignalingMessageType::PeerConnectionDescriptionMessage:
      if (_peerConnection)
      {
        auto descriptionMessage = static_cast<const PeerConnectionDescriptionMessage*>(message);
        _peerConnection->setRemoteDescription(*descriptionMessage->Description);
        _logger.log(log_severity::debug, "Remote description:\n{}", *descriptionMessage->Description);
      }
      break;

    case WarprSignalingMessageType::PeerConnectionCandidateMessage:
      if (_peerConnection)
      {
        auto descriptionMessage = static_cast<const PeerConnectionCandidateMessage*>(message);
        _peerConnection->addRemoteCandidate(*descriptionMessage->Candidate);
        _logger.log(log_severity::debug, "Remote candidate: {}", *descriptionMessage->Candidate);
      }
      break;

    default:
      break;
    }
  }

  void WebRtcClient::OnPairingComplete(const PairingCompleteMessage* message)
  {
    _configuration.ConnectionTimeout = duration<float>(*message->ConnectionTimeout);
    _configuration.IceServers.clear();
    _configuration.IceServers.reserve(message->IceServers->size());
    for (auto& server : *message->IceServers)
    {
      _configuration.IceServers.emplace_back(server);
    }
    _configuration.IsConnectionUnordered = *message->StreamChannelReliability->IsUnordered;
    _configuration.MaxRetransmits = *message->StreamChannelReliability->MaxRetransmits;
    _configuration.MaxPacketLifetime = *message->StreamChannelReliability->MaxPacketLifetime;

    _connectionThread.reset();
    _connectionThread = background_thread({ this, &WebRtcClient::Connect }, "* webrtc connect");
  }

  void WebRtcClient::Connect()
  {
    while (!_connectionThread.is_exiting())
    {
      _logger.log(log_severity::information, "Initializing WebRTC connection...");

      {
        lock_guard lock(_mutex);

        //Reset existing connection
        _auxChannel.reset();
        _controlChannel.reset();
        _streamChannel.reset();
        _peerConnection.reset();
        _auxMessageAssembler.Reset();

        //Create configuration
        Configuration config;
        config.iceServers = _configuration.IceServers;
        config.maxMessageSize = 256 * 1024;
        config.mtu = 1500;

        //Create peer connection
        _peerConnection = make_unique<PeerConnection>(config);

        _peerConnection->onLocalDescription([=](const Description& description) {
          PeerConnectionDescriptionMessage message;
          *message.Description = string(description);
          _signaler->SendMessage(message);

          _logger.log(log_severity::debug, "Local description:\n{}", *message.Description);
          });

        _peerConnection->onLocalCandidate([=](const Candidate& candidate) {
          PeerConnectionCandidateMessage message;
          *message.Candidate = string(candidate);
          _signaler->SendMessage(message);

          _logger.log(log_severity::debug, "Local candidate: {}", *message.Candidate);
          });

        _peerConnection->onStateChange([=](PeerConnection::State state) {
          _logger.log(log_severity::information, "State changed to '{}'.", _stateNames[size_t(state)]);

          if (state == PeerConnection::State::Connected)
          {
            _auxMessageSplitter.MaxMessageSize = _peerConnection->remoteMaxMessageSize();
          }
          });

        _peerConnection->onIceStateChange([=](PeerConnection::IceState state) {
          _logger.log(log_severity::information, "ICE State changed to '{}'.", _iceStateNames[size_t(state)]);
          });

        _peerConnection->onGatheringStateChange([=](PeerConnection::GatheringState state) {
          _logger.log(log_severity::information, "Gathering State changed to '{}'.", _gatheringStateNames[size_t(state)]);
          });

        //Create data channel
        _streamChannel = _peerConnection->createDataChannel("stream", {
          .reliability = {
            .unordered = _configuration.IsConnectionUnordered,
            .maxPacketLifeTime = _configuration.MaxPacketLifetime,
            .maxRetransmits = _configuration.MaxRetransmits
          }
          });
        _controlChannel = _peerConnection->createDataChannel("control");
        _auxChannel = _peerConnection->createDataChannel("aux");

        _controlChannel->onMessage([=](message_variant data) {
          _events.raise(ControlMessageReceived, this, &data);
          });
        _auxChannel->onMessage([=](message_variant data) {
          if (holds_alternative<binary>(data))
          {
            auto message = _auxMessageAssembler.PushMessage(get<binary>(data));
            if (message)
            {
              _events.raise(AuxMessageReceived, this, &*message);
            }
          }
          });
      }

      this_thread::sleep_for(_configuration.ConnectionTimeout);
      if (_peerConnection->state() == PeerConnection::State::Connected) break;

      _logger.log(log_severity::warning, "Failed to connect, retrying...");
    }
  }
}