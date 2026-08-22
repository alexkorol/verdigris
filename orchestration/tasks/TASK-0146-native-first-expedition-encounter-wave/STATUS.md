# TASK-0146 STATUS

state: REVIEW_REQUESTED
coordinator: ox-alpha (OpenCode PC lane)
worker: ox-pc-l
machine: DESKTOP-TVU7OR7
ports: 6840-6859
provider: openrouter
model: stealth/ox-alpha
harness: OpenCode (opencode-ai 1.18.21)
branch: codex/TASK-0146-native-first-expedition-encounter-wave-ox-pc-l-r2
routed_head: 68d5f1a361534eb4e59bc6edacaf43468fe5c09c
immutable_spec_base: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2
worktree: Z:\Code\.worktrees\verdigris\ox-pc-l
clone_path: Z:\Code\.worktrees\verdigris\ox-pc-l
started_at: 2026-08-22 03:32 -07:00

Replacement claim per RELEASE.md (2026-08-22): the stale ox-pc-d claim
(7e416ff3) is released; its quarantined worktree was not inspected, resumed,
or copied. Independent implementation from clean current program head.

## Implementation commits

- 78a0c4a0 CLAIMED STATUS
- e0ca05f6 deterministic first-expedition Warden pack wave in core
- (this commit) REVIEW_REQUESTED + REPORT

## Acceptance evidence

All literal SPEC gates green on the committed tree:
build.ps1 -RunTests -RunClientScenarios (denylist, core, networking,
camera2d, session tests, all 7 client scenarios PASS); the three named
scenarios re-run individually exit 0; `git diff --check` clean. Full
transcript, approach, constants reused, test changes, deviations, and risks
in REPORT.md.
