#include "warpr_includes.h"
#include "WebRtcClient.h"

using namespace Axodox::Infrastructure;
using namespace rtc;
using namespace std;

namespace Warpr::Messaging
{
  WebRtcClient::WebRtcClient(Axodox::Infrastructure::dependency_container* container) :
    _settings(container->resolve<WarpConfiguration>()),
    _signaler(container->resolve<WebSocketClient>()),
    _signalerConnectedSubscription(_signaler->Connected({ this, &WebRtcClient::OnSignalerConnected }))
  {
    if (_signaler->IsConnected()) OnSignalerConnected(_signaler.get());
  }

  void WebRtcClient::OnSignalerConnected(WebSocketClient* sender)
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

      _logger.log(log_severity::debug, "Local description: {}", *message.Description);
      });

    _peerConnection->onLocalCandidate([=](const Candidate& candidate) {
      PeerConnectionCandidateMessage message;
      *message.Candidate = string(candidate);
      _signaler->SendMessage(message);

      _logger.log(log_severity::debug, "Local candidate: {}", *message.Candidate);
      });

    //Create data channel
    _dataChannel = _peerConnection->createDataChannel("control");
  }
}