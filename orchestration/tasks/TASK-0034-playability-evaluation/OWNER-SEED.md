# Owner-reported friction seed (2026-08-16 late — verbatim intent)

The owner played the native exe. Findings that seed the 0034 friction
list (the browser-game evaluation must check for the same classes):

- Projection/2.5D feel broken; "not 2d or 3d"; movement unexpected.
- Enemies attack from very far away (range/speed incoherence — D-114).
- UI obstruction: text/buttons covering the game world, "nearly
  unplayable."
- Missing game UI entirely: character pane, inventory, passive tree,
  menus, health/mana orbs, skill bar.
- Graphics: wants the webchat-artifact approach (procedural/vector +
  minimal image input, D-113), NOT Delaford assets; scenery must scale
  correctly relative to player/enemy art.
- Overall: expected a much higher-quality product.

Evaluators: treat every one of these as a hypothesis to test against the
BROWSER game too, and rank accordingly.

## Second owner session (browser game, ~22:55) — confirmed frictions

- "Somewhat playable... you get bored in like 10 seconds" — the
  engagement cliff is THE headline finding for 0034 to explain.
- CONFIRMED REGRESSION (screenshot on file): inventory pane — backpack
  grid beside the paperdoll, both tiny, huge dead space below
  (→ TASK-0036, which also sweeps every other pane for this class).
- Movement "janky/jagged"; webchat demos felt great immediately
  (→ TASK-0037 with diagnosis-first directive).
- Wants LMB/RMB attacks + full key/mouse rebinding (→ TASK-0038).
- Possible: MP orb absent in the screenshot while inventory open —
  evaluators verify.
