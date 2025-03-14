#pragma once
#include "warpr_includes.h"
#include "InputSink.h"
#include "Messaging/StreamConnection.h"

namespace Warpr::Input
{
  class InputProvider
  {
    inline static const Axodox::Infrastructure::logger _logger{ "InputProvider" };
    Axodox::Infrastructure::event_owner _events;

  public:
    InputProvider(Axodox::Infrastructure::dependency_container* container);

    InputSink* Sink() const;

  private:
    std::shared_ptr<Messaging::StreamConnection> _connection;
    Axodox::Infrastructure::event_subscription _subscription;

    std::unique_ptr<InputSink> _sink;

    void OnMessageReceived(Messaging::StreamConnection* sender, const Messaging::WarprStreamingMessage* message);
  };
}