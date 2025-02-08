#include "warpr_includes.h"
#include "FrameProvider.h"
#include "Core/WarpConfiguration.h"
#include "DesktopDuplicationSource.h"

using namespace Axodox::Infrastructure;
using namespace Warpr;
using namespace std;

namespace Warpr::Capture
{
  FrameProvider::FrameProvider(Axodox::Infrastructure::dependency_container* container) :
    FrameArrived(_events)
  {
    _logger.log(log_severity::information, "Creating frame source...");

    auto configuration = container->resolve<WarpConfiguration>();
    _source = FrameSource::Create(container, configuration->FrameSource.get());
    //_source = make_unique<DesktopDuplicationSource>(container);
    if (_source)
    {
      _source->FrameArrived(no_revoke, { this, &FrameProvider::OnFrameArrived });
    }

    _logger.log(log_severity::information, "Frame source ready.");
  }

  void FrameProvider::OnFrameArrived(FrameSource* sender, const Frame& eventArgs)
  {
    _events.raise(FrameArrived, sender, eventArgs);
  }
}
