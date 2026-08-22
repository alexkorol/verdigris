---
task: TASK-0114
title: Stage-2 renderer backend evaluation matrix
state: READY
packet: BOUNDED-DESIGN
topology: EXPLORATORY
job: BOUNDED-DESIGN
priority: P2
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
supersedes: TASK-0073
owner_visible_contribution: narrows the cross-platform renderer choice while preserving deterministic captures and panel delivery
dependencies: []
owner_input_dependency: owner approves any production dependency after ADR; evaluation is not blocked
owned_paths: [orchestration/tasks/TASK-0114-renderer-backend-evaluation/**]
forbidden_paths: [everything else]
resource_capsule: research-only; internet citations allowed; no downloads, builds, dependencies, or ports
---

# Outcome and frozen invariants

Produce `EVALUATION.md` and `captures/source-index.json` comparing Direct3D
11, OpenGL 3.3 core, SDL2 plus an explicit batching layer, sokol_gfx, and
optimized GDI as the null option. Cover Windows/macOS viability, sprites,
atlases, shaders, text, offscreen capture, resource lifetime, plain MSVC/CMake
integration, licenses, binary/dependency weight, and migration from render-list
ops. Recommend two; do not decide. Core simulation, render-list determinism,
headless tests, and no package manager are frozen.

# Evidence and acceptance

Every nontrivial claim cites a primary source URL and access date. Include
base SHA, version/SHA of each evaluated upstream, risk table, migration sketch,
and explicit unknowns. Run and paste exit codes:

```powershell
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0114-renderer-backend-evaluation/captures/source-index.json','utf8')); console.log('source index: PASS')"
rg -n "Windows|macOS|sprite|atlas|shader|text|offscreen|license|CMake|GDI" orchestration/tasks/TASK-0114-renderer-backend-evaluation/EVALUATION.md
git diff --check
git diff --name-only
```

Expected: all criteria present and only this task folder changed. Negative
control: preserve at least one material UNKNOWN or unsupported candidate fact
rather than inferring it. Stop on unclear licensing or any need to add/run a
dependency. Fallback: complete remaining candidates and the null-option matrix.
