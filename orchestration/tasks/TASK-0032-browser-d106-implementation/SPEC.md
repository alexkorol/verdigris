---
id: TASK-0032
title: Browser death/relic alignment with D-106/D-109 (from the 0031 audit)
state: READY
track: web-demo
priority: critical
base_commit: current program tip (coordinator records the SHA)
dependencies: [TASK-0031]
parallel_safe: true
owned_paths:
  - server/**
  - tests/unit/**
  - tests/e2e/**
forbidden_paths:
  - native/**
  - prototypes/**
  - src/**
  - package.json
  - playwright.config.js
acceptance_commands:
  - npm run test:unit
  - npm run playtest
  - npm run smoke:browser
---

## Goal

Implement the 0031 audit's change-set forecast so the browser game obeys
the owner rulings: death never destroys items (D-106) and
disconnect/save failure never loses value (D-109).

## Authority

`orchestration/tasks/TASK-0031-browser-d106-audit/REPORT.md` is the
governing evidence — its delta table defines done. DECISIONS D-106/D-109
outrank any existing browser behavior or test.

## Scope (the audit's forecast, binding highlights)

1. **Authoritative death transfer**: on hard Chronicles death, EVERY
   equipped item, pack item, and carried trophy enters the recovery
   pools (no `collectNotableGear` filtering losses); successor starts
   empty per D-004.
2. **Durable trophy pool** with death/re-entry/claim paths; existing
   socketed trophies remain with their item (simplest consistent rule —
   document it).
3. **Single relic authority**: converge or adapter the SQLite/JSON
   stores so a relic cannot be committed in one and surfaced in the
   other; no duplicate UUIDs after migration.
4. **Instance retirement**: explicit retirement with active-instance
   membership and one-time requeue of unclaimed surfaced candidates
   (mirror TASK-0025 semantics).
5. **D-109 hardening**: a failed disconnect save must not proceed to
   silent removal — retry/blocking boundary or durable queue; add the
   combat-disconnect and teardown-disconnect tests the audit calls for.
6. Migration safety for existing saves per the audit's risk section.

## Non-goals

Client/UI changes, native changes, resurface-cadence retuning beyond
what convergence requires (keep the existing browser cadence unless the
audit's convergence choice forces one; document the choice).

## Acceptance criteria

- All three gates green (playtest 31/31 — the respawn/session-arc/zones
  scenarios constrain you per the audit; update scenario expectations
  only where D-106/D-109 genuinely invalidates them, justified in
  REPORT.md).
- A unit/integration test per delta-table row proving the new semantics.
- Migration path exercised in a test (old-format save loads clean).

## Review focus

Delta-table rows each closed with a named test; no value-loss path
remains; store-authority convergence; playtest stability.

## Stop conditions

Any ambiguity about which store is product-authoritative long-term →
implement the adapter that preserves both, file a question for the
owner-facing choice; any Vesselforge formula temptation → stop (owner
domain).
