# TASK-0145 REPORT — Native Chronicles owner-facing journey (ox-pc-i replacement)

- Worker: `ox-pc-i` (PC Ox Alpha lane, ports 6780-6799, machine `DESKTOP-TVU7OR7`)
- Provider/model: `openrouter` / `stealth/ox-alpha` (OpenCode CLI 1.18.21 fleet provisioning)
- Branch: `codex/TASK-0145-native-chronicles-owner-journey-ox-pc-i-r2`
- Claim base: `b58dc3dbba354106af7df4fc29ddbc708fcf477b`
- Commits: `226e5149` (STATUS-only replacement claim, RELEASE-provenance recorded) →
  `0e9b7b5d` (implementation + evidence captures) → REVIEW_REQUESTED head (this commit)
- Release provenance honored: prior claim `4aa9e0c3` (ox-pc-b) was explicitly released via
  `RELEASE.md`; the quarantined ox-pc-b worktree and its uncommitted edit were not read,
  copied, or touched. This is an independent implementation from the SPEC against current
  source.

## Executive summary

The native client's remote owner path now opens at a coherent **Chronicles front door**
instead of dropping a nameless guest into a game window. Against the already-accepted
Gate-B wire contract (TASK-0081 freeze, verified line-by-line from current
`native/src/networking.cpp`), an owner can: see their account's chronicle state,
found a House, name Scions, arm the mortal-oath field, admit through the oath-bearing
`player:chronicles:select` path, explore with the untouched TASK-0142 presentation, die a
server-authoritative final death, land back on a door that records the fall/crypt/relic
circulation, create and admit an heir through the succession-select path, recover the
circulating heirloom (crypt flips lost → recovered, named toast + HUD line), quit,
reconnect, and see the persisted House/heir/crypt roster rendered before choosing to walk
again. Local play and every pre-existing scenario remain green.

New deterministic headless scenario `chronicles-gate-b` drives the REAL remote session
seam (`RemoteProtocolSession`) against a real in-process protocol server bound to this
lane's loopback capsule (6780–6799): 38 assertions covering screen transitions and
actionable controls, not just parsed fields.

## Approach

1. Re-read the frozen contract first: `captures/gate-b-wire-contract.json` (TASK-0081)
   and current `native/src/networking.cpp` handlers/payload builders. No envelope was
   invented; parsers consume exactly the documented shapes (`chronicles:state`,
   `player:chronicles:ready`, `player:chronicles:update`, `chronicles:scion-fallen`,
   admission `player:login`, `dev:state.chroniclesRecord`, `groundItems.chroniclesRelic`).
2. Kept all authority boundaries: ClientModel gains plain chronicle structs;
   RemoteProtocolSession remains the only socket owner; LocalCoreSession synthesizes the
   same view from its simulation; presentation renders the model and never mints Houses,
   Scions, oaths, or relics.
3. Front-door menu actions are rebuilt deterministically from the model each frame and
   are the same table keyboard input fires, so what the scenario asserts is what the
   owner presses.
4. Wire-path honesty drove two mapping rules:
   - the mortal oath rides only on `player:chronicles:select`, so an armed oath admits
     through select; a plain living-scion outing claims the road purse via
     `chronicles:scion:set-out`;
   - after a fall (active scion == fallen scion), admission switches to the succession
     select path, because select alone resets the permadead lifecycle.

## Changed files (all inside owned paths plus one recorded deviation)

| File | Change |
|---|---|
| `native/client/client_model.hpp` | Chronicle structs (House/Scion/Crypt/Fallen), `ClientPlayer.display_name`, relic-marked ground items, `ClientModel.chronicle/chronicles_pending/lifecycle`, roster lookup helpers |
| `native/client/session.hpp` | DEVIATION (see below): four new typed intents `FoundHouse/CreateScion/SelectScion/SetOut` + factories |
| `native/client/local_session.cpp` | Factory definitions; local chronicle synthesis; new intents are explicit local no-ops |
| `native/client/remote_session.cpp` | Non-quick login sends `{guestId, awaitChronicles:true}`; chronicle payload parsing; scion-fallen handling; dev:state lifecycle/hp/chronicleRecord/relic-ground sync; four submit envelope branches |
| `native/client/render_list.hpp` | New draw classes `Op::Chronicles` (front-door lines incl. `action:*`) and `Op::HouseChip` |
| `native/client/main.cpp` | Screen enum + front-door painter/menu/key handling; screen authority rule; House/scion identity chip; relic-recovery toast; `paint_connection_chip` extraction (shared chrome, verbatim body); `--remote [host] [port] [guest] [--quick]` owner-default parsing; `scenario_chronicles_gate_b` + pump helpers; capture writer into this task folder |
| `native/client/presentation_state.cpp` | `sync_world_from_model` prefers authoritative chronicle identity for House/Scion names |

