# TASK-0173 acceptance harness.
#
# Pass requires ALL of:
#   1. normal test build runs green (all checks pass, exit 0)
#   2. the /DNEGATIVE_CONTROL build MUST fail (exit 1): it asserts the
#      inverted zero-duration-attack expectation, proving the suite detects
#      an invisible-zero-length-attack regression rather than passing vacuously.
param()

$ErrorActionPreference = "Stop"
$taskDir = $PSScriptRoot
$root = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $taskDir))
$clientInclude = Join-Path $root "native\client"
$buildDir = Join-Path $taskDir "build"
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
$vsInstaller = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer"
$probed = [System.Collections.Generic.List[string]]::new()
$vcvars = $null

if (Test-Path $vswhere) {
  $probed.Add($vswhere)
  $vsInstall = (& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null | Select-Object -First 1)
  if ($vsInstall) {
    $vsInstall = $vsInstall.ToString().Trim()
    $vswhereVcvars = Join-Path $vsInstall "VC\Auxiliary\Build\vcvars64.bat"
    $probed.Add($vswhereVcvars)
    if ($vswhereVcvars -and (Test-Path $vswhereVcvars)) { $vcvars = $vswhereVcvars }
  }
} else {
  $probed.Add($vswhere)
}

foreach ($candidate in @(
  "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
  "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)) {
  if (-not $probed.Contains($candidate)) { $probed.Add($candidate) }
  if (-not $vcvars -and (Test-Path $candidate)) { $vcvars = $candidate }
}
foreach ($year in @("2022", "2019", "2017")) {
  foreach ($edition in @("BuildTools", "Community", "Professional", "Enterprise")) {
    $candidate = "C:\Program Files (x86)\Microsoft Visual Studio\$year\$edition\VC\Auxiliary\Build\vcvars64.bat"
    if (-not $probed.Contains($candidate)) { $probed.Add($candidate) }
    if (-not $vcvars -and (Test-Path $candidate)) { $vcvars = $candidate }
  }
}

if (-not $vcvars) {
  $probeText = ($probed | ForEach-Object { "  - $_" }) -join [Environment]::NewLine
  throw "MSVC Build Tools vcvars64.bat was not found. Probed:`n$probeText"
}

$testSource = Join-Path $taskDir "actor_animation_tests.cpp"
$pathSetup = ''
if (Test-Path $vsInstaller) {
  $pathSetup = 'set "PATH=' + $vsInstaller + ';%PATH%" && '
}

function Invoke-MsvcStep {
  param(
    [string]$Label,
    [string]$Command,
    [int[]]$AcceptExitCodes = @(0)
  )
  & cmd.exe /d /s /c $Command
  if ($AcceptExitCodes -notcontains $LASTEXITCODE) {
    throw "$Label failed with exit code $LASTEXITCODE"
  }
}

function Build-And-Run {
  param([string]$Suffix, [string]$Defines, [int[]]$RunAccept)

  $object = Join-Path $buildDir "actor_animation_tests_$Suffix.obj"
  $exe = Join-Path $buildDir "actor_animation_tests_$Suffix.exe"

  $compile = $pathSetup + 'call "' + $vcvars + '" && cl /nologo /std:c++20 /EHsc /W4 ' +
             $Defines + ' /I"' + $clientInclude + '" /c "' + $testSource +
             '" /Fo"' + $object + '"'
  Invoke-MsvcStep "compiling actor animation tests ($Suffix)" $compile

  $link = $pathSetup + 'call "' + $vcvars + '" && cl /nologo "' + $object +
          '" /Fe"' + $exe + '"'
  Invoke-MsvcStep "linking actor animation tests ($Suffix)" $link

  Invoke-MsvcStep "running actor animation tests ($Suffix)" ('"' + $exe + '"') $RunAccept
}

Build-And-Run "acceptance" "" @(0)
Write-Host "TASK-0173 acceptance build: all checks PASS"

Build-And-Run "negative" "/DNEGATIVE_CONTROL" @(1)
Write-Host "TASK-0173 negative control: inverted assertion failed as required (exit 1)"

Write-Host "TASK-0173 actor animation acceptance harness: PASS"
