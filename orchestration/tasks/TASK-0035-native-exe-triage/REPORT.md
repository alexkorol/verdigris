---
task: TASK-0035
state: REVIEW_REQUESTED
branch: codex/TASK-0035-native-exe-triage
commits:
  - 809de7bb
  - e562ad1e
base_commit: b141cd9f
---

## Executive summary

The native executable is now honestly presented as a core testbed rather
than a debug-stuffed game window. Combat and world scale are derived from
the current 220-units/second movement cadence, the default view contains only
compact life/resource bars and the three-slot skill strip, and all diagnostics
are behind F3. The title is `Verdigris Core Testbed`.

## Implementation

### D-114 scale table

| Measure | Derivation | Result |
|---|---|---:|
| Player walk | 220 units/s × 50ms | 11 units/tick |
| Melee contact | 13 walk ticks | 143 units / 0.65s |
| Thrust contact | melee × 1.5 | 214 units / 0.97s |
| Extraction interaction | 8 walk ticks | 88 units / 0.40s |
| Enemy spawn | melee × 5 | 715 units / 3.25s |
| Arena half-extent | melee × 6 | 858 units / 3.90s |
| Actor collider | melee / 5 | 28 units |
| Scenery collider | melee / 2 | 71 units |

The same table drives native combat constants, extraction range, spawn
distance, arena bounds, and scenery/actor collision envelopes.

## Changed files

- `native/include/verdigris/core.hpp`
- `native/src/core.cpp`
- `native/client/main.cpp`
- `native/tests/core_tests.cpp`
- `native/client/triage-captures/**`

## Interfaces

No client protocol or simulation event shape changed. The scale values remain
native constants consumed by the deterministic core and client lab.

## Verification

- `powershell -File native/build.ps1 -RunTests -RunClient` — passed.
- Native denylist — passed.
- Core tests — passed.
- Headless output — unchanged/passed.
- `git diff --check b141cd9f..e562ad1e` — passed.
- Scope is limited to the four permitted native paths.

## Manual checks

Committed captures:

- `native/client/triage-captures/default-clean.png` — default sparse view.
- `native/client/triage-captures/adjacent-before-strike.png` and
  `adjacent-after-strike.png` — player at `(627,0)` against a monster at the
  715-unit spawn coordinate; life changed from 78 to 67 after adjacent melee.
- `native/client/triage-captures/debug-f3.png` — diagnostics visible only
  after F3.

## Specification deviations

None. This task deliberately does not add inventory, orbs, menus, or other
game UI; those belong to the browser product under D-112.

## Risks and limitations

The capture driver was temporary and removed; the evidence is retained as
PNG files. TASK-0039 remains sequenced because it overlaps the native core
paths until this task is accepted/integrated. The independent validator
accepted the task and noted one review caveat: the stills prove adjacent
contact/damage but do not show continuous enemy-closing motion; the architect
may request that additional driven capture under the D-115 play gate.

## Integration notes

Requires architect review before integration. Integrate `809de7bb` and
`e562ad1e` together after acceptance.


