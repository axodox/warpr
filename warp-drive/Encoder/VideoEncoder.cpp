#include "warpr_includes.h"
#include "VideoEncoder.h"
#include "Graphics/GraphicsDevice.h"
#include "d3d11.h"

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
    
    //Open encode session
    {
      _logger.log(log_severity::information, "Opening encode session...");
      auto device = container->resolve<GraphicsDevice>()->Device();

      NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS encodeSessionExParams = { 
        .version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER,
        .deviceType = NV_ENC_DEVICE_TYPE_DIRECTX,
        .device = device.get(),
        .apiVersion = NVENCAPI_VERSION
      };
      check_nvenc(_nvenc.nvEncOpenEncodeSessionEx(&encodeSessionExParams, &_encoder));

      _logger.log(log_severity::information, "Encode session ready.");
    }
  }

  VideoEncoder::~VideoEncoder()
  {
    check_nvenc(_nvenc.nvEncDestroyEncoder(_encoder));
  }

  void VideoEncoder::PushFrame(const Capture::Frame& frame)
  {
    //Ensure session
    auto encoderProperties = GetEncoderProperties(frame);
    EnsureEncoder(encoderProperties);


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

    _logger.log(log_severity::information, "Initializing encoder...");

    NV_ENC_PRESET_CONFIG presetConfig = { NV_ENC_PRESET_CONFIG_VER, 0, { NV_ENC_CONFIG_VER } };
    check_nvenc(_nvenc.nvEncGetEncodePresetConfigEx(_encoder, NV_ENC_CODEC_HEVC_GUID, NV_ENC_PRESET_P4_GUID, NV_ENC_TUNING_INFO_LOW_LATENCY, &presetConfig));

    NV_ENC_CONFIG encodeConfig = presetConfig.presetCfg;

    encodeConfig.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CONSTQP;
    encodeConfig.rcParams.averageBitRate = 2500;
    //encodeConfig.rcParams.vbvBufferSize = 0;
    //encodeConfig.rcParams.vbvInitialDelay = 0;
    encodeConfig.rcParams.maxBitRate = 2500;

    encodeConfig.encodeCodecConfig.hevcConfig.inputBitDepth = NV_ENC_BIT_DEPTH_8;
    encodeConfig.encodeCodecConfig.hevcConfig.outputBitDepth = NV_ENC_BIT_DEPTH_8;
    encodeConfig.encodeCodecConfig.hevcConfig.chromaFormatIDC = 3;
    encodeConfig.encodeCodecConfig.hevcConfig.idrPeriod = encodeConfig.gopLength;
    encodeConfig.frameIntervalP = min(encodeConfig.frameIntervalP, encodeConfig.gopLength);

    NV_ENC_INITIALIZE_PARAMS initializeParams{
      .version = NV_ENC_INITIALIZE_PARAMS_VER,
      .encodeGUID = NV_ENC_CODEC_HEVC_GUID,
      .presetGUID = NV_ENC_PRESET_P4_GUID,
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
      .enableUniDirectionalB = 1,
      .reservedBitFields = 0,
      .privDataSize = 0,
      .reserved = 0,
      .privData = nullptr,
      .encodeConfig = &encodeConfig,
      .maxEncodeWidth = 0,
      .maxEncodeHeight = 0,
      .maxMEHintCountsPerBlock = { 0, 0 },
      .tuningInfo = NV_ENC_TUNING_INFO_LOW_LATENCY,
      .bufferFormat = NV_ENC_BUFFER_FORMAT_NV12,
      .numStateBuffers = 0,
      .outputStatsLevel = NV_ENC_OUTPUT_STATS_NONE,
      .reserved1 = {},
      .reserved2 = {}
    };

    check_nvenc(_nvenc.nvEncInitializeEncoder(_encoder, &initializeParams));

    _logger.log(log_severity::information, "Encoder initialized.");
  }
}