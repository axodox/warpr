#include "warpr_includes.h"
#include "VideoEncoder.h"
#include "Graphics/GraphicsDevice.h"

using namespace Axodox::Infrastructure;
using namespace DirectX;
using namespace Warpr::Graphics;
using namespace std;
using namespace winrt;

namespace
{
  void check_nvenc(NVENCSTATUS status, const NV_ENCODE_API_FUNCTION_LIST* nvenc = nullptr, void* encoder = nullptr)
  {
    if (status != NV_ENC_SUCCESS)
    {
      auto details = nvenc && encoder && nvenc->nvEncGetLastErrorString ? nvenc->nvEncGetLastErrorString(encoder) : nullptr;
      throw runtime_error(format("NVEnc call failed with status {}: {}", int(status), details && *details ? details : "no details."));
    }
  }

  GUID get_codec_guid(Warpr::Encoder::VideoCodec codec)
  {
    switch (codec)
    {
    case Warpr::Encoder::VideoCodec::H264:
      return NV_ENC_CODEC_H264_GUID;
    case Warpr::Encoder::VideoCodec::H265:
      return NV_ENC_CODEC_HEVC_GUID;
    default:
      throw logic_error("Unsupported video codec.");
    }
  }
}

namespace Warpr::Encoder
{
  VideoEncoder::VideoEncoder(Axodox::Infrastructure::dependency_container* container)
  {
    _logger.log(log_severity::information, "Initializing NVEnc API...");

    //Check API support
    {
      uint32_t version = 0;
      uint32_t currentVersion = (NVENCAPI_MAJOR_VERSION << 4) | NVENCAPI_MINOR_VERSION;
      check_nvenc(NvEncodeAPIGetMaxSupportedVersion(&version));
      if (currentVersion > version)
      {
        _logger.log(log_severity::error, "Current Driver Version does not support this NvEncodeAPI version, please upgrade driver.");
        throw runtime_error("Outdated driver encountered.");
      }
    }

    //Initialize API
    {
      _nvenc = { NV_ENCODE_API_FUNCTION_LIST_VER };
      check_nvenc(NvEncodeAPICreateInstance(&_nvenc));
      if (!_nvenc.nvEncOpenEncodeSession)
      {
        _logger.log(log_severity::error, "EncodeAPI is not supported on this machine.");
        throw runtime_error("EncodeAPI is not supported.");
      }
    }

    _logger.log(log_severity::information, "NVEnc API ready.");

    _device = container->resolve<GraphicsDevice>()->Device();

    QuerySupportedCodecs();
  }

  void VideoEncoder::QuerySupportedCodecs()
  {
    _logger.log(log_severity::information, "Querying supported codecs...");

    //Open a temporary session, codec support can only be queried from one
    void* encoder = nullptr;
    NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS encodeSessionExParams = {
      .version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER,
      .deviceType = NV_ENC_DEVICE_TYPE_DIRECTX,
      .device = _device.get(),
      .apiVersion = NVENCAPI_VERSION
    };
    check_nvenc(_nvenc.nvEncOpenEncodeSessionEx(&encodeSessionExParams, &encoder));

    try
    {
      uint32_t guidCount = 0;
      check_nvenc(_nvenc.nvEncGetEncodeGUIDCount(encoder, &guidCount));

      vector<GUID> guids(guidCount);
      check_nvenc(_nvenc.nvEncGetEncodeGUIDs(encoder, guids.data(), guidCount, &guidCount));
      guids.resize(guidCount);

      for (auto codec : { VideoCodec::H264, VideoCodec::H265 })
      {
        if (ranges::find(guids, get_codec_guid(codec)) == guids.end()) continue;

        _supportedCodecs.emplace(codec);
        _logger.log(log_severity::information, "{} encoding is supported.", codec);
      }

      _nvenc.nvEncDestroyEncoder(encoder);
    }
    catch (...)
    {
      _nvenc.nvEncDestroyEncoder(encoder);
      throw;
    }

    if (_supportedCodecs.empty())
    {
      _logger.log(log_severity::error, "No supported video codec found.");
      throw runtime_error("No supported video codec found.");
    }
  }

  const std::vector<VideoCodec>& VideoEncoder::CodecPreference() const
  {
    return _codecPreference;
  }

  void VideoEncoder::CodecPreference(const std::vector<VideoCodec>& value)
  {
    _codecPreference = value;
  }

  VideoCodec VideoEncoder::SelectCodec() const
  {
    for (auto codec : _codecPreference)
    {
      if (_supportedCodecs.contains(codec)) return codec;
    }

    //No overlap with the client, fall back to the best codec the GPU supports
    return *_supportedCodecs.rbegin();
  }

