---
task: TASK-0190
title: Native town and NPC runtime integration
state: AUTO_RELEASE
packet: IMPLEMENTATION
topology: PIPELINED
job: IMPLEMENTATION
priority: P1
parent_outcome: integrated native Owner Demo recognizable as Verdigris
dependencies: [TASK-0177 ACCEPTED]
owner_input_dependency: none unless dependency records a new owner-only gate
likely_paths: [native/src/core.cpp, orchestration/tasks/TASK-0190-native-town-runtime-integration/**]
acceptance_class: integrated player-visible evidence plus relevant native gates and npm run playtest for gameplay claims
release_predicate: dependencies accepted; current-tip paths interfaces resource capsule base and collisions validate
heartbeat_minutes: 15
lease_minutes: 40
retry_limit: 2
fallback: preserve evidence and split smallest path-disjoint vertical slice
successor_rule: on ACCEPTED release dependency-satisfied successors; after second failure split or change approach
generation_provenance: owner interview 2026-08-23 Owner Demo P0
---

# Outcome

Load non-combat town, three NPC roles, services, and crisis direction into runtime.

# Promotion requirements

Before READY, stamp exact current base, exact owned/forbidden paths, collision-free capsule, dependency verdicts, literal commands, and visual evidence. Gameplay completion requires npm run playtest on the exact integrated state. Reject disconnected showcases, generic substitutes, invisible attacks, and local-only fake state.
