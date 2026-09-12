param(
  # Keep native scenario evidence out of source-controlled historical capture
  # folders by default. The build script validates that the path is contained
  # by this checkout before writing anything.
  [string]$CaptureRoot = 'native/build/native-acceptance'
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$buildScript = Join-Path $repoRoot 'native\build.ps1'

$buildArgs = @{
  RunTests = $true
  RunClientScenarios = $true
  CaptureRoot = $CaptureRoot
}
& $buildScript @buildArgs
if ($LASTEXITCODE -ne 0) {
  throw "Native acceptance failed with exit code $LASTEXITCODE."
}
Write-Output 'Native acceptance passed: build, native tests, denylist, and client scenario suite.'
