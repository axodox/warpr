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

  struct PairingCompleteMessage : public WarprSignalingMessage
  {
    virtual WarprSignalingMessageType Type() const override;

    PairingCompleteMessage();

    Axodox::Json::json_property<std::vector<std::string>> IceServers;
    Axodox::Json::json_property<float> ConnectionTimeout;
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