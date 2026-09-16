# GATEB-FLAKE repro harness (not part of the gate).
#
# Rebuilds and repeatedly runs the gate-b-only repro binary
# (native/tools/gateb_flake_repro.cpp, which includes the frozen
# session_tests.cpp unchanged) and optionally puts the machine under CPU load
# to reproduce the FULL-GATE contention conditions.
#
# Usage (from the repository root):
#   powershell -NoProfile -ExecutionPolicy Bypass \
#     -File orchestration/vg/GATEB-FLAKE/repro.ps1 [-Runs 3] [-LoadWorkers 6] [-Tag baseline]
#
# Output: one log per run under orchestration/vg/GATEB-FLAKE/evidence/ plus an
# appended line per run in evidence/results-<Tag>.tsv.
param(
  [int]$Runs = 3,
  [int]$LoadWorkers = 0,
  [string]$Tag = "run",
  [string]$Binary = "gateb_flake_repro.exe",
  [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\..\.."))
$nativeRoot = Join-Path $repoRoot "native"
$buildRoot = Join-Path $nativeRoot "build"
$evidence = Join-Path $PSScriptRoot "evidence"
New-Item -ItemType Directory -Force -Path $evidence | Out-Null

$reproExe = Join-Path $buildRoot $Binary

if (-not $SkipBuild) {
  # Compile+link through a batch helper so vcvars + cl quoting stays in cmd.
  $bat = Join-Path $PSScriptRoot "build-repro.bat"
  if ($Binary -eq "gateb_flake_repro_patched.exe") {
    $bat = Join-Path $PSScriptRoot "build-repro-patched.bat"
  }
  Push-Location $nativeRoot
  try {
    & cmd.exe /c $bat
    if ($LASTEXITCODE -ne 0) { throw "repro build failed ($LASTEXITCODE)" }
  } finally {
    Pop-Location
  }
  Write-Host "repro binary: $reproExe"
}

$jobs = @()
if ($LoadWorkers -gt 0) {
  Write-Host "starting $LoadWorkers CPU spin workers"
  for ($i = 0; $i -lt $LoadWorkers; ++$i) {
    $jobs += Start-Job -ScriptBlock { while ($true) { $x = 1 * 1 } }
  }
}

$resultsFile = Join-Path $evidence "results-$Tag.tsv"
if (-not (Test-Path $resultsFile)) {
  "run`texit`telapsed_s`tfail_signature`tlog" | Out-File -Encoding utf8 $resultsFile
}

try {
  for ($run = 1; $run -le $Runs; ++$run) {
    $log = Join-Path $evidence ("{0}-run{1}.log" -f $Tag, $run)
    $started = Get-Date
    & $reproExe > $log 2>&1
    $exit = $LASTEXITCODE
    $elapsed = [int]((Get-Date) - $started).TotalSeconds
    $signature = 0
    if (Select-String -Path $log -Pattern "hunt aborted - the successor fell to ordinary combat" -Quiet) {
      $signature = 1
    }
    "$run`t$exit`t$elapsed`t$signature`t$log" | Out-File -Encoding utf8 -Append $resultsFile
    Write-Host ("run {0}: exit={1} elapsed={2}s flake_signature={3}" -f $run, $exit, $elapsed, $signature)
  }
} finally {
  foreach ($job in $jobs) { Stop-Job $job; Remove-Job $job -Force }
}
