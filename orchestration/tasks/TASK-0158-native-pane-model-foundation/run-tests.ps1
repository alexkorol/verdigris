# TASK-0158 acceptance harness: compiles the production pane_model.hpp with the
# installed MSVC environment against the self-contained test source in this
# task folder, runs it, and propagates the verdict as the exit code.
# All outputs (obj/exe/logs) stay inside this task folder.

$ErrorActionPreference = 'Stop'

$taskDir = $PSScriptRoot
if (-not $taskDir) {
  throw "run-tests.ps1 must be executed from its task folder."
}
$repoRoot = (Resolve-Path (Join-Path $taskDir '..\..\..')).Path
$clientDir = Join-Path $repoRoot 'native\client'
$testSource = Join-Path $taskDir 'pane_model_tests.cpp'
$objectFile = Join-Path $taskDir 'pane_model_tests.obj'
$testExe = Join-Path $taskDir 'pane_model_tests.exe'
$logFile = Join-Path $taskDir 'test-run.log'

foreach ($required in @($testSource)) {
  if (-not (Test-Path $required)) { throw "Missing required file: $required" }
}

# Locate vcvars64.bat the same way native/build.ps1 does: vswhere first, then
# deterministic fallback candidates, so a missing toolchain yields an
# actionable probe list instead of a cryptic cmd.exe failure.
$programFilesX86 = ${env:ProgramFiles(x86)}
$vsInstaller = Join-Path $programFilesX86 "Microsoft Visual Studio\Installer"
$vswhere = Join-Path $vsInstaller "vswhere.exe"
$probed = [System.Collections.Generic.List[string]]::new()
$vcvars = $null

if (Test-Path $vswhere) {
  $probed.Add($vswhere)
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

$pathSetup = ''
if (Test-Path $vsInstaller) {
  # Keep vswhere quiet inside vcvars64.bat, matching native/build.ps1.
  $pathSetup = 'set "PATH=' + $vsInstaller + ';%PATH%" && '
}
$compileCommand = $pathSetup + 'call "' + $vcvars + '" && cl /nologo /std:c++20 /EHsc /W4 /I"' +
  $clientDir + '" "' + $testSource + '" /Fo"' + $objectFile + '" /Fe"' + $testExe + '"'

Write-Host "== TASK-0158 pane model tests =="
Write-Host "vcvars: $vcvars"

$transcript = & cmd.exe /d /s /c "$compileCommand && `"$testExe`"" 2>&1
$exitCode = $LASTEXITCODE
$transcript | Set-Content -Path $logFile
$transcript | ForEach-Object { Write-Host $_ }

if ($exitCode -ne 0) {
  Write-Error "TASK-0158 harness failed with exit code $exitCode (log: $logFile)"
  exit $exitCode
}

Remove-Item -Path $objectFile -ErrorAction SilentlyContinue
Write-Host "pane_model tests: PASS (harness exit 0; log: $logFile)"
exit 0
