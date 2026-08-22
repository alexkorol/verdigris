---
task: TASK-0123
title: Native audio and music runtime wave
state: DRAFT
packet: ARCHITECTURE
topology: PIPELINED
job: ARCHITECTURE
priority: P1
dependencies: [TASK-0117 ACCEPTED, backend/license decision, owner audio direction or synthetic test assets]
owned_paths: [to be frozen after TASK-0117]
forbidden_paths: [gameplay authority, unapproved music/sound assets]
---

# Intended outcome

Implement device/bus/voice lifecycle and the first event-driven combat/UI/
ambience proof with deterministic no-device tests, settings persistence, clean
shutdown, packaging, and owner-visible listening evidence. Music/content waits
on owner approval. Not claimable while DRAFT.
