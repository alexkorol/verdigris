# VG-GOV-001 — Freeze a reproducible baseline

Draft record 2026-09-06 by Cursor Grok. This is the working baseline
manifest for the parallel pack. It is **not** an owner-frozen package
hash. Planning IDs stay DRAFT; this does not mint TASK numbers.

Companion: `docs/execution/BASELINE.md`.

## Recorded heads (do not collapse these)

| Role | Ref | SHA | Notes |
|---|---|---|---|
| Pack source review (historical) | `origin/master` | `2d3e92a5` | Stale for READY stamping |
| Pack integration ref (historical) | `origin/codex/native-reconstitution` at pack write | `8597c654` | Stale for READY stamping |
| Architect checkout (this tree) | `codex/native-reconstitution` | `486058f3` plus the Cursor HUD/pack wave landing with this file | Cursor lease; HUD/GPU/ART/GOV artifacts |
| Kimi READY / program head | `origin/codex/goal-aaa-systems` | `e7b65360` | 59 commits ahead of pack integration ref; AAA systems lane |

A second checkout that is not at one of these recorded SHAs is a
**mismatch**, not a silent baseline. Treating `2d3e92a5` or `8597c654` as
the live program head fails the negative control.

## Reproduce

Architect checkout:

```
git fetch --prune origin
git checkout codex/native-reconstitution
git rev-parse HEAD
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
```

Kimi clone / READY base:

```
git fetch --prune origin
git checkout e7b65360
npm run playtest
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests
```

Kimi reported 2026-09-05 at `e7b65360`: playtest 32/32 exit 0; native
379 PASS / 0 FAIL. Cursor's presentation scenarios in this tree are
proven on the architect checkout, not by pretending this tree is
`e7b65360`.

## Package hash

**Unset.** Owner freeze still required. Until then, identity is the git
SHAs above plus the path list of the commit that lands this file.

## Negative control

A checkout at a different commit, or a missing pack under
`docs/execution/pack/`, must be reported and must not be accepted as
this baseline.
