---
task: TASK-0147
title: Procedural native visual polish wave
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
dependencies: [TASK-0141 ACCEPTED, TASK-0144 ACCEPTED]
base_commit: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2
owned_paths: [native/client/assets/**, orchestration/tasks/TASK-0141-procedural-native-visual-kit/generate-assets.mjs, orchestration/tasks/TASK-0141-procedural-native-visual-kit/asset-kit.test.mjs, orchestration/tasks/TASK-0147-procedural-native-visual-polish-wave/**]
forbidden_paths: [native/client/main.cpp, native/client/*.cpp, native/client/*.hpp, native/src/**, native/include/**, native/tests/**, server/**, src/**, playtest/**, .github/**, external downloads, final owner art]
---

# Outcome

Turn the accepted embedded vector kit from functional silhouettes into a
cohesive, attractive procedural placeholder set while preserving its public
symbol/role interface. Improve the existing player, raider, elite, tree,
ruin, dwelling, shrine, and two terrain motifs with readable silhouettes,
layering, material cues, and a restrained Verdigris palette that remains
legible at gameplay scale. The same generator must produce SVG references and
the C++ header deterministically; do not add downloaded or provenance-unknown
assets.

This is an art-direction pass, not bookkeeping. At least the player, raider,
elite, scenery family, and both terrain motifs must have materially richer
geometry than TASK-0141 while retaining stable role names and standards-
conforming C++ literals. Keep shapes bounded enough for the existing GDI
consumer and preserve deterministic bytes.

# Acceptance

```powershell
node --test orchestration/tasks/TASK-0141-procedural-native-visual-kit/asset-kit.test.mjs
node orchestration/tasks/TASK-0141-procedural-native-visual-kit/generate-assets.mjs --check
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
native/build/verdigris_client.exe --scenario first-fight
git diff --check
git diff --name-only 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2..HEAD
```

All commands exit 0. REPORT includes before/after SVG composites and a fresh
native default-resolution capture. Tests pin role names, bounds, determinism,
non-conforming-literal absence, and a meaningful geometry increase without
making exact artistic coordinates the public API.

# Stop conditions

Stop before changing the client consumer, simulation, content canon, or final
owner art. Do not introduce fonts, binaries, network dependencies, random
generation, or per-run timestamps.
