# TASK-0060 report — C3 session seam scaffold (architect, self-review)

- Branch: codex/native-reconstitution (architect single-writer), base 0ce26b15.
- **Program first: the native client session connected to verdigris_server
  over a real RFC6455 WebSocket, sent `player:login`, and mirrored the
  authoritative snapshot into a ClientModel.** Axis-2 convergence has begun.

## Delivered

- `native/client/session.hpp` — IClientSession + ClientCommand +
  ConnectionState (connecting/connected/ready/disconnected/retrying/
  rejected/protocol-mismatch) + invariant comments.
- `native/client/client_model.hpp`, `presentation_events.hpp` — plain
  presentation-facing model + transient event stream.
- `native/client/local_session.{hpp,cpp}` — LocalCoreSession wrapping
  Simulation (the only client code allowed to own one); scenario escape
  hatch for the existing D-119 harness.
- `native/client/remote_session.{hpp,cpp}` — WS client transport
  (masked frames, ping->pong, close handling), guest login, snapshot →
  model mapping, NO silent local fallback. Recorded interim couplings:
  links verdigris_networking for the envelope codec; fixed
  Sec-WebSocket-Key + deterministic mask (loopback-only transport).
- Build wiring: CMakeLists lib `verdigris_client_session` + test exe;
  build.ps1 compiles/links/runs session tests with hard exit-code check.
- `native/tests/session_tests.cpp` — 19 checks: local adapter
  (ready-without-handshake, movement through the seam, events,
  shutdown), authentic negative (dead endpoint → Rejected, hard error,
  no fallback), remote handshake (server on architect-capsule port
  6572+, login → Ready, identity+scene mirrored, survives a command,
  clean dual shutdown).
- `docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md` seeded (connect/login/
  negative/disconnect ✅; movement→extraction rows assigned to 0061).

## Evidence (architect-run, 2026-08-20 ~01:35)

- `native/build.ps1 -RunTests`: denylist PASS, core PASS, networking
  PASS, camera2d PASS, session tests 19/19 PASS.
- `native/build.ps1 -RunClientScenarios`: PASS (0 failures) — main.cpp
  untouched; existing D-119 scenarios unaffected.

## Known stubs → successor

- ClientCommand::Aim / Extract are presentation-local no-ops in remote
  mode (0061 maps them when the journey needs them).
- main.cpp does not yet consume IClientSession — 0061 migrates it under
  the single-writer rule.
- Upgrade response accept-key not cryptographically verified (loopback
  dev transport; revisit if transport leaves loopback).
