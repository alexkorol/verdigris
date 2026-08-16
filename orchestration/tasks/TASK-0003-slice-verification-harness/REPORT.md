---
task: TASK-0003
state: REVIEW_REQUESTED
branch: codex/TASK-0003-slice-verification-harness
commits:
  - 278f7dd
base_commit: 0e02aa7
spec_base_commit: f5b4b72
---

## Executive summary

The Founding of a House prototype now has a self-contained one-command
Playwright gate. It rebuilds into a temporary copy, rejects committed-output
drift without overwriting `index.html`, serves the slice from an ephemeral
HTTP port, checks load cleanliness, drives all three founding directions, and
proves the full death/relic/successor/founding arc.

## Implementation

- Added `run-checks.mjs` with temporary build freshness comparison, HTTP
  serving, Playwright launch, and pass/fail summary.
- Added `tests/slice-checks.mjs` covering load, direction stats, full loop, and
  persistence/console behavior through the existing `window.__V` surface.
- Kept all edits inside the task-owned prototype harness paths.

## Changed files

- `prototypes/founding-slice/run-checks.mjs`
- `prototypes/founding-slice/tests/slice-checks.mjs`
- `prototypes/founding-slice/tests/REPORT.md`

## Interfaces

- New command: `node prototypes/founding-slice/run-checks.mjs`.
- No game or shared repository configuration changes.

## Verification

```text
PASS  build freshness / drift guard
PASS  load: boot with no console errors
PASS  fresh-house arc: all three directions have distinct stats
PASS  full loop: equip → crisis → LMB combat → clear → death/relic → successor → founding

4/4 checks passed in 18.2s
```

ESLint also passed. A deliberate scratch-copy drift probe failed as expected
with `NEGATIVE PASS: deliberate scratch index drift detected`; the committed
`index.html` remained unchanged.

## Manual checks

- Worktree was clean after commit.
- No dependencies were added.
- No production slice HTML, assets, build script, native code, or shared test
  configuration was changed.

## Specification deviations

None reported.

## Risks and limitations

The harness depends on the repository's installed Playwright/Chromium
environment and the current `window.__V` debug surface. It intentionally does
not add CI wiring or visual-regression assertions.

## Questions and follow-up

No owner decision is required. A future CI task may call this command without
changing the harness contract.
