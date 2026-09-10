param([string]$VcVars='')
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$artifactRoot=Join-Path $repoRoot '.ci-artifacts\hud-chrome'
if(-not $VcVars){
    $vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    $installation=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if($installation){$VcVars=Join-Path ($installation | Select-Object -First 1) 'VC\Auxiliary\Build\vcvars64.bat'}
}
if(-not $VcVars -or -not (Test-Path -LiteralPath $VcVars)){throw 'MSVC C++ Build Tools required.'}
New-Item -ItemType Directory -Force -Path $artifactRoot | Out-Null
$source=Join-Path $PSScriptRoot 'test_hud_chrome.cpp'
$object=Join-Path $artifactRoot 'test_hud_chrome.obj'
$exe=Join-Path $artifactRoot 'test_hud_chrome.exe'
Push-Location $repoRoot
try{
    $command='call "'+$VcVars+'" >nul && cl /nologo /std:c++20 /EHsc /W4 /O2 /UNDEBUG "'+$source+'" /Fo"'+$object+'" /Fe"'+$exe+'" /link user32.lib gdi32.lib'
    & cmd.exe /d /s /c $command
    if($LASTEXITCODE -ne 0){throw "HUD probe compile failed: $LASTEXITCODE"}
    & $exe | Tee-Object -FilePath (Join-Path $artifactRoot 'result.log')
    if($LASTEXITCODE -ne 0){throw "HUD probe failed: $LASTEXITCODE"}
}finally{Pop-Location}
