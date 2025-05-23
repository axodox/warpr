#pragma once
#include "Json/JsonSerializer.h"

namespace Warpr::Messaging
{
  named_enum(WarprSignalingMessageType, 
    Unknown,
    ConnectionRequest,
    PairingCompleteMessage,
    PeerConnectionDescriptionMessage,
    PeerConnectionCandidateMessage
  );

  struct WarprSignalingMessage : public Axodox::Json::json_object_base
  {
    static Axodox::Infrastructure::type_registry<WarprSignalingMessage> derived_types;

    virtual WarprSignalingMessageType Type() const = 0;
  };

  struct ConnectionRequest : public WarprSignalingMessage
  {
    Axodox::Json::json_property<Axodox::Infrastructure::uuid> SessionId;

    ConnectionRequest();

    virtual WarprSignalingMessageType Type() const override;
  };

  struct DataChannelReliability : public Axodox::Json::json_object_base
  {
    Axodox::Json::json_property<bool> IsUnordered;
    Axodox::Json::json_property<std::optional<uint32_t>> MaxRetransmits;
    Axodox::Json::json_property<std::optional<std::chrono::milliseconds>> MaxPacketLifetime;

    DataChannelReliability();
  };

  struct VideoQualityOptions : public Axodox::Json::json_object_base
  {
    Axodox::Json::json_property<uint32_t> Bitrate;
    Axodox::Json::json_property<float> RenderingResolutionScale;
    Axodox::Json::json_property<float> StreamingResolutionScale;

    VideoQualityOptions();
  };

  struct PairingCompleteMessage : public WarprSignalingMessage
  {
    Axodox::Json::json_property<std::vector<std::string>> IceServers;
    Axodox::Json::json_property<float> ConnectionTimeout;
    Axodox::Json::json_property<DataChannelReliability> StreamChannelReliability;
    Axodox::Json::json_property<VideoQualityOptions> VideoQuality;

    PairingCompleteMessage();

    virtual WarprSignalingMessageType Type() const override;
  };

  struct PeerConnectionDescriptionMessage : public WarprSignalingMessage
  {
    Axodox::Json::json_property<std::string> Description;

    PeerConnectionDescriptionMessage();

    virtual WarprSignalingMessageType Type() const override;
  };

  struct PeerConnectionCandidateMessage : public WarprSignalingMessage
  {
    Axodox::Json::json_property<std::string> Candidate;

    PeerConnectionCandidateMessage();

    virtual WarprSignalingMessageType Type() const override;
  };
}