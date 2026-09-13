# TASK-0100 — Deterministic replay coverage and divergence audit (FINDINGS)

- Lane: `ox-pc-bd` · Model: `openrouter/stealth/ox-alpha`
- Base commit: `d2423873c577d299b3b39c56024d1d840993c72b`
- Audited head: `b0a09741b1542fd0a8227aa4d6f9d78a6358be76`
- Machine-readable companion: [`captures/replay-surfaces.json`](captures/replay-surfaces.json)
  (`$contract: verdigris.audit.replay-surfaces`, version 1)
- Authority: `docs/product/VERDIGRIS_CONSTITUTION.md:157-173` fixes the native
  invariant — "The simulation is fixed-step, seeded where practical, headless,
  and command/event driven." This audit changes no code; every claim below is a
  citation into the tree at the audited head.

## Method

1. Ran the spec gate verbatim:
   `rg -n "seed|rng|random|tick|fixed|replay|snapshot|determin|clock|time" native/include native/src native/tests`
   and triaged every hit into the surface classes below (full transcript in
   REPORT.md).
2. Read the seam headers/implementations end to end: `core.hpp` (1017 lines),
   the snapshot/restore block of `core.cpp`, `networking.cpp`
   session/server/tick-thread regions, `persistence/adapter.hpp`,
   `seasonal.hpp`, `local_session.{hpp,cpp}`, the scenario harness in
   `client/main.cpp`, and the seam READMEs (`networking`, `persistence`,
   `platform`, `renderer`, `content`).
3. Cross-checked each determinism claim against an existing test or recorded a
   gap. Nothing was patched; core was treated as read-only per spec.

## 1. Command surface (replayable today — strongest seam)

- Vocabulary: `CommandType` + factories at `native/include/verdigris/core.hpp:264-292`;
  `ActionType` values are explicitly frozen for recorded streams
  (`core.hpp:25-27`) and event ordinals are append-only so stored numeric codes
  survive (`core.hpp:249-252`). These two stability comments are the de facto
  command/event contract any record format must pin.
- Resolution model: one `dispatch()` = one command resolved = one
  `advance_tick()` (`native/src/core.cpp:309`, `722-742`). Time only advances
  through commands; there is no timer inside the core. A replay therefore needs
  the *complete* command stream including explicit `Wait`s used to burn ticks.
  The scenario harness demonstrates exactly this discipline
  (`native/client/main.cpp:4167-4188`: fixed seed `0xC011AB1E`, then
  `scenario_step(state, Command::…)`).
- Cadence constants: `kSimulationTickMs=50` / `movement_step_per_tick`
  (`core.hpp:34-41`), telegraph `kTelegraphTicks=3` (`core.hpp:29-32`),
  dash burst 10 ticks (`core.hpp:75-76`), D-114 world-scale table derived from
  the tick (`core.hpp:43-73`). Presentation pins the same 50 ms tick
  (`native/client/presentation_events.hpp:49`, asserted
  `presentation_events_tests.cpp:61`) and audio orders cues by `scheduled_tick`
  (`native/audio/cue_spec.hpp:47`, proven `audio_mixer_tests.cpp:213-230`).
- Client translation is a pure mapping (`native/client/local_session.cpp:114-159`),
  so client replay fixtures can log `ClientCommand`s losslessly.

## 2. Seeds

| Seed | Where | Notes |
| --- | --- | --- |
| Simulation seed | `native/src/core.cpp:192` | Directly seeds the splitmix64-style core Rng |
| World seed | `native/src/core.cpp:1493-1494` | Per-session tile-space world |
| Instance/floor seed | `fnv1a(theme+":"+layout, seed_)` `core.cpp:1802`, floor variant `core.cpp:3222` | Deterministic per-house roads also derive from it (`networking.cpp:1365`) |
| Guest identity seed | FNV-1a over identity → uint64 (`networking.cpp:1575-1577`, login path `2988`) | Same identity ⇒ same seed ⇒ reproducible guest |
| Session RNG seed | `session_rng_(uint32(seed ^ seed>>32))` (`networking.cpp:579`) | **Lossy truncation** to 32 bits |
| Dev-command seeds | optional payload seed → local Mulberry32 (`networking.cpp:1064-1067`, `1095-1098`) | Without it, draws consume the unrecorded session stream |

