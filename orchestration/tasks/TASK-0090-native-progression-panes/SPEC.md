---
task: TASK-0090
title: Native character, inventory, and passive-tree panes
state: DRAFT
packet: BOUNDED-DESIGN
topology: PIPELINED
priority: high (constitution UI parity)
dependencies:
  - TASK-0087 ACCEPTED
  - protocol/state audit confirms authoritative stats, wear, inventory, and tree payloads
owned_paths:
  - native/client/**
  - orchestration/tasks/TASK-0090-native-progression-panes/**
forbidden_paths:
  - native/src/**
  - playtest/**
---

# Intended outcome

Build the native character+inventory diptych and passive-tree presentation on
the accepted pane shell, consuming authoritative state only. No passive-tree
formula or owner-only progression rule may be invented. Exact payload and
interaction gates will be frozen before promotion. This task is not claimable
while DRAFT.
