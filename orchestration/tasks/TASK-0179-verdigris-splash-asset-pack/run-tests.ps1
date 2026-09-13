# TASK-0179 acceptance harness.
param()

$ErrorActionPreference = "Stop"
$taskDir = $PSScriptRoot
$root = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $taskDir))
$verifier = Join-Path $root "native\tools\verify_wizard_splash_assets.py"

python $verifier
if ($LASTEXITCODE -ne 0) { throw "positive verification failed $LASTEXITCODE" }

python $verifier --corrupt
if ($LASTEXITCODE -eq 0) { throw "negative control should fail" }

$denylist = Join-Path $root "native\tools\check_legacy_denylist.py"
python $denylist
if ($LASTEXITCODE -ne 0) { throw "denylist failed $LASTEXITCODE" }

Write-Host "TASK-0179 verdigris splash asset pack harness: PASS"
