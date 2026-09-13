# REVIEW — TASK-0182 native-item-art-render-adapter

- reviewer: independent validator (claude subagent), judged and recorded by
  coordinator-of-day claude-architect-pc, 2026-08-24 ~08:40 PDT
- head reviewed: 0171e0a2 (branch
  codex/TASK-0182-native-item-art-render-adapter-cursor; SHA resolved from
  heartbeat commit 42c66681 — STATUS.md omitted it; already ancestor of
  the program branch)
- verdict: **REVISE** — harness green and scope clean, but the core
  mapping the SPEC asks for is not delivered.

## What passed

- Harness reproduced: 25 checks + legacy denylist green (MSVC 2019,
  detached review worktree).
- Scope exact (item_art_renderer.hpp + task dir); frozen surfaces
  untouched; native boundary respected.
- Provenance intact: all 12 catalog PNGs exist; footprints/categories/
  filenames match items/manifest.json; wizard_commit matches
  source_manifest.json.

## REVISE corrections (numbered, testable)

1. **Re-key the adapter on real ids.** resolve() keys on invented
   std::uint32_t ids 101-602 (item_art_renderer.hpp:102-113) that neither
   the sim (string ids, e.g. "bronze-sword" — native/src/core.cpp:2636+,
   core.hpp:287 pick_up(const std::string&)) nor the art pack (string ids,
   e.g. "dagger_bronze" — items/manifest.json) uses. Nothing can consume
   this adapter without a third mapping that doesn't exist. Re-key on the
   manifest string ids (or sim ItemDef ids with an explicit, tested
   mapping table). Acceptance: a test resolves at least three real sim
   item ids end-to-end to manifest art entries.
2. **Add a drift guard.** The header hardcodes a copy of 0169 manifest
   data with no cross-check. Add a test that parses items/manifest.json
   (or a generated snapshot of it) and fails on any divergence in
   footprint/category/filename, with a deliberate-mismatch negative
   control.

## Process notes (coordinator-side)

3. STATUS.md lacks the frozen head SHA (recurring lane-template gap).
4. SPEC never promoted to READY (still AUTO_RELEASE; no owned/forbidden
   paths or base stamped) — factory promotion gap, recorded for the
   backlog-factory follow-up.
5. Dependency gate: TASK-0169 ACCEPTED required before any ACCEPT here
   regardless of corrections 1-2.

Minor, non-blocking: copy_path leaves buffer unterminated for >=32-char
names (none currently); degenerate cells return Ok with zero-width blit.

- revision lane: claude-a claims the r2 revision per BUS (branch
  codex/TASK-0182-native-item-art-render-adapter-claude-r2).
