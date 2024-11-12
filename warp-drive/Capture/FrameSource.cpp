#include "warpr_includes.h"
#include "FrameSource.h"
#include "WindowSource.h"

using namespace std;

namespace Warpr::Capture
{
  FrameSource::FrameSource() :
    FrameArrived(_events)
  { }

  std::unique_ptr<FrameSource> FrameSource::Create(const FrameSourceDescription& description)
  {
    switch (description.Type())
    {
    case FrameSourceKind::Window:
      return make_unique<WindowSource>(static_cast<const WindowSourceDescription&>(description));
    default:
      throw logic_error("Unsupported frame source type description encountered.");
    }
  }

  void FrameSource::PushFrame(const Frame& frame)
  {
    _events.raise(FrameArrived, this, frame);
  }
}