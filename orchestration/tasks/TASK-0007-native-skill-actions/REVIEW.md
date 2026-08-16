---
task: TASK-0007
verdict: ACCEPTED
reviewed_commits:
  - e7505ad
---

## What was reviewed

Full diff `74e58a0..e7505ad` (core.hpp, core.cpp, tests) and an independent
`build.ps1 -RunTests` rerun in the worker worktree (denylist PASS, all
tests PASS).

## What is correct

- The `resolve_actor_action` refactor is the right architectural move:
  actions now run for any actor kind against the opposing kind — D-003
  actor symmetry is structural, not aspirational. Monsters can be given
  skills later without another refactor.
- Every number is a named constant; all math is integer (no float
  determinism risk). Buff state lives on the Actor; regen, cooldown, and
  expiry are handled in `advance_tick` with a correct expiry-edge event.
- `record_equipped_item_use` correctly guards player-kind, preserving
  baseline behavior for enemy attacks.
- `spawn_monster` is a genuinely general seam (production `spawn_enemy`
  now uses it) rather than test-only scaffolding — exactly what the spec
  asked for.
- ActionType values appended, existing values stable — recorded command
  streams stay valid.
- Sweep's in-loop `handle_death` is safe: nothing in the death path
  reallocates `actors_`.
- The WarCry-shares-no-cooldown deviation is correctly reasoned from the
  spec text and documented.

## Problems

None blocking.

1. (Observation) Thrust's +x-only facing proxy means a monster behind the
   player is unhittable by Thrust. Documented honestly in code; a real
   facing field is future client/core work — when TASK-0009 binds skills,
   the client's mouse-aim cannot yet influence Thrust direction. Carry
   this into the eventual facing task; not a correction here.

## Required corrections

None.

## Architectural effect

The resource stat is now live gameplay; Q/E/R can bind (TASK-0009
promotes once this integrates). Integration approved.
