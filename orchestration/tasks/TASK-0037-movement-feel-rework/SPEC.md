---
id: TASK-0037
title: Browser movement feel — kill the jag
state: READY
track: web-demo
priority: critical
base_commit: current program tip (coordinator records the SHA)
dependencies: []
parallel_safe: true
owned_paths:
  - src/core/player/**
  - src/core/utilities/input-controller.js
  - server/core/entities/player/movement-handler.js
  - tests/unit/movement*.spec.js
  - orchestration/tasks/TASK-0037-movement-feel-rework/captures/**
forbidden_paths:
  - src/components/**
  - src/core/config/controls.js
  - native/**
  - prototypes/**
  - package.json
acceptance_commands:
  - npm run test:unit
  - npm run playtest
  - npm run smoke:browser
---

## Goal

Movement stops feeling "jagged" (owner verdict). Reference feel: the
webchat Fable demos and the founding slice — responsive acceleration,
smooth continuous motion, normalized diagonals, no visible stepping.

## Diagnosis directive (do this first, in REPORT.md)

The owner suspects "delaford movement with many iterations slapped on."
The TASK-0005 audit mapped a legacy queued path alongside continuous
movement in `movement-handler.js`. Profile what ACTUALLY produces the
jag: tile quantization? server round-trip stepping without client
interpolation? mixed legacy/continuous paths fighting? rAF vs tick
mismatch? Name the mechanism with evidence before changing code.

## Scope

Whatever the diagnosis demands within owned paths, typical shape:
client-side interpolation/prediction between authoritative positions,
one movement path (delete or bypass the legacy queued path if it is the
culprit), normalized diagonal speed, acceleration curve. Server remains
authoritative (no trust shift). D-114: one table of movement constants
(speed, accel, interpolation window) with seconds-to-cross-screen
derivations.

## Acceptance criteria

- All three gates green (playtest movement scenarios may need deliberate
  expectation updates — justify each).
- A short screen recording or dense capture sequence demonstrating
  smooth motion (walk, direction changes, diagonals) — the architect
  will ALSO play it personally (D-115) and judge against the slice's
  feel before accepting.

## Stop conditions

Any change to the wire protocol shape; any client-authority drift.
