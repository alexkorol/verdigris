---
task: TASK-0204
title: Owner Demo audio cue integration
state: AUTO_RELEASE
packet: IMPLEMENTATION
topology: PIPELINED
job: IMPLEMENTATION
priority: P1
parent_outcome: integrated native Owner Demo recognizable as Verdigris
dependencies: [TASK-0157 ACCEPTED; TASK-0203 ACCEPTED]
owner_input_dependency: none unless dependency records a new owner-only gate
likely_paths: [native/audio/event_cues.cpp, orchestration/tasks/TASK-0204-owner-demo-audio-beats/**]
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

Add cues for attacks, hits, boss death, level-up, gate, loot, and menus.

# Promotion requirements

Before READY, stamp exact current base, exact owned/forbidden paths, collision-free capsule, dependency verdicts, literal commands, and visual evidence. Gameplay completion requires npm run playtest on the exact integrated state. Reject disconnected showcases, generic substitutes, invisible attacks, and local-only fake state.
