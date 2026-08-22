# Native client protocol matrix (D-122, seeded by TASK-0060)

One row per journey step. This is the checklist that keeps client,
server, and presentation implementing the SAME journey. 0061 fills the
remaining rows; a step is DONE only when every column is real.

Legend: ✅ working · 🧩 server ready, client pending (0061) · ⬜ pending.

| Journey step | Player input | Outbound event | Server handler | Authoritative response | ClientModel update | Presentation evidence | Automated test |
|---|---|---|---|---|---|---|---|
| Connect | launch | WS upgrade | WebSocketServer::handle_connection | 101 Switching Protocols | ConnectionState Connected | ConnectionEstablished event | ✅ `native/tests/session_tests.cpp:121` — `remote: connect + upgrade + login sent` |
| Guest login | auto after connect | `player:login` {guestId, quickGuest} | handle_message login branch | `player:login` {player, scene} | uuid/x/y/facing/scene/inventory | SessionReady event | ✅ `native/tests/session_tests.cpp:123` — `remote: player:login acknowledged -> ready` |
| Dead endpoint (negative) | launch vs dead port | TCP connect | — | refused | ConnectionState Rejected (NO local fallback) | ConnectionLost event | ✅ `native/tests/session_tests.cpp:105` — `remote-negative: state is rejected, not a silent local fallback` |
| Session replaced | second login same guest | — | session-replacement flush | `player:session-replaced` | Disconnected | ConnectionLost event | ✅ `native/tests/session_tests.cpp:405-406` — `replaced: first session is disconnected`; `replaced: ConnectionLost from player:session-replaced` |
| Zone entry | E / route select | `world:zone:enter` {nodeId} | handler exists | `world:scene:transition` | scene + stairs_up | scene name / stairs pad | ✅ `native/tests/networking_tests.cpp:134-145` — `solo entry emits a scene transition`; `state reports instance`; `both stairs exist` |
| Movement | WASD | `player:move` {direction} | handler exists | `player:movement` | x/y/facing | actor motion | ✅ `native/tests/networking_tests.cpp:111-114` — `applied sample broadcasts player:movement`; `right sample moves east` |
| Aim | mouse | presentation-local | — | — | facing | aim reticle / facing | ✅ `native/tests/session_tests.cpp:219` — `journey: aim updates facing` |
| Primary action | click/space | `player:skill:trigger` | handler exists | combat events | last_outgoing_hit | swing/impact fx | ✅ `native/tests/session_tests.cpp:243` — `journey: outgoing combat:hit reached the client` |
| Telegraph | — | — | combat pipeline | `monster:telegraph` | — | telegraph flash + banner | ✅ `native/tests/session_tests.cpp:295` — `journey: monster:telegraph reached the client` |
| Damage in/out | — | — | process_combat | `combat:hit` | life / last_*_hit | floating numbers, flashes | ✅ `native/tests/session_tests.cpp:243,294` — `journey: outgoing combat:hit reached the client`; `journey: incoming combat:hit reached the client` |
| Enemy death | — | — | combat pipeline | `combat:hit` died | kills | death ring | ✅ `native/tests/session_tests.cpp:244` — `journey: enemy death reached the client` |
| Item drop | — | — | drop pipeline | `item:change`, `world:itemDropped` | ground item list | drop sparkle + log | ✅ `native/tests/networking_tests.cpp:262-266` — `floor treasure emits item:change and world:itemDropped`; `ground envelope has uuid, id, name, x, y` |
| Pickup | X | `player:take:underfoot` | handler exists | `core:refresh:inventory` | inventory slots | pickup log + backpack | ✅ `native/tests/session_tests.cpp:259-260` — `journey: named item entered inventory (pickup)`; `journey: ItemPickedUp or inventory growth` |
| Equip | 1-9 | `item:equip` {item.uuid} | handler exists | `player:equippedAnItem` | equipped slot + item stats | equipped HUD | ✅ `native/tests/networking_tests.cpp:402-412` — `item:equip emits player:equippedAnItem`; `equip response includes wear-slot state`; `snapshot wear matches the equip response` |
| Extraction | walk stairs-up | `player:move` onto stairs | stair transition | `player:extract` / surface message | extracted + town scene | banked banner | ✅ `native/tests/session_tests.cpp:314-317` — `walking onto stairs-up returns to the surface (extract)`; `ExtractionCompleted from surface message` |
| Disconnect | quit / server kill | WS close frame | connection close | — | Disconnected/Retrying | CONNECTION LOST banner | ✅ `native/tests/session_tests.cpp:352-354` — `reconnect: unexpected drop enters Retrying`; `reconnect: ConnectionLost is visible (no silent local fallback)` |
| Reconnect | relaunch | login again | session reuse | restored session | full model | — | ✅ `native/tests/session_tests.cpp:365-368` — `reconnect: Retrying then Ready after server restart`; `reconnect: same guest identity re-logged in`; `reconnect: login snapshot is authoritative` |
| Persistence | relaunch after death | login | N5 persistence | persisted House | House state | — | ⬜ Gap: N5 durable House/Scion restore envelope and client model coverage; owner family TASK-0056 (N5) |

