#include "warpr_includes.h"
#include "InputSink.h"
#include "InputSinkFactory.h"
#include "Messaging/StreamConnection.h"

using namespace Axodox::Infrastructure;
using namespace Warpr::Messaging;
using namespace std;

namespace Warpr::Input
{
  std::unique_ptr<InputSink> InputSink::Create(Axodox::Infrastructure::dependency_container* container, const InputSinkDescription* description)
  {
    if (!description)
    {
      _logger.log(log_severity::warning, "No input sink configured, inputs will be discarded.");
      return nullptr;
    }

    switch (description->Type())
    {
    case InputSinkKind::Factory:
      return static_cast<const InputSinkFactory*>(description)->Factory();

    default:
      throw logic_error("Unsupported frame source type description encountered.");
    }
  }

  void InputSink::OnPointerInput(const PointerInput& input)
  { }
}