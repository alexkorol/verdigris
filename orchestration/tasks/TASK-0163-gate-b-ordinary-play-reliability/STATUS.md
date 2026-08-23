# TASK-0163 — Gate-B ordinary-play journey reliability

- state: CLAIMED
- lane: ox-pc-bh
- model: openrouter/stealth/ox-alpha
- base SHA: 75ef6b7b68dd5986f26943728a2796d29e9eec23
- branch: worker/verdigris/pc/ox-pc-bh
- claimed at: 2026-08-23T17:27:35Z

## Scope

Owned: `native/tests/session_tests.cpp`,
`orchestration/tasks/TASK-0163-gate-b-ordinary-play-reliability/**`.

Minimal change per owner decision (QUESTION-0001, option a): the gate-b
driver's `monster:telegraph` handler must identify the boss ground-slam by its
authored signature (`skillId == "boss:ground-slam"`), not by "any
monster:telegraph". Non-slam telegraphs (ranged trash warnings) must be ignored
by the elite-hunt state machine. No timeout weakening, no assertion deletion,
no runtime/gameplay changes.
