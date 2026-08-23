# TASK-0158 architect review

Verdict: **ACCEPTED**

Reviewed worker head:
`016d35a25a86d048ab1c1412fbd23e3dc1f33243`

## Scope and lineage

- The immutable SPEC base
  `ad1a1e178e689df442d4655937f8e8e037cf4cd2` and routed base
  `1f69e82af310121ec0d20548ff15735984467406` are both ancestors of the
  frozen handoff.
- The complete post-base diff contains only
  `native/client/pane_model.hpp` and the TASK-0158 task folder. No forbidden
  CMake, painting, gameplay, networking, persistence, server, browser, or
  authored-content path changed.
- Worker head equals its remote and the detached architect review worktree
  was clean.

## Independent verification

The architect reran the literal acceptance harness against the frozen head:

```text
powershell -NoProfile -ExecutionPolicy Bypass -File orchestration/tasks/TASK-0158-native-pane-model-foundation/run-tests.ps1
pane model tests: PASS (413 checks)
TASK-0158 pane model acceptance harness: PASS
```

MSVC 2019 v16.11.42 compiled the production header with C++20 and `/W4`
without warnings. The 413 assertions cover the required 960x600, 1366x768,
and 1920x1080 viewports; zero and large row sets; deterministic geometry;
tab stability; reserved-region non-overlap; shrink-to-fit behavior; invalid
viewports/minimums; and explicit unpaintable degradation. Additional checks:

```text
git diff --check <routed-base>..<worker-head>  -> PASS
python native/tools/check_legacy_denylist.py  -> PASS
```

## Load-bearing inspection

1. `pane::plan()` validates all externally supplied dimensions before
   emitting geometry, computes row products in 64-bit space, and narrows only
   after saturation.
2. Successful panel and child rectangles remain inside the safe band between
   the 148px top HUD reserve and 96px bottom reserve. Failed plans retain only
   a status and zero rectangles.
3. The header depends only on `<algorithm>` and `<cstdint>`; the Win32/GDI
   independence STOP condition was not approached.
4. The seam is intentionally not wired into production painting by this
   packet. That integration and its rendered evidence remain successor work.

This is a bounded-design acceptance. The authoring lane used Ox Alpha and the
architect inspection used the independent Codex family; no additional
different-family reviewer was available. Deterministic evidence is retained
in the task folder and the public seam remains reversible because no existing
consumer includes it yet.

Integrate implementation commit `5c65a0f290dd685295f65760e5746bf595b4a584`
and handoff/evidence commit
`016d35a25a86d048ab1c1412fbd23e3dc1f33243`, then rerun the focused harness
and denylist from the combined program head before marking INTEGRATED.

## Program integration

- Architect verdict commit: `9fd04e98`
- Program implementation commit: `14809519`
- Program evidence/handoff commit: `2be77c30`
- Combined-head MSVC harness: PASS (413 checks, `/W4`)
- Combined-head native legacy denylist: PASS
- Combined-head diff hygiene: PASS

Lifecycle is now **INTEGRATED**.
