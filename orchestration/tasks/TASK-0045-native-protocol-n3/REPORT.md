# TASK-0045 — Parity wave N3 report

## Executive summary

Implemented the smallest deterministic native combat slice required by the
unchanged N3 scenarios. Monsters now expose combat HP, damage, death, roles,
rare modifiers, aura state, and the named Old Barrow boss telegraph contract.
The transport emits the existing `combat:hit` and `monster:telegraph` event
shapes and extends `dev:state` without changing the harness. The server keeps
the loopback bind at `127.0.0.1` and treats stdin EOF as an idle condition.

This is **REVIEW_REQUESTED / not architect-accepted**. No acceptance is
claimed here.

## Approach and reference parity

Rules remain in `WorldSimulation`; protocol translation remains in
`ProtocolSession`. Tile-space position from `WorldSimulation` is the sole
wire/combat position authority. The implementation was shaped against the
read-only browser references:

- `server/core/monster.js` — actor payload and lifecycle shape.
- `server/core/entities/monster/combat-controller.js` — hit payload and skill
  naming.
- `server/core/entities/monster/behaviours/melee.js` — named ground-slam
  telegraph/impact behavior.
- `server/core/entities/monster/behaviours/buffer.js` — aura/Empowered state.
- `server/core/map.js` — zone pack roles, rare modifier, and boss definition.

N3 constants are named in `native/src/core.cpp` with the D-114-derived combat
table comments: trash life 30, player damage/cadence 18/350 ms, boss life 120,
damage 12, telegraph radius 2, and window 1000 ms.

## Changed files

Native implementation and tests only in commit
`d9c6fc6fc95a95dfad61a7c7890c38573a66c20d`:

- `native/include/verdigris/core.hpp`
- `native/include/verdigris/networking.hpp`
- `native/src/core.cpp`
- `native/src/networking.cpp`
- `native/src/server_main.cpp`
- `native/tests/networking_tests.cpp`

Evidence captures added with this report:

- `captures/native-build-tests-2026-08-17.txt`
- `captures/attach-7-of-7-2026-08-17.txt`
- `captures/authentic-negative-telegraph-radius-2026-08-17.txt`

## Public interfaces and wire behavior

- `WorldMonster` carries behavior role, rarity, modifiers, Empowered state,
  boss state, HP, and telegraph timing.
- `WorldSimulation::start_player_attack` and `advance_combat` provide the
  deterministic combat seam.
- `dev:setlevel` and `dev:heal` update the authoritative simulation actor.
- `player:skill:trigger` starts nearest/encounter-authored combat.
- `combat:hit` carries attacker/target IDs, skill, amount, HP, and `died`.
- `monster:telegraph` carries attacker, skill, center, radius, and
  `durationMs`.
- `dev:state.monsters[]` carries behavior, rarity, modifiers, HP, and aura
  effects; minimum drop payloads appear in `groundItems`.

## Verification

1. `powershell -NoProfile -File native/build.ps1 -RunTests` — denylist,
   core tests, and networking tests all PASS. See the literal transcript in
   `captures/native-build-tests-2026-08-17.txt`.
2. Rebuilt native server attach command, unchanged harness — all 7 scenarios
   PASS: `combat`, `encounter-variety`, `boss-mechanic`, `quickstart`,
   `single-session`, `movement`, `zones`. Harness commit:
   `babba96fcde5ca1d55610853ad59af4fd57e2374`. No playtest files were
   changed.
   See `captures/attach-7-of-7-2026-08-17.txt`.
3. C++ assertions added in `native/tests/networking_tests.cpp` cover: marsh
   population, one rare named modifier, Empowered aura state, named Warden,
   boss telegraph radius/window, and the emitted telegraph skill ID.
4. Authentic negative: radius 2 was temporarily changed to 1; the networking
   test failed its readable telegraph assertion; the constant was restored and
   the final build/tests returned green. See
   `captures/authentic-negative-telegraph-radius-2026-08-17.txt`.

## Stubs, deviations, and follow-ups

- Ground drops intentionally expose only the minimum item-shaped payload used
  by the N3 combat slice; full item identity, pickup, and inventory rules are
  N4 successors.
- Aura behavior is a deterministic presentation/state marker and damage
  buffer; broader effect stacking and persistence are later combat/item
  successors.
- The native protocol has no independent AI timer yet. Fixed-step combat is
  advanced at protocol command/state boundaries, which is sufficient for this
  bounded attach slice; a future server tick owns continuous AI scheduling.
- No authority/product questions were invented or resolved by this task.

## Risks and architect handoff

The architect should rebuild the branch and personally rerun the attach set,
per the task spec, then decide ACCEPTED/REVISE. In particular, review the
owner-felt boss timing and the intentionally bounded command-boundary combat
heartbeat before acceptance.