## Public interfaces added/changed

- `ClientCommand::Type::{FoundHouse, CreateScion, SelectScion, SetOut}` +
  factories (`found_house(name)`, `create_scion(name)`, `select_scion(id, mortal)`,
  `set_out(id)`).
- `IClientSession` unchanged (no virtual changes); concrete sessions translate the new
  intents into the frozen envelopes (`chronicles:house:found`, `chronicles:scion:create`
  with active-house resolution, `player:chronicles:select {scionId, houseId, scionName,
  mortal}` with roster-resolved house/name, `chronicles:scion:set-out`).
- `RemoteProtocolSession` non-quick construction now logs in with `awaitChronicles:true`
  (quick-guest byte stream unchanged; `--quick` preserves the legacy interactive path).
- Render list: `Op::Chronicles`, `Op::HouseChip` (recorded next to their draws so a
  suppressed draw fails scenarios, per D-119 rules).
- Executable surface: `verdigris_client.exe --remote [host] [port] [guest] [--quick]`;
  remote default is now the Chronicles door; `--headless`, `--scenario`, and local
  window behavior unchanged.

## Acceptance commands (literal, from repository root)

```text
PS> powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
native legacy denylist: PASS
verdigris core tests: PASS
verdigris networking tests: PASS
camera2d tests: PASS
session tests passed
== scenario move-and-camera ==      PASS (0 failures)
== scenario first-fight ==          PASS (0 failures)
== scenario loot-to-bank ==         PASS (0 failures)
== scenario telegraph-dodge ==      PASS (0 failures)
== scenario combat-juice ==         PASS (0 failures)
== scenario remote-render-list ==   PASS (0 failures)
== scenario zoom-invariance ==      PASS (0 failures)
== scenario chronicles-gate-b ==    PASS (0 failures)
EXIT=0

PS> native\build\verdigris_client.exe --scenario chronicles-gate-b
38 x "ok:" checks (full transcript below), "PASS (0 failures)"
EXIT=0

PS> git diff --check
EXIT=0

PS> git diff --name-only 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2..HEAD
( coordination wave commits between the immutable base and this branch base are the
  architect's; this worker's own diff vs claim base b58dc3db is exactly the seven
  client seam files + this task folder )
EXIT=0
```

### Full chronicles-gate-b transcript (final green run at HEAD `0e9b7b5d+label fix`)

```text
ok: chronicles-gate-b: bound ox-pc-i capsule server
ok: chronicles-gate-b: session start
ok: front door: the account chronicle opens the door
ok: front door: the Chronicles title is on screen
ok: front door: founding a House is an actionable control
ok: front door: founding renders the House roster
ok: front door: the new House is named on screen
ok: front door: naming a Scion is offered
ok: front door: the first Scion joins the roster
ok: front door: set-out is actionable for the new Scion
ok: front door: the mortal-oath field renders its soft state
ok: front door: the mortal-oath field arms on demand
ok: admission: the mortal-oath select lands in the world
ok: admission: the front door is dismissed
ok: admission: the expedition names the House and Scion
ok: expedition: the road instance is entered
ok: expedition: the route renders with the existing presentation
ok: expedition: earned gear enters the inventory
ok: consequence: scion-fallen names the fallen Scion
ok: consequence: the fall returns the owner to the chronicles
ok: consequence: the fall is recorded on the front door
ok: consequence: the fallen Scion rests in the crypt
ok: succession: naming a successor is actionable after the fall
ok: evidence: front-door capture written
ok: succession: the heir joins the living roster
ok: succession: heirship admission is actionable without the oath
ok: succession: the heirship select admits the successor
ok: succession: the heir takes a healed field
ok: succession: the identity chip names the heir
ok: recovery: the surfaced heirloom carries its provenance
ok: recovery: the crypt record flips lost to recovered
ok: recovery: the recovery toast names the fallen
ok: recovery: the expedition HUD announces the recovery
ok: reconnect: session restarts
ok: reconnect: House, heir, and crypt render on return
ok: reconnect: the living heir is listed
ok: reconnect: re-admission returns to the expedition
ok: evidence: expedition HUD capture written
PASS (0 failures)
```

