---
id: TASK-0030
title: Native persistence — snapshot/restore per ADR-002 (D-109)
state: READY
track: native
priority: high
base_commit: current program tip (coordinator records the SHA)
dependencies: []
parallel_safe: true
owned_paths:
  - native/src/**
  - native/include/**
  - native/tests/**
  - native/persistence/**
forbidden_paths:
  - native/client/**
  - native/CMakeLists.txt
  - native/build.ps1
  - native/tools/**
  - src/**
  - server/**
  - prototypes/**
acceptance_commands:
  - powershell -File native/build.ps1 -RunTests
---

## Goal

Implement the accepted ADR-002 (as amended by owner ruling D-109): pure
`snapshot()`/`restore()` functions over the durable House/Scion state,
versioned, with RNG state included so a restored simulation continues
deterministically. No I/O in the core.

## Product and architectural invariants

- ADR-002 verbatim: schemaVersion field mandatory; unknown-field
  tolerance mandatory; covers House (routes/branches/knowledge-
  equivalents, stores, relic pool + lost trophies + pending re-entry
  pools, Legends, lineage/fallen), current Scion, `Rng::state`+`serial`.
- D-109: restore lands the Scion in the House (out of any instance) WITH
  everything carried — leaving an instance via restore loses nothing.
  Mid-instance state is NOT serialized (instances retire on snapshot
  boundaries the same way retire_instance treats abandonment — but per
  D-109 carried items stay carried; only floor state is gone).
- Format: choose the simplest deterministic representation (a versioned
  JSON-like text or fixed binary — implementer's choice, documented);
  byte-stable for identical states.
- File I/O (atomic temp+rename) belongs in `native/persistence/` as a
  thin adapter the tests may use; the core functions take/return bytes.

## Scope

1. `snapshot(const Simulation&) -> std::vector<uint8_t>` and
   `restore(bytes) -> Simulation` (or equivalent factory) in the core.
2. The persistence-adapter file layer (write/read with temp+rename).
3. Tests: round-trip byte-equality (snapshot→restore→snapshot identical);
   deterministic continuation (restored sim + identical commands ==
   never-snapshotted sim + same commands, including RNG-dependent drops);
   D-109 semantics (snapshot mid-instance → restore → Scion in House,
   carried items intact, floor gone, relic/lost pools correct); version
   field present; unknown-field tolerance (inject an extra field, restore
   still works).

## Non-goals

Client save/load UI, browser persistence, seasonal transforms, save-file
obfuscation (owner ruled none for v1).

## Acceptance criteria

Gates green; the scope-3 tests named in REPORT.md; format documented in
`native/persistence/README.md` (that README is inside owned paths).

## Review focus

RNG state fidelity (no reroll-by-reload), D-109 carried-items semantics
vs retire_instance interplay, format stability.

## Stop conditions

Any temptation to serialize live instance/monster state → stop (ADR-002
explicitly defers it); any owner-lore naming → stop.
