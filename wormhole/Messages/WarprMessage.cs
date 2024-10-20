using System.Text.Json.Serialization;

namespace Warpr.Gateway.Messages
{
  [JsonPolymorphic(TypeDiscriminatorPropertyName = "Type")]
  [JsonDerivedType(typeof(StreamingSourcesMessage), nameof(StreamingSourcesMessage))]
  [JsonDerivedType(typeof(StreamSignalingMessage), nameof(StreamSignalingMessage))]
  public abstract class WarprMessage
  {

  }

  public class StreamingSourcesMessage : WarprMessage
  {
    public string[]? Sources { get; set; }
  }

  public class StreamSignalingMessage : WarprMessage
  {
    public string? Source { get; set; }
    public string? Data { get; set; }
  }
}