Risk: the identity→session-RNG truncation means two distinct 64-bit seeds can
collide onto one 32-bit Mulberry32 state; acceptable for dev tooling, but a
record contract should carry the original 64-bit seed plus the derived value.

## 3. RNG streams (five distinct streams)

1. **Core `Simulation::Rng`** — additive counter + splitmix64 finalizer +
   serial-suffixed tokens (`core.cpp:140-154`). Fully serialized:
   `rng.state` + `rng.serial` in `snapshot()` (`core.cpp:1229-1230`) and
   restored verbatim (`core.cpp:1307-1308`). Proven to continue identically
   across save/load (`core_tests.cpp:778-818`). This is the model citizen.
2. **`Mulberry32`** JS-parity helper (`core.hpp:450-459`) — used for item/forge
   parity when callers pass `CreateItemOptions.rng` (`core.hpp:628`).
   Not serialized; lifetime owned by callers.
3. **`VesselForge::rand_`** persistent forge stream (`core.hpp:534-542`),
   reseeded by consuming exactly one draw of another stream
   (`core.cpp:2804-2806`, `3173-3176`). Lives inside `WorldSimulation`; **no
   capture path**.
4. **`world_random_state_`** splitmix64 (`core.hpp:1013`,
   `core.cpp:3067-3072`) — drives monster scatter, loot chance/pool picks, coin
   rolls (`core.cpp:1701-1705`, `3166-3202`). Advances constantly, **captured
   nowhere**.
5. **`session_rng_`** (`networking.cpp:579`) — dev-tool draws; **captured
   nowhere**.

Only stream 1 participates in any byte-equality guarantee. Streams 3–5 are the
divergence reservoir documented in §6.

## 4. Ticks vs clocks

- Core purity holds: `rg chrono|system_clock|steady_clock` over
  `native/include` + `native/src` matches only `server_main.cpp` (keep-alive
  sleep) and `networking.cpp` (transport timing). The headless simulation's
  time base is the command-driven tick counter alone.
- Wall-clock boundaries that *do* enter outcomes, all at the transport edge:
  - `ProtocolSession::now_ms()` = `system_clock` (`networking.cpp:653`);
  - server tick thread: ~150 ms sleep loop feeding `system_clock` now into
    every session (`networking.cpp:2811`) — arrival jitter is real;
  - combat deadlines are wall-clock ms, not ticks:
    `next_attack_ms = now + 1200/1500`, player attack interval, boss
    `telegraph_until_ms` windows (`core.cpp:1880`, `1912-1913`, `1973`,
    `1987-2019`); the code itself calls this "real-clock cadence"
    (`networking.cpp:2507`);
  - respawn protection / first-goal timestamps reset as raw ms values
    (`networking.cpp:635-640`);
  - ground-item menu order uses a monotonic `++serial_` timestamp
    (`core.cpp:3090`, `3102`; sort at `networking.cpp:1892`).
- Measurement-only clocks (harness/tool backoff, throttles, benchmark timers)
  stay outside outcomes: `session_tests.cpp:42-64`,
  `remote_session.cpp:350-370,515-517`, `server_lifecycle_soak.cpp:41-57`,
  `entity_density_bench.cpp:281-321`.

Consequence: the *core* layer replays byte-for-byte from commands; the
*tile-space world* layer (N2/N3/N4 surfaces) does not, because its cadence is
wall-clock and its state is uncaptured.

## 5. Snapshots and persistence

- Canonical bytes: free-function `snapshot()/restore()`
  (`core.hpp:430-434`), schemaVersion-gated line format with fixed field order
  (writer `core.cpp:1226-1295`, reader `1297-1351`, version check `1299-1301`).
  Unknown fields tolerated on restore, malformed required fields fail loudly
  (`core_tests.cpp:761-775`) — stale persisted data degrades gracefully, which
  the AGENTS boundary requires us to preserve in any successor format.
