# TASK-0180 acceptance harness.
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
    $candidate = Join-Path $vsInstall.ToString().Trim() "VC\Auxiliary\Build\vcvars64.bat"
    if (Test-Path $candidate) { $vcvars = $candidate }
  }
}
foreach ($candidate in @(
  "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)) {
  if (-not $vcvars -and (Test-Path $candidate)) { $vcvars = $candidate }
}
if (-not $vcvars) { throw "MSVC vcvars64.bat not found" }

$testSource = Join-Path $taskDir "framekit_renderer_tests.cpp"
$testObject = Join-Path $buildDir "framekit_renderer_tests.obj"
$testExe = Join-Path $buildDir "framekit_renderer_tests.exe"

$compileCommand = 'call "' + $vcvars + '" && cl /nologo /std:c++20 /EHsc /W4 /I"' + $clientInclude + '" /c "' + $testSource + '" /Fo"' + $testObject + '"'
& cmd.exe /d /s /c $compileCommand
if ($LASTEXITCODE -ne 0) { throw "compile failed $LASTEXITCODE" }

$linkCommand = 'call "' + $vcvars + '" && cl /nologo "' + $testObject + '" /Fe"' + $testExe + '"'
& cmd.exe /d /s /c $linkCommand
if ($LASTEXITCODE -ne 0) { throw "link failed $LASTEXITCODE" }

& $testExe
if ($LASTEXITCODE -ne 0) { throw "tests failed $LASTEXITCODE" }

python (Join-Path $root "native\tools\check_legacy_denylist.py")
if ($LASTEXITCODE -ne 0) { throw "denylist failed $LASTEXITCODE" }

Write-Host "TASK-0180 framekit render adapter harness: PASS"
