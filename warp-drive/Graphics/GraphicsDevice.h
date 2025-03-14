#pragma once
#include "warpr_includes.h"

namespace Warpr::Graphics
{
  class GraphicsDevice
  {
    inline static const Axodox::Infrastructure::logger _logger{ "GraphicsDevice" };

  public:
    GraphicsDevice(Axodox::Infrastructure::dependency_container* container);

    const winrt::com_ptr<ID3D11Device>& Device() const;
    const winrt::com_ptr<ID3D11DeviceContext>& Context() const;

  private:
    winrt::com_ptr<ID3D11Device> _device;
    winrt::com_ptr<ID3D11DeviceContext> _context;
  };
}