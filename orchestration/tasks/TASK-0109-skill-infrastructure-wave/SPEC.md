---
task: TASK-0109
title: Content-neutral skill infrastructure wave
state: DRAFT
packet: ARCHITECTURE
topology: PIPELINED
job: ARCHITECTURE
priority: P1
dependencies: [TASK-0102 ACCEPTED, architect freezes slot/action/effect interfaces]
owned_paths: [to be frozen after TASK-0102]
forbidden_paths: [production magic, new skill content, balance]
---

# Intended outcome

Scaffold authoritative LMB/RMB/Q/E/R definitions, validation, persistence, and
presentation seams without inventing magic or skill content. Lock the seam with
tests before a worker implementation packet. Not claimable while DRAFT.
