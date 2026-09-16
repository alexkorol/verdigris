# Club attack entry and recovery proof

The previous eight-frame attack begins and ends in Quaternius `Sword_Attack`'s staggered ready stance. Its immediate cut from upright club idle changes stance abruptly before image generation. Earlier visual inspection incorrectly described a raised right foot: the rear foot projects higher in the oblique view, but target ankle heights differ by only 1.98 mm for the male and 1.36 mm for the female. The later source/target audit in `provenance/retarget-grounding-audit.json` verifies this distinction.

This candidate wraps the existing eight phases in actual skeletal preparation and recovery. Each sex has 16 frames in four cardinal directions: grounded idle, three quaternion blends, the eight original attack phases, three recovery blends, grounded idle. The original attack assets remain untouched. Original clothing keys are copied for the interior phases and idle endpoints. New clothing keys follow the blended skeleton; the club remains bound to the right hand.

Each frame is 128×128 at 48 pixels per metre, anchor (64,96). The suggested playback is 50 ms per frame, 800 ms total. The editable scenes use keys at 1+4*i with 80 fps. `manifest.json` records every frame path and hash. Contact sheets and GIFs use nearest-neighbor enlargement only.

Verification: all 128 new frames stay inside their native canvas. The 64 interior attack silhouettes match the original thresholded alpha masks exactly. First and last images are identical in all eight clips. Existing 96×96 club-idle references padded by 16 pixels differ only at 8–18 thresholded edge pixels; camera scale and skeletal pose are unchanged.

The painted male attack's phase 1→2 whole-silhouette centroid discrepancy is not evidence of equivalent body translation. Windows around projected Blender landmarks give motion differences of 1.042 px at the head, 0.266 px at the chest, and 0.272 px at the pelvis. The annotated overlay confirms the head/chest/belt follow the reference while painted arm, club and hair coverage changes silhouette area. These window centroids are diagnostics, not automatic anatomical landmark detection.

This is a source candidate, not final acceptance. Entry/recovery soles are grounded, but horizontal foot planting is not constrained. The inherited extreme attack poses can expose masked underarm skin; repair that source geometry before accepting a projected final sequence. The new preparation/recovery poses were inspected front and side for braid/clothing behavior. No new image generation was requested for this revision.

Rebuild: `render_starter_combat_transition.py` in background Blender with `-- male` or `-- female`, then run `render_starter_combat_transition_qa.py` with Python.
