using System.Net;
using System.Net.NetworkInformation;
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

    public static X509Certificate2 CreateSelfSigned(string subjectName)
    {
      using var encryptionAlghorithm = RSA.Create(2048);
      var certificateRequest = new CertificateRequest($"CN={subjectName}", encryptionAlghorithm, HashAlgorithmName.SHA256, RSASignaturePadding.Pkcs1);
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
      foreach (var name in GetNames())
      {
        alternativeNameBuilder.AddDnsName(name);
      }
      certificateRequest.CertificateExtensions.Add(alternativeNameBuilder.Build());

      var now = DateTimeOffset.UtcNow;
      return certificateRequest.CreateSelfSigned(now, now.AddYears(1));
    }

    public static bool Validate(X509Certificate2 certificate2)
    {
      if (DateTime.UtcNow > certificate2.NotAfter) return false;

      var alternateNames = certificate2.Extensions
        .OfType<X509Extension>()
        .Where(p => p.Oid?.Value == "2.5.29.17") //Subject Alternative Name
        .Select(p => p.Format(false))
        .FirstOrDefault() ?? "";
      foreach (var name in GetNames())
      {
        if (!alternateNames.Contains(name, StringComparison.InvariantCultureIgnoreCase)) return false;
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

    public static bool TryStore(X509Certificate certificate, string filePath)
    {
      try
      {
        var pfxBytes = certificate.Export(X509ContentType.Pfx);
        File.WriteAllBytes(filePath, pfxBytes);
        return true;
      }
      catch
      {
        return false;
      }
    }

    public static bool TryExportPemAndKey(X509Certificate2 certificate, string filePath)
    {
      try
      {
        var pem = certificate.ExportCertificatePem();
        if (pem == null) return false;
        File.WriteAllText(filePath + ".pem", pem);

        var key = certificate.GetRSAPrivateKey()?.ExportPkcs8PrivateKeyPem();
        if (key == null) return false;
        File.WriteAllText(filePath + ".key", key);

        return true;
      }
      catch
      {
        return false;
      }
    }

    public static X509Certificate2? TryLoad(string filePath)
    {
      try
      {
        var pfxBytes = File.ReadAllBytes(filePath);
        return new X509Certificate2(pfxBytes, null as string, X509KeyStorageFlags.Exportable);
      }
      catch
      {
        return null;
      }
    }

    public static List<string> GetNames()
    {
      var results = new List<string>();
      var domain = TryGet(() => IPGlobalProperties.GetIPGlobalProperties().DomainName);
      var computerName = TryGet(() => Dns.GetHostName());
      var ips = NetworkInterface.GetAllNetworkInterfaces().SelectMany(p => p.GetIPProperties().UnicastAddresses.Select(q => q.Address.ToString()));

      results.Add("localhost");
      if (!string.IsNullOrEmpty(computerName)) results.Add(computerName);
      if (!string.IsNullOrEmpty(computerName) && !string.IsNullOrEmpty(domain)) results.Add(computerName + "." + domain);
      results.AddRange(ips);
      return results;
    }
  }
}