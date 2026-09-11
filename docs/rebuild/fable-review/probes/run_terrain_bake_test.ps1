param(
  [string]$VcVars = 'C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Auxiliary/Build/vcvars64.bat',
  [switch]$CompileOnly
)
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../../../..')).Path
$output = Join-Path $repo '.ci-artifacts/fable-renderer/probes'
New-Item -ItemType Directory -Force -Path $output | Out-Null
if (!(Test-Path -LiteralPath $VcVars)) { throw 'Pass -VcVars with an installed MSVC x64 environment script.' }
$sourceHeader = Join-Path $repo 'native/client/fable_world.hpp'
$header = [System.IO.File]::ReadAllText($sourceHeader)
$end = $header.IndexOf('inline Renderer& renderer()')
if ($end -lt 0) { throw 'Renderer boundary not found; update this isolated fixture explicitly.' }
# Use the unchanged production helpers and complete Renderer. The fixture
# supplies only minimal world data and the unrelated equipment painter.
[System.IO.File]::WriteAllText((Join-Path $output 'terrain-renderer-under-test.hpp'),
  $header.Substring(0, $end) + "`n}`n", [System.Text.UTF8Encoding]::new($false))
$prefixBytes = [System.Text.Encoding]::UTF8.GetBytes($header.Substring(0, $end))
$sha = [System.Security.Cryptography.SHA256]::Create()
[System.IO.File]::WriteAllText((Join-Path $output 'terrain-renderer-under-test.sha256'),
  ([System.BitConverter]::ToString($sha.ComputeHash($prefixBytes))).Replace('-', '').ToLowerInvariant() + "`n")
$sha.Dispose()
Push-Location $repo
try {
  $source = Join-Path $PSScriptRoot 'test_terrain_bake.cpp'
  $object = Join-Path $output 'test_terrain_bake.obj'
  $exe = Join-Path $output 'test_terrain_bake.exe'
  $command = 'call "' + $VcVars + '" >nul && cl /nologo /std:c++20 /EHsc /W4 /WX /O2 /I"' + $output + '" "' + $source + '" /Fo"' + $object + '" /Fe"' + $exe + '" /link user32.lib gdi32.lib'
  & cmd.exe /d /s /c $command
  if ($LASTEXITCODE -ne 0) { throw "Terrain bake test compile failed: $LASTEXITCODE" }
  if ($CompileOnly) { return }
  & $exe *> (Join-Path $output 'test_terrain_bake.log')
  $result = $LASTEXITCODE
  Get-Content (Join-Path $output 'test_terrain_bake.log')
  if ($result -ne 0) { throw "Terrain bake test failed: $result" }
} finally { Pop-Location }
