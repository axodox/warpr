#include "warpr_includes.h"
#include "FrameProvider.h"
#include "Core/WarpConfiguration.h"

using namespace Axodox::Infrastructure;
using namespace Warpr;
using namespace Warpr::Messaging;

namespace Warpr::Capture
{
  FrameProvider::FrameProvider(Axodox::Infrastructure::dependency_container* container) :
    FrameArrived(_events)
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

  void FrameProvider::ResizeSurface(uint32_t width, uint32_t height)
  {
    if (!_source || width == 0 || height == 0) return;
    _source->Resize(width, height);
  }

  void FrameProvider::OnFrameArrived(FrameSource* sender, const Frame& eventArgs)
  {
    _events.raise(FrameArrived, sender, eventArgs);
  }
}
