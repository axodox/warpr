#include "warpr_includes.h"
#include "GraphicsDevice.h"
#include "Core/WarpConfiguration.h"

using namespace Axodox::Infrastructure;
using namespace winrt;

namespace Warpr::Graphics
{
  GraphicsDevice::GraphicsDevice(Axodox::Infrastructure::dependency_container* container)
  {
    auto configuration = container->resolve<WarpConfiguration>();
    if (configuration->Device)
    {
      _logger.log(log_severity::information, "Using user provided device.");

      _device = configuration->Device;
      _device->GetImmediateContext(_context.put());
    }
    else
    {
      _logger.log(log_severity::information, "Initializing graphics device...");

      check_hresult(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
#ifdef _DEBUG
        D3D11_CREATE_DEVICE_DEBUG,
#else
        0,
#endif
        nullptr,
        0,
        D3D11_SDK_VERSION,
        _device.put(),
        nullptr,
        _context.put()
      ));

      _logger.log(log_severity::information, "Graphics device ready.");
    }
  }

  const winrt::com_ptr<ID3D11Device>& GraphicsDevice::Device() const
  {
    return _device;
  }

  const winrt::com_ptr<ID3D11DeviceContext>& GraphicsDevice::Context() const
  {
    return _context;
  }
}