<#
.SYNOPSIS
Test production effect colors on DIB/DDB surfaces and the fixed actor flash.
.DESCRIPTION
Requires Visual Studio C++ Build Tools. The test draws raw reference colors and
the production vector_art helpers, checks GetPixel/BGRA bytes and exported PNGs,
and writes all binaries, swatches and colors.csv under .ci-artifacts/effect-colors.
No exporter or test-side red/blue compensation is applied. Nonzero means failure.
The sprite-flash checks cover alpha holes, nearest alignment, facing, opacity,
repeat-cache reuse and shared eviction. A 3x production hero preview is saved.
.PARAMETER Label
Artifact subdirectory, useful for preserving before/after evidence.
.PARAMETER VcVars
Optional explicit path to vcvars64.bat.
.EXAMPLE
./native/tools/raster/test_effect_colors.ps1 -Label after
#>
param([ValidatePattern('^[a-zA-Z0-9_-]+$')][string]$Label = 'current', [string]$VcVars = '')
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$artifactRoot = Join-Path $repoRoot ".ci-artifacts\effect-colors\$Label"
if (-not $VcVars) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $installation = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($LASTEXITCODE -eq 0 -and $installation) {
            $VcVars = Join-Path ($installation | Select-Object -First 1) 'VC\Auxiliary\Build\vcvars64.bat'
        }
    }
}
if (-not $VcVars -or -not (Test-Path -LiteralPath $VcVars -PathType Leaf)) {
    throw 'MSVC vcvars64.bat was not found. Install C++ Build Tools or pass -VcVars.'
}
New-Item -ItemType Directory -Force -Path $artifactRoot | Out-Null
$source = Join-Path $PSScriptRoot 'test_effect_colors.cpp'
$object = Join-Path $artifactRoot 'test_effect_colors.obj'
$exe = Join-Path $artifactRoot 'test_effect_colors.exe'
Push-Location $repoRoot
try {
    $command = 'call "' + $VcVars + '" >nul && cl /nologo /std:c++20 /EHsc /W4 /O2 /UNDEBUG "' + $source + '" /Fo"' + $object + '" /Fe"' + $exe + '" /link user32.lib gdi32.lib'
    & cmd.exe /d /s /c $command
    if ($LASTEXITCODE -ne 0) { throw "Effect color probe compile failed: $LASTEXITCODE" }
    & $exe $artifactRoot | Tee-Object -FilePath (Join-Path $artifactRoot 'result.log')
    if ($LASTEXITCODE -ne 0) { throw "Effect color probe failed: $LASTEXITCODE (see $artifactRoot)" }
    Write-Host "Effect color checks passed: $artifactRoot"
} finally {
    Pop-Location
}
