# TASK-0170 report — native menu and Escape-state model

## References

- `FLEET_HANDOFF.md` Owner Demo non-negotiable: Escape opens/closes menus, never exits
- `native/client/input_focus.hpp` (TASK-0165) — pane-close within gameplay; complementary seam
- `native/client/main.cpp` `handle_escape_key` — current production still quits on bare Esc (integration deferred to TASK-0183)

## Deliverable

Pure header `native/client/menu_scene.hpp` encoding title → playing → paused flow:

- **Title:** Escape consumed (no quit); Start → Playing; explicit Quit → RequestQuit
- **Playing:** Escape/Cancel → Paused (Main pane); gameplay intents PassToGameplay; Escape never RequestQuit
- **Paused:** Resume or Escape on Main → Playing; nested Settings/Help via Open*; Escape pops stack; explicit Quit opens ConfirmQuit then RequestQuit on Confirm

Task-local harness: 69 checks, MSVC `/W4` clean.

## Commands and exit codes

```text
powershell -NoProfile -ExecutionPolicy Bypass -File orchestration/tasks/TASK-0170-native-menu-scene-model/run-tests.ps1
  -> exit 0; "69 checks passed"; harness PASS

python native/tools/check_legacy_denylist.py
  -> exit 0; PASS

git diff --check
  -> exit 0, silent

git diff --name-only (after commit)
  -> empty
```

## Files

| Path | Role |
|---|---|
| `native/client/menu_scene.hpp` | Production reducer |
| `orchestration/tasks/TASK-0170-native-menu-scene-model/menu_scene_tests.cpp` | Acceptance tests |
| `orchestration/tasks/TASK-0170-native-menu-scene-model/run-tests.ps1` | MSVC harness |
| `orchestration/tasks/TASK-0170-native-menu-scene-model/STATUS.md` | Claim |
| `orchestration/tasks/TASK-0170-native-menu-scene-model/REPORT.md` | This file |

## Evidence

No runtime screenshot — pure model packet per SPEC forbidden_paths (`main.cpp` untouched). Negative control: global sweep proves Escape/Cancel never yield RequestQuit in any modeled state.

## Residual gaps

- Not wired into `main.cpp`; Owner journey still exits on bare Esc until TASK-0183 splash/menu integration
- Gear overlay during play is not modeled here (owned by `input_focus.hpp`); integrator must compose both reducers

## Successors

- TASK-0183 native splash/menu integration (depends 0170 + 0179 + 0180)
