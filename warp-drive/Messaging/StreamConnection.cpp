#include "warpr_includes.h"
#include "StreamConnection.h"

using namespace Axodox::Json;
using namespace std;

namespace Warpr::Messaging
{
  StreamConnection::StreamConnection(Axodox::Infrastructure::dependency_container* container) :
    MessageReceived(_events),
    _client(container->resolve<WebRtcClient>()),
    _messageReceivedSubscription(_client->MessageReceived({ this, &StreamConnection::OnMessageReceived }))
  { }

  void StreamConnection::OnMessageReceived(WebRtcClient* sender, WebRtcMessage message)
  {
    if (!holds_alternative<string_view>(message.Data)) return;

    auto text = get<string_view>(message.Data);
    auto result = try_parse_json<unique_ptr<WarprStreamingMessage>>(text);
    if (!result) return;

    _events.raise(MessageReceived, this, result->get());
  }
}