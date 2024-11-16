#pragma once
#include "warpr_includes.h"

#include <d3d11.h>
#include <winrt/base.h>

namespace Warpr::Graphics
{
  class GraphicsDevice
  {
    inline static const Axodox::Infrastructure::logger _logger{ "GraphicsDevice" };

  public:
    GraphicsDevice();

    const winrt::com_ptr<ID3D11Device>& Device() const;
    const winrt::com_ptr<ID3D11DeviceContext>& Context() const;

  private:
    winrt::com_ptr<ID3D11Device> _device;
    winrt::com_ptr<ID3D11DeviceContext> _context;
  };
}