#include "warpr_includes.h"
#include "Messages.h"
#include "Infrastructure/ValuePtr.h"

using namespace std;
using namespace winrt;
using namespace Axodox::Infrastructure;
using namespace Warpr::Messaging;

namespace Warpr::Messaging
{
  Axodox::Infrastructure::type_registry<WarprMessage> WarprMessage::derived_types = type_registry<WarprMessage>::create<
    ConnectionRequest, 
    PairingCompleteMessage, 
    PeerConnectionDescriptionMessage, 
    PeerConnectionCandidateMessage>();

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
