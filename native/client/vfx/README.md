# Native sprite effects

`particle_system.hpp` owns a fixed 50 ms presentation simulation, capped at 32 emitters and 384 live particles. Gameplay remains authoritative in the server. `game_effects.hpp` maps confirmed events and locomotion samples onto it. `fable_world.hpp` submits point-sampled atlas quads to the existing perspective renderer; contiguous compatible atlas draws are batched without changing alpha order. Two optional effect lights maximum. There is no additional fullscreen pass.

The external definitions in `../assets/effects` control burst/continuous spawning, local/world space, lifetime, velocity, acceleration, drag, spin, animation frames, color/alpha and size interpolation. The 64x64 atlas is stored as hand-authored alpha digits in JSON (0 transparent, 9 opaque), decoded once into one RGBA texture. These tiny geometric signals are original procedural assets, not extracted Nox artwork.

## Triggers

- `level_up`: server `player:level-up` event on a real XP threshold crossing. Four layers: vertical core, descending streaks, body burst, residual motes. Login/snapshot level changes never emit it.
- `melee_hit_small`: confirmed damage contact, including the local test path.
- `foot_dust`: 38 world units of confirmed locomotion; teleports over 100 units and scene changes reset sampling.
- `simple_death_puff`: confirmed actor death, retaining the prior position if the actor left the live roster.
- `bowl_ember_idle`: equipped bowl/censer/fire vessel name, attached to its equipped hand. Unequip, death and disconnect stop it. The current server catalog does not supply a playable fire-bowl item; the hook and captured equipped-item fixture are ready for that content.
- `burning_touch_contact`: a confirmed `burning-touch` skill contact selects this recipe. Current native actions do not yet expose that spell; the shipped scenario renders the recipe without inventing damage rules.
- `projectile_trail_simple`: reusable continuous world-space trail. `play` + `attach` move its emitter; `stop` ends emission while detached sparks expire. The current native ranged warning is a telegraph, not an authoritative flying projectile, so it is not misrepresented as one. The packaged scenario exercises and captures the trail.

## Anchors and temporary choices

Root/feet, head, main hand, off hand and explicit world points are supported. The actor adapter uses the existing facing direction with authored offsets: head 110 units high, hands 55 high and 18 forward/side. These are centralized approximations until the actor atlas exports per-frame socket data. `ProjectileCenter` can be supplied by an entity resolver; moving explicit world points are supported now. Equipped fire-vessel detection currently uses display names and should become an item presentation tag when those items ship.

All state is transient. No save format or inventory authority changes. Missing anchors discard attached particles; detached world particles finish normally. Scene changes/disconnect clear the transient system. Do not reload the asset map while live emitters hold recipe references.

## Verification entry

Run the exact client with `--scenario menu-particles` and a contained `VERDIGRIS_CAPTURE_ROOT`, then the complete `--scenario all` package gate. Captures call production paint, and the VFX scenario requires the actual perspective GPU atlas path. Networking tests earn level two through real kills; session tests exercise the explicit event over a real scripted socket and reject foreign/fractional events.
