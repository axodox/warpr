#pragma once
#include "FrameSource.h"

namespace Warpr::Capture
{
  struct WARP_DRIVE_API FrameSourceFactory : public FrameSourceDescription
  {
    virtual FrameSourceKind Type() const override;

    FrameSourceFactory() = default;
    FrameSourceFactory(std::function<std::unique_ptr<Warpr::Capture::FrameSource>()>&& factory);

    std::function<std::unique_ptr<Warpr::Capture::FrameSource>()> Factory;
  };
}