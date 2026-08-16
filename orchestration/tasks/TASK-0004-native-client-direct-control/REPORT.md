---
task: TASK-0004
state: REVIEW_REQUESTED
branch: codex/TASK-0004-native-client-direct-control
commits:
  - 7ac51d4
base_commit: bc73ce0
---

# TASK-0004 report — native client direct control

## Implementation

Commit `7ac51d4` updates only `native/client/main.cpp` to implement the D-007
control contract. WASD/mouse aim, LMB melee, RMB/Space dash, disabled Q/E/R
skill slots, deterministic nearest-drop X pickup, Z loot labels, contextual F
extraction, and the I gear/House overlay are wired through existing simulation
commands. Equip uses the simplest deterministic UX: Up/Down selects a carried
item and Enter or LMB equips it while the overlay is open. The old P pickup, E
equip, and X extraction bindings are removed; the `--headless` path is
unchanged.

## Verification

Worker verification passed:

```text
powershell -NoProfile -File native/build.ps1 -RunTests -RunClient
Native denylist: PASS
Core tests: PASS
Headless output:
Verdigris native client shell
House: House Verdigris | trophies stored: 1 | items stored: 1
```

The driven Win32 PostMessage/PrintWindow pass also passed: Q hint and disabled
strip, Z labels, WASD/LMB combat and drops, X nearest trophy/item pickup, I
overlay, Enter equip, movement to the pad, and F extraction with core event log
and House-store updates. Captures are retained untracked under
`native/client/direct-control-captures/` in the worker worktree as evidence.

The historical `native/README.md` still documents the old controls; it is
outside this task's owned paths and is noted for a later owner/coordinator doc
update.

Independent validator `/root/validate_task_0004` returned **ACCEPT** after
reviewing commit `7ac51d4`, rerunning the full native gate, checking
`git diff --check`, confirming the D-007 command semantics and unchanged
headless path, and inspecting the driven-input captures. Architect review and
integration follow.

## Changed files

- `native/client/main.cpp` only.
- Driven-input captures remain untracked under
  `native/client/direct-control-captures/` and are evidence artifacts, not
  implementation changes.

## Interfaces

No simulation interfaces or commands were added. The client calls existing
`pick_up`, `equip`, `extract`, `action_use(Melee)`, and
`action_use(Dash)` commands.

## Manual checks

The Win32 harness exercised Q, Z, WASD/LMB combat, X pickup ordering, I/Enter
equip, movement to the extraction pad, and F extraction, with PrintWindow
captures and event/store assertions.

## Specification deviations

None. The legacy README still has stale controls, but it is outside the task's
owned paths and is explicitly deferred to the owner/coordinator documentation
pass.

## Risks and limitations

The captures are local untracked evidence and will not travel in the commit.
The client remains dependent on the existing native build guard while
TASK-0002 review is finalized.

## Questions for Fable or the owner

None.

## Integration notes

Integrated `7ac51d4` as `6396a0e`; the architect review is `ACCEPTED`. It is
client-only and follows the accepted build/tooling foundation.
