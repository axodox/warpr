#include "warpr_includes.h"
#include "WarpSession.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace Warpr::Capture;
using namespace Warpr::Encoder;
using namespace Warpr::Messaging;
using namespace std;
using namespace std::chrono;

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

    static uint32_t counter = 0;
    if (counter++ % 2 != 0) return;

    if (!_webRtcClient->IsConnected())
    {
      _isKeyFrameSent = false;
      return;
    }

    auto inputFrame = eventArgs;
    _videoPreprocessor->ProcessFrame(inputFrame);
    
    auto encodedFrame = _videoEncoder->EncodeFrame(inputFrame);
    if (encodedFrame.Type == FrameType::Key)
    {
      _logger.log(log_severity::information, "Key frame {}.", encodedFrame.Index);
    }

    if (_isKeyFrameSent || encodedFrame.Type == FrameType::Key)
    {
      _isKeyFrameSent = true;
      SendFrame(encodedFrame);
    }

    _frameCount++;
    auto now = steady_clock::now();
    if (now - _lastStateTime > 1s)
    {
      auto fps = _frameCount / duration_cast<duration<float>>(now - _lastStateTime).count();
      _logger.log(log_severity::debug, "Output: {:.1f} fps", fps);
      _lastStateTime = now;
      _frameCount = 0;
    }
  }

  void WarpSession::SendFrame(const Encoder::EncodedFrame& frame)
  {
    memory_stream message;
    message.write(frame.Type);
    message.write(frame.Index);
    message.write(frame.Bytes);
    _webRtcClient->SendMessage(message, WebRtcChannel::LowLatency);
  }
}