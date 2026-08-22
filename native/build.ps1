param(
  [switch]$RunTests,
  [switch]$RunClient,
  [switch]$RunClientScenarios,
  [switch]$RunDensityBench,
  [switch]$RunServerLifecycleSoak
)

$ErrorActionPreference = "Stop"
$nativeRoot = $PSScriptRoot
$buildRoot = Join-Path $nativeRoot "build"
New-Item -ItemType Directory -Force -Path $buildRoot | Out-Null

# Prefer the supported Visual Studio installation query, then retain a
# deterministic fallback for machines where vswhere is unavailable.  Keep the
# probe list so a missing toolchain produces an actionable error instead of a
# cryptic failure from cmd.exe.
$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
$vsInstaller = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer"
$probed = [System.Collections.Generic.List[string]]::new()
$vcvars = $null

if (Test-Path $vswhere) {
  $probed.Add($vswhere)
  # vswhere can print an error when an installation is incomplete; that is
  # harmless here and must not pollute the build output.
  $vsInstall = (& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 2>$null | Select-Object -First 1)
  if ($vsInstall) {
    $vsInstall = $vsInstall.ToString().Trim()
    $vswhereVcvars = Join-Path $vsInstall "VC\Auxiliary\Build\vcvars64.bat"
    $probed.Add($vswhereVcvars)
    if (Test-Path $vswhereVcvars) { $vcvars = $vswhereVcvars }
  }
} else {
  $probed.Add($vswhere)
}

