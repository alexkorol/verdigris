---
task: TASK-0044
verdict: ACCEPTED
reviewed_commits:
  - d476788
  - 16f03fa
---

## Architect verification (all rerun personally, 2026-08-17 ~09:25)

- **Scope**: branch diff vs current program tip is native/** +
  orchestration docs only; zero non-native commits; no stale-base
  reverts possible (no path overlap with 0043).
- **Build + native gates**: built the branch tip myself in an isolated
  worktree; `native/build.ps1 -RunTests` → denylist PASS, core tests
  PASS, networking tests PASS.
- **Parity gate**: started MY OWN build of `verdigris_server.exe` on
  :6511 and attached the UNCHANGED post-0043 harness from the program
  tip: `movement`, `zones`, `quickstart`, `single-session` — **4/4
  PASS** (movement 4652ms, zones 1141ms; N1 pair re-verified under the
  new session-reuse semantics).
- **Constants**: `tile_movement` mirrors `server/shared/movement.js`
  literally (150ms travel, 50ms sample, 1/3 distance, precision 6) —
  spec requirement met.
- **Loopback**: server binds/reports ws://127.0.0.1 — already complies
  with the new standing guidance (no Windows Firewall prompts).

## Judgment

The stub inventory (8 items, honestly labeled: stub floor geometry,
open-field town, no descent past depth 1, no tile-map payload, no loop,
no enter-throttle, data-only monsters) is exactly the "minimum the
scenarios exercise" the spec authorized, and each stub names its N3+
successor. The session-reuse tightening is a real semantic choice,
argued correctly and covered by the re-run N1 pair. This is the wave-N2
milestone: world, movement, and zones now hold over the C++ server
under the same harness that gates the browser game.

## Notes for N3 (carry into the next spec)

1. Real instance generator + town tile tables replace stub geometry.
2. Monsters become Simulation actors; wire position reconciles with
   combat Actor positions.
3. Depth >1 descent (`transitionFloor`, "· Floor N" naming).
4. One operational fix worth a small commit in N3: `server_main` should
   idle-loop when stdin is closed instead of exiting on EOF (running it
   detached/piped currently requires feeding stdin to keep it alive).

Integration approved; architect merges to the program branch now.
