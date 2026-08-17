---
task: TASK-0039
state: REVIEW_REQUESTED
branch: codex/TASK-0039-native-protocol-server-n1
commits:
  - 120abd19
  - 413b3aff
base_commit: f0df8de6
---

## Executive summary

The N1 native transport now exposes a small RFC6455 WebSocket server and a
transport-independent `ProtocolSession`. It accepts the existing `{event,
data}` envelope, admits guest sessions, enters the quick-start route, serves
the development state probe, grants the existing test item, and preserves a
session across replacement login. The deterministic core remains free of
socket/JSON dependencies.

## Implementation

- Added `verdigris_networking` and `verdigris_server` CMake targets.
- Added a dependency-free in-tree JSON/envelope parser and bounded masked text
  frame adapter with ping/pong and close handling.
- Added guest login, quick-start zone entry, `dev:state`, `dev:give`, and
  same-identity replacement handoff.
- Added protocol/session tests and ADR-003 comparison of IXWebSocket,
  uWebSockets, Boost.Beast/Asio, and the selected in-tree adapter.
- Added `VERDIGRIS_PORT`/argv port selection so the unchanged harness can run
  on an alternate port without changing its semantics.

## Changed files

- `native/networking/README.md`
- `native/include/verdigris/networking.hpp`
- `native/src/networking.cpp`
- `native/src/server_main.cpp`
- `native/tests/networking_tests.cpp`
- `native/CMakeLists.txt`
- `native/build.ps1`
- `docs/rebuild/ADR-003-networking-library.md`

## Interfaces

`ProtocolSession` is the transport-independent boundary. The wire contract is
the existing `player:login`, `world:zone:enter`, `dev:give`, and `dev:state`
events; responses retain the existing event/data shape and request IDs.

## Verification

- `powershell -NoProfile -File native/build.ps1 -RunTests` — PASS: core tests,
  networking tests, and legacy denylist.
- `node playtest/run.mjs --attach quickstart single-session` with
  `PLAYTEST_WS_URL=ws://127.0.0.1:6511` — PASS, 2/2 (quickstart 158 ms,
  single-session 314 ms). Server was started from the native binary and
  stopped cleanly with `quit`.
- `git diff --check` — PASS.

## Specification deviations

The N1 server is intentionally a bounded RFC6455 subset: no TLS, compression,
fragmentation, production HTTP/static serving, or multiplayer. These are
explicitly outside N1 and documented in ADR-003.

## Risks and limitations

Architect must rerun the native server and inspect the protocol boundary under
D-115. The current build script compiles the native client as part of the
existing native gate; no client source was changed. Full parity scenarios are
deferred to later waves.

## Questions for Fable or the owner

ADR-003 is a draft and needs architect ratification. Confirm that the in-tree
RFC6455 subset is acceptable for N1 and that later HTTP/TLS requirements may
replace it behind `ProtocolSession` without moving transport logic into the
simulation.

## Integration notes

Source is ready for review but remains isolated until architect acceptance.
Integration cherry-picks are `37e1397a` and `4c4c3aee`; no JS server, client,
or playtest harness files were changed.
