#pragma once
#include "warpr_includes.h"

namespace Warpr::Capture
{
  using Frame = winrt::com_ptr<ID3D11Texture2D>;

  enum class FrameSourceKind
  {
    Unknown,
    Window,
    Factory
  };

  struct WARP_DRIVE_API FrameSourceDescription
  {
    virtual FrameSourceKind Type() const = 0;
    virtual ~FrameSourceDescription() = default;
  };

  class WARP_DRIVE_API FrameSource
  {
    inline static const Axodox::Infrastructure::logger _logger{ "FrameSource" };
    Axodox::Infrastructure::event_owner _events;

  public:
    Axodox::Infrastructure::event_publisher<FrameSource*, const Frame&> FrameArrived;

    FrameSource();
    virtual ~FrameSource() = default;

    virtual void Resize(uint32_t width, uint32_t height);

    static std::unique_ptr<FrameSource> Create(Axodox::Infrastructure::dependency_container* container, const FrameSourceDescription* description);

  protected:
    void PushFrame(const Frame& frame);
  };
}