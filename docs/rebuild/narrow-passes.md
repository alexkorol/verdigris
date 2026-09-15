# Owner priorities after Art Direction Grill

Source: owner follow-up to the review of
https://chatgpt.com/c/6aa2daae-9388-83e9-aef2-eea123c16ba5.

## Immediate authorized fixes

1. Level display: mirror the authoritative character level in the native HUD
   and character panel. Earn level 2 and preserve it across reload.
2. Melee contact: short attacks must connect near the target at impact.
   Moving out during wind-up must cause a miss.
3. Starting movement: reduce the default speed; the previous fast movement
   should require earned character progression. The initial tuning candidate
   is four tiles per second (250 ms per tile), retaining 50 ms input samples.

## Larger accepted priorities

- Native skill tree is one complete module: load the full WIZARD node data,
  render its connections, and implement its interactions. Nodes and links
  belong together; rendering alone does not complete the module.
- Organic terrain is a very important next feature. Avoid visibly robotic
  grids and address the WIZARD Cartographer module alongside the game work.
- HUD/orbs: settle game-view and HUD pixel-art resolutions before deciding
  which new orb art is needed. Preserve the actual WIZARD shader behavior.

## Explicit hold

Do not repair or regenerate the east walk animation yet. The owner wants to
redesign the sprite-sheet creation process first. Do not interpret the earlier
four-frame proposal as approval to resume generation.

The art-direction chat monitor was cancelled by the owner. Read that chat only
on request; do not recreate recurring polling.
