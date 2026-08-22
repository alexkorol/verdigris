---
task: TASK-0153
title: Native first-session clarity implementation wave
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: a5f4133eac6d5b3fd41fb2b046b604460395e7b7
owner_visible_contribution: makes the first expedition goal, extraction action, and essential controls legible in the normal native owner path
dependencies: [TASK-0119]
owner_input_dependency: none; final narrative wording, lore, and House/Scion naming remain owner-only and out of scope
owned_paths: [native/client/main.cpp, native/client/presentation_events.hpp, native/client/presentation_state.cpp, native/client/presentation_state.hpp, native/client/session.hpp, native/client/remote_session.cpp, native/client/remote_session.hpp, native/README.md, orchestration/tasks/TASK-0153-native-first-session-clarity-wave/**]
forbidden_paths: [native/src/**, native/include/**, native/tests/**, server/**, src/**, docs/product/**, docs/rebuild/**, orchestration/tasks/TASK-0119-onboarding-first-session-audit/**, everything else]
resource_capsule: native client implementation; use only the shared 6580-6599 test capsule; never touch port 6500
---

# Outcome and invariants

Implement the first unblocked, non-lore fixes from accepted TASK-0119 so a new
owner understands what the game expects during the first native expedition.
The normal HUD—not an F3/debug surface—must make these three facts truthful and
legible:

1. the current authoritative expedition phase/goal;
2. the real extraction action for the active session mode (remote owner path
   must never say `press F` if walking onto EXIT is the contract);
3. the essential combat/loot/gear controls, including the dash answer to enemy
   telegraphs, without becoming a checklist tutorial.

Use existing state and event seams. Do not invent quests, narrative copy,
routes, progression rules, lore, names, or server authority. Do not add a
second tutorial system. Extend the established objective/status-chip/hint
presentation language and keep the existing loose-guidance character.

# Required implementation

- Correct the local/remote extraction instruction mismatch with an explicit,
  testable mode-aware presentation contract.
- Surface the existing `ExpeditionPhaseChanged` signal, or the equivalent
  already-authoritative phase state, through the owner-visible objective/event
  presentation. It must distinguish the slay phase from the carry-to-exit
  phase and may not fabricate state client-side.
- Add a restrained always-available controls hint for WASD/mouse, attack,
  dash, pickup/labels, and gear. It must coexist with the scene at 960x600 and
  1366x768 and must not require F3.
- Correct the stale native README bindings to match the implemented client.
- Add or extend a deterministic client scenario named
  `first-session-clarity` (or an equivalently explicit name) proving all three
  contracts through the real dispatch/ingest/present pipeline. Scenario-only
  shortcuts may select the deterministic scene but may not create production
  behavior that exists only under test.
- Preserve all existing quickstart, first-fight, telegraph-dodge,
  loot-to-bank, combat-juice, and chronicles-gate-b behavior.

# Acceptance and evidence

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
native/build/verdigris_client.exe --scenario first-session-clarity
git diff --check
git diff --name-only
```

Expected: full native build/tests/all client scenarios pass; the focused
scenario exits zero and proves truthful remote extraction guidance,
authoritative phase presentation, and a normal-HUD controls hint. Only owned
paths change. Commit and push only the worker branch, then set STATUS to
`REVIEW_REQUESTED` with exact gate output and commit SHAs.

# Negative controls

- Remote owner mode must not render `press F there` as the extraction action.
- The controls contract must be visible with F3/debug overlay disabled.
- No passive-tree UI, name entry, narrative/lore copy, quest checklist,
  server change, port 6500 use, or direct-state production shortcut.
