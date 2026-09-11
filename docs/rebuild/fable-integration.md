# Fable reference integration — 2026-09-10

Owner request: implement the shared Art Direction Grill discussion and close
the visible/playable gap with the supplied Fable demo. This resumes the paused
work. New imagegen assets, if needed, must be surfaced for owner review.
The owner identifies all original demo art as ChatGPT Image-2 output.

## Reference and current implementation

The exact supplied archive is retained under `docs/reference/fable-demo/`.
It includes a self-contained playable build, 27 PNG assets, source, build
script, handoff, projection/shader equations and known failure remedies.
The demo art remains a comparison reference. Production begins with existing
accepted runtime art. The subsequent owner-supplied male/female references now
provide the playable actor set described in [the lineage wave](hero-lineage-20260910.md).

The starting client used Win32/GDI/GDI+ for the world and HUD, with a uniform
orthographic camera. Its earlier GPU modules were CPU proofs. This change adds
the actual hardware D3D11 world pass to normal remote play. Authoritative grid
positions still convert at107.25 native world units per tile. HUD and inventory
continue through ui_skin.hpp; simulation remains fixed-step and headless.

## Concrete implementation

`native/client/fable_camera.hpp` contains the reference projection, inverse
picking and one scene-fixed height field. The X/Y/W equations match the demo.
D3D clip Z is affine in depth so triangles crossing the near plane clip
correctly. `fable_gpu.hpp` owns hardware D3D11 resources and rendering;
`fable_world.hpp` assembles the native scene. Normal remote play selects this
path; F8 switches to the retained legacy renderer. The existing camera2d
contract and legacy fixtures remain available.

Terrain uses a2048² map texture, mipmaps and anisotropic sampling over a176×140
mesh spanning80×64 native tiles. A20-tile X/16-tile Y rebase preserves both
texel and vertex alignment. Same-scene movement starts one CPU-only worker;
frames retain the current patch until the new matching mesh/texture is ready.
Route changes and first load can wait. Immutable worker inputs, one future,
main-thread GPU upload, source/scene retirement and retained recovery pixels
bound the lifetime and memory. Elevation is flat indoors and restrained
outdoors. Terrain, props, actors and mouse picking share this field.

Existing authored pixel poses and equipment use nearest sampling. Extended
weapons have a padded union canvas with a stable body scale and foot pivot.
Actors shrink into the distance. Ground contact shadows and settled remains
sit below standing actors. Blocked native cells produce wall caps/faces with
player cutaways. Native drops, impact flashes, dust and damage numbers stay
event driven. Four screen-space cloud fields, the reference90-second ambient
cycle, projected warm lights, continuous focus blur, fog and vignette tie the
world together. BGRA readback into the existing HUD is included in performance
measurements. Hardware failure is explicit and returns to the legacy view.

## Playability changes found during integration

- Local display interpolation follows accepted player movement while retaining
  authoritative coordinates for collision and commands. Mouse attacks submit
  fresh projected aim before the skill. A short key tap survives between50ms
  ticks; held left mouse repeats the primary skill at250ms and focus loss
  releases held input.
  Immediate attack presentation uses the submitted aim while waiting for
  the echoed facing; the real server fixture covers an opposite-facing click.
- Space requests an authoritative collision-checked dash: ten ordinary
  normalized steps, approximately3.33 tiles, provisional500ms lockout. No
  invulnerability or resource cost was added. Accepted travel emits segment
  dust; rejected travel produces no fake movement effect.
- Actual ground-item events replace synthetic kill drops. Pickup retains the
  selected UUID; extraction validates life, instance and distance to upstairs.
- Confirmed contact produces camera shake and one55ms presentation-only pose
  hold. Simultaneous contacts share a hold; authority, input and network polling
  keep advancing.
- New Scions arrive at38,116 beside the town fountain instead of a distant
  House plot. Services and their visible stalls cluster around that square.
  House plots and progression remain intact. Indoor routes no longer inherit
  village trees by route-number accident; wall-side braziers and pottery use
  existing art. Minimap markers clip to their panel.

## Acceptance

The supported native build runs pure projection tests, core/network/session/
presentation/audio suites and all client scenarios. `fable-world` captures
legacy/new native scenes, measures20 full3440×1440 frames and then crosses a
terrain rebase while rendering. Travel must reuse the resident patch, adopt
the new one without a loading wait, stay below40ms average and retain less
than48MiB of CPU terrain data. `quick-movement-tap` covers down/up before a
tick, held-input sampling and pane cancellation. The independent GPU probe
checks actual pixels, clipping, sampling, lights, clouds, ordering, resource
bounds and recovery. Browser playtest remains a separate real-server check.

Exact observed results and manual-play scope belong in the retained review
and HANDOFF. A still frame does not establish completed combat or extraction.

## Scope still visible

This is a native integration of the reference's spatial model and rendering
passes, not a claim of complete art or UI parity. The existing charcoal/gold
HUD remains; the light-stone UI direction still needs an art/layout pass.
Monsters retain their existing directional animation coverage. The new male
and female actors each have eight directions, two walking contacts and three
attack frames (including idle recovery). Ground repetition, wall cutaways,
scene composition and consistency between the new actors and older scenery
remain areas for visual iteration. Generated character previews and the
playable selection screen are surfaced for the owner's review.
