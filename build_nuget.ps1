# Initialize build environment
Write-Host 'Finding Visual Studio...' -ForegroundColor Magenta
$vsPath = .\Tools\vswhere.exe -latest -property installationPath
Write-Host $vsPath

Write-Host 'Importing environment variables...' -ForegroundColor Magenta
cmd.exe /c "call `"$vsPath\VC\Auxiliary\Build\vcvars64.bat`" && set > %temp%\vcvars.txt"
Get-Content "$env:temp\vcvars.txt" | Foreach-Object {
  if ($_ -match "^(.*?)=(.*)$") {
      Set-Content "env:\$($matches[1])" $matches[2]
  }
}

# Build projects
$coreCount = (Get-CimInstance -class Win32_ComputerSystem).NumberOfLogicalProcessors
$configurations = "Debug", "Release"
$platforms = "x64", "x86"

foreach ($platform in $platforms) {
  foreach ($config in $configurations) {
    Write-Host "Building $platform $config..." -ForegroundColor Magenta
    MSBuild.exe .\warpr.sln -t:warp-drive -p:Configuration=$config -p:Platform=$platform -m:$coreCount -v:m
  }
}

# Pack nuget
Write-Host 'Creating output directory...' -ForegroundColor Magenta
New-Item -Path '.\Output' -ItemType Directory -Force

Write-Host 'Patching nuspec...' -ForegroundColor Magenta
$nuspec = [xml](Get-Content "$PSScriptRoot\warp-drive\Warpr.WarpDrive.nuspec")

$nuspec.package.metadata.version = if ($null -ne $env:APPVEYOR_BUILD_VERSION) { $env:APPVEYOR_BUILD_VERSION } else { "1.0.0.0" }
$nuspec.package.metadata.repository.branch = if ($null -ne $env:APPVEYOR_REPO_BRANCH) { $env:APPVEYOR_REPO_BRANCH } else { "main" }
$commit = if ($null -ne $env:APPVEYOR_REPO_COMMIT) { $env:APPVEYOR_REPO_COMMIT } else { $null }
if ($null -ne $commit) {
  $nuspec.package.metadata.repository.SetAttribute("commit", $commit)
}

$nuspec.Save("$PSScriptRoot\warp-drive\WarpDrive.Patched.nuspec")

Write-Host 'Creating nuget package...' -ForegroundColor Magenta
.\Tools\nuget.exe pack .\warp-drive\WarpDrive.Patched.nuspec -OutputDirectory .\Output
Remove-Item -Path '.\warp-drive\WarpDrive.Patched.nuspec'