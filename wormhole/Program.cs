using Microsoft.AspNetCore.HttpOverrides;
using System.Reflection;
using Warpr.Gateway.Session;
using Warpr.Gateway.Sources;

var builder = WebApplication.CreateBuilder(args);
builder.WebHost.UseKestrel(p => {
  p.ListenAnyIP(5000);
  p.ListenAnyIP(5001, p => p.UseHttps("axodox-pc.pfx", "p2cyfqR4"));
});

// Add services to the container.
builder.Services
  .AddControllers()
  .AddApplicationPart(Assembly.GetExecutingAssembly());

builder.Services
  .Configure<ForwardedHeadersOptions>(options =>
  {
    options.ForwardedHeaders =
        ForwardedHeaders.XForwardedFor | ForwardedHeaders.XForwardedProto;
  });

builder.Services
  .AddSingleton<IStreamingSourceRepository, StreamingSourceRepository>()
  .AddSingleton<IStreamingSinkRepository, StreamingSinkRepository>()
  .AddSingleton<IMatchmaker, Matchmaker>();

var app = builder.Build();

app.Services.GetService<IMatchmaker>();

// Configure the HTTP request pipeline.
app.UseDefaultFiles();
app.UseStaticFiles();
//app.UseHttpsRedirection();
app.UseAuthorization();
app.UseForwardedHeaders();
app.UseWebSockets();
app.MapControllers();
app.MapFallbackToFile("/index.html");
app.Run();