#pragma once
#include "FrameSource.h"
#include "Graphics/GraphicsDevice.h"

#include <winrt/windows.foundation.h>
#include <winrt/windows.graphics.h>
#include <winrt/windows.graphics.capture.h>
#include <winrt/windows.graphics.display.h>
#include <winrt/windows.graphics.directx.h>
#include <winrt/windows.graphics.directx.direct3d11.h>
#include <windows.graphics.directx.direct3d11.interop.h>

namespace Warpr::Capture
{
  struct WindowSourceDescription : public FrameSourceDescription
  {
    virtual FrameSourceKind Type() const override;

    std::variant<winrt::Windows::Graphics::DisplayId, winrt::Windows::UI::WindowId> Source;

    WindowSourceDescription(std::variant<winrt::Windows::Graphics::DisplayId, winrt::Windows::UI::WindowId> source);
  };

  class WindowSource : public FrameSource
  {
    inline static const Axodox::Infrastructure::logger _logger{ "WindowSource" };

  public:
    WindowSource(Axodox::Infrastructure::dependency_container* container, const WindowSourceDescription* description);

    std::variant<winrt::Windows::Graphics::DisplayId, winrt::Windows::UI::WindowId> Source() const;

  private:
    winrt::com_ptr<ID3D11Device> _device;
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem _captureItem{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool _framePool{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession _session{ nullptr };
    std::variant<winrt::Windows::Graphics::DisplayId, winrt::Windows::UI::WindowId> _source;
  };
}