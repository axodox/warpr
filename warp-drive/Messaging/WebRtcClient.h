#pragma once
#include "WebSocketClient.h"
#include "Core/WarpConfiguration.h"

namespace Warpr::Messaging
{
  class WebRtcClient
  {
    inline static const Axodox::Infrastructure::logger _logger{ "WebRtcClient" };
    Axodox::Infrastructure::event_owner _events;

  public:
    WebRtcClient(Axodox::Infrastructure::dependency_container* container);

  private:
    std::shared_ptr<WarpConfiguration> _settings;
    std::shared_ptr<WebSocketClient> _signaler;

    std::unique_ptr<rtc::PeerConnection> _peerConnection;
    std::shared_ptr<rtc::DataChannel> _dataChannel;

    Axodox::Infrastructure::event_subscription _signalerConnectedSubscription;

    void OnSignalerConnected(WebSocketClient* sender);
  };
}