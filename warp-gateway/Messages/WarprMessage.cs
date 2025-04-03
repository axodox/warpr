using System.Text.Json.Serialization;

namespace Warpr.Gateway.Messages
{
  [JsonPolymorphic]
  [JsonDerivedType(typeof(ConnectionRequest), nameof(ConnectionRequest))]
  [JsonDerivedType(typeof(PairingCompleteMessage), nameof(PairingCompleteMessage))]
  [JsonDerivedType(typeof(PeerConnectionDescriptionMessage), nameof(PeerConnectionDescriptionMessage))]
  [JsonDerivedType(typeof(PeerConnectionCandidateMessage), nameof(PeerConnectionCandidateMessage))]
  [JsonDerivedType(typeof(StreamingSourcesMessage), nameof(StreamingSourcesMessage))]
  public class WarprMessage
  {

  }

  public class ConnectionRequest : WarprMessage
  {
    public Guid SessionId { get; set; }

    public ConnectionRequest() { }
  }

  public class PairingCompleteMessage : WarprMessage
  {
    public string[] IceServers { get; init; } = [];

    public float ConnectionTimeout { get; init; }
  }

  public class PeerConnectionDescriptionMessage : WarprMessage
  {
    public string? Description { get; set; }
  }

  public class PeerConnectionCandidateMessage : WarprMessage
  {
    public string? Candidate { get; set; }
  }

  public class StreamingSourcesMessage : WarprMessage
  {
    public string[]? Sources { get; set; }
  }
}
