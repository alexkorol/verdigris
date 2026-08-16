param(
  [switch]$RunTests,
  [switch]$RunClient
)

$ErrorActionPreference = "Stop"
$nativeRoot = $PSScriptRoot
$buildRoot = Join-Path $nativeRoot "build"
New-Item -ItemType Directory -Force -Path $buildRoot | Out-Null

$vsCandidates = @(
  "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
  "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)
$vcvars = $vsCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $vcvars) { throw "MSVC Build Tools vcvars64.bat was not found." }

$include = Join-Path $nativeRoot "include"
$coreSources = @(
  (Join-Path $nativeRoot "src\core.cpp"),
  (Join-Path $nativeRoot "src\seasonal.cpp")
)
$coreObject = Join-Path $buildRoot "core.obj"
$seasonalObject = Join-Path $buildRoot "seasonal.obj"
$testExe = Join-Path $buildRoot "verdigris_core_tests.exe"
$clientExe = Join-Path $buildRoot "verdigris_client.exe"

function Invoke-Msvc([string]$arguments) {
  $command = 'call "' + $vcvars + '" && cl /nologo /std:c++20 /EHsc /W4 /I"' + $include + '" ' + $arguments
  & cmd.exe /d /s /c $command
  if ($LASTEXITCODE -ne 0) { throw "MSVC command failed with exit code $LASTEXITCODE" }
}

Invoke-Msvc ('/c "' + $coreSources[0] + '" /Fo"' + $coreObject + '"')
Invoke-Msvc ('/c "' + $coreSources[1] + '" /Fo"' + $seasonalObject + '"')
Invoke-Msvc ('/c "' + $nativeRoot + '\tests\core_tests.cpp" /Fo"' + $buildRoot + '\tests.obj"')
Invoke-Msvc ('/c "' + $nativeRoot + '\client\main.cpp" /Fo"' + $buildRoot + '\client.obj"')
Invoke-Msvc ('"' + $buildRoot + '\tests.obj" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $testExe + '"')
Invoke-Msvc ('"' + $buildRoot + '\client.obj" "' + $coreObject + '" "' + $seasonalObject + '" /Fe"' + $clientExe + '" /link user32.lib gdi32.lib')

python (Join-Path $nativeRoot "tools\check_legacy_denylist.py")
if ($RunTests) { & $testExe }
if ($RunClient) { & $clientExe --headless }
