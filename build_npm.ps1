Write-Host 'Collecting build version information...' -ForegroundColor Magenta
$version = if ($null -ne $env:APPVEYOR_BUILD_VERSION) { $env:APPVEYOR_BUILD_VERSION } else { "1.0.0.0" }
$version = [Version]::Parse($version).ToString(3)
Write-Host "Version: $version"

Write-Host 'Updating package.json...' -ForegroundColor Magenta
Push-Location ./warp-client

$packageJson = Get-Content ./projects/warpr-lib/package.json | ConvertFrom-Json
$packageJson.version = $version
Set-Content ./projects/warpr-lib/package.json -Value ($packageJson | ConvertTo-Json)

Write-Host 'Installing dependencies...' -ForegroundColor Magenta
npm install

Write-Host 'Building warpr-lib...' -ForegroundColor Magenta
npm run build_custom warpr-lib

if ($LastExitCode -eq 0) {
  Write-Host "Building warpr-lib succeeded!" -ForegroundColor Green
}
else {
  Write-Host "Building warpr-lib failed!" -ForegroundColor Red 
  throw "Building warpr-lib failed!"
}

Write-Host 'Creating npm package...' -ForegroundColor Magenta
Push-Location ./projects/warpr-lib
npm pack

if ($env:APPVEYOR) {
  Write-Host 'Publishing npm package...' -ForegroundColor Magenta
  npm config set //registry.npmjs.org/:_authToken=$env:NPM_TOKEN
  npm publish
}

Pop-Location
Pop-Location

Write-Host 'Done.' -ForegroundColor Green