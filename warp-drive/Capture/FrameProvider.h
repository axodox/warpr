#pragma once
#include "warpr_includes.h"
#include "FrameSource.h"

namespace Warpr::Capture
{
  class FrameProvider
  {
    inline static const Axodox::Infrastructure::logger _logger{ "FrameProvider" };
    Axodox::Infrastructure::event_owner _events;

  public:
    Axodox::Infrastructure::event_publisher<FrameSource*, const Frame&> FrameArrived;

    FrameProvider(Axodox::Infrastructure::dependency_container* container);

  private:
    std::unique_ptr<FrameSource> _source;

    void OnFrameArrived(FrameSource* sender, const Frame& eventArgs);
  };
}