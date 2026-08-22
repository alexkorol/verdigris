---
task: TASK-0142
title: Native client owner-facing presentation slice
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
dependencies: [TASK-0141 ACCEPTED]
base_commit: d0f74af3d30f238479218f8be412a01d61e21df3
owned_paths: [native/client/main.cpp, native/client/render_list.hpp, orchestration/tasks/TASK-0142-native-client-presentation-slice/**]
forbidden_paths: [native/client/assets/**, native/src/**, native/include/**, native/tests/**, server/**, src/**, playtest/**, .github/**, CI or machine mutation]
promotion_provenance:
  generator: codex-pc-architect
  parent_packet: presentation gap in PROGRAM_GRAPH T5
  dependency_event: TASK-0141 vector kit contract is required before claim
  validator: single-writer native client packet; collision clear at d0f74af3
---

# Outcome

Make the native Windows client visibly owner-facing instead of a debug shell
or fallback-only wireframe. Consume the TASK-0141 data-only vector kit without
changing its files. Make asset-root discovery reliable from the repository
root, build directory, and installed-style executable directory; retain a
deterministic vector fallback when PNG/GDI+ assets are unavailable. Improve the
floor/scenery/actor silhouettes, extraction marker, hit feedback, HUD hierarchy,
and status copy enough that a first expedition reads as a game at a glance.

Keep simulation authority in the existing core and preserve the recorded render
list. Add or update client scenarios that assert the visible operations and
that the owner-facing path reaches combat, loot, equip, and extraction. Do not
hide missing assets by claiming they loaded; expose an honest status line.

# Acceptance commands

From repository root, record literal output and exit codes in REPORT:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
native/build/verdigris_client.exe --scenario first-fight
git diff --check
git diff --name-only d0f74af3d30f238479218f8be412a01d61e21df3..HEAD
```

The real scenario harness must exit 0, and the diff must remain inside the
owned client/task paths. No browser or server fallback counts as acceptance.

# Stop conditions

STOP before editing the simulation, server, browser, CI, asset-kit files, or
claiming a screenshot/visual improvement without running the real scenario
harness. Do not add a renderer dependency or use port 6500.
