# Native first-slice actor completion

`starter-wave.png` is the actual first-pack production capture with the final
player, NPCs, enemies and procedural village dressing. The other two captures
exercise strike and removed-actor corpse rendering beside the actual player.
No image compositing was used to produce these game captures.

Enemy source contacts were inspected at native and2x size in all four cardinal
views. The hit set was changed to include recovery; the death set includes an
intermediate falling pose instead of repeating a settled corpse. All final
frames retain shared48px/metre scale and the fixed96/128px ground pivots. The
two NPCs were replaced after in-game review exposed their older heavy contours.

Validation: `native/build.ps1 -RunTests`, `test-install-first-slice-art.py`,
`verdigris_client --scenario first-slice-art`, and
`verdigris_client --scenario starter-slice` passed. The first-slice regression
actually renders every frame of both enemy identities and checks corpse
selection after authority removes the actor. The starter scenario exercises
occupation, tool, three encounters, victory, earned skill point and departure
through the socket/reducer/production painter. Real contact cadence and damage
are separately verified by starter authority and session tests.

Package verification must run from the clean source commit and retain every
physical-GPU40ms gate. Technical completion does not claim owner visual approval.
