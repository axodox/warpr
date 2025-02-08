#include "warpr_includes.h"
#include "DesktopDuplicationSource.h"

using namespace Axodox::Threading;
using namespace Warpr::Graphics;
using namespace winrt;

namespace Warpr::Capture
{
  DesktopDuplicationSource::DesktopDuplicationSource(Axodox::Infrastructure::dependency_container* container) :
    _device(container->resolve<GraphicsDevice>()->Device()),
    _output(GetDefaultOutput())
  {
    _worker = background_thread{ { this, &DesktopDuplicationSource::Worker }, "* desktop duplication" };
  }

  winrt::com_ptr<IDXGIOutput6> DesktopDuplicationSource::GetDefaultOutput()
  {
    com_ptr<IDXGIAdapter> adapter;
    _device.as<IDXGIDevice>()->GetAdapter(adapter.put());

    com_ptr<IDXGIOutput> dxgiOutput;
    for (uint32_t outputIndex = 0u; adapter->EnumOutputs(outputIndex, dxgiOutput.put()) != DXGI_ERROR_NOT_FOUND; outputIndex++)
    {
      auto dxgiOutput2 = dxgiOutput.try_as<IDXGIOutput6>();
      if (dxgiOutput2) return dxgiOutput2;

      dxgiOutput = nullptr;
    }
    return nullptr;
  }

  void DesktopDuplicationSource::Worker()
  {
    while (!_worker.is_exiting())
    {
      if (!_duplication)
      {
        //DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        //check_hresult(_output->DuplicateOutput1(_device.get(), 0, 1, &format, _duplication.put()));
        check_hresult(_output->DuplicateOutput(_device.get(), _duplication.put()));
      }

      DXGI_OUTDUPL_FRAME_INFO frameInfo{};
      com_ptr<IDXGIResource> resource;
      auto result = _duplication->AcquireNextFrame(8, &frameInfo, resource.put());
      if (result == ERROR_SUCCESS)
      {
        auto texture = resource.as<ID3D11Texture2D>();
        PushFrame(texture);

        texture = nullptr;
        resource = nullptr;
        result = _duplication->ReleaseFrame();
        if (result == DXGI_ERROR_ACCESS_LOST)
        {
          _duplication = nullptr;
        }
      }
      else if (result == DXGI_ERROR_WAIT_TIMEOUT)
      {
        continue;
      }
      else
      {
        _duplication = nullptr;
      }
    }
  }
}