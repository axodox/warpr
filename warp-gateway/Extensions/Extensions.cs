using System.Reflection;
using Warpr.Configuration;
using Warpr.Gateway.Session;
using Warpr.Gateway.Sources;

namespace Warpr.Gateway.Extensions
{
  public static class Extensions
  {
    public static string RemoteEndPoint(this ConnectionInfo connection)
    {
      return $"{connection.RemoteIpAddress}:{connection.RemotePort}";
    }

    public static void AddWarprControllers(this IMvcBuilder builder)
    {
      builder.AddApplicationPart(Assembly.GetExecutingAssembly());
    }

    public static void AddWarprServices(this IServiceCollection services)
    {
      services
        .AddSingleton<IGatewayConfiguration, GatewayConfiguration>()
        .AddSingleton<IStreamingSourceRepository, StreamingSourceRepository>()
        .AddSingleton<IStreamingSinkRepository, StreamingSinkRepository>()
        .AddSingleton<IMatchmaker, Matchmaker>();
    }

    public static void InitializeWarpr(this IServiceProvider serviceProvider)
    {
      serviceProvider.GetService<IMatchmaker>();
    }
  }
}
