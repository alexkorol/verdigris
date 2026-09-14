param(
  [Parameter(Mandatory=$true)][string]$InstallationDirectory,
  [Parameter(Mandatory=$true)][string]$EvidenceDirectory
)
$ErrorActionPreference='Stop'
$installRoot=[IO.Path]::GetFullPath($InstallationDirectory)
$evidenceRoot=[IO.Path]::GetFullPath($EvidenceDirectory)
$entry=Join-Path $installRoot 'Verdigris.exe'
$profile=Join-Path $installRoot 'profile'
$manifest=Get-Content -LiteralPath (Join-Path $installRoot 'package-manifest.json') -Raw | ConvertFrom-Json
if($manifest.sourceCommit -notmatch '^[0-9a-f]{40}$'){throw 'Installation has no valid source manifest'}
New-Item -ItemType Directory -Force -Path $evidenceRoot | Out-Null
$beforeFiles=@{}
$saveRoot=Join-Path $profile 'saves'
if(Test-Path -LiteralPath $saveRoot) {
  foreach($file in Get-ChildItem -LiteralPath $saveRoot -Recurse -File -Filter '*.json') {
    $beforeFiles[$file.FullName]=[IO.File]::ReadAllText($file.FullName)
    Copy-Item -LiteralPath $file.FullName -Destination (Join-Path $evidenceRoot ('before-'+$file.Name))
  }
}
$settings=Join-Path $env:LOCALAPPDATA 'Verdigris/settings.ini'
$settingsHash=if(Test-Path -LiteralPath $settings){(Get-FileHash -LiteralPath $settings).Hash}else{'absent'}
$entryHash=(Get-FileHash -LiteralPath $entry).Hash
$started=[DateTime]::UtcNow
# Application-owned title-only diagnostic, not desktop automation. No gameplay
# or Settings actions are invoked; normal profile selection is deliberately used.
$process=Start-Process -FilePath $entry -ArgumentList @('--verify-launch','smoke') -WorkingDirectory $installRoot -WindowStyle Hidden -PassThru
$retainedHandle=$process.Handle
if(!$process.WaitForExit(90000)) {
  $process.Kill();$process.WaitForExit()
  throw 'Normal-launch smoke timed out; only the fresh diagnostic launcher was stopped'
}
$process.WaitForExit()
if($null -eq $process.ExitCode -or $process.ExitCode -ne 0){throw "Normal-launch smoke failed ($($process.ExitCode))"}
$log=Get-ChildItem -LiteralPath (Join-Path $profile 'logs') -File -Filter 'session-*.log' |
  Where-Object LastWriteTimeUtc -GE $started | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
if(!$log){throw 'Normal-launch smoke produced no new session log'}
$body=Get-Content -LiteralPath $log.FullName -Raw
Copy-Item -LiteralPath $log.FullName -Destination (Join-Path $evidenceRoot 'normal-launch.log')
foreach($expected in @('isolatedSettings=False',('phase=smoke source='+$manifest.sourceCommit),('profile='+$profile),
  ('image='+[IO.Path]::GetFullPath((Join-Path $installRoot 'native/build/verdigris_client.exe'))),
  ('image='+[IO.Path]::GetFullPath((Join-Path $installRoot 'native/build/verdigris_server.exe'))),
  ('cwd='+$installRoot),'perspective=1 authored-poses=96')) {
  if(!$body.Contains($expected)){throw "Missing normal-launch evidence: $expected"}
}
$ownedIds=@([regex]::Matches($body,'(?:verdigris_client|verdigris_server)\.exe pid=(\d+)') | ForEach-Object {[int]$_.Groups[1].Value})
if($ownedIds.Count -ne 2){throw 'Normal launcher did not record both owned child processes'}
foreach($childId in $ownedIds) {
  if(Get-Process -Id $childId -ErrorAction SilentlyContinue){throw "Child $childId remains after normal launcher exit"}
  if($body -notmatch "cleaned pid=$childId exit=0"){throw "Child $childId did not record clean shutdown"}
}
function Assert-Preserved($before,$after,[string]$path) {
  if($null -eq $before){if($null -ne $after){throw "Changed saved value: $path"};return}
  if($before -is [PSCustomObject]) {
    if($after -isnot [PSCustomObject]){throw "Changed saved object: $path"}
    foreach($property in $before.PSObject.Properties) {
      $next=$after.PSObject.Properties[$property.Name]
      if($null -eq $next){throw "Removed saved field: $path/$($property.Name)"}
      # Authorized purse migration removes only its backpack coordinates.
      if($before.id -eq 'coins' -and $property.Name -in @('slot','position') -and
         ($null -eq $next.Value -or ($property.Name -eq 'slot' -and $next.Value -eq -1))){continue}
      Assert-Preserved $property.Value $next.Value "$path/$($property.Name)"
    }
  } elseif($before -is [Array]) {
    if($after -isnot [Array] -or $before.Count -ne $after.Count){throw "Changed saved list: $path"}
    for($i=0;$i -lt $before.Count;$i++){Assert-Preserved $before[$i] $after[$i] "$path/$i"}
  } elseif($before -cne $after){throw "Changed saved value: $path"}
}
foreach($file in $beforeFiles.Keys) {
  if(!(Test-Path -LiteralPath $file)){throw "Existing save disappeared: $file"}
  $after=[IO.File]::ReadAllText($file)
  Assert-Preserved ($beforeFiles[$file] | ConvertFrom-Json) ($after | ConvertFrom-Json) $file
  Copy-Item -LiteralPath $file -Destination (Join-Path $evidenceRoot ('after-'+[IO.Path]::GetFileName($file)))
}
$afterSettings=if(Test-Path -LiteralPath $settings){(Get-FileHash -LiteralPath $settings).Hash}else{'absent'}
if($afterSettings -ne $settingsHash){throw 'Normal-launch smoke changed user settings'}
if((Get-FileHash -LiteralPath $entry).Hash -ne $entryHash){throw 'Normal launch entry changed during verification'}
Copy-Item -LiteralPath (Join-Path $profile 'logs/smoke-title.png') -Destination (Join-Path $evidenceRoot 'normal-title.png')
"PASS exact normal entry, working directory, actual child images, normal profile, perspective startup, confirmed menu quit and owned-process cleanup. Source=$($manifest.sourceCommit)" | Tee-Object -FilePath (Join-Path $evidenceRoot 'normal-result.txt')
"PASS $($beforeFiles.Count) existing saves preserved (additive schema and purse-coordinate migration only); settings=$settingsHash. Entry SHA256=$entryHash" | Tee-Object -FilePath (Join-Path $evidenceRoot 'preservation.txt')