## Gate B — Chronicles wire-contract freeze (TASK-0081)

TASK-0081 froze the already-landed native-server surface at worker base
`986264f4`. Per-step records with exact payload/response keys live in
`orchestration/tasks/TASK-0081-gate-b-wire-contract/captures/gate-b-wire-contract.json`.
Every envelope below was read from `native/src/networking.cpp` at that base;
nothing is invented and no code changed. Status: 🧩 server response proven
from cited source lines, client pending · ⬜ explicit RED gap on the wire or
store itself. The automated-test column is RED for every chronicles step:
`rg -n 'chronicles' native/tests/networking_tests.cpp native/tests/session_tests.cpp`
returns no matches at this base; TASK-0077 must name these labels when it
implements the client against this frozen surface.

| Envelope / journey | Frozen wire surface (handler `file:line` in native/src/networking.cpp unless noted) | Status | Automated test |
|---|---|---|---|
| House lifecycle | Found: `chronicles:house:found` {name} → `chronicles:state` {player.socket_id, chronicle{version,houses[{id,name,scions[],crypt[]}],activeHouseId,activeScionId}} (`ProtocolSession::handle` :2498-2508; builder `chronicles_state_payload` :2235-2241; `ensure_chronicle_house` :2242-2272). Alt existing mutate path: `player:chronicles:mutate` {type:"found-house", house.id, house.name} → `player:chronicles:update` {player.socket_id, chronicles, chroniclesRevision, chroniclesExists:true} (:2549-2572). Restore: `player:login` {guestId} → `chronicles:state`; `{awaitChronicles:true}` → `player:chronicles:ready` {player.socket_id, chroniclesAccountId, accountName, level, chronicles, chroniclesRevision, chroniclesExists} (:2655-2681; ready keys :2224-2234) | 🧩 | 🔴 RED: no native test label |
| Scion lifecycle | Create: `chronicles:scion:create` {houseId, name} → `chronicles:state` + `createdScionId` (:2509-2518; scion entry id/name/level/mortal/deaths via `ensure_chronicle_scion` :2273-2301). Select/admission: `player:chronicles:select` {scionId, houseId, scionName, mortal} → `player:login` {player(…, player.chronicles{mortal,scionId,houseId} :582-591), scene, droppedItems[]} via `emit_login`/`login_payload` (:2632-2654, :2302, :593-595); resets commission chain + purse (:2643-2649). Set-out: `chronicles:scion:set-out` {scionId} → `player:login` (+ one-shot `game:send:message` daily road-purse notice :2530-2534; once-per-scion starter kit :2537-2543; wagon-pitch town reset :2544-2546) | 🧩 | 🔴 RED: no native test label |
| Death | Server-initiated fatal fall (no client request): lethal wound with mortal oath armed (`process_combat` trigger :2111-2114) → `ProtocolSession::handle_final_death` :2128-2207 emits `chronicles:scion-fallen` {fallen{scionId|null,name,level}, relicCount, chronicle} (:2190); broadcasts `chronicles:scion-witnessed` {fallen.name, relicCount} (:2191-2198); direct `player:stats:update` {playerId, lifecycle{state:"permadead",mode:"hard"}} (:2199-2206). Fallen scion moves houses[].scions[] → houses[].crypt[] with relic{status:"lost",count} (:2147-2176); earned gear enters the circulation pool (:2133-2146) | 🧩 | 🔴 RED: no native test label |
| Successor | No dedicated successor event: reuse `chronicles:scion:create` → `player:chronicles:select` (:2509-2518, :2632-2654). Return path: `player:chronicles:return` {} → `player:chronicles:ready` {player.socket_id, fallen{scionId,scionName}, + ready keys} and queues crypt heirloom relic{status:"queued", item|null} (:2586-2630). Recovery leg: elite kill surfaces circulating heirloom (`process_combat` :2083-2104; dev shortcut `dev:release-relic` :2411-2434); ground JSON carries chroniclesRelic{relicId,scionId,scionName} (:393-413); pickup `player:take:underfoot` → `core:refresh:inventory` + `world:itemDropped`/`item:change` and `mark_relic_recovered` flips crypt relic lost→recovered (:1659-1671, :1632-1658, :1602-1631, ground emit :919-927) | 🧩 | 🔴 RED: no native test label anywhere on the chain |
| Persistence | In-process session reuse proven: new socket same identity re-login restores account state incl. chronicle_ within a live server (`reset_world_for_new_socket` :538-579; permadead deliberately not resurrected :554-562); generic reconnect labels exist (session_tests.cpp:409-411, 423-425) but zero assert chronicle content. Explicit RED gaps: no quit/logout envelope exists (transport WS close only; only "quit" token is the server console command, server_main.cpp:30) and no durable cross-process store exists (`rg 'ofstream|ifstream|fopen|fwrite' native/src` → no matches), so a server restart loses Houses/Scions/crypt state | ⬜ RED: durable restore envelope + end-to-end test missing | Durable leg unrouted (candidate successor of TASK-0097 persistence audit); client tests TASK-0077 |
