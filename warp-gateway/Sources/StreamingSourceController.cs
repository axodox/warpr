using Microsoft.AspNetCore.Mvc;
using Warpr.Gateway.Messaging;

namespace Warpr.Gateway.Sources
{
  [ApiController]
  [Route("api/sources")]
  public class StreamingSourceController : WebSocketController
  {
    public StreamingSourceController(
      ILogger<StreamingSourceController> logger,
      IStreamingSourceRepository repository) : base(logger, repository) 
    { }
  }
}
