# OI-009 — sound and music direction

**State:** WAITING_EVIDENCE on TASK-0117. **Deadline:** before authored audio or
music ships; backend-neutral runtime/test work may proceed first.

Decision required: approve the audio backend/dependency, sound provenance and
creation policy, ambience/combat/UI direction, music composition/licensing,
mix targets, dynamic music states, and accessibility defaults.

Recommended choice: first approve an event-driven cross-platform runtime with
no-device tests, buses/voice limits, persisted controls, package provenance,
and synthetic test assets; then compare owner-approved authored sound/music
sets in the playable client. Alternatives: licensed library/composer sources
with archived rights; or procedural/temporary non-production cues while music
is deferred. Acceptance requires Windows/macOS behavior, clean shutdown,
readable combat priorities, independent volume/mute, captions where relevant,
license manifest, and owner listening/play verdict. No generation is requested
yet. Fallback: TASK-0117 and interface/test decomposition continue.
