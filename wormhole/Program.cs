using Microsoft.AspNetCore.HttpOverrides;
using System.Reflection;
using Warpr.Configuration;
using Warpr.Gateway.Session;
using Warpr.Gateway.Sources;

var builder = WebApplication.CreateBuilder(args);
builder.WebHost.UseKestrel(p =>
{
  p.ListenAnyIP(5164);
  p.ListenAnyIP(7074, p => p.UseHttps("axodox-pc.pfx", "p2cyfqR4"));
});

// Add services to the container.
builder.Services
  .AddCors(p => p.AddPolicy("CorsPolicy", p => p
           .AllowAnyMethod()
           .AllowAnyHeader()        
           .AllowAnyOrigin()))
  .AddControllers()
  .AddApplicationPart(Assembly.GetExecutingAssembly());

builder.Services
  .Configure<ForwardedHeadersOptions>(options =>
  {
    options.ForwardedHeaders =
        ForwardedHeaders.XForwardedFor | ForwardedHeaders.XForwardedProto;
  });

builder.Services
  .AddSingleton<IGatewayConfiguration, GatewayConfiguration>()
  .AddSingleton<IStreamingSourceRepository, StreamingSourceRepository>()
  .AddSingleton<IStreamingSinkRepository, StreamingSinkRepository>()
  .AddSingleton<IMatchmaker, Matchmaker>();

var app = builder.Build();

app.Services.GetService<IMatchmaker>();

// Configure the HTTP request pipeline.
app.UseDefaultFiles(new DefaultFilesOptions());
app.UseStaticFiles(new StaticFileOptions
{
  ServeUnknownFileTypes = true,
  OnPrepareResponse = (context) =>
  {
    context.Context.Response.Headers.Append("Cross-Origin-Opener-Policy", "same-origin");
    context.Context.Response.Headers.Append("Cross-Origin-Embedder-Policy", "require-corp");
  }
});

app.UseRouting();
app.UseCors("CorsPolicy");
app.Use((context, next) =>
{
  context.Response.Headers.Append("Cross-Origin-Opener-Policy", "same-origin");
  context.Response.Headers.Append("Cross-Origin-Embedder-Policy", "require-corp");
  return next();
});

//app.UseHttpsRedirection();
app.UseAuthorization();
app.UseForwardedHeaders(new ForwardedHeadersOptions() { });
app.UseWebSockets();
app.MapControllers();
app.MapFallbackToFile("/index.html");
app.Run();