#include "warpr_includes.h"
#include "WebRtcClient.h"

using namespace Axodox::Infrastructure;
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

  WebRtcClient::WebRtcClient(Axodox::Infrastructure::dependency_container* container) :
    _settings(container->resolve<WarpConfiguration>()),
    _signaler(container->resolve<WebSocketClient>()),
    _signalerMessageReceivedSubscription(_signaler->MessageReceived({ this, &WebRtcClient::OnSignalerMessageReceived }))
  { }

  void WebRtcClient::OnSignalerMessageReceived(WebSocketClient* sender, const WarprMessage* message)
  {
    lock_guard lock(_mutex);

    switch (message->Type())
    {
    case WarprMessageType::PairingCompleteMessage:
      Connect();
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

  void WebRtcClient::Connect()
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
    
    //Create data channel
    _dataChannel = _peerConnection->createDataChannel("control");
    
  }
}