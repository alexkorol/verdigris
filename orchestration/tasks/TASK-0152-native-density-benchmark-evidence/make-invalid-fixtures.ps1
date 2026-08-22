# Revision-1 fixture generator (TASK-0152 REVIEW correction 1).
# Derives focused negative fixtures from the committed positive capture
# density-n500-seed777-runA.json via exact textual edits. Run from repo root:
#   powershell -NoProfile -File orchestration\tasks\TASK-0152-native-density-benchmark-evidence\make-invalid-fixtures.ps1
$ErrorActionPreference = "Stop"
$taskDir = Join-Path $PSScriptRoot "."
$base = Get-Content (Join-Path $taskDir "captures\density-n500-seed777-runA.json") -Raw
$out = Join-Path $taskDir "captures\invalid"

function New-Fixture([string]$name, [hashtable]$edits) {
  $text = $base
  foreach ($key in $edits.Keys) {
    if (-not $text.Contains($key)) { throw "anchor not found for ${name}: $key" }
    $text = $text.Replace($key, $edits[$key])
  }
  $path = Join-Path $out $name
  [System.IO.File]::WriteAllText($path, $text)
  Write-Host "wrote $name"
}

# Check value disagrees with its evidence source field (stored pass stays true).
New-Fixture "tampered-check-value.json" @{
  '{"id": "update_p99_within_budget", "value": 0.004100000' =
    '{"id": "update_p99_within_budget", "value": 0.001100000'
}

# Contract bound loosened below the documented row.
New-Fixture "tampered-check-bound.json" @{
  '{"id": "ticks_per_sec_floor", "value": 2457.315810050, "op": "min", "bound": 20.000000000' =
    '{"id": "ticks_per_sec_floor", "value": 2457.315810050, "op": "min", "bound": 10.000000000'
}

# Operator flipped against the contract.
New-Fixture "tampered-check-op.json" @{
  '{"id": "frame_p99_within_budget", "value": 1.119100000, "op": "max"' =
    '{"id": "frame_p99_within_budget", "value": 1.119100000, "op": "min"'
}

# Stored pass result contradicts the healthy evidence.
New-Fixture "tampered-check-pass.json" @{
  '{"id": "reproducible", "value": 1.000000000, "op": "min", "bound": 1.000000000, "pass": true}' =
    '{"id": "reproducible", "value": 1.000000000, "op": "min", "bound": 1.000000000, "pass": false}'
}

# Genuinely threshold-failing frame p99 kept green by a fabricated pass flag:
# the check value matches the tampered timings field exactly and percentiles
# stay monotonic, so ONLY recomputation from bound/value can reject it.
New-Fixture "tampered-threshold-fail.json" @{
  '"frame_ms": {"p50": 0.372700000, "p90": 0.385100000, "p99": 1.119100000, "max": 1.131300000, "mean": 0.406781700}' =
    '"frame_ms": {"p50": 0.372700000, "p90": 0.385100000, "p99": 61.119100000, "max": 61.131300000, "mean": 0.406781700}'
  '{"id": "frame_p99_within_budget", "value": 1.119100000, "op": "max", "bound": 50.000000000, "pass": true}' =
    '{"id": "frame_p99_within_budget", "value": 61.119100000, "op": "max", "bound": 50.000000000, "pass": true}'
}

# Fixed scenario route binding violated.
New-Fixture "tampered-route.json" @{
  '"route": "route:tin:1:0"' = '"route": "route:tin:9:9"'
}

# Fixed scenario action binding violated.
New-Fixture "tampered-action.json" @{
  '"action": "Melee"' = '"action": "Sweep"'
}
