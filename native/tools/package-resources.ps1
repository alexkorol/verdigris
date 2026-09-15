# Runtime requirements shared by creation and validation, independent of the
# package's self-reported file inventory. Candidates under docs/art-review are
# deliberately not runtime resources.
function Get-FirstSlicePackageResources([string]$Root) {
  $prefix = 'native/client/assets/first-slice/runtime/'
  $manifest = Join-Path $Root ($prefix + 'manifest.tsv')
  if (-not (Test-Path -LiteralPath $manifest -PathType Leaf)) { throw 'Accepted first-slice runtime manifest is missing.' }
  $resources = @($prefix + 'manifest.tsv')
  foreach ($line in Get-Content -LiteralPath $manifest) {
    if ([string]::IsNullOrWhiteSpace($line) -or $line.StartsWith('#')) { continue }
    $fields = $line -split "`t"
    if ($fields.Count -lt 11) { throw 'Invalid first-slice runtime row.' }
    foreach ($name in $fields[10..($fields.Count - 1)]) {
      if ($name -notmatch '^fs_[a-z0-9_-]+_[0-9a-f]{10}$') { throw "Invalid first-slice frame name: $name" }
      $relative = $prefix + $name + '.png'
      $hash = (Get-FileHash -LiteralPath (Join-Path $Root $relative) -Algorithm SHA256).Hash.ToLowerInvariant()
      if (-not $hash.StartsWith($name.Substring($name.Length - 10))) { throw "First-slice frame hash mismatch: $relative" }
      $resources += $relative
    }
  }
  if ($resources.Count -eq 1) { throw 'First-slice runtime manifest contains no sprite frames.' }
  return $resources | Sort-Object -Unique
}

function Get-NativePackageResources([string]$Root) {
  $required = @(
    'native/build/verdigris_client.exe', 'native/build/verdigris_server.exe',
    'native/client/assets/menu/bronze-gateway.png', 'native/client/assets/menu/amber-control.png',
    'native/client/assets/effects/particles.atlas.json',
    'native/client/assets/effects/level_up.effect.json',
    'native/client/assets/effects/melee_hit_small.effect.json',
    'native/client/assets/effects/foot_dust.effect.json',
    'native/client/assets/effects/bowl_ember_idle.effect.json',
    'native/client/assets/effects/burning_touch_contact.effect.json',
    'native/client/assets/effects/simple_death_puff.effect.json',
      'native/client/assets/effects/projectile_trail_simple.effect.json',
      'native/client/assets/effects/war_cry.effect.json',
      'native/client/assets/effects/dash_dust.effect.json',
      'native/client/assets/effects/critical_hit.effect.json',
      'native/client/assets/effects/pickup_motes.effect.json',
    'native/client/assets/fonts/sans/VerdigrisSans.ttf',
    'native/client/assets/fonts/sans/CC0.txt', 'native/client/assets/fonts/sans/README.md',
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
  $required += @(Get-FirstSlicePackageResources -Root $Root)
  return $required | Sort-Object -Unique
}
