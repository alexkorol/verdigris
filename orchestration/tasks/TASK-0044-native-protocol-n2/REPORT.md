# TASK-0044 REPORT — Parity wave N2: world, movement, and zones over the C++ server

## Executive summary

The C++ server now serves world/zone payloads, authoritative continuous
movement, and portal/zone transitions. The UNCHANGED playtest harness passes
its `movement` and `zones` scenarios against the C++ server via
`PLAYTEST_WS_URL` attach — 2/2 with the pre-0043 harness, and 4/4
(movement, zones, plus the N1 `quickstart`/`single-session` regression pair)
with the post-0043 merged harness at program tip `e462c26`. Native gates
(`powershell -File native/build.ps1 -RunTests`) are green.

Base commit: `32d7b6e` (program tip at claim time, post-0039 integration).
Worker branch: `codex/TASK-0044-native-protocol-n2`.

## Approach

Per the spec, RULES went into the deterministic core, TRANSPORT mapping into
`networking/`:

- **`native/include/verdigris/core.hpp` / `native/src/core.cpp`** — new N2
  section: `tile_movement` (constants and math mirroring
  `server/shared/movement.js` exactly: `PLAYER_TILE_TRAVEL_MS=150`,
  `PLAYER_MOVE_SAMPLE_MS=50`, `PLAYER_MOVE_DISTANCE=1/3`,
  `POSITION_PRECISION=6`, 8-way normalised direction vectors,
  `roundPosition` integer snap, `occupiedTile` rounding), plus
  `TileGrid`, `WorldMonster`, `InstanceMetadata`, `MovementStepInfo`,
  the `adventure_zones()` table (row-for-row from `party.js`
  `ADVENTURE_ZONES`), and `WorldSimulation` — the tile-space world for one
  player: town scene, solo instance generation, per-sample movement with the
  JS blocking rules (rounded target tile, living-monster occupancy,
  both-orthogonal-neighbours diagonal rule), facing collapse for diagonals,
  movement-step sequencing, stair portals, and pre-instance position
  save/restore (first entry only, matching `teleportMembersToSpawns`).
- **`native/src/networking.cpp` / `networking.hpp`** — transport mapping:
  `player:move` → one movement sample → `player:movement` broadcast (player
  payload + `meta.movementStep`) to every live connection;
  `instance:enterSolo` → `party:scene:transition` with
  `scene {id,type,name,metadata}` and `playerState {uuid,x,y,sceneId}`;
  `dev:teleport` → floor onto the tile, portal check, dev message;
  stair return → `game:send:message` "The party returns to the surface." +
  `party:scene:transition`; `dev:state` snapshot extended with fractional
  `x/y`, `sceneId/sceneType/sceneName`, `sceneMetadata`
  (`layout/theme/seed/depth/stairsUp/stairsDown/spawnPoints`), and instance
  monsters. The combat `Simulation` remains the HP/inventory authority;
  position/scene/monsters come from `WorldSimulation`.
- **Session reuse semantics tightened:** a `player:login` only resumes an
  existing session when that session still has a LIVE connection to replace
  (the `single-session` handoff case — verified still passing). A login
  after the old connection closed starts fresh at the town spawn. This
  matches how sequential playtest scenarios behave against the JS server and
  is required for `movement` → `zones` sequencing in one attach run.

No harness edits. No combat scope (N3). No Chronicles scope (N5).

## Changed files

- `native/include/verdigris/core.hpp` — N2 tile-space world types and
  `WorldSimulation` declaration
- `native/src/core.cpp` — movement rules, zone table, instance generation,
  stair/return logic
- `native/include/verdigris/networking.hpp` — session broadcast hook,
  `WebSocketServer::broadcast`
- `native/src/networking.cpp` — envelope mapping for the N2 verbs,
  extended snapshot/transition payloads, live-connection session reuse
- `native/tests/core_tests.cpp` — N2 rule coverage
- `native/tests/networking_tests.cpp` — N2 wire coverage

## Public interfaces

