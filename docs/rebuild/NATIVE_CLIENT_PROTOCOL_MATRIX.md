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

## Gate B — N5 envelopes still required

These rows are intentionally open until the N5 server surface is released and
the native client has named protocol tests for the durable House journey.

| Envelope / journey | Required wire surface | Status | Gap owner |
|---|---|---|---|
| House lifecycle | House create/select/restore snapshot | ⬜ | TASK-0056 N5 must publish the authoritative House envelope and test label |
| Scion lifecycle | Scion create/select/current identity | ⬜ | TASK-0056 N5 must publish the Scion lifecycle envelope and test label |
| Death | Scion death result plus recoverable House pools | ⬜ | TASK-0056 N5 must publish death/recovery events and client assertions |
| Successor | Successor creation after death with House history | ⬜ | TASK-0056 N5 must publish successor response and persistence assertions |
| Persistence | Reloaded House/Scion state across relaunch | ⬜ | TASK-0056 N5 must publish a durable restore envelope and end-to-end test |
