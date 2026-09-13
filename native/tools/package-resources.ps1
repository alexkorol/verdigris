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
    'native/client/assets/raster/runtime/catalog.json'
  )
  foreach ($name in @('scion_str', 'raider', 'boss', 'tree', 'ruin', 'dwelling', 'shrine', 'terrain1', 'terrain4')) {
    $required += "prototypes/founding-slice/assets/$name.png"
  }
  $catalog = Get-Content -LiteralPath (Join-Path $Root 'native/client/assets/raster/runtime/catalog.json') -Raw | ConvertFrom-Json
  if (@($catalog.assets).Count -eq 0) { throw 'Raster catalog has no assets.' }
  foreach ($asset in $catalog.assets) {
    if ([string]::IsNullOrWhiteSpace($asset.file) -or $asset.file -match '[/\\:]' -or $asset.file -notmatch '\.png$') {
      throw 'Invalid raster catalog filename.'
    }
    $required += 'native/client/assets/raster/runtime/' + $asset.file
  }
  return $required | Sort-Object -Unique
}
