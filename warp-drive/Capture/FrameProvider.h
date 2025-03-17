#pragma once
#include "warpr_includes.h"
#include "FrameSource.h"
#include "Messaging/StreamConnection.h"

namespace Warpr::Capture
{
  class FrameProvider
  {
    inline static const Axodox::Infrastructure::logger _logger{ "FrameProvider" };
    Axodox::Infrastructure::event_owner _events;

  public:
    Axodox::Infrastructure::event_publisher<FrameSource*, const Frame&> FrameArrived;

    FrameProvider(Axodox::Infrastructure::dependency_container* container);

    FrameSource* Source() const;

  private:
    std::shared_ptr<Messaging::StreamConnection> _connection;
    std::unique_ptr<FrameSource> _source;

    Axodox::Infrastructure::event_subscription _messageReceivedSubscription;

    void OnFrameArrived(FrameSource* sender, const Frame& eventArgs);

    void OnMessageReceived(Messaging::StreamConnection* sender, const Messaging::WarprStreamingMessage* eventArgs);
  };
}