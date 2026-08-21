---
task: TASK-0089
title: Native Gate C campaign-decision journey
state: DRAFT
packet: BOUNDED-DESIGN
topology: PIPELINED
priority: critical (D-122 axis 2 completion)
dependencies:
  - TASK-0077 ACCEPTED
  - TASK-0078 ACCEPTED
  - TASK-0086 ACCEPTED
  - architect resolves every MISSING Gate C contract field
owned_paths:
  - native/client/**
  - orchestration/tasks/TASK-0089-gate-c-native-client/**
forbidden_paths:
  - native/src/**
  - playtest/**
---

# Intended outcome

Native client presents and executes a Gate C route choice using concrete goal,
danger/boss, expected reward family, depth, consequence, and return condition;
then proves the decision in a real networked client journey. Exact interfaces
and gates will be frozen after TASK-0086. This task is not claimable while
DRAFT.
