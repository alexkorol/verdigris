param(
  [string]$VcVars = 'C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Auxiliary/Build/vcvars64.bat',
  [switch]$SkipBenchmark,
  [switch]$CompileOnly
)
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../../../..')).Path
$output = Join-Path $repo '.ci-artifacts/fable-renderer/probes'
New-Item -ItemType Directory -Force -Path $output | Out-Null
if (!(Test-Path -LiteralPath $VcVars)) { throw 'Pass -VcVars with an installed MSVC x64 environment script.' }
Push-Location $repo
try {
  $source = Join-Path $PSScriptRoot 'test_gpu.cpp'
  $object = Join-Path $output 'test_gpu.obj'
  $exe = Join-Path $output 'test_gpu.exe'
  $command = 'call "' + $VcVars + '" >nul && cl /nologo /std:c++20 /EHsc /W4 /WX /O2 "' + $source + '" /Fo"' + $object + '" /Fe"' + $exe + '" /link user32.lib gdi32.lib'
  & cmd.exe /d /s /c $command
  if ($LASTEXITCODE -ne 0) { throw "GPU test compile failed: $LASTEXITCODE" }
  if ($CompileOnly) { return }
  if ($SkipBenchmark) { & $exe --skip-benchmark *> (Join-Path $output 'test_gpu.log') }
  else { & $exe *> (Join-Path $output 'test_gpu.log') }
  $result = $LASTEXITCODE
  Get-Content (Join-Path $output 'test_gpu.log')
  if ($result -ne 0) { throw "GPU test failed: $result" }
} finally { Pop-Location }
