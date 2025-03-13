#pragma once
#define NOMINMAX

//Axodox.Common
#include "Include/Axodox.Infrastructure.h"
#include "Include/Axodox.Json.h"
#include "Include/Axodox.Threading.h"
#include "Include/Axodox.Storage.h"

//libdatachannel
#include "rtc/rtc.hpp"

//NVEnc
#include <nvEncodeAPI.h>

//WinAPI
#include <d3d11.h>
#include <winrt/base.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.graphics.h>
#include <winrt/windows.graphics.capture.h>
#include <winrt/windows.graphics.display.h>
#include <winrt/windows.graphics.directx.h>
#include <winrt/windows.graphics.directx.direct3d11.h>
#include <winrt/windows.ui.input.preview.injection.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <windows.graphics.display.interop.h>

#undef SendMessage
#pragma comment(lib, "d3d11.lib")

#ifdef WARP_DRIVE_EXPORT
#define WARP_DRIVE_API __declspec(dllexport)
#else
#define WARP_DRIVE_API __declspec(dllimport)
#pragma comment (lib,"WarpDrive.lib")
#endif