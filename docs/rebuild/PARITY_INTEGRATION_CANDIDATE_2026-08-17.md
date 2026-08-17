# Native/browser parity integration candidate

Coordinator checkpoint: 2026-08-17. This is a disposable integration
candidate, not an architectural acceptance or production merge.

## Candidate

- Base coordinator tip: `2b3c6764`
- Candidate tip: `aa45c78f79c9f397f5bb8633441dd8de95a437c2`
- Branch: `codex/integration-parity-candidate`
- Applied TASK-0043 correction chain (13 commits through `f9527d9c`) and
  TASK-0044 implementation `d476788`; all cherry-picks were conflict-free.

## Scope

The candidate changes only the union of the two owned surfaces:

- `playtest/**` (TASK-0043 harness/timing correction)
- six TASK-0044 native files under `native/include/verdigris/`,
  `native/src/`, and `native/tests/`

No `server/`, `src/`, package, product, or orchestration implementation files
were changed. `git diff --check` passed.

## Verification

- `npm run test:unit`: **122 files / 779 tests passed**.
- `powershell -NoProfile -File native/build.ps1 -RunTests -RunClient`:
  **PASS** (denylist, core, networking, client shell; stored item/trophy 1/1).
- Native server attach on port 6530 with unchanged harness:
  `quickstart`, `single-session`, `movement`, `zones` — **4/4 passed**.
- Zones exercised all six N2 combinations, stairs, and saved-position
  restoration (`38,115` → `38,115`); N2’s deliberate 18-monster stub remains
  documented and is not claimed as full encounter parity.

The candidate remains outside `codex/native-reconstitution` until Fable
accepts TASK-0043 and TASK-0044 independently.

## Current-tip refresh

Because the coordinator advanced with documentation commits after the first
dry-run, a fresh disposable candidate was rebuilt directly from `9fea5668`:

- branch `codex/integration-parity-candidate-v3`, tip `3636b729`;
- the TASK-0043 correction chain and TASK-0044 `d476788` applied cleanly;
- browser unit gate: 122 files / 779 tests;
- unchanged browser playtest: 31/31, runner exit 0;
- native denylist/core/networking/client gate: PASS;
- unchanged native attach (`quickstart`, `single-session`, `movement`,
  `zones`): 4/4, runner exit 0.

Raw task-folder captures record the exact commands and diagnostics. This is
still a review candidate, not an architect acceptance or production merge.
