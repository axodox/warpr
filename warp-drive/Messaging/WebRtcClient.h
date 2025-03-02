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

  struct WebRtcMessage
  {
    std::variant<std::span<const uint8_t>, std::string_view> Data = {};
    WebRtcChannel Channel = {};
  };

  class WebRtcClient
  {
    inline static const Axodox::Infrastructure::logger _logger{ "WebRtcClient" };
    Axodox::Infrastructure::event_owner _events;

  public:
    WebRtcClient(Axodox::Infrastructure::dependency_container* container);

    bool IsConnected() const;
    void SendMessage(std::span<const uint8_t> bytes, WebRtcChannel channelType);

    Axodox::Infrastructure::event_publisher<WebRtcClient*, WebRtcMessage> MessageReceived;

  private:
    static const std::string_view _stateNames[];
    static const std::string_view _iceStateNames[];
    static const std::string_view _gatheringStateNames[];
    Axodox::Infrastructure::dependency_container_ref _containerRef;

    uint32_t _messageIndex = 0;

    std::mutex _mutex;
    std::shared_ptr<WarpConfiguration> _settings;
    std::shared_ptr<WebSocketClient> _signaler;

    std::unique_ptr<rtc::PeerConnection> _peerConnection;
    std::shared_ptr<rtc::DataChannel> _reliableChannel;
    std::shared_ptr<rtc::DataChannel> _lowLatencyChannel;

    Axodox::Infrastructure::event_subscription _signalerMessageReceivedSubscription;

    void OnSignalerMessageReceived(WebSocketClient* sender, const WarprMessage* message);
    winrt::fire_and_forget ConnectAsync();

    void OnMessageReceived(WebRtcChannel channel, rtc::message_variant message);
  };
}