param(
  [Parameter(Mandatory=$true)][string]$PackageDirectory,
  [Parameter(Mandatory=$true)][string]$EvidenceDirectory
)
$ErrorActionPreference='Stop'
$packageRoot=[IO.Path]::GetFullPath($PackageDirectory)
$evidenceRoot=[IO.Path]::GetFullPath($EvidenceDirectory)
if(!$evidenceRoot.StartsWith($packageRoot.TrimEnd('\')+'\',[StringComparison]::OrdinalIgnoreCase)) {
  throw 'Packaged capture evidence must be inside this package, for example <package>/qa/verification.'
}
New-Item -ItemType Directory -Force -Path $evidenceRoot | Out-Null
# No Computer Use session or foreground window is required. All interactions
# belong to isolated native regression fixtures, never an existing player HWND.
& (Join-Path $PSScriptRoot 'test-player-package.ps1') -PackageDirectory $packageRoot *> (Join-Path $evidenceRoot 'package-check.log')
if($LASTEXITCODE -ne 0){throw 'Package identity/resource verification failed'}
$oldCapture=$env:VERDIGRIS_CAPTURE_ROOT
try {
  $env:VERDIGRIS_CAPTURE_ROOT=$evidenceRoot
  $process=Start-Process -FilePath (Join-Path $packageRoot 'native/build/verdigris_client.exe') -ArgumentList @('--scenario','all') -WorkingDirectory $packageRoot -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $evidenceRoot 'scenarios.log') -RedirectStandardError (Join-Path $evidenceRoot 'scenarios-stderr.log')
  if(!$process.WaitForExit(900000)) {
    $process.Kill();$process.WaitForExit()
    throw 'Packaged scenarios timed out; retained logs identify the last completed scenario'
  }
  if($process.ExitCode -ne 0){throw "Packaged scenarios failed ($($process.ExitCode)); see $evidenceRoot/scenarios.log"}
} finally {
  if($null -eq $oldCapture){Remove-Item Env:VERDIGRIS_CAPTURE_ROOT -ErrorAction SilentlyContinue}
  else {$env:VERDIGRIS_CAPTURE_ROOT=$oldCapture}
}
& (Join-Path $PSScriptRoot 'test-launcher-lifecycle.ps1') -PackageDirectory $packageRoot -EvidenceDirectory $evidenceRoot *> (Join-Path $evidenceRoot 'launcher-check.log')
$manifest=Get-Content -LiteralPath (Join-Path $packageRoot 'package-manifest.json') -Raw | ConvertFrom-Json
Write-Output "PASS automated packaged verification: source=$($manifest.sourceCommit), all native scenarios, two launcher lifecycles, same-profile settings restart. Evidence=$evidenceRoot"
Write-Output 'Inspect the retained production captures separately for visual quality; automated success is not owner acceptance.'
