#include "warpr_includes.h"
#include "SignalingMessages.h"
#include "Infrastructure/ValuePtr.h"

using namespace std;
using namespace winrt;
using namespace Axodox::Infrastructure;
using namespace Warpr::Messaging;

namespace Warpr::Messaging
{
  Axodox::Infrastructure::type_registry<WarprSignalingMessage> WarprSignalingMessage::derived_types = type_registry<WarprSignalingMessage>::create<
    ConnectionRequest, 
    PairingCompleteMessage, 
    PeerConnectionDescriptionMessage, 
    PeerConnectionCandidateMessage>();

  ConnectionRequest::ConnectionRequest() :
    SessionId(this, "SessionId")
  { }

  WarprSignalingMessageType ConnectionRequest::Type() const
  {
    return WarprSignalingMessageType::ConnectionRequest;
  }

  WarprSignalingMessageType PairingCompleteMessage::Type() const
  {
    return WarprSignalingMessageType::PairingCompleteMessage;
  }

  PeerConnectionDescriptionMessage::PeerConnectionDescriptionMessage() :
    Description(this, "Description")
  { }

  WarprSignalingMessageType PeerConnectionDescriptionMessage::Type() const
  {
    return WarprSignalingMessageType::PeerConnectionDescriptionMessage;
  }

  PeerConnectionCandidateMessage::PeerConnectionCandidateMessage() :
    Candidate(this, "Candidate")
  { }

  WarprSignalingMessageType PeerConnectionCandidateMessage::Type() const
  {
    return WarprSignalingMessageType::PeerConnectionCandidateMessage;
  }
}
