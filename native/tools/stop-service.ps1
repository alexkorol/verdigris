param([Parameter(Mandatory = $true)][string]$DataDirectory)
$ErrorActionPreference = 'Stop'
$server = Join-Path $PSScriptRoot 'verdigris_server.exe'
if (-not (Test-Path -LiteralPath $server)) { $server = Join-Path $PSScriptRoot '../build/verdigris_server.exe' }
& $server --stop --data ([IO.Path]::GetFullPath($DataDirectory))
if ($LASTEXITCODE -ne 0) { throw 'Graceful shutdown request failed.' }
Write-Output 'Wait for the service process to exit before backup, enrollment or restore operations.'
