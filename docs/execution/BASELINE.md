# VG-GOV-001 — reproducible baseline (draft record)

Recorded 2026-09-05, refreshed 2026-09-06 by Cursor Grok. Working
manifest, not an owner-frozen package hash. Decision write-up:
`docs/execution/decisions/freeze-a-reproducible-baseline.md`.

| Field | Value |
|---|---|
| Repository | https://github.com/alexkorol/verdigris |
| Architect checkout | `Z:\Code\Games\delaford\delaford_game` |
| Architect branch | `codex/native-reconstitution` |
| Architect HEAD at wave start | `486058f31002c4f1c55cd0e71888defc204bbd3e` |
| Architect wave commit | `0ff5182a` native: land pack ingest and Cursor-lane presentation wave |
| Tip subject then | docs: handoff for vector art + themed roads run |
| Kimi READY / program head | `origin/codex/goal-aaa-systems` @ `e7b65360` |
| Origin of this branch | `origin/codex/native-reconstitution` (architect checkout was 20 commits ahead at record; owner now asked to push) |
| Pack edition | Verdigris native design and multi-agent execution pack 1.0 (2026-09-04) at `docs/execution/pack/` |
| Pack source review SHAs (historical, not live heads) | master `2d3e92a5`, integration `8597c654` |
| Native build | `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios` |
| Presentation gate | `native/tools/play-native.ps1` + `native/tools/capture-window.ps1`; `--scenario all` includes `frame-budget` |
| Browser gate | `npm run playtest` (historical reference; port 6500) |
| Product authority | `docs/product/VERDIGRIS_CONSTITUTION.md` |
| Orchestration | `orchestration/PROTOCOL.md` — first-STATUS-write-wins; this checkout is architect-owned |
| Parallel agent map | `orchestration/CURSOR_KIMI_LANES.md` |

Negative control: a checkout that is neither `486058f3` (plus this wave)
nor `e7b65360`, or one that treats `2d3e92a5`/`8597c654` as current,
must not be accepted as this baseline. Package hash is **unset** until
the owner freezes it.
