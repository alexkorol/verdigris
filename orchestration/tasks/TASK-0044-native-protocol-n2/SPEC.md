---
id: TASK-0044
title: Parity wave N2 — world, movement, and zones over the C++ server
state: READY
track: native
priority: critical
base_commit: program tip after 0039 integration (coordinator records the SHA)
dependencies: [TASK-0039]
parallel_safe: true
owned_paths:
  - native/networking/**
  - native/src/**
  - native/include/**
  - native/tests/**
  - native/CMakeLists.txt
forbidden_paths:
  - native/client/**
  - src/**
  - server/**
  - playtest/**
  - prototypes/**
  - package.json
acceptance_commands:
  - powershell -File native/build.ps1 -RunTests
---

## Goal

Parity roadmap wave N2: the C++ server serves world/zone payloads,
authoritative continuous movement (D-114 tables; the 0037-validated feel
constants are the reference), and portal/zone transitions — proven by
the UNCHANGED harness passing its movement and `zones` scenarios via
`PLAYTEST_WS_URL` attach, transcripts in REPORT.md and RERUN BY THE
ARCHITECT before acceptance (the N1 precedent).

## Scope

Read the JS implementations the 0005 audit cites (movement-handler,
world/zone payloads, portals) for the wire contract; port RULES into the
deterministic core, TRANSPORT mapping into networking/. Movement
constants must mirror the browser's post-0037 values (parity means same
feel). Zone/instance generation may stub to the minimum the scenarios
exercise — document what is stubbed for N3+.

## Acceptance criteria

Native gates green; `zones` + movement scenarios PASS against the C++
server (architect reruns); no gameplay in transport; stub inventory
documented.

## Stop conditions

Harness edits; combat scope (N3); Chronicles scope (N5).
