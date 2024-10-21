#include "Messages.h"
#include "Infrastructure/ValuePtr.h"

using namespace std;
using namespace Axodox::Infrastructure;
using namespace Warpr::Messaging;

namespace Warpr::Messaging
{
  ConnectionDescriptionMessage::ConnectionDescriptionMessage() :
    Description(this, "Description")
  { }

  WarprMessageType ConnectionDescriptionMessage::Type() const
  {
    return WarprMessageType::ConnectionDescription;
  }
}

namespace Axodox::Json
{
  Infrastructure::value_ptr<json_value> json_serializer<unique_ptr<WarprMessage>>::to_json(const std::unique_ptr<Warpr::Messaging::WarprMessage>& value)
  {
    //Serialize data
    auto result = json_serializer<WarprMessage>::to_json(*value);

    //Add type info
    {
      auto object = static_cast<json_object*>(result.get());

      const char* type;
      switch (value->Type())
      {
      case WarprMessageType::ConnectionDescription:
        type = "ConnectionDescription";
        break;
      default:
        throw logic_error("Message type not implemented!");
      }

      object->at("Type") = make_value<json_string>(type);
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

      //Instantiate result
      if (type == "ConnectionDescription")
      {
        value = make_unique<ConnectionDescriptionMessage>();
      }
      else
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
