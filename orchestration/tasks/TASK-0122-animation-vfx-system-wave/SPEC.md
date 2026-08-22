---
task: TASK-0122
title: Production animation and VFX system wave
state: DRAFT
packet: ARCHITECTURE
topology: PIPELINED
job: ARCHITECTURE
priority: P1
dependencies: [TASK-0116 ACCEPTED, renderer/text strategy frozen, owner asset policy available or procedural fallback approved]
owned_paths: [to be frozen after TASK-0116]
forbidden_paths: [simulation authority changes, invented production art]
---

# Intended outcome

Scaffold and ship the first production animation/VFX slice with authoritative
event timing, interpolation-only presentation, deterministic captures, combat
readability tests, both target resolutions, and owner play evidence. DRAFT until
interfaces, assets/fallback, paths, and exact gates are frozen.
