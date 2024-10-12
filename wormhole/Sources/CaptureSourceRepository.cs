using Warpr.Gateway.Extensions;

namespace Warpr.Gateway.Sources
{
  public interface ICaptureSourceRepository
  {
    void RegisterSource(IWebSocketStream stream);
  }

  public class CaptureSourceRepository : ICaptureSourceRepository
  {
    private readonly ILogger _logger;

    public CaptureSourceRepository(
      ILogger<CaptureSourceRepository> logger)
    {
      _logger = logger;
    }

    public void RegisterSource(IWebSocketStream stream)
    {
      stream.Connected += OnSourceConnected;
      stream.Disconnected += OnStreamDisconnected;
    }

    private void OnSourceConnected(object? sender, EventArgs e)
    {
      var connection = (sender as IWebSocketStream)!.Request!.HttpContext.Connection;
      _logger.LogInformation($"Source {connection.RemoteEndPoint()} connected.");
    }

    private void OnStreamDisconnected(object? sender, EventArgs e)
    {
      var connection = (sender as IWebSocketStream)!.Request!.HttpContext.Connection;
      _logger.LogInformation($"Source {connection.RemoteEndPoint()} disconnected.");
    }
  }
}