  VideoEncoder::~VideoEncoder()
  {
    ReleaseResources();
  }

  uint32_t VideoEncoder::Bitrate() const
  {
    return _bitrate;
  }

  void VideoEncoder::Bitrate(uint32_t value)
  {
    _bitrate = value;
  }

  EncodedFrame VideoEncoder::EncodeFrame(const Capture::Frame& frame, bool forceIdrFrame)
  {
    Stopwatch watch{ "Frame encode" };

    //Ensure session
    auto encoderProperties = GetEncoderProperties(frame);
    EnsureEncoder(encoderProperties);

    //Register input
    RegisterFrame(frame);

    //Map input
    void* mappedResource;
    NV_ENC_BUFFER_FORMAT mappedFormat;

    {
      NV_ENC_MAP_INPUT_RESOURCE description{
        .version = NV_ENC_MAP_INPUT_RESOURCE_VER,
        .subResourceIndex = 0,
        .inputResource = 0,
        .registeredResource = _inputResource,
        .mappedResource = nullptr,
        .mappedBufferFmt = NV_ENC_BUFFER_FORMAT_UNDEFINED,
        .reserved1 = {},
        .reserved2 = {}
      };
      check_nvenc(_nvenc.nvEncMapInputResource(_encoder, &description), &_nvenc, _encoder);
      mappedResource = description.mappedResource;
      mappedFormat = description.mappedBufferFmt;
    }

    //Encode frame
    {
      NV_ENC_PIC_PARAMS description{
        .version = NV_ENC_PIC_PARAMS_VER,
        .inputWidth = _encoderProperties.Width,
        .inputHeight = _encoderProperties.Height,
        .inputPitch = 0,
        .encodePicFlags = forceIdrFrame ? NV_ENC_PIC_FLAG_FORCEIDR : 0u,
        .frameIdx = _frameIndex++,
        .inputTimeStamp = 0,
        .inputDuration = 0,
        .inputBuffer = mappedResource,
        .outputBitstream = _outputBuffer,
        .completionEvent = nullptr, //for later
        .bufferFmt = mappedFormat,
        .pictureStruct = NV_ENC_PIC_STRUCT_FRAME,
        .pictureType = NV_ENC_PIC_TYPE_P
      };
      check_nvenc(_nvenc.nvEncEncodePicture(_encoder, &description), &_nvenc, _encoder);
    }

    //Unmap input
    {
      check_nvenc(_nvenc.nvEncUnmapInputResource(_encoder, mappedResource), &_nvenc, _encoder);
    }

    //Get output
    EncodedFrame result;
    {
      NV_ENC_LOCK_BITSTREAM description{
        .version = NV_ENC_LOCK_BITSTREAM_VER,
        .doNotWait = false,
        .getRCStats = true, //DIAGNOSTIC
        .outputBitstream = _outputBuffer
      };
      check_nvenc(_nvenc.nvEncLockBitstream(_encoder, &description), &_nvenc, _encoder);

      if (description.bitstreamBufferPtr)
      {
        result.Bytes.resize(description.bitstreamSizeInBytes);
        memcpy(result.Bytes.data(), description.bitstreamBufferPtr, result.Bytes.size());
      }

      //DIAGNOSTIC: intra refresh activity
      {
        _diagFrames++;
        _diagIntraBlocks += description.intraMBCount;
        _diagTotalBlocks += description.intraMBCount + description.interMBCount;
        if (description.pictureType == NV_ENC_PIC_TYPE_INTRA_REFRESH) _diagRefreshStarts++;
        if (description.pictureType == NV_ENC_PIC_TYPE_IDR) _diagKeyFrames++;

        auto now = chrono::steady_clock::now();
        if (now - _diagLastTime > 1s)
        {
          _logger.log(log_severity::information, "DIAG {}: {} frames, intra blocks {:.1f}% ({} / {}), refresh starts {}, key frames {}",
            _encoderProperties.Codec, _diagFrames, _diagTotalBlocks ? 100.0 * _diagIntraBlocks / _diagTotalBlocks : 0.0, _diagIntraBlocks, _diagTotalBlocks, _diagRefreshStarts, _diagKeyFrames);
          _diagLastTime = now;
          _diagFrames = _diagIntraBlocks = _diagTotalBlocks = _diagRefreshStarts = _diagKeyFrames = 0;
        }
      }

      result.Index = description.frameIdx;
      result.Type = description.pictureType == NV_ENC_PIC_TYPE_IDR ? FrameType::Key : FrameType::Delta;
      result.Width = _encoderProperties.Width;
      result.Height = _encoderProperties.Height;
      result.Codec = _encoderProperties.Codec;

      check_nvenc(_nvenc.nvEncUnlockBitstream(_encoder, _outputBuffer), &_nvenc, _encoder);
    }

    //Return result
    return result;
  }

