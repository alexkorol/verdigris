# TASK-0167 acceptance harness.
param()

$ErrorActionPreference = "Stop"
$taskDir = $PSScriptRoot
$root = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $taskDir))
$verifier = Join-Path $root "native\tools\verify_framekit_assets.py"

python $verifier
if ($LASTEXITCODE -ne 0) { throw "positive verification failed $LASTEXITCODE" }

python $verifier --corrupt
if ($LASTEXITCODE -eq 0) { throw "negative control should fail" }

python (Join-Path $root "native\tools\check_legacy_denylist.py")
if ($LASTEXITCODE -ne 0) { throw "denylist failed $LASTEXITCODE" }

Write-Host "TASK-0167 framekit raster pack harness: PASS"
