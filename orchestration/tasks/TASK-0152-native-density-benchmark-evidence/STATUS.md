# STATUS — TASK-0152-native-density-benchmark-evidence

state: REVIEW_REQUESTED
task: TASK-0152-native-density-benchmark-evidence
worker (lane): ox-pc-ac
coordinator: ox-alpha
provider: openrouter
model alias: stealth/ox-alpha
harness: OpenCode CLI 1.18.21, variant max
endpoint: https://openrouter.ai/api/v1 (harness-managed; no listener used)
ports capsule: 7160-7179 loopback only — this task uses none of them and never
touches port 6500.
machine: DESKTOP-TVU7OR7 (user Alex, Windows, win32/pwsh host)
clone path / worktree: Z:\Code\.worktrees\verdigris\ox-pc-ac
branch: codex/TASK-0152-native-density-benchmark-evidence-ox-pc-ac
routed base/head SHA at provisioning: c2b814488278f4f093e754cf695ea9ed749d81fb
  (verified: HEAD equals this SHA; working tree clean before claim)
spec base_commit: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2
  (verified ancestor of the routed base HEAD)
task family: IMPLEMENTATION / topology INDEPENDENT / priority P1 / state READY
configuration provenance: launched from local packet START_HERE_OX_PC_AC.md;
  read AGENTS.md, orchestration/PROTOCOL.md, native/README.md,
  orchestration/REENTRY-OX-ALPHA-PC.md, and the complete task SPEC.md before
  claiming. `git fetch --prune origin` performed; program branch observed at
  origin/codex/native-reconstitution = 72a11d83c16772b19995957d402f70f9516241ac.
claim basis: first STATUS write wins — no STATUS.md or RELEASE.md existed for
  this task on origin/codex/native-reconstitution at the SHA above when this
  claim was written. Owned paths:
  native/tools/entity_density_bench.cpp,
  orchestration/tasks/TASK-0152-native-density-benchmark-evidence/**.
  All other paths treated as read-only.
started_at: 2026-08-22T20:18:16Z

transition:
  state: REVIEW_REQUESTED
  at: 2026-08-22T21:05:00Z (UTC, approximate completion of acceptance runs)
  claim_commit: 5156c33e826481f6e427498b0b755f35245c3ae2
  evidence:
    - captures/density-n{50,500,1000}-seed*-run{A,B}.json (six seeded process
      invocations; A/B agree on counts and fnv1a64 state checksums; all
      threshold checks pass; --validate exit 0)
    - captures/invalid/{truncated,garbage,missing-provenance,repro-fail,
      incomplete-percentiles}.json (all fail --validate with exit 1)
  gates:
    - native/build.ps1 -RunTests EXIT=0 (unit gates + legacy denylist green)
    - bench compiled with build.ps1's exact cl flags, /W4 clean, EXIT=0
    - git diff --check clean
  report: REPORT.md (executive summary, transcripts, deviations, risks)