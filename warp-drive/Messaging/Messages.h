#pragma once
#include "Json/JsonSerializer.h"

namespace Warpr::Messaging
{
  enum WarprMessageType
  {
    Unknown = 0,
    ConnectionDescription
  };

  struct WarprMessage : public Axodox::Json::json_object_base
  {
    virtual WarprMessageType Type() const = 0;
  };

  struct ConnectionDescriptionMessage : public WarprMessage
  {
    Axodox::Json::json_property<std::string> Description;

    ConnectionDescriptionMessage();

    virtual WarprMessageType Type() const override;
  };
}

namespace Axodox::Json
{
  template <>
  struct json_serializer<std::unique_ptr<Warpr::Messaging::WarprMessage>>
  {
    static Infrastructure::value_ptr<json_value> to_json(const std::unique_ptr<Warpr::Messaging::WarprMessage>& value);
    static bool from_json(const json_value* json, std::unique_ptr<Warpr::Messaging::WarprMessage>& value);
  };
}