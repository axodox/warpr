using Microsoft.AspNetCore.Mvc;
using Warpr.Gateway.Extensions;
using Warpr.Gateway.Messaging;

namespace Warpr.Gateway.Streams
{
  public abstract class WebSocketController : ControllerBase
  {
    private readonly ILogger _logger;
    private readonly IWebSocketRepository _repository;

    public WebSocketController(
      ILogger logger,
      IWebSocketRepository repository)
    {
      _logger = logger;
      _repository = repository;
    }

    [HttpGet("connect")]
    public ActionResult Connect()
    {
      var connection = Request.HttpContext.Connection;
      _logger.LogInformation($"Connection attempt from {connection.RemoteEndPoint()}.");

      if (!HttpContext.WebSockets.IsWebSocketRequest) return BadRequest("This API only supports web socket requests.");

      var result = new WebSocketResult();
      _repository.RegisterStream(result);
      return result;
    }
  }
}
