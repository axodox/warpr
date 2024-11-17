#pragma once
#include "warpr_includes.h"
#include "Capture/FrameProvider.h"
#include "Encoder/VideoPreprocessor.h"
#include "Encoder/VideoEncoder.h"
#include "Messaging/WebRtcClient.h"

namespace Warpr::Core
{
  class WarpSession
  {
    inline static const Axodox::Infrastructure::logger _logger{ "WarpSession" };

  public:
    WarpSession(Axodox::Infrastructure::dependency_container* container);

  private:
    std::mutex _mutex;

    std::shared_ptr<Capture::FrameProvider> _frameProvider;
    std::shared_ptr<Encoder::VideoPreprocessor> _videoPreprocessor;
    std::shared_ptr<Encoder::VideoEncoder> _videoEncoder;
    std::shared_ptr<Messaging::WebRtcClient> _webRtcClient;
    bool _isKeyFrameSent = false;

    Axodox::Infrastructure::event_subscription _frameArrivedSubscription;

    void OnFrameArrived(Capture::FrameSource* sender, const Capture::Frame& eventArgs);
  };
}