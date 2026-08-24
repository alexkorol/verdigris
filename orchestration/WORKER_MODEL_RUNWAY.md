# Cursor worker — model runway summary

Worker lane: header-only models and adapters (no `main.cpp` / integrator hotspots).

Branch tip: `codex/TASK-0197-chronicles-pane-model-slice-cursor` (`642043fc`).

## REVIEW_REQUESTED packets (cursor)

| Task | Deliverable |
|------|-------------|
| 0167–0169 | WIZARD asset packs (framekit, orbs, items) |
| 0170–0176 | Client models (menu, inventory, paper doll, actor, VFX, gates, instances) |
| 0180–0182 | Render adapters (framekit, orb, item art) |
| 0191 | `cartographer_adapter.hpp` |
| 0193 | `geometric_skill_tree.hpp` |
| 0195 | `spell_lattice.hpp` |
| 0200 | `house_progression.hpp` |
| 0197 | `chronicles_owner_pane.hpp` (model slice) |
| 0198 | `brand_crafting.hpp` (model slice) |
| 0199 | `bond_progress.hpp` (model slice) |
| 0202 | `relic_provenance.hpp` (model slice) |
| 0203 | `village_defense.hpp` (model slice) |
| 0204 | `owner_demo_audio_beats.hpp` (model slice) |

## Playtest

Worktree tip: `npm run playtest` 32/32 exit 0 (port 6510). Evidence on TASK-0203 and TASK-0204 packets.

## Model lane status

**Stocked.** No further path-disjoint model packets identified without integrator lease.

## Blocked integrator lanes

0183–0189, 0190+, 0201–0205 full integration require coordinator ACCEPTED on dependencies and `main.cpp` / `core.cpp` / `event_cues.cpp` lease.
