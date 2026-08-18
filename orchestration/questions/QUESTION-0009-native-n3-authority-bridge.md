---
question: QUESTION-0009
related_task: TASK-0045 (proposed native parity N3)
owner: Fable / architect
state: OPEN
---

# Native N3 authority bridge and task issuance

## Decision needed

Please issue a READY N3 combat/skills task and choose the authoritative bridge
between the native N2 `WorldSimulation` and deterministic `Simulation` before
implementation begins.

## Evidence

- `ProtocolSession` at Kimi commit `d476788` owns both models.
- `player:move` updates only `WorldSimulation`; it does not dispatch
  `Command::move` or turn an occupied monster tile into primary melee.
- World monsters are shallow records with no UUID-to-`Actor` mapping; N2
  exposes 18 `<theme>-lurker` records and empty ground-loot arrays.
- `Simulation` already owns shared actions, damage/death, drops, and events,
  but its generic events lack enough source/skill/killer metadata for a
  lossless browser `combat:hit` projection.
- Full line-oriented evidence is in
  `orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-n3-core-gap-audit-2026-08-17.txt`.

## Options

1. **Promote `Simulation` as the sole encounter authority (recommended).**
   Extend its encounter/content API and event metadata, then make the world
   projection consume those actors and events. This best preserves D-002 and
   avoids two combat authorities.
2. **Retain both models behind an explicit adapter.** Add a deterministic
   UUID/coordinate synchronization layer and define which model owns every
   mutation. This minimizes immediate world rewrites but adds a maintained
   translation seam and must prohibit networking-side gameplay rules.
3. **Replace the N2 world roster with a core-backed scene projection.** Larger
   change, but removes the duplicate monster model before N3/N4 content grows.

## Recommendation

Choose option 1 or 3 if the native core is intended to remain the system of
record. If option 2 is preferred for sequencing, the N3 SPEC should enumerate
the synchronization invariants, coordinate conversion, event cursor, and
failure behavior explicitly.

## Scope status

This question blocks only the proposed N3 implementation task. It does not
block browser regression work, current N2 review evidence, or owner decisions
already documented in `DECISIONS.md`. No source code was changed.
