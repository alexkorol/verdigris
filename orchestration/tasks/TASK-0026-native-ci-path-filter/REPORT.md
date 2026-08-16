---
task: TASK-0026
state: IMPLEMENTED
branch: codex/TASK-0026-native-ci-path-filter
commits:
  - 146e3b7
base_commit: 41b5d75
---

# TASK-0026 report — native CI path filters

## Executive summary

The native workflow now runs when the native source, legacy denylist, or the
workflow itself changes, for both pushes and pull requests.

## Implementation

Both `push.paths` and `pull_request.paths` contain exactly:

- `native/**`
- `config/legacy-denylist.json`
- `.github/workflows/native.yml`

All existing permissions, concurrency, jobs, and native steps are unchanged.

## Changed files

- `.github/workflows/native.yml`

## Verification

- YAML parse: `YAML OK: .github/workflows/native.yml parsed successfully`
- `git diff --check 41b5d75..146e3b7` — PASS
- Diff scope is exactly the single owned workflow file.

## Specification deviations

None.

## Risks and limitations

The worker branch has no upstream configured; this does not affect the
workflow-only change or local validation.

## Integration notes

Awaiting independent validator review and architect acceptance before
integration.
