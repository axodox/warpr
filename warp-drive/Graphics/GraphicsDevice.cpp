#include "warpr_includes.h"
#include "GraphicsDevice.h"

using namespace Axodox::Infrastructure;
using namespace winrt;

namespace Warpr::Graphics
{
  GraphicsDevice::GraphicsDevice()
  {
    _logger.log(log_severity::information, "Initializing graphics device...");

    check_hresult(D3D11CreateDevice(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      0,
      nullptr,
      0,
      D3D11_SDK_VERSION,
      _device.put(),
      nullptr,
      _context.put()
    ));

    _logger.log(log_severity::information, "Graphics device ready.");
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