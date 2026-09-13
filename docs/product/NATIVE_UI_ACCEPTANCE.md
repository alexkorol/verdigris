# VERDIGRIS — NATIVE UI REPAIR DIRECTIVE
## Apply to the inventory work already underway

This is owner direction for implementation, not a request for another
plan, audit, or approval round.

Continue the current inventory repairs on the consolidated native game.
Incorporate the requirements below into that work. Do not finish only
the underlying inventory mechanics while leaving the demonstrated
presentation problems for an unspecified future polish pass.

The owner has repeatedly reported these problems and cannot supervise
every implementation detail. Do not require another exhaustive bug list.

FIRST: MAKE THIS DIRECTION DURABLE, THEN KEEP IMPLEMENTING

Update the actual repository instruction entry point, normally AGENTS.md,
with a short set of binding rules and a link to the detailed UI contract.

Put the detailed contract in the existing relevant native UI document,
or docs/product/NATIVE_UI_ACCEPTANCE.md if there is no suitable document.
Do not create several competing specifications.

The short AGENTS.md addition must establish:

- The native game is the product. Work must land in the consolidated
  native implementation, not an isolated demonstration.
- Owner-supplied visual references constrain implementation. A generic
  substitute with similar colors or borders is not equivalent.
- Normal player UI must not expose development diagnostics, placeholder
  copy, redundant instructional text, or overlapping controls.
- Existing reported defects remain obligations until the affected
  behavior is demonstrably fixed in the actual package.
- Tests passing does not establish visual quality or owner acceptance.
- Commit and push verified implementation under the standing policy.
  Incomplete checks must be reported accurately; they do not establish
  a blanket ban on publishing independently verified work.
- Documentation is supporting work. After updating it, resume the
  current implementation without requesting another planning approval.

Preserve unrelated instructions and existing work. Resolve directly
conflicting active UI instructions rather than appending a contradictory
paragraph. Do not conduct a repository-wide policy rewrite.

## 1. Evidence and authority

Two attached images have different roles.

REFERENCE A — CURRENT NATIVE UI / FAILURE EXAMPLE

The landscape gameplay screenshot shows the native character sheet,
equipment/backpack panel, HUD, and messages. It demonstrates presentation
problems. Its current arrangement is not the target.

REFERENCE B — WIZARD INVENTORY / PRESENTATION TARGET

The portrait inventory image is the owner-supplied target for inventory
composition and item presentation. It shows:

- One coherent outer frame.
- A dark, visually quiet interior.
- Large item artwork as the primary content.
- Differently proportioned equipment regions arranged deliberately.
- Tall weapon and shield regions on the outer sides.
- Garment, helmet, torso, jewelry, belt, and footwear regions arranged
  within that composition.
- Consistent spacing between regions.
- A broad, uninterrupted backpack grid below the equipment.
- Little permanent text competing with the objects.

Inspect both images directly. Do not work from this description alone.

Inspect the actual WIZARD source, layout, item-art assets, and applicable
FrameKit resources before inventing replacements. Relevant repositories:

https://github.com/alexkorol/WIZARD
https://github.com/alexkorol/verdigris

Identify which source implementation corresponds to the reference.
Do not assume that the most recently found UI file is the right one.

WIZARD is an authoring/reference source, not permission to revive the
legacy browser game. Reuse appropriate assets and design logic in the
native implementation. Do not replace native interaction with a static
screenshot, a browser overlay, or an unrelated mockup.

IMPORTANT LIMITS OF THE REFERENCE

The reference establishes presentation, not every gameplay rule.
It does not independently approve:

- Inventory capacity or a new equipment-slot schema.
- Rarity meanings, colored-border rules, or diamond-marker mechanics.
- New item statistics or a fully equipped starter character.
- Every depicted object as approved production content.

Preserve authoritative gameplay data. Do not change capacity or grant
equipment simply to make the screenshot easier to reproduce.

The portrait reference also does not require making the game portrait.
Adapt the composition deliberately to the supported game viewport.