  VideoEncoder::EncoderProperties VideoEncoder::GetEncoderProperties(const Capture::Frame& frame)
  {
    D3D11_TEXTURE2D_DESC description;
    frame->GetDesc(&description);

    return {
      .Width = description.Width,
      .Height = description.Height,
      .Bitrate = _bitrate,
      .Codec = SelectCodec()
    };
  }

  void VideoEncoder::EnsureEncoder(const EncoderProperties& encoderProperties)
  {
    if (_encoderProperties == encoderProperties) return;
    _encoderProperties = encoderProperties;

    _logger.log(log_severity::information, "Encoder property change detected.");

    //Release resources
    ReleaseResources();

    //Open encode session
    {
      _logger.log(log_severity::information, "Opening encode session...");
      NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS encodeSessionExParams = {
        .version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER,
        .deviceType = NV_ENC_DEVICE_TYPE_DIRECTX,
        .device = _device.get(),
        .apiVersion = NVENCAPI_VERSION
      };
      check_nvenc(_nvenc.nvEncOpenEncodeSessionEx(&encodeSessionExParams, &_encoder));

      _logger.log(log_severity::information, "Encode session ready.");
    }

    //Initialize encoder
    {
      _logger.log(log_severity::information, "Initializing {} encoder...", encoderProperties.Codec);
      auto codecGuid = get_codec_guid(encoderProperties.Codec);

      NV_ENC_PRESET_CONFIG presetConfig = { NV_ENC_PRESET_CONFIG_VER, 0, { NV_ENC_CONFIG_VER } };
      check_nvenc(_nvenc.nvEncGetEncodePresetConfigEx(_encoder, codecGuid, NV_ENC_PRESET_P1_GUID, NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY, &presetConfig), &_nvenc, _encoder);

      NV_ENC_CONFIG encodeConfig = presetConfig.presetCfg;

      encodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
      encodeConfig.rcParams.zeroReorderDelay = 1;
      encodeConfig.rcParams.averageBitRate = encoderProperties.Bitrate;
      //encodeConfig.rcParams.vbvBufferSize = 0;
      //encodeConfig.rcParams.vbvInitialDelay = 0;
      //encodeConfig.rcParams.maxBitRate = 2500;
      encodeConfig.gopLength = NVENC_INFINITE_GOPLENGTH; //Required for intra refresh, key frames are only sent when requested
      encodeConfig.rcParams.multiPass = NV_ENC_MULTI_PASS_DISABLED;

      switch (encoderProperties.Codec)
      {
      case VideoCodec::H264:
        encodeConfig.profileGUID = NV_ENC_H264_PROFILE_HIGH_GUID;
        encodeConfig.encodeCodecConfig.h264Config.enableIntraRefresh = 1;
        encodeConfig.encodeCodecConfig.h264Config.intraRefreshPeriod = IntraRefreshPeriod;
        encodeConfig.encodeCodecConfig.h264Config.intraRefreshCnt = IntraRefreshCount;
        encodeConfig.encodeCodecConfig.h264Config.chromaFormatIDC = 1;
        encodeConfig.encodeCodecConfig.h264Config.idrPeriod = encodeConfig.gopLength;
        encodeConfig.encodeCodecConfig.h264Config.sliceMode = 2; //NVEnc does not support byte based slices for H.264, so use one slice per macroblock row
        encodeConfig.encodeCodecConfig.h264Config.sliceModeData = 1;
        encodeConfig.encodeCodecConfig.h264Config.repeatSPSPPS = 1;
        encodeConfig.encodeCodecConfig.h264Config.h264VUIParameters.bitstreamRestrictionFlag = 1; //Signals no frame reordering, so decoders output frames without buffering
        break;
      case VideoCodec::H265:
        encodeConfig.encodeCodecConfig.hevcConfig.enableIntraRefresh = 1;
        encodeConfig.encodeCodecConfig.hevcConfig.intraRefreshPeriod = IntraRefreshPeriod;
        encodeConfig.encodeCodecConfig.hevcConfig.intraRefreshCnt = IntraRefreshCount;
        encodeConfig.encodeCodecConfig.hevcConfig.inputBitDepth = NV_ENC_BIT_DEPTH_8;
        encodeConfig.encodeCodecConfig.hevcConfig.outputBitDepth = NV_ENC_BIT_DEPTH_8;
        encodeConfig.encodeCodecConfig.hevcConfig.chromaFormatIDC = 1;
        encodeConfig.encodeCodecConfig.hevcConfig.idrPeriod = encodeConfig.gopLength;
        encodeConfig.encodeCodecConfig.hevcConfig.sliceMode = 1;
        encodeConfig.encodeCodecConfig.hevcConfig.sliceModeData = MaxSliceSize;
        encodeConfig.encodeCodecConfig.hevcConfig.repeatSPSPPS = 1;
        break;
      }
      encodeConfig.frameIntervalP = 1;

      NV_ENC_INITIALIZE_PARAMS description{
        .version = NV_ENC_INITIALIZE_PARAMS_VER,
        .encodeGUID = codecGuid,
        .presetGUID = NV_ENC_PRESET_P1_GUID,
        .encodeWidth = _encoderProperties.Width,
        .encodeHeight = _encoderProperties.Height,
        .darWidth = _encoderProperties.Width,
        .darHeight = _encoderProperties.Height,
        .frameRateNum = 60,
        .frameRateDen = 1,
        .enableEncodeAsync = 0,
        .enablePTD = 1,
        .reportSliceOffsets = 0,
        .enableSubFrameWrite = 0,
        .enableExternalMEHints = 0, //Maybe we could use motion vectors from rendering later
        .enableMEOnlyMode = 0,
        .enableWeightedPrediction = 0,
        .splitEncodeMode = 0,
        .enableOutputInVidmem = 0,
        .enableReconFrameOutput = 0,
        .enableOutputStats = 0,
        .enableUniDirectionalB = 0,
        .reservedBitFields = 0,
        .privDataSize = 0,
        .reserved = 0,
        .privData = nullptr,
        .encodeConfig = &encodeConfig,
        .maxEncodeWidth = _encoderProperties.Width,
        .maxEncodeHeight = _encoderProperties.Height,
        .maxMEHintCountsPerBlock = { 0, 0 },
        .tuningInfo = NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY,
        .bufferFormat = NV_ENC_BUFFER_FORMAT_UNDEFINED,
        .numStateBuffers = 0,
        .outputStatsLevel = NV_ENC_OUTPUT_STATS_NONE,
        .reserved1 = {},
        .reserved2 = {}
      };

      check_nvenc(_nvenc.nvEncInitializeEncoder(_encoder, &description), &_nvenc, _encoder);
      _logger.log(log_severity::information, "Encoder initialized.");
    }

    //Create output buffer
    {
      _logger.log(log_severity::information, "Allocating output buffer...");
      NV_ENC_CREATE_BITSTREAM_BUFFER description = { NV_ENC_CREATE_BITSTREAM_BUFFER_VER };
      check_nvenc(_nvenc.nvEncCreateBitstreamBuffer(_encoder, &description), &_nvenc, _encoder);
      _outputBuffer = description.bitstreamBuffer;
      _logger.log(log_severity::information, "Output buffer allocated.");
    }

    //Reset data
    {
      _frameIndex = 0u;
    }
  }

