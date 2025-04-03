using System.Collections.Concurrent;
using System.Net.WebSockets;
using System.Text;
using System.Text.Json;
using Warpr.Gateway.Extensions;
using Warpr.Gateway.Messages;

namespace Warpr.Gateway.Messaging
{
  public interface IWebSocketRepository
  {
    void RegisterStream(IWebSocketStream stream);

    event EventHandler<WebSocketConnection> ConnectionAdded, ConnectionRemoved;
  }

  public class WebSocketConnection
  {
    private readonly IWebSocketStream _stream;

    public string EndPoint { get; }

    public Guid SessionId { get; set; }

    public void SendMessage(WarprMessage message)
    {
      var jsonMessage = JsonSerializer.Serialize(message);
      var binaryMessage = Encoding.UTF8.GetBytes(jsonMessage);
      _stream.SendMessageAsync(binaryMessage, WebSocketMessageType.Text);
    }

    public event EventHandler<WarprMessage>? MessageReceived;

    public WebSocketConnection(IWebSocketStream stream)
    {
      _stream = stream;
      EndPoint = stream.EndPoint;

      _stream.MessageReceived += OnMessageReceived;
    }

    private void OnMessageReceived(object? sender, WebSocketMessageReceivedEventArgs e)
    {
      var jsonMessage = Encoding.UTF8.GetString(e.Payload);
      var warprMessage = JsonSerializer.Deserialize<WarprMessage>(jsonMessage);
      if (warprMessage != null) MessageReceived?.Invoke(this, warprMessage);
    }
  }

  public abstract class WebSocketRepository : IWebSocketRepository
  {
    private readonly ILogger _logger;
    private readonly ConcurrentDictionary<IWebSocketStream, WebSocketConnection> _connections = new();

    public event EventHandler<WebSocketConnection>? ConnectionAdded, ConnectionRemoved;

    public WebSocketRepository(
      ILogger logger)
    {
      _logger = logger;
    }

    public void RegisterStream(IWebSocketStream stream)
    {
      stream.Connected += OnStreamConnected;
      stream.MessageReceived += OnStreamMessageReceived;
      stream.Disconnected += OnStreamDisconnected;
    }

    private void OnStreamMessageReceived(object? sender, WebSocketMessageReceivedEventArgs e)
    {
      if (e.Type != WebSocketMessageType.Text) return;

      var jsonMessage = Encoding.UTF8.GetString(e.Payload);
      var warprMessage = JsonSerializer.Deserialize<WarprMessage>(jsonMessage);

      if (warprMessage is ConnectionRequest request)
      {
        var stream = (sender as IWebSocketStream)!;
        var connection = new WebSocketConnection(stream);
        connection.SessionId = request.SessionId;
        _connections[stream] = connection;

        ConnectionAdded?.Invoke(this, connection);

        _logger.LogInformation($"Endpoint {connection.EndPoint} connected with session id {connection.SessionId}.");
      }
    }

    private void OnStreamConnected(object? sender, EventArgs e)
    {
      _logger.LogInformation($"{(sender as IWebSocketStream)!.EndPoint} connecting...");
    }

    private void OnStreamDisconnected(object? sender, EventArgs e)
    {
      var stream = (sender as IWebSocketStream)!;
      if (_connections.TryRemove(stream, out var connection))
      {
        ConnectionRemoved?.Invoke(this, connection);
      }

      _logger.LogInformation($"{stream.EndPoint} disconnected.");
    }
  }
}
