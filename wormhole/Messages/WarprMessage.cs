using System.Text.Json.Serialization;

namespace Warpr.Gateway.Messages
{
  [JsonPolymorphic(TypeDiscriminatorPropertyName = "Type")]
  [JsonDerivedType(typeof(ConnectionRequest), nameof(ConnectionRequest))]
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

  public class PeerConnectionDescriptionMessage : WarprMessage
  {
    public string? Description { get; set; }

    public PeerConnectionDescriptionMessage() { }
  }

  public class PeerConnectionCandidateMessage : WarprMessage
  {
    public string? Candidate { get; set; }
    public PeerConnectionCandidateMessage() { }
  }

  public class StreamingSourcesMessage : WarprMessage
  {
    public string[]? Sources { get; set; }
    public StreamingSourcesMessage() { }
  }
}
