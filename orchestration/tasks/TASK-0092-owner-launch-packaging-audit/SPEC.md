---
task: TASK-0092
title: Owner launch and packaging readiness audit
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P1
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: turns the current developer launcher into a measured path toward a double-clickable owner build
dependencies: []
owner_input_dependency: none; signing and distribution remain owner-only
owned_paths: [orchestration/tasks/TASK-0092-owner-launch-packaging-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no launcher execution and no ports
---

# Outcome

Produce `FINDINGS.md` and `captures/package-inventory.json` inventorying the
current launcher, executable/runtime dependencies, generated files, asset and
save locations, clean-machine assumptions, failure messages, version metadata,
and Windows/macOS gaps. Define sequenced packaging packets without changing
builds, shortcuts, signing, accounts, or release infrastructure.

# Frozen invariants and evidence

Owner port 6500, loopback-only servers, forgiving persistence, and current
one-command owner path are frozen. Cite every claim. Separate build portability,
packaging, signing/notarization, installer, and launch UX. Report base SHA,
files, commands, JSON, risks, and smallest successors.

# Acceptance

```powershell
rg -n "play-native|verdigris_client|verdigris_server|6520|6539" native/README.md native/tools native/build.ps1
rg -n "CMAKE|MSVC|WIN32|APPLE|install|package" native/CMakeLists.txt native/CMakePresets.json native/build.ps1
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0092-owner-launch-packaging-audit/captures/package-inventory.json','utf8')); console.log('package inventory: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: identify one clean-
machine assumption not proven by the current launcher and show the missing
check. Stop before executing installers, changing PATH, signing, or deciding a
distribution channel; continue documenting independent gaps if one is blocked.
