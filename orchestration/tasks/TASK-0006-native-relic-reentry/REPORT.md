# TASK-0006 report — native relic re-entry

## Implementation

Worker commit `f542a04f83d0e78896edd19635f8353c207b6fe3` implements deterministic relic re-entry in the native core. It adds the `RelicResurfaced` event, a named one-in-four shared-RNG roll, oldest-candidate transfer from the House relic pool to ground, stable identity/history preservation, and the `relic_resurfaced` legend record. Existing extraction and death paths preserve single ownership and record `relic_extracted` or return the relic to the pool exactly once.

## Verification

Worker verification passed:

```text
powershell -NoProfile -File native/build.ps1 -RunTests
native legacy denylist: PASS
verdigris core tests: PASS
```

Independent validator `/root/validate_task_0006` reviewed the spec and commit, confirmed the commit is based on `bc73ce0`, verified the owned-path scope, reran the native gate and direct core test executable, and returned **ACCEPT** with no corrections.

## Tests added

- `test_relic_resurface_round_trip`
- `test_relic_loss_again_returns_once`
- `test_relic_resurface_replay_is_deterministic`

## Scope

Only the task-owned native core paths changed:

- `native/include/verdigris/core.hpp`
- `native/src/core.cpp`
- `native/tests/core_tests.cpp`

Architect review and integration remain pending.