## 2. Immediate cleanup: remove the development dashboard from player UI

Reference A visibly contains several competing information surfaces:

- House/Scion identification and stacked control reminders at top left.
- A separate audio/status panel with volume, theme, and loop information.
- A “Gear opened” message despite the gear panel visibly being open.
- An additional “audio muted” message.
- Abbreviated equipment buttons such as HEAD, AMUL, CLOK, GLV, and R1.
- Repeated “Skill tree: no data yet” text.
- Instructional footers explaining keys and dragging.
- Character-sheet phrases such as “reach | bronze pike,”
  “pressure | close blade,” and “magic | attuned vessel.”
- Dense statistics, empty-state copy, and administrative counters.

Do not normalize this as acceptable placeholder UI.

A. Remove redundant confirmation messages

Opening the inventory should visibly open the inventory. It does not
also need a floating “Gear opened” announcement.

Do not announce every routine open, close, focus, and successful click.
Keep feedback for meaningful events, errors, and actions whose result
would otherwise be unclear.

B. Move diagnostics out of normal play

Audio theme/loop identifiers, renderer details, raw source breakdowns,
and other developer state belong in the existing opt-in diagnostic
surface or logs—not in permanent player-facing panels.

Useful audio controls belong in Settings. A compact mute indicator may
be appropriate; several competing audio text blocks are not.

Retain build provenance in an unobtrusive appropriate location. Do not
turn traceability into another always-visible debug paragraph.

C. Replace permanent instruction litter with contextual help

Do not scatter “press key,” “Enter equips,” “Esc closes,” and similar
reminders around every window.

Use real clickable controls and normal focus behavior. Explain an
unfamiliar action at the point it matters, through a tooltip, concise
contextual hint, or help view.

Hotbar key labels may remain compact and intentional. Display actual
bindings, not stale hard-coded strings.

D. Remove unfinished-system advertising

“Skill tree: no data yet” is not a player feature. Do not display it
repeatedly in normal inventory or character views.

Hide nonfunctional scaffolding rather than presenting it as usable.
Keep any necessary implementation diagnostics elsewhere.

Do not hide meaningful gameplay information merely because it is zero.
Distinguish genuinely useful state from placeholder output.

E. Eliminate misleading descriptive copy

The bronze-pike/close-blade/attuned-vessel lines must not remain as
unexplained static character decoration.

Establish what they actually represent. Remove obsolete concept copy;
bind genuine equipment or ability descriptions to actual state.
Do not imply that an unarmed character has equipment it does not own.

This cleanup does NOT authorize deleting useful combat warnings,
important errors, or accessibility information.

## 3. Inventory composition: reproduce the actual design principles

The target is an object-led equipment display with a backpack beneath
it—not a matrix of equally sized text buttons beside a narrow grid.

EQUIPMENT

Use the WIZARD arrangement and actual source as the starting point.

Give equipment artwork enough space to communicate silhouette,
material, and identity. Preserve aspect ratio and transparent margins.
Do not stretch artwork to fit a slot or crop off functional parts.

Equipment display regions may have different shapes and proportions.
They are not required to be the same size as backpack storage cells.

Small accessories should have deliberately smaller regions. A weapon
should not be reduced to the same tiny visual treatment as a ring.

Occupied regions show the actual equipped item, not a category label
or an unrelated illustrative item.

Empty regions should remain discoverable without a wall of shorthand:
use the existing appropriate slot treatment, restrained silhouettes,
and focus/hover labels where supported.

Use readable names in tooltips and accessibility/focus descriptions.
Internal identifiers such as AMUL or CLOK are not the primary visual
interface.

BACKPACK

Place a coherent storage grid beneath the equipment, following the
reference's visual hierarchy.

Use consistent cell geometry, subtle boundaries, and a quiet background.
Do not render every empty cell as a separate conspicuous rounded button.

Respect the authoritative inventory dimensions and occupancy model.
Do not squeeze cells into thin slivers to force an oversized grid into
the old panel width.

