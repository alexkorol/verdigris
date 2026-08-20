---
task: TASK-0067
state: CLAIMED
coordinator: cursor
worker_branch: codex/TASK-0067-native-journey-ci-cursor
base_commit: 1f45eb337b29995485ba2b5adf60f5cdb00393c3e0
started_at: 2026-08-20T04:35:00-07:00
architect_review_required: true
expected_verification: green Actions run on worker branch + reverted canary proving journey step fails; architect retriggers once
notes: native.yml drives build.ps1 -RunTests -RunClientScenarios (owner path). No density bench. Ports unused (session tests bind ephemeral loopback).
---

Claimed per RUN_STATUS: 0067 READY (Gate A CI regression guard). 0068/0069 left for later.
