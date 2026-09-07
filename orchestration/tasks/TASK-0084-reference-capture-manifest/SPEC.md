---
task: TASK-0084
title: Reference-capture integrity manifest
state: SUPERSEDED
superseded_by: integrated (reviewed head 3e84a878, 2026-08-23)
packet: MECHANICAL
topology: INDEPENDENT
priority: medium-high (presentation regression evidence)
lane: luna-mac; Qwen drafting allowed with machine verification
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owned_paths:
  - orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.mjs
  - orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.json
  - orchestration/tasks/TASK-0084-reference-capture-manifest/**
forbidden_paths:
  - native/**
  - src/**
  - server/**
  - playtest/**
  - orchestration/benchmarks/side-by-side-2026-08-20/*.png
  - orchestration/benchmarks/side-by-side-2026-08-20/*.jpg
---

# Outcome

Create a dependency-free Node verifier and checked-in manifest for the frozen
browser/native/composite benchmark. It must enumerate every expected scene,
read PNG/JPEG dimensions without decoding or rewriting images, hash each file,
validate the five-scene naming matrix, validate native render-list JSON, and
fail on missing, duplicate, zero-byte, wrong-resolution, malformed, or
unmanifested evidence.

`--write` may regenerate only `reference-manifest.json`; default mode is
read-only verification. The manifest records relative path, byte length,
dimensions, SHA-256, scene id, side, and source revision.

# Acceptance commands

Paste literal transcripts and exit codes in `REPORT.md`:

```bash
node --check orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.mjs
node orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.mjs --write
node orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.mjs --verify
git diff --check
```

Run an authentic negative against a copied manifest with one bad hash; capture
the non-zero exit, then remove the disposable copy without changing evidence.

# Stop conditions

STOP if verification would require rewriting image evidence or adding a binary
dependency.
