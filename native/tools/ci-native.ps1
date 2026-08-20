# Native CI helper (TASK-0067). Owner path: build.ps1 -RunTests -RunClientScenarios.
# Does not invoke -RunDensityBench. Transcripts land in native/build/ci-logs/.
$ErrorActionPreference = "Stop"
$nativeRoot = Split-Path $PSScriptRoot -Parent
$logDir = Join-Path $nativeRoot "build\ci-logs"
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$transcript = Join-Path $logDir "native-gates.log"
$buildScript = Join-Path $nativeRoot "build.ps1"

Write-Host "ci-native: $buildScript -RunTests -RunClientScenarios"
Start-Transcript -Path $transcript -Force | Out-Null
$code = 0
try {
  & powershell -NoProfile -File $buildScript -RunTests -RunClientScenarios
  $code = $LASTEXITCODE
} finally {
  Stop-Transcript | Out-Null
}
if ($code -ne 0) {
  throw "ci-native: native gates failed with exit $code (see $transcript)"
}
Write-Host "ci-native: PASS"
