# TASK-0182 report

## Deliverable (r2, lane claude-a)

`native/client/item_art_renderer.hpp` — catalog of 12 TASK-0169 items
keyed on the REAL manifest string ids (items/manifest.json), with
aspect-preserving cell fit and deterministic diagnostics
(Ok/NoArt/NotFound/Invalid, category_name/status_name).

REVIEW corrections applied:

1. Invented uint32 id space dropped. `resolve(manifest_id, cell)` keys on
   manifest strings; `resolve_sim(sim_id, cell)` goes end-to-end from the
   sim vocabulary via the single explicit constexpr `sim_art_map()` table
   covering all 28 `kItemCatalogue` ids (native/src/core.cpp:2636-2674).
   4 concept matches are mapped (bronze-dagger -> dagger_bronze,
   bronze-pike -> boar_pike, bronze-boots -> boots_fur,
   knife -> cur_knife); the remaining 24 sim items are documented no-art
   and return `Status::NoArt` with empty entry/blit — a status, never an
   invented fallback sprite.
2. Drift guard: the test binary receives the actual
   `native/client/assets/wizard/items/manifest.json` path from
   run-tests.ps1, parses it minimally (dependency-free, string-aware span
   scanner), and fails on any divergence in id/category/filename/footprint
   in either direction, with five deliberate-mismatch negative controls
   proving the guard can fail.

Minor r1 notes fixed in passing: bounded copies always null-terminate
(`copy_str`); degenerate zero-width/height blits are skipped (`Invalid`
instead of `Ok` with an undrawable blit).

## Evidence

Harness PASS (worktree claude-a, MSVC 2019 x64, /W4, C++20):
627 checks + legacy denylist. Compile-time `static_assert`s prove the
constexpr surface, including sim-id resolution at compile time.

## Successor

TASK-0184 inventory integration, TASK-0198 Brand crafting loop.
