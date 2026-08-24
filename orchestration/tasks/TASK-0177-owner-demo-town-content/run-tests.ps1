# TASK-0177 acceptance harness.
param()

$ErrorActionPreference = "Stop"
$taskDir = $PSScriptRoot
$root = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $taskDir))
$validator = Join-Path $taskDir "verify_owner_demo_town.py"

python $validator
if ($LASTEXITCODE -ne 0) { throw "positive validation failed $LASTEXITCODE" }

python $validator --negative
if ($LASTEXITCODE -ne 0) { throw "negative control failed $LASTEXITCODE" }

$denylist = Join-Path $root "native\tools\check_legacy_denylist.py"
python $denylist
if ($LASTEXITCODE -ne 0) { throw "denylist failed $LASTEXITCODE" }

Write-Host "TASK-0177 owner demo town content harness: PASS"
