param([Parameter(Mandatory = $true)][string]$ServerExecutable, [string]$EvidenceDirectory)
$ErrorActionPreference = 'Stop'
$server = [IO.Path]::GetFullPath($ServerExecutable)
function Invoke-Control([string[]]$Arguments) {
  $ErrorActionPreference = 'Continue'
  & $server @Arguments 2>&1 | Out-Null
  return $LASTEXITCODE
}
if (-not $EvidenceDirectory) { $EvidenceDirectory = Join-Path $PSScriptRoot ('../build/service-operations-' + [Guid]::NewGuid().ToString('N')) }
$root = [IO.Path]::GetFullPath($EvidenceDirectory)
if (Test-Path -LiteralPath $root) { throw 'Use a fresh service-operations evidence directory.' }
New-Item -ItemType Directory -Path $root | Out-Null
$data = Join-Path $root 'data'
$codePath = Join-Path $root 'enrollment.txt'
$output = & $server --enroll --data $data --output $codePath 2>&1 | Out-String
if ($LASTEXITCODE -ne 0 -or $output -match 'enr_[a-f0-9]{64}') { throw 'Enrollment failed or printed a credential.' }
if (-not ((Get-Content -LiteralPath $codePath -Raw).Trim() -match '^enr_[a-f0-9]{64}$')) { throw 'Enrollment file is invalid.' }
$acl = Get-Acl -LiteralPath $codePath
if (-not $acl.AreAccessRulesProtected -or @($acl.Access).Count -ne 2) { throw 'Enrollment output did not receive a private explicit ACL.' }
if ((Invoke-Control @('--enroll', '--data', $data, '--output', $codePath)) -eq 0) { throw 'Enrollment overwrote an existing file.' }
$probe = New-Object Net.Sockets.TcpListener([Net.IPAddress]::Loopback, 0)
$probe.Start(); $port = $probe.LocalEndpoint.Port; $probe.Stop()
$info = New-Object Diagnostics.ProcessStartInfo
$info.FileName = $server
$info.Arguments = '--service --data "' + $data + '" --port ' + $port + ' --qa-policy coop-v1'
$info.UseShellExecute = $false; $info.CreateNoWindow = $true
$info.RedirectStandardInput = $true; $info.RedirectStandardOutput = $true; $info.RedirectStandardError = $true
$process = [Diagnostics.Process]::Start($info)
$stdout = $process.StandardOutput.ReadToEndAsync(); $stderr = $process.StandardError.ReadToEndAsync()
try {
  $ready = $false; $deadline = [DateTime]::UtcNow.AddSeconds(10)
  while ([DateTime]::UtcNow -lt $deadline -and -not $process.HasExited) {
    if ((Invoke-Control @('--health', '--data', $data)) -eq 0) { $ready = $true; break }
    Start-Sleep -Milliseconds 100
  }
  if (-not $ready) { throw 'Service never became ready.' }
  $process.StandardInput.WriteLine('quit'); $process.StandardInput.Flush(); $process.StandardInput.Close()
  Start-Sleep -Milliseconds 200
  if ($process.HasExited) { throw 'Service lifetime incorrectly depends on stdin.' }
  if ((Invoke-Control @('--enroll', '--data', $data, '--output', (Join-Path $root 'must-not-exist.txt'))) -eq 0) { throw 'Running service did not hold exclusive storage ownership.' }
  & $server --stop --data $data | Out-Null
  if ($LASTEXITCODE -ne 0 -or -not $process.WaitForExit(5000) -or $process.ExitCode -ne 0) { throw 'Service did not stop cleanly.' }
  $stdout.Result | Set-Content -LiteralPath (Join-Path $root 'service-stdout.log')
  $stderr.Result | Set-Content -LiteralPath (Join-Path $root 'service-stderr.log')
  if ($stdout.Result -notmatch 'stopped cleanly' -or $stdout.Result -match 'enr_[a-f0-9]{64}') { throw 'Unexpected shutdown log or credential exposure.' }
} finally {
  if (-not $process.HasExited) { $null = Invoke-Control @('--stop', '--data', $data); if (-not $process.WaitForExit(5000)) { $process.Kill(); $process.WaitForExit() } }
  $process.Dispose()
}
$backup = Join-Path $root 'backup'
& $server --backup $backup --data $data
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath (Join-Path $backup 'backup.complete'))) { throw 'Backup did not complete.' }
$restored = Join-Path $root 'restored'
& $server --restore $backup --data $restored
if ($LASTEXITCODE -ne 0) { throw 'Restore failed.' }
if ((Invoke-Control @('--restore', $backup, '--data', $restored)) -eq 0) { throw 'Restore overwrote an existing directory.' }
& $server --enroll --data $restored --output (Join-Path $root 'restored-enrollment.txt')
if ($LASTEXITCODE -ne 0) { throw 'Restored Store could not reopen and admit operator enrollment.' }
Write-Output "PASS service operations: private enrollment, exclusive store, readiness, stdin independence, graceful stop, backup, fresh restore and restored Store operation. Evidence: $root"
