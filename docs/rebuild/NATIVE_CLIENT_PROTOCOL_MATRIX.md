# Native client protocol matrix (D-122, seeded by TASK-0060)

One row per journey step. This is the checklist that keeps client,
server, and presentation implementing the SAME journey. 0061 fills the
remaining rows; a step is DONE only when every column is real.

Legend: ✅ working · 🧩 server ready, client pending (0061) · ⬜ pending.

| Journey step | Player input | Outbound event | Server handler | Authoritative response | ClientModel update | Presentation evidence | Automated test |
|---|---|---|---|---|---|---|---|
| Connect | launch | WS upgrade | WebSocketServer::handle_connection | 101 Switching Protocols | ConnectionState Connected | ConnectionEstablished event | ✅ session_tests "connect + upgrade" |
| Guest login | auto after connect | `player:login` {guestId, quickGuest} | handle_message login branch | `player:login` {player, scene} | uuid/x/y/facing/scene/inventory | SessionReady event | ✅ session_tests "acknowledged -> ready" |
| Dead endpoint (negative) | launch vs dead port | TCP connect | — | refused | ConnectionState Rejected (NO local fallback) | ConnectionLost event | ✅ session_tests remote-negative |
| Session replaced | second login same guest | — | session-replacement flush | `player:session-replaced` | Disconnected | ConnectionLost event | ✅ session_tests remote_session_replaced |
| Zone entry | E / route select | `world:zone:enter` {nodeId} | handler exists | `world:scene:transition` | scene + stairs_up | scene name / stairs pad | ✅ session_tests journey zone enter |
| Movement | WASD | `player:move` {direction} | handler exists | `player:movement` | x/y/facing | actor motion | ✅ session_tests journey movement echo |
| Aim | mouse | presentation-local | — | — | facing | aim reticle / facing | ✅ session_tests journey aim |
| Primary action | click/space | `player:skill:trigger` | handler exists | combat events | last_outgoing_hit | swing/impact fx | ✅ session_tests journey outgoing hit |
| Telegraph | — | — | combat pipeline | `monster:telegraph` | — | telegraph flash + banner | ✅ session_tests journey telegraph |
| Damage in/out | — | — | process_combat | `combat:hit` | life / last_*_hit | floating numbers, flashes | ✅ session_tests journey in/out |
| Enemy death | — | — | combat pipeline | `combat:hit` died | kills | death ring | ✅ session_tests journey kill |
| Item drop | — | — | drop pipeline (no envelope) | loot in-world only | sparkle at scion (server gap) | drop sparkle + log | ✅ session_tests ItemDropped on kill |
| Pickup | X | `player:take:underfoot` | handler exists | `core:refresh:inventory` | inventory slots | pickup log + backpack | ✅ session_tests journey named item |
| Equip | 1-9 | `item:equip` {item.uuid} | handler exists | inventory refresh | equipped slot + item stats | equipped HUD | ✅ session_tests journey equip |
| Extraction | walk stairs-up | `player:move` onto stairs | stair transition | "returns to the surface" | extracted + town scene | banked banner | ✅ session_tests journey extract |
| Disconnect | quit / server kill | WS close frame | connection close | — | Disconnected | CONNECTION LOST banner | ✅ session_tests mid-session kill |
| Reconnect | relaunch | login again | session reuse | restored session | full model | — | ⬜ post-N5 |
| Persistence | relaunch after death | login | N5 persistence | persisted House | House state | — | ⬜ post-N5 (Gate B) |
