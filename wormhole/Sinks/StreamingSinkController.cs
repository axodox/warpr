using Microsoft.AspNetCore.Mvc;
using Warpr.Gateway.Streams;

namespace Warpr.Gateway.Sources
{
  [ApiController]
  [Route("api/sinks")]
  public class StreamingSinkController : WebSocketController
  {
    public StreamingSinkController(
      ILogger<StreamingSinkController> logger,
      IStreamingSinkRepository repository) : base(logger, repository)
    { }
  }
}
