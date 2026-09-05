# Combat recovery feedback — 2026-09-04

Scope: the owner's missing/misleading skill feedback and a related travel
edge. This is not the unlocked-skill/hotbar-binding implementation.

## Changes

- The simulation records the duration of an accepted attack separately from
  its remaining recovery. Physical skills share that recovery; Sweep, combo
  finishers, equipment speed and temporary haste keep their actual cadence.
- Both combat-state updates and full snapshots publish `cooldownTotalTicks`.
  Remote/local models and the production quickbar preserve it. The radial
  wipe uses remaining/total, not the previous fixed 30-tick denominator.
  An older server without this field gets an honestly blocked slot, rather
  than an invented clock. No save format change: these are transient fields.
- Town return and new-floor entry discard pending recovery, as they already
  discard pending attacks/combo state. The remote transition clears the UI
  recovery immediately, before the next snapshot arrives.
- Remote physical-skill prediction no longer draws swings in town, before
  admission, while dead, during known recovery, or without enough resource.
  Input still reaches the server: stale client state cannot veto a legal
  authoritative action. Eligible whiffs retain immediate feedback, bounded
  to one predicted effect per presentation tick.
- LocalCoreSession now mirrors recovery and active Warcry duration, which
  were previously omitted from its player model.

## Regression coverage

- Core: accepted Thrust/Sweep duration; automatic combo-finisher duration;
  half recovery; rejected casts cannot change the denominator; equipment
  haste; town/new-floor reset and immediate attack in the new instance.
- Protocol: both push and snapshot carry the actual accepted duration.
- Real WebSocket mirror: combat update, snapshot cadence change, immediate
  town-transition clear and older-payload fallback.
- Native production painter: full/half/ready/unknown-duration clocks, plus
  the finisher capture. Input scenario covers every suppressed prediction
  state, recovery to ready, request authority and same-tick bounds.

## Acceptance

- `native/build.ps1 -RunTests -RunClientScenarios -BuildSubdirectory
  combat-feedback -CaptureRoot native/build/combat-feedback/captures`: exit 0.
  Core/network/camera/session/presentation/audio suites and all 36 client
  scenarios pass. Fullscreen frame budget: 11.7 ms average over 20 actual
  3440x1440 production frames, below the unchanged 40 ms ceiling.
  Log: `native/build/combat-feedback-acceptance.log`.
- `npm run playtest`: 32/32 scenarios passed, exit 0.
  Log: `native/build/combat-feedback-playtest.log`; generated session-arc row
  retained in `docs/loop-journal.md`.
- `node native/tools/test-account-restart.mjs
  native/build/combat-feedback/verdigris_server.exe`: exit 0. Vendor purchase,
  gear/currency, exact item identity/rolls, reserve Scions, pre-admission oath,
  zone travel, abrupt restart, final death/relic recovery and storage failure
  controls pass. Log: `native/build/combat-feedback/restart-proof.log`.
  Retained fixture: `C:\Users\Alex\AppData\Local\Temp\verdigris-restart-aECV5y`.
- Viewed `native/build/combat-feedback/captures/
  combat-cadence-finisher-1366x768.png`: production quickbar shows the dark
  recovery wipe and hand over the physical actions. This is a synthetic-world
  painter fixture, not a live-window capture or a whole-game art assessment.

The first compile caught a new test calling private `return_to_town`; the
test now exercises public `reset_to_town`. The full gate above passed after
that correction. Desktop controls remain untouched after the owner's earlier
Escape interruption; live combat UI acceptance remains pending. Existing
running title-screen client/server were not replaced or restarted.

## Still open

Warcry already has an authoritative resource-consuming buff action, but it is
still exposed as a default R binding. The owner's requested unlock gating and
hold-to-select LMB/RMB/Q/E/R/T hotbar are not implemented by this patch. An
asynchronous question asks whether active unlocks come from skill-tree nodes,
equipment or town training; do not silently invent this progression policy.

The rest of the voice checklist remains in `owner-2026-09-03.md`.
