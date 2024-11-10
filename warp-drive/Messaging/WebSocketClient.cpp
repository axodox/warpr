#include "warpr_includes.h"
#include "WebSocketClient.h"
#include "Core/WarpConfiguration.h"

using namespace Axodox::Infrastructure;
using namespace Axodox::Json;
using namespace Axodox::Threading;
using namespace std;
using namespace std::string_literals;

namespace Warpr::Messaging
{
  WebSocketClient::WebSocketClient(Axodox::Infrastructure::dependency_container* container) :
    Connected(_events),
    MessageReceived(_events),
    Disconnected(_events),
    _uri(container->resolve<WarpConfiguration>()->GatewayUri)
  {
    _socket.onOpen([=] {
      _logger.log(log_severity::information, "Connected to {}.", _uri);
      _events.raise(Connected, this);
      });
    _socket.onMessage([=](const auto& message) {
      ReceiveMessage(message);
      });
    _socket.onClosed([=] {
      _logger.log(log_severity::information, "Disconnected from {}.", _uri);
      _events.raise(Disconnected, this);
      if (_isDisposing) return;

      this_thread::sleep_for(1s);
      _socket.open(_uri);
      });
    _socket.open(_uri);
  }

  WebSocketClient::~WebSocketClient()
  {
    _isDisposing = true;
  }

  bool WebSocketClient::IsConnected() const
  {
    return _socket.isOpen();
  }

  void WebSocketClient::SendMessage(const WarprMessage& warprMessage)
  {
    unique_ptr<WarprMessage> ptr{ const_cast<WarprMessage*>(&warprMessage) };
    std::string jsonMessage = stringify_json(ptr);
    ptr.release();

    _socket.send(jsonMessage);
  }

  void WebSocketClient::ReceiveMessage(const rtc::message_variant& rawMessage)
  {
    if (!holds_alternative<string>(rawMessage))
    {
      _logger.log(log_severity::warning, "Expected text message, but received binary.");
      return;
    }
    auto& jsonMessage = get<string>(rawMessage);

    auto warprMessage = try_parse_json<unique_ptr<WarprMessage>>(jsonMessage);
    if (!warprMessage)
    {
      _logger.log(log_severity::error, "Failed to parse message:\n{}", jsonMessage);
      return;
    }

    _events.raise(MessageReceived, this, warprMessage->get());
  }
}
