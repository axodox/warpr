using System.Reflection;
using Warpr.Gateway.Sources;

var builder = WebApplication.CreateBuilder(args);

// Add services to the container.
builder.Services
  .AddControllers()
  .AddApplicationPart(Assembly.GetExecutingAssembly());

builder.Services
  .AddSingleton<ICaptureSourceRepository, CaptureSourceRepository>();

var app = builder.Build();

// Configure the HTTP request pipeline.
app.UseHttpsRedirection();
app.UseAuthorization();
app.MapControllers();
app.UseWebSockets();
app.Run();
