---
task: TASK-0108
title: First post-parity combat-depth wave
state: DRAFT
packet: BOUNDED-DESIGN
topology: PIPELINED
job: BOUNDED-DESIGN
priority: P1
dependencies: [TASK-0101 ACCEPTED, architect selects a content-neutral gap, locking tests scaffolded]
owned_paths: [to be frozen after TASK-0101]
forbidden_paths: [balance, naming, lore, unrelated client/server paths]
---

# Intended outcome

Ship one coherent owner-visible combat vocabulary increment with shared-actor
authority, telegraph/impact presentation, deterministic tests, and D-115 play
evidence. The audit—not genre convention—selects the wave. DRAFT until frozen.

## Selected gap after TASK-0101 ACCEPTED (not yet READY)

TASK-0101 selected **W1 / GAP-RANGED-BEHAVIOUR**: realize authored
`behaviour_type` ranged (and keep buffer inert) in tile-space combat together
with a deterministic readable telegraph and client-visible attributed hit beat.
Do not invent projectile art, cadence, or damage values.

**Hold:** do not stamp READY while TASK-0161 still owns `native/client/main.cpp`.
After 0161 is independently ACCEPTED/INTEGRATED, freeze owned_paths, base SHA,
capsule, and locking tests from FINDINGS.md W1 on the then-current program tip.
