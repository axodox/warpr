#pragma once
#include "WebSocketClient.h"
#include "Core/WarpConfiguration.h"
#include "MessageAssembler.h"
#include "MessageSplitter.h"

namespace Warpr::Messaging
{
  class WebRtcClient
  {
    inline static const Axodox::Infrastructure::logger _logger{ "WebRtcClient" };
    Axodox::Infrastructure::event_owner _events;

    struct PairingConfiguration
    {
      std::vector<rtc::IceServer> IceServers;
      std::chrono::duration<float> ConnectionTimeout;

      bool IsConnectionUnordered;
      std::optional<uint32_t> MaxRetransmits;
      std::optional<std::chrono::milliseconds> MaxPacketLifetime;
    };

  public:
    WebRtcClient(Axodox::Infrastructure::dependency_container* container);

    bool IsConnected() const;
    void SendVideoPacket(const rtc::binary& packet);
    void SendControlMessage(const rtc::message_variant& message);
    void SendAuxMessage(const rtc::message_variant& message);

    Axodox::Infrastructure::event_publisher<WebRtcClient*, const rtc::message_variant*> ControlMessageReceived, AuxMessageReceived;

  private:
    static const std::string_view _stateNames[];
    static const std::string_view _iceStateNames[];
    static const std::string_view _gatheringStateNames[];

    Axodox::Infrastructure::dependency_container_ref _containerRef;

    std::mutex _mutex;
    std::shared_ptr<WarpConfiguration> _settings;
    std::shared_ptr<WebSocketClient> _signaler;

    std::unique_ptr<rtc::PeerConnection> _peerConnection;
    std::shared_ptr<rtc::DataChannel> _streamChannel;
    std::shared_ptr<rtc::DataChannel> _controlChannel;
    std::shared_ptr<rtc::DataChannel> _auxChannel;

    std::chrono::steady_clock::time_point _lastReportingTime;
    uint64_t _dataSentSinceLastReport;

    MessageAssembler _auxMessageAssembler;
    MessageSplitter _auxMessageSplitter;

    PairingConfiguration _configuration;

    Axodox::Infrastructure::event_subscription _signalerMessageReceivedSubscription;
    Axodox::Threading::background_thread _connectionThread;

    void OnSignalingMessageReceived(WebSocketClient* sender, const WarprSignalingMessage* message);
    void OnPairingComplete(const PairingCompleteMessage* message);
    void Connect();
  };
}