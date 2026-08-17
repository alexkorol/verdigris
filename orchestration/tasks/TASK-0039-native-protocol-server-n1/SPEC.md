---
id: TASK-0039
title: Parity wave N1 — C++ protocol server (transport + login)
state: READY
track: native
priority: critical
base_commit: current program tip (coordinator records the SHA)
dependencies: []
parallel_safe: true
owned_paths:
  - native/networking/**
  - native/src/**
  - native/include/**
  - native/tests/**
  - native/CMakeLists.txt
  - native/build.ps1
  - docs/rebuild/ADR-003-networking-library.md
forbidden_paths:
  - native/client/**
  - src/**
  - server/**
  - prototypes/**
  - package.json
acceptance_commands:
  - powershell -File native/build.ps1 -RunTests
---

## Goal

Parity roadmap wave N1 (`docs/rebuild/PARITY_ROADMAP.md`): a C++ server
binary that speaks the existing `{event,data}` WebSocket protocol well
enough that the UNCHANGED playtest harness passes its `quickstart` and
`single-session` scenarios against it on :6500.

## The protocol contract (from the TASK-0005 audit — binding)

JSON envelope `{event, data}`; handlers receive payload at `data.data`;
server emitter adds optional `meta`/timestamps; message size caps and
heartbeat; guest login via `player:login` with the guest-session shape;
`dev:state` endpoint for the harness when NODE_ENV!==production
(replicate its contract). Read the JS implementations cited in the 0005
report §Networking before writing code.

## Scope

1. **ADR-003 draft** (in the owned docs path): choose the WebSocket/HTTP
   library from permissively licensed candidates (IXWebSocket,
   uWebSockets, Boost.Beast standalone-ASIO, or equivalent) with
   build-integration reality on MSVC/CMake as a first-class criterion;
   vendor or FetchContent per the choice; architect ratifies the ADR in
   review. Dependency-free core stays dependency-free — the lib lives
   in `native/networking/` only.
2. A `verdigris_server` target: accepts WS connections on :6500, parses
   the envelope, routes to a session layer that instantiates the
   existing deterministic core per player (single-player sessions are
   sufficient for N1), serves guest `player:login` and whatever minimal
   message set `quickstart`/`single-session` scenarios require
   (discover by reading the scenarios + running them), plus `dev:state`.
3. Static file serving for the built client IF the harness requires it
   (check how playtest boots; if it serves via the JS process only, the
   harness invocation for C++ may point at the C++ port with the client
   from dist/ — document the harness invocation used).
4. Tests: envelope parse/emit round-trip; session lifecycle; a
   README-documented command to run the playtest scenarios against the
   C++ server (e.g. env var for target port/binary), with the two
   scenarios' output committed in REPORT.md.

## Non-goals

Full scenario matrix (later waves), multiplayer, TLS, performance work,
client changes.

## Acceptance criteria

- Native gates green (core untouched semantically; build additions OK —
  build files are in-scope THIS task only).
- `quickstart` + `single-session` playtest scenarios PASS against the
  C++ server, transcript in REPORT.md, invocation documented and
  repeatable (this may need port coordination — use an alternate port
  + harness override if 6500 is occupied; document).
- ADR-003 present with a real comparison, not a rubber stamp.

## Review focus

Protocol fidelity (byte-shape of envelopes vs JS), session/core
boundary cleanliness (no gameplay in transport), ADR-003 reasoning,
scenario transcript authenticity (architect will rerun).

## Stop conditions

Any need to modify the JS server or harness semantics → stop and file a
question (a harness TARGET override flag is acceptable if additive and
lives in playtest config — ask first).
