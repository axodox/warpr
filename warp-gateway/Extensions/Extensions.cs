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

    public static IMvcBuilder AddWarprControllers(this IMvcBuilder builder)
    {
      return builder.AddApplicationPart(Assembly.GetExecutingAssembly());
    }

    public static IMvcCoreBuilder AddWarprControllers(this IMvcCoreBuilder builder)
    {
      return builder.AddApplicationPart(Assembly.GetExecutingAssembly());
    }

    public static IServiceCollection AddWarprServices(this IServiceCollection services)
    {
      return services
        .AddSingleton<IGatewayConfiguration, GatewayConfiguration>()
        .AddSingleton<IStreamingSourceRepository, StreamingSourceRepository>()
        .AddSingleton<IStreamingSinkRepository, StreamingSinkRepository>()
        .AddSingleton<IMatchmaker, Matchmaker>();
    }

    public static IServiceProvider InitializeWarpr(this IServiceProvider serviceProvider)
    {
      serviceProvider.GetService<IMatchmaker>();
      return serviceProvider;
    }
  }
}
