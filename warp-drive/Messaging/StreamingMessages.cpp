#include "warpr_includes.h"
#include "StreamingMessages.h"

using namespace Axodox::Infrastructure;
using namespace Warpr::Input;

namespace Warpr::Messaging
{
  Axodox::Infrastructure::type_registry<WarprStreamingMessage> WarprStreamingMessage::derived_types = type_registry<WarprStreamingMessage>::create<
    PointerInputMessage,
    ResizeSurfaceMessage>();

  Point::Point() :
    X(this, "X"),
    Y(this, "Y")
  { }

  PointerInputMessage::PointerInputMessage() :
    PointerId(this, "Id"),
    PointerType(this, "Type"),
    PointerAction(this, "Action"),
    Position(this, "Position"),
    Flags(this, "Flags"),
    WheelDelta(this, "WheelDelta")
  { }

  WarprStreamingMessageType PointerInputMessage::Type() const
  {
    return WarprStreamingMessageType::PointerInputMessage;
  }

  Input::PointerInput PointerInputMessage::ToInput() const
  {
    return PointerInput{
      .Id = *PointerId,
      .Type = *PointerType,
      .Action = *PointerAction,
      .X = *Position->X,
      .Y = *Position->Y,
      .Flags = *Flags,
      .WheelDelta = *WheelDelta
    };
  }

  ResizeSurfaceMessage::ResizeSurfaceMessage() :
    Width(this, "Width"),
    Height(this, "Height")
  { }

  WarprStreamingMessageType ResizeSurfaceMessage::Type() const
  {
    return WarprStreamingMessageType::ResizeSurfaceMessage;
  }
}