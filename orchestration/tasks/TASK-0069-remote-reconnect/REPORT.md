---
task: TASK-0069
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0069-remote-reconnect-cursor
base_commit: 1f45eb337b29995485ba2b5adf60f5cdb00393c3
architect_review_required: true
---

# TASK-0069 REPORT — remote session reconnect/retry

## Executive summary

An unexpected drop no longer jumps straight to `Disconnected`. The remote
session enters `Retrying`, waits 1s/2s/4s across three attempts, reconnects
with the same guest id, and returns to `Ready` from the server's login
snapshot. `player:session-replaced` sets a terminal `Disconnected` and does
not retry. There is still no local-sim fallback.

## Approach

- Split socket+upgrade+login into `connect_transport`; `poll` drains the
  inbox first so a replaced envelope wins over the TCP close.
- First unexpected drop after `Ready` emits `ConnectionLost` and schedules
  backoff. Successful login resets the attempt counter.
- Explicit `shutdown()` suppresses retry.

## Changed files

- `native/client/remote_session.hpp`
- `native/client/remote_session.cpp`
- `native/tests/session_tests.cpp` — resume + no-retry (acceptance requires
  these; not listed in owned_paths)

## Verification

`powershell -File native/build.ps1 -RunTests` (2026-08-20): denylist, core,
networking, camera2d, session tests all green, including:

- drop → `Retrying` → server restart → `Ready`, same guest, snapshot present
- `player:session-replaced` → `Disconnected`, never `Retrying`

Architect reruns `-RunTests`.

## Deviations

None. No `native/src` edits. Server session reuse is used as-is; a full
process restart yields a fresh authoritative login snapshot for that guest.
