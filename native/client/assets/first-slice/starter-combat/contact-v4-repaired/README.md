# Contact timing and grounded reaction sources

Native attacks use normalized effect phase, not the Blender preview FPS. For 16 frames, preparation occupies indices 0–7, contact is index 8 at phase 0.5, and recovery occupies 9–15. Melee effects last 300 ms and sweeps 400 ms; these source changes do not alter gameplay timing.

Current club sources are `male-attack/club-attack-transition.blend` and `female-attack/club-attack-transition.blend`. Their maximum forward club extension is at index 8, corresponding to the original `Sword_Attack` source frame 19. The previous 16-frame version put this extension at index 9. All new samples come from the actual source or skeletal entry/recovery blends; no raster offsets are used.

The `unarmed-attack` sources here use real `Punch_Cross`, not a hidden-weapon club swing. Index 8 corresponds to maximum right-fist extension at source frame 9. Their final finger closure is superseded by `../contact-v5-fist/`; use those two scenes for projection. Both preserve this phase mapping.

| Index | Club source | Punch source |
|---:|---|---|
| 0 | Actual equipped idle | Actual unarmed idle |
| 1–3 | 25/50/75% skeletal entry | 25/50/75% skeletal entry |
| 4 | Source 0 | Source 0 |
| 5 | Source 5 | Source 3 |
| 6 | Source 9 | 1/3 skeletal travel from source 3 to 9 |
| 7 | Source 15 | 2/3 skeletal travel from source 3 to 9 |
| 8 | Source 19, contact | Source 9, contact |
| 9 | Source 25 | Source 15 |
| 10 | Source 36 | Source 19 |
| 11 | 20% recovery to idle | Source 24 |
| 12–14 | 40/60/80% recovery to idle | 25/50/75% recovery to idle |
| 15 | Actual equipped idle | Actual unarmed idle |

All attack scenes use keys at `1 + 4*i`, 128×128 frames, anchor (64,96), and 48 px/metre. `manifest.json` provides exact paths, hashes and normalized phase roles. Twenty-FPS GIFs are review previews only. Independent reach and endpoint checks are in `../provenance/aligned-contact-reach.json`; the contact landmark is a forward-extent proxy, not a replacement for the game's collision solver.

`female-death/club-death.blend` supersedes the earlier repaired female death. Active-character mesh inspection found real floor penetration. World-Z translation of the root and bound cloth together corrects only penetrating poses, preserving all horizontal motion and camera settings. Final correction is +0.049575 m; all eight samples have minimum geometry height at least +0.008 m. Native frame/anchor remain 128×128/(64,96). Its `grounding.json` records each change. Four directions were inspected and newly restored skin/tunic intersections remain zero. Male death and both hit reactions remain in `../transition-v3-skin/`.

The earlier interpretation of a raised rear foot in the initial combat stance was incorrect. Source ankle heights differ by about 0.55 mm; target differences are 0.066–7.501 mm, and mapped thigh/calf/foot directions agree with the source. The rear foot projects higher because it is behind the front foot. `../provenance/retarget-grounding-audit.json` records source and target ankles, toes and segment angles. Hit endpoints remain grounded, recover the head forward from recoil, and retain exact world pivot and scale. No anticipation was added to hit/death.
