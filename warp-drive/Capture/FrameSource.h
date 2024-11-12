#pragma once
#include "warpr_includes.h"
#include <d3d11.h>

namespace Warpr::Capture
{
  using Frame = winrt::com_ptr<ID3D11Texture2D>;

  enum class FrameSourceKind
  {
    Unknown,
    Window
  };

  struct FrameSourceDescription
  {
    virtual FrameSourceKind Type() const = 0;
    virtual ~FrameSourceDescription() = default;
  };

  class FrameSource
  {
  private:
    Axodox::Infrastructure::event_owner _events;

  public:
    Axodox::Infrastructure::event_publisher<FrameSource*, const Frame&> FrameArrived;

    FrameSource();
    virtual ~FrameSource() = default;

    static std::unique_ptr<FrameSource> Create(const FrameSourceDescription& description);

  protected:
    void PushFrame(const Frame& frame);
  };
}