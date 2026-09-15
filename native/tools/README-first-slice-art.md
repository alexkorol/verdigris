# First-slice runtime art

`install-first-slice-art.py` copies visually accepted PNGs into
`native/client/assets/first-slice/runtime` without changing any pixel or alpha
value. The native client reads `manifest.tsv` there. Production packages already
include this directory through the existing native asset packaging path.

```powershell
python native/tools/install-first-slice-art.py path/to/accepted-manifest.json
python native/tools/test-install-first-slice-art.py
$env:VERDIGRIS_CAPTURE_ROOT = "$PWD/native/build/first-slice-integration"
native/build/verdigris_client.exe --scenario first-slice-art
native/build/verdigris_client.exe --scenario fable-world
```

The import JSON contains `accepted: true` and a `clips` array. Each clip supplies
`identity`, `action`, `direction`, `fps`, `loop`, `frame: [width,height]`,
`anchor: [x,y]`, `pixels_per_metre: 48`, and an ordered `frames` array of PNG
paths relative to that JSON. Explicit visual acceptance precedes import; alpha
and dimension checks cannot detect wrong anatomy, poses or painted checkerboards.

Actor directions are `front`, `right`, `back`, `left`. Actor frames have a fixed
canvas and pivot within each clip. Different actions may add transparent padding:
96x96 with pivot [48,80] and 128x128 with pivot [64,96] place the same source
pixels at the same world position. No frame is fitted to its alpha bounds.
Scenery uses the same 48 pixels per world tile and its authored ground pivot.
Inventory uses action `icon`, direction `front`; its transparent padding may be
excluded from fitting so native ink fits the authoritative item footprint.

Player identities include equipment: `player_male_unarmed`, `player_male_club`,
and the corresponding female identities. Complete idle, walk, sprint, attack,
hit and death clips in all four directions are required for **both** starter
equipment variants before a new player family becomes active. Monsters require
idle, walk, attack, hit and death in all four directions. Partial collections
can be inspected in the native scenario, but do not replace gameplay actors.
NPC identities are explicit scene bindings, never inferred from display names.

Monster death effects retain the exact authored identity and event facing after
the live snapshot removes the actor. Death playback uses elapsed time and that
clip's FPS, canvas and pivot, then holds its last frame until the corpse expires.
It does not reuse the old raider death when a complete new family is active.
Player death similarly retains the last living appearance, equipment variant
and facing after authority clears carried items. The same update may retire the
instance to the surface; this single death transition retains visual identity.
A later scene change or a new life clears it. No gameplay item is retained.

Attack clips follow the existing normalized combat beat, not their manifest FPS:
melee spans six 50 ms ticks; sweep spans eight. Input preparation begins at phase
0 and authoritative contact reconciles to phase 0.5. For 16 frames, index 8 is
contact, indices 0-7 lead into it, and indices 9-15 recover to idle. This preserves
gameplay timing; a longer frame list adds pose samples rather than slowing combat.

Imports replace the entire incoming identity's clip list. All equipment variants
of one player appearance form one replacement cohort: importing a male unarmed
update also removes old male club entries unless that import includes them. This
prevents animation or equipment transitions from mixing different appearances.
Unreferenced old PNGs are not loaded. Other character cohorts are preserved.

The terrain uses a 3840x3072 point-sampled image for its existing 80x64 tile
streaming patch. It retains one resident packet and one asynchronous bake, about
98 MiB peak CPU terrain memory. Generated mip chains are disabled for this
logical pixel grid. The existing 40 ms average native frame gate is unchanged.

The inspection scenario detaches its test world from simulation synchronization,
then renders through the production GPU and inventory painters. It checks actual
asset trace entries and GPU success, rather than treating a saved PNG as proof
that a sprite was drawn. Its captures establish presentation evidence, not a
completed village story, authority integration, or asset coverage.
