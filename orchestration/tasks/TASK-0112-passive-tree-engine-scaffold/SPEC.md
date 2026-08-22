---
task: TASK-0112
title: Authoritative passive-tree engine scaffold
state: DRAFT
packet: ARCHITECTURE
topology: PIPELINED
job: ARCHITECTURE
priority: P0
dependencies: [TASK-0105 ACCEPTED, owner provides or approves topology/content source]
owned_paths: [to be frozen after TASK-0105]
forbidden_paths: [invented nodes, approximation ratification, balance]
---

# Intended outcome

Replace the known projection approximation with versioned topology/allocation
interfaces, migration rules, persistence, and tests using owner-approved source
data. Falls back to content-neutral schema/test scaffolding while input waits.
Not claimable while DRAFT.
