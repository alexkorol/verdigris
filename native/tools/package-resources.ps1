# Runtime requirements shared by creation and validation, independent of the
# package's self-reported file inventory. Candidates under docs/art-review are
# deliberately not runtime resources.
function Get-NativePackageResources([string]$Root) {
  $required = @(
    'native/build/verdigris_client.exe', 'native/build/verdigris_server.exe',
    'src/assets/fonts/pixelmix.ttf', 'src/assets/fonts/pixelmix_bold.ttf', 'src/assets/fonts/PxPlus_IBM_VGA8.ttf',
    'src/assets/inventory/frame_ornate.png', 'src/assets/orbs/wizard/art.png',
    'src/assets/orbs/wizard/mask_fullres.png', 'src/assets/orbs/wizard/empty_aligned.jpg',
    'native/client/assets/wizard/framekit/textures/panel.png', 'native/client/assets/wizard/framekit/textures/slot.png',
    'native/client/assets/wizard/splash/background_fallback.png',
    'native/client/assets/raster/runtime/catalog.json',
    'native/client/assets/wizard/inventory/manifest.json',
    'native/client/assets/wizard/inventory/handstone_flint.png',
    'native/client/assets/wizard/inventory/handstone-provenance.json'
  )
  foreach ($name in @('scion_str', 'raider', 'boss', 'tree', 'ruin', 'dwelling', 'shrine', 'terrain1', 'terrain4')) {
    $required += "prototypes/founding-slice/assets/$name.png"
  }
  $inventory = Get-Content -LiteralPath (Join-Path $Root 'native/client/assets/wizard/inventory/manifest.json') -Raw | ConvertFrom-Json
  if (@($inventory.assets).Count -lt 30) { throw 'Inventory art manifest is incomplete.' }
  foreach ($asset in $inventory.assets) {
    if ($asset.file -match '[/\\:]' -or $asset.file -notmatch '\.png$') { throw 'Invalid inventory artwork filename.' }
    $relative = 'native/client/assets/wizard/inventory/' + $asset.file
    if ((Get-FileHash -LiteralPath (Join-Path $Root $relative) -Algorithm SHA256).Hash -ne $asset.sha256) {
      throw "Inventory reference artwork hash mismatch: $relative"
    }
    $required += $relative
  }
  $handstone = Get-Content -LiteralPath (Join-Path $Root 'native/client/assets/wizard/inventory/handstone-provenance.json') -Raw | ConvertFrom-Json
  foreach ($entry in $handstone.files.PSObject.Properties) {
    if ($entry.Name -notmatch '^native/client/assets/(wizard/inventory/|raster/runtime/)[a-zA-Z0-9_./-]+\.png$' -or $entry.Name.Contains('..')) { throw 'Invalid handstone provenance path.' }
    if ((Get-FileHash -LiteralPath (Join-Path $Root $entry.Name) -Algorithm SHA256).Hash -ne $entry.Value) { throw "Handstone conversion hash mismatch: $($entry.Name)" }
    $required += $entry.Name
  }
  $catalog = Get-Content -LiteralPath (Join-Path $Root 'native/client/assets/raster/runtime/catalog.json') -Raw | ConvertFrom-Json
  if (@($catalog.assets).Count -eq 0) { throw 'Raster catalog has no assets.' }
  foreach ($asset in $catalog.assets) {
    if ([string]::IsNullOrWhiteSpace($asset.file) -or $asset.file -match '[/\\:]' -or $asset.file -notmatch '\.png$') {
      throw 'Invalid raster catalog filename.'
    }
    $required += 'native/client/assets/raster/runtime/' + $asset.file
  }
  # Independent minimum: removing entries from a catalog must not certify an
  # incomplete authored animation package.
  foreach ($sex in @('male', 'female')) {
    foreach ($pose in @('', '_walk0', '_walk1', '_strike0', '_strike1', '_strike2')) {
      foreach ($heading in @('n', 'ne', 'e', 'se', 's', 'sw', 'w', 'nw')) {
        $required += "native/client/assets/raster/runtime/hero_$sex$($pose)_$heading.png"
      }
    }
  }
  return $required | Sort-Object -Unique
}
