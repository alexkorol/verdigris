# Native CI helper (TASK-0067).
# GitHub windows-latest ships VS 2022 under "Program Files", which native/build.ps1
# does not probe (it looks under Program Files (x86) / vswhere). This script uses
# the MSVC developer PATH from ilammy/msvc-dev-cmd and the CMake preset the
# previous native workflow already ran, then adds the owner-path steps CMake
# omitted: denylist, camera2d tests, local --scenario all. Session tests (remote
# journey + clean shutdown) are part of ctest. Density bench is not invoked.
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
  Invoke-Logged "cmake configure" { cmake --preset windows-msvc }
  Invoke-Logged "cmake build" { cmake --build --preset windows-msvc }
  Invoke-Logged "legacy denylist" { python tools/check_legacy_denylist.py }
  Invoke-Logged "ctest (core/networking/session)" { ctest --preset windows-msvc --output-on-failure }

  $camObj = Join-Path $logDir "camera2d_tests.obj"
  $camExe = Join-Path $logDir "camera2d_tests.exe"
  Invoke-Logged "camera2d compile" {
    cl /nologo /std:c++20 /EHsc /W4 /I"$nativeRoot\include" `
      "$nativeRoot\tests\camera2d_tests.cpp" /Fo"$camObj" /Fe"$camExe"
  }
  Invoke-Logged "camera2d tests" { & $camExe }

  $clientExe = Join-Path $nativeRoot "build\cmake\windows-msvc\verdigris_client.exe"
  if (-not (Test-Path $clientExe)) {
    throw "ci-native: missing $clientExe"
  }
  Invoke-Logged "local client scenarios" { & $clientExe --scenario all }

  Write-Host "ci-native: PASS"
} finally {
  Stop-Transcript | Out-Null
}
