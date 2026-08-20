---
task: TASK-0069
title: Remote session reconnect/retry (Gate B groundwork)
state: READY
packet: BOUNDED-DESIGN
lane: any native lane
priority: medium (Gate B requires relaunch/reconnect)
owned_paths:
  - native/client/**
  - orchestration/tasks/TASK-0069-remote-reconnect/**
forbidden_paths:
  - native/src/** (server session reuse exists; the 0056 lane owns
    server changes — file notes)
  - playtest/**
---

# Outcome

Implement ConnectionState::Retrying for real: on an unexpected drop
the remote session retries with backoff (3 attempts, 1s/2s/4s),
re-logs-in with the SAME guest identity, and resumes from the server's
authoritative snapshot (server already supports session reuse and
replacement flush). Explicit failure after the attempts — never a
silent local fallback. A deliberate mid-session server restart in the
session tests proves resume. player:session-replaced (another client
taking the identity) must NOT trigger retry — that stays a terminal
Disconnected.

# Acceptance

build.ps1 -RunTests green incl. new reconnect tests (drop → Retrying →
Ready resume; replaced → Disconnected, no retry). Architect reruns.
