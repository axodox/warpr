#pragma once
#include "warpr_includes.h"
#include "Capture/FrameProvider.h"
#include "Encoder/VideoPreprocessor.h"
#include "Encoder/VideoEncoder.h"
#include "Messaging/WebRtcClient.h"
#include "Messaging/WebSocketClient.h"
#include "Input/InputProvider.h"

namespace Warpr::Core
{
  class WarpSession
  {
    inline static const Axodox::Infrastructure::logger _logger{ "WarpSession" };

  public:
    WarpSession(Axodox::Infrastructure::dependency_container* container);

  private:
    std::mutex _mutex;

    std::shared_ptr<WarpConfiguration> _configuration;
    std::shared_ptr<Capture::FrameProvider> _frameProvider;
    std::shared_ptr<Input::InputProvider> _inputProvider;
    std::shared_ptr<Encoder::VideoPreprocessor> _videoPreprocessor;
    std::shared_ptr<Encoder::VideoEncoder> _videoEncoder;
    std::shared_ptr<Messaging::WebRtcClient> _webRtcClient;
    std::shared_ptr<Messaging::StreamConnection> _connection;
    std::shared_ptr<Messaging::WebSocketClient> _webSocketClient;

    bool _isKeyFrameSent = false;
    std::chrono::steady_clock::time_point _lastStateTime = {}, _lastFrameTime = {};
    uint32_t _frameCount = 0;
    float _renderingResolutionScale = 1.f;

    Axodox::Infrastructure::event_subscription _streamingMessageReceivedSubscription;
    Axodox::Infrastructure::event_subscription _signalingMessageReceivedSubscription;
    Axodox::Infrastructure::event_subscription _frameArrivedSubscription;

    void OnFrameArrived(Capture::FrameSource* sender, const Capture::Frame& eventArgs);

    void SendFrame(const Encoder::EncodedFrame& frame);

    void OnStreamingMessageReceived(Messaging::StreamConnection* sender, const Messaging::WarprStreamingMessage* message);
    void OnPointerInputMessage(const Messaging::PointerInputMessage* message);
    void OnResizeSurfaceMessage(const Messaging::ResizeSurfaceMessage* message);
    void OnRequestKeyFrameMessage(const Messaging::RequestKeyFrameMessage* message);

    void OnSignalingMessageReceived(Messaging::WebSocketClient* sender, const Messaging::WarprSignalingMessage* message);
    void OnPairingCompleteMessage(const Messaging::PairingCompleteMessage* message);
  };
}