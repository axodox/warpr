namespace Warpr.Configuration
{
  public interface IGatewayConfiguration
  {
    List<string> IceServers { get; set; }

    float ConnectionTimeout { get; set; }

    DataChannelReliability StreamChannelReliability { get; set; }

    VideoQualityOptions VideoQuality { get; set; }
  }

  public class DataChannelReliability
  {
    public bool IsUnordered { get; set; } = true;

    public uint? MaxRetransmits { get; set; } = 0;

    public uint? MaxPacketLifetime { get; set; } = null; //In milliseconds
  }

  public class VideoQualityOptions
  {
    public uint Bitrate { get; set; } = 12_800_000; //In bits/sec

    public float RenderingResolutionScale { get; set; } = 1;
    public float StreamingResolutionScale { get; set; } = 1;
  }

  public class GatewayConfiguration : IGatewayConfiguration
  {
    public List<string> IceServers { get; set; } = [
      "stun:stun.l.google.com:19302",
      "stun:stun.relay.metered.ca:80"
    ];

    public float ConnectionTimeout { get; set; } = 1;

    public DataChannelReliability StreamChannelReliability { get; set; } = new DataChannelReliability();

    public VideoQualityOptions VideoQuality { get; set; } = new VideoQualityOptions();
  }
}
