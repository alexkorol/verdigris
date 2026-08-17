---
id: TASK-0033
title: Default to permanent daytime; day/night cycle behind a settings toggle
state: READY
track: web-demo
priority: high
base_commit: current program tip (coordinator records the SHA)
dependencies: []
parallel_safe: true
owned_paths:
  - src/core/rendering/**
  - src/components/ui/Settings*.vue
  - src/core/config/**
  - tests/unit/rendering*.spec.js
  - tests/unit/perspective-camera.spec.js
forbidden_paths:
  - server/**
  - native/**
  - prototypes/**
  - package.json
acceptance_commands:
  - npm run test:unit
  - npm run smoke:browser
---

## Goal

Per D-111: the game boots in permanent midday (the accepted Phase-3
neutral grade at its brightest keyframe), and the full day/night cycle
becomes an opt-in settings toggle (persisted client-side), OFF by
default. No lighting code is deleted — the owner may rule either way
later.

## Scope

Freeze the ambient clock at the midday keyframe unless the toggle is on;
expose the toggle in the existing settings UI surface; persist the
choice; keep emitter lights working in both modes; captures (task
folder) proving default boot is midday and the toggle restores the
cycle.

## Acceptance criteria

Gates green; default-boot capture is midday-bright; toggle capture shows
the cycle running; setting survives reload.

## Stop conditions

Any settings-UI redesign temptation — one toggle row only.
