# UI Regression Checklist

## WIZARD Framekit Raster Contract

- Build from the repository root and confirm the Framekit verifier reports 99
  flagship rasters and 11/11 required runtime assets. Removing or changing any
  frozen file must fail the build before gameplay checks begin.
- In town and on a road, confirm the normal HUD reports `WIZARD Framekit 11/11`.
  The life/resource orbs must carry the asymmetric statue chrome, all four
  action wells must show their authored medallions, and the bottom XP fill must
  sit inside the long brass rail without overlapping either orb.
- Open Gear, Character, Journal, Vesselforge, dialogue, and the tactical map.
  Their production panes must use the marble-and-brass raster border and dark
  stone center at both 960x600 and 1366x768; text, hit targets, and backpack
  footprints must remain inside the usable center rather than the decoration.
- Hover a foe, townsfolk, or drop with all modal panes closed. Confirm the
  tooltip uses WIZARD's authored frame. Opening a modal must still suppress the
  tooltip. If decoding fails, the on-screen status must name the fallback and
  the client must remain usable rather than drawing an empty pane.

## Native Town Vesselforge

- Stand beside Tamar the Vesselwright and press `T`. Confirm her authored
  conversation opens and `Open the Vesselforge` leads to a centered Framekit
  list/detail surface. The normal objective and controls HUD must not draw
  through the modal.
- Confirm every carried vessel row shows its current name and either `READY | 100
  GOLD` or a precise full-vessel, spent-patience, or insufficient-gold reason.
  The detail side must agree on item level, material/form, Vessel use, Patience,
  and active versus Dormant lines.
- Use Up/Down and Enter, then repeat with a pointer click. Exactly the selected
  UUID must gain one Brand, lose one Patience and 100 gold, and refresh in the
  still-open pane without stale name or combat values.
- Attempt the same action from across town and after leaving the town. Neither
  the vessel nor the purse may change. Equipped gear must not be offered as if
  it were carried.
- At 960x600 and 1366x768, confirm five rows, the item detail, action line,
  paging count, and footer stay inside the Framekit panel. Moving beyond the
  fifth row must page the list while retaining keyboard focus.
- Clear a floor while wearing unsated Vessel gear. Confirm the completion feed
  names that exact item and awards 16–30 Attunement; carried and stored items
  must not learn. Reconnect and confirm the worn item's progress and road-theme
  memory are unchanged.
- Continue clearing dungeon roads until the first 80-point threshold. Tamar's
  pane must report one evolution and one Warding Bond, with the full tier-I
  effect labelled `Dormant - BOND` rather than presented as a live combat stat.
- On a three-slot unbranded test Vessel, continue through three Bonds, tier III,
  and awakening. Confirm the awakened name begins with the current Scion's
  name, the power stays visibly Dormant, and the pane remains bounded at both
  reference resolutions. Attempting to sear a full bonded Vessel must change
  neither item nor purse.

## Native Vesselforge Active Properties

- Equip an Obsidian Macuahuitl and confirm its card says `Hits cause Bleeding`
  without a Dormant prefix. A Long Reach brand must appear as a second active
  line, and the compact forge summary must match the authoritative worn totals.
- Strike a durable foe and confirm bleed application has a crimson ring/slash
  treatment and `BLEED` label. Later ticks must use crimson numbers without
  replaying the weapon swing or producing the generic gold impact burst.
- While bleed remains active, confirm the foe keeps a restrained crimson
  status ring. Hovering it should say `Bleeding`; ranged marsh/grove and
  dungeon foes should identify River or Ember damage respectively.
- Equip Sandals/Surefoot gear and confirm held movement covers the increased
  authoritative distance without changing the 50 ms input cadence or moving
  through walls. Long Reach should admit targets beyond the unmodified edge.
- Compare a River or Ember volley before and after resistance gear. The damage
  number must match server mitigation, and reconnect must restore both worn
  totals and any still-active bleed state.
- Equip an Atlatl and confirm `+20% Projectile Range` admits a target exactly
  one tile beyond the ordinary ranged edge. Equip Grips and confirm held primary
  attacks visibly recover faster while the summary reports the exact speed.
