#include "warpr_includes.h"
#include "Messages.h"
#include "Infrastructure/ValuePtr.h"

using namespace std;
using namespace winrt;
using namespace Axodox::Infrastructure;
using namespace Warpr::Messaging;

namespace Warpr::Messaging
{
  ConnectionRequest::ConnectionRequest() :
    SessionId(this, "SessionId")
  { }

  WarprMessageType ConnectionRequest::Type() const
  {
    return WarprMessageType::ConnectionRequest;
  }

  WarprMessageType PairingCompleteMessage::Type() const
  {
    return WarprMessageType::PairingCompleteMessage;
  }

  PeerConnectionDescriptionMessage::PeerConnectionDescriptionMessage() :
    Description(this, "Description")
  { }

  WarprMessageType PeerConnectionDescriptionMessage::Type() const
  {
    return WarprMessageType::PeerConnectionDescriptionMessage;
  }

  PeerConnectionCandidateMessage::PeerConnectionCandidateMessage() :
    Candidate(this, "Candidate")
  { }

  WarprMessageType PeerConnectionCandidateMessage::Type() const
  {
    return WarprMessageType::PeerConnectionCandidateMessage;
  }
}

namespace Axodox::Json
{
  const std::string json_serializer<unique_ptr<WarprMessage>>::type_names[] = {
    "Unknown",
    "ConnectionRequest",
    "PairingCompleteMessage",
    "PeerConnectionDescriptionMessage",
    "PeerConnectionCandidateMessage"
  };

  const std::function<unique_ptr<Warpr::Messaging::WarprMessage>()> json_serializer<unique_ptr<WarprMessage>>::type_factories[] = {
    [] { return nullptr; },
    [] { return make_unique<ConnectionRequest>(); },
    [] { return make_unique<PairingCompleteMessage>(); },
    [] { return make_unique<PeerConnectionDescriptionMessage>(); },
    [] { return make_unique<PeerConnectionCandidateMessage>(); }
  };

  Infrastructure::value_ptr<json_value> json_serializer<unique_ptr<WarprMessage>>::to_json(const std::unique_ptr<Warpr::Messaging::WarprMessage>& value)
  {
    auto result = make_value<json_object>();
    result->at("Type") = make_value<json_string>(type_names[size_t(value->Type())]);
    
    auto serialized = json_serializer<WarprMessage>::to_json(*value);
    for (auto& [propertyName, propertyValue] : static_cast<json_object*>(serialized.get())->value)
    {
      result->value[propertyName] = move(propertyValue);
    }

    return result;
  }

  bool json_serializer<unique_ptr<WarprMessage>>::from_json(const json_value* json, std::unique_ptr<Warpr::Messaging::WarprMessage>& value)
  {
    if (json && json->type() == json_type::object)
    {
      auto object = static_cast<const json_object*>(json);

      //Decode type info
      json_value* typeValue;
      if (!object->try_get_value("Type", typeValue) || typeValue->type() != json_type::string) return false;

      auto& type = static_cast<const json_string*>(typeValue)->value;
      for (auto i = 0; auto & name : type_names)
      {
        if (name == type)
        {
          value = type_factories[i]();
        }

        i++;
      }

      //Instantiate result
      if (!value)
      {
        return false;
      }

      //Deserialize data
      return json_serializer<WarprMessage>::from_json(json, *value);
    }
    else
    {
      return false;
    }
  }
}
