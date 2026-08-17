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

