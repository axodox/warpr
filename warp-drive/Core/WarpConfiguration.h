#pragma once
#include "warpr_includes.h"
#include "Capture/FrameSource.h"

namespace Warpr
{
	struct WARP_DRIVE_API WarpConfiguration
	{
		Axodox::Infrastructure::uuid SessionId{"a5d26e70-2b61-451f-9ecb-7d1b9a76ed1b"};
		Axodox::Infrastructure::log_severity NetworkingLogSeverity = Axodox::Infrastructure::log_severity::warning;
		std::string GatewayUri = "ws://127.0.0.1:5164/api/sources/connect";
		std::vector<std::string> IceServers = { 
			"stun:stun.l.google.com:19302",
			"stun:stun.relay.metered.ca:80",
			"turn:cb51ef701a61be3cdc89466f:BBnGCvnTLaxhVbuf@global.relay.metered.ca:80",
			"turn:cb51ef701a61be3cdc89466f:BBnGCvnTLaxhVbuf@global.relay.metered.ca:443",
			"turn:cb51ef701a61be3cdc89466f:BBnGCvnTLaxhVbuf@global.relay.metered.ca:80?transport=tcp",
			"turn:cb51ef701a61be3cdc89466f:BBnGCvnTLaxhVbuf@global.relay.metered.ca:443?transport=tcp"
		};

		winrt::com_ptr<ID3D11Device> Device;
		std::shared_ptr<Capture::FrameSourceDescription> FrameSource;

		std::chrono::milliseconds MinimumFrameInterval = std::chrono::milliseconds{ 12 };
	};
}