- Deliberate exclusions: live instance state (actors, drops, events, pending
  wave) never persists (`core.hpp:344-350`); D-109 retires an active instance
  at the boundary while surfaced candidates return to pending queues
  (`core.cpp:1265-1291`), all proven by tests (`core_tests.cpp:778-877`).
- File adapter: atomic temp+rename with Windows `MoveFileExW` replacement
  (`persistence/adapter.hpp:23-73`). Round-trip proven only by unit test
  (`core_tests.cpp:880-892`). **No production caller exists** — neither
  `verdigris_server` nor the client saves/loads House bytes, so cross-restart
  replay is structurally unproven even though every ingredient works.
- The wire `snapshot()` JSON (`networking.cpp:879-984`) is a lossy projection;
  reconnect treats it as authoritative (`session_tests.cpp:443`, chronicle
  equality `1965/2011`), but it is not a replay artifact and must not be
  conflated with canonical bytes. Note also the unrelated `ServerParty
  snapshot` copies (`networking.cpp:2906-2979`) — naming collision worth
  cleaning up before a contract lands.

## 6. Divergence risks (ranked) and the negative control

**Negative control (primary): `WorldSimulation` live state has no capture path
at all.** Player position/scene, monsters (with their wall-clock
`telegraph_until_ms`/`next_attack_ms` deadlines), ground items plus the town
stash, pre-instance position stash, `engaged_by` gate, the `VesselForge`
stream, and the `world_random_state_` counter exist only in memory
(`core.hpp:973-1014`); the sole serialization touching them is the lossy wire
JSON (`networking.cpp:879-984`). Concretely: `world_random_state_` advances on
every loot roll (`core.cpp:3067-3072`, consumers `3166-3202`) yet appears in no
snapshot — after any restart or projection round-trip, identical command
histories diverge in loot forever. Current replay proof cannot see this
because every existing byte-equality test lives below `Simulation`'s boundary.

Secondary risks, in order:

1. **Wall-clock-keyed world combat** (`GAP-2`): byte-identical command streams
   through the server yield different timelines under tick-thread jitter; the
   deadlines are milliseconds from `system_clock`, not tick counts.
2. **Unrecorded session RNG stream** (`GAP-3`): unseeded `dev:give/drop` draws
   silently consume it (`networking.cpp:1064-1101`).
3. **Unversioned adapter input**: `passive_tree_` accepted wholesale from the
   client payload without schema/version/size checks (`networking.cpp:1230-1232`).
4. **Ephemeral placement ordering** (`GAP-5`): `++serial_` timestamps drive
   newest-first menus but reset per construction.
5. **Persistence unwired** (`GAP-6`): adapter proven, integration absent.

## 7. Existing replay/byte-equality proof inventory

Core layer (all green at audited head): whole-run determinism
(`core_tests.cpp:894`), movement replay (:271), facing replay (:321), elite
skill replay (:514), war-cry expiry replay (:556), relic resurface replay
(:729), legend stable-id replay (:1667), pack lifecycle replay (:1149-1163),
objective timeline replay (:1248-1261), expedition wave spawn/replay
(:1274-1334), D-106 recovery order (:1490), snapshot byte stability + tolerance
(:761-775), D-109 mid-instance RNG continuation (:778-818), recovery pools
(:836-843), file adapter round-trip (:880-892).

Adjacent layers: local seam determinism (`session_tests.cpp:67`), reconnect
login-snapshot authority (:443), chronicle equality (:1965/:2011), presentation
spawn detection byte-determinism (`presentation_events_tests.cpp:159-176`),
audio schedule ordering (`audio_mixer_tests.cpp:213-230`), content-validator
byte-stable diagnostics (`native/content/README.md`, negative suite).

## 8. Versioned replay record contract (definition only — not implemented)

