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
  void check_nvenc(NVENCSTATUS status)
  {
    if (status != NV_ENC_SUCCESS)
    {
      throw runtime_error("NVEnc call failed.");
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
        throw runtime_error("Current Driver Version does not support this NvEncodeAPI version, please upgrade driver");
      }
    }

    //Initialize API
    {
      _nvenc = { NV_ENCODE_API_FUNCTION_LIST_VER };
      check_nvenc(NvEncodeAPICreateInstance(&_nvenc));
      if (!_nvenc.nvEncOpenEncodeSession) throw runtime_error("EncodeAPI not found");
    }

    _logger.log(log_severity::information, "NVEnc API ready.");

    _device = container->resolve<GraphicsDevice>()->Device();
  }

  VideoEncoder::~VideoEncoder()
  {
    ReleaseResources();
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
      check_nvenc(_nvenc.nvEncMapInputResource(_encoder, &description));
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
      check_nvenc(_nvenc.nvEncEncodePicture(_encoder, &description));
    }

    //Unmap input
    {
      check_nvenc(_nvenc.nvEncUnmapInputResource(_encoder, mappedResource));
    }

    //Get output
    EncodedFrame result;
    {
      result.Slices.resize(2048);

      NV_ENC_LOCK_BITSTREAM description{
        .version = NV_ENC_LOCK_BITSTREAM_VER,
        .doNotWait = false,
        .outputBitstream = _outputBuffer,
        .sliceOffsets = result.Slices.data()
      };
      check_nvenc(_nvenc.nvEncLockBitstream(_encoder, &description));

      if (description.bitstreamBufferPtr)
      {
        result.Bytes.resize(description.bitstreamSizeInBytes);
        memcpy(result.Bytes.data(), description.bitstreamBufferPtr, result.Bytes.size());
      }

      result.Slices.resize(description.numSlices);
      result.Index = description.frameIdx;
      result.Type = description.pictureType == NV_ENC_PIC_TYPE_IDR ? FrameType::Key : FrameType::Delta;
      result.Width = _encoderProperties.Width;
      result.Height = _encoderProperties.Height;

      check_nvenc(_nvenc.nvEncUnlockBitstream(_encoder, _outputBuffer));
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
      .Height = description.Height
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
      _logger.log(log_severity::information, "Initializing encoder...");
      NV_ENC_PRESET_CONFIG presetConfig = { NV_ENC_PRESET_CONFIG_VER, 0, { NV_ENC_CONFIG_VER } };
      check_nvenc(_nvenc.nvEncGetEncodePresetConfigEx(_encoder, NV_ENC_CODEC_HEVC_GUID, NV_ENC_PRESET_P1_GUID, NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY, &presetConfig));

      NV_ENC_CONFIG encodeConfig = presetConfig.presetCfg;

      encodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
      encodeConfig.rcParams.zeroReorderDelay = 1;
      encodeConfig.rcParams.averageBitRate = 2 * 6400000;
      //encodeConfig.rcParams.vbvBufferSize = 0;
      //encodeConfig.rcParams.vbvInitialDelay = 0;
      //encodeConfig.rcParams.maxBitRate = 2500;
      //encodeConfig.gopLength = 120; //Key frame interval
      encodeConfig.rcParams.multiPass = NV_ENC_MULTI_PASS_DISABLED;

      encodeConfig.encodeCodecConfig.hevcConfig.enableIntraRefresh = 1;
      encodeConfig.encodeCodecConfig.hevcConfig.intraRefreshPeriod = 120;
      encodeConfig.encodeCodecConfig.hevcConfig.intraRefreshCnt = 120;
      encodeConfig.encodeCodecConfig.hevcConfig.inputBitDepth = NV_ENC_BIT_DEPTH_8;
      encodeConfig.encodeCodecConfig.hevcConfig.outputBitDepth = NV_ENC_BIT_DEPTH_8;
      encodeConfig.encodeCodecConfig.hevcConfig.chromaFormatIDC = 1;
      encodeConfig.encodeCodecConfig.hevcConfig.idrPeriod = encodeConfig.gopLength;
      encodeConfig.encodeCodecConfig.hevcConfig.sliceMode = 3;
      encodeConfig.encodeCodecConfig.hevcConfig.sliceModeData = 2048;
      encodeConfig.encodeCodecConfig.hevcConfig.repeatSPSPPS = 1;
      encodeConfig.frameIntervalP = 1;

      NV_ENC_INITIALIZE_PARAMS description{
        .version = NV_ENC_INITIALIZE_PARAMS_VER,
        .encodeGUID = NV_ENC_CODEC_HEVC_GUID,
        .presetGUID = NV_ENC_PRESET_P1_GUID,
        .encodeWidth = _encoderProperties.Width,
        .encodeHeight = _encoderProperties.Height,
        .darWidth = _encoderProperties.Width,
        .darHeight = _encoderProperties.Height,
        .frameRateNum = 60,
        .frameRateDen = 1,
        .enableEncodeAsync = 0,
        .enablePTD = 1,
        .reportSliceOffsets = 1, //Slice offsets could be useful for frame splitting
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

      check_nvenc(_nvenc.nvEncInitializeEncoder(_encoder, &description));
      _logger.log(log_severity::information, "Encoder initialized.");
    }

    //Create output buffer
    {
      _logger.log(log_severity::information, "Allocating output buffer...");
      NV_ENC_CREATE_BITSTREAM_BUFFER description = { NV_ENC_CREATE_BITSTREAM_BUFFER_VER };
      check_nvenc(_nvenc.nvEncCreateBitstreamBuffer(_encoder, &description));
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

    if (_inputResource) check_nvenc(_nvenc.nvEncUnregisterResource(_encoder, _inputResource));

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

    check_nvenc(_nvenc.nvEncRegisterResource(_encoder, &description));
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