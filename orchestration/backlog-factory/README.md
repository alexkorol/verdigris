# D-128 machine-scale planning manifests

This directory is architect-owned planning authority, not gameplay or worker
implementation. `build-manifests.mjs` deterministically expands the curated
ARPG domain/subsystem catalog into:

- `generated/product-graph.nodes.jsonl`: 2,000 concrete terminal-graph nodes;
- `generated/packet-reserve.jsonl`: 500 detailed DRAFT/AUTO_RELEASE packets;
- `generated/summary.json`: counts, composition, domain coverage, and the
  intentionally honest runway-confidence state.

Regenerate after changing the curated catalog:

```powershell
node orchestration/backlog-factory/build-manifests.mjs
```

Verify committed outputs are canonical and internally consistent:

```powershell
node orchestration/backlog-factory/build-manifests.mjs --check
```

Generated DRAFT/AUTO_RELEASE packets deliberately have no immutable base SHA.
Promotion to READY still requires current-tip validation, exact owned/forbidden
paths, resource capsule, acceptance commands, evidence, and collision checks.
The manifests do not route another Verdigris worker or authorize concurrent
implementation.
