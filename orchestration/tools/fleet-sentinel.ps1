# fleet-sentinel.ps1 - deterministic fleet-health check for the Verdigris fleet.
# No LLM, no network, no credits: reads local git + fleet logs, writes an alert
# file when the fleet needs owner attention, stays quiet when green.
# Designed for Windows Task Scheduler (every 30 min). PowerShell 5.1 compatible.
#
# Durable state (RUN_STATUS.md, LEADER_BRIEF files) is read from the program
# BRANCH via git show, never from the working tree - lanes have been observed
# switching the architect checkout to their own branches, and the sentinel must
# not go blind when that happens (it flags it instead).
#
# Red conditions:
#   1. No commit on any local branch within -StaleHours.
#   2. Owner-attention flags in the newest RUN_STATUS.md heartbeat on the
#      program branch ("human decision needed: YES", "healthy: false").
#   3. A LEADER_BRIEF.md on the program branch still AWAITING OWNER RULING.
#   4. New nonzero EXIT= lines in .fleet\logs\launcher-exits.txt since last run.
#   5. Architect checkout not on the program branch (tree hijack).
#   6. The sentinel itself cannot read the repo (fails loud, never silent-green).
#
# Green: appends one line to sentinel.log and removes any previous alert file.
# The log line doubles as the sentinel's own liveness proof.

param(
    [string]$RepoRoot      = "Z:\Code\Games\delaford\delaford_game",
    [string]$ProgramBranch = "codex/native-reconstitution",
    [string]$FleetDir      = "Z:\Code\.fleet",
    [string]$AlertPath     = "$env:USERPROFILE\Desktop\FLEET-ALERT.md",
    [double]$StaleHours    = 6
)

$logDir    = Join-Path $FleetDir "logs"
$logPath   = Join-Path $logDir "sentinel.log"
$statePath = Join-Path $FleetDir "sentinel-state.json"
if (-not (Test-Path $logDir)) { New-Item -ItemType Directory -Force $logDir | Out-Null }

$now     = Get-Date
$stamp   = $now.ToString("yyyy-MM-ddTHH:mm:ssK")
$reasons = @()

# --- 1. Newest commit age across all local branches -------------------------
$newestUnix = [long]0
$gitOut = git -C $RepoRoot for-each-ref --format="%(committerdate:unix)" refs/heads 2>$null
if ($LASTEXITCODE -ne 0 -or -not $gitOut) {
    $reasons += "SENTINEL FAULT: cannot read git refs in $RepoRoot"
} else {
    foreach ($line in $gitOut) {
        $u = [long]0
        if ([long]::TryParse($line.Trim(), [ref]$u)) {
            if ($u -gt $newestUnix) { $newestUnix = $u }
        }
    }
    $newestUtc = [DateTimeOffset]::FromUnixTimeSeconds($newestUnix).UtcDateTime
    $ageHours  = ($now.ToUniversalTime() - $newestUtc).TotalHours
    if ($ageHours -gt $StaleHours) {
        $reasons += ("STALE: no commit on any branch for {0:N1}h (threshold {1}h)" -f $ageHours, $StaleHours)
    }
}

# --- 2. Owner-attention flags in newest RUN_STATUS heartbeat (from branch) --
$runStatusRef = "{0}:orchestration/RUN_STATUS.md" -f $ProgramBranch
$rsLines = git -C $RepoRoot show $runStatusRef 2>$null
if ($LASTEXITCODE -ne 0 -or -not $rsLines) {
    $reasons += "SENTINEL FAULT: cannot read RUN_STATUS.md from branch $ProgramBranch"
} else {
    $head = @($rsLines)[0..([Math]::Min(44, @($rsLines).Count - 1))]
    if ($head -match "human decision needed:\s*YES") {
        $reasons += "OWNER DECISION NEEDED: flagged in RUN_STATUS.md newest heartbeat"
    }
    if ($head -match "healthy:\s*false") {
        $reasons += "BOARD UNHEALTHY: board sentinel reports healthy: false (likely backlog exhausted)"
    }
}

# --- 3. Pending Tier C leader briefs (from branch) --------------------------
$treeFiles = git -C $RepoRoot ls-tree -r --name-only $ProgramBranch -- orchestration/tasks 2>$null
if ($LASTEXITCODE -eq 0 -and $treeFiles) {
    foreach ($f in $treeFiles) {
        if ($f -notmatch "LEADER_BRIEF\.md$") { continue }
        $content = git -C $RepoRoot show ("{0}:{1}" -f $ProgramBranch, $f) 2>$null
        if ($content -match "AWAITING OWNER RULING") {
            $taskDir = Split-Path (Split-Path $f -Parent) -Leaf
            $reasons += ("TIER C RULING PENDING: {0}" -f $taskDir)
        }
    }
}

# --- 4. New nonzero launcher exits since last run ---------------------------
$exitsPath = Join-Path $logDir "launcher-exits.txt"
$lastCount = 0
if (Test-Path $statePath) {
    try {
        $state = Get-Content $statePath -Raw | ConvertFrom-Json
        if ($state.exitLineCount) { $lastCount = [int]$state.exitLineCount }
    } catch {}
}
$curCount = 0
if (Test-Path $exitsPath) {
    $exitLines = @(Get-Content $exitsPath)
    $curCount = $exitLines.Count
    if ($curCount -gt $lastCount) {
        $newLines = $exitLines[$lastCount..($curCount - 1)]
        $bad = @($newLines | Where-Object { $_ -match "EXIT=(?!0\b)\d+" })
        if ($bad.Count -gt 0) {
            $reasons += ("LAUNCHER FAILURES: {0} new nonzero exit(s), e.g. `"{1}`"" -f $bad.Count, $bad[0].Trim())
        }
    }
}

# --- 5. Architect checkout identity -----------------------------------------
$treeBranch = (git -C $RepoRoot rev-parse --abbrev-ref HEAD 2>$null)
if ($treeBranch -and $treeBranch -ne $ProgramBranch) {
    $reasons += ("TREE HIJACKED: architect checkout is on '{0}', expected '{1}'" -f $treeBranch, $ProgramBranch)
}

# --- Persist state + verdict ------------------------------------------------
@{ lastRun = $stamp; exitLineCount = $curCount } |
    ConvertTo-Json | Set-Content -Encoding utf8 $statePath

if ($reasons.Count -eq 0) {
    Add-Content -Encoding utf8 $logPath "$stamp GREEN"
    if (Test-Path $AlertPath) { Remove-Item -Force $AlertPath }
    exit 0
}

Add-Content -Encoding utf8 $logPath ("$stamp RED " + ($reasons -join " | "))
$body = @()
$body += "# FLEET ALERT - $stamp"
$body += ""
$body += "The Verdigris fleet needs attention:"
$body += ""
foreach ($r in $reasons) { $body += "- $r" }
$body += ""
$body += "Details: Z:\Code\.fleet\logs\sentinel.log and orchestration/RUN_STATUS.md"
$body += "on the program branch. This file is sentinel-managed: it is removed"
$body += "automatically once every condition above clears."
$body -join "`r`n" | Set-Content -Encoding utf8 $AlertPath
exit 1
