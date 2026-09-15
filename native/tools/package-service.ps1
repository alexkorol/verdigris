param([Parameter(Mandatory = $true)][string]$OutputDirectory)
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$destination = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $destination) { throw 'Service package destination must be a fresh directory.' }
$commit = (& git -C $repoRoot rev-parse HEAD).Trim()
$tree = (& git -C $repoRoot rev-parse 'HEAD^{tree}').Trim()
function Assert-CleanSource {
  if (@(& git -C $repoRoot status --porcelain --untracked-files=normal).Count -or $LASTEXITCODE -ne 0) { throw 'Service packaging requires a clean committed worktree.' }
  if ((& git -C $repoRoot rev-parse HEAD).Trim() -ne $commit) { throw 'Source changed during service packaging.' }
}
Assert-CleanSource
$started = [DateTime]::UtcNow.ToString('O')
& powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $repoRoot 'native/build.ps1')
if ($LASTEXITCODE -ne 0) { throw 'Fresh native service build failed.' }
Assert-CleanSource
$server = Join-Path $repoRoot 'native/build/verdigris_server.exe'
$identity = (& $server --build-info | Out-String).Trim()
if ($LASTEXITCODE -ne 0 -or $identity -ne ($commit + ' clean')) { throw "Service executable identity mismatch: $identity" }
New-Item -ItemType Directory -Path $destination | Out-Null
Copy-Item -LiteralPath $server -Destination (Join-Path $destination 'verdigris_server.exe')
Copy-Item -LiteralPath (Join-Path $repoRoot 'LICENSE') -Destination (Join-Path $destination 'HISTORICAL-LICENSE.txt')
foreach ($file in @('SERVICE-OPERATIONS.md', 'Caddyfile.example', 'start-service.ps1', 'stop-service.ps1', 'verify-service-package.ps1')) {
  Copy-Item -LiteralPath (Join-Path $PSScriptRoot $file) -Destination (Join-Path $destination $file)
}
$manifest = [ordered]@{
  schema = 'verdigris-service/1'; createdUtc = [DateTime]::UtcNow.ToString('O'); sourceCommit = $commit; sourceTree = $tree
  sourceStatus = @(); freshBuild = $true; buildStartedUtc = $started; embeddedIdentity = $identity
  platform = 'Windows x64, Windows 10/Server 2016 or later'; entryPoint = 'verdigris_server.exe'; authority = 'compiled into executable'
  runtimeDependencies = @('Microsoft Windows Winsock, CNG and WinSQLite system libraries', 'Visual C++ runtime matching native build', 'Optional separately operated Caddy TLS gateway; not bundled')
  content = 'Authoritative gameplay data is compiled into the server. No client renderer, art, profiles, saves, enrollment codes or TLS keys are packaged.'
  files = @(Get-ChildItem -LiteralPath $destination -File | ForEach-Object { [ordered]@{path = $_.Name; bytes = $_.Length; sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()} })
}
$manifest | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $destination 'service-manifest.json') -Encoding UTF8
& powershell.exe -NoProfile -File (Join-Path $destination 'verify-service-package.ps1') -PackageDirectory $destination
if ($LASTEXITCODE -ne 0) { throw 'Exact service package verification failed.' }
Write-Output "Service package: $destination; source: $commit"
