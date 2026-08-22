---
task: TASK-0126
title: Clean-machine packaging and release harness
state: DRAFT
packet: ARCHITECTURE
topology: PIPELINED
job: ARCHITECTURE
priority: P0
dependencies: [TASK-0092 ACCEPTED, TASK-0094 ACCEPTED, TASK-0099 ACCEPTED, TASK-0120 ACCEPTED, renderer/runtime dependencies frozen]
owned_paths: [to be frozen after release audits]
forbidden_paths: [signing/notarization/account actions without owner approval]
---

# Intended outcome

Build a reproducible clean-clone/build/package/launch/shutdown/support-bundle
gate for Windows and CI equivalents for macOS, with exact artifact manifests,
runtime dependencies, rollback, and authentic failure controls. DRAFT until
release contract and account boundaries are frozen.
