<#
.SYNOPSIS
Compile and run the production raster equipment and fractional sampling checks.
.DESCRIPTION
Requires Windows and Visual Studio C++ Build Tools. The script finds the repo
from its own location and puts all binaries, fixtures, and contact sheets under
.ci-artifacts/raster-equipment. It does not import or change runtime artwork.
The production-rendered strike-action-review.html includes playback and a pose
slider at 288px actor height. The preview's 90ms frames do not test simulation
contact timing. An optional Python with Pillow also writes a portable GIF.
Eight-frame directional walks also produce fixed-pivot native/3x playback
pages and optional GIFs, using the same production equipment draw path.
.PARAMETER VcVars
Optional path to vcvars64.bat; otherwise discover it with vswhere or standard
Visual Studio 2019/2022 installation paths.
.PARAMETER Python
Optional Python executable with Pillow installed, for the rendered preview GIF.
.EXAMPLE
./native/tools/raster/test_equipment.ps1
#>
param([string]$VcVars = '', [string]$Python = '')

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$artifactRoot = Join-Path $repoRoot '.ci-artifacts\raster-equipment'

if (-not $VcVars) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $installation = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($LASTEXITCODE -eq 0 -and $installation) {
            $VcVars = Join-Path ($installation | Select-Object -First 1) 'VC\Auxiliary\Build\vcvars64.bat'
        }
    }
    if (-not $VcVars -or -not (Test-Path -LiteralPath $VcVars)) {
        foreach ($base in @(${env:ProgramFiles(x86)}, $env:ProgramFiles)) {
            foreach ($year in @('2022', '2019')) {
                foreach ($edition in @('BuildTools', 'Community', 'Professional', 'Enterprise')) {
                    $candidate = Join-Path $base "Microsoft Visual Studio\$year\$edition\VC\Auxiliary\Build\vcvars64.bat"
                    if (Test-Path -LiteralPath $candidate) { $VcVars = $candidate; break }
                }
                if ($VcVars -and (Test-Path -LiteralPath $VcVars)) { break }
            }
            if ($VcVars -and (Test-Path -LiteralPath $VcVars)) { break }
        }
    }
}
if (-not $VcVars -or -not (Test-Path -LiteralPath $VcVars -PathType Leaf)) {
    throw 'MSVC vcvars64.bat was not found. Install C++ Build Tools or pass -VcVars.'
}

New-Item -ItemType Directory -Force -Path (Join-Path $artifactRoot 'sampling') | Out-Null
Push-Location $repoRoot
try {
    foreach ($name in @('test_equipment', 'test_equipment_sampling')) {
        $source = Join-Path $PSScriptRoot "$name.cpp"
        $object = Join-Path $artifactRoot "$name.obj"
        $exe = Join-Path $artifactRoot "$name.exe"
        $command = 'call "' + $VcVars + '" >nul && cl /nologo /std:c++20 /EHsc /W4 /O2 /UNDEBUG "' + $source + '" /Fo"' + $object + '" /Fe"' + $exe + '" /link user32.lib gdi32.lib'
        & cmd.exe /d /s /c $command
        if ($LASTEXITCODE -ne 0) { throw "$name compile failed with exit code $LASTEXITCODE" }
        & $exe
        if ($LASTEXITCODE -ne 0) { throw "$name failed with exit code $LASTEXITCODE" }
    }
    if ($Python) {
        @'
import sys
from pathlib import Path
from PIL import Image
root = Path(sys.argv[1])
frames = [Image.open(root / f"strike-action-frame{i}-3x.png").convert("RGB") for i in range(7)]
order = [6, 0, 1, 2, 3, 4, 5, 6]
sequence = [frames[i] for i in order]
sequence[0].save(root / "strike-action-review-3x.gif", save_all=True,
                 append_images=sequence[1:], loop=0,
                 duration=[360, 90, 90, 90, 90, 90, 90, 360], disposal=2)
print("Wrote strike-action-review-3x.gif from production-rendered frames.")
for facing in ("nw", "sw", "ne"):
    prefix = f"strike-{facing}-action"
    paths = [root / f"{prefix}-frame{i}-3x.png" for i in range(7)]
    if not all(path.exists() for path in paths):
        continue
    frames = [Image.open(path).convert("RGB") for path in paths]
    sequence = [frames[i] for i in order]
    sequence[0].save(root / f"{prefix}-review-3x.gif", save_all=True,
                     append_images=sequence[1:], loop=0,
                     duration=[360, 90, 90, 90, 90, 90, 90, 360], disposal=2)
    print(f"Wrote {prefix}-review-3x.gif from production-rendered frames.")
for facing in ("sw", "nw", "ne"):
    for scale in (1, 3):
        paths = [root / f"walk-{facing}-action-frame{i}-{scale}x.png" for i in range(8)]
        if not all(path.exists() for path in paths):
            continue
        frames = [Image.open(path).convert("RGB") for path in paths]
        frames[0].save(root / f"walk-{facing}-action-review-{scale}x.gif", save_all=True,
                       append_images=frames[1:], loop=0, duration=100, disposal=2)
        print(f"Wrote walk-{facing}-action-review-{scale}x.gif from production-rendered frames.")
'@ | & $Python - $artifactRoot
        if ($LASTEXITCODE -ne 0) { throw "Strike preview GIF failed with exit code $LASTEXITCODE" }
    }
    Write-Host "Equipment checks passed. Contact sheets, strike animation and sampling fixtures: $artifactRoot"
} finally {
    Pop-Location
}
