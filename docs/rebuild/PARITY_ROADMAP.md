# Native parity roadmap (D-116)

Mission: the C++ version reaches the current web version's level or
better. Measuring stick: the existing Vue client + the 31-scenario
playtest harness, pointed at the C++ server.

## The strategy in one sentence

Build a C++ server that speaks the existing `{event, data}` WebSocket
protocol on :6500, so the UNCHANGED web client and the UNCHANGED playtest
harness run against it — parity is then a scenario matrix, not an
opinion.

## Why this path

- The protocol is small and audited (TASK-0005: envelope, `data.data`
  payloads, rate/auth gates, heartbeats, caps).
- The playtest harness already encodes what "the current web version's
  level" means, scenario by scenario (quickstart, session-arc, zones,
  combat, vesselforge, respawn, world-web…). Each scenario green against
  C++ is parity BANKED and regression-guarded forever.
- The deterministic core already implements the hard rules (actors,
  skills, death/relics per D-106, persistence per D-109) with tests.
  The server harness wraps it; gameplay logic ports INTO the core, never
  into the transport.

## Sweep layers (D-116)

1. **Protocol scenarios**: playtest matrix dual-run (JS vs C++);
   divergence reports per scenario.
2. **UI panes**: the 0036 sweep pattern, re-run per milestone.
3. **Core determinism**: replay + snapshot byte-equality suites (exist).
4. **Experience**: D-115 architect play gate on every feel milestone.

## Waves (each = tasks with the playtest scenarios it must turn green)

- **N1. Transport + handshake**: WS server harness (permissively
  licensed lib — candidate set IXWebSocket / uWebSockets / Boost.Beast;
  the N1 task drafts ADR-003 with the choice, architect ratifies),
  envelope parsing, heartbeat, `player:login` guest path, dev:state.
  Green bar: `quickstart`, `single-session`.
- **N2. World + movement**: map/zone payloads, authoritative movement
  (D-114 tables), portals. Green: `zones`, movement scenarios.
- **N3. Combat + skills**: core combat pipeline behind protocol; drops.
  Green: combat scenarios, `encounter-variety`.
- **N4. Items/inventory + Vesselforge data**: item payloads from curated
  data (LEGACY_MATRIX KEEP-as-data), equip/pickup. Green: item and
  `vesselforge` scenarios (formula parity where owner-ruled).
- **N5. Chronicles**: House/Scion lifecycle, D-106 death, relics,
  persistence files. Green: `respawn`, `session-arc`, Chronicles
  scenarios.
- **N6. World-web + quests**: route graph, Wardens, quest state.
  Green: `world-web`, `quest`, remaining matrix — FULL PARITY.
- **N7. Better-than**: performance headroom proof (entity-density
  benchmark vs JS), then the native client resumes (browser-validated
  design, D-113 art) as the superior front end.

Waves land sequentially; within a wave, tasks split by disjoint files.
The browser playability wave (0032–0038) continues in parallel — it
raises the reference bar and its fixes define what parity means.

## Standing rules

- No gameplay logic in the transport layer; the core stays deterministic
  and headless (D-002).
- Every wave ends with the dual-run scenario matrix report committed.
- Web-side changes after a wave lands must re-run that wave's matrix
  (regression sweep layer 1).

## Coordinator evidence checkpoint — 2026-08-17

The N2 contract matrix has now been run against both implementations with the
same unchanged scenario set (`quickstart`, `single-session`, `movement`,
`zones`): JavaScript reference **4/4**, native commit `d476788` **4/4**.
The native run exercised all six zone/layout combinations and restored the
pre-entry position. The comparison also records the intentional N2 boundary:
the JS reference reports authored populations `33/56/31/46/57/54`, while the
native adapter returns 18 actors per zone. N2 permits a minimum scenario stub;
exact population/composition parity is therefore an explicit N3+ obligation,
not a silently accepted mismatch. Raw comparison:
[`coordinator-dual-run-matrix-2026-08-17.txt`](../../orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-dual-run-matrix-2026-08-17.txt).
