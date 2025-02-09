#include "warpr_includes.h"
#include "WindowSource.h"

using namespace Axodox::Infrastructure;
using namespace Warpr::Graphics;
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
  WindowSource::WindowSource(Axodox::Infrastructure::dependency_container* container, const WindowSourceDescription* description) :
    _device(container->resolve<GraphicsDevice>()->Device())
  {
    _logger.log(log_severity::information, "Initializing window capture frame source...");

    //Get WinRT device
    IDirect3DDevice winrtDevice;
    check_hresult(CreateDirect3D11DeviceFromDXGIDevice(
      _device.as<IDXGIDevice>().get(),
      reinterpret_cast<IInspectable**>(put_abi(winrtDevice)))
    );

    //Create display capture
    if (holds_alternative<DisplayId>(description->Source))
    {
      auto displayId = get<DisplayId>(description->Source);
      _logger.log(log_severity::information, "Source is display {}.", displayId.Value);
      _captureItem = GraphicsCaptureItem::TryCreateFromDisplayId(displayId);
    }
    else if (holds_alternative<WindowId>(description->Source))
    {
      auto windowId = get<WindowId>(description->Source);
      _logger.log(log_severity::information, "Source is window {}.", windowId.Value);
      _captureItem = GraphicsCaptureItem::TryCreateFromWindowId(windowId);
    }
    else
    {
      throw logic_error("Unsupported window capture source encountered.");
    }

    //Create frame pool
    _logger.log(log_severity::information, "Creating frame pool...");
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

        frame.Close();
      }
    });

    //Start capture session
    _logger.log(log_severity::information, "Starting capture session...");
    _session = _framePool.CreateCaptureSession(_captureItem);
    _session.IsBorderRequired(false);
    _session.IsCursorCaptureEnabled(true);
    _session.StartCapture();
    _logger.log(log_severity::information, "Capture started.");
  }

  FrameSourceKind WindowSourceDescription::Type() const
  {
    return FrameSourceKind::Window;
  }

  WindowSourceDescription::WindowSourceDescription(std::variant<winrt::Windows::Graphics::DisplayId, winrt::Windows::UI::WindowId> source) :
    Source(source)
  { }
}