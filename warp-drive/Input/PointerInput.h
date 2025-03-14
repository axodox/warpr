#pragma once
#include "warpr_includes.h"

namespace Warpr::Input
{
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
    Released,
    WheelChanged
  );

  enum class PointerFlags {
    None = 0,
    LeftButton = 1,
    MiddleButton = 2,
    RightButton = 4,
    Wheel = 8,
    Captured = 16
  };

  struct WARP_DRIVE_API PointerInput
  {
    int Id = 0;
    PointerType Type = PointerType::Unknown;
    PointerAction Action = PointerAction::Unknown;
    float X = 0, Y = 0;
    PointerFlags Flags = PointerFlags::None;
    int WheelDelta = 0;
  };
}