Purpose: let a successor (TASK-0106 lineage) capture, store, and re-drive a
core-layer run without new authority questions.

```text
ReplayRecord v1  (content type: application/vnd.verdigris.replay-record.v1)
{
  "record_version": 1,              // mandatory first field, restore refuses others
  "layer": "core",                  // v1 covers Simulation only; "world" reserved
  "seed": <uint64 decimal>,         // original 64-bit ctor seed (not the truncated one)
  "house_name": <string>,
  "command_count": <n>,
  "commands": [                     // canonical field order: type,dx,dy,action,target
    {"type": "MoveIntent", "dx": 1, "dy": 0, "action": "Wait", "target": ""},
    ...                              // ActionType/CommandType names frozen at
                                     // core.hpp:25-27/264-274 ordinals; append-only
  ],
  "expected": {
    "events_sha256": "<hex>",       // digest of Event records in emission order
    "legends_sha256": "<hex>",      // LegendEntry records (ordinal,tick,ids)
    "final_snapshot_sha256": "<hex>" // digest of snapshot() bytes at final tick
  },
  "provenance": { "base_commit": ..., "created_by": ... }
}
```

Rules: (a) digests are computed over the same canonical serialization style the
v1 snapshot bytes already use (fixed field order, hex strings); (b) enum names
map to frozen ordinals — a record is invalid if it names a value the audited
header does not define; (c) unknown fields are ignored on load and required
fields fail loudly, mirroring snapshot()/restore() tolerance semantics so stale
records remain loadable; (d) v1 explicitly does NOT cover WorldSimulation,
networking cadence, or presentation — those carry `"layer"` markers in v2
space and must not be smuggled into v1.

## 9. Divergence report contract (definition only — not implemented)

```text
DivergenceReport v1 (application/vnd.verdigris.divergence-report.v1)
{
  "report_version": 1,
  "record_ref": <sha256 of the ReplayRecord>,
  "gate": <exact failing command line>,
  "first_divergence": {
    "command_index": <i>,           // index into commands[] where outputs differ
    "layer": "core",                // core | world | networking | presentation | audio
    "field_path": "actors[2].position.x",
    "expected": ..., "actual": ...
  },
  "rng_state": {                    // copied from both runs at divergence point
    "expected": {"state": "...", "serial": "..."},
    "actual":   {"state": "...", "serial": "..."}
  },
  "tick": {"expected": ..., "actual": ...},
  "classification":                 // closed vocabulary for triage metrics
    "seed_mismatch" | "command_mismatch" | "rng_stream_fork" |
    "clock_leak" | "adapter_input" | "unknown",
  "evidence_digests": {...},        // hex snippets / digests, never full dumps
  "notes": <free text>
}
```

Rules: reports reference records by digest, embed no secrets, classify with
the closed vocabulary above (mapping guidance: GAP-1/GAP-3 findings surface as
`rng_stream_fork`, GAP-2 as `clock_leak`, GAP-4 as `adapter_input`), and a
report is valid only when produced by the literal gate command recorded in
`gate`.

## 10. Smallest scaffold for the successor (no implementation shipped here)

1. `native/tests/replay_record_tests.cpp` — golden-record test: build a
   ReplayRecord v1 from the existing deterministic drivers, assert
   events/legends/final-snapshot digests match on a second run; plus a
   negative case proving a mutated command fails at `first_divergence`.
2. A header-only record codec beside the tests (same pattern as
   `persistence/adapter.hpp`: include-only, zero core linkage), reusing the
   snapshot writer's canonical field-order helpers rather than duplicating
   them.
3. One CI hook: run the golden record after `build.ps1 -RunTests`.
4. Defer everything touching WorldSimulation until GAP-1/GAP-2 have an owner —
   recording wall-clock inputs would legitimize the leak instead of fixing it.

## 11. Gate summary

All four acceptance commands were executed literally against the audited head;
transcripts and exit codes live in REPORT.md. `git diff --name-only` shows only
files under `orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/`.
No forbidden path touched; no ports opened; port 6500 never used.
