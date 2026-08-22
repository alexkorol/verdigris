---
task: TASK-0144
title: Native visual-kit C++ literal portability correction
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
dependencies: [TASK-0141 ACCEPTED, TASK-0142 ACCEPTED]
base_commit: c0b79e5bdc4017507f0cb833b293480ee3f8140e
owned_paths: [native/client/assets/**, native/client/main.cpp, orchestration/tasks/TASK-0144-native-visual-kit-msvc-portability/**]
forbidden_paths: [native/src/**, native/include/**, native/tests/**, server/**, src/**, playtest/**, .github/**, CI, final owner art]
promotion_provenance:
  generator: codex-pc-architect
  parent_packet: TASK-0141 accepted visual kit plus TASK-0142 MSVC consumer shim
  dependency_event: generated data header emits non-conforming integer-float tokens such as 1f
  validator: generator check plus native client build; remove shim only after compile proof
---

# Outcome

Make the accepted native visual-kit contract compile portably without a
consumer workaround. Update the TASK-0141 generator's deterministic float
serializer so every emitted C++ literal is standards-conforming (`0.f`,
`1.f`, or an equivalent explicit decimal form), regenerate
`native/client/assets/generated/visual_kit.h`, and remove the temporary
reserved-suffix literal shim from `native/client/main.cpp` introduced by
TASK-0142. Preserve byte-stable generation, SVG/manifest output, role and
shape counts, and all existing owner-facing behavior.

## Acceptance commands

From repository root, record literal output and exit codes in REPORT:

```powershell
node --test orchestration/tasks/TASK-0141-procedural-native-visual-kit/asset-kit.test.mjs
node orchestration/tasks/TASK-0141-procedural-native-visual-kit/generate-assets.mjs --check
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
git diff --check
```

The generated header must contain no `\b[0-9]+f\b` tokens, the client must
compile without a reserved literal shim, all scenario gates must remain green,
and only the owned paths may change. Do not change simulation behavior or
owner-approved art.

## Stop conditions

Stop and report BLOCKED if the generator cannot preserve deterministic bytes,
if removing the shim changes rendered behavior, or if any forbidden path would
be required.
