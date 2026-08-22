---
task: TASK-0161
title: Native scenario capture-output isolation
state: READY
packet: IMPLEMENTATION
topology: PIPELINED
job: IMPLEMENTATION
priority: P1
base_commit: 30cdad4bfa1cf1f07944ed5ac2fb8327569aa63a
owner_visible_contribution: keeps full native verification from rewriting historical evidence and falsely dirtying active integration worktrees
dependencies: [TASK-0159 ACCEPTED]
owner_input_dependency: none
owned_paths: [native/client/main.cpp, native/build.ps1, orchestration/tasks/TASK-0161-native-capture-output-isolation/**]
forbidden_paths: [native/client/remote_session.cpp, native/client/presentation_state.cpp, native/src/**, native/include/**, native/tests/**, server/**, src/**, CI, historical capture contents, gameplay rules, everything else]
resource_capsule: loopback ports 7260-7279; never touch port 6500
ready_promoted_at: 2026-08-22T16:18:00-07:00
promotion_provenance: PC architect validation after TASK-0159 ACCEPTED/INTEGRATED; exact main.cpp interface frozen at 30cdad4b; owned-path collision scan clear
---

# Outcome

Give native scenario runs an explicit capture root so a full validation gate can
write fresh evidence into a disposable/task-owned directory without rewriting
committed captures from TASK-0122, TASK-0145, TASK-0156, or future packets.
Default owner play must remain unchanged; only evidence-output location changes.

# Acceptance

- Add one explicit build/scenario option or environment seam for a capture root,
  validate it as a contained path, and thread it through every scenario capture
  helper without changing render behavior.
- Prove a complete `-RunClientScenarios` run with an isolated capture root exits
  zero and leaves `git status --short` empty.
- Prove an invalid/outside-repository capture target fails before writing.
- Preserve default scenario behavior for direct task-specific evidence runs.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios -CaptureRoot orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review
git status --short
git diff --check
git diff --name-only
```

# Negative controls and STOP conditions

No deletion/restoration of historical captures, no `git checkout`/`git restore`
cleanup, no swallowed write errors, no renderer/gameplay change, and no writes
outside the explicit contained root. This task shares `main.cpp` with TASK-0159
and is claimable only from a routed base containing accepted/integrated
TASK-0159 with its exact interface frozen.
