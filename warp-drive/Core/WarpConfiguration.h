#pragma once
#include "warpr_includes.h"
#include "Capture/FrameSource.h"
#include "Input/InputSink.h"

namespace Warpr
{
	struct WARP_DRIVE_API WarpConfiguration
	{
		Axodox::Infrastructure::uuid SessionId;
		Axodox::Infrastructure::log_severity NetworkingLogSeverity = Axodox::Infrastructure::log_severity::warning;
		std::string GatewayUri = "ws://127.0.0.1:5164/api/sources/connect";

		winrt::com_ptr<ID3D11Device> Device;
		std::shared_ptr<Capture::FrameSourceDescription> FrameSource;
		std::shared_ptr<Input::InputSinkDescription> InputSink;

		std::chrono::milliseconds MinimumFrameInterval = std::chrono::milliseconds{ 0 };
	};
}