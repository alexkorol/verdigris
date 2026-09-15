param([Parameter(Mandatory = $true)][string]$PackageDirectory)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath($PackageDirectory).TrimEnd('\')
$manifest = Get-Content -LiteralPath (Join-Path $root 'service-manifest.json') -Raw | ConvertFrom-Json
if ($manifest.schema -ne 'verdigris-service/1' -or -not $manifest.freshBuild -or $manifest.sourceCommit -notmatch '^[0-9a-f]{40}$' -or @($manifest.sourceStatus).Count) { throw 'Invalid service provenance.' }
foreach ($file in $manifest.files) {
  $target = [IO.Path]::GetFullPath((Join-Path $root $file.path))
  if (-not $target.StartsWith($root + '\', [StringComparison]::OrdinalIgnoreCase) -or (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash -ne $file.sha256) { throw 'Service manifest path/hash mismatch.' }
}
$identity = (& (Join-Path $root 'verdigris_server.exe') --build-info | Out-String).Trim()
if ($LASTEXITCODE -ne 0 -or $identity -ne ($manifest.sourceCommit + ' clean')) { throw 'Service executable identity mismatch.' }
$actual = @(Get-ChildItem -LiteralPath $root -Recurse -File)
if ($actual.Count -ne @($manifest.files).Count + 1) { throw 'Service package contains unmanifested resources.' }
Write-Output "PASS service package hashes and embedded source: $identity"
