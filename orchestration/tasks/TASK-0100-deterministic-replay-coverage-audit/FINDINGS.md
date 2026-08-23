# TASK-0100 — Deterministic replay coverage and divergence audit

- Worker lane: `ox-sw-f`
- Base SHA: `d2423873c577d299b3b39c56024d1d840993c72b`
- Head SHA at audit completion: see `REPORT.md` (commit list); audit performed on
  `codex/TASK-0100-deterministic-replay-coverage-audit-ox-sw-f` in worktree
  `Z:\Code\.worktrees\verdigris\ox-sw-f`.
- Machine-readable surface map: `captures/replay-surfaces.json` (validates via the
  acceptance `node -e JSON.parse(...)` gate).
- Mode: READ-ONLY AUDIT (BOUNDED-DESIGN). No production file was modified.
  Every citation below is `file:line` against the base tree.

## 1. Authority verdict — no violation found

The frozen invariant holds as implemented:

- The core `Simulation` is fixed-step (`kSimulationTickMs = 50`, 20 Hz:
  `native/include/verdigris/core.hpp:34-41`) and headless. Ticks advance only
  when a command resolves (`advance_tick()`: `native/src/core.cpp:678-679`,
  called from dispatch at `native/src/core.cpp:305`). There is no wall clock,
  socket, renderer, or filesystem reference anywhere in `Simulation`.
- Commands are authoritative input; events are authoritative output
  (`CommandType`: `core.hpp:261-289`, `EventType`: `core.hpp:221-249`, events
  tick-stamped at `native/src/core.cpp:262`).
- Wall clock exists **only** behind transport seams:
  `ProtocolSession::now_ms()` (`native/src/networking.cpp:581`,
  `system_clock`) and the server 150 ms heartbeat thread
  (`native/src/networking.cpp:2713`). These feed tile-world combat/movement
  parameters but never enter the core `Simulation` math.
- Persistence is a header-only byte adapter (`native/persistence/adapter.hpp:23-84`)
  consumed by tests only; production persists nothing
  (`native/src/server_main.cpp:20-40` keeps sessions memory-only).

**Conclusion: continue. No stop condition triggered.**

## 2. RNG/clock boundary census (every stream cited)

Five distinct randomness sources; two are serialized, three are not.

| # | Stream | Algorithm | Declared / seeded / consumed | Serialized? |
|---|--------|-----------|------------------------------|-------------|
| 1 | `Simulation::Rng` | splitmix64 | `core.hpp:342-350`; seeded ctor `core.cpp:192`; ids `193,201,203,250`; drops `698-710,725,736`; resurface gate `710,725` | YES — `rng.state`+`rng.serial` written `1161-1162`, restored `1239-1240` |
| 2 | Vesselforge `Mulberry32` | mulberry32 (JS parity) | class `core.hpp:422-430`; forge stream `core.hpp:505-542`; reseeded from a caller draw `core.cpp:2736-2738, 3105-3108, 3132-3134` | NO |
| 3 | `session_rng_` | mulberry32 | `networking.hpp:240`; keyed `seed^(seed>>32)` `networking.cpp:513`; unseeded dev:give/dev:drop `992-995, 1023-1026`; client payload seed overrides | NO |
| 4 | world random | splitmix64 | `next_world_random()` `core.cpp:2999-3007`; initial state FIXED constant `core.hpp:984`; loot `3098-3101`, treasure `3123-3134`, crit roll `1890` | NO |
| 5 | monster scatter LCG | xorshift64 | local closure `core.cpp:1633-1654`; keyed off `metadata_.seed` `1637`; floor seeds = `fnv1a(theme:layout:floor-N, seed_)` `1734, 3154` (`fnv1a` at `1409-1415`) | implicit (pure fn of floor metadata) |

Seed derivations:

- Guest identity → session seed: FNV-1a inline loop in the login handler
  (`native/src/networking.cpp:2890`) and again for personal worlds on party
  leave (`native/src/networking.cpp:1503-1505`). Deterministic per identity.
