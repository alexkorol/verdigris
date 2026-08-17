# WIZARD seam verification

Coordinator run: 2026-08-17 on `codex/native-reconstitution`.

The focused suite covers the WIZARD components named in the product brief:

    npx vitest run \
      tests/unit/hud-orb-ui.spec.js \
      tests/unit/rendering-wizard-orb.spec.js \
      tests/unit/inventory-system.spec.js \
      tests/unit/inventory-commit.spec.js \
      tests/unit/inventory-store.spec.js \
      tests/unit/inventory-layout.spec.js \
      tests/unit/inventory-item-presentation.spec.js \
      tests/unit/map-carve-bounds.spec.js \
      tests/unit/map-pathfinding.spec.js \
      tests/unit/world-layout.spec.js

Result: **10 files passed, 73 tests passed**, exit code 0.

Coverage mapping:

- Orbs: `hud-orb-ui` (8) and `rendering-wizard-orb` (2).
- Brands & Bonds / inventory: inventory system, commit, store, layout, and
  item-presentation suites (50 tests total).
- Cartographer/map seam: carve-bounds, pathfinding, and world-layout suites
  (13 tests total).
- Verdigris Splash remains a presentation/reference seam; it has no focused
  automated test in this repository and remains an owner-facing presentation
  decision, not a native simulation dependency.

Authoritative design boundaries remain in
[`WIZARD_ARCANE_LATTICE_REFERENCE.md`](../product/WIZARD_ARCANE_LATTICE_REFERENCE.md)
and [`VERDIGRIS_CONSTITUTION.md`](../product/VERDIGRIS_CONSTITUTION.md).