  void VideoEncoder::RegisterFrame(const Capture::Frame& frame)
  {
    if (frame.get() == _frameResource) return;
    _frameResource = frame.get();

    if (_inputResource) check_nvenc(_nvenc.nvEncUnregisterResource(_encoder, _inputResource), &_nvenc, _encoder);

    _logger.log(log_severity::information, "Registering input resource...");
    NV_ENC_REGISTER_RESOURCE description{
      .version = NV_ENC_REGISTER_RESOURCE_VER,
      .resourceType = NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX,
      .width = _encoderProperties.Width,
      .height = _encoderProperties.Height,
      .pitch = 0,
      .subResourceIndex = 0,
      .resourceToRegister = frame.get(),
      .registeredResource = nullptr,
      .bufferFormat = NV_ENC_BUFFER_FORMAT_NV12,
      .bufferUsage = NV_ENC_INPUT_IMAGE,
      .pInputFencePoint = nullptr,
      .chromaOffset = { 0, 0 },
      .reserved1 = {},
      .reserved2 = {}
    };

    check_nvenc(_nvenc.nvEncRegisterResource(_encoder, &description), &_nvenc, _encoder);
    _inputResource = description.registeredResource;
    _logger.log(log_severity::information, "Input resource registered.");
  }

  void VideoEncoder::ReleaseResources()
  {
    _logger.log(log_severity::information, "Releasing resources...");

    if (_inputResource)
    {
      _nvenc.nvEncUnregisterResource(_encoder, _inputResource);
      _inputResource = nullptr;
    }

    //The frame must be registered again with the new encoder, even if the texture is the same
    _frameResource = nullptr;

    if (_outputBuffer)
    {
      _nvenc.nvEncDestroyBitstreamBuffer(_encoder, _outputBuffer);
      _outputBuffer = nullptr;
    }

    if (_encoder)
    {
      _nvenc.nvEncDestroyEncoder(_encoder);
      _encoder = nullptr;
    }

    _logger.log(log_severity::information, "Resources released.");
  }
}