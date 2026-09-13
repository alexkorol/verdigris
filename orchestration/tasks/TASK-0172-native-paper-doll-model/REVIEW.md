# REVIEW — TASK-0172 native-paper-doll-model

- reviewer: independent validator (claude subagent), judged and recorded by
  coordinator-of-day claude-architect-pc, 2026-08-24 ~10:10 PDT
- head reviewed: aee15245 (branch
  codex/TASK-0172-native-paper-doll-model-cursor; tip c9618594 is a
  heartbeat; already ancestor of the program branch)
- verdict: **REVISE** — harness green (39 checks reproduced), scope exact,
  determinism sound; three corrections before the TASK-0184 adapter builds
  on this surface.

## REVISE corrections (numbered, testable)

1. **Off-hand must accept one-handed weapons (dual-wield).** WIZARD's
   equip-legality (Z:\Code\WIZARD\tools\rpg_inventory\index.html:1100)
   allows offHand = weapon|shield|focus; kind_fits_slot
   (paper_doll.hpp:199-200,206-207) narrows to shield/focus only,
   contradicting the cited source AND the sim's two hand seats
   (core.cpp:2965-2971) + two_handed flag. Accept 1H weapons in OffHand;
   two-handed-in-main still blocks it; decide and TEST the status for a 2H
   item offered to OffHand.
2. **Resolve the id space.** Item.id/Equipped.id are uint32; everything
   that will feed this model is string-keyed (Command::equip(const
   std::string&) core.hpp:288; GameItem.id core.hpp:605; manifest ids
   "dagger_bronze"). Not the full 0182 failure (ids are caller-supplied
   handles, no invented catalog) but the third-mapping burden lands on
   0184 unstated. Re-key on string ids (preferred, matches 0182 r2's
   direction) OR add an explicit tested contract comment naming 0184 as
   the bridge owner. If re-keyed: round-trip test with real ids.
3. **Ring full-case swap targets the wrong slot.** equip_auto replaces
   Ring1 when both rings occupied (paper_doll.hpp:298); the ported sim
   rule resolve_seat returns seats.back() -> ring2 (core.cpp:2996). Align
   on ring2 + test.

## Evidence

- Harness reproduced: 39 checks PASS (count re-derived: 3+4+4+4+6+3+14+1),
  denylist PASS, diff --check clean.
- Slots: 14-slot set matches WIZARD DEFAULT_EQUIPMENT
  (index.html:818-822) exactly, in order; constitution-clean, no legacy
  class terms.
- Rejection paths (WrongSlot/TwoHandedConflict/InvalidItem/Empty) serve as
  real negative controls.

## Non-blocking notes (0184 checklist + possible QUESTION)

4. Kind::Tool absent though SPEC outcome names "weapon/tool" and manifest
   category tool exists (cur_chisel, cur_knife have no Kind) — worker may
   file a QUESTION rather than silently omit.
5. Manifest categories coarser than Kind (armor can't split
   BodyArmor/Boots; no two_handed in manifest) — 0184/manifest metadata
   gap, recorded.
6. WIZARD warhorn accepts kind curio (index.html:1106), unmodeled.
7. State::valid() doesn't check kind_fits_slot per occupied slot.
8. Frozen head contains commit-msg.txt scratch; STATUS carries short SHA
   only (recurring lane-template gaps).

- revision lane: claude-c claims the r2 revision per BUS.
