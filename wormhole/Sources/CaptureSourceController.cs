using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;
using Warpr.Gateway.Extensions;

namespace Warpr.Gateway.Sources
{
  [ApiController]
  [Route("api/sources")]
  public class CaptureSourceController : ControllerBase
  {
    private readonly ILogger _logger;
    private readonly ICaptureSourceRepository _repository;

    public CaptureSourceController(
      ILogger<CaptureSourceController> logger,
      ICaptureSourceRepository repository)
    {
      _logger = logger;
      _repository = repository;
      _logger.LogInformation($"Source controller started.");
    }

    [HttpGet("connect")]
    public ActionResult Connect()
    {
      var connection = Request.HttpContext.Connection;
      _logger.LogInformation($"Source connection attempt from {connection.RemoteEndPoint()}.");

      if (!HttpContext.WebSockets.IsWebSocketRequest) return BadRequest("This API only supports web socket requests.");

      var result = new WebSocketResult();
      _repository.RegisterSource(result);
      return result;
    }
  }
}
