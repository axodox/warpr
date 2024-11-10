#include "warpr_includes.h"
#include "WarpDrive.h"
#include "Messaging/WebSocketClient.h"
#include "Infrastructure/RtcLogger.h"

using namespace Axodox::Infrastructure;
using namespace Warpr;
using namespace Warpr::Infrastructure;
using namespace Warpr::Messaging;
using namespace std;

namespace Warpr
{
  WarpDrive::WarpDrive(const WarpConfiguration& configuration)
  {
    _logger.log(log_severity::information, L"Hello Warpr!");
    _logger.log(log_severity::information, L"Initializing warp drive...");
    InitRtcLogger(configuration.NetworkingLogSeverity);

    _container.add<WarpConfiguration>(make_shared<WarpConfiguration>(configuration));
    _container.resolve<WebSocketClient>();

    _logger.log(log_severity::information, L"Warp drive ready.");
  }

  WarpDrive::~WarpDrive()
  {
    _logger.log(log_severity::information, L"Warp drive shutting down...");
  }
}