namespace Warpr.Configuration
{
  public interface IGatewayConfiguration
  {
    List<string> IceServers { get; set; }

    public float ConnectionTimeout { get; set; }
  }

  public class GatewayConfiguration : IGatewayConfiguration
  {
    public List<string> IceServers { get; set; } = [
      "stun:stun.l.google.com:19302",
      "stun:stun.relay.metered.ca:80"
    ];

    public float ConnectionTimeout { get; set; } = 1;
  }
}
