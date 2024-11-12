#include "warpr_includes.h"
#include "WindowSource.h"

using namespace std;
using namespace winrt;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI;
using namespace ::Windows::Graphics::DirectX::Direct3D11;

namespace Warpr::Capture
{
  WindowSource::WindowSource(const WindowSourceDescription& description)
  {
    //Create device
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
      nullptr
    ));

    //Get WinRT device
    IDirect3DDevice winrtDevice;
    check_hresult(CreateDirect3D11DeviceFromDXGIDevice(
      _device.as<IDXGIDevice>().get(),
      reinterpret_cast<IInspectable**>(put_abi(winrtDevice)))
    );

    //Create display capture
    if (holds_alternative<DisplayId>(description.Source))
    {
      _captureItem = GraphicsCaptureItem::TryCreateFromDisplayId(get<DisplayId>(description.Source));
    }
    else if (holds_alternative<WindowId>(description.Source))
    {
      _captureItem = GraphicsCaptureItem::TryCreateFromWindowId(get<WindowId>(description.Source));
    }
    else
    {
      throw logic_error("Unsupported window capture source encountered.");
    }

    //Create frame pool
    _framePool = Direct3D11CaptureFramePool::CreateFreeThreaded(
      winrtDevice,
      DirectXPixelFormat::R8G8B8A8UIntNormalized,
      2,
      _captureItem.Size());

    //Attach frame arrived event handler
    _framePool.FrameArrived([=](auto&&...) {
      while (true)
      {
        auto frame = _framePool.TryGetNextFrame();
        if (!frame) return;

        com_ptr<ID3D11Texture2D> texture;
        auto surfaceAccess = frame.Surface().as<IDirect3DDxgiInterfaceAccess>();
        check_hresult(surfaceAccess->GetInterface(IID_PPV_ARGS(texture.put())));

        PushFrame(texture);
      }
      });

    //Start capture session
    auto captureSession = _framePool.CreateCaptureSession(_captureItem);
    captureSession.IsBorderRequired(false);
    captureSession.IsCursorCaptureEnabled(false);
    captureSession.StartCapture();
  }

  FrameSourceKind WindowSourceDescription::Type() const
  {
    return FrameSourceKind::Window;
  }
}