- Equip a Sling against an armoured foe. Its card must say `Ignores half of
  Armour`, the foe hover must show its Armour value, and the improved hit must
  carry a cyan `PIERCE` number matching the server's mitigation facts.
- At 1366x768 and 960x600, confirm both forge lines, both summary rows, banked row,
  and footer stay inside the Framekit gear pane while bleed feedback remains
  visible in the world.

## Native Monster Pressure Roles

- Approach a ranged foe from four to six tiles. Confirm a violet circular
  reticle appears on the Scion's sampled tile for roughly 800 ms and does not
  falsely originate beneath the caster.
- Leave the entire marked area before it resolves and confirm no damage lands.
  Stay inside the next warning and confirm exactly one named volley lands.
- Land the third melee cadence beat during a volley warning and confirm the
  finisher interrupts it. The warning must disappear immediately and no volley
  damage may resolve from it.
- Wound a pack member near a buffer and confirm the most-wounded ally receives
  a green double ring, cross, and `+N` number matching the authoritative heal.
- Approach a mixed pack and confirm melee/Warden actors route around walls,
  ranged actors retreat before casting when crowded, and supports close on a
  wounded ally. Steps should glide for their authored 400 ms rather than snap.
- At 1366x768 and 960x600, confirm circular warnings are clipped away from the
  minimap and bottom HUD while the support treatment remains visible in-world.

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
  on each exact road. Copper completion should open `ACT III THE DEEP ROADS`,
  not seal the campaign or grant a tablet.
- Confirm the Deep Roads revisit tier-two Tin, Salt, Chalk, and Copper in that
  order and name the Quarry Saint, Brine Widow, Ossuary Bell, and Cinder Judge.
  Each journal rite must advance only after exact entry, Warden defeat, and a
  living return; the Cinder return must open Act IV at twelve points without
  granting a tablet.
- Continue through tier-three Crownless Marches and tier-four War of Claimants.
  Confirm each road chart preserves parent gates, real depth scaling, and its
  canonical Warden name. Act VI must open at twenty points.
- Complete the tier-five Iron and Salt claims, then verify `A Crown Without a
  King` retains its Chalk return at objective 4/6 before sending the Scion to
  Copper. Only the Verdigris Usurper's defeat followed by a living return may
  seal 23/23 and grant the first charted tablet.
- With eight or more completed deeds, confirm the left Chronicle rail remains
  inside Framekit. Once history exceeds nine visible rows it must collapse the
  oldest entries into a truthful `EARLIER DEEDS` summary rather than clipping.
- Clear a tier-one road holding, re-admit a Scion, and confirm the chart still
  marks it cleared and opens its tier-two children. A direct request for a
  barred child must leave the player in town.

## Native Wayfinder Mastery

- Fight one Warden from each road family and confirm the warning identity is
  readable before impact: gold Stonefall for Tin, blue Tidal Mark for Salt,
  bone-violet Grave Ring for Chalk, and orange Ember Crucible for Copper.
- Step out of a sampled Stonefall/Tidal Mark before resolution and confirm the
  original marked tile does not follow the Scion. Stand inside Grave Ring's
  inner eye and confirm it is safe; stand in the annulus and confirm it hits.
- Confirm River resistance mitigates Tidal Mark and Ember resistance mitigates
  Ember Crucible. Warning circles and labels must not paint over the minimap or
  bottom Framekit quickbar at either supported capture size.
- Break Barrow, Reeds, Crown, and Thorns tablets and confirm their Seal-Bound
  Wardens reuse the corresponding four learned disciplines at tablet-scaled
  damage.
- Select a charted tablet in the town gear pane and confirm its footer exposes
  a gold `V rechart 50g` keyboard-and-pointer action. Re-chart once: exactly 50 carried gold must be spent, both
  visible clauses must change as a set, and UUID, family, tier, and `NEW
  MASTERY`/`MASTERED` state must remain unchanged. The same request in an
  expedition or without 50 gold must change nothing.

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
