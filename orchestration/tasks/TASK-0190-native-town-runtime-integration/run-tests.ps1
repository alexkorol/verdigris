# TASK-0190 layout prep acceptance harness.
param()

$ErrorActionPreference = "Stop"
$taskDir = $PSScriptRoot
$root = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $taskDir))
$clientInclude = Join-Path $root "native\client"
$buildDir = Join-Path $taskDir "build"
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
$vcvars = $null
if (Test-Path $vswhere) {
  $vsInstall = (& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null | Select-Object -First 1)
  if ($vsInstall) {
    $c = Join-Path ($vsInstall.ToString().Trim()) "VC\Auxiliary\Build\vcvars64.bat"
    if (Test-Path $c) { $vcvars = $c }
  }
}
if (-not $vcvars) {
  $vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
}
if (-not (Test-Path $vcvars)) { throw "MSVC not found" }

$testSource = Join-Path $taskDir "town_runtime_layout_tests.cpp"
$testObject = Join-Path $buildDir "town_runtime_layout_tests.obj"
$testExe = Join-Path $buildDir "town_runtime_layout_tests.exe"

$compile = 'call "' + $vcvars + '" && cl /nologo /std:c++20 /EHsc /W4 /I"' + $clientInclude + '" /c "' + $testSource + '" /Fo"' + $testObject + '"'
& cmd.exe /d /s /c $compile
if ($LASTEXITCODE -ne 0) { throw "compile failed" }

$link = 'call "' + $vcvars + '" && cl /nologo "' + $testObject + '" /Fe"' + $testExe + '"'
& cmd.exe /d /s /c $link
if ($LASTEXITCODE -ne 0) { throw "link failed" }

& $testExe
if ($LASTEXITCODE -ne 0) { throw "tests failed" }
Write-Host "TASK-0190 town_runtime_layout planner: PASS"
