#include "warpr_includes.h"
#include "WarpSession.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace Warpr::Capture;
using namespace Warpr::Encoder;
using namespace Warpr::Input;
using namespace Warpr::Messaging;
using namespace std;
using namespace std::chrono;

namespace Warpr::Core
{
  WarpSession::WarpSession(Axodox::Infrastructure::dependency_container* container) :
    _configuration(container->resolve<WarpConfiguration>()),
    _frameProvider(container->resolve<FrameProvider>()),
    _inputProvider(container->resolve<InputProvider>()),
    _videoPreprocessor(container->resolve<VideoPreprocessor>()),
    _videoEncoder(container->resolve<VideoEncoder>()),
    _webRtcClient(container->resolve<WebRtcClient>()),
    _connection(container->resolve<StreamConnection>()),
    _messageReceivedSubscription(_connection->MessageReceived({ this, &WarpSession::OnMessageReceived })),
    _frameArrivedSubscription(_frameProvider->FrameArrived({ this, &WarpSession::OnFrameArrived }))
  { }

  void WarpSession::OnFrameArrived(Capture::FrameSource* sender, const Capture::Frame& eventArgs)
  {
    lock_guard lock(_mutex);

    auto now = steady_clock::now();
    if (now - _lastFrameTime < _configuration->MinimumFrameInterval) return;
    _lastFrameTime = now;

    if (!_webRtcClient->IsConnected())
    {
      _isKeyFrameSent = false;
      return;
    }

    auto inputFrame = eventArgs;
    _videoPreprocessor->ProcessFrame(inputFrame);
    
    auto encodedFrame = _videoEncoder->EncodeFrame(inputFrame, !_isKeyFrameSent);
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
    now = steady_clock::now();
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
    Stopwatch watch{"Send message"};

    memory_stream message;
    message.write(frame.Type);
    message.write(frame.Index);
    message.write(frame.Width);
    message.write(frame.Height);
    message.write(frame.Bytes);
    _webRtcClient->SendVideoFrame(message);
  }

  void WarpSession::OnMessageReceived(Messaging::StreamConnection* sender, const Messaging::WarprStreamingMessage* message)
  {
    switch (message->Type())
    {
    case WarprStreamingMessageType::PointerInputMessage:
      OnPointerInputMessage(static_cast<const PointerInputMessage*>(message));
      break;
    case WarprStreamingMessageType::ResizeSurfaceMessage:
      OnResizeSurfaceMessage(static_cast<const ResizeSurfaceMessage*>(message));
      break;
    case WarprStreamingMessageType::RequestKeyFrameMessage:
      OnRequestKeyFrameMessage(static_cast<const RequestKeyFrameMessage*>(message));
      break;
    }
  }

  void WarpSession::OnPointerInputMessage(const Messaging::PointerInputMessage* message)
  {
    _inputProvider->PushInput(message->ToInput());
  }

  void WarpSession::OnResizeSurfaceMessage(const Messaging::ResizeSurfaceMessage* message)
  {
    _logger.log(log_severity::information, "Surface resize requested to {}x{}.", *message->Width, *message->Height);
    _frameProvider->ResizeSurface(*message->Width, *message->Height);
  }

  void WarpSession::OnRequestKeyFrameMessage(const Messaging::RequestKeyFrameMessage* message)
  {
    _logger.log(log_severity::information, "Key frame requested.");
    _isKeyFrameSent = false;
  }
}