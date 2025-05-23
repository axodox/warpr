#include "warpr_includes.h"
#include "StreamConnection.h"

using namespace Axodox::Json;
using namespace std;

namespace Warpr::Messaging
{
  StreamConnection::StreamConnection(Axodox::Infrastructure::dependency_container* container) :
    MessageReceived(_events),
    _client(container->resolve<WebRtcClient>()),
    _messageReceivedSubscription(_client->ControlMessageReceived({ this, &StreamConnection::OnStreamingMessageReceived }))
  { }

  void StreamConnection::OnStreamingMessageReceived(WebRtcClient* sender, const rtc::message_variant* message)
  {
    if (!holds_alternative<string>(*message)) return;

    auto text = get<string>(*message);
    auto result = try_parse_json<unique_ptr<WarprStreamingMessage>>(text);
    if (!result) return;

    _events.raise(MessageReceived, this, result->get());
  }
}