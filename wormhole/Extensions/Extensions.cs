namespace Warpr.Gateway.Extensions
{
  public static class Extensions
  {
    public static string RemoteEndPoint(this ConnectionInfo connection)
    {
      return $"{connection.RemoteIpAddress}:{connection.RemotePort}";
    }
  }
}
