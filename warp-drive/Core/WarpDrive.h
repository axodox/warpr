#pragma once
#include "WarpConfiguration.h"

namespace Warpr
{
  struct WARP_DRIVE_API WarpDrive
  {
    inline static const Axodox::Infrastructure::logger _logger{ "WarpDrive" };
    Axodox::Infrastructure::event_owner _events;

  public:
    WarpDrive(const WarpConfiguration& configuration = {});
    ~WarpDrive();

    Axodox::Infrastructure::event_publisher<WarpDrive*, const rtc::message_variant*> MessageReceived;
    void SendMessage(const rtc::message_variant& message);

  private:
    Axodox::Infrastructure::dependency_container _container;
    Axodox::Infrastructure::event_subscription _messageReceivedSubscription;
  };
}