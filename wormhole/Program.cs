using Microsoft.AspNetCore.HttpOverrides;
using Warpr.Gateway.Extensions;

//Ensure certificates
var rootCertificate = CertificateHelper.TryImport("wormhole-ca.pfx");
if (rootCertificate == null)
{
  rootCertificate = CertificateHelper.CreateRoot("wormhole-ca");
  CertificateHelper.TryExport(rootCertificate, "wormhole-ca.pfx");
  CertificateHelper.TryExport(rootCertificate, "wormhole-ca.cer");
}

var certificate = CertificateHelper.TryImport("wormhole.pfx");
if (certificate == null || !CertificateHelper.Validate(certificate))
{
  certificate = CertificateHelper.Create("wormhole", rootCertificate);
  CertificateHelper.TryExport(certificate, "wormhole.cer");
  CertificateHelper.TryExport(certificate, "wormhole.pfx");
  CertificateHelper.TryExport(certificate, "wormhole.pem");
  CertificateHelper.TryExport(certificate, "wormhole.key");
}

if (!certificate.Verify())
{
  CertificateHelper.TryInstallCertificate(certificate);
}

//Create builder
var builder = WebApplication.CreateBuilder(args);
builder.WebHost.UseKestrel(p =>
{
  p.ListenAnyIP(5164);
  p.ListenAnyIP(7074, p => p.UseHttps(certificate));
});

// Add services to the container.
builder.Services
  .AddCors(p => p.AddPolicy("CorsPolicy", p => p
           .AllowAnyMethod()
           .AllowAnyHeader()
           .AllowAnyOrigin()))
  .AddControllers()
  .AddWarprControllers();

builder.Services
  .Configure<ForwardedHeadersOptions>(options =>
  {
    options.ForwardedHeaders =
        ForwardedHeaders.XForwardedFor | ForwardedHeaders.XForwardedProto;
  });

builder.Services.AddWarprServices();

var app = builder.Build();
app.Services.InitializeWarpr();

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