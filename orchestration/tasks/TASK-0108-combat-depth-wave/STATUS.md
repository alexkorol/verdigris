# TASK-0108-combat-depth-wave — STATUS

state: CLAIMED
lane: ox-pc-ba
model: openrouter/stealth/ox-alpha
base_commit: 763684666b07483caeeebc2055c804f80bb1515e
branch: worker/verdigris/pc/ox-pc-ba
claimed_at: 2026-08-22T22:51:31-07:00

## Claim notes

Preflight complete: worktree clean at base commit `763684666b07483caeeebc2055c804f80bb1515e`
(matches SPEC `base_commit`). Branch `worker/verdigris/pc/ox-pc-ba` checked out,
no upstream yet (will push claim to `origin worker/verdigris/pc/ox-pc-ba`).
Resource capsule respected: loopback 7280-7299 only; port 6500 untouched.

Implementation will proceed strictly within owned_paths:

- native/src/core.cpp
- native/include/verdigris/core.hpp
- native/tests/core_tests.cpp
- native/client/presentation_state.cpp
- native/client/render_list.hpp
- native/client/main.cpp
- native/tests/presentation_events_tests.cpp
- orchestration/tasks/TASK-0108-combat-depth-wave/**
