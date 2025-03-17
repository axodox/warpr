#pragma once
#include "warpr_includes.h"
#include "Input/PointerInput.h"

namespace Warpr::Messaging
{
  named_enum(WarprStreamingMessageType,
    Unknown,
    PointerInputMessage,
    ResizeSurfaceMessage
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

    Point();
  };

  struct PointerInputMessage : public WarprStreamingMessage
  {
    Axodox::Json::json_property<int> PointerId;
    Axodox::Json::json_property<Input::PointerType> PointerType;
    Axodox::Json::json_property<Input::PointerAction> PointerAction;
    Axodox::Json::json_property<Point> Position;
    Axodox::Json::json_property<Input::PointerFlags> Flags;
    Axodox::Json::json_property<int> WheelDelta;

    PointerInputMessage();

    virtual WarprStreamingMessageType Type() const override;

    Input::PointerInput ToInput() const;
  };

  struct ResizeSurfaceMessage : public WarprStreamingMessage
  {
    Axodox::Json::json_property<uint32_t> Width;
    Axodox::Json::json_property<uint32_t> Height;

    ResizeSurfaceMessage();

    virtual WarprStreamingMessageType Type() const override;
  };
}