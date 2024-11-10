#pragma once
#include "warpr_includes.h"

namespace Warpr
{
	struct WarpConfiguration
	{
		Axodox::Infrastructure::log_severity NetworkingLogSeverity = Axodox::Infrastructure::log_severity::information;
		std::string GatewayUri = "wss://127.0.0.1:7074/api/sources/connect";
	};
}