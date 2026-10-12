param($root = $PSScriptRoot, [hashtable] $versions = @{})

# Packages to update to their latest stable version on nuget.org
$packageIds = @(
  "Axodox.Common"
  "libdatachannel"
  "nvenc"
)

# Rewrites a file with a regex replacement, preserving its BOM
function Update-File($path, $pattern, $replacement) {
  $bytes = [IO.File]::ReadAllBytes($path)
  $hasBom = ($bytes.Length -ge 3) -and ($bytes[0] -eq 0xEF) -and ($bytes[1] -eq 0xBB) -and ($bytes[2] -eq 0xBF)
  $content = [IO.File]::ReadAllText($path)
  $updated = $content -replace $pattern, $replacement
  if ($updated -ne $content) {
    [IO.File]::WriteAllText($path, $updated, (New-Object Text.UTF8Encoding $hasBom))
  }
}

# Returns the latest stable version of a package on nuget.org
function Get-LatestVersion($packageId) {
  $index = Invoke-RestMethod -Uri "https://api.nuget.org/v3-flatcontainer/$($packageId.ToLower())/index.json"
  $version = $index.versions |
    Where-Object { $_ -match '^\d+\.\d+\.\d+(\.\d+)?$' } |
    Sort-Object { [Version]$_ } |
    Select-Object -Last 1

  if ($null -eq $version) {
    throw "No stable $packageId version found."
  }

  return $version
}

$projects = Resolve-Path "$root/*/*.vcxproj"
$nuspecs = Resolve-Path "$root/*/*.nuspec"
$result = @{}

foreach ($packageId in $packageIds) {
  # Select package version
  $version = $versions[$packageId]
  if ($null -eq $version) {
    Write-Host "Fetching $packageId version..." -ForegroundColor Magenta
    try {
      $version = Get-LatestVersion $packageId
    }
    catch {
      Write-Host "Failed to fetch $packageId version: $_" -ForegroundColor Red
      throw
    }
    Write-Host "$packageId version is $version"
  }
  else {
    Write-Host "Using $packageId version $version." -ForegroundColor Magenta
  }

  $result[$packageId] = $version
  $escapedId = [Regex]::Escape($packageId)

  # Process projects
  foreach ($projectPath in $projects) {
    $configPath = Join-Path (Split-Path $projectPath) 'packages.config'
    if (-not (Test-Path $configPath) -or -not (Select-String -Path $configPath -Pattern "id=`"$escapedId`"" -Quiet)) {
      continue
    }

    Write-Host "  $projectPath"

    # Update project file
    Update-File $projectPath "\\packages\\$escapedId\.\d[^\\]*\\" "\packages\$packageId.$version\"

    # Update packages.config
    Update-File $configPath "(<package id=`"$escapedId`" version=`")[^`"]+`"" "`${1}$version`""
  }

  # Process nuspecs
  foreach ($nuspecPath in $nuspecs) {
    Update-File $nuspecPath "(<dependency id=`"$escapedId`" version=`")[^`"]+`"" "`${1}$version`""
  }
}

return $result
