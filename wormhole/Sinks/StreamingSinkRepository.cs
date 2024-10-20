using Warpr.Gateway.Messaging;

namespace Warpr.Gateway.Sources
{
  public interface IStreamingSinkRepository : IWebSocketRepository { }

  public class StreamingSinkRepository : WebSocketRepository, IStreamingSinkRepository
  {
    public StreamingSinkRepository(ILogger<StreamingSinkRepository> logger) :
      base(logger)
    { }
  }
}
