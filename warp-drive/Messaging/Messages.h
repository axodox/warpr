#pragma once
#include "Json/JsonSerializer.h"

namespace Warpr::Messaging
{
  enum class WarprMessageType
  {
    Unknown = 0,
    ConnectionRequest,
    PairingCompleteMessage,
    PeerConnectionDescriptionMessage,
    PeerConnectionCandidateMessage
  };

  struct WarprMessage : public Axodox::Json::json_object_base
  {
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

namespace Axodox::Json
{
  template <>
  struct json_serializer<std::unique_ptr<Warpr::Messaging::WarprMessage>>
  {
    static const std::string type_names[];
    static const std::function<std::unique_ptr<Warpr::Messaging::WarprMessage>()> type_factories[];
    
    static Infrastructure::value_ptr<json_value> to_json(const std::unique_ptr<Warpr::Messaging::WarprMessage>& value);
    static bool from_json(const json_value* json, std::unique_ptr<Warpr::Messaging::WarprMessage>& value);
  };
}