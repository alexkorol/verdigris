---
task: TASK-0048
title: Chronicles first combat is mechanically silent — diagnose and fix
state: READY
priority: critical (playable-first D-110; blocks the House/Scion path)
owned_paths:
  - server/**
  - src/**
  - tests/**
  - orchestration/tasks/TASK-0048-chronicles-silent-combat/**
forbidden_paths:
  - playtest/** assertions may not be loosened (adding a NEW scenario or
    strengthening one is allowed and encouraged)
  - native/**
base: current program tip (>= d687e004; includes 0038 LMB/RMB controls)
architect_review_required: true
---

## The bug (from accepted TASK-0046 arcs)

Same build, same input surface, ten driven minutes each:

- Guest (`?play`) arc in Old Barrow: three readable melee kills, XP,
  incoming damage, 60-gold pickup. Combat works.
- Chronicles mortal-oath arc (House Ember → Asha → Set Out) in Old
  Barrow: an actor visibly overlaps the player, but sustained WASD +
  Bronze Arc + quickbar attempts produce ZERO combat log lines, no
  damage in either direction, no kill, no loot; HP pinned 110/110.
  Alongside it: repeated non-directive `Not enough mana.` rejections.

The asymmetry points at Chronicles/scion state initialization — combat
profile, skill kit, resource state, or target/faction wiring — not the
input path (which the guest arc exercises identically and 0038's frame
evidence proves ends in `player:skill:trigger` reaching the server).

## Scope

1. Diagnose root cause on the REAL path: drive a mortal-oath scion
   through login → Set Out → Old Barrow using real protocol traffic
   (the playtest harness helpers are the model), not mocks. Document
   the found cause in the REPORT before fixing.
2. Fix it server-side (or client-side if that's truly where it is).
3. Mana half: whatever makes the starter kit spam `Not enough mana.`
   for a fresh scion — fix the state error if it is one; if it's a
   balance choice functioning as designed, DO NOT retune numbers
   (owner/D-114 territory) — instead make the rejection directive
   (what's missing, when it recovers) and record the balance question
   for the owner in the REPORT.
4. Regression protection: add (or extend) a playtest scenario that
   logs in via the Chronicles path with a mortal-oath scion and
   asserts the first fight connects (hit line + damage + kill within
   the TTK bound). This closes the gap 0046 exposed: all 31 current
   scenarios enter combat via paths that DON'T reproduce this.

## Acceptance evidence

1. Root-cause narrative with the failing code path named.
2. Literal transcript of the new/extended scenario failing on the
   unfixed base and passing on the fix (authentic negative built in).
3. Full `npm run playtest` 31/31 (or 32/32 with the new scenario) +
   `npm run test:unit` green, transcripts included.
4. One rendered capture of a mortal-oath scion's first kill (combat
   log visible) — the 0038 capture-script pattern (hard-fail checks)
   is the bar.

The architect will rerun the Chronicles scenario personally before
ACCEPTED.
