using Warpr.Gateway.Messaging;

namespace Warpr.Gateway.Sources
{
  public interface IStreamingSourceRepository : IWebSocketRepository { }

  public class StreamingSourceRepository : WebSocketRepository, IStreamingSourceRepository
  {
    public StreamingSourceRepository(ILogger<StreamingSourceRepository> logger) :
      base(logger)
    { }
  }
}
