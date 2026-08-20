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
| Session replaced | second login same guest | — | session-replacement flush | `player:session-replaced` | Disconnected | ConnectionLost event | ⬜ 0061 |
| Zone entry | route select | `world:zone:enter` {nodeId} | 🧩 handler exists | `world:scene:transition` | scene fields | transition presentation | ⬜ 0061 |
| Movement | WASD | `player:move` {direction} | 🧩 handler exists | movement broadcast | x/y update | actor motion | ⬜ 0061 |
| Aim | mouse | (presentation-local) | — | — | facing | aim reticle | ⬜ 0061 |
| Primary action | click/space | `player:skill:trigger` | 🧩 handler exists | combat events | — | swing/impact fx | ⬜ 0061 |
| Telegraph | — | — | combat pipeline | telegraph envelope | — | telegraph dominates scenery | ⬜ 0061 |
| Damage in/out | — | — | process_combat | damage envelopes | life values | floating numbers, flashes | ⬜ 0061 |
| Enemy death | — | — | combat pipeline | death envelope | — | death ring, corpse removal | ⬜ 0061 |
| Item drop | — | — | drop pipeline | drop envelope | ground items | drop visible + label | ⬜ 0061 |
| Pickup | Z/X | `player:take:underfoot` | 🧩 handler exists | inventory refresh | inventory slots | pickup fx | ⬜ 0061 |
| Equip | inventory UI | `item:equip` {uuid} | 🧩 handler exists | inventory/wear refresh | equipped state | stat/behavior change visible | ⬜ 0061 |
| Extraction | reach exit | ⬜ envelope TBD vs server surface | ⬜ | banked state | — | bank ceremony | ⬜ 0061 |
| Disconnect | quit | WS close frame | connection close | — | Disconnected | clean shutdown | ✅ session_tests "clean shutdown" |
| Reconnect | relaunch | login again | session reuse | restored session | full model | — | ⬜ post-N5 |
| Persistence | relaunch after death | login | N5 persistence | persisted House | House state | — | ⬜ post-N5 (Gate B) |
