<#
.SYNOPSIS
Compile and run the production raster equipment and fractional sampling checks.
.DESCRIPTION
Requires Windows and Visual Studio C++ Build Tools. The script finds the repo
from its own location and puts all binaries, fixtures, and contact sheets under
.ci-artifacts/raster-equipment. It does not import or change runtime artwork.
.PARAMETER VcVars
Optional path to vcvars64.bat; otherwise discover it with vswhere or standard
Visual Studio 2019/2022 installation paths.
.EXAMPLE
./native/tools/raster/test_equipment.ps1
#>
param([string]$VcVars = '')

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
    Write-Host "Equipment checks passed. Contact sheets and sampling fixtures: $artifactRoot"
} finally {
    Pop-Location
}
