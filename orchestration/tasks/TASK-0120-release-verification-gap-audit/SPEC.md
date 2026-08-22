---
task: TASK-0120
title: Clean-machine, migration, soak, and release verification audit
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P0
base_commit: 42718fbc4340589e606fff94a6eaa3dfbd03ad1c
owner_visible_contribution: defines the proof required before a native build can be called safely releasable
dependencies: []
owner_input_dependency: signing, notarization, distribution, and account actions remain owner-only
owned_paths: [orchestration/tasks/TASK-0120-release-verification-gap-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no installer execution, account mutation, or ports
---

# Outcome and invariants

Produce `FINDINGS.md` and `captures/release-gates.json` inventorying current CI,
clean clone/build, dependency/runtime packaging, launcher, server shutdown,
long soak, protocol regression, deterministic replay, performance, crash/save
recovery, save-version migration, upgrade/rollback, asset/license manifest,
Windows/macOS coverage, signing/notarization, installer, logs/support bundle,
and release acceptance. Separate proven, missing, owner-only, and external gates.

# Acceptance and evidence

```powershell
rg -n "CI|clean|build|package|launcher|soak|migration|upgrade|rollback|sign|notar|installer|release|artifact|crash|save" .github native docs orchestration --glob "*.md" --glob "*.yml" --glob "*.yaml" --glob "*.ps1" --glob "*.json"
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0120-release-verification-gap-audit/captures/release-gates.json','utf8')); console.log('release gates: PASS')"
git diff --check
git diff --name-only
```

Expected: only this folder changes and no narrow test is promoted to broad
release proof. Negative control: identify one release claim not supported by a
clean-machine or migration artifact. Stop before account/irreversible actions;
continue staging machine-verifiable gates.
