---
task: TASK-0155
title: Deterministic loot playtest reliability
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: ad1a1e178e689df442d4655937f8e8e037cf4cd2
owner_visible_contribution: keeps the real kill, coin-drop, context-menu Take, and underfoot-pickup journey trustworthy under release-runner load
dependencies: []
owner_input_dependency: none
owned_paths: [playtest/scenarios/loot.mjs, orchestration/tasks/TASK-0155-deterministic-loot-playtest-reliability/**]
forbidden_paths: [server/**, src/**, native/**, tests/**, .github/**, package.json, package-lock.json, timing-budget inflation, gameplay/drop-rate changes, everything else]
resource_capsule: PLAYTEST_PORT must stay inside the routed lane range; never bind or attach to port 6500
---

# Outcome and incident evidence

Make the existing `loot` goal-harness scenario deterministically obtain and
pick up a second guaranteed coin drop without depending on ambient pack
movement or an unspecified nearest target. Protected PR #56 CI run
`32594398265` attempt 1 passed 31/32 scenarios but timed out after 52.657 s
waiting for `a second coin drop`; the same frozen tree passed its one bounded
retry. Preserve this as a load-sensitive scenario defect, not a product-rule
failure and not permission to hide or rerun failures.

The scenario must still exercise the real server, a real living non-elite
monster, real attack dispatch, the guaranteed server coin drop, the real
server-built context menu and Take action for the first stack, and the real
underfoot pickup command for the second stack. Select and retain an explicit
second target identity/precondition so polling cannot silently chase changing
pack members. The second drop must have a UUID different from the first.

# Acceptance

Run from the worker worktree with `PLAYTEST_PORT` set inside its reserved
range:

```powershell
1..5 | ForEach-Object { npm run playtest -- loot; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE } }
npm run playtest
git diff --check
git diff --name-only
```

All five focused runs and the complete goal harness must exit 0. Preserve the
authored 30 s waits and global load-mode cap. Record each focused duration and
the full 32/32 summary in REPORT.md.

# Negative controls and STOP conditions

- No timeout increase, internal retry, swallowed assertion, fake ground item,
  direct inventory mutation, dev kill/clear-floor shortcut, server/product
  change, or drop-rate change.
- Do not make the scenario pass by accepting the first coin stack twice.
- STOP if deterministic selection requires changing gameplay authority or a
  file outside `owned_paths`.

