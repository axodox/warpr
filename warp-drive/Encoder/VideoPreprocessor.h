#pragma once
#include "Capture/FrameSource.h"

namespace Warpr::Encoder
{
  class VideoPreprocessor
  {
    inline static const Axodox::Infrastructure::logger _logger{ "VideoPreprocessor" };

    struct TargetProperties
    {
      uint32_t Width = 0, Height = 0;
      bool operator==(const TargetProperties&) const = default;
    };

  public:
    VideoPreprocessor(Axodox::Infrastructure::dependency_container* container);

    void ProcessFrame(Capture::Frame& frame);

  private:
    winrt::com_ptr<ID3D11Device> _device;
    winrt::com_ptr<ID3D11VideoDevice> _videoDevice;
    winrt::com_ptr<ID3D11VideoContext> _context;

    winrt::com_ptr<ID3D11VideoProcessorEnumerator> _processorEnumerator;
    winrt::com_ptr<ID3D11VideoProcessor> _processor;

    winrt::com_ptr<ID3D11Texture2D> _outputTexture;
    winrt::com_ptr<ID3D11VideoProcessorOutputView> _outputView;

    float _resolutionScale = 1.f;

    std::map<ID3D11Texture2D*, winrt::com_ptr<ID3D11VideoProcessorInputView>> _inputViews;

    TargetProperties _targetProperties{};

    TargetProperties GetTargetProperties(Capture::Frame& frame);
    void EnsureTargets(const TargetProperties& targetProperties);

    static uint32_t MakeEven(uint32_t value);
  };
}