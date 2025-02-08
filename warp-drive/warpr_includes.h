#pragma once
#define NOMINMAX
#include <shlwapi.h>

#include "Include/Axodox.Infrastructure.h"
#include "Include/Axodox.Json.h"
#include "Include/Axodox.Threading.h"
#include "Include/Axodox.Storage.h"

#include "rtc/rtc.hpp"
#include "dxgi1_6.h"

#undef SendMessage
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")