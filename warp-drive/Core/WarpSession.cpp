#include "warpr_includes.h"
#include "WarpSession.h"

using namespace Warpr::Capture;
using namespace Warpr::Encoder;

namespace Warpr::Core
{
  WarpSession::WarpSession(Axodox::Infrastructure::dependency_container* container) :
    _frameProvider(container->resolve<FrameProvider>()),
    _videoPreprocessor(container->resolve<VideoPreprocessor>()),
    _videoEncoder(container->resolve<VideoEncoder>()),
    _frameArrivedSubscription(_frameProvider->FrameArrived({ this, &WarpSession::OnFrameArrived }))
  {

  }

  void WarpSession::OnFrameArrived(Capture::FrameSource* sender, const Capture::Frame& eventArgs)
  {
    auto frame = eventArgs;
    _videoPreprocessor->ProcessFrame(frame);
    _videoEncoder->PushFrame(frame);
  }
}