#pragma once
#include "warpr_includes.h"
#include "FrameSource.h"
#include "Messaging/StreamConnection.h"

namespace Warpr::Capture
{
  class FrameProvider
  {
    inline static const Axodox::Infrastructure::logger _logger{ "FrameProvider" };
    Axodox::Infrastructure::event_owner _events;

  public:
    Axodox::Infrastructure::event_publisher<FrameSource*, const Frame&> FrameArrived;

    FrameProvider(Axodox::Infrastructure::dependency_container* container);

    FrameSource* Source() const;

    void ResizeSurface(uint32_t width, uint32_t height);

  private:
    std::unique_ptr<FrameSource> _source;

    void OnFrameArrived(FrameSource* sender, const Frame& eventArgs);
  };
}