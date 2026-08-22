---
task: TASK-0094
title: Native asset provenance manifest audit
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P2
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: prevents presentation work from depending on unshippable or unidentified assets
dependencies: []
owner_input_dependency: production asset policy and final selections remain owner-only
owned_paths: [orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no network downloads
---

# Outcome

Produce `FINDINGS.md` and `captures/assets.json` listing every asset currently
consumed or proposed by native presentation: relative path, type, dimensions,
bytes, hash, provenance evidence, license status, build/package use, and
KEEP/UNKNOWN/BLOCKED classification. Include WIZARD and reference assets only
as candidates; do not copy them.

# Frozen invariants and evidence

No asset, license, package manifest, or product canon changes. Hash only files
already present. Cite inventories and code consumers. Required report includes
base SHA, exact commands, unresolved provenance, and a packaging successor.

# Acceptance

```powershell
rg -n "asset|atlas|terrain|splash|orb|png|jpg|bmp" native docs/rebuild docs/product --glob "*.md" --glob "*.cpp" --glob "*.hpp" --glob "*.json"
node -e "const x=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0094-asset-provenance-manifest-audit/captures/assets.json','utf8')); if(!Array.isArray(x.assets)) process.exit(1); console.log('asset manifest: PASS')"
git diff --check
git diff --name-only
```

Expected: only owned evidence changes. Negative control: at least one UNKNOWN
asset remains non-shippable and is named. Stop on missing provenance; do not
infer a license. Continue hashing and classifying other assets.
