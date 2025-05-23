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

  DataChannelReliability::DataChannelReliability() :
    IsUnordered(this, "IsUnordered", true),
    MaxRetransmits(this, "MaxRetransmits", 0),
    MaxPacketLifetime(this, "MaxPacketLifetime", nullopt)
  { }

  VideoQualityOptions::VideoQualityOptions() :
    Bitrate(this, "Bitrate", 12'800'000),
    RenderingResolutionScale(this, "RenderingResolutionScale", 1.f),
    StreamingResolutionScale(this, "StreamingResolutionScale", 1.f)
  { }

  PairingCompleteMessage::PairingCompleteMessage() :
    IceServers(this, "IceServers"),
    ConnectionTimeout(this, "ConnectionTimeout"),
    StreamChannelReliability(this, "StreamChannelReliability"),
    VideoQuality(this, "VideoQuality")
  { }

  WarprSignalingMessageType PairingCompleteMessage::Type() const
  {
    return WarprSignalingMessageType::PairingCompleteMessage;
  }

  WarprSignalingMessageType PeerConnectionCandidateMessage::Type() const
  {
    return WarprSignalingMessageType::PeerConnectionCandidateMessage;
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
}
