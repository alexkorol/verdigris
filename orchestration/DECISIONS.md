# Canonical decisions

Settled = binding until the owner reverses it. Provisional = architect's
call, cheap to reverse, flagged for owner attention. Owner-only = no agent
may decide it.

## Settled

- **D-001 Language**: native runtime is C++20 (ADR-001,
  `docs/rebuild/ADR-001-native-language.md`). MSVC on Windows now; CMake is
  the cross-platform path. No general-purpose engine (no Godot/Unreal/
  Unity/Bevy-as-engine).
- **D-002 Simulation boundary**: deterministic fixed-timestep headless core;
  commands in, events out; no windowing/GPU/sockets/DB/DOM/assets in the
  core. Presentation may only consume events and snapshots.
- **D-003 Actor symmetry**: one stat schema and one damage pipeline for
  players, monsters, future mercenaries. Elites differ by level, build,
  equipment, and abilities — never by a separate stat universe.
- **D-004 House/Scion model**: the House is the persistent identity
  (lineage/tribe, not a building); Scions are mortal individuals. Campaign
  progress (routes, branches, knowledge) is House-owned. Unextracted value
  is lost on death; significant carried items enter a House relic pool and
  can re-enter the loot stream with history.
- **D-005 Setting**: Bronze Age / pre-iron fantasy. The Delaford legacy
  denylist (`config/legacy-denylist.json`) governs what may enter native
  production code. No medieval defaults, no Delaford starter kits.
- **D-006 Track isolation**: `prototypes/founding-slice/` is a disposable
  feel lab. Its code and numbers carry no design authority; findings enter
  canon only through this file or the constitution.
- **D-007 Native client control contract** (unblocks the client task; the
  E-key conflict raised by Codex is resolved as follows):
  - WASD movement (continuous), mouse aiming.
  - LMB primary attack, RMB weapon skill.
  - Space dodge/dash.
  - Q / E / R: additional skills (E is a SKILL slot, not equip).
  - X: pick up nearest item; Z: toggle loot-name filter; gold-like
    currency (if/when it exists) auto-picks.
  - F: contextual interact (extraction standard, shrines, doors).
  - I or Tab: gear/inventory; no context-menu dependency; no piano bar.
  Equip moves to the inventory UI, not a world key.

## Provisional (architect's call, owner may override)

- **D-101 Player base-life offset**: the slice gives player-kind actors a
  modestly higher base-life constant within the shared formula. If adopted
  natively, express it as starting equipment/traits instead of a kind check.
- **D-102 Camera envelope**: 2.5D perspective-billboard, pitch ~52–62°,
  mild depth perspective, tilt-shift optional. The slice's "Miniature"
  preset is the current directional target; final projection stays an open
  experiment (camera lab preserved in both clients).
- **D-103 Slice banking**: the demo banks pack items at node completion for
  pacing. Native keeps true extraction risk per the constitution.

## Owner-only (do not decide by agent)

- **D-O1 Seasonal inheritance rule** — what survives a season reset
  (see `docs/product/OPEN_DECISIONS.md`).
- **D-O2 Asset pipeline/provenance** — vendoring full-resolution plates,
  packaging, and any generated-asset policy for the native game.
- **D-O3 Magic system** — production spell design waits on the actual
  WIZARD Spell/Arcane Lattice material
  (`docs/product/WIZARD_ARCANE_LATTICE_REFERENCE.md`); no generic mana
  wizard in the meantime.
- **D-O4 Monetization/distribution, final naming, lore canon.**
- **D-O5 Economy scope** — trade, passive income, currency exchange
  (flagged in the feature checklist; seams only until decided).
