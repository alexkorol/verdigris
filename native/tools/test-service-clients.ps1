param(
  [Parameter(Mandatory=$true)][string]$PackageDirectory,
  [Parameter(Mandatory=$true)][string]$Endpoint,
  [Parameter(Mandatory=$true)][string]$CodeAFile,
  [Parameter(Mandatory=$true)][string]$CodeBFile,
  [Parameter(Mandatory=$true)][string]$EvidenceDirectory,
  [int]$TimeoutSeconds=480
)
$ErrorActionPreference='Stop'
$package=[IO.Path]::GetFullPath($PackageDirectory)
$evidence=[IO.Path]::GetFullPath($EvidenceDirectory)
$client=Join-Path $package 'native/build/verdigris_client.exe'
if(-not (Test-Path -LiteralPath $client -PathType Leaf)){throw "Packaged client missing: $client"}
$codeFiles=@{A=[IO.Path]::GetFullPath($CodeAFile);B=[IO.Path]::GetFullPath($CodeBFile)}
foreach($codeFile in $codeFiles.Values){if(-not (Test-Path -LiteralPath $codeFile -PathType Leaf)){throw 'An enrollment code file is missing.'}}
if((Test-Path -LiteralPath $evidence) -and @(Get-ChildItem -LiteralPath $evidence -Force).Count){throw 'Evidence directory must be new or empty; stale synchronization files are rejected.'}
New-Item -ItemType Directory -Path $evidence -Force | Out-Null
$identity=(& $client --build-info | Out-String).Trim()
if($LASTEXITCODE -ne 0){throw 'Packaged executable identity could not be read.'}
$clients=@()
try {
  foreach($role in @('A','B')) {
    $roleDirectory=Join-Path $evidence $role
    $profile=Join-Path $roleDirectory 'profile'
    New-Item -ItemType Directory -Path $profile -Force | Out-Null
    $start=New-Object Diagnostics.ProcessStartInfo
    $start.FileName=$client
    $start.Arguments='--scenario service-client'
    $start.WorkingDirectory=$package
    $start.UseShellExecute=$false
    $start.CreateNoWindow=$true
    $start.WindowStyle=[Diagnostics.ProcessWindowStyle]::Hidden
    $start.RedirectStandardOutput=$true
    $start.RedirectStandardError=$true
    $start.EnvironmentVariables['VERDIGRIS_QA_ENDPOINT']=$Endpoint
    $start.EnvironmentVariables['VERDIGRIS_QA_CODE_FILE']=$codeFiles[$role]
    $start.EnvironmentVariables['VERDIGRIS_QA_ROLE']=$role
    $start.EnvironmentVariables['VERDIGRIS_QA_SYNC_DIR']=$evidence
    $start.EnvironmentVariables['VERDIGRIS_SERVICE_PROFILE']=$profile
    $start.EnvironmentVariables['VERDIGRIS_SETTINGS_PATH']=(Join-Path $profile 'settings.json')
    $process=New-Object Diagnostics.Process
    $process.StartInfo=$start
    if(-not $process.Start()){throw "Could not launch role $role."}
    $clients+=@{Role=$role;Process=$process;Output=$process.StandardOutput.ReadToEndAsync();Error=$process.StandardError.ReadToEndAsync();Directory=$roleDirectory}
  }
  $until=[DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
  while(@($clients | Where-Object {-not $_.Process.HasExited}).Count) {
    if([DateTime]::UtcNow -ge $until){throw 'Dedicated graphical client check exceeded its bounded timeout.'}
    Start-Sleep -Milliseconds 500
  }
  $results=@()
  foreach($entry in $clients) {
    [IO.File]::WriteAllText((Join-Path $entry.Directory 'client.stdout.log'),$entry.Output.Result)
    [IO.File]::WriteAllText((Join-Path $entry.Directory 'client.stderr.log'),$entry.Error.Result)
    $resultFile=Join-Path $entry.Directory 'result.json'
    if(-not (Test-Path -LiteralPath $resultFile)){throw "Role $($entry.Role) produced no result. The packaged scenario may be missing."}
    $result=Get-Content -LiteralPath $resultFile -Raw | ConvertFrom-Json
    if($entry.Process.ExitCode -ne 0 -or $result.status -ne 'passed'){throw "Role $($entry.Role) failed: $($result.error)"}
    $results+=@($result)
  }
  if($results[0].pid -eq $results[1].pid -or $results[0].actor -eq $results[1].actor){throw 'Independent process/actor identity evidence failed.'}
  if(-not $results[0].instance -or $results[0].instance -ne $results[1].instance){throw 'Clients did not report the same nonempty instance ID.'}
  if($results[0].house -eq $results[1].house -or $results[0].scion -eq $results[1].scion){throw 'House/Scion ownership separation evidence failed.'}
  $summary=@{status='passed';executable=$client;build=$identity;sha256=(Get-FileHash -LiteralPath $client -Algorithm SHA256).Hash;topology='Two separate packaged native client processes; independent externally started service; application-owned hidden windows';roles=$results}
  $summary | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath (Join-Path $evidence 'summary.json') -Encoding UTF8
  Write-Output "Two packaged graphical clients passed. Evidence: $evidence"
} finally {
  foreach($entry in $clients) {
    # Only processes created by this isolated test are owned here. The shared
    # service and any owner's normal game remain independently managed.
    if(-not $entry.Process.HasExited){$entry.Process.Kill();$entry.Process.WaitForExit()}
    if($entry.Output.IsCompleted){[IO.File]::WriteAllText((Join-Path $entry.Directory 'client.stdout.log'),$entry.Output.Result)}
    if($entry.Error.IsCompleted){[IO.File]::WriteAllText((Join-Path $entry.Directory 'client.stderr.log'),$entry.Error.Result)}
    $entry.Process.Dispose()
  }
}
