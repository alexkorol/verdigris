---
task: TASK-0069
state: CLAIMED
coordinator: cursor
worker_branch: codex/TASK-0069-remote-reconnect-cursor
base_commit: 1f45eb337b29995485ba2b5adf60f5cdb00393c3
started_at: 2026-08-20T05:00:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests (drop to Retrying then Ready; session-replaced stays Disconnected, no retry)
notes: ConnectionState::Retrying with 3-attempt 1s/2s/4s backoff, same guest, no local fallback. Ports 6580-6599.
---

Claimed per RUN_STATUS: 0069 READY (Gate B reconnect). 0067 and 0068 are REVIEW_REQUESTED.
