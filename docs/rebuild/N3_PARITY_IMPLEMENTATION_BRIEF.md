# Native N3 parity implementation brief

Coordinator read-only derivation: 2026-08-17. This is a worker handoff aid,
not a task claim or a product decision. It is derived from the unchanged
browser server, `playtest/harness.mjs`, the native deterministic core, and the
N3 negative baseline.

## Required inbound wire contract

| Wire event | Payload shape used by the reference | Native core translation |
|---|---|---|
| `player:move` | `{ id, direction, stopped? }` | `Command::move(dx, dy)`; when the step is occupied by a living monster, resolve the primary melee action instead of advancing |
| `player:skill:trigger` | `{ id, skillId, direction, issuedAt, modifiers, phase }` | `Command::aim(dx, dy)` followed by `Command::action_use(ActionType)`; map `primary-attack`, `dash`, `ability-1/2/3` to the existing action/catalog rules |
| `dev:give` | `{ itemId, qty }` | Development-only inventory grant; must not become a production gameplay shortcut |
| `dev:state` | `{ requestId }` | Snapshot projection containing player, live monsters, effects, combat state, ground items, and recent event-derived state |

The reference treats movement into an adjacent monster as combat input. Skill
input is server-authoritative: cooldown/resource gates, hit detection, damage,
XP, auto-attack continuation, and loot all resolve before the broadcast.

## Required outbound wire contract

The harness and browser consume these authoritative events during N3:

- `player:combat:update` — player combat/cooldown/buff/animation state.
- `combat:hit` — attacker/target IDs and names, skill/style, amount, health,
  critical/beastbane fields, `died`, and optional experience.
- `world:skill:effect` — source, skill, direction, origin/destination, radius,
  duration, healing, and armour bonus.
- `world:projectile` — projectile path, travel time, skill, and blocked state.
- `player:stats:update` — resource/life changes and progression state.
- `monster:telegraph` — elite warning/action and remaining windup.
- `game:send:message` — kill, resource, or progression feedback.
- Scene/monster/item updates — live monster health/roles/effects, dead-monster
  removal, ground loot, and inventory refresh after pickup.

## Existing core hooks

The native core already exposes the needed deterministic primitives:

- `ActionType::{Melee, Dash, Thrust, Sweep, WarCry}`
- `Command::{move, aim, action_use, pick_up, equip}`
- `EventType::{AttackStarted, DamageApplied, ActorDied, ItemDropped,
  TrophyDropped, ItemPickedUp, BuffApplied, AttackTelegraphed, ...}`
- `Simulation::actors()`, `ground_items()`, `ground_trophies()`, `events()`,
  and `legends()` for snapshot/event projection.

The transport must translate and serialize these hooks; it must not reimplement
combat, encounter, cooldown, loot, or progression rules in
`native/src/networking.cpp`.

## Current N2 boundary observed at `d476788`

The review-requested N2 implementation has also been replayed on the current
coordinator tip as disposable candidate `f602dab4` (base `27db1611`). Its
native denylist/core/networking/client gate and unchanged N2 attach matrix
(`quickstart`, `single-session`, `movement`, `zones`) pass 4/4. This proves
the N2 handoff applies cleanly to the current program tip without claiming
architect acceptance or integration; the exact transcript is preserved in
[`coordinator-current-tip-n2-candidate-2026-08-17.txt`](../../orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-current-tip-n2-candidate-2026-08-17.txt).

This is an implementation constraint, not a product decision:

- `ProtocolSession` currently owns both a deterministic `Simulation` and a
  separate `WorldSimulation`. The session snapshot reads player health and
  inventory from the former, but live monster positions/roster and scene
  metadata from the latter.
- `ProtocolSession::handle()` currently maps movement to
  `WorldSimulation::apply_movement_sample()` and has no
  `player:skill:trigger` branch. Movement therefore does not yet dispatch the
  deterministic combat simulation.
- The native `Simulation` already has shared actor actions, damage/death
  resolution, item/trophy drops, and event emission, while the N2 world adapter
  still exposes a minimum authored roster and empty ground-item arrays in its
  protocol snapshot.
- The N2 protocol currently generates 18 shallow world monsters (all
  `<theme>-lurker` records) and has no world-monster-to-`Actor` UUID/position
  mapping. It also does not handle `player:skill:trigger`, pickup, or combat
  event projection; `dev:give` writes a separate session inventory rather than
  the simulation's carried-item state.
- The reusable event stream is not yet rich enough for a lossless browser
  projection: `DamageApplied` carries a target but no source/skill/style, and
  `ActorDied` carries no killer. Ground `Item`/`Trophy` records also lack floor
  coordinates, and the core has no public time-advance seam independent of
  command dispatch for polling-driven aura/AI behavior. These are concrete
  core/API requirements to resolve in the N3 spec, not reasons to infer
  protocol-side gameplay.

N3 should establish one authoritative bridge between these existing seams (or
deliberately move the world roster into the core) and project its events at
the protocol edge. A networking-only combat implementation would create two
authorities and is explicitly out of bounds under D-002.

The line-oriented reconnaissance is preserved in
[`coordinator-n3-core-gap-audit-2026-08-17.txt`](../../orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-n3-core-gap-audit-2026-08-17.txt).

## Acceptance matrix for N3

1. Existing native denylist/core/networking/client gates remain green.
2. `combat` passes against native: at least 20 authored monsters, a reachable
   non-elite target, real kill within the existing 30-second bound, survival,
   and a `combat:hit` death event.
3. `encounter-variety` passes against native: crypt melee/buffer composition,
   marsh ranged pressure, one rare modifier, live aura effect, and kill within
   the existing 15-second bound.
4. N2 matrix (`quickstart`, `single-session`, `movement`, `zones`) remains
   green after N3 changes.
5. A JS/native dual-run transcript records scenario results and any deliberate
   remaining N4+ stub; no assertion may be weakened to hide divergence.
