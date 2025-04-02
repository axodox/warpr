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

Write-Host 'Updating dependencies...' -ForegroundColor Magenta
.\Tools\nuget.exe restore .\warpr.sln

Write-Host 'Collecting build version information...' -ForegroundColor Magenta
$version = if ($null -ne $env:APPVEYOR_BUILD_VERSION) { $env:APPVEYOR_BUILD_VERSION } else { "1.0.0.0" }
Write-Host "Version: $version"

$branch = if ($null -ne $env:APPVEYOR_REPO_BRANCH) { $env:APPVEYOR_REPO_BRANCH } else { "master" }
Write-Host "Branch: $branch"

$commit = if ($null -ne $env:APPVEYOR_REPO_COMMIT) { $env:APPVEYOR_REPO_COMMIT } else { $null }
Write-Host "Commit: $commit"

# Build projects
Write-Host 'Building warp-drive...' -ForegroundColor Magenta
$coreCount = (Get-CimInstance -class Win32_ComputerSystem).NumberOfLogicalProcessors
$configurations = "Debug", "Release"
$platforms = "x64", "x86"

foreach ($platform in $platforms) {
  foreach ($config in $configurations) {
    Write-Host "Building $platform $config..." -ForegroundColor Magenta
    MSBuild.exe .\warpr.sln -t:warp-drive -p:Configuration=$config -p:Platform=$platform -m:$coreCount -v:m
    
    if ($LastExitCode -eq 0) {
      Write-Host "Building $platform $config succeeded!" -ForegroundColor Green
    }
    else {
      Write-Host "Building $platform $config failed!" -ForegroundColor Red 
      throw "Building $platform $config failed!"
    }
  }
}

# Pack nuget
Write-Host 'Creating output directory...' -ForegroundColor Magenta
New-Item -Path '.\Output' -ItemType Directory -Force

Write-Host 'Patching nuspec...' -ForegroundColor Magenta
$nuspec = [xml](Get-Content "$PSScriptRoot\warp-drive\Warpr.WarpDrive.nuspec")

$nuspec.package.metadata.version = $version
$nuspec.package.metadata.repository.branch = $branch
if ($null -ne $commit) {
  $nuspec.package.metadata.repository.SetAttribute("commit", $commit)
}

$nuspec.Save("$PSScriptRoot\warp-drive\WarpDrive.Patched.nuspec")

Write-Host 'Creating nuget package...' -ForegroundColor Magenta
.\Tools\nuget.exe pack .\warp-drive\WarpDrive.Patched.nuspec -OutputDirectory .\Output
Remove-Item -Path '.\warp-drive\WarpDrive.Patched.nuspec'

## Build gateway
Write-Host 'Building warp-gateway...' -ForegroundColor Magenta
MSBuild.exe .\warpr.sln -t:warp-gateway -p:Configuration=Release -m:$coreCount -v:m -p:VersionPrefix=$version

if ($LastExitCode -eq 0) {
  Write-Host "Building warp-gateway succeeded!" -ForegroundColor Green
}
else {
  Write-Host "Building warp-gateway failed!" -ForegroundColor Red 
  throw "Building warp-gateway failed!"
}

## Build client
Write-Host 'Building warp-client...' -ForegroundColor Magenta

Push-Location ./warp-client
Push-Location ./dist/warpr-lib
npm link
Pop-Location

Push-Location ./dist/warpr-app
npm link warpr
Pop-Location

npm install
npm run build_custom warpr-app

if ($LastExitCode -eq 0) {
  Write-Host "Building warp-client succeeded!" -ForegroundColor Green
}
else {
  Write-Host "Building warp-client failed!" -ForegroundColor Red 
  throw "Building warp-client failed!"
}
Pop-Location

Write-Host 'Patching nuspec...' -ForegroundColor Magenta
$nuspec = [xml](Get-Content "$PSScriptRoot\warp-client\Warpr.Client.nuspec")

$nuspec.package.metadata.version = $version
$nuspec.package.metadata.repository.branch = $branch
if ($null -ne $commit) {
  $nuspec.package.metadata.repository.SetAttribute("commit", $commit)
}

$nuspec.Save("$PSScriptRoot\warp-client\Warpr.Client.Patched.nuspec")

Write-Host 'Creating nuget package...' -ForegroundColor Magenta
.\Tools\nuget.exe pack .\warp-client\Warpr.Client.Patched.nuspec -OutputDirectory .\Output
Remove-Item -Path '.\warp-client\Warpr.Client.Patched.nuspec'

Write-Host 'Done.' -ForegroundColor Green