- Explicit client-supplied `seed` payloads override `session_rng_` per grant:
  `networking.cpp:994-995, 1025-1026`.
- Bench default seed is deterministic: `native/tools/entity_density_bench.cpp:116-119`.

Clock boundaries (wall time may not enter core determinism — verified):

- `ProtocolSession::now_ms()` → `system_clock::now()`:
  `native/src/networking.cpp:581`. Call sites: player:move `2316`;
  dev:teleport `2317`; respawn schedule +2000 ms `2394`; skill trigger +
  combat poll `2435`; dev:state poll `2496`; first-goal timestamps
  `1556,1596`; relic `recovered_at` `1617`; wagon teleport `2545`.
- Server tick thread: `sleep_for(150ms)` + `system_clock`
  (`native/src/networking.cpp:2713`) driving `ProtocolSession::tick(now)`
  (`531-536` → `maybe_respawn` + `process_combat`).
- Combat timing lives in wall-ms units inside `WorldSimulation`:
  swing interval 350 ms (`kN3PlayerAttackIntervalMs`, `core.cpp:1401`),
  `next_player_attack_ms_` (`1812,1905`), pack attacks +1200 ms
  (`1844-1845`), boss telegraph window 1000 ms (`1406`, resolved `1919-1943`).
- Tile movement is sample-based, not timer-based: 1/3 tile per sample,
  150 ms/tile, 50 ms/sample (`core.hpp:720-735`; applied
  `core.cpp:1467-1492`; step ledger `1458-1465`).
- Presentation clocks are quarantined to the client: retry/backoff
  `steady_clock` (`native/client/remote_session.cpp:269,275,288,393`), Win32
  frame pump (`native/client/main.cpp:2329,2509`), scenario sleeps
  (`main.cpp:2869,2876,2912`). Bench clock measures only
  (`entity_density_bench.cpp:141-144`).

## 3. Existing replay / byte-equality proof (what IS covered)

Core simulation (all in `native/tests/core_tests.cpp`):

| Test | Lines | Proof strength |
|------|-------|----------------|
| `test_determinism` | 868-889 | same seed + command stream → identical `relevant()` fingerprints |
| `relevant()` helper | 69-86 | event type/ids/text/value + house/scion ids + legend tuples |
| `test_movement_replay_is_deterministic` | 245-262 | identical positions + event count |
| `test_facing_replay_is_deterministic` | 295-312 | byte-identical events + facing |
| `test_elite_skill_replay_is_deterministic` | 488-508 | telegraph windup/skill resolution identical incl. pending ticks |
| `test_war_cry_buff_expiry_and_replay_determinism` | 530-569 | buff expiry at exact tick under replay |
| `test_legend_stable_ids_and_deterministic_replay` | 1327-1343 | byte-identical legend records |
| `test_relic_resurface_replay_is_deterministic` | 703-727 | stable item identity across replays |
| `test_d106_recovery_is_ordered_and_deterministic` | 1166-1183 | recovery pool order stable |
| snapshot trio | 729-819 | schemaVersion=1 prefix (`737`); restore→snapshot byte-stable (`740`); unknown-field tolerance (`749`); D-109 RNG continuation drop identity (`787-791`); pool equality (`817`) |
| file adapter | 854-865 | atomic write/read preserves exact bytes |
| Mulberry32 JS ground truth | 1450-1465 (+ rolls 1467-1532) | C++ equals JS captures (TASK-0047) |

Client/session seam:

- `local_session_ready_and_deterministic`
  (`native/tests/session_tests.cpp:51-80`): seeded local play through the
  client adapter.
- Remote journey tests assert wire shapes and login-snapshot authority
  (`session_tests.cpp:160,425`) — connectivity, not replay equality.

Networking (`native/tests/networking_tests.cpp`):

- Envelope round trip (21), lifecycle (33), continuous movement (86),
  instance/stairs (124), N3 combat wire events (178), gate-A loot/extract/
  equip shapes (244/333/388; login snapshot fields `274`, wear totals `412`).
- **These prove shape parity, never byte-equality across a restart or an RNG
  continuation at the protocol layer.**

