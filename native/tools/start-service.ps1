param(
  [Parameter(Mandatory = $true)][string]$DataDirectory,
  [ValidateRange(1,65535)][int]$Port = 6540,
  [Parameter(Mandatory = $true)][ValidateSet('coop-v1')][string]$QaPolicy
)
$ErrorActionPreference = 'Stop'
$server = Join-Path $PSScriptRoot 'verdigris_server.exe'
if (-not (Test-Path -LiteralPath $server)) { $server = Join-Path $PSScriptRoot '../build/verdigris_server.exe' }
$data = [IO.Path]::GetFullPath($DataDirectory)
if (-not (Test-Path -LiteralPath $data)) {
  $security = New-Object Security.AccessControl.DirectorySecurity
  $security.SetAccessRuleProtection($true, $false)
  foreach ($sid in @([Security.Principal.WindowsIdentity]::GetCurrent().User, (New-Object Security.Principal.SecurityIdentifier('S-1-5-18')))) {
    $rule = New-Object Security.AccessControl.FileSystemAccessRule($sid, 'FullControl', 'ContainerInherit,ObjectInherit', 'None', 'Allow')
    $security.AddAccessRule($rule)
  }
  [IO.Directory]::CreateDirectory($data, $security) | Out-Null
}
# Keep the process attached to its operator/process manager. Service mode does
# not read stdin; closing a client has no relationship to this process.
& $server --service --data $data --port $Port --qa-policy $QaPolicy
exit $LASTEXITCODE
