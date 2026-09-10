<#
.SYNOPSIS
Compile the production raster orb probe and save full-size comparison renders.
.DESCRIPTION
Requires Visual Studio C++ Build Tools. Uses existing WIZARD art/empty-glass/mask
plates without changing assets. Outputs stay under .ci-artifacts/orb-raster.
.EXAMPLE
./native/tools/raster/test_orb_raster.ps1
#>
param([string]$VcVars='')
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$artifactRoot=Join-Path $repoRoot '.ci-artifacts\orb-raster'
if(-not $VcVars){
    $vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if(Test-Path -LiteralPath $vswhere){
        $installation=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if($LASTEXITCODE -eq 0 -and $installation){$VcVars=Join-Path ($installation | Select-Object -First 1) 'VC\Auxiliary\Build\vcvars64.bat'}
    }
}
if(-not $VcVars -or -not (Test-Path -LiteralPath $VcVars -PathType Leaf)){throw 'Install MSVC C++ Build Tools or pass -VcVars.'}
New-Item -ItemType Directory -Force -Path $artifactRoot | Out-Null
$source=Join-Path $PSScriptRoot 'test_orb_raster.cpp'
$object=Join-Path $artifactRoot 'test_orb_raster.obj'
$exe=Join-Path $artifactRoot 'test_orb_raster.exe'
Push-Location $repoRoot
try{
    $command='call "'+$VcVars+'" >nul && cl /nologo /std:c++20 /EHsc /W4 /O2 /UNDEBUG "'+$source+'" /Fo"'+$object+'" /Fe"'+$exe+'" /link user32.lib gdi32.lib'
    & cmd.exe /d /s /c $command
    if($LASTEXITCODE -ne 0){throw "Orb probe compile failed: $LASTEXITCODE"}
    & $exe | Tee-Object -FilePath (Join-Path $artifactRoot 'result.log')
    if($LASTEXITCODE -ne 0){throw "Orb probe failed: $LASTEXITCODE"}
}finally{Pop-Location}
