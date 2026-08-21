---
task: TASK-0088
title: Stage 2 renderer backend ADR
state: DRAFT
packet: ARCHITECTURE
topology: PIPELINED
priority: high (cross-platform presentation foundation)
dependencies:
  - TASK-0073 ACCEPTED
owned_paths:
  - docs/rebuild/ADR-004-renderer-backend.md
  - orchestration/tasks/TASK-0088-renderer-adr/**
forbidden_paths:
  - native/**
  - package manifests
---

# Intended outcome

Architect-authored ADR selecting or explicitly deferring the Stage 2 renderer
backend from TASK-0073's evidence. It must preserve render-list contracts,
Windows/macOS viability, deterministic offscreen capture, and the dependency
policy. Owner approval is required before adding a new production dependency.
This task is not claimable while DRAFT.
