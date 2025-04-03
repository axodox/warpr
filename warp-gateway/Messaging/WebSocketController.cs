using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.Routing;
using System.Diagnostics.CodeAnalysis;
using Warpr.Gateway.Extensions;

namespace Warpr.Gateway.Messaging
{
  public class HttpWebSocketMethodAttribute : HttpMethodAttribute
  {
    private static readonly IEnumerable<string> _supportedMethods = new[] { "GET", "CONNECT" };

    /// <summary>
    /// Creates a new <see cref="HttpGetAttribute"/>.
    /// </summary>
    public HttpWebSocketMethodAttribute()
        : base(_supportedMethods)
    {
    }

    /// <summary>
    /// Creates a new <see cref="HttpGetAttribute"/> with the given route template.
    /// </summary>
    /// <param name="template">The route template. May not be null.</param>
    public HttpWebSocketMethodAttribute([StringSyntax("Route")] string template)
        : base(_supportedMethods, template)
    {
      ArgumentNullException.ThrowIfNull(template);
    }
  }

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

    [HttpWebSocketMethod("connect")]
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