New core API (all in `namespace verdigris`, header `verdigris/core.hpp`):
`tile_movement::{kTileTravelMs, kSampleMs, kMoveDistance, kPositionPrecision,
movement_delta, round_position, occupied_tile}`, `TileGrid`, `WorldMonster`,
`InstanceMetadata`, `MovementStepInfo`, `ZoneDescriptor`,
`adventure_zones()`, `is_zone_template()`, `is_zone_layout()`,
`WorldSimulation` (`apply_movement_sample`, `teleport`,
`enter_solo_instance`, scene/monster/metadata accessors).
Wire verbs added: `player:move`, `instance:enterSolo`, `dev:teleport`;
events emitted: `player:movement` (broadcast), `party:scene:transition`,
`game:send:message`.

## Test commands and outcomes

### Native gates

```
$ powershell -File native/build.ps1 -RunTests
native legacy denylist: PASS
verdigris core tests: PASS
verdigris networking tests: PASS
```

### Attach run (harness at my branch tip, pre-0043 merge)

```
$ PLAYTEST_WS_URL=ws://127.0.0.1:6511 node playtest/run.mjs --attach movement zones
  ▶ movement
      ✓ stopped between tiles (115.333333)
      ✓ moved down (115 -> 118.333333)
      ✓ moved right (38 -> 41)
      ✓ moved back up-left
      ✓ still in the instance after 2s (scene: The Old Barrow)
      ✓ no bounce back to town
      ✓ moved inside the instance
  PASS movement (4594ms)
  ▶ zones
      ✓ dungeon/warren: entered an instance
      ✓ dungeon/warren: layout applied (warren)
      ✓ dungeon/warren: both stairs exist
      ✓ dungeon/warren: scene has a display name (The Old Barrow)
      ✓ dungeon/warren: populated (18 monsters)
      ✓ grove/clearings: entered an instance
      ✓ grove/clearings: layout applied (clearings)
      ✓ grove/clearings: both stairs exist
      ✓ grove/clearings: scene has a display name (Verdant Grove)
      ✓ grove/clearings: populated (18 monsters)
      ✓ crypt/gauntlet: entered an instance
      ✓ crypt/gauntlet: layout applied (gauntlet)
      ✓ crypt/gauntlet: both stairs exist
      ✓ crypt/gauntlet: scene has a display name (Sunken Colonnade)
      ✓ crypt/gauntlet: populated (18 monsters)
      ✓ crypt/warren: entered an instance
      ✓ crypt/warren: layout applied (warren)
      ✓ crypt/warren: both stairs exist
      ✓ crypt/warren: scene has a display name (Weir Crypt)
      ✓ crypt/warren: populated (18 monsters)
      ✓ wilds/clearings: entered an instance
      ✓ wilds/clearings: layout applied (clearings)
      ✓ wilds/clearings: both stairs exist
      ✓ wilds/clearings: scene has a display name (The Wilds)
      ✓ wilds/clearings: populated (18 monsters)
      ✓ marsh/clearings: entered an instance
      ✓ marsh/clearings: layout applied (clearings)
      ✓ marsh/clearings: both stairs exist
      ✓ marsh/clearings: scene has a display name (Marsh of Reeds)
      ✓ marsh/clearings: populated (18 monsters)
      ✓ returned to the pre-entry position (38,115 vs 38,115)
  PASS zones (1101ms)
2/2 scenarios passed
```

### Attach run against the POST-0043 merged harness (program tip e462c26)

Run from a detached worktree of `origin/codex/native-reconstitution`
(harness updated by the provisional TASK-0043 merge), same C++ server binary:

```
$ PLAYTEST_WS_URL=ws://127.0.0.1:6511 node playtest/run.mjs --attach movement zones quickstart single-session
  PASS  movement (4592ms)
  PASS  quickstart (161ms)
  PASS  single-session (315ms)
  PASS  zones (1110ms)
4/4 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":32.243711,"maxEventLoopLagMs":32.980991}
```

