param(
  [switch]$Local,
  [switch]$Rebuild,
  [int]$Port = 0
)

$ErrorActionPreference = "Stop"
$nativeRoot = Split-Path $PSScriptRoot -Parent
$buildRoot = Join-Path $nativeRoot "build"
$logDir = Join-Path $buildRoot "logs"
$serverExe = Join-Path $buildRoot "verdigris_server.exe"
$clientExe = Join-Path $buildRoot "verdigris_client.exe"
$buildScript = Join-Path $nativeRoot "build.ps1"

if ($Port -eq 6500) {
  throw "play-native: port 6500 is reserved for the historical browser server. Use 6520-6539."
}

function Test-PortFree([int]$candidate) {
  try {
    $listener = [System.Net.Sockets.TcpListener]::new(
      [System.Net.IPAddress]::Loopback, $candidate)
    $listener.Start()
    $listener.Stop()
    return $true
  } catch {
    return $false
  }
}

function Find-OwnerPlayPort {
  foreach ($candidate in 6520..6539) {
    if (Test-PortFree $candidate) { return $candidate }
  }
  throw "play-native: no free owner-play port in 6520-6539"
}

function Test-ExeStale([string]$exe) {
  if (-not (Test-Path $exe)) { return $true }
  $exeTime = (Get-Item $exe).LastWriteTimeUtc
  $roots = @(
    (Join-Path $nativeRoot "src"),
    (Join-Path $nativeRoot "client"),
    (Join-Path $nativeRoot "include"),
    (Join-Path $nativeRoot "build.ps1")
  )
  $newest = Get-ChildItem -Path $roots -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -in ".cpp", ".hpp", ".h", ".ps1" } |
    Sort-Object LastWriteTimeUtc -Descending |
    Select-Object -First 1
  if (-not $newest) { return $false }
  return $newest.LastWriteTimeUtc -gt $exeTime
}

function Invoke-NativeBuild {
  Write-Host "play-native: building via native/build.ps1"
  & powershell.exe -NoProfile -File $buildScript
  if ($LASTEXITCODE -ne 0) { throw "play-native: native/build.ps1 failed with exit $LASTEXITCODE" }
}

if ($Rebuild -or (Test-ExeStale $clientExe) -or (-not $Local -and (Test-ExeStale $serverExe))) {
  Invoke-NativeBuild
}

if (-not (Test-Path $clientExe)) { throw "play-native: missing $clientExe" }
if (-not $Local -and -not (Test-Path $serverExe)) { throw "play-native: missing $serverExe" }

New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$serverLog = Join-Path $logDir "server-$stamp.log"
$serverErr = Join-Path $logDir "server-$stamp.err.log"

$server = $null
$chosenPort = $Port
try {
  if (-not $Local) {
    if ($chosenPort -le 0) { $chosenPort = Find-OwnerPlayPort }
    if ($chosenPort -eq 6500) {
      throw "play-native: port 6500 is reserved for the historical browser server."
    }
    Write-Host "play-native: starting server ws://127.0.0.1:$chosenPort"
    Write-Host "play-native: server log $serverLog"
    $server = Start-Process -FilePath $serverExe -ArgumentList "$chosenPort" -PassThru `
      -WindowStyle Hidden `
      -RedirectStandardOutput $serverLog `
      -RedirectStandardError $serverErr
    $deadline = (Get-Date).AddSeconds(12)
    $ready = $false
    while ((Get-Date) -lt $deadline) {
      if ($server.HasExited) { break }
      if (Test-Path $serverLog) {
        $text = Get-Content -Path $serverLog -Raw -ErrorAction SilentlyContinue
        if ($text -match "listening") { $ready = $true; break }
      }
      Start-Sleep -Milliseconds 150
    }
    if (-not $ready) {
      throw "play-native: server did not print a listening line (see $serverLog)"
    }
    Write-Host "play-native: starting client --remote 127.0.0.1 $chosenPort"
    Write-Host "play-native: close the window or press Esc to quit"
    & $clientExe --remote "127.0.0.1" $chosenPort
  } else {
    Write-Host "play-native: starting local simulation client"
    Write-Host "play-native: close the window or press Esc to quit"
    & $clientExe
  }
} finally {
  if ($server -and -not $server.HasExited) {
    Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue
    try { Wait-Process -Id $server.Id -Timeout 5 -ErrorAction SilentlyContinue } catch {}
  }
}

$orphans = @(Get-Process -Name "verdigris_server", "verdigris_client" -ErrorAction SilentlyContinue)
if ($orphans.Count -gt 0) {
  $ids = ($orphans | ForEach-Object { "$($_.ProcessName):$($_.Id)" }) -join ", "
  Write-Host "play-native: ORPHAN PROCESSES STILL RUNNING: $ids"
  Write-Host "play-native: server log $serverLog"
  exit 1
}

Write-Host "play-native: no orphan verdigris_server/client processes"
if (-not $Local) { Write-Host "play-native: server log $serverLog" }
exit 0
