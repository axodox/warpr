using System.Collections.Concurrent;
using Warpr.Gateway.Extensions;

namespace Warpr.Gateway.Messaging
{
  public interface IWebSocketRepository
  {
    void RegisterStream(IWebSocketStream stream);
  }

  public class MessagingSource
  {
    private readonly IWebSocketStream _stream;

    public string EndPoint { get; }

    public MessagingSource(IWebSocketStream stream)
    {
      _stream = stream;
      EndPoint = stream.EndPoint;
    }
  }

  public abstract class WebSocketRepository : IWebSocketRepository
  {
    private readonly ILogger _logger;
    private readonly ConcurrentDictionary<string, MessagingSource> _sources = new();

    public WebSocketRepository(
      ILogger logger)
    {
      _logger = logger;
    }

    public void RegisterStream(IWebSocketStream stream)
    {
      stream.Connected += OnStreamConnected;
      stream.Disconnected += OnStreamDisconnected;
    }

    private void OnStreamConnected(object? sender, EventArgs e)
    {
      var source = new MessagingSource((sender as IWebSocketStream)!);
      _sources[source.EndPoint] = source;
      _logger.LogInformation($"{source.EndPoint} connected.");
    }

    private void OnStreamDisconnected(object? sender, EventArgs e)
    {
      var stream = (sender as IWebSocketStream)!;
      _sources.Remove(stream.EndPoint, out var source);
      _logger.LogInformation($"{stream.EndPoint} disconnected.");
    }
  }
}
