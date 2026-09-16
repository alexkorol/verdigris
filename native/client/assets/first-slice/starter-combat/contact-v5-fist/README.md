# Final empty-fist punch sources

Use `male-unarmed-attack/unarmed-attack-transition.blend` and the female equivalent. These preserve the contact-v4 real `Punch_Cross` motion, grounded unarmed idle endpoints, 16-phase mapping, and contact at index 8. The club is hidden throughout; this is not a weapon-swing animation.

Close-up inspection found the inherited tool-grip hand left a finger gap. The correction moves the four curved striking fingers toward a palm-plane target with bounded joint rotations, then blends that closed shape through preparation and recovery. The thumb remains outside. No wrist, arm, body, clothing, camera or world pivot changes. The actual idle hand shape is preserved at both endpoints.

All 128 directional references pass native 128×128 bounds at anchor (64,96), 48 px/metre. First/last images match exactly in all eight clips. Independent bone/reach checks confirm identical endpoint matrices and maximum forward fist extension at index 8 for both sexes. Native sheets and 384-pixel four-direction contact views were inspected; a 1536-pixel close-up verifies the compact closed-fist silhouette. The supporting measurements are in `../provenance/fist-contact-reach.json` and each scene's `fist.json`.

`manifest.json` maps exact frame hashes and normalized effect phases. Game timing comes from the native effect's phase; the 20 FPS GIF is only a review aid. Projection must use the final frozen Blender hashes in `../provenance/package.json`, not an earlier prepare-only preview.
