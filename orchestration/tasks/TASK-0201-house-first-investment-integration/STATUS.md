---
task: TASK-0201
state: BRIDGE_PREP
worker: cursor (composer-2.5)
worker_branch: codex/TASK-0201-house-investment-layout-prep-cursor
worktree: Z:\Code\.worktrees\verdigris\owner-demo-runway
routed_head: f1e636605cf9d1d8b466edde22b50dd6647a8988
started_at: 2026-08-24T11:08:00Z
heartbeat_at: 2026-08-24T14:30:00Z
completed_at: 2026-08-24T14:30:00Z
heartbeat_minutes: 15
lease_minutes: 40
claim: durable first-STATUS-write-wins; path-disjoint integrator prep header
note: path-disjoint investment dialog layout; main.cpp blocked on lease + 0190/0200 ACCEPTED
---

House investment layout (`house_investment_layout.hpp`) delivered for integrator lane.

Evidence:
- `run-tests.ps1` exit 0 (15 checks)
- `check_legacy_denylist.py` PASS
- `playtest-tip-evidence.txt` — 32/32 exit 0 (port 6510)
