#include "warpr_includes.h"
#include "FrameSource.h"
#include "WindowSource.h"
#include "FrameSourceFactory.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace Warpr::Capture
{
  FrameSource::FrameSource() :
    FrameArrived(_events)
  { }

  std::unique_ptr<FrameSource> FrameSource::Create(Axodox::Infrastructure::dependency_container* container, const FrameSourceDescription* description)
  {
    if (!description)
    {
      _logger.log(log_severity::warning, "No frame source configured, there is nothing to stream.");
      return nullptr;
    }

    switch (description->Type())
    {
    case FrameSourceKind::Window:
      return make_unique<WindowSource>(container, static_cast<const WindowSourceDescription*>(description));

    case FrameSourceKind::Factory:
      return static_cast<const FrameSourceFactory*>(description)->Factory();

    default:
      throw logic_error("Unsupported frame source type description encountered.");
    }
  }

  void FrameSource::PushFrame(const Frame& frame)
  {
    _events.raise(FrameArrived, this, frame);
  }
}