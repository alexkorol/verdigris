---
task: TASK-0049
title: First-session UI wave — identity, directive feedback, and next-decision surfaces
state: READY
priority: high (D-110; sourced from accepted 0046 SURVIVES items)
owned_paths:
  - src/**
  - tests/**
  - orchestration/tasks/TASK-0049-first-session-ui-wave/**
forbidden_paths:
  - server/** (client presentation only — if a needed datum is missing
    from existing payloads, STOP and file a question instead)
  - playtest/** assertions may not be loosened (new/strengthened
    scenarios welcome)
  - native/**
base: current program tip (>= 1244b5bf)
architect_review_required: true
---

## Goal

Close the accepted TASK-0046 SURVIVES items that are pure client
presentation. Five deliverables, smallest-that-lands versions:

1. **House/Scion identity in the world HUD.** After Set Out, the HUD
   keeps "House Ember — Asha (Mortal oath)" visible (compact, near the
   HP orb or top bar). Data already arrives at login/admission.
2. **Directive mana rejection** (architect-ratified in 0048 review):
   replace bare "Not enough mana." with the missing amount and
   recovery cadence, e.g. "Need 12 more mana — recovering 2 every 2s."
   Wording exactly informative, no balance change; numbers read from
   the live resource state the client already has.
3. **Tutorial ticker legibility.** The chat/tutorial beat ticker is
   easy to miss; give the FIRST-SESSION beats (Aldwyn's guidance) a
   readable presentation — larger transient banner or expanded-by-
   default log for the first N beats — without burying combat lines.
4. **Zone objective preview.** The Adventure panel's zone rows state a
   concrete draw: named Warden, guaranteed item-level treasure, depth
   record — from data already in the adventure/world-web payloads.
5. **Skill tree first-allocation hint.** With unspent points and no
   allocations, the tree pane highlights a sensible starter node path
   (data-driven from the existing tree payload; presentation only).

## Evidence bar (the 0038 standard)

- Real rendered screenshots of each of the five, captured by a
  hard-fail Playwright script (checks assert the on-screen text).
- `npm run test:unit` and `npm run smoke:browser` green transcripts.
- `npm run playtest` full suite green (32/32).
- Wire proof per the amended standard if any capture drives a live
  session (owned-server log correlation is fine).

The architect will inspect the screenshots and rerun gates before
ACCEPTED.
