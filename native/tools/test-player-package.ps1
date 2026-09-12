param([Parameter(Mandatory = $true)][string]$PackageDirectory)
$ErrorActionPreference = 'Stop'
$packageRoot = [IO.Path]::GetFullPath($PackageDirectory)
$manifest = Get-Content -LiteralPath (Join-Path $packageRoot 'package-manifest.json') -Raw | ConvertFrom-Json
foreach ($entry in $manifest.files) {
  $path = [IO.Path]::GetFullPath((Join-Path $packageRoot $entry.path))
  if (-not $path.StartsWith($packageRoot.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Manifest path escapes package.' }
  if ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $entry.sha256) { throw "Package hash mismatch: $($entry.path)" }
}
Write-Output "PASS package hashes: $($manifest.files.Count)"
# Exercise the packaged production lock method, not a duplicate implementation.
# The old hash-mutex code allowed both aliases to hold locks simultaneously.
$assembly = [Reflection.Assembly]::LoadFile((Join-Path $packageRoot 'Verdigris.exe'))
$method = $assembly.GetType('PlayerLauncher').GetMethod('AcquireProfileLock', [Reflection.BindingFlags]'NonPublic,Static')
if (-not $method) { throw 'Packaged launcher has no profile lock method.' }
$fixture = Join-Path ([IO.Path]::GetTempPath()) ('verdigris-lock-test-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $fixture | Out-Null
$first = $method.Invoke($null, [object[]]@([string]$fixture))
$second = $null
try {
  $denied = $false
  try { $second = $method.Invoke($null, [object[]]@([string]($fixture + '\'))) }
  catch { $denied = $_.Exception.ToString().Contains('already open or unavailable') }
  if (-not $denied) { throw 'Trailing-separator alias acquired a second lock.' }
  Write-Output 'PASS simultaneous trailing-separator alias refused'
} finally {
  if ($second) { $second.Dispose() }
  $first.Dispose()
}
$reopened = $method.Invoke($null, [object[]]@([string]($fixture + '\')))
$reopened.Dispose()
Write-Output 'PASS profile opens again after previous lock closes'
Write-Output "Retained disposable fixture: $fixture"
