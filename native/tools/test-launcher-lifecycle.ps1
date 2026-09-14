param(
  [Parameter(Mandatory=$true)][string]$PackageDirectory,
  [Parameter(Mandatory=$true)][string]$EvidenceDirectory
)
$ErrorActionPreference='Stop'
$packageRoot=[IO.Path]::GetFullPath($PackageDirectory)
$entry=Join-Path $packageRoot 'Verdigris.exe'
if(!(Test-Path -LiteralPath $entry)){throw 'Packaged top-level launcher is missing'}
$evidenceRoot=[IO.Path]::GetFullPath($EvidenceDirectory)
New-Item -ItemType Directory -Force -Path $evidenceRoot | Out-Null
# Every invocation owns a new profile. Never target the owner's live window or
# normal profile, and never inject desktop input. Both phases run the same exe.
$qaProfile=Join-Path $evidenceRoot ('launcher-qa-'+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $qaProfile | Out-Null
$entryHash=(Get-FileHash -LiteralPath $entry).Hash
$clientHash=(Get-FileHash -LiteralPath (Join-Path $packageRoot 'native/build/verdigris_client.exe')).Hash
foreach($phase in @('save','reload')) {
  $process=Start-Process -FilePath $entry -ArgumentList @('--profile',('"'+$qaProfile+'"'),'--verify-launch',$phase) -WorkingDirectory $packageRoot -WindowStyle Hidden -PassThru
  $retainedHandle=$process.Handle
  if(!$process.WaitForExit(90000)) {
    # Only the fresh QA launcher started above; its job owns its child cleanup.
    $process.Kill();$process.WaitForExit()
    throw "Launcher QA $phase timed out. Profile and logs retained: $qaProfile"
  }
  $process.WaitForExit()
  if($null -eq $process.ExitCode -or $process.ExitCode -ne 0){throw "Launcher QA $phase failed ($($process.ExitCode)); see $qaProfile/logs"}
  $log=Get-ChildItem -LiteralPath (Join-Path $qaProfile 'logs') -File | Sort-Object LastWriteTime -Descending | Select-Object -First 1
  $body=Get-Content -LiteralPath $log.FullName -Raw
  if($body -notmatch "launch-check: PASS phase=$phase" -or $body -notmatch 'client exit=0' -or $body -notmatch 'isolatedSettings=True') {
    throw "Incomplete launcher QA evidence in $($log.FullName)"
  }
  $ownedIds=@([regex]::Matches($body,'(?:verdigris_client|verdigris_server)\.exe pid=(\d+)') | ForEach-Object {[int]$_.Groups[1].Value})
  if($ownedIds.Count -ne 2){throw 'Launcher did not record both owned children'}
  foreach($childId in $ownedIds) {
    if(Get-Process -Id $childId -ErrorAction SilentlyContinue){throw "QA child $childId remains after launcher exit"}
    if($body -notmatch "cleaned pid=$childId exit=0"){throw "No clean exit evidence for QA child $childId"}
  }
  if((Get-FileHash -LiteralPath $entry).Hash -ne $entryHash -or (Get-FileHash -LiteralPath (Join-Path $packageRoot 'native/build/verdigris_client.exe')).Hash -ne $clientHash){throw 'Package changed between launch phases'}
  Write-Output "PASS launcher $phase, menu confirmation, owned client/server shutdown; profile=$qaProfile"
}
Write-Output "PASS same executable and same QA profile settings restart. Entry SHA256=$entryHash client SHA256=$clientHash"