(`quickstart`/`single-session` rerun to prove the N1 gates did not regress
under the new session-reuse semantics.)

## Manual verification

C++ unit coverage added: sample distance stays fractional mid-tile and
accumulates three tiles over nine samples; `player:movement` broadcast shape
with sequenced `movementStep` meta; unknown directions are no-ops; zone
display names from the adventure table; layout metadata; ≥15 monsters on
walkable tiles only; mid-walk entry does not bounce and emits no message;
teleport onto the entry stairs restores the exact pre-entry town position
with the surface message; diagonal normalisation and the integer-snap
rounding; stair tiles/spawn walkable invariants.

## Commits

- `d476788` feat(native): N2 world, movement, and zones over the C++ server (TASK-0044)
- `293419b` fix(native): bind server to 127.0.0.1 — folds in the board's
  loopback-bind standing guidance (wildcard binds stall on Windows Firewall
  consent prompts per fresh binary). Gates + a full 4-scenario attach run
  against the newest merged harness (program tip `50ca60a`) re-verified
  green after this change:
  `PASS movement (4615ms) / quickstart (162ms) / single-session (315ms) /
  zones (1153ms) — 4/4 scenarios passed`.
- (report/STATUS commit follows on the same branch)

## Deviations and stub inventory (for N3+)

Documented per spec ("zone/instance generation may stub to the minimum the
scenarios exercise"):

1. **Instance generation is stub geometry.** 40×40 floors with border walls,
   a protected spawn clearing, and layout-shaped obstacle bodies (warren
   ribs, clearings thickets, gauntlet corridor). The JS seeded template
   generator (`GameMap.generateInstance`) is NOT ported. N3+ must replace
   this with the real generator.
2. **Town collision geometry is an open field.** The 200×200 town grid is
   all-walkable; town tile tables (`MapUtils.gridWalkable` /
   `UI.tileWalkable` object/tileset rows) are not ported. Movement-rule
   plumbing (blocked checks, diagonal rule) is fully ported and exercised by
   instance walls/monsters.
3. **stairsDown does not descend.** Metadata generates both stairs; the
   portal check only handles stairsUp at depth 1 → return to town. Deeper
   floors (`transitionFloor`, depth > 1 naming "· Floor N") are N3+.
4. **Scene payloads omit the tile map.** `scene {id,type,name,metadata}` is
   emitted; the JS `map` layer arrays / npcs / droppedItems are not
   (no consumer in N2; the harness reads `scene.name` only).
5. **No game loop.** Stair checks are event-driven (movement sample /
   teleport landing) instead of the JS periodic `checkStairTransitions`;
   behavior is identical for every harness-observable path.
6. **Instance-start throttling not ported.** The JS anti-spam cooldown
   (`party:error` "not yet open") is intentionally absent — `enterSolo`
   always succeeds (the harness tolerates and prefers this).
7. **Monster actors are data, not AI.** They block tiles and populate
   snapshots; combat/behaviour is N3. The combat `Simulation` is untouched
   and still owns HP/inventory; its `Actor` positions are not the wire
   position (N3 must reconcile when combat ships).
8. **D-114 note:** the D-114 world-unit table remains the combat envelope
   authority; N2 wire movement deliberately mirrors the browser tile-space
   constants (post-0037), as the spec requires. The two scales are bridged
   only through `world_scale::kPlayerMoveSpeed` documentation; no code
   coupling was added.

## Risks

- Session-reuse tightening assumes the server notices a closed connection
  before the next scenario logs in (run.mjs settles 500 ms between
  scenarios; observed clean across repeated runs, including under the
  post-0043 paced harness).
- Stub floors guarantee walkability only around the spawn clearing; a
  future scenario that walks deep into a floor may hit un-JS-like wall
  layouts until the real generator lands (N3+).

## Follow-ups

- N3: real instance generator + town tile tables + combat behind the
  protocol (monsters become `Simulation` actors).
- Architect rerun of the acceptance commands before ACCEPTED, per the N1
  precedent.
