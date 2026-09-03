# UI Regression Checklist

## Native Combat Cadence

- Hold primary attack against a durable non-boss. Confirm the quickbar pips
  advance one, two, three and the caption reads `Strike II`, `FINISH`, then
  `Strike I` without waiting for another input.
- Confirm the third hit is visibly distinct: verdigris double ring and cross,
  verdigris target flash, and a larger `finisher` damage number. A non-critical
  finisher must still receive this treatment; a critical finisher must retain
  both identities.
- Confirm the first two hits use the normal recovery, the finisher has a
  slightly longer recovery, and a non-boss cannot retaliate during the brief
  stagger. Boss damage should increase on beat three without boss stagger.
- Pause longer than the continuation window or use Thrust/Sweep, then attack
  again. The first primary hit and one lit pip must restart the sequence.
- Reconnect while a chain is active and confirm the same step/window return
  from the snapshot rather than flashing an invented local step.
- At 1366x768 and 960x600, confirm all three pips fit inside the LMB Framekit
  slot, the caption remains legible, and the finisher flare does not overlap
  the minimap or bottom HUD safe zones.

## Native Campaign Journal

- Enter the world as a named Scion and press `J`; confirm Chronicle
  Commissions opens with the server-authored quest title, commissioner,
  current rite, reward, quest points, House renown, and completed deeds.
- Advance one quest objective and confirm both the town tracker and open
  journal move to the next rite without reconnecting.
- Re-admit the same living Scion and confirm the exact objective checkpoint
  returns. A new Scion should start fresh unless the House campaign is sealed.
- Open gear, character, or tree first, then press `J`; the narrower pane must
  close. `Esc` closes the journal without exiting, and a second bare `Esc`
  exits normally.
- At 1366x768, confirm all journal copy and the `J / ESC` footer stay inside
  the Framekit panel and no world hover tooltip draws through it.
- After Rot in the Reeds, confirm the journal opens Oath of Tin, then advances
  through Salt, Chalk, and Copper only after entry, Warden defeat, and return
  on each exact road. Copper completion should seal the campaign and grant the
  first charted tablet.
- Clear a tier-one road holding, re-admit a Scion, and confirm the chart still
  marks it cleared and opens its tier-two children. A direct request for a
  barred child must leave the player in town.

## Native Wayfinder Mastery

- After sealing the campaign, press `J` in town. Confirm the Chronicle panel
  becomes the Wayfinder's Ledger and shows Barrow, Reeds, Crown, and Thorns as
  four rows of sixteen tier objectives, plus total mastery, highest tier, and
  the current ascent chance.
- Select an unmastered charted tablet in gear and confirm the footer says
  `NEW MASTERY`; a previously cleared family/tier must say `MASTERED`.
- Clear the Seal-Bound Warden and confirm the first clear adds one mastery,
  grants tier-scaled House renown, and updates ascent chance immediately.
  Repeating the exact family/tier should increase only the expedition count.
- Re-admit another Scion from the same House and confirm the mastery pips and
  counts persist. Malformed, duplicate, or out-of-range Chronicle keys must
  never create extra pips.

## Native Tactical Map

- Press `Tab` in town and on a road; confirm the translucent Framekit chart
  opens over the live world and renders the authoritative walkable topology,
  Scion, foe/elite, townsfolk, and exit markers.
- Use the mouse wheel or `[` / `]` while the chart is open and confirm map zoom
  changes without changing the world camera. Use `-` / `=` to step opacity.
- Press `Shift+M`, close the chart, and confirm the compact map changes sides.
  Open gear while the right side is selected; the compact map should yield to
  the left rail and return right when gear closes.
- Restart the client and confirm zoom, opacity, and compact-map side persist,
  while the large chart starts closed. Escape must close the chart before it
  exits the client.

## Movement Smoothing
- Log into a world, hold `W` then change direction diagonally and confirm transitions stay eased with no snapping.
- Compare the move duration to `DEFAULT_MOVE_DURATION_MS` by counting tiles travelled; ensure long strides remain proportional.
- Watch NPC patrols for hitching after resizing the window; interpolation should remain smooth at 60 FPS.

## Pane Mechanics
- Desktop >= 1200px: click the HP orb (or press `S` in debug) to open the Stats pane; confirm the pane docks left, the world view stays centered, and closing with `Esc` restores layout.
- Desktop: click the MP orb to open Inventory; verify the pane docks right, retains scroll position on reopen, and outside clicks close it.
- Tablet 768-1199px: resize the window, open Stats/Inventory, ensure the overlay slides in, ESC + backdrop click closes, and focus returns to game.

## Chat + Quickbar
- With chat collapsed, send/receive a message; badge increments, preview updates, and `Show chat` opens the overlay without shifting the canvas.
- Keep chat unpinned, interact, and verify it auto-collapses after ~8s of inactivity; pinning disables auto-hide.
- Press numeric keys `1-8` while the canvas is focused; the matching quick slot highlights briefly and activates its assigned skill.
- Hit `/` to open chat and focus the input, then `Esc` to collapse (when unpinned) without affecting panes.

## Responsive Behaviour
- Sweep viewport widths 480px to 1920px ensuring:
  - Canvas scales smoothly while preserving a 16:10 aspect ratio; horizontal scroll never appears.
  - Pixel edges remain crisp at 2x scale; no browser smoothing or stretching artifacts when resizing.
  - Quickbar stays anchored between orbs; chat toggle relocates (fixed) on mobile.
  - Pane overlay switches between push (desktop) and float (tablet/mobile) with safe tap targets.
- Mobile (<768px) landscape: open chat; overlay behaves like a bottom sheet and world remains scrollable.

## Pane Scroll Checks
- Populate Inventories (use bank debug) and confirm overflow areas support native scroll momentum on all breakpoints.
- Switch between Stats <-> Inventory; the previously visited pane should restore its scroll offset and focus the first control when reopened.
