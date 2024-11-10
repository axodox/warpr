#pragma once
#include "warpr_includes.h"

namespace Warpr
{
	struct WarpConfiguration
	{
		Axodox::Infrastructure::uuid SessionId{"a5d26e70-2b61-451f-9ecb-7d1b9a76ed1b"};
		Axodox::Infrastructure::log_severity NetworkingLogSeverity = Axodox::Infrastructure::log_severity::warning;
		std::string GatewayUri = "wss://127.0.0.1:7074/api/sources/connect";
		std::vector<std::string> IceServers = { "stun:stun.l.google.com:19302" };
	};
}