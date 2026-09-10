Apply `ne-equipment-integration.patch` once from the repository root after the six normalized NE PNGs have been promoted with their recorded hashes. This combined patch contains both the equipment metadata and test additions. The separate header-only patch is an intermediate artifact; do not apply both.

The patch changes only `native/client/raster_equipment.hpp` and `native/tools/raster/test_equipment.cpp`. It adds `hero_strike0_ne` through `hero_strike5_ne`, all on 80x96 canvases, with the reviewed physical-left grips and far-body replay. The six melee angles are 0, -15, -45, 65, 100 and -15 degrees clockwise. Existing metadata and fingerprints are preserved.

The six normalized input PNGs are recorded in `ne-normalized-fingerprints.json`. Several correct grips are dark hand-shadow pixels. Their new test rows check exact ARGB values in addition to the hand-region fingerprint. The older bright-skin checks remain unchanged for all old rows.

No PowerShell probe change is needed. The existing production test enumerates every pose and all five weapons, so the added rows expand its coverage from 54 to 60 poses. The directional neighborhood count becomes42; the separate six SE fingerprints remain unchanged. Existing directional playback generation already includes NE when metadata exists. Its90ms showcase timing is not production timing.

Validation performed in an isolated fixture beneath this review folder:

- `git apply --check ne-equipment-integration.patch` passed before application.
- `test_ne_all_weapons.cpp`: all6 NE poses x5 weapons x1x/2x/3x. Binary alpha, full-canvas clearance, grip rounding<=0.5px and source-exact body occlusion passed. All15 rendered weapon/scale sheets were viewed. Repeating all90 placements100 times caused zero image loads, zero bitmap builds and no GDI handle growth; cache69 bitmaps /5,680,904bytes.
- Copied production `test_equipment.cpp` with the proposed two-file patch and only fixture path/include substitutions:60 poses x5 weapons at96/173/288px passed. All old fingerprints plus six new ones passed.120 strike placements repeated100 times with zero new loads/builds; GDI287->287; cache140 bitmaps /25,912,924bytes. The production PowerShell wrapper and its separate eight-size sampling executable were not rerun here; the raster implementation is unchanged.

The axe, sword and club support this shared overhead swing. The bow and staff attach correctly but retain zero-degree carry orientation while the arm rises: these frames do not show a bow draw/release or a weapon-specific staff attack. Palette normalization improves color consistency; follow-through still has broader/darker body geometry, and its hand drops42px into recovery. Final contact/recovery and facing acceptance belongs to the supported native live game review.

`browser-policy-rejection.txt` preserves the exact prior Browser Use denial, attempted file URL and no-workaround scope. No blocked playback was retried. Ordered frame inspection and encoded timing are not reported as watched video.
