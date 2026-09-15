# Native CI uses the MSVC developer environment supplied by the workflow.
# CTest owns all native test targets, including the full client scenario suite;
# do not run that suite or the camera tests a second time after CTest.
param([switch]$SoftwareRenderer)
$ErrorActionPreference = "Stop"
$nativeRoot = Split-Path $PSScriptRoot -Parent
Set-Location $nativeRoot
$logDir = Join-Path $nativeRoot "build\ci-logs"
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$transcript = Join-Path $logDir "native-gates.log"

function Invoke-Logged([string]$label, [scriptblock]$body) {
  Write-Host "ci-native: $label"
  & $body
  if ($LASTEXITCODE -ne 0) {
    throw "ci-native: $label failed with exit $LASTEXITCODE"
  }
}

Start-Transcript -Path $transcript -Force | Out-Null
try {
  $softwareCi = if ($SoftwareRenderer) { 'ON' } else { 'OFF' }
  Invoke-Logged "cmake configure" { cmake --preset windows-msvc "-DVERDIGRIS_SOFTWARE_RENDERER_CI=$softwareCi" }
  Invoke-Logged "cmake build" { cmake --build --preset windows-msvc }
  Invoke-Logged "legacy denylist" { python tools/check_legacy_denylist.py }
  $previousCaptureRoot = $env:VERDIGRIS_CAPTURE_ROOT
  try {
    $env:VERDIGRIS_CAPTURE_ROOT = Join-Path $logDir "captures"
    New-Item -ItemType Directory -Force -Path $env:VERDIGRIS_CAPTURE_ROOT | Out-Null
    # Verbose output retains explicit DEFERRED timing records on successful
    # functional runs instead of hiding them behind a generic CTest PASS.
    Invoke-Logged "ctest (all native tests and client scenarios)" { ctest --preset windows-msvc --output-on-failure --verbose }
  } finally {
    if ($null -eq $previousCaptureRoot) { Remove-Item Env:VERDIGRIS_CAPTURE_ROOT -ErrorAction SilentlyContinue }
    else { $env:VERDIGRIS_CAPTURE_ROOT = $previousCaptureRoot }
  }

  Write-Host "ci-native: PASS"
} finally {
  Stop-Transcript | Out-Null
}
