# Contact, palette and item-art integration — September 10, 2026

The 139-asset catalog now includes six NE hero strikes and six SW raider strikes.
The hero uses the accepted idle palette through actual Pixel Respecter CIELAB
palette snapping. Equipment follows its measured hand. Drops select their
physical item family, and an existing village dwelling uses the compatible
storehut image. No generated pixels from Diablo enter this repository.

## Viewed production evidence

- `live-3440x1440.png`: supported final `play-native.ps1 -Local` launch, viewed
  at original resolution; clean Escape exit 0 and no remaining game windows.
- `raider-strike-warning-0.png` through `-2.png`, `raider-strike-contact.png`,
  and `raider-strike-recovery-1.png` through `-4.png`: a real elite Sweep
  advances warning/attack events and reduces player life 100→92. The first
  confirmed hit paints phase 3. Remaining presentation-clock samples show
  follow-through 4, recovery 5 and return to idle; they do not dispatch a second
  simulation attack.
- `warning-opaque-rejected.png`: the first capture hid both actors under an
  opaque red disk. Sweep and thrust now preserve interior pixels while their
  boundaries and timing chips remain. The corrected warning was viewed.
- `strike-equipped-ne-contact.png`: an axe earned through kill/pickup/equip,
  then a real NE hit with phase 3 and resolved damage. Contact flashes obscure
  some body detail; the native equipment grids separately show every pose
  without the flashes. These ordered images were inspected, not claimed as
  watched real-time animation.
- `raster-loot-1366x768.png`: twelve supplied review drops paint distinct item
  families through production drawing. Fixture labels are descriptive IDs in
  a valid local session; separate assertions exercise supplied display names.
  This static art grid does not prove pickup/loot generation.
- `village-storehut-1366x768.png`: the existing foreground dwelling changes art
  without new scenery or collision changes. Town selection is implemented at
  Mara's existing stall; a separate live town capture was not performed.

`retained-evidence.json` maps all 32 retained artifacts to original paths and
raw hashes. Text is retained as UTF8/LF; binary bytes remain exact. NE and raider
source ancestry and selected recipes have separate dependency manifests under
`native/tools/raster/`. Historical discarded batches remain historical and are
not represented as reproducible from the curated subset. The active imports
are reproducible from their preserved selected native/source inputs.

## Validation and limits

Supported final build/all 68 native scenarios and native suites exit 0. Browser
playtest passes 32/32, importer 10/10, and equipment 60 poses×5 weapons×3 scales plus
fractional sampling/cache checks pass. Twenty 3440×1440 frames average 22.550 ms;
moving frames average 23.993 ms with peak 33.926 ms. The 40 ms average gates remain.

The first loot fixture reset its simulation and crashed when the existing
pickup prompt dereferenced the missing session. Keeping a valid local session
fixed the fixture; it did not require changing normal gameplay behavior.

NE follow-through is still broader/darker and its return to recovery is abrupt.
Bow/staff attach but do not depict firing or a dedicated staff action. Raider
attack art is SW-only. Native plain melee emits no attacker-owned AttackStarted,
so this verified elite action does not establish animation for every ordinary
native enemy hit; identified remote hit events have a contact fallback. Native
monster pursuit, other monster motions, pixel effect/death art, cross-clip
identity and terrain repetition remain open.

The earlier browser rejected local-file animation playback; the exact denial
and no-workaround scope are retained. Frame inspection, encoded timings and
production event tests do not certify smooth watched playback. This closes an
integration milestone while the broader visual goal remains active.
