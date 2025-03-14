#include "warpr_includes.h"
#include "WarpDrive.h"
#include "Infrastructure/RtcLogger.h"
#include "Core/WarpSession.h"

using namespace Axodox::Infrastructure;
using namespace Warpr;
using namespace Warpr::Core;
using namespace Warpr::Infrastructure;
using namespace std;

namespace Warpr
{
  WarpDrive::WarpDrive(const WarpConfiguration& configuration)
  {
    _logger.log(log_severity::information, L"Hello Warpr!");
    _logger.log(log_severity::information, L"Initializing warp drive...");
    InitRtcLogger(configuration.NetworkingLogSeverity);

    _container.add<WarpConfiguration>(make_shared<WarpConfiguration>(configuration));
    _container.resolve<WarpSession>();

    _logger.log(log_severity::information, L"Warp drive ready.");
  }

  WarpDrive::~WarpDrive()
  {
    _logger.log(log_severity::information, L"Warp drive shutting down...");
  }
}