---
task: TASK-0160
title: Native procedural visual-kit packaging proof
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
base_commit: dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17
owner_visible_contribution: makes the committed vector/procedural placeholders reproducible and verifiably present instead of silently disappearing between source and owner launch
dependencies: [TASK-0144 ACCEPTED]
owner_input_dependency: none for integrity and reproducibility; final art direction and replacement assets remain owner-only
owned_paths: [native/client/assets/**, native/tools/verify_native_visual_kit.py, orchestration/tasks/TASK-0160-native-visual-kit-packaging-proof/**]
forbidden_paths: [native/client/main.cpp, native/client/remote_session.cpp, native/src/**, native/include/**, native/build.ps1, native/CMakeLists.txt, server/**, src/**, binary production art, third-party dependencies, final art decisions, everything else]
---

# Outcome

Turn the existing SVG manifest and generated embedded visual-kit header into a
reproducible, dependency-free asset contract. A clean validator/generator must
prove every manifest entry exists, is valid bounded SVG, maps to one stable
generated symbol, and reproduces the committed header byte-for-byte. The
runtime fallback remains embedded and asset-neutral; this packet does not add
or select final art.

# Acceptance

Add a Python-stdlib command at `native/tools/verify_native_visual_kit.py` with
check and explicit regeneration modes. Check mode must be read-only, reject a
missing/unknown/duplicate manifest entry and malformed or unsafe SVG, verify
stable ordering/hash metadata, and fail if the committed generated header is
stale. Commit focused negative fixtures only under the task folder.

```powershell
python native/tools/verify_native_visual_kit.py --check
python orchestration/tasks/TASK-0160-native-visual-kit-packaging-proof/run_negative_tests.py
git diff --check
git diff --name-only
```

# Negative controls and STOP conditions

No network, package install, raster asset, runtime/client paint change, WIZARD
import, copyrighted external asset, lore, naming, or aesthetic judgment. STOP
if the existing files cannot be reproduced without changing their visual
meaning; report the exact mismatch instead.
