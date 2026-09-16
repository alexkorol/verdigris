# GATEB-FLAKE — diagnosis milestone report

**Status:** DIAGNOSIS + tooling delivered; fix NOT applied (behavior/policy boundary — routed).
**Branch:** `kimiwork/gateb-flake-harness`, base `e7b65360`. Author: kimi-work, 2026-09-06.
**Task origin:** CURSOR_KIMI_LANES.md — "gate-b flake is a harness follow-up on your side;
session_tests.cpp stays frozen." (4+ cross-lane sightings of the same signature.)

## Symptom

`session_tests.cpp` gate-b journey intermittently fails at
"slain rare guardian surfaces the circulating heirloom" with
"hunt aborted - the successor fell to ordinary combat". Fails under full-gate load
(repeatedly, incl. 2/2 on one worktree); passes standalone (3/3+). Present at unmodified
baseline; in-code note at `session_tests.cpp:1590-1607` already knew the mode.

## Root cause (evidence-backed)

The env-gated trace (`VERDIGRIS_GATEB_TRACE=1`, committed in `native/src/networking.cpp`,
inert by default) captured the actual kill shot in a failing run
(`evidence/patched-trace-run1.log`):

```
GATEB-TRACE t=... type=hit attacker=Stone Lurker(3,10,alive=1) skill=monster:attack
target=PLAYER amount=4 health=0/100 died=1 player_at=(2,9)
```

An ordinary Stone Lurker melees the **successor** dead during the relic-pickup walk.
The mechanism: after succession, the driver walks the heir to the heirloom through live
adds; the walk's survival is wall-clock-timing-dependent. Ambient load (parallel suites,
second agent on the machine) stretches the driver's real-time loop while the sim keeps
ticking, so adds get extra swings. The N5 respawn ward ("monsters cannot damage a
freshly-respawned scion until the scion acts", `networking.cpp` process_combat) does not
protect this leg: the ward ends the moment the scion acts, and the driver issues movement
immediately — so protection duration ≈ one command under any load.

**Leading hypothesis (needs one more verification pass):** the ward applies to the N5
respawn path only; the chronicles *succession* path (heir set-out/select) gets no
equivalent grace, and even where the ward applies it is action-ended, not time/tick-bounded.

## Repro harness (committed)

- `native/tools/gateb_flake_repro.cpp` + `build-repro.bat` / `repro.ps1` — reruns the
  journey under induced CPU load with measured failure counts.
- `evidence/`: base vs patched, load vs no-load logs. Measured: base fails under load
  (`base-load8b-run1` etc.), passes unloaded; the patched-driver experiment
  (`patched/session_tests_patched.cpp` — a COPY, the frozen file was never edited)
  reduced but did NOT eliminate the flake (patched logs still contain one FAIL),
  confirming the mechanism is server-side timing, not only driver pacing.

## Why no fix is applied

The honest fixes all cross a boundary I may not unilaterally redraw:

1. **Sim-behavior option:** extend spawn/succession protection to the chronicles-succession
   path or make the ward tick-bounded instead of first-action-ended — touches mortality
   fairness semantics (D-106/D-109 territory; Tier C).
2. **Frozen-test option:** the pickup leg's timing assumptions live in frozen
   `session_tests.cpp` (D-129).
3. **Balance option:** adds lethality during pickup is a cadence/damage retune — owner-only
   per TASK-0108's frozen constraints.

Recommended routing: architect/owner picks (1) — a succession grace window is most
consistent with D-109's forgiving-persistence intent ("disconnect deaths must be
prevented"); a scripted successor dying to adds mid-pickup is the same injustice class.

## Commands / exit codes (representative)

- `native/build/verdigris_session_tests.exe` standalone ×3: exit 0.
- Full `-RunTests` gate on the merge worktree under load: exit 1 twice, sole failure this check.
- Repro harness: see `evidence/` logs (base-load8b-run1: 1 FAIL; patched-noload-run1: 0 FAILs).

## Limitations

- Ward-coverage hypothesis not yet confirmed to file:line for the succession path.
- Repro rates are indicative, not statistical (single machine, shared with another agent).
- The committed trace adds zero behavior when the env var is unset (static init, no I/O).
