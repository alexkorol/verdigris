---
task: TASK-0045
title: Parity wave N3 — combat and skills over the C++ server
state: READY
priority: critical (mission critical path, D-116)
owned_paths:
  - native/**
  - orchestration/tasks/TASK-0045-native-protocol-n3/**
forbidden_paths:
  - playtest/** (the harness is the measuring stick — never edit it here)
  - server/**, src/** (browser reference is read-only for this task)
base: program tip AFTER TASK-0044 integration (>= 5b84f51; verify by
  finding native/src/core.cpp WorldSimulation before branching)
architect_review_required: true
---

## Goal

The UNCHANGED playtest harness passes its combat-family scenarios
against the C++ server via `PLAYTEST_WS_URL` attach:

- `combat` (populate, target, kill within TTK bound, survive, log)
- `encounter-variety` (pack composition per zone, aura buffers, rares
  with named modifiers, aura TTK bound)
- `boss-mechanic` (named boss, telegraph radius/window, dodge vs hit,
  survivable at band)

plus a no-regression rerun of the N1/N2 set (`quickstart`,
`single-session`, `movement`, `zones`).

## Scope (RULES in core, TRANSPORT in networking — same split as N2)

1. Monsters graduate from N2 data-actors to combat actors: HP, damage,
   death, per-zone pack composition, aura buffers, rare modifiers, and
   the boss telegraph state machine — deterministic, fixed-step, in
   `verdigris_core`. Mirror the JS reference implementations the
   scenarios exercise (read them first; cite files in the REPORT the
   way 0005 did).
2. Combat verbs/events over the wire exactly as the JS server speaks
   them: whatever `combat`/`boss-mechanic` scenarios send and expect —
   attack initiation, damage application, HP updates, kill events,
   combat log lines, telegraph warnings. Payloads at `data.data`
   (protocol crib sheet in AGENTS.md).
3. Wire position and combat position reconcile: the N2 `WorldSimulation`
   tile position is the single authority the wire reports (0044 review
   note 2).
4. Drops at the minimum the combat scenarios assert (full item system
   is N4 — stub payload shapes are fine if a scenario only checks
   presence).
5. Feel constants come from the D-114 tables (seconds-to-contact,
   TTK bands); where the harness asserts a bound, the JS-derived value
   wins. No new constants without a table row.
6. Carry-over fix from 0044 review: `server_main` idles when stdin is
   closed (EOF ≠ exit; keep "quit"/"stop" working) so the server can
   run detached. Keep the 127.0.0.1 bind (standing guidance).

Stubs allowed at the minimum the scenarios exercise, each documented
with its N4/N5 successor — 0044's stub inventory is the format bar.

## Acceptance evidence (literal transcripts in REPORT.md)

1. `powershell -File native/build.ps1 -RunTests` — all three PASS lines.
2. Attach transcript: `PLAYTEST_WS_URL=ws://127.0.0.1:<port> node
   playtest/run.mjs --attach combat encounter-variety boss-mechanic
   quickstart single-session movement zones` — 7/7, harness unchanged
   (state the harness commit).
3. C++ unit coverage for the new combat rules (list the assertions).
4. One authentic negative: break one combat constant, show the harness
   catching it, restore (0043's negative-run format).

The architect will rebuild the branch and rerun the attach set
personally before ACCEPTED — plan for it, don't claim early. Two
false-green strikes on prior tasks made literal transcripts mandatory.

## Notes

- The `combat` scenario uses `dev:state` heavily; extend the snapshot
  the way N2 did rather than inventing new dev verbs.
- The boss telegraph timing is owner-felt (D-115); implement to the JS
  reference numbers, not to what merely passes.
