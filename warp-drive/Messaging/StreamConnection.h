#pragma once
#include "warpr_includes.h"
#include "StreamingMessages.h"
#include "WebRtcClient.h"

namespace Warpr::Messaging
{
  class StreamConnection
  {
    Axodox::Infrastructure::event_owner _events;

  public:
    Axodox::Infrastructure::event_publisher<StreamConnection*, const WarprStreamingMessage*> MessageReceived;

    StreamConnection(Axodox::Infrastructure::dependency_container* container);

  private:
    std::shared_ptr<WebRtcClient> _client;
    Axodox::Infrastructure::event_subscription _messageReceivedSubscription;

    void OnMessageReceived(WebRtcClient* sender, const rtc::message_variant* message);
  };
}