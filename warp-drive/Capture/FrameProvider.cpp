#include "warpr_includes.h"
#include "FrameProvider.h"
#include "Core/WarpConfiguration.h"

using namespace Axodox::Infrastructure;
using namespace Warpr;
using namespace Warpr::Messaging;

namespace Warpr::Capture
{
  FrameProvider::FrameProvider(Axodox::Infrastructure::dependency_container* container) :
    FrameArrived(_events),
    _connection(container->resolve<StreamConnection>()),
    _messageReceivedSubscription(_connection->MessageReceived({ this, &FrameProvider::OnMessageReceived }))
  {
    _logger.log(log_severity::information, "Creating frame source...");

    auto configuration = container->resolve<WarpConfiguration>();
    _source = FrameSource::Create(container, configuration->FrameSource.get());
    if (_source)
    {
      _source->FrameArrived(no_revoke, { this, &FrameProvider::OnFrameArrived });
    }

    _logger.log(log_severity::information, "Frame source ready.");
  }

  FrameSource* FrameProvider::Source() const
  {
    return _source.get();
  }

  void FrameProvider::OnFrameArrived(FrameSource* sender, const Frame& eventArgs)
  {
    _events.raise(FrameArrived, sender, eventArgs);
  }

  void FrameProvider::OnMessageReceived(StreamConnection* sender, const WarprStreamingMessage* eventArgs)
  {
    if (!_source || eventArgs->Type() != WarprStreamingMessageType::ResizeSurfaceMessage) return;

    auto message = static_cast<const ResizeSurfaceMessage*>(eventArgs);
    if (*message->Width > 0 && *message->Height > 0)
    {
      _source->Resize(*message->Width, *message->Height);
    }
  }
}
