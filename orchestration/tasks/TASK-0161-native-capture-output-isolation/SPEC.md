---
task: TASK-0161
title: Native scenario capture-output isolation
state: DRAFT
packet: IMPLEMENTATION
topology: PIPELINED
job: IMPLEMENTATION
priority: P1
base_commit: dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17
owner_visible_contribution: keeps full native verification from rewriting historical evidence and falsely dirtying active integration worktrees
dependencies: [TASK-0159 ACCEPTED]
owner_input_dependency: none
owned_paths: [native/client/main.cpp, native/build.ps1, orchestration/tasks/TASK-0161-native-capture-output-isolation/**]
forbidden_paths: [native/client/remote_session.cpp, native/client/presentation_state.cpp, native/src/**, native/include/**, native/tests/**, server/**, src/**, CI, historical capture contents, gameplay rules, everything else]
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
and is not claimable until that task is ACCEPTED and its exact interface is
frozen.
