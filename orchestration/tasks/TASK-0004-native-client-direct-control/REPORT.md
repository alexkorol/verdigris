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

Independent validation is pending; architect review and integration follow.
