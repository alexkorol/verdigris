# TASK-0182 acceptance harness.
param()

$ErrorActionPreference = "Stop"
$taskDir = $PSScriptRoot
$root = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $taskDir))
$clientInclude = Join-Path $root "native\client"
$buildDir = Join-Path $taskDir "build"
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvars)) { throw "MSVC not found" }
$testSource = Join-Path $taskDir "item_art_renderer_tests.cpp"
$testObject = Join-Path $buildDir "item_art_renderer_tests.obj"
$testExe = Join-Path $buildDir "item_art_renderer_tests.exe"
$compile = 'call "' + $vcvars + '" && cl /nologo /std:c++20 /EHsc /W4 /I"' + $clientInclude + '" /c "' + $testSource + '" /Fo"' + $testObject + '"'
& cmd.exe /d /s /c $compile
if ($LASTEXITCODE -ne 0) { throw "compile failed" }
$link = 'call "' + $vcvars + '" && cl /nologo "' + $testObject + '" /Fe"' + $testExe + '"'
& cmd.exe /d /s /c $link
if ($LASTEXITCODE -ne 0) { throw "link failed" }
# The drift guard reads the ACTUAL shipped manifest at test time.
$manifest = Join-Path $root "native\client\assets\wizard\items\manifest.json"
if (-not (Test-Path $manifest)) { throw "items manifest not found: $manifest" }
& $testExe $manifest
if ($LASTEXITCODE -ne 0) { throw "tests failed" }
python (Join-Path $root "native\tools\check_legacy_denylist.py")
if ($LASTEXITCODE -ne 0) { throw "denylist failed" }
Write-Host "TASK-0182 item art render adapter harness: PASS"
