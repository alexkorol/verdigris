param(
  [Parameter(Mandatory = $true)][string]$OutputDirectory,
  [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$destination = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $destination) { throw "Package destination already exists; choose a new directory to preserve its files and saves: $destination" }
if (-not $SkipBuild) {
  & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $repoRoot 'native\build.ps1')
  if ($LASTEXITCODE -ne 0) { throw 'Native build failed.' }
}
$required = @(
  'native/build/verdigris_client.exe', 'native/build/verdigris_server.exe',
  'src/assets/fonts/pixelmix.ttf', 'src/assets/fonts/pixelmix_bold.ttf', 'src/assets/fonts/PxPlus_IBM_VGA8.ttf',
  'src/assets/inventory/frame_ornate.png', 'src/assets/orbs/wizard/art.png', 'src/assets/orbs/wizard/mask_fullres.png',
  'native/client/assets/wizard/framekit/textures/panel.png', 'native/client/assets/wizard/framekit/textures/slot.png'
)
foreach ($relative in $required) {
  if (-not (Test-Path -LiteralPath (Join-Path $repoRoot $relative) -PathType Leaf)) { throw "Required package resource missing: $relative" }
}
New-Item -ItemType Directory -Path $destination | Out-Null
function Copy-Resource([string]$relative) {
  $target = Join-Path $destination $relative
  New-Item -ItemType Directory -Force -Path (Split-Path $target -Parent) | Out-Null
  Copy-Item -LiteralPath (Join-Path $repoRoot $relative) -Destination $target -Recurse
}
foreach ($relative in @('native/build/verdigris_client.exe', 'native/build/verdigris_server.exe',
  'native/client/assets', 'src/assets/fonts', 'src/assets/inventory', 'src/assets/orbs/wizard', 'prototypes/founding-slice/assets')) {
  Copy-Resource $relative
}
$launcher = Join-Path $destination 'Verdigris.exe'
Add-Type -Path (Join-Path $PSScriptRoot 'player-launcher.cs') -OutputAssembly $launcher -OutputType WindowsApplication -ReferencedAssemblies System.Windows.Forms
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'PLAYER-README.txt') -Destination (Join-Path $destination 'READ-ME.txt')
$sourceCommit = (& git -C $repoRoot rev-parse HEAD).Trim()
$sourceStatus = @(& git -C $repoRoot status --short)
$manifest = [ordered]@{
  schema = 'verdigris-local-review/1'; createdUtc = [DateTime]::UtcNow.ToString('O')
  sourceCommit = $sourceCommit; sourceStatus = $sourceStatus; entryPoint = 'Verdigris.exe'
  platform = 'Windows x64'; signing = 'unsigned local review'; buildCommand = 'powershell -NoProfile -File native/build.ps1'
  runtimeDependencies = @('Windows .NET Framework 4.x', 'Windows user32/gdi32/gdiplus/ws2_32/winmm system libraries')
  files = @(Get-ChildItem -LiteralPath $destination -Recurse -File | ForEach-Object {
    [ordered]@{ path = $_.FullName.Substring($destination.Length + 1).Replace('\', '/'); bytes = $_.Length; sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant() }
  })
}
$manifest | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $destination 'package-manifest.json') -Encoding UTF8
Write-Output "Native review package: $launcher"
Write-Output "Files: $($manifest.files.Count); source: $sourceCommit; preserved source-status entries: $($sourceStatus.Count)"
