param(
  [switch]$RunTests,
  [switch]$RunClient
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
$testExe = Join-Path $buildRoot "verdigris_core_tests.exe"
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
Invoke-Msvc ('/c "' + $nativeRoot + '\tests\core_tests.cpp" /Fo"' + $buildRoot + '\tests.obj"')
$clientCompileArguments = '/c "' + $nativeRoot + '\client\main.cpp" /DVERDIGRIS_NATIVE_WINDOWS=1 /Fo"' + $buildRoot + '\client.obj"'
# Guard the compile command itself so dropping the define cannot silently
# regress the Windows client into its console fallback.  This is intentionally
# script-side; the native client sources are outside this task's ownership.
Invoke-Msvc $clientCompileArguments -RequireNativeWindowsDefine
Invoke-Msvc ('"' + $buildRoot + '\tests.obj" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $testExe + '"')
Invoke-Msvc ('"' + $buildRoot + '\client.obj" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $clientExe + '" /link user32.lib gdi32.lib')

python (Join-Path $nativeRoot "tools\check_legacy_denylist.py")
if ($RunTests) { & $testExe }
if ($RunClient) { & $clientExe --headless }
