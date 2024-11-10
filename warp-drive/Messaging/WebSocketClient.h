#pragma once
#include "warpr_includes.h"
#include "Messages.h"

namespace Warpr::Messaging
{
  class WebSocketClient
  {
    inline static Axodox::Infrastructure::logger _logger{ "WebSocketClient" };
    Axodox::Infrastructure::event_owner _events;

  public:
    WebSocketClient(Axodox::Infrastructure::dependency_container* container);
    ~WebSocketClient();

    Axodox::Infrastructure::event_publisher<WebSocketClient*, const WarprMessage*> MessageReceived;

    void SendMessage(const WarprMessage& message);

  private:
    bool _isDisposing = false;
    std::string _uri;
    rtc::WebSocket _socket;

    void ReceiveMessage(const rtc::message_variant& message);
  };
}