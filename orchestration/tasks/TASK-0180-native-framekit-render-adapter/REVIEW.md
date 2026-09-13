# REVIEW — TASK-0180 native-framekit-render-adapter

- reviewer: independent validator (claude subagent), judged and recorded by
  coordinator-of-day claude-architect-pc, 2026-08-24 ~08:25 PDT
- head reviewed: 084e3160 (delivery commit a874b6c1, branch
  codex/TASK-0180-native-framekit-render-adapter-cursor; already ancestor
  of the program branch)
- verdict: **ACCEPTED, conditional on TASK-0167 acceptance** (SPEC
  dependency; 0167 verdict must be recorded before anything downstream
  claims player-visible completion).

## Evidence

- Harness PASS, reproduced independently: 18 checks + legacy denylist
  green (MSVC 2019, /std:c++20 /W4, real exe run from detached review
  worktree).
- Scope exact: native/client/framekit_renderer.hpp (+240) + task dir;
  heartbeat line in .orch/events.ndjson. Frozen surfaces untouched.
  Native boundary compliant (header-only constexpr planner, no IO).
- Nine-slice math hand-verified (piece tiling exact; sw-l-r underflow
  guarded at framekit_renderer.hpp:61-68; undersized dests rejected :154).
- Provenance verified: framekit manifest carries WIZARD commit 66a5d9ff;
  panel.png sha256 re-hashed and matches manifest byte-for-byte; renderer
  defaults match manifest metrics exactly.

## Conditions / advisories

1. TASK-0167 ACCEPTED required before chain claims (worker disclosed).
2. Not player-visible by itself — no production consumer until
   TASK-0183/0184; do not mark the chain player-visible until they land.
3. Drift risk: default_*_asset() constants (framekit_renderer.hpp:102-124)
   hardcode manifest metrics; regenerating 0167 assets can silently
   diverge. Successor should derive from manifest.json.
4. Zero-size center/edge regions emitted at exact-minimum dests — the
   eventual blitter must skip zero-size regions.
5. Process: STATUS.md lacks a frozen head SHA in frontmatter (recovered
   from heartbeat commit message). Lane template should stamp it per
   BUS.md.
