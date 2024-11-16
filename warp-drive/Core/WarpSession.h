#pragma once
#include "warpr_includes.h"
#include "Capture/FrameProvider.h"
#include "Encoder/VideoPreprocessor.h"
#include "Encoder/VideoEncoder.h"

namespace Warpr::Core
{
  class WarpSession
  {
  public:
    WarpSession(Axodox::Infrastructure::dependency_container* container);

  private:
    std::shared_ptr<Capture::FrameProvider> _frameProvider;
    std::shared_ptr<Encoder::VideoPreprocessor> _videoPreprocessor;
    std::shared_ptr<Encoder::VideoEncoder> _videoEncoder;

    Axodox::Infrastructure::event_subscription _frameArrivedSubscription;

    void OnFrameArrived(Capture::FrameSource* sender, const Capture::Frame& eventArgs);
  };
}