Gates that execute them: `./native/build.ps1 -RunTests`
(`native/README.md:30-34`), cross-platform ctest (`README.md:67-71`), CI
pipeline including client scenarios (`native/tools/ci-native.ps1:25-53`;
density bench deliberately excluded, line 7).

## 4. Risks, ranked

### R1 (HIGH) — Live tile-world state has zero capture/replay proof
`WorldSimulation` (monsters incl. `next_attack_ms`/`telegraph_until_ms`
windows, ground items + timestamps, player position/facing, forge stream,
`world_random_state_`, `serial_`, `engaged_by`, boss timers) has **no
serializer and no restore path** anywhere in `native/`. The durable snapshot
deliberately retires active floors at the D-109 boundary
(`core.cpp:1197-1223`), so every existing proof stops at "floor ends". A
mid-floor crash/restart (or future content patch mid-season) is
unreproducible today. See §6 negative control.

### R2 (HIGH) — Combat resolution granularity is poll-timing dependent
Swings resolve only when `now >= next_player_attack_ms_` is *observed*
(`core.cpp:1875`), observed either on inbound envelopes (`2435`) or on the
150 ms heartbeat (`2713`). Two identical envelope transcripts delivered with
different real-time gaps can produce different hit ordering, crit draws
(`1890` consumes a world-RNG draw only on crit-capable swings), and loot
draws (`3098-3107`). The ms timeline is an unrecorded hidden input.

### R3 (MEDIUM) — `world_random_state_` starts from a constant unrelated to `seed_`
`core.hpp:984`. Two sessions with different identity seeds begin their
loot/crit streams identically; divergence appears only after draw-count paths
separate. Any comparison trial that assumes "different seed ⇒ different early
loot" is subtly wrong until the first draw.

### R4 (MEDIUM) — Session-level streams (`session_rng_`, forge state) are unsaved
`session_rng_` (`networking.cpp:513`) and the per-session forge Mulberry32
stream advance on dev grants/gear generation but appear in no snapshot. Item
uuids minted after a restart cannot be tied to any recorded state.

### R5 (LOW) — Stray debug output in the combat hot path
`fprintf(stderr, "[swing] ...")` at `native/src/core.cpp:1874` fires on every
combat poll while a target is held. Harmless to determinism, noisy across
threads; flagged only (read-only audit — not removed).

### R6 (LOW) — Wire snapshots are shape-only
`ProtocolSession::snapshot()` (`networking.cpp:807+`) carries display state;
it contains no RNG stream state and is not canonical, so it cannot serve as a
replay checkpoint even though its name suggests one.

## 5. Smallest scaffold proposal — versioned replay record + divergence report (DEFINED, NOT IMPLEMENTED)

Design constraints honored: pure functions beside existing seams; reuse of the
already-canonical `snapshot()` bytes; no new dependencies; nothing enters the
core; presentation/transport stay outside.

### ReplayRecord v1 (capture side)

```jsonc
{
  "recordVersion": 1,                       // bump on any field change
  "producer": { "git_sha": "<head>", "schema": { "coreSnapshot": 1 } },
  "checkpoint": {
    // verbatim bytes of verdigris::snapshot(simulation) — already canonical
    // and already proven byte-stable (core_tests.cpp:729-750)
    "core_snapshot_b64": "<...>",
    // NEW pure readouts (const getters or friend serializer; no behavior change):
    "world": {
      "identity_seed_fnv1a": "<u64>",
      "scene": { "type": "...", "id": "...", "theme": "...", "layout": "...",
                 "depth": 1, "metadata_seed": "<u64>" },
      "player": { "x": 0.0, "y": 0.0, "facing": "down", "level": 1,
                  "life": 100, "life_max": 100 },
      "monsters": [ { "uuid": "...", "x": 0, "y": 0, "life": 30, "life_max": 30,
                      "alive": true, "rarity": "common", "tags": [],
                      "next_attack_ms": 0, "telegraph_until_ms": 0 } ],
      "ground_items": [ { "uuid": "...", "id": "...", "x": 0.0, "y": 0.0,
                          "timestamp": 0, "relic_record_id": "" } ],
      "streams": { "world_random_state": "<u64>", "forge_rand_state": "<u32>",
                   "serial": "<u64>", "session_rng_state": "<u32>" }
    }
  },
  "inputs": [
    // one entry per dispatched Command OR delivered envelope OR heartbeat tick
    { "i": 0, "kind": "command",          // command | envelope | heartbeat
      "at_ms": 1719999999123,             // transport now_ms; null for core-only replays
      "payload_canonical": "move:1:0" },  // Command tuple or emit_envelope text
    { "i": 1, "kind": "heartbeat", "at_ms": 1719999999273 }
  ],
  "event_fingerprints": ["<fnv1a64 of each emitted event's relevant()-style tuple>"]
}
```

