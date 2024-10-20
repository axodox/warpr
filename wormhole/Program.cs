using Microsoft.AspNetCore.HttpOverrides;
using System.Reflection;
using Warpr.Gateway.Sources;

var builder = WebApplication.CreateBuilder(args);

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
  .AddSingleton<IStreamingSinkRepository, StreamingSinkRepository>();

var app = builder.Build();

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