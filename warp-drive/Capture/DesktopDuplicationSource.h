#pragma once
#pragma once
#include "FrameSource.h"
#include "Graphics/GraphicsDevice.h"

namespace Warpr::Capture
{
  class DesktopDuplicationSource : public FrameSource
  {
    inline static const Axodox::Infrastructure::logger _logger{ "DesktopDuplicationSource" };

  public:
    DesktopDuplicationSource(Axodox::Infrastructure::dependency_container* container);

  private:
    winrt::com_ptr<ID3D11Device> _device;
    winrt::com_ptr<IDXGIOutput6> _output;
    winrt::com_ptr<IDXGIOutputDuplication> _duplication;
    Axodox::Threading::background_thread _worker;

    winrt::com_ptr<IDXGIOutput6> GetDefaultOutput();
    void Worker();
  };
}