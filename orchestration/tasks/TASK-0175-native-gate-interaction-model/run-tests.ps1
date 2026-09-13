# TASK-0175 acceptance harness.
param()

$ErrorActionPreference = "Stop"
$taskDir = $PSScriptRoot
$root = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $taskDir))
$clientInclude = Join-Path $root "native\client"
$buildDir = Join-Path $taskDir "build"
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvars)) { throw "MSVC not found" }

$testSource = Join-Path $taskDir "gate_interaction_tests.cpp"
$testObject = Join-Path $buildDir "gate_interaction_tests.obj"
$testExe = Join-Path $buildDir "gate_interaction_tests.exe"

$compile = 'call "' + $vcvars + '" && cl /nologo /std:c++20 /EHsc /W4 /I"' + $clientInclude + '" /c "' + $testSource + '" /Fo"' + $testObject + '"'
& cmd.exe /d /s /c $compile
if ($LASTEXITCODE -ne 0) { throw "compile failed" }

$link = 'call "' + $vcvars + '" && cl /nologo "' + $testObject + '" /Fe"' + $testExe + '"'
& cmd.exe /d /s /c $link
if ($LASTEXITCODE -ne 0) { throw "link failed" }

& $testExe
if ($LASTEXITCODE -ne 0) { throw "tests failed" }
Write-Host "TASK-0175 gate interaction acceptance harness: PASS"
