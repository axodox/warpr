#pragma once
#include "InputSink.h"

namespace Warpr::Input
{
  struct WARP_DRIVE_API InputSinkFactory : public InputSinkDescription
  {
    virtual InputSinkKind Type() const override;

    InputSinkFactory() = default;
    InputSinkFactory(std::function<std::unique_ptr<InputSink>()>&& factory);

    std::function<std::unique_ptr<InputSink>()> Factory;
  };
}