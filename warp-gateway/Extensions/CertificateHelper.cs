using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;

namespace Warpr.Gateway.Extensions
{
  public static class CertificateHelper
  {
    private static T? TryGet<T>(Func<T> function)
    {
      try
      {
        return function();
      }
      catch
      {
        return default;
      }
    }

    public static X509Certificate2 CreateRoot(string subjectName)
    {
      using var privateKey = RSA.Create(2048);
      var certificateRequest = new CertificateRequest($"CN={subjectName}", privateKey, HashAlgorithmName.SHA256, RSASignaturePadding.Pkcs1);
      certificateRequest.CertificateExtensions.Add(new X509BasicConstraintsExtension(true, false, 0, true));
      certificateRequest.CertificateExtensions.Add(new X509KeyUsageExtension(X509KeyUsageFlags.KeyCertSign | X509KeyUsageFlags.CrlSign, true));

      var validFrom = DateTimeOffset.UtcNow.AddDays(-1);
      var validUntil = validFrom.AddYears(10).AddDays(1);
      return certificateRequest.CreateSelfSigned(validFrom, validUntil);
    }

    public static X509Certificate2 Create(string subjectName, X509Certificate2? parentCertificate = null)
    {
      using var privateKey = RSA.Create(2048);
      var certificateRequest = new CertificateRequest($"CN={subjectName}", privateKey, HashAlgorithmName.SHA256, RSASignaturePadding.Pkcs1);
      certificateRequest.CertificateExtensions.Add(new X509BasicConstraintsExtension(false, false, 0, false));
      certificateRequest.CertificateExtensions.Add(new X509KeyUsageExtension(X509KeyUsageFlags.DigitalSignature | X509KeyUsageFlags.KeyEncipherment, true));
      certificateRequest.CertificateExtensions.Add(
        new X509EnhancedKeyUsageExtension(
          new OidCollection
          {
          new Oid("1.3.6.1.5.5.7.3.1"), //Server authentication
          new Oid("1.3.6.1.5.5.7.3.2")  //Client authentication
          },
          false));

      var alternativeNameBuilder = new SubjectAlternativeNameBuilder();
      foreach (var name in GetDnsNames())
      {
        alternativeNameBuilder.AddDnsName(name);
      }
      foreach (var address in GetIPAddresses())
      {
        alternativeNameBuilder.AddIpAddress(address);
      }
      certificateRequest.CertificateExtensions.Add(alternativeNameBuilder.Build());

      var validFrom = DateTimeOffset.UtcNow.AddDays(-1);
      var validUntil = validFrom.AddYears(1).AddDays(1);
      if (parentCertificate == null)
      {
        return certificateRequest.CreateSelfSigned(validFrom, validUntil);
      }
      else
      {
        if (parentCertificate.NotBefore > validFrom) validFrom = parentCertificate.NotBefore;
        if (parentCertificate.NotAfter < validUntil) validUntil = parentCertificate.NotAfter;
        var childCertificate = certificateRequest.Create(parentCertificate, validFrom, validUntil, Guid.NewGuid().ToByteArray());
        return childCertificate.CopyWithPrivateKey(privateKey);
      }
    }

    public static bool Validate(X509Certificate2 certificate2)
    {
      if (DateTime.UtcNow > certificate2.NotAfter) return false;

      var alternateNames = certificate2.Extensions
        .OfType<X509Extension>()
        .Where(p => p.Oid?.Value == "2.5.29.17") //Subject Alternative Name
        .Select(p => p.Format(false))
        .FirstOrDefault() ?? "";

      foreach (var name in GetDnsNames())
      {
        if (!alternateNames.Contains(name, StringComparison.InvariantCultureIgnoreCase)) return false;
      }

      foreach (var address in GetIPAddresses())
      {
        if (!alternateNames.Contains(address.ToString(), StringComparison.InvariantCultureIgnoreCase)) return false;
      }

      return true;
    }

    public static bool TryInstallCertificate(X509Certificate2 certificate)
    {
      try
      {
        using var certificateStore = new X509Store(StoreName.Root, StoreLocation.LocalMachine);
        certificateStore.Open(OpenFlags.ReadWrite);
        certificateStore.Add(certificate);
        certificateStore.Close();
        return true;
      }
      catch
      {
        return false;
      }
    }

    public static bool TryExport(X509Certificate2 certificate, string filePath)
    {
      try
      {
        var extension = Path.GetExtension(filePath).ToLower();
        switch (extension)
        {
          case ".pfx":
            File.WriteAllBytes(filePath, certificate.Export(X509ContentType.Pfx));
            break;
          case ".cer":
            File.WriteAllBytes(filePath, certificate.Export(X509ContentType.Cert));
            break;
          case ".pem":
            File.WriteAllText(filePath, certificate.ExportCertificatePem());
            break;
          case ".key":
            File.WriteAllText(filePath, certificate.GetRSAPrivateKey()?.ExportPkcs8PrivateKeyPem());
            break;
          default:
            throw new ArgumentException($"Unsupported extension '{extension}'.", nameof(filePath));
        }
        return true;
      }
      catch
      {
        return false;
      }
    }

    public static X509Certificate2? TryImport(string filePath)
    {
      try
      {
        if (!File.Exists(filePath)) return null;

        var pfxBytes = File.ReadAllBytes(filePath);
        return new X509Certificate2(pfxBytes, null as string, X509KeyStorageFlags.Exportable);
      }
      catch
      {
        return null;
      }
    }

    public static List<IPAddress> GetIPAddresses()
    {
      return NetworkInterface
        .GetAllNetworkInterfaces()
        .SelectMany(p => p.GetIPProperties().UnicastAddresses.Select(q => q.Address))
        .Where(p => p.AddressFamily == AddressFamily.InterNetwork)
        .ToList();
    }

    public static List<string> GetDnsNames()
    {
      var results = new List<string>();
      var domain = TryGet(() => IPGlobalProperties.GetIPGlobalProperties().DomainName);
      var computerName = TryGet(() => Dns.GetHostName());

      results.Add("localhost");
      if (!string.IsNullOrEmpty(computerName)) results.Add(computerName);
      if (!string.IsNullOrEmpty(computerName) && !string.IsNullOrEmpty(domain)) results.Add(computerName + "." + domain);
      return results;
    }
  }
}