If available space is insufficient, use an intentional responsive
layout or scrolling treatment. Do not silently alter capacity.

Multi-cell items must occupy and render across their actual footprint.
Draw one item image for the occupied region, not a repeated icon in
every cell. Preserve each item's orientation rules already supported
by the game; do not invent rotation or stacking mechanics.

Capacity information should be compact and meaningful. Do not place an
“Empty” label over a usable grid or duplicate the same information in
several places.

CHARACTER INFORMATION

The reference does not require deleting the character sheet. It also
does not justify forcing a second dense sheet into every gear view.

Give equipment management and detailed character information deliberate
placement. When both are visible, they must fit without collision.

Show readable labels and aligned values, with a clear distinction
between essential values and optional detail. Expanded calculations
belong in a deliberate detail view or tooltip.

Character previews and equipment illustrations must reflect actual
character state. Do not substitute a fixed armored figure for a
working appearance/equipment presentation.

## 4. Finish inventory behavior, not only its appearance

Continue the already-identified repairs to item footprints, equipment
destinations, and authoritative character statistics.

Required behavior:

- Dragging shows the actual item's footprint and a readable drag image.
- Hovering a valid destination gives clear feedback.
- Invalid placement is visibly rejected with a useful concise reason.
- Dropping onto every supported compatible equipment slot works,
  not only the main weapon slot.
- Equip, unequip, and swap operations preserve item identity.
- A full backpack cannot cause an item to vanish, duplicate, or
  silently replace another item.
- Any multi-slot equipment rules follow existing authoritative data.
- Pointer interaction with UI does not also attack, move, or activate
  something behind the panel.
- Rendering bounds, hover bounds, and drop targets use the same layout.
- Cancellation and closing during a drag leave inventory consistent.
- Keyboard focus remains usable without requiring arbitrary hidden
  key sequences for ordinary actions.

Bind equipment and character values to the authoritative state.
Do not reconstruct combat statistics using a separate client guess.

When an operation is pending, rejected, or acknowledged by the server,
the UI must handle that transition honestly. A local drag animation is
not proof that equipping succeeded.

Keep Scion equipment, carried inventory, and House storage distinct.
Do not relabel or intermingle them to fit a convenient widget.

Use existing schemas and interaction conventions where they are sound.
This is not authorization to redesign combat, itemization, or progression.

## 5. Layout, text, and visual implementation

NO OVERLAP OR ACCIDENTAL LAYERING

Panels, headings, tooltips, toasts, and HUD elements need explicit bounds,
padding, stacking order, and ownership of their screen regions.

Do not fix one overlap by moving a string a few pixels without correcting
the layout relationship that caused it.

Tooltips must stay within the viewport. Text fields, long item names,
and larger values must not run through adjacent controls.

Handle supported smaller viewports and display scaling deliberately.
Do not make the screenshot look acceptable only by enlarging the window
or shrinking all text until it becomes unreadable.

Where a compact layout is necessary, reorganize or scroll content.
Do not pile panels on top of one another.

TYPOGRAPHY

Use the project's selected UI typefaces and a small consistent hierarchy.
Do not introduce another arbitrary default font or mix incompatible
font treatments.

Maintain readable contrast and intentional line spacing. Important
information must not depend solely on color.

FRAME AND SURFACE RENDERING

Use the actual appropriate FrameKit assets and intended slicing rules.
Preserve corners, edge thickness, content insets, and material treatment.

A gold border around a diagnostic panel is not completion.

The dark block/checker-like backing visible in Reference A is not the
surface target. Determine whether it comes from a placeholder texture,
transparency handling, or another rendering choice. Replace it with the
quiet intended treatment rather than preserving it as accidental style.

Handle transparency correctly. Do not bake checkerboards into assets,
stretch borders, expose black rectangles around cutouts, or introduce
halos through inappropriate resampling.

Respect the established pixel-scale direction. Do not run an arbitrary
pixelization filter over the WIZARD artwork or replace it with a generic
retro style. Inspect the actual imported result at game display size.

