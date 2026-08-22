---
task: TASK-0113
title: Versioned campaign and content-tooling scaffold
state: DRAFT
packet: ARCHITECTURE
topology: PIPELINED
job: ARCHITECTURE
priority: P1
dependencies: [TASK-0095 ACCEPTED, TASK-0096 ACCEPTED, schema and graph validators frozen]
owned_paths: [to be frozen after TASK-0095 and TASK-0096]
forbidden_paths: [campaign canon, names, lore, rewards, pacing decisions]
---

# Intended outcome

Scaffold a versioned, deterministic content schema and campaign graph validator
with migration and connectivity tests. Owner-authored content remains data, not
an agent invention. Not claimable while DRAFT.
