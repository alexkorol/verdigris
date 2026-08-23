---
task: TASK-0161
state: REVIEW_REQUESTED
coordinator: codex
worker: ox-pc-ah
machine: DESKTOP-TVU7OR7
root: Z:\Code\.worktrees\verdigris\ox-pc-ah
worker_branch: codex/TASK-0161-native-capture-output-isolation-ox-pc-ah
base_commit: 610a240e1e4bdfacfd77bec49e36be945a1ced13
spec_base_commit: 30cdad4bfa1cf1f07944ed5ac2fb8327569aa63a
ports: 7260-7279 loopback only; port 6500 never touched
provider: openrouter
model: stealth/ox-alpha
harness: OpenCode CLI variant max
started_at: 2026-08-22T23:26:21Z
claim_commit: 7730e49a3a1d7743132438d23b13500f3106564b
implementation_commit: c8cedf1730455d0fc62badc3c707dcbc96123254
expected_verification: powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios -CaptureRoot orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review; git status --short; git diff --check; git diff --name-only; plus negative controls proving invalid/outside-repository capture roots fail before writing and default scenario behavior is preserved
verification_outcome: literal acceptance command EXIT=0 twice (denylist PASS; core/networking/camera2d/session/presentation-events/audio suites green; all 12 client scenarios PASS); post-commit re-run left git status --short, git diff --check, and git diff --name-only all empty with no cleanup commands; client-seam and build-layer negative controls (outside-repo, ..-escaping, uncreatable-under-file) all fail loudly with exit 1 and zero filesystem writes; no-env scenario runs stay green on the unchanged historical ladder
captures:
  - orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review/front-door-960x600.png
  - orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review/expedition-hud-960x600.png
  - orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review/animation-vfx-phase-a-960x600.png
  - orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review/animation-vfx-phase-a-1366x768.png
  - orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review/progression-surface-nonzero-960x600.png
  - orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review/progression-surface-zero-1366x768.png
  - orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review/hud-pane-readability-closed-960x600.png
  - orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review/hud-pane-readability-open-960x600.png
  - orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review/hud-pane-readability-closed-1366x768.png
  - orchestration/tasks/TASK-0161-native-capture-output-isolation/captures/review/hud-pane-readability-open-1366x768.png
known_risks: none blocking; see REPORT.md (capture byte-stability observed across two full gate runs; scenario servers keep their inherited fixed loopback capsules, unchanged by this task)
---

CLAIMED: TASK-0161 (native scenario capture-output isolation) at routed base
610a240e1e4bdfacfd77bec49e36be945a1ced13 on worker branch
codex/TASK-0161-native-capture-output-isolation-ox-pc-ah. Preflight proved:
clean tree, HEAD exactly at the routed base, which contains the immutable SPEC
base 30cdad4bfa1cf1f07944ed5ac2fb8327569aa63a (accepted/integrated TASK-0159,
interface frozen) as an ancestor; the pushed program tip origin/codex/native-reconstitution
still contains the routed base; no STATUS.md or RELEASE.md existed in the task
folder on current origin (only SPEC.md). Work is confined to native/client/main.cpp,
native/build.ps1, and this task folder. Historical capture contents are
untouched.

REVIEW_REQUESTED: implementation commit
c8cedf1730455d0fc62badc3c707dcbc96123254 adds one explicit `-CaptureRoot`
build option (validated strictly repository-contained before any build or run
work), hands the resolved absolute root to the client through the single
`VERDIGRIS_CAPTURE_ROOT` seam, and threads it through all five scenario capture
helpers with fail-before-write guards; without the variable every helper keeps
its historical ladder. The literal acceptance command passed twice at EXIT=0
(all six test suites, all 12 scenarios), and the post-commit re-run left
`git status --short`, `git diff --check`, and `git diff --name-only` empty with
no cleanup commands. Negative controls (outside-repo absolute, `..`-escaping
relative, uncreatable path under an existing file, and build-layer rejection)
all fail loudly at exit 1 with zero filesystem writes. Changed paths are
exactly the owned ones plus this task's fresh review captures. Full transcripts,
interfaces, deviations, risks, and SHAs in REPORT.md.
