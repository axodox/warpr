#include "warpr_includes.h"
#include "InputProvider.h"
#include "Core/WarpConfiguration.h"

using namespace Axodox::Infrastructure;
using namespace Warpr;
using namespace Warpr::Messaging;

namespace Warpr::Input
{
  InputProvider::InputProvider(Axodox::Infrastructure::dependency_container* container) :
    _connection(container->resolve<StreamConnection>()),
    _subscription(_connection->MessageReceived({ this, &InputProvider::OnMessageReceived }))
  {
    _logger.log(log_severity::information, "Creating input sink...");

    auto configuration = container->resolve<WarpConfiguration>();
    _sink = InputSink::Create(container, configuration->InputSink.get());

    _logger.log(log_severity::information, "Frame source ready.");
  }

  InputSink* InputProvider::Sink() const
  {
    return _sink.get();
  }

  void InputProvider::OnMessageReceived(Messaging::StreamConnection* sender, const Messaging::WarprStreamingMessage* message)
  {
    if (!_sink) return;

    switch (message->Type())
    {
    case WarprStreamingMessageType::PointerInputMessage:
      _sink->OnPointerInput(static_cast<const PointerInputMessage*>(message)->ToInput());
      break;
    }
  }
}
