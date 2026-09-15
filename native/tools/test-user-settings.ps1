param([string]$Executable = (Join-Path $PSScriptRoot '..\build\verdigris_user_settings_tests.exe'))
$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $Executable)) { throw 'Build the native tests first with native/build.ps1.' }
& $Executable
if ($LASTEXITCODE -ne 0) { throw 'Settings failure-handling tests failed.' }
$testRoot = Join-Path ([IO.Path]::GetTempPath()) ('verdigris-settings-process-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testRoot | Out-Null
$previousSettingsPath = $env:VERDIGRIS_SETTINGS_PATH
try {
  $env:VERDIGRIS_SETTINGS_PATH = Join-Path $testRoot 'settings.ini'
  & $Executable --write
  if ($LASTEXITCODE -ne 0) { throw 'Fresh-process settings save failed.' }
  # A separate executable invocation proves persisted state, not reused memory.
  & $Executable --reload
  if ($LASTEXITCODE -ne 0) { throw 'Fresh-process settings reload failed.' }
  Write-Host "Fresh-process evidence: $testRoot"
} finally {
  if ($null -eq $previousSettingsPath) { Remove-Item Env:VERDIGRIS_SETTINGS_PATH -ErrorAction SilentlyContinue }
  else { $env:VERDIGRIS_SETTINGS_PATH = $previousSettingsPath }
}
