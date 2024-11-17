#include "warpr_includes.h"
#include "WarpSession.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace Warpr::Capture;
using namespace Warpr::Encoder;
using namespace Warpr::Messaging;
using namespace std;

namespace Warpr::Core
{
  WarpSession::WarpSession(Axodox::Infrastructure::dependency_container* container) :
    _frameProvider(container->resolve<FrameProvider>()),
    _videoPreprocessor(container->resolve<VideoPreprocessor>()),
    _videoEncoder(container->resolve<VideoEncoder>()),
    _webRtcClient(container->resolve<WebRtcClient>()),
    _frameArrivedSubscription(_frameProvider->FrameArrived({ this, &WarpSession::OnFrameArrived }))
  { }

  void WarpSession::OnFrameArrived(Capture::FrameSource* sender, const Capture::Frame& eventArgs)
  {
    lock_guard lock(_mutex);

    if (!_webRtcClient->IsConnected())
    {
      _isKeyFrameSent = false;
      return;
    }

    auto inputFrame = eventArgs;
    _videoPreprocessor->ProcessFrame(inputFrame);
    
    auto encodedFrame = _videoEncoder->EncodeFrame(inputFrame);
    if (encodedFrame.Type == FrameType::Key) _logger.log(log_severity::information, "Key frame.");
    
    if (_isKeyFrameSent || encodedFrame.Type == FrameType::Key)
    {
      _isKeyFrameSent = true;

      memory_stream message;
      message.write(encodedFrame.Index);
      message.write(encodedFrame.Type);
      message.write(encodedFrame.Bytes);
      _webRtcClient->SendMessage(message, WebRtcChannel::LowLatency);
    }
  }
}