#include "warpr_includes.h"
#include "WebRtcClient.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
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
    _signalerMessageReceivedSubscription(_signaler->MessageReceived({ this, &WebRtcClient::OnSignalingMessageReceived }))
  { }

  bool WebRtcClient::IsConnected() const
  {
    return _peerConnection && _peerConnection->state() == PeerConnection::State::Connected;
  }

  void WebRtcClient::SendVideoFrame(std::span<const uint8_t> bytes)
  {
    lock_guard lock(_mutex);

    //If we are not connected then do not send
    if (!IsConnected()) return;

    //Split to fragments and send
    {
      auto messageSize = _peerConnection->remoteMaxMessageSize() - 16;

      memory_stream stream;
      stream.reserve(_peerConnection->remoteMaxMessageSize());

      size_t position = 0u;
      auto fragmentIndex = 0u;
      while (position < bytes.size())
      {
        auto fragmentLength = min(messageSize, bytes.size() - position);
        stream.reset();

        stream.write(uint32_t(_messageIndex));
        stream.write(uint32_t(bytes.size()));
        stream.write(uint32_t(messageSize));
        stream.write(uint32_t(fragmentIndex++));
        stream.write(bytes.subspan(position, fragmentLength));

        _streamChannel->send(reinterpret_cast<const std::byte*>(stream.data()), stream.length());
        position += fragmentLength;
      }

      _messageIndex++;
    }

    using namespace std::chrono;
    static steady_clock::time_point _lastReportingTime = {};
    static uint64_t _dataSent = 0;
    _dataSent += bytes.size();
    auto now = steady_clock::now();
    if (now - _lastReportingTime > 1s)
    {
      _logger.log(log_severity::debug, L"Output buffer size: {} bytes", _streamChannel->bufferedAmount());
      _logger.log(log_severity::debug, L"Output data rate: {} bytes/second", _dataSent);
      _lastReportingTime = now;
      _dataSent = 0;
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
    _auxChannel->send(message);
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
            .unordered = true,
            .maxRetransmits = 0
          }
          });
        _controlChannel = _peerConnection->createDataChannel("control");
        _auxChannel = _peerConnection->createDataChannel("aux");

        _controlChannel->onMessage([=](message_variant data) {
          _events.raise(ControlMessageReceived, this, &data);
          });
        _auxChannel->onMessage([=](message_variant data) {
          _events.raise(AuxMessageReceived, this, &data);
          });
      }

      this_thread::sleep_for(_configuration.ConnectionTimeout);
      if (_peerConnection->state() == PeerConnection::State::Connected) break;

      _logger.log(log_severity::warning, "Failed to connect, retrying...");
    }
  }
}