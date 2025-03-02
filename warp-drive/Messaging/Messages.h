#pragma once
#include "Json/JsonSerializer.h"

namespace Warpr::Messaging
{
  named_enum(WarprMessageType, 
    Unknown,
    ConnectionRequest,
    PairingCompleteMessage,
    PeerConnectionDescriptionMessage,
    PeerConnectionCandidateMessage
  );

  struct WarprMessage : public Axodox::Json::json_object_base
  {
    static Axodox::Infrastructure::type_registry<WarprMessage> derived_types;

    virtual WarprMessageType Type() const = 0;
  };

  struct ConnectionRequest : public WarprMessage
  {
    Axodox::Json::json_property<Axodox::Infrastructure::uuid> SessionId;

    ConnectionRequest();

    virtual WarprMessageType Type() const override;
  };

  struct PairingCompleteMessage : public WarprMessage
  {
    virtual WarprMessageType Type() const override;
  };

  struct PeerConnectionDescriptionMessage : public WarprMessage
  {
    Axodox::Json::json_property<std::string> Description;

    PeerConnectionDescriptionMessage();

    virtual WarprMessageType Type() const override;
  };

  struct PeerConnectionCandidateMessage : public WarprMessage
  {
    Axodox::Json::json_property<std::string> Candidate;

    PeerConnectionCandidateMessage();

    virtual WarprMessageType Type() const override;
  };
}