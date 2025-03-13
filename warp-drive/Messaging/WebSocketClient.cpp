#include "warpr_includes.h"
#include "WebSocketClient.h"

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
    _settings(container->resolve<WarpConfiguration>())
  {
    //Connected
    _socket.onOpen([=] {
      _logger.log(log_severity::information, "Connected to {}.", _settings->GatewayUri);

      ConnectionRequest connectionRequest;
      *connectionRequest.SessionId = _settings->SessionId;
      SendMessage(connectionRequest);

      _events.raise(Connected, this);
      });

    //Received
    _socket.onMessage([=](const auto& message) {
      ReceiveMessage(message);
      });

    //Disconnected
    _socket.onClosed([=] {
      _logger.log(log_severity::information, "Disconnected from {}.", _settings->GatewayUri);
      _events.raise(Disconnected, this);
      if (_isDisposing) return;

      this_thread::sleep_for(1s);
      _socket.open(_settings->GatewayUri);
      });

    //Start
    _socket.open(_settings->GatewayUri);
  }

  WebSocketClient::~WebSocketClient()
  {
    _isDisposing = true;
  }

  bool WebSocketClient::IsConnected() const
  {
    return _socket.isOpen();
  }

  void WebSocketClient::SendMessage(const WarprSignalingMessage& warprMessage)
  {
    std::string jsonMessage = stringify_json(&warprMessage);
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
    auto warprMessage = try_parse_json<unique_ptr<WarprSignalingMessage>>(jsonMessage);
    if (!warprMessage)
    {
      _logger.log(log_severity::error, "Failed to parse message:\n{}", jsonMessage);
      return;
    }

    _events.raise(MessageReceived, this, warprMessage->get());
  }
}
