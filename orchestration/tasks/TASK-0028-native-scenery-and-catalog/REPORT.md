---
task: TASK-0028
state: REVIEW_REQUESTED
branch: codex/TASK-0028-native-scenery-and-catalog
commits:
  - 53d2b06c58f8eb44dedd935ff068a8878989764c
  - b6c13c1518a99731e0d3c3106356f8b1db41f6a3
  - b90e6984600cb148706941a9566b597837b521ea
base_commit: e7a7a78b4521b29f84e985242dde70b0fa492e00
---

## Executive summary

The native client now renders deterministic route-seeded `tree`, `ruin`,
`dwelling`, and `shrine` scenery from the keyed plate loader with grounded
billboards, contact shadows, depth sorting, fallback shapes, and simple circle
colliders. Movement and dash use swept segment-vs-expanded-circle checks.
Skill costs and telegraph ranges read the core `PresentationCatalog`; client
mirrors were removed.

## Implementation

- Added FNV/splitmix route-seeded scenery placement and keyed plate slots.
- Added scenery depth ordering alongside actors and grounded contact shadows.
- Added scenery collision for normal movement and 10-tick dashes.
- Kept assets read-only and degraded to fallback capsules/shapes when missing.
- Replaced client-side skill-cost and telegraph-range literals with catalog reads.

## Changed files

- `native/client/main.cpp`
- `orchestration/tasks/TASK-0028-native-scenery-and-catalog/captures/*`

The implementation commits do not touch Simulation/core, build files, browser
paths, or vendored source assets.

## Interfaces

No public core interfaces changed. The client consumes the existing
`Simulation::presentation_catalog()` and existing route/actor snapshots only.

## Verification

- `powershell -NoProfile -File native/build.ps1 -RunTests -RunClient` — PASS
  (denylist, core tests, headless client, and native client build).
- `git diff --check` — PASS.
- Driven Win32 `PostMessage`/`PrintWindow` pass — PASS.
- Capture set is lossy 960×600 JPEGs, each approximately 100–106 KB.
- Grep proof from the implementation worktree:
  `git grep -n -E 'kThrustResourceCost|kSweepResourceCost|kWarCryResourceCost|kThrustRange|kSweepRange|kWarCryRange' -- native/client/main.cpp`
  returned no matches.

## Manual checks

The driven evidence captures scenery, behind/front tree depth traversal at
approximately `(154,-165)` and `(154,121)` around the fixed tree at
`(260,-100)`, dwelling collision blocking, and a rejected dash with the
`Dash blocked by scenery` hint. The skill strip shows Q/E/R costs `10/15/20`.

## Specification deviations

None. The first implementation used destination-only dash collision and an
insufficient depth route; both were corrected in the two revision commits.

## Risks and limitations

The scenery catalog is presentation-only and intentionally does not persist
layout state beyond the deterministic route seed. Full-resolution asset
provenance remains the owner decision recorded elsewhere; missing plates use
fallback geometry.

## Questions for Fable or the owner

None for this task. Architectural review is still required before integration.

## Integration notes

Integrate `53d2b06c`, `b6c13c15`, and `b90e6984` together, preserving commit
provenance. The task depends on integrated TASK-0015 and TASK-0016 and should
remain ahead of any client task that assumes scenery or catalog adoption.
