#include "warpr_includes.h"
#include "VideoPreprocessor.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/WarpConfiguration.h"

using namespace Axodox::Infrastructure;
using namespace Warpr::Graphics;
using namespace std;
using namespace winrt;

namespace Warpr::Encoder
{
  VideoPreprocessor::VideoPreprocessor(Axodox::Infrastructure::dependency_container* container)
  {
    _logger.log(log_severity::information, "Initializing...");

    auto configuration = container->resolve<WarpConfiguration>();
    _resolutionScale = configuration->ResolutionScale;

    auto device = container->resolve<GraphicsDevice>();
    _device = device->Device();
    _videoDevice = _device.as<ID3D11VideoDevice>();
    _context = device->Context().as<ID3D11VideoContext>();

    _logger.log(log_severity::information, "Initialized.");
  }

  void VideoPreprocessor::ProcessFrame(Capture::Frame& frame)
  {
    //Create targets
    auto targetProperties = GetTargetProperties(frame);
    EnsureTargets(targetProperties);

    //Create source view
    auto& inputView = _inputViews[frame.get()];
    if (!inputView)
    {
      D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC description{
        .FourCC = 0,
        .ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D,
        .Texture2D = {
          .MipSlice = 0, 
          .ArraySlice = 0 
        }
      };

      check_hresult(_videoDevice->CreateVideoProcessorInputView(frame.get(), _processorEnumerator.get(), &description, inputView.put()));
    }

    //Process frame
    D3D11_VIDEO_PROCESSOR_STREAM stream{
      .Enable = true,
      .OutputIndex = 0,
      .InputFrameOrField = 0,
      .PastFrames = 0,
      .FutureFrames = 0,
      .ppPastSurfaces = nullptr,
      .pInputSurface = inputView.get(),
      .ppFutureSurfaces = nullptr,
      .ppPastSurfacesRight = nullptr,
      .pInputSurfaceRight = nullptr,
      .ppFutureSurfacesRight = nullptr
    };

    //Run RGB => NV12 conversion
    check_hresult(_context->VideoProcessorBlt(_processor.get(), _outputView.get(), 0, 1, &stream));

    frame = _outputTexture;
  }

  VideoPreprocessor::TargetProperties VideoPreprocessor::GetTargetProperties(Capture::Frame& frame)
  {
    D3D11_TEXTURE2D_DESC description;
    frame->GetDesc(&description);

    return TargetProperties{ description.Width, description.Height };
  }

  void VideoPreprocessor::EnsureTargets(const TargetProperties& targetProperties)
  {
    if (_targetProperties == targetProperties) return;
    _targetProperties = targetProperties;

    _logger.log(log_severity::information, "Allocating resources...");

    //Calculate output resolution
    auto width = max(256u, MakeEven(uint32_t(targetProperties.Width * _resolutionScale)));
    auto height = max(256u, MakeEven(uint32_t(targetProperties.Height * _resolutionScale)));

    //Create video processor enumerator
    {
      D3D11_VIDEO_PROCESSOR_CONTENT_DESC description{
        .InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE,
        .InputFrameRate = {
          .Numerator = 1,
          .Denominator = 1
        },
        .InputWidth = targetProperties.Width,
        .InputHeight = targetProperties.Height,
        .OutputFrameRate = {
          .Numerator = 1,
          .Denominator = 1
        },
        .OutputWidth = width,
        .OutputHeight = height,
        .Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL
      };

      check_hresult(_videoDevice->CreateVideoProcessorEnumerator(&description, _processorEnumerator.put()));
    }

    //Create video processor
    check_hresult(_videoDevice->CreateVideoProcessor(_processorEnumerator.get(), 0, _processor.put()));

    //Create backing texture
    {
      D3D11_TEXTURE2D_DESC description{
        .Width = width,
        .Height = height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = DXGI_FORMAT_NV12,
        .SampleDesc = {
          .Count = 1,
          .Quality = 0 
        },
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
        .CPUAccessFlags = 0,
        .MiscFlags = 0
      };

      check_hresult(_device->CreateTexture2D(&description, nullptr, _outputTexture.put()));
    }

    //Create video processor input view
    {
      D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC description{
        .ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D,
        .Texture2D = {
          .MipSlice = 0 
        }
      };

      check_hresult(_videoDevice->CreateVideoProcessorOutputView(_outputTexture.get(), _processorEnumerator.get(), &description, _outputView.put()));
    }

    _logger.log(log_severity::information, "Resources allocated.");
  }

  uint32_t VideoPreprocessor::MakeEven(uint32_t value)
  {
    return value % 2 == 0 ? value : value + 1;
  }
}