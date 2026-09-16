# Starter player combat references

This package contains the original 576 Blender references across 20 character/action
clips, four imagegen paint proof sheets, and versioned combat corrections. It is
not a complete promoted game sprite pack. `reference-manifest.json` maps the
original frames; the versioned folders have their own current manifests.

For appearance projection, use the eight preferred scenes listed in
`provenance/package.json`: club attacks in `contact-v4-repaired/`, actual punches
in `contact-v5-fist/`, the grounded female death in `contact-v4-repaired/`, and
male death/both hit reactions in `transition-v3-skin/`. Those 384 directional
references supersede the corresponding earlier combat sources. Version READMEs
document exact paths, timing, mask changes and review evidence. Intermediate
versions remain intact for reproducible comparisons.

- Male and female: unarmed idle, unarmed attack/hit/death, and branch-club
  idle/walk/sprint/attack/hit/death. Four actual camera facings per clip.
- Idle clips have four subtle breathing samples; every other clip has eight.
- Idle, movement and hit: 96×96 canvas, root anchor (48,80).
- Attack and death: 128×128 canvas, root anchor (64,96).
- Every canvas retains 48 pixels per world metre at the player plane. Wider
  action canvases add transparent margins; they do not shrink the character.

Motion comes from the selected CC0 Quaternius animation library recorded in
`mocap/source.json`, except locomotion copied from the existing reviewed sources
and idle built from the anatomical rest pose. Rest fingers are already curled;
the club binds to `hand_r` and wrist orientation transfers through palm frames.
No gait frames are labeled as attacks, hit reactions or deaths.

Rebuild with background Blender and `native/tools/render_starter_combat.py --
male attack` (replace sex/action as required). Equip existing movement with
`render_starter_combat_equipped.py -- female walk`. The original source milestones
are preserved. `render_starter_combat_contact.py` packs lossless guides and fails
if a reference touches its canvas edge.

Paint proof originals and exact prompts are under `originals/`. Three attack
proofs returned 1254×1254 despite requesting 1024×1024. Do not resize these to the
requested size: recover their implied pixel lattice and register the resulting
128px frames to their Blender references. The female hit proof returned the
requested 1536×1024. Alpha was inspected by compositing onto opaque green;
hidden RGB outside alpha=0 is not a background to remove.

Review limits: source wardrobe transport uses existing baked cloth weights,
not a new cloth simulation for every action. Earlier combat sources had omitted
underarm skin; the current six repaired scenes restore existing skin while
excluding covered faces that contact the tunic in any sampled pose. All repaired
samples pass that specific clearance check. A remaining dark underarm crescent
is shaded deformation, confirmed with emission and full-skin diagnostics. The
transition entry/recovery has ground-height correction but no horizontal foot
plant constraint. None of these references or paint proofs constitute final
in-game acceptance. The parent lane owns native-grid recovery, animation
playback checks and promotion.

The reproducible helper inventory and source relationships are recorded in
`provenance/package.json`. `provenance/package-files.json` records exact package
paths and SHA-256 hashes at handoff; later projection work may add new files.
Blender automatic `.blend1` backups are preserved locally but excluded from that
active package inventory.

All `render_starter_combat*.py` helpers remain in `native/tools`: base
rendering, equipment clones, library extraction, contacts, transition rendering,
transition QA, mask repair, mask/cloth geometry QA, contact timing, retarget audits,
corpse grounding, closed-fist refinement and repaired contact packing.
Each is part of the reproducible workflow. No one-off debug helper remains in
`native/tools`, so no helper relocation is necessary. Diagnostic renders remain
beside the evidence they explain. The parent-owned `recover_starter_combat.py`
is outside this lane's ownership and is unchanged.
