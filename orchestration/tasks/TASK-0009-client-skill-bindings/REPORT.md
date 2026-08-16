---
task: TASK-0009
state: REVIEW_REQUESTED
branch: codex/TASK-0009-client-skill-bindings
commits:
  - 629a1c0
base_commit: 0c51439
---

## Executive summary

The native Win32 client now binds Q/E/R to the integrated core skill actions:
Q → Thrust, E → Sweep, and R → WarCry. The skill strip presents names and
resource costs, greys unaffordable/cooldown skills, and shows WarCry's active
buff state. The HUD now exposes life and resource bars, while Sweep and WarCry
render procedural area/aura effects from authoritative core events.

## Implementation

- Added client-side skill metadata and dispatch mapping without duplicating
  targeting, cooldown, or resource rules.
- Added full-circle Sweep arc and short-lived WarCry aura rendering.
- Added resource/life HUD bars and skill state text.
- Preserved D-007 movement, mouse aim, pickup/filter/interact/equip controls and
  the accepted +x Thrust facing limitation.

## Changed files

- `native/client/main.cpp` only.

## Interfaces

- Client key mapping requests `ActionType::Thrust`, `Sweep`, and `WarCry` via
  the existing `Command::action_use` seam.
- Client consumes `AttackStarted`, `BuffApplied`, and `BuffExpired` events for
  presentation; simulation remains authoritative.

## Verification

- `powershell -NoProfile -File native/build.ps1 -RunTests -RunClient` — PASS
  (denylist, core tests, and headless client shell).
- Live Win32 `PostMessage` pass — PASS: moved into range, Q caused Thrust
  damage, E caused Sweep attack/damage, R caused WarCry, HUD showed `29/50`
  after spending and `49/50` after regeneration, and captures showed Sweep and
  WarCry rings.
- `git diff --check` — PASS.
- Worker worktree clean; temporary probes/captures were not committed.

Independent validator `/root/validate_task_0009` returned **ACCEPT**. It
confirmed the exact one-file scope, authoritative `Command::action_use` and
event/HUD path, no client-side combat duplication, no disabled-slot hint, D-007
preservation, and passing denylist self-test. Its shell could not expose a
desktop window handle for a second live capture, so the PostMessage evidence in
the worker packet is supplemented by source-level inspection.

## Specification deviations

None reported. The core was not changed.

## Risks and limitations

- The client uses the fixed resource-cost metadata accepted by TASK-0007; a
  future public cost schema would need a deliberate interface task.
- The accepted core Thrust facing proxy remains +x-only by architectural
  decision; this task does not attempt to correct it.
- Presentation costs duplicate the current core constants and could drift if a
  future core interface changes them; this is a non-blocking validator risk.

## Questions for Fable or the owner

None.

## Integration notes

Cherry-pick `629a1c0` after independent validation. Then rerun the native full
gate and the driven client pass on the integration tip before architect review.
