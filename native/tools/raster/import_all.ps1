param(
    [string]$PixelRespecterRoot = 'Z:\Code\Python\pixel-perfecter',
    [string]$Python = '',
    [switch]$Previews
)
$ErrorActionPreference = 'Stop'
if (-not $Python) { $Python = Join-Path $PixelRespecterRoot '.venv\Scripts\python.exe' }
if (-not (Test-Path -LiteralPath $Python)) { throw "Python runtime not found: $Python" }
$importer = Join-Path $PSScriptRoot 'import_assets.py'
# The isolated, genuine-alpha hero anchor deliberately overrides the older
# provisional sheet SE frame. build_catalog.py resolves the same order.
$manifests = @('inventory', 'terrain', 'props', 'gate', 'actors', 'bestiary', 'weapons', 'hero-single', 'bestiary-singles', 'hero-walk', 'hero-directions', 'environment-singles', 'large-props', 'hero-strike-se', 'hit-spark', 'hero-walk-sw', 'hero-walk-nw', 'hero-walk-ne', 'hero-strike-nw', 'hero-strike-sw', 'terrain-quiet', 'raider-walk-sw', 'hero-strike-ne', 'raider-strike-sw')
foreach ($name in $manifests) {
    $arguments = @($importer, (Join-Path $PSScriptRoot "$name.json"), '--project', $PixelRespecterRoot)
    if ($Previews) {
        $arguments += @('--contact-sheet', (Join-Path $PSScriptRoot "$name-preview-3x.png"), '--preview-scale', '3')
    }
    & $Python @arguments
    if ($LASTEXITCODE -ne 0) { throw "Raster import failed: $name" }
}
& $Python (Join-Path $PSScriptRoot 'build_catalog.py')
if ($LASTEXITCODE -ne 0) { throw 'Active runtime catalog verification failed' }
