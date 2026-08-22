---
task: TASK-0141
title: Procedural native visual kit and vector asset source
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
dependencies: []
base_commit: d0f74af3d30f238479218f8be412a01d61e21df3
owned_paths: [native/client/assets/**, orchestration/tasks/TASK-0141-procedural-native-visual-kit/**]
forbidden_paths: [native/client/main.cpp, native/src/**, native/include/**, native/tests/**, server/**, src/**, playtest/**, .github/**, CI or machine mutation]
promotion_provenance:
  generator: codex-pc-architect
  parent_packet: presentation gap in PROGRAM_GRAPH T5
  dependency_event: current native client has an asset-loader seam but no native-local asset kit
  validator: task-folder plus asset-folder only; collision clear at d0f74af3
---

# Outcome

Create a small, dependency-free, deterministic vector/procedural art kit for
the native client. Produce SVG sources and a generated C++ data header under
`native/client/assets/` for the player, raider, elite, tree, ruin, dwelling,
shrine, and two terrain motifs. The art should be deliberately stylized and
legible at the current camera scale: strong silhouettes, readable team/enemy
contrast, transparent/background-safe shapes, and enough variation to stop the
owner-facing scene reading as capsules on an empty grid.

Include a manifest naming each role, source SVG, generated symbol, palette, and
generator version. Use only the standard Node runtime and deterministic math;
do not download packages, embed third-party art, or make product/content
claims. The C++ header is data only: no Win32 calls, simulation state, or
renderer ownership. TASK-0142 will consume this interface.

# Acceptance commands

From repository root, record literal output and exit codes in REPORT:

```powershell
node --test orchestration/tasks/TASK-0141-procedural-native-visual-kit/asset-kit.test.mjs
node orchestration/tasks/TASK-0141-procedural-native-visual-kit/generate-assets.mjs --check
git diff --check
git diff --name-only d0f74af3d30f238479218f8be412a01d61e21df3..HEAD
```

The tests must prove deterministic regeneration, all eight roles, valid SVG
roots, generated-header symbol coverage, and absence of port 6500 or external
network/package references. No native build is required because this packet
does not edit client code.

# Stop conditions

STOP before editing `native/client/main.cpp`, simulation/source code, CI,
packaging, or owner-approved production art; do not contact external services
or claim that this placeholder kit is final art.
