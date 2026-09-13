param(
  [Parameter(Mandatory = $true)][string]$OutputDirectory,
  [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$destination = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $destination) { throw "Package destination already exists; choose a new directory to preserve its files and saves: $destination" }
if ($SkipBuild) { throw 'SkipBuild cannot establish source provenance. Package from a clean commit with a fresh build.' }
$sourceCommit = (& git -C $repoRoot rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0) { throw 'Cannot identify the source commit.' }
$sourceTree = (& git -C $repoRoot rev-parse 'HEAD^{tree}').Trim()
function Assert-CleanSource {
  $status = @(& git -C $repoRoot status --porcelain --untracked-files=normal)
  if ($LASTEXITCODE -ne 0 -or $status.Count -ne 0) { throw 'Commit source changes before packaging; use a clean isolated worktree.' }
  if ((& git -C $repoRoot rev-parse HEAD).Trim() -ne $sourceCommit) { throw 'Source commit changed during packaging.' }
}
Assert-CleanSource
$buildStartedUtc = [DateTime]::UtcNow.ToString('O')
& powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $repoRoot 'native\build.ps1')
if ($LASTEXITCODE -ne 0) { throw 'Native build failed.' }
Assert-CleanSource
. (Join-Path $PSScriptRoot 'package-resources.ps1')
$required = Get-NativePackageResources -Root $repoRoot
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
Assert-CleanSource
$sourceStatus = @()
$manifest = [ordered]@{
  schema = 'verdigris-local-review/1'; createdUtc = [DateTime]::UtcNow.ToString('O')
  sourceCommit = $sourceCommit; sourceTree = $sourceTree; sourceStatus = $sourceStatus; entryPoint = 'Verdigris.exe'
  buildStartedUtc = $buildStartedUtc; buildCompletedUtc = [DateTime]::UtcNow.ToString('O'); freshBuild = $true
  platform = 'Windows x64'; signing = 'unsigned local review'; buildCommand = 'powershell -NoProfile -File native/build.ps1'
  runtimeDependencies = @('Windows .NET Framework 4.x', 'Windows user32/gdi32/gdiplus/ws2_32/winmm system libraries')
  files = @(Get-ChildItem -LiteralPath $destination -Recurse -File | ForEach-Object {
    [ordered]@{ path = $_.FullName.Substring($destination.Length + 1).Replace('\', '/'); bytes = $_.Length; sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant() }
  })
}
$manifest | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $destination 'package-manifest.json') -Encoding UTF8
Write-Output "Native review package: $launcher"
Write-Output "Files: $($manifest.files.Count); source: $sourceCommit; preserved source-status entries: $($sourceStatus.Count)"