Capture rules: append `(i, kind, at_ms, payload)` at the two existing choke
points only — `Simulation::dispatch` callers and
`ProtocolSession::handle`/`tick` — so the record adds no new authority. Core-
only replays set `at_ms=null` and skip heartbeats; full-stack replays include
them so R2 becomes diagnosable rather than invisible.

### DivergenceReport v1 (compare side)

```jsonc
{
  "reportVersion": 1,
  "record_version_used": 1,
  "context": { "base_sha": "...", "candidate_sha": "...", "gate": "<command>" },
  "outcome": "identical | diverged | aborted",
  "first_divergence": {
    "input_index": 42,
    "input_kind": "envelope",
    "expected_event_fingerprint": "<u64 hex>",
    "actual_event_fingerprint":   "<u64 hex>",
    "expected_core_snapshot_sha": "<sha256 of snapshot() bytes>",
    "actual_core_snapshot_sha":   "<sha256>",
    "field_path_hint": "house.legends[3].tick"   // optional diagnostic
  },
  "counts": { "inputs_replayed": 1200, "events_compared": 3400 }
}
```

Compare rules: replay inputs in order after restoring the checkpoint; after
each input compare (a) the next event fingerprint, then (b) on mismatch, the
`snapshot()` sha256 to localize. First mismatch wins; report it and stop.
Byte tools already exist: canonical snapshot bytes (`core.cpp:1158-1227`),
FNV-1a (`core.cpp:1409-1415`), atomic file adapter
(`persistence/adapter.hpp`).

Smallest concrete increment for a successor (in order):
1. Add a pure `replay_record` writer in `native/tools/` (test-side only)
   wrapping an existing test transcript — proves the format without touching
   production code.
2. Extend the D-109 RNG-continuation test pattern
   (`core_tests.cpp:752-793`) to fingerprint-per-event instead of
   end-state-only equality.
3. Add const readout getters for the §R1 world fields, then a
   resume-mid-floor byte-equality test (the negative control below).
4. Only then consider recording `at_ms` timelines for R2.

## 6. Negative control (required)

**Uncaptured surface: live WorldSimulation floor state + its wall-clock
timeline.** Full argument, uncaptured field list, and why current proofs miss
it are recorded in `captures/replay-surfaces.json` under `negative_control`.

One-line probe a successor can run: drive two sessions through an identical
envelope transcript mid-instance, varying only real-time spacing between
`player:skill:trigger` sends; hit ordering and subsequent loot draws diverge
because swing gates (`core.cpp:1875`) and crit/loot draws
(`1890`, `3098-3107`) depend on the unrecorded `now_ms` timeline — no current
artifact can detect or explain this.

## 7. Commands run during audit

- Preflight: `git status --short` (clean), `git fetch --prune origin`,
  `git status -sb`, remote inventory (no upstream on lane branch; never push).
- Surface scan (acceptance #1): exit 0, 380 matches — see REPORT.md.
- Targeted greps and reads across `native/include`, `native/src`,
  `native/tests`, `native/client`, `native/persistence`, `native/tools`
  (all citations above).
- Acceptance #2 JSON gate: PASS after fixing one syntax error in this task's
  own capture file (object closed with `]`); final run exit 0.

No core file was modified; the only writes are inside
`orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/`.
