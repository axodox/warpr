#include "warpr_includes.h"
#include "StreamingMessages.h"

using namespace Axodox::Infrastructure;

namespace Warpr::Messaging
{
  Axodox::Infrastructure::type_registry<WarprStreamingMessage> WarprStreamingMessage::derived_types = type_registry<WarprStreamingMessage>::create<
    PointerInputMessage>();

  Point::Point() :
    X(this, "X"),
    Y(this, "Y")
  { }
  
  PointerInputMessage::PointerInputMessage() :
    PointerId(this, "PointerId"),
    PointerType(this, "PointerType"),
    PointerAction(this, "PointerAction"),
    Position(this, "Position")
  { }

  WarprStreamingMessageType PointerInputMessage::Type() const
  {
    return WarprStreamingMessageType::PointerInputMessage;
  }
}