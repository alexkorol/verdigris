param(
  [ValidateRange(6580, 6599)]
  [int]$Port = 6580
)

$ErrorActionPreference = "Stop"
$nativeBuild = Join-Path $PSScriptRoot "..\..\..\native\build"
$serverExe = Join-Path $nativeBuild "verdigris_server.exe"
$clientExe = Join-Path $nativeBuild "verdigris_client.exe"

if (-not (Test-Path $serverExe) -or -not (Test-Path $clientExe)) {
  throw "Build the native tree first: powershell -File native/build.ps1"
}

Write-Host "Starting verdigris_server on ws://127.0.0.1:$Port"
$server = Start-Process -FilePath $serverExe -ArgumentList "$Port" -PassThru -WindowStyle Hidden
Start-Sleep -Seconds 1
try {
  Write-Host "Starting verdigris_client --remote 127.0.0.1 $Port"
  & $clientExe --remote 127.0.0.1 $Port
} finally {
  if ($server -and -not $server.HasExited) {
    Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue
  }
}
