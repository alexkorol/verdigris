param([Parameter(Mandatory=$true)][string]$ServerExecutable, [string]$EvidenceDirectory, [ValidateRange(10,600)][int]$Seconds=120)
$ErrorActionPreference='Stop'
if (-not $EvidenceDirectory) { $EvidenceDirectory=Join-Path $PSScriptRoot ('../build/service-soak-'+[Guid]::NewGuid().ToString('N')) }
$root=[IO.Path]::GetFullPath($EvidenceDirectory)
if (Test-Path -LiteralPath $root) { throw 'Use a fresh soak evidence directory.' }
New-Item -ItemType Directory -Path $root | Out-Null
$source=[IO.Path]::GetFullPath($ServerExecutable)
$sourceHash=(Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash
$server=Join-Path $root 'verdigris_server.exe'
Copy-Item -LiteralPath $source -Destination $server
if ((Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash -ne $sourceHash -or (Get-FileHash -LiteralPath $server -Algorithm SHA256).Hash -ne $sourceHash) { throw 'Executable changed during copy.' }
$identity=(& $server --build-info | Out-String).Trim()
if ($LASTEXITCODE -ne 0) { throw 'Cannot read copied server identity.' }
$data=Join-Path $root 'data'; $credentials=@()
for ($i=0;$i -lt 4;$i++) { $private=Join-Path $root ('enrollment-'+$i+'.txt'); & $server --enroll --data $data --output $private | Out-Null; if ($LASTEXITCODE -ne 0) { throw 'Disposable enrollment failed.' }; $credentials+=(Get-Content -LiteralPath $private -Raw).Trim() }
$probe=New-Object Net.Sockets.TcpListener([Net.IPAddress]::Loopback,0); $probe.Start(); $port=$probe.LocalEndpoint.Port; $probe.Stop()
$info=New-Object Diagnostics.ProcessStartInfo
$info.FileName=$server; $info.Arguments='--service --data "'+$data+'" --port '+$port+' --qa-policy coop-v1'
$info.UseShellExecute=$false; $info.CreateNoWindow=$true; $info.RedirectStandardInput=$true; $info.RedirectStandardOutput=$true; $info.RedirectStandardError=$true
$process=[Diagnostics.Process]::Start($info); $stdout=$process.StandardOutput.ReadToEndAsync(); $stderr=$process.StandardError.ReadToEndAsync()
function Invoke-Control([string[]]$Arguments) { $ErrorActionPreference='Continue'; & $server @Arguments 2>&1 | Out-Null; return $LASTEXITCODE }
try {
  $ready=$false; $deadline=[DateTime]::UtcNow.AddSeconds(10)
  while ([DateTime]::UtcNow -lt $deadline -and -not $process.HasExited) { if ((Invoke-Control @('--health','--data',$data)) -eq 0) { $ready=$true; break }; Start-Sleep -Milliseconds 100 }
  if (-not $ready) { throw 'Copied service did not become ready.' }
  Add-Type -Path (Join-Path $PSScriptRoot 'service-soak.cs') -ReferencedAssemblies System.dll,System.Core.dll,System.Web.Extensions.dll
  $result=[VerdigrisSoak]::Run($port,$process.Id,[string[]]$credentials,$Seconds) | ConvertFrom-Json
  $result | Add-Member sourceExecutable $source
  $result | Add-Member copiedExecutableSha256 $sourceHash.ToLowerInvariant()
  $result | Add-Member embeddedIdentity $identity
  $result | Add-Member scriptSha256 (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash.ToLowerInvariant()
  $result | Add-Member helperSha256 (Get-FileHash -LiteralPath (Join-Path $PSScriptRoot 'service-soak.cs') -Algorithm SHA256).Hash.ToLowerInvariant()
  $result | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath (Join-Path $root 'soak-results.json') -Encoding UTF8
  if ((Invoke-Control @('--health','--data',$data)) -ne 0) { throw 'Service lost readiness after soak.' }
  Write-Output "PASS: four clients, two instances, injected directional delay/jitter, interrupted authenticated recovery. Evidence: $root"
} catch {
  ('Stage: '+[VerdigrisSoak]::Stage+[Environment]::NewLine+$_.Exception.ToString()) | Set-Content -LiteralPath (Join-Path $root 'soak-failure.txt') -Encoding UTF8
  throw
} finally {
  $credentials=$null
  if (-not $process.HasExited) { $null=Invoke-Control @('--stop','--data',$data); if (-not $process.WaitForExit(5000)) { $process.Kill(); $process.WaitForExit() } }
  $stdout.Result | Set-Content -LiteralPath (Join-Path $root 'service-stdout.log')
  $stderr.Result | Set-Content -LiteralPath (Join-Path $root 'service-stderr.log')
  $process.Dispose()
}
