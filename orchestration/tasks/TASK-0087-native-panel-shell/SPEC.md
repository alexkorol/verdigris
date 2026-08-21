---
task: TASK-0087
title: Native pane shell and panel model
state: DRAFT
packet: BOUNDED-DESIGN
topology: PIPELINED
priority: high (presentation delta #4)
dependencies:
  - TASK-0079 ACCEPTED
  - TASK-0078 ACCEPTED
  - architect freezes pane interfaces from TASK-0079 findings
owned_paths:
  - native/client/**
  - orchestration/tasks/TASK-0087-native-panel-shell/**
forbidden_paths:
  - native/src/**
  - playtest/**
---

# Intended outcome

Implement the first native pane shell using the accepted browser inventory as
contract: deterministic panel model, anchor/layout rules at 1366x768 and
1920x1080, open/close focus behavior, and the first load-bearing panels. Exact
panel order and frozen interfaces will be added by the architect after
TASK-0079. This task is not claimable while DRAFT.
