<#
.SYNOPSIS
Build and run an isolated production ground cache probe (no client rebuild).
.DESCRIPTION
Matches native/build.ps1 compiler optimization defaults. Assertions remain
enabled explicitly; this probe does not add /O2 absent from the client build.
#>
param([string]$VcVars='')
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$artifactRoot=Join-Path $repoRoot '.ci-artifacts\ground-raster'
if(-not $VcVars){
    $vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if(Test-Path -LiteralPath $vswhere){
        $installation=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if($LASTEXITCODE -eq 0 -and $installation){$VcVars=Join-Path ($installation | Select-Object -First 1) 'VC\Auxiliary\Build\vcvars64.bat'}
    }
}
if(-not $VcVars -or -not (Test-Path -LiteralPath $VcVars -PathType Leaf)){throw 'Install MSVC C++ Build Tools or pass -VcVars.'}
New-Item -ItemType Directory -Force -Path $artifactRoot | Out-Null
$source=Join-Path $PSScriptRoot 'test_ground_raster.cpp'
$object=Join-Path $artifactRoot 'test_ground_raster.obj'
$exe=Join-Path $artifactRoot 'test_ground_raster.exe'
Push-Location $repoRoot
try{
    $command='call "'+$VcVars+'" >nul && cl /nologo /std:c++20 /EHsc /W4 /UNDEBUG "'+$source+'" /Fo"'+$object+'" /Fe"'+$exe+'" /link user32.lib gdi32.lib'
    & cmd.exe /d /s /c $command
    if($LASTEXITCODE -ne 0){throw "Ground probe compile failed: $LASTEXITCODE"}
    'compiler_flags=/std:c++20 /EHsc /W4 /UNDEBUG; optimization=MSVC default (matches native/build.ps1)' |
        Tee-Object -FilePath (Join-Path $artifactRoot 'result.log')
    & $exe | Tee-Object -Append -FilePath (Join-Path $artifactRoot 'result.log')
    if($LASTEXITCODE -ne 0){throw "Ground probe failed: $LASTEXITCODE"}
}finally{Pop-Location}
