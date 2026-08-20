# TASK-0060 — C3 session seam + remote scaffold (ARCHITECTURE, architect-owned)

Packet: ARCHITECTURE. Owner: architect (Fable), tracked per D-120/D-122.
Base: current program tip at claim.

## Outcome

The native client gains the D-122 session boundary so implementation
lanes can build the networked journey inside frozen interfaces:

- `native/client/session.hpp` — `IClientSession`, `ClientCommand`,
  connection-state enum (connecting/connected/ready/disconnected/
  retrying/rejected/protocol-mismatch).
- `native/client/client_model.hpp` — plain-struct presentation model
  (no JSON, sockets, or `Simulation*` exposed).
- `native/client/presentation_events.hpp` — transient event stream
  (attack, hit, kill, drop, pickup, equip, extraction, death,
  connection lost/restored).
- `native/client/local_session.{hpp,cpp}` — wraps the existing
  in-process sim; ALL existing client scenarios stay green through it.
- `native/client/remote_session.{hpp,cpp}` — WS client skeleton to
  `verdigris_server` reusing the existing envelope codec; handshake +
  guest login + first authoritative snapshot; NO silent local fallback.
- Build wiring in `native/CMakeLists.txt` (client links networking or
  an extracted codec — smallest safe step, coupling recorded).
- One failing-then-green remote test: server on ephemeral loopback,
  client session connects, receives a real snapshot, both terminate.
- `docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md` seeded (journey rows
  for the N4 surface).

## Non-goals

Full guest journey (TASK-0061), renderer extraction, main.cpp rewrite
(extract only what the seam needs; main.cpp stays single-writer).

## Acceptance

`powershell -File native/build.ps1 -RunTests -RunClient
-RunClientScenarios` fully green + the new remote handshake test green
+ malformed-envelope/dead-endpoint negative hard-fails. Architect
self-review recorded; owner sees the connection state on screen.
