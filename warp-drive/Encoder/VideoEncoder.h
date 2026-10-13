#pragma once
#include "warpr_includes.h"
#include "Capture/FrameSource.h"

namespace Warpr::Encoder
{
  enum class FrameType : uint32_t
  {
    Unknown,
    Key,
    Delta
  };

  named_enum(VideoCodec,
    Unknown,
    H264,
    H265
  );

  struct EncodedFrame
  {
    uint32_t Index = 0;
    FrameType Type = FrameType::Unknown;
    uint32_t Width = 0;
    uint32_t Height = 0;
    VideoCodec Codec = VideoCodec::Unknown;
    std::vector<uint8_t> Bytes;
    std::vector<uint32_t> Slices;
  };

  class VideoEncoder
  {
    inline static const Axodox::Infrastructure::logger _logger{ "VideoEncoder" };

    struct EncoderProperties
    {
      uint32_t Width = 0, Height = 0;
      uint32_t Bitrate = 0;
      VideoCodec Codec = VideoCodec::Unknown;
      bool operator==(const EncoderProperties&) const = default;
    };

  public:
    VideoEncoder(Axodox::Infrastructure::dependency_container* container);
    ~VideoEncoder();

    uint32_t Bitrate() const;
    void Bitrate(uint32_t value);

    const std::vector<VideoCodec>& CodecPreference() const;
    void CodecPreference(const std::vector<VideoCodec>& value);

    EncodedFrame EncodeFrame(const Capture::Frame& frame, bool forceIdrFrame = false);

  private:
    winrt::com_ptr<ID3D11Device> _device;

    NV_ENCODE_API_FUNCTION_LIST _nvenc;
    void* _encoder = nullptr;
    void* _frameResource = nullptr;
    void* _inputResource = nullptr;
    void* _outputBuffer = nullptr;
    uint32_t _frameIndex = 0u;
    EncoderProperties _encoderProperties{};

    uint32_t _bitrate = 12'800'000;
    std::vector<VideoCodec> _codecPreference{ VideoCodec::H265, VideoCodec::H264 };
    std::set<VideoCodec> _supportedCodecs;

    void QuerySupportedCodecs();
    VideoCodec SelectCodec() const;

    EncoderProperties GetEncoderProperties(const Capture::Frame& frame);
    void EnsureEncoder(const EncoderProperties& properties);

    void RegisterFrame(const Capture::Frame& frame);
    void ReleaseResources();
  };
}