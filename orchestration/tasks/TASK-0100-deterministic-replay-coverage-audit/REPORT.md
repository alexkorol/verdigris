# TASK-0100 REPORT — Deterministic replay coverage and divergence audit

- Worker: `ox-sw-f` · Coordinator: codex · Packet: BOUNDED-DESIGN · Priority P0
- Branch: `codex/TASK-0100-deterministic-replay-coverage-audit-ox-sw-f`
- Worktree: `Z:\Code\.worktrees\verdigris\ox-sw-f`
- Base: `d2423873c577d299b3b39c56024d1d840993c72b` · Final head: see §6
- Mode: read-only audit. No production code modified; owned task folder only.

## 1. Executive summary

Audited every RNG stream, seed derivation, tick/cadence mechanism, clock
boundary, snapshot surface, persistence boundary, networking adapter, and
existing byte-equality/replay test under `native/include`, `native/src`,
`native/tests`. The frozen invariant holds: the core `Simulation` is fixed-step
(50 ms / 20 Hz, `core.hpp:34-41`, tick advance `core.cpp:678-679`),
command-driven, and free of wall clock/sockets/renderer/filesystem; wall time
is confined to transport seams (`networking.cpp:581`, `:2713`). Two of five RNG
streams are snapshot-serialized (`rng.state`/`rng.serial`, `core.cpp:1161-1162`);
three are not. Core replay proof is strong (11 dedicated determinism/byte-
equality tests), but live tile-world floor state has **no** capture/replay
proof at all — that is the negative control. A versioned ReplayRecord v1 +
DivergenceReport v1 contract is fully defined (not implemented) in
`FINDINGS.md §5`, reusing the already-canonical `snapshot()` bytes and existing
choke points. No authority violation; no stop.

## 2. Approach

1. Preflight (clean tree at base, fetch --prune, remote inventory; lane branch
   intentionally has no upstream — local commits only).
2. Ran the acceptance rg scan verbatim (380 matches) to enumerate surfaces.
3. Deep-read `core.hpp/core.cpp` (Rng, Mulberry32/VesselForge, WorldSimulation,
   fnv1a seeds, snapshot/restore), `networking.hpp/cpp` (session_rng_, clock
   call sites, 150 ms heartbeat thread, identity→FNV seed), persistence
   adapter, client adapters (`local_session.cpp`, `remote_session.cpp`,
   `main.cpp` timers), tools (bench, ci-native.ps1), and all three test files.
4. Cross-checked every claim to `file:line`; recorded machine-readable map in
   `captures/replay-surfaces.json`; ranked risks; defined the successor
   contract in `FINDINGS.md`.
5. Negative control identified and argued (§4 below).

## 3. Changed files

```
orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/STATUS.md        (claim → review states)
orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/FINDINGS.md      (audit evidence + contract)
orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/captures/replay-surfaces.json
orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/REPORT.md        (this file)
```

## 4. Test commands + outcomes

All run verbatim from worktree root (PowerShell), final pass:

| # | Command | Exit | Outcome |
|---|---------|------|---------|
| 1 | `rg -n "seed\|rng\|random\|tick\|fixed\|replay\|snapshot\|determin\|clock\|time" native/include native/src native/tests` | 0 | 380 matches across include/src/tests; census recorded |
| 2 | `node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/captures/replay-surfaces.json','utf8')); console.log('replay surfaces: PASS')"` | 0 | `replay surfaces: PASS` |
| 3 | `git diff --check` | 0 | clean (no whitespace/conflict markers) |
| 4 | `git diff --name-only` | 0 | empty working tree; base-diff `d2423873..HEAD` contains ONLY the four task-folder files |

Note on #2: first historical run failed (exit 1, ENOENT) because the JSON had
not yet been written, then failed once more on a syntax error inside my own
capture file (object closed with `]`); both were fixed within the owned folder;
the verbatim command now passes. No production gate was touched.

Negative control (required): recorded in `FINDINGS.md §6` and
`captures/replay-surfaces.json → negative_control` — live WorldSimulation
floor state (monsters/timers, ground items, forge + world RNG streams, engaged
flags) and its wall-clock timeline are captured by no current replay proof;
D-109 deliberately retires floors at snapshot (`core.cpp:1197-1223`) and no
test restarts a mid-floor world.

## 5. Key findings

1. Authority verified: core is deterministic by construction; all wall-clock
   entry points cited (`now_ms()` networking.cpp:581 with 8 call sites; 150 ms
   heartbeat thread :2713; presentation clocks quarantined client-side).
2. Five RNG streams mapped (splitmix64 core [serialized], VesselForge mulberry32,
   session_rng_, world random, scatter LCG) — three unserialized.
3. R1/R2 HIGH: no live-floor replay path; tile-world combat resolution depends
   on unrecorded `now_ms` polling gaps (swing gate `core.cpp:1875`, crit draw
   `:1890`, loot draws `:3098-3107`).
4. R3 MEDIUM: `world_random_state_` initializes from a fixed constant
   (`core.hpp:984`), not from the session seed.
5. R5 LOW: stray `fprintf(stderr,"[swing] ...")` debug in combat hot path
   (`core.cpp:1874`) — flagged only, not modified.

Full detail: `FINDINGS.md`.

## 6. Commit SHAs

| SHA | Message |
|-----|---------|
| `143d4890` | chore(TASK-0100): claim lane ox-sw-f |
| `c093424f` | audit(TASK-0100): replay surface census, risks, replay-record contract |
| `<final>` | report(TASK-0100): lane ox-sw-f evidence complete (this commit) |

Final head SHA after the last commit: `git rev-parse HEAD` at close — recorded
in the final report-back message.

## 7. Deviations

- None blocking. Two notes:
  - SPEC.md was not present on this worktree's branch (base predates it); read
    in full from the coordinator checkout at
    `Z:\Code\Games\delaford\delaford_game\orchestration\tasks\TASK-0100-...​/SPEC.md`
    and treated as binding. The task folder itself is owned by this lane, so
    all artifacts were created here as instructed.
  - One JSON syntax fix inside my own capture file before the acceptance gate
    passed (see §4 note).

## 8. Risks

- Findings reflect base `d2423873`; later merges may shift cited line numbers.
- The defined contract (FINDINGS §5) intentionally leaves "const readout
  getters for WorldSimulation" as successor work — until then R1 remains open.
- `session_tests` binds ports 6572–6579 (capsule); this audit ran no servers
  and opened no ports (read-only capsule honored).

## 9. Follow-ups (successor queue)

1. Implement ReplayRecord v1 writer as a test-side tool first (no production
   change), per FINDINGS §5 increment list.
2. Add resume-mid-floor byte-equality test once world readout getters exist
   (closes negative control).
3. Derive `world_random_state_` initial value from session seed (R3) — one-line
   core change requiring its own determinism re-run.
4. Remove `[swing]` debug fprintf (`core.cpp:1874`).
5. Consider recording `at_ms` timelines to make R2 diagnosable end-to-end.