$vsCandidates = @(
  # Preserve the original fallback candidates first.
  "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
  "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)
foreach ($year in @("2017", "2019", "2022")) {
  foreach ($edition in @("BuildTools", "Community", "Professional", "Enterprise")) {
    $vsCandidates += "C:\Program Files (x86)\Microsoft Visual Studio\$year\$edition\VC\Auxiliary\Build\vcvars64.bat"
  }
}

foreach ($candidate in $vsCandidates) {
  if (-not $probed.Contains($candidate)) { $probed.Add($candidate) }
  if (-not $vcvars -and (Test-Path $candidate)) { $vcvars = $candidate }
}

if (-not $vcvars) {
  $probeText = ($probed | ForEach-Object { "  - $_" }) -join [Environment]::NewLine
  throw "MSVC Build Tools vcvars64.bat was not found. Probed:`n$probeText"
}

$include = Join-Path $nativeRoot "include"
$coreSources = @(
  (Join-Path $nativeRoot "src\core.cpp"),
  (Join-Path $nativeRoot "src\seasonal.cpp")
)
$coreObject = Join-Path $buildRoot "core.obj"
$seasonalObject = Join-Path $buildRoot "seasonal.obj"
$networkingSource = Join-Path $nativeRoot "src\networking.cpp"
$networkingObject = Join-Path $buildRoot "networking.obj"
$testExe = Join-Path $buildRoot "verdigris_core_tests.exe"
$networkingTestExe = Join-Path $buildRoot "verdigris_networking_tests.exe"
$sessionTestExe = Join-Path $buildRoot "verdigris_session_tests.exe"
$presentationEventsTestExe = Join-Path $buildRoot "verdigris_presentation_events_tests.exe"
$audioTestExe = Join-Path $buildRoot "verdigris_audio_mixer_tests.exe"
$camera2dTestExe = Join-Path $buildRoot "camera2d_tests.exe"
$serverExe = Join-Path $buildRoot "verdigris_server.exe"
$clientExe = Join-Path $buildRoot "verdigris_client.exe"

function Invoke-Msvc([string]$arguments, [switch]$RequireNativeWindowsDefine) {
  if ($RequireNativeWindowsDefine -and $arguments -notmatch '(?i)(?:^|\s)/DVERDIGRIS_NATIVE_WINDOWS=1(?:\s|$)') {
    throw "Client compile command is missing /DVERDIGRIS_NATIVE_WINDOWS=1. Refusing to build the console fallback."
  }
  $pathSetup = ''
  if (Test-Path $vsInstaller) {
    # vcvars64.bat invokes vswhere internally.  Put the installer directory
    # first in the child cmd's PATH so that lookup is quiet and deterministic.
    $pathSetup = 'set "PATH=' + $vsInstaller + ';%PATH%" && '
  }
  $command = $pathSetup + 'call "' + $vcvars + '" && cl /nologo /std:c++20 /EHsc /W4 /I"' + $include + '" ' + $arguments
  & cmd.exe /d /s /c $command
  if ($LASTEXITCODE -ne 0) { throw "MSVC command failed with exit code $LASTEXITCODE" }
}

Invoke-Msvc ('/c "' + $coreSources[0] + '" /Fo"' + $coreObject + '"')
Invoke-Msvc ('/c "' + $coreSources[1] + '" /Fo"' + $seasonalObject + '"')
Invoke-Msvc ('/c "' + $networkingSource + '" /Fo"' + $networkingObject + '"')
Invoke-Msvc ('/c "' + $nativeRoot + '\tests\core_tests.cpp" /Fo"' + $buildRoot + '\tests.obj"')
Invoke-Msvc ('/c "' + $nativeRoot + '\tests\networking_tests.cpp" /Fo"' + $buildRoot + '\networking_tests.obj"')
Invoke-Msvc ('/c "' + $nativeRoot + '\tests\camera2d_tests.cpp" /Fo"' + $buildRoot + '\camera2d_tests.obj"')
Invoke-Msvc ('/c "' + $nativeRoot + '\client\local_session.cpp" /Fo"' + $buildRoot + '\local_session.obj"')
Invoke-Msvc ('/c "' + $nativeRoot + '\client\remote_session.cpp" /Fo"' + $buildRoot + '\remote_session.obj"')
Invoke-Msvc ('/c "' + $nativeRoot + '\client\presentation_state.cpp" /I"' + $nativeRoot + '\client" /Fo"' + $buildRoot + '\presentation_state.obj"')
Invoke-Msvc ('/c "' + $nativeRoot + '\tests\session_tests.cpp" /I"' + $nativeRoot + '\client" /Fo"' + $buildRoot + '\session_tests.obj"')
# TASK-0122 Phase A: dedicated presentation-events test binary.
Invoke-Msvc ('/c "' + $nativeRoot + '\tests\presentation_events_tests.cpp" /I"' + $nativeRoot + '\client" /Fo"' + $buildRoot + '\presentation_events_tests.obj"')
# TASK-0157 revision: narrow test-target wiring granted by REVIEW correction 1.
# Builds and links the dedicated audio test executable at the SPEC-required
# path so -RunTests alone proves it; no other build behavior changes.
Invoke-Msvc ('/c "' + $nativeRoot + '\audio\cue_spec.cpp" /I"' + $nativeRoot + '\audio" /I"' + $nativeRoot + '\client" /Fo"' + $buildRoot + '\audio_cue_spec.obj"')
Invoke-Msvc ('/c "' + $nativeRoot + '\audio\event_cues.cpp" /I"' + $nativeRoot + '\audio" /I"' + $nativeRoot + '\client" /Fo"' + $buildRoot + '\audio_event_cues.obj"')
Invoke-Msvc ('/c "' + $nativeRoot + '\audio\audio_mixer.cpp" /I"' + $nativeRoot + '\audio" /I"' + $nativeRoot + '\client" /Fo"' + $buildRoot + '\audio_audio_mixer.obj"')
Invoke-Msvc ('/c "' + $nativeRoot + '\tests\audio_mixer_tests.cpp" /I"' + $nativeRoot + '\audio" /I"' + $nativeRoot + '\client" /Fo"' + $buildRoot + '\audio_mixer_tests.obj"')
$serverCompileArguments = '/c "' + $nativeRoot + '\src\server_main.cpp" /Fo"' + $buildRoot + '\server.obj"'
Invoke-Msvc $serverCompileArguments
$clientCompileArguments = '/c "' + $nativeRoot + '\client\main.cpp" /DVERDIGRIS_NATIVE_WINDOWS=1 /I"' + $nativeRoot + '\client" /Fo"' + $buildRoot + '\client.obj"'
# Guard the compile command itself so dropping the define cannot silently
# regress the Windows client into its console fallback.  This is intentionally
# script-side; the native client sources are outside this task's ownership.
Invoke-Msvc $clientCompileArguments -RequireNativeWindowsDefine
$remotePlayCompileArguments = '/c "' + $nativeRoot + '\client\remote_play.cpp" /DVERDIGRIS_NATIVE_WINDOWS=1 /I"' + $nativeRoot + '\client" /Fo"' + $buildRoot + '\remote_play.obj"'
Invoke-Msvc $remotePlayCompileArguments -RequireNativeWindowsDefine
Invoke-Msvc ('"' + $buildRoot + '\tests.obj" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $testExe + '"')
Invoke-Msvc ('"' + $buildRoot + '\networking_tests.obj" "' + $networkingObject + '" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $networkingTestExe + '" /link ws2_32.lib')
Invoke-Msvc ('"' + $buildRoot + '\server.obj" "' + $networkingObject + '" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $serverExe + '" /link ws2_32.lib')
Invoke-Msvc ('"' + $buildRoot + '\client.obj" "' + $buildRoot + '\remote_play.obj" "' + $buildRoot + '\remote_session.obj" "' + $buildRoot + '\local_session.obj" "' + $buildRoot + '\presentation_state.obj" "' + $networkingObject + '" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $clientExe + '" /link user32.lib gdi32.lib ws2_32.lib')

Invoke-Msvc ('"' + $buildRoot + '\camera2d_tests.obj" /Fe"' + $camera2dTestExe + '"')
Invoke-Msvc ('"' + $buildRoot + '\session_tests.obj" "' + $buildRoot + '\local_session.obj" "' + $buildRoot + '\remote_session.obj" "' + $buildRoot + '\presentation_state.obj" "' + $networkingObject + '" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $sessionTestExe + '" /link ws2_32.lib')
Invoke-Msvc ('"' + $buildRoot + '\presentation_events_tests.obj" "' + $buildRoot + '\local_session.obj" "' + $buildRoot + '\remote_session.obj" "' + $buildRoot + '\presentation_state.obj" "' + $networkingObject + '" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $presentationEventsTestExe + '" /link ws2_32.lib')
Invoke-Msvc ('"' + $buildRoot + '\audio_mixer_tests.obj" "' + $buildRoot + '\audio_cue_spec.obj" "' + $buildRoot + '\audio_event_cues.obj" "' + $buildRoot + '\audio_audio_mixer.obj" /Fe"' + $audioTestExe + '"')

python (Join-Path $nativeRoot "tools\check_legacy_denylist.py")
if ($LASTEXITCODE -ne 0) { throw "legacy denylist failed" }
if ($RunTests) { & $testExe }
if ($RunTests) { & $networkingTestExe }
if ($RunTests) { & $camera2dTestExe }
if ($RunTests) { & $sessionTestExe; if ($LASTEXITCODE -ne 0) { throw "session tests failed" } }
if ($RunTests) { & $presentationEventsTestExe; if ($LASTEXITCODE -ne 0) { throw "presentation events tests failed" } }
if ($RunTests) { & $audioTestExe; if ($LASTEXITCODE -ne 0) { throw "audio mixer tests failed" } }
if ($RunClient) { & $clientExe --headless }
if ($RunClientScenarios) { & $clientExe --scenario all }
if ($RunDensityBench) {
  $densityBenchObj = Join-Path $buildRoot "entity_density_bench.obj"
  $densityBenchExe = Join-Path $buildRoot "entity_density_bench.exe"
  Invoke-Msvc ('/c "' + $nativeRoot + '\tools\entity_density_bench.cpp" /Fo"' + $densityBenchObj + '"')
  Invoke-Msvc ('"' + $densityBenchObj + '" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $densityBenchExe + '"')
  $captureDir = Join-Path $nativeRoot "..\orchestration\tasks\TASK-0065-entity-density-benchmark\captures"
  New-Item -ItemType Directory -Force -Path $captureDir | Out-Null
  foreach ($n in @(50, 200, 500, 1000)) {
    foreach ($run in @(1, 2, 3)) {
      $outFile = Join-Path $captureDir ("density-n{0}-run{1}.json" -f $n, $run)
      & $densityBenchExe --n $n --run $run --out $outFile
      if ($LASTEXITCODE -ne 0) { throw "density bench failed n=$n run=$run" }
    }
  }
}
if ($RunServerLifecycleSoak) {
  $soakObj = Join-Path $buildRoot "server_lifecycle_soak.obj"
  $soakExe = Join-Path $buildRoot "server_lifecycle_soak.exe"
  Invoke-Msvc ('/c "' + $nativeRoot + '\tools\server_lifecycle_soak.cpp" /I"' + $nativeRoot + '\client" /Fo"' + $soakObj + '"')
  # Link against the proven session-seam object set so the soak drives the
  # REAL WebSocketServer through the same client transport the tests use.
  Invoke-Msvc ('"' + $soakObj + '" "' + $buildRoot + '\remote_session.obj" "' + $buildRoot + '\local_session.obj" "' + $buildRoot + '\presentation_state.obj" "' + $networkingObject + '" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $soakExe + '" /link ws2_32.lib')
  $soakCaptureDir = Join-Path $nativeRoot "..\orchestration\tasks\TASK-0129-server-lifecycle-soak\captures"
  New-Item -ItemType Directory -Force -Path $soakCaptureDir | Out-Null
  # Timestamped output: each independent gate invocation preserves its own
  # JSON evidence under this task's captures/ folder.
  $soakOutFile = Join-Path $soakCaptureDir ("lifecycle-soak-{0}.json" -f (Get-Date -Format "yyyyMMdd-HHmmss"))
  & $soakExe --out $soakOutFile
  if ($LASTEXITCODE -ne 0) { throw "server lifecycle soak failed with exit code $LASTEXITCODE" }
}
