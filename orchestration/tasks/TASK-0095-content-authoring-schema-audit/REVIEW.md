---
task: TASK-0095
title: Native content and asset-authoring schema audit
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: d902c861
reviewed_at: 2026-08-23T20:30:00Z
revision: 1
---

# Review — TASK-0095 (native content and asset-authoring schema audit)

## Verdict: ACCEPTED

Frozen head `d902c861` (worker branch `worker/verdigris/pc/ox-pc-bd`),
content head `8413d122`, reviewed in detached worktree
`review-task0095-d902c861`.

## Scope

Worker's own commits (`6a57bece..d902c861`) touch only
`orchestration/tasks/TASK-0095-content-authoring-schema-audit/**`
(FINDINGS.md, REPORT.md, STATUS.md, captures/content-surfaces.json). No
schema/content implemented; read-only capsule honored (no generators, no
servers, port 6500 untouched). `git diff --check` clean.

## Acceptance gates

1. `rg -n "catalog|theme|zone|monster|skill|item|trophy|quest|seed" native/content native/src native/include server --glob ...`
   → 3721 lines, exit 0.
2. `node -e "JSON.parse(...content-surfaces.json...); console.log('content surfaces: PASS')"`
   → prints `content surfaces: PASS`, exit 0.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → owned additions only, exit 0.

## Evidence quality

- FINDINGS.md is excellent and comprehensive: maps where every content surface
  is authored today (zones/layouts, actors, skills, items/trophies, quests,
  presentation), the stable-ID contracts (frozen ordinals, wire keys, identifier
  grammar), deterministic seed boundaries (with a key structural fact: seeded
  generation consumes authored tables positionally), validation gaps, the
  negative control, a versioned schema/tool pipeline, and migration risks.
- **Negative control verified genuine:** `gear_drop_pool()` (core.cpp:2748-2757)
  is declared in load-bearing order and the gear roll selects
  `pool[... * pool.size()]` positionally (core.cpp:3167-3168; comment "pool
  index is rolled"). Externalizing to JSON preserves values but not order
  intent — any sort/merge/rewrite reshuffles drops with zero type/unit errors.
  Before externalization it needs a byte-order locking test + a parity fixture.
- Validation gaps are concrete (no cross-language enum lock, no pool-order
  test, no referential integrity for quest↔zone-id/boss-name coupling, narrow
  seam coverage, absent migration tooling). The proposed pipeline (lockstep
  checker extracting C++ literals and asserting committed seeds byte-for-byte;
  ref-by-reference quest objectives; parsing outside the headless sim)
  correctly preserves the no-I/O core rule and determinism.
- Owner-only vs mechanical split correctly stops at names/lore/balance.
- Machine-readable twin `captures/content-surfaces.json` parses.

## Capsule

Read-only audit respected: no schema/content implemented, no generators/servers
run, no ports bound, port 6500 untouched, only owned task-folder paths changed.

## Follow-up

Successor should implement the lockstep checker + the byte-order pool locking
tests, and externalize the next entities (item-base with explicit sizes, loot
pools with explicit `order` integers, quest refs by id) incrementally with
before/after parity fixtures.
