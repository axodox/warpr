#pragma once
#include "WebSocketClient.h"
#include "Core/WarpConfiguration.h"

namespace Warpr::Messaging
{
  enum class WebRtcChannel
  {
    Reliable,
    LowLatency
  };

  class WebRtcClient
  {
    inline static const Axodox::Infrastructure::logger _logger{ "WebRtcClient" };
    Axodox::Infrastructure::event_owner _events;

  public:
    WebRtcClient(Axodox::Infrastructure::dependency_container* container);

    bool IsConnected() const;
    void SendMessage(std::span<const uint8_t> bytes, WebRtcChannel channelType);

  private:
    static const std::string_view _stateNames[];

    std::mutex _mutex;
    std::shared_ptr<WarpConfiguration> _settings;
    std::shared_ptr<WebSocketClient> _signaler;

    std::unique_ptr<rtc::PeerConnection> _peerConnection;
    std::shared_ptr<rtc::DataChannel> _reliableChannel;
    std::shared_ptr<rtc::DataChannel> _lowLatencyChannel;

    Axodox::Infrastructure::event_subscription _signalerMessageReceivedSubscription;

    void OnSignalerMessageReceived(WebSocketClient* sender, const WarprMessage* message);
    void Connect();
  };
}