## Evidence captures (default 960x600 resolution, committed)

- `captures/front-door-960x600.png` — post-fall front door: title, account, House
  roster, crypt entry with heirloom-circulation count, the recorded fall, successor
  action, armed oath field, connection chip.
- `captures/expedition-hud-960x600.png` — admitted heir in-expedition: House/Scion
  identity chip, TASK-0142 objective/art/minimap/orbs/quickbar presentation intact, and
  the named heirloom-recovery toast.

## Manual verification notes

The interactive path uses the same painter/menu code the scenario exercises
(`paint_scene` offscreen vs `WM_PAINT` double-buffered both call it; key handling routes
through `handle_chronicles_key` → the same `chronicle_actions` table). Owner launch:
`verdigris_client.exe --remote 127.0.0.1 <port> <guest>` opens the door;
`play-native.ps1 -Port 652x` remains the one-command local play path (local mode never
shows the door).

## Gate-B quality rubric — implementer evidence map (verdict belongs to the architect)

| Axis | Self | Evidence |
|---|---|---|
| input response | 2 | menu keys fire the same deterministic action table the painter records; oath toggle renders immediately; expedition input untouched (prior scenarios green) |
| combat legibility | 2 | TASK-0142 combat presentation unchanged; telegraph-dodge/combat-juice still PASS |
| reward clarity | 2 | surfaced heirloom carries provenance; pickup flips crypt lost→recovered; toast + HUD line name the fallen |
| navigation clarity | 1 | door states are explicit (connecting/opening/roster); objective strip intact; no new wayfinding invented |
| UI hierarchy | 2 | full-screen door with title→account→roster→actions order; identity chip on HUD |
| visual cohesion | 1 | door shares the HUD accent palette; serif door typography is a deliberate first pass |

Implementer self-total 10/12 with no zeroes; architect scoring governs.

## Deviations

1. `session.hpp` edited though absent from `owned_paths`. The four typed intents must
   live on the shared `ClientCommand` seam whose implementations (`local_session.*`,
   `remote_session.*`) ARE owned; omitting them would force stringly-typed commands or
   duplicated dispatch. The edit adds only enum values + factory declarations. Recorded
   here for the reviewer; happy to relocate if the architect prefers.
2. Scenario uses accepted dev control surfaces (`dev:kill`, `dev:give` seeded,
   `dev:release-relic`, `dev:heal`) through the session's existing raw-envelope escape
   hatch to make fatal death, earned gear, and relic circulation deterministic. The
   production journey paths exercised around them are the real chronicles envelopes.
3. Founder/Scion naming derives deterministically from the account identity ("House of
   X", "X Firstborn/Secondborn…") because the front door intentionally has no text-input
   console; the wire accepts any name.

## RECORDED RED — server-side seam found during the journey (not invented around)

On the current tip, `player:chronicles:select` resets the lifecycle but does NOT restore
the Simulation actor's life, and the soft `awaiting-respawn` transition exists only on
the `dev:kill` branch of `ProtocolSession::handle` (never in `process_combat`). A
successor admitted onto a fallen actor's seat therefore inherits a zero-life seat until
a fresh socket heal (`reset_world_for_new_socket`). Per SPEC stop-condition discipline I
did not touch `native/src/**`: the scenario continues every independent UI state via the
accepted `dev:heal` surface (labeled check), and the client already renders the honest
states around it. Exact pointers for the networking owner (TASK-0148 lane):
`networking.cpp:2632-2654` (select, no actor heal), `:2384-2396` (dev:kill-only soft
death), `core.cpp:1543` (`reset_to_town` leaves actor stats untouched). Browser-parity
question: whether JS `select` heals the fresh-scion profile seat.

## Risks / follow-ups

- Session-test port capsule (6580–6599) showed one transient collision failure while
  multiple lanes build concurrently on this machine; two subsequent full-suite runs at
  HEAD were fully green. Consider per-lane capsules for test servers as a hardening task.
- Long House/Scion names can overlap the centered objective chip on narrow windows;
  cosmetic, left for the pane-system wave.
- Relic toast lifetime is fixed (160 presentation ticks); persistence could come from the
  pane/HUD wave rather than more toast logic.

## Unresolved questions

None blocking acceptance. The RECORDED RED above is filed for the architect/networking
owner to route; everything else in the SPEC's outcome is delivered and gated.
