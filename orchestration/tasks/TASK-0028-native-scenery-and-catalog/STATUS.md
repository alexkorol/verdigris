---
task: TASK-0028
state: REVISE
coordinator: codex
worker: Luna native-client implementation worker
worker_branch: codex/TASK-0028-native-scenery-and-catalog
worktree: C:\Users\Alex\Documents\ChatGPT\verdigris\.codex\worktrees\TASK-0028-native-scenery-and-catalog
base_commit: e7a7a78b4521b29f84e985242dde70b0fa492e00
started_at: 2026-08-16T19:10:00-07:00
dependencies: TASK-0015 and TASK-0016 integrated at current program tip
expected_verification: powershell -File native/build.ps1 -RunTests -RunClient; git diff --check; driven native client pass with task captures
known_risks: native/client-only scope; scenery colliders and depth sorting must not alter Simulation or headless output; catalog exports must already suffice
architect_review_required: true
implementation_commit: 53d2b06c58f8eb44dedd935ff068a8878989764c
validator: independent Luna validator
validator_result: REVISE — native gate and scope pass; dash sweep and driven evidence need correction
revision_required:
  - replace destination-only dash scenery collision with swept/intermediate collision
  - prove behind/in-front traversal and dash blocking in driven captures
---
