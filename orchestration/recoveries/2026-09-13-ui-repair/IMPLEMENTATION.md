# Native UI repair

This repair continues `codex/native-consolidated-20260913` from
`295cff296d150717f7342ad6e1237e48a95c398a`. It does not replace the consolidated
renderer or reset either recovered history.

The native equipment panel now follows the supplied WIZARD composition: four
deliberate columns, tall outer hand regions, garment/neck/jewelry and
head/body/waist interiors, and an uninterrupted square backpack below. The
native 12 by 7 capacity and all 14 existing seats remain authoritative. Empty
seats have quiet outlines; actual items use aspect-fitted artwork. Item names
and comparisons are contextual. Equip, Unequip, Character, Close and the normal
Equipment/Character/Settings controls operate through production input handlers.
The character sheet shows authoritative values with optional attack details.

Normal play excludes the diagnostic mixer, asset status, duplicate identity and
permanent control paragraphs. F3 retains diagnostics. Error feedback and mute
state remain available. The combat log is bounded above the combat HUD. The
empty, unauthoritative skill-tree placeholder is hidden.

## Reference reuse

Both supplied images depict WIZARD; the landscape image also contains browser
chrome. The native before-image comes from the previous packaged native build,
not from relabeling that browser screenshot.

`Z:/Code/WIZARD/tools/rpg_inventory/index.html` supplies the actual `.doll-col`
composition and frame treatment. The quiet native gradient replaces the
checkerboard caused by stretching the small FrameKit texture. Existing ornate
frame artwork is reused.

`native/tools/import-inventory-art.py` imports 38 unchanged current WIZARD PNGs
from `00be6a0fe4ecfbd0b7862249012308268f92f0d7`, plus six individually identified
historical compatibility PNGs for forms/materials still present in native core.
Their individual source commits and SHA-256 values are recorded in the asset
manifest. Those six imports retain existing game content; they do not reinstate
retired WIZARD mechanics or change drops. Bronze Gloves use the matching bronze
scale glove illustration, and Bronze Med Helm uses the bronze helmet.

The early flint handaxe identifier is retained for saves/protocol compatibility,
but its inventory illustration and world attachment are an unhafted handstone.
Generated color and segmentation originals are preserved. The reproducible
conversion makes a real alpha channel; Pixel Respecter is used only for the
12 by 18 world attachment at the existing actor texel scale. WIZARD inventory
illustrations are not pixelized. Conversion provenance identifies the local
Pixel Respecter source hashes, including its pre-existing dirty state.

## Verification scope

The expanded inventory scenario connects to a real native WebSocket server,
arranges an isolated owned-item fixture, and drives the production HWND handlers.
It covers complete footprints, compatible and rejected destinations, visible
Equip/Unequip buttons, equip acknowledgements, two-hand conflicts, full-pack
atomic rejection, conservation, cancellation, tooltip bounds, and UI input not
leaking into combat. Representative equipment covers the ten seats with native
item content. Cloak, Warhorn, Quick Rig and Attendant retain their empty regions;
no content is invented merely to fill a screenshot.

Obsolete visual assertions requiring the old dashboard and placeholder panels
were replaced with bounds and absence checks. Gameplay, protocol, conservation,
animation and frame-budget requirements were retained. Package verification and
before/after evidence are recorded separately after building the clean source.
Passing tests and agent-reviewed captures do not imply owner acceptance.
