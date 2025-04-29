#include "warpr_includes.h"
#include "WarpDrive.h"
#include "Infrastructure/RtcLogger.h"
#include "Core/WarpSession.h"
#include "Messaging/WebRtcClient.h"

using namespace Axodox::Infrastructure;
using namespace Warpr;
using namespace Warpr::Core;
using namespace Warpr::Infrastructure;
using namespace Warpr::Messaging;
using namespace std;

namespace Warpr
{
  WarpDrive::WarpDrive(const WarpConfiguration& configuration) :
    MessageReceived(_events)
  {
    _logger.log(log_severity::information, L"Hello Warpr!");
    _logger.log(log_severity::information, L"Initializing warp drive...");
    InitRtcLogger(configuration.NetworkingLogSeverity);

    _container.add<WarpConfiguration>(make_shared<WarpConfiguration>(configuration));
    _container.resolve<WarpSession>();

    _messageReceivedSubscription = _container.resolve<WebRtcClient>()->AuxMessageReceived([=](auto, const rtc::message_variant* message) {
      _events.raise(MessageReceived, this, message);
      });

    _logger.log(log_severity::information, L"Warp drive ready.");
  }

  WarpDrive::~WarpDrive()
  {
    _logger.log(log_severity::information, L"Warp drive shutting down...");
  }

  void WarpDrive::SendMessage(const rtc::message_variant& message)
  {
    _container.resolve<WebRtcClient>()->SendAuxMessage(message);
    
  }
}