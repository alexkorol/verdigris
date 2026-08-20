# Native CI helper (TASK-0067).
# GitHub windows-latest ships VS 18 under Program Files; native/build.ps1 does
# not probe that path. This script uses the MSVC developer PATH from
# ilammy/msvc-dev-cmd and the CMake preset the previous native workflow already
# ran, then adds the owner-path steps CMake omitted: denylist, camera2d tests,
# local --scenario all. Session tests (remote journey + clean shutdown) are
# part of ctest. Density bench is not invoked.
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

  # Match native/build.ps1 (cl via cmd so /std:c++20 is not eaten by pwsh).
  # windows-latest is VS 18; range-for over a braced list needs <initializer_list>
  # and the test file does not include it. /FI avoids editing unowned tests.
  $camObj = Join-Path $logDir "camera2d_tests.obj"
  $camExe = Join-Path $logDir "camera2d_tests.exe"
  $camSrc = Join-Path $nativeRoot "tests\camera2d_tests.cpp"
  $camInc = Join-Path $nativeRoot "include"
  Invoke-Logged "camera2d compile" {
    $compile = 'cl /nologo /std:c++20 /EHsc /W4 /FIinitializer_list /I"' + $camInc + '" /c "' + $camSrc + '" /Fo"' + $camObj + '"'
    Write-Host "ci-native: $compile"
    & cmd.exe /d /s /c $compile
  }
  Invoke-Logged "camera2d link" {
    $link = 'cl /nologo "' + $camObj + '" /Fe"' + $camExe + '"'
    Write-Host "ci-native: $link"
    & cmd.exe /d /s /c $link
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