Preserve source assets. Make any necessary conversions reproducible,
and keep originals intact.

## 6. Preserve the game and its established direction

This work must retain the consolidated perspective renderer, intended
focus treatment, animation, equipment attachment behavior, and existing
movement, melee, and level-persistence fixes.

Do not switch back to the orthographic fallback to make UI development
or a test easier. Do not replace authored characters with geometric
stand-ins.

Use the active consolidated native lineage and preserve work already
underway. Do not reset to an older reported hash or create another
competing “latest demo.”

Keep native UI implementation separate from legacy web-game acceptance.
WIZARD references are useful; legacy browser tests are not proof that the
native UI works.

Preserve owner saves. Use isolated test data for destructive checks and
art-populated inventory fixtures.

SETTING REMINDER

The early strength-path weapon is a palm-held knapped stone with a
sharpened edge: no haft, no handle, no metal axe head.

The richly equipped WIZARD reference is a presentation reference, not
permission to raise starting technology or equip the novice with its
entire illustrated loadout.

Do not add speculative magic systems, rarity rules, lore objects, or
new equipment categories during this UI repair.

## 7. Implementation and verification must answer the actual complaints

Do not restart discovery of issues already established above.
Inspect only what is needed to implement correctly and reuse existing
WIZARD/native work.

Deliver the visual composition and functioning interactions together.
Do not declare inventory complete because the data model changed while
the normal UI still resembles Reference A.

Use focused automated checks for occupancy, item conservation, valid and
invalid equipment destinations, UI input routing, and authoritative
stat updates. Preserve relevant existing regression coverage.

Also capture and inspect the actual native package in these states:

1. Ordinary gameplay, with developer overlays disabled.
   No stray debug/status panels or redundant instructional clutter.

2. Inventory open with representative actual equipment and multi-cell
   backpack items.
   Clearly show the equipment-first arrangement and quieter bag below.

3. An actual drag/equip interaction and a rejected invalid placement.
   Demonstrate the destination, resulting item state, and stat update.

Use isolated fixtures where necessary and label them as fixtures.
Do not present fabricated item arrangements as evidence of functioning
gameplay interactions.

Check the UI at the supported small viewport corresponding to the
failure example, at 1280x800 where supported, and at the owner's normal
display configuration. Check scaling behavior, not just image resizing.

Compare against Reference B for composition, spacing, artwork dominance,
grid treatment, and text density. Different aspect ratios do not require
pixel-identical screenshots, but they do not excuse abandoning the design.

Explicitly distinguish:

- Behavior tested.
- Visual states actually captured and inspected.
- Remaining defects.
- Owner approval, which must never be invented.

A test count, file-hash count, or another AI's “looks good” statement
cannot override a plainly cluttered or incorrectly composed screenshot.

If Computer Use is stopped, stop automated computer input. Do not treat
that alone as cancellation of independent coding and noninteractive tests.
Keep unfinished live checks marked unfinished; do not repeatedly ask for
acceptance before continuing unrelated authorized repairs.

## 8. Definition of the next deliverable

The deliverable is an improved native game the owner can launch—not a
new backlog, another standalone UI demo, or an instruction-file update.

It must include:

- The current inventory functionality repairs integrated.
- A substantial implementation of the supplied WIZARD composition.
- Removal of the demonstrated player-facing debug/text clutter.
- Preservation of the current renderer, animation, and gameplay fixes.
- Updated concise standing instructions linked to this detailed contract.
- Committed and pushed implementation with exact branch/source identity.
- A clearly identified package and honest verification status.
- Actual before/after evidence from the native game.

Do not mark the overall UI goal complete while the equipment remains
primarily abbreviated text buttons and the screen remains littered with
debug messages.

Do not switch the owner's normal launcher to an unverified replacement.
Do not let that promotion requirement block independent repairs.

Begin by incorporating this direction into the inventory work already
in progress. Make the necessary documentation change, then keep coding.
Do not end the task at documentation, planning, or another request for
the owner to explain the same problems.