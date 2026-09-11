# Hero lineage asset wave — 2026-09-10

This wave derives male and female playable actors from the owner's two selected images. It preserves ordinary adult anatomy, plain beige belted tunics, sandals, empty hands and the supplied rendering detail. The camera is elevated over a straight grid; the actor turns through eight headings. New image prompts omit the game name and do not introduce patina, equipment or decorative costume.

**Integrated and verified; owner art review remains open.** Both appearances are selectable in the normal native client. Final production checks passed, and the integration owner inspected native-scale playback, equipped production sheets, and both appearances in the live server/client.

## References and actual prompt evidence

Both supplied originals are 1254×1254 RGBA. South idle preserves each original rather than regenerating its identity.

| Reference | Original local path | SHA-256 |
| --- | --- | --- |
| Male | `C:/Users/Alex/Downloads/ChatGPT Image Sep 10, 2026, 05_22_21 PM.png` | `fa6bb5a57dbb0791095abc6078920ae3e834782e3db8dd24b95e16c8f37b5e1f` |
| Female | `C:/Users/Alex/Downloads/ccd7ce15-5d1a-46c4-af49-f305bf7fad90 (1).png` | `9693437c9b6d88674cf64e55749135979c8db8db5576353967b2fe8a57fba742` |

Measured alpha≥128 body bounds, with exclusive right/bottom coordinates: male `[432,128,822,1098]`; female `[445,115,809,1107]`. These are identity calibration, not permission to stretch each animation pose to its bounding box.

The following actual ChatGPT user messages were read through `read_thread`, including during this documentation pass:

- **Create RPG Sprite PNG** — `6aa348ce-61f8-83e9-9f3e-c2bbe13aa16d`: the female single-image prompt.
- **Create RPG Sprite** — `6aa348a0-d934-83ea-96c8-4572be09ac1d`: the male single-image prompt.
- **Generate Character Sprites** — `6aa3465c-3ff4-83ea-b134-66889084fbb1`: the earlier multi-output exploration.

The newer male/female prompts explicitly request “ordinary human proportions,” “physically convincing form,” and contours “slightly irregular and natural.” They retain economical pixel rendering while rejecting rounded cartoon anatomy. Both specify one isolated figure, mundane tunic/sandals, empty hands, a downward-facing elevated frontal view and genuine transparency. The earlier exploration emphasizes clean silhouettes, broad color clusters and multiple style takes. For this wave, the supplied selected images and the newer physical-form instructions govern; a generic simplification template would lose the successful anatomy. The thread retrieval exposed text, not an independently verified mapping of each generated attachment to each downloaded file.

## Deliverables and generation method

The package is `native/tools/raster/hero-lineage-20260910/`. Direction order is `n, ne, e, se, s, sw, w, nw`.

Each sex/direction has five selected source poses in `candidates/<sex>/<direction>/`: `idle.png`, two opposed walk contacts `walk0.png`/`walk1.png`, and attack anticipation/contact `strike0.png`/`strike1.png`. The third runtime attack pose, `strike2`, is a byte-identical idle recovery. This gives 80 selected source poses and 96 runtime entries, including the 16 explicitly reused recoveries. It is a two-contact walk, not an eight-frame gait.

Generation uses the built-in image tool, one isolated pose per call. Original identity accompanies directional edits; animation uses an accepted directional idle and, where useful, the preceding pose. Source manifests retain exact prompts, reference roles/hashes, output paths, rejected attempts and approximate anatomical-left hand measurements. The built-in tool does not expose its model/version, so the package does not independently certify an “Image 2.5” variant.

A useful repair was **an existing correct pose followed by identity replacement**. Female NW contact B repeatedly retained the wrong support. Its final edit uses the successful male NW contact B as geometry and the original woman as identity, preserving joint/contact positions while replacing anatomy, hair and clothing identity. The exact prompt is `candidates/female/nw/walk1-identity-swap-v5.prompt.txt`; its manifest pins both references. Male NE also reuses successful female geometry in `walk1-female-geometry-repair.prompt.txt`. These are pose guides, not mirrored or relabeled images. Male SW used a frontal opposite-contact guide followed by a heading correction; its residual frontal toes remain documented.

This solved specific repeated-support failures. It does not guarantee temporal continuity: inspect which thigh overlaps, which foot supports, the opposite arm swing, facing, scale and return to idle.

## Actual reconstruction and registration

`prepare.py` preserves raw sources and writes separate `derived/` mattes. Most new outputs have opaque RGB checkerboards despite explicit alpha requests; the ten male W/NW originals all failed alpha delivery. File mode and `actual_source_alpha` in each `derived/.../repair.json` are the evidence.

The staging operation removes measured neutral pixels where channel range≤18 and minimum channel≥120, retaining the largest connected body. Sources with actual alpha use threshold80 instead. This can remove pale clothing detail or detached fingers, so the recorded body/component counts and visual matte inspection matter.

The importer calls the actual `pixel_perfecter.workspace.reconstruct` API from `Z:/Code/Python/pixel-perfecter`. Directional provenance records engine source hashes, dependencies, grid offsets, source/output hashes and geometry; the observed engine revision is `cf7786cb438ad51be5dfc08d963c2a6edcfd72f7` with local changes recorded. It is not a resize-only substitute.

Settings are common pitch10, crisp alpha, 160×192 canvas, 32px padding, shared maximum96 colors per directional cycle and `preserve_scale=true`. The directional idle establishes one source anchor for its entire clip, mapped to native `[80,160]`. Each pose retains the reconstructed scale; trim/translation place it without independent body fitting. Approximate source wrists are mapped through the actual grid and translation before equipment review. They are not final hand-pixel measurements.

## Reproduce reviews and play

Run from this checkout. The final evidence below distinguishes importer checks, production scenarios and firsthand play.

```powershell
Set-Location 'C:/Users/Alex/Documents/ChatGPT/verdigris-fable-renderer'
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' native/tools/raster/hero-lineage-20260910/prepare.py --project 'Z:/Code/Python/pixel-perfecter'
& 'Z:/Code/Python/pixel-perfecter/.venv/Scripts/python.exe' native/tools/raster/hero-lineage-20260910/prepare.py --previews-only
```

Inspect `previews/*-idle-eight-1x.png`, per-direction native/3× contact strips and GIFs, and `review-index.json`. Review GIF holds are walk140/140ms and attack80/100/160ms; production attacks follow event phases. Production walking remains distance driven at low speed and caps two-contact cycles at280ms during rapid road travel and dashes, without changing authoritative movement. A still strip does not establish playback acceptance.

After source review, `prepare.py --equipment-only` regenerates mapped sockets and previews. `prepare.py --promote-approved` requires exact output hashes in `approvals.json`; changed pixels invalidate that approval. Neither command itself proves correct on-screen equipment.

```powershell
powershell -NoProfile -File native/build.ps1 -RunTests
.\native\build\verdigris_client.exe --scenario lineage-art
.\native\build\verdigris_client.exe --scenario all
powershell -NoProfile -File native/tools/play-native.ps1
```

The lineage scenario exercises all96 authored selectors/canvases and produces `lineage-male-all.png`, `lineage-female-all.png` and corresponding hardware world captures under `docs/execution/captures/art-wave/` by default. Actual play uses the native server/client launcher; WASD moves, mouse aims and left mouse attacks. In another terminal, capture the live window and inspect it:

```powershell
powershell -NoProfile -File native/tools/capture-window.ps1 -OutPath '.ci-artifacts/hero-lineage-live.png'
```

Residual limits include missing passing phases, weak/asymmetric strides, body/cloth variation between directions, source baseline drift, abrupt idle recovery, far-hand occlusion and equipment-form differences. For example, male NW walk1's source sole is about21px below its idle; male W idle/strikes extend about20px below the original reference. Common-scale registration and native playback must decide whether those differences are acceptable.

## Final production verification

- Combined runtime provenance SHA-256: `90710314fe9f8f9fe8266c65ef0ccb737901f30d7de821ac7daa51472953f286`. Equipment include SHA-256: `78538f5d94f176d06d87593e80eecd21a615ef13554377e66e2aa4f4f938c9c2`. Validation passes all96 runtime entries and all96 hand attachments; catalog passes272 total active PNGs.
- Root watched the native-scale walk/attack loops and inspected all48 production sword-equipped poses for each appearance. Each frame uses its authored selector and socket. The normal client preserves body scale and foot registration when equipment expands the drawing bounds.
- Root created and entered Thirdborn(female), restarted the normal server/client, entered saved Secondborn(male), and observed movement and attacks in town. A live westward male attack follows the newly clicked aim immediately. The new regression checks the same behavior before authoritative facing is echoed. Manual kills and a completed expedition were not demonstrated in this asset wave.
- Final supported build and all76 native client scenarios pass in `.ci-artifacts/appearance-selection/shipping-client.log`. Core/network/session/presentation/audio/camera suites passed in `final-native.log` before subsequent presentation-only fixes. Browser `npm run playtest` passes32/32 in `browser-playtest.log` using Node22.11.
- Hardware GTX1660SUPER:20 full3440×1440 Fable frames average28.555ms; streamed travel34 frames average24.530ms, peak44.126ms. The unchanged gate is40ms average. CPU terrain peak37,343,568bytes. Legacy moving-frame gate averages25.8ms, peak36.1ms.
- A failed wight-motion fixture was repaired by warming assets before timed pursuit; its original movement/sample assertions remain intact. The initial missing18 attachment failures were resolved by rebuilding after the complete96-entry include arrived. These failures and the final successful runs remain in local probe logs.
- Remaining art limits: two-contact gait stiffness, ambiguous side-view legs, diagonal torso/foot variation, modest source baseline drift, slightly shorter female NW hair/stronger build in one repaired contact, and abrupt idle attack recovery. Bow/staff-specific attack choreography is still absent. These are reviewable first-pass animations, not a claim of polished temporal consistency.

Retained captures, compact logs and test instructions: [production review](../../native/client/assets/raster/reviews/2026-09-10-lineage/README.md). The local review page is `http://127.0.0.1:8873/global-playback.html`; its source and GIFs are retained under the wave's `previews/` directory.
