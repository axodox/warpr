#include "warpr_includes.h"
#include "WebRtcClient.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace rtc;
using namespace std;

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
    _containerRef(container->get_ref()),
    _settings(container->resolve<WarpConfiguration>()),
    _signaler(container->resolve<WebSocketClient>()),
    _signalerMessageReceivedSubscription(_signaler->MessageReceived({ this, &WebRtcClient::OnSignalerMessageReceived }))
  { }

  bool WebRtcClient::IsConnected() const
  {
    return _peerConnection && _peerConnection->state() == PeerConnection::State::Connected;
  }

  void WebRtcClient::SendMessage(std::span<const uint8_t> bytes, WebRtcChannel channelType)
  {
    //If we are not connected then do not send
    if (!IsConnected()) return;

    //Select channel
    DataChannel* channel = nullptr;

    switch (channelType)
    {
    case WebRtcChannel::Reliable:
      channel = _reliableChannel.get();
      break;
    case WebRtcChannel::LowLatency:
      channel = _lowLatencyChannel.get();
      break;
    }

    if (!channel) return;

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

        channel->send(reinterpret_cast<const std::byte*>(stream.data()), stream.length());
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
      _logger.log(log_severity::debug, L"Output buffer size: {} bytes", channel->bufferedAmount());
      _logger.log(log_severity::debug, L"Output data rate: {} bytes/second", _dataSent);
      _lastReportingTime = now;
      _dataSent = 0;
    }
  }

  void WebRtcClient::OnSignalerMessageReceived(WebSocketClient* sender, const WarprMessage* message)
  {
    lock_guard lock(_mutex);

    switch (message->Type())
    {
    case WarprMessageType::PairingCompleteMessage:
      ConnectAsync();
      break;

    case WarprMessageType::PeerConnectionDescriptionMessage:
      if (_peerConnection)
      {
        auto descriptionMessage = static_cast<const PeerConnectionDescriptionMessage*>(message);
        _peerConnection->setRemoteDescription(*descriptionMessage->Description);
        _logger.log(log_severity::debug, "Remote description:\n{}", *descriptionMessage->Description);
      }
      break;

    case WarprMessageType::PeerConnectionCandidateMessage:
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

  winrt::fire_and_forget WebRtcClient::ConnectAsync()
  {
    _logger.log(log_severity::information, "Initializing WebRTC connection...");

    //Create configuration
    Configuration config;
    config.iceServers.reserve(_settings->IceServers.size());
    for (auto& server : _settings->IceServers)
    {
      config.iceServers.emplace_back(server);
    }

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
    _reliableChannel = _peerConnection->createDataChannel("reliable");
    _lowLatencyChannel = _peerConnection->createDataChannel("low_latency", {
      .reliability = {
        .unordered = true,
        .maxRetransmits = 0
      }
    });

    auto lifetime = _containerRef.try_lock();
    co_await 1s;

    if (_peerConnection->state() == PeerConnection::State::Connected) co_return;

    _logger.log(log_severity::warning, "Failed to connect, retrying...");
    _lowLatencyChannel.reset();
    _reliableChannel.reset();
    _peerConnection.reset();

    ConnectAsync();
  }
}