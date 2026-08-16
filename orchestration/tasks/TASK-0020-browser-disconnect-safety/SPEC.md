---
id: TASK-0020
title: Browser game — forgiving disconnects per owner ruling D-109
state: READY
track: web-demo
priority: critical
base_commit: current program tip (coordinator records the SHA in STATUS.md)
dependencies: []
parallel_safe: true
owned_paths:
  - server/**
  - tests/unit/**
  - tests/e2e/**
forbidden_paths:
  - native/**
  - prototypes/**
  - src/**
  - docs/**
  - package.json
acceptance_commands:
  - npm run test:unit
  - npm run playtest
---

## Goal

The browser game implements D-109: a disconnect or logout never costs the
player their life or items — the character is pulled from danger safely
and returns in town on next login.

## Why this task exists

Owner ruling 2026-08-16: with permadeath, connection loss must be
forgiving. The browser game is the near-term shippable; today its
disconnect behavior predates this ruling and must be audited and aligned.

## Product and architectural invariants

- D-109 exactly: on socket close/timeout — persist the player's current
  carried state immediately, remove the character from the live instance/
  world (no lingering body that monsters can kill), and mark their next
  spawn as the town/House location. Death remains the only loss event.
- If the player is mid-combat at disconnect, they must NOT die after the
  socket closes (no posthumous damage ticks).
- Server-authoritative persistence paths only (PlayerPersistenceService /
  Chronicles repositories — see the TASK-0005 audit report for the seams);
  do not invent parallel save paths.
- Do not change the wire protocol `{event, data}`.

## Scope

1. Audit the current disconnect path (`server/socket.js` close handling,
   player cleanup) and document in REPORT.md what happens today.
2. Implement: immediate persist on disconnect; safe removal from combat/
   instance; next-login spawn at town. Reuse existing spawn/town logic.
3. Tests: unit spec for the disconnect handler (persist called, actor
   removed, no damage after close, next spawn location town); extend an
   e2e/playtest scenario if the harness supports a mid-combat disconnect
   simulation (report if it does not — do not build new harness
   infrastructure in this task).

## Non-goals

Native core changes, logout-delay anti-abuse (future balance), UI
messaging, reconnect-resume-in-place.

## Acceptance criteria

Both acceptance commands green; the scope-3 unit tests named in
REPORT.md; the audit section documents before/after behavior.

## Review focus

No posthumous deaths, persistence ordering (persist BEFORE removal), and
that guest and account/Chronicles paths both get the behavior.

## Stop conditions

If town-spawn semantics are ambiguous for a mode (e.g., mid-campaign
Chronicles Scion), stop and file a question rather than inventing spawn
rules.
