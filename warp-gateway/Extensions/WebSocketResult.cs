using Microsoft.AspNetCore.Mvc;
using System.Net.WebSockets;

namespace Warpr.Gateway.Extensions
{
  public class WebSocketMessageReceivedEventArgs
  {
    public WebSocketMessageType Type { get; init; }
    public required byte[] Payload { get; init; }
  }

  public interface IWebSocketStream
  {
    event EventHandler? Connected, Disconnected;
    event EventHandler<WebSocketMessageReceivedEventArgs>? MessageReceived;
    HttpRequest? Request { get; }

    string EndPoint => Request!.HttpContext.Connection.RemoteEndPoint();

    Task SendMessageAsync(ReadOnlyMemory<byte> message, WebSocketMessageType messageType);
    Task CloseAsync();
  }

  public class WebSocketResult : ActionResult, IWebSocketStream, IDisposable
  {
    private WebSocket? _socket;
    private Task? _sendTask;
    private readonly object _sendSyncRoot = new();

    public HttpRequest? Request { get; private set; }

    public event EventHandler<WebSocketMessageReceivedEventArgs>? MessageReceived;
    public event EventHandler? Connected;
    public event EventHandler? Disconnected;

    public async Task CloseAsync()
    {
      if (_socket?.State == WebSocketState.Open)
      {
        try
        {
          await _socket.CloseAsync(WebSocketCloseStatus.Empty, "", CancellationToken.None);
        }
        catch
        {
          _socket.Abort();
        }
      }
    }

    public void Dispose()
    {
      _socket?.Dispose();
    }

    public override async Task ExecuteResultAsync(ActionContext context)
    {
      Request = context.HttpContext.Request;
      _socket = await context.HttpContext.WebSockets.AcceptWebSocketAsync();
      
      Connected?.Invoke(this, EventArgs.Empty);

      var buffer = new byte[64 * 1024];
      var stream = new MemoryStream();

      try
      {
        while (_socket.State == WebSocketState.Open)
        {
          var result = await _socket.ReceiveAsync(buffer, CancellationToken.None);
          stream.Write(buffer, 0, result.Count);
          if (result.EndOfMessage)
          {
            var eventArgs = new WebSocketMessageReceivedEventArgs
            {
              Type = result.MessageType,
              Payload = stream.ToArray()
            };

            MessageReceived?.Invoke(this, eventArgs);
            stream.SetLength(0);
          }
        }
      }
      catch { }
      finally
      {
        Disconnected?.Invoke(this, EventArgs.Empty);
      }
    }

    public async Task SendMessageAsync(ReadOnlyMemory<byte> message, WebSocketMessageType messageType)
    {
      if (_socket is null) throw new InvalidOperationException("The socket is not initialized.");

      Task task;
      lock (_sendSyncRoot)
      {
        var taskSource = () => _socket.SendAsync(message, messageType, true, CancellationToken.None);
        if (_sendTask is null)
        {
          _sendTask = taskSource().AsTask();
        }
        else
        {
          _sendTask = _sendTask.ContinueWith(p => taskSource());
        }
        task = _sendTask;
      }
      await task;
    }
  }
}
