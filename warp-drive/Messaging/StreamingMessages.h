#pragma once
#include "warpr_includes.h"

namespace Warpr::Messaging
{
  named_enum(WarprStreamingMessageType,
    Unknown,
    PointerInputMessage
  );

  struct WarprStreamingMessage : public Axodox::Json::json_object_base
  {
    static Axodox::Infrastructure::type_registry<WarprStreamingMessage> derived_types;

    virtual WarprStreamingMessageType Type() const = 0;
  };

  struct Point : public Axodox::Json::json_object_base
  {
    Axodox::Json::json_property<float> X;
    Axodox::Json::json_property<float> Y;

    Point() :
      X(this, "X"),
      Y(this, "Y")
    { }
  };

  named_enum(PointerType,
    Unknown,
    Mouse,
    Pen,
    Touch
  );

  named_enum(PointerAction,
    Unknown,
    Pressed,
    Moved,
    Released
  );

  struct PointerInputMessage : public WarprStreamingMessage
  {
    Axodox::Json::json_property<int> PointerId;
    Axodox::Json::json_property<PointerType> PointerType;
    Axodox::Json::json_property<PointerAction> PointerAction;
    Axodox::Json::json_property<Point> Position;

    PointerInputMessage() :
      PointerId(this, "PointerId"),
      PointerType(this, "PointerType"),
      PointerAction(this, "PointerAction"),
      Position(this, "Position")
    { }

    virtual WarprStreamingMessageType Type() const override
    {
      return WarprStreamingMessageType::PointerInputMessage;
    }
  };
}