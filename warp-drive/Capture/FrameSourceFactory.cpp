#include "warpr_includes.h"
#include "FrameSourceFactory.h"

using namespace Axodox::Infrastructure;
using namespace std;

namespace Warpr::Capture
{
  FrameSourceFactory::FrameSourceFactory(std::function<std::unique_ptr<Warpr::Capture::FrameSource>()>&& factory) :
    Factory(move(factory))
  { }

  FrameSourceKind FrameSourceFactory::Type() const
  {
    return FrameSourceKind::Factory;
  }
}