#include "warpr_includes.h"
#include "InputProvider.h"
#include "Core/WarpConfiguration.h"

using namespace Axodox::Infrastructure;
using namespace Warpr;
using namespace Warpr::Messaging;

namespace Warpr::Input
{
  InputProvider::InputProvider(Axodox::Infrastructure::dependency_container* container)
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

  void InputProvider::PushInput(const PointerInput& input)
  {
    if (!_sink) return;
    _sink->OnPointerInput(input);
  }
}
