#pragma once
#include "warpr_includes.h"
#include "Capture/FrameSource.h"
#include "nvEncodeAPI.h"

namespace Warpr::Encoder
{
  enum class FrameType
  {
    Unknown,
    Key,
    Delta
  };

  struct EncodedFrame
  {
    uint32_t Index = 0;
    FrameType Type = FrameType::Unknown;
    std::vector<uint8_t> Bytes;
    std::vector<uint32_t> Slices;
  };

  class VideoEncoder
  {
    inline static const Axodox::Infrastructure::logger _logger{ "VideoEncoder" };

    struct EncoderProperties
    {
      uint32_t Width = 0, Height = 0;
      bool operator==(const EncoderProperties&) const = default;
    };

  public:
    VideoEncoder(Axodox::Infrastructure::dependency_container* container);
    ~VideoEncoder();

    EncodedFrame EncodeFrame(const Capture::Frame& frame);

  private:
    NV_ENCODE_API_FUNCTION_LIST _nvenc;
    void* _encoder = nullptr;
    void* _frameResource = nullptr;
    void* _inputResource = nullptr;
    void* _outputBuffer = nullptr;
    uint32_t _frameIndex = 0u;
    EncoderProperties _encoderProperties{};

    EncoderProperties GetEncoderProperties(const Capture::Frame& frame);
    void EnsureEncoder(const EncoderProperties& properties);

    void RegisterFrame(const Capture::Frame& frame);
    void ReleaseResources();
  };
}