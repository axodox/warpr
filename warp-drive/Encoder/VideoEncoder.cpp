#include "warpr_includes.h"
#include "VideoEncoder.h"
#include "d3d11.h"

using namespace Axodox::Infrastructure;
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
  VideoEncoder::VideoEncoder()
  {
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
    static NV_ENCODE_API_FUNCTION_LIST m_nvenc = { NV_ENCODE_API_FUNCTION_LIST_VER };
    
    //Initialize API
    {
      check_nvenc(NvEncodeAPICreateInstance(&m_nvenc));
      if (!m_nvenc.nvEncOpenEncodeSession) throw runtime_error("EncodeAPI not found");
    }
    
    //Create device
    com_ptr<ID3D11Device> device;
    {
      check_hresult(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        device.put(),
        nullptr,
        nullptr
      ));
    }

    //Open encode session
    void* encoder = nullptr;
    {
      NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS encodeSessionExParams = { 
        .version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER,
        .deviceType = NV_ENC_DEVICE_TYPE_DIRECTX,
        .device = device.get(),
        .apiVersion = NVENCAPI_VERSION
      };
      check_nvenc(m_nvenc.nvEncOpenEncodeSessionEx(&encodeSessionExParams, &encoder));
    }

    //Initialize session
    {
      NV_ENC_PRESET_CONFIG presetConfig = { NV_ENC_PRESET_CONFIG_VER, 0, { NV_ENC_CONFIG_VER } };
      check_nvenc(m_nvenc.nvEncGetEncodePresetConfigEx(encoder, NV_ENC_CODEC_HEVC_GUID, NV_ENC_PRESET_P4_GUID, NV_ENC_TUNING_INFO_LOW_LATENCY, &presetConfig));

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
        .encodeWidth = 1920,
        .encodeHeight = 1080,
        .darWidth = 1920,
        .darHeight = 1080,
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
        .bufferFormat = NV_ENC_BUFFER_FORMAT_UNDEFINED,
        .numStateBuffers = 0,
        .outputStatsLevel = NV_ENC_OUTPUT_STATS_NONE,
        .reserved1 = {},
        .reserved2 = {}
      };
      
      check_nvenc(m_nvenc.nvEncInitializeEncoder(encoder, &initializeParams));
    }

    //void ConvertRGBToNV12(ID3D11Texture2D*pRGBSrcTexture, ID3D11Texture2D* pDestTexture) for conversion to NV12

    __nop();

    check_nvenc(m_nvenc.nvEncDestroyEncoder(encoder));

    system("pause");
  }
}