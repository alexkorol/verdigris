$ErrorActionPreference = 'Stop'
$assetRoot = Join-Path $PSScriptRoot '../client/assets/wizard/splash/world'
$manifest = Get-Content -LiteralPath (Join-Path $assetRoot 'manifest.json') -Raw | ConvertFrom-Json
foreach ($entry in $manifest.files.PSObject.Properties) {
  if ($entry.Name -notmatch '^celestial_world_[a-z0-9_]+\.(png|glb)$') { throw 'Unexpected title asset path' }
  $actual = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $assetRoot $entry.Name)).Hash.ToLowerInvariant()
  if ($actual -ne $entry.Value) { throw "Title source asset changed: $($entry.Name)" }
}
Write-Host 'WIZARD native title: authored mesh and three raster hashes verified'
