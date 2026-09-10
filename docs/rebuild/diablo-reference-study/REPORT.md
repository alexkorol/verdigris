# Diablo reference milestone: implementation and evidence

## Delivered

- A reproducible read-only CASC extractor and eleven-source hash/observation
  manifest for the installed D2R build `3.2.92777`.
- Cited D1/D2 architecture research and concrete Verdigris integration notes
  in `README.md`, with historical engine evidence distinguished from installed
  D2R data and from unmeasured gameplay behavior.
- An independently implemented native attack-cadence correction. Repeated
  input, changing target, and leaving/reentering range can no longer shorten
  the existing 350 ms recovery. First contact remains immediate.
- A deterministic regression, a corrected networking loot fixture that
  advances the server clock instead of exploiting click spam, and build-script
  failure propagation for core, networking and camera test binaries.

## Changed production behavior

`native/src/core.cpp::WorldSimulation::start_player_attack` used to assign
`next_player_attack_ms_ = now` on every trigger. The network handler processes
combat immediately after a trigger, so a second input 1 ms after contact
could resolve another hit instead of respecting the 350 ms deadline.

The new assignment retains the later of the outstanding deadline and the
nonnegative current time. Input chooses the target; resolved contact remains
responsible for advancing the attack clock. No damage formulas, item data,
network envelope, or art assets changed. Rapid-click effective damage is
lower than the buggy behavior; the intended attack interval is unchanged.

## Verification

From the isolated checkout:

```powershell
npm run playtest
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios -CaptureRoot .ci-artifacts/diablo-reference-study/final
```

- Browser protocol harness: exit 0, **32/32 scenarios passed**. Browser source
  was unchanged. Dependencies were reused through a local node_modules junction;
  the harness-generated journal append was removed from this patch.
- Final native build/test invocation: exit 0. Core, networking, camera,
  session, presentation-event and audio tests passed; **61 client scenarios
  passed**, including the real session journey and frame budget.
- Final frame budget: **30.7 ms average**, 20 real 32bpp frames at 3440×1440,
  existing requirement <40 ms. This is a gate result, not a controlled claim
  of performance improvement over the baseline.
- Cadence regression: rebuilt the original `HEAD:native/src/core.cpp` in an
  isolated negative-control binary, linked to the new tests. It failed at
  `repeat input cannot bypass attack recovery` after hits at 1000 and 1001 ms.
  The corrected production core passes the same test, including deadline,
  target-change, disengage/reengage and idle-resume cases.
- Build failure guards: executed the actual parsed core/network/camera
  RunTests blocks with a test command returning exit 17. All three threw the
  expected failure instead of continuing.
- Copied extractor: reproduced **11/11 matching SHA256 hashes**, **987,860
  bytes**; exit 0. Repository-output and nonallowlisted-path negative controls
  both exited 2 as expected. Raw outputs remain outside Git.
- `git diff --check`: pass.

Full local logs are in `.ci-artifacts/diablo-reference-study/`:
`native-final.log`, `browser-playtest.log`, `cadence-negative.log`,
`extraction-verification.log`, and `fail-fast-verification.json`.
The intermediate `native-after.log` deliberately remains as evidence of the
old loot fixture failure; **it is not the final acceptance run**.

The live native client was launched using `native/tools/play-native.ps1 -Local`,
captured using `native/tools/capture-window.ps1`, and the baseline capture was
viewed at original resolution. It is retained in `evidence/verdigris-baseline.png`.
That client was stopped after inspection. This patch changes cadence, not the
rendered composition, so no visual-overhaul acceptance is claimed.

## Initial milestone limits (September 8)

D2R launched, but the Windows capture helper failed twice with E_NOINTERFACE.
There is no measured D2R play sequence, audio mix, input latency or screen-pixel
layout in this milestone. The extracted data and public source research are
real evidence; they do not substitute for a future controlled play comparison.

The separately identified visible-windup/contact mismatch and remote duplicate
swing need a focused presentation fix with recorded-frame acceptance. Broader
HUD, materials, lighting, animation, audio and encounter changes are not made
by this patch. Prioritized experiments and their exact source seams are in
the reference README.

## September 9: actual D2R play and native contact presentation

The capture blocker is resolved through a foreground, window-bounded ImageGrab
capture while Computer Use supplies input. Created the offline Barbarian
`verdigris`, entered the Blood Moor, fought and killed a zombie, died and
recovered equipment, used a belt potion, collected gold and inspected inventory.
The README names the retained screenshots and maps direct observations to
Verdigris. Exact D2 animation timing and audio were not measured.

The native client now maintains actor-owned strikes. Speculative input cannot
restart a live preparation/recovery. An authoritative attack contact replaces
that actor's preparation with one Active strike; player pose and lunge read
the same strike. Enemy arcs cannot drive the player's attack pose. Damage,
the 350 ms cadence and the network protocol remain unchanged. This is visual
reconciliation; the wire has no request ID or miss/rejection result.

Changed six implementation/test files: `native/client/main.cpp`,
`native/client/presentation_state.hpp`, `native/client/presentation_state.cpp`,
`native/client/remote_session.cpp`, `native/tests/presentation_events_tests.cpp`,
and `native/tests/session_tests.cpp`.

Independent review caught a remote Sweep regression before commit: the decoder
put facing in the event text, so contact replaced the predicted SweepArc with
a Swing. The decoder now consumes the existing authoritative `skillId`; a
real-network Sweep check verifies one confirmed SweepArc and no Swing.

Verification:

- Native build, denylist, core, networking, camera, presentation-event, audio
  and full real-network session suites passed.
- All **62 client scenarios** passed. Frame budget: **27.3 ms average** over
  20 production frames at 3440x1440, below the unchanged 40 ms bound.
- The `strike-contact` scenario captures production before/input/contact/
  recovery frames at 960x600. Contact comes from actual local simulation damage;
  preparation uses the production prediction helper. A separate session test
  verifies actual decoded remote contact leaves exactly one Active strike.
- Early, late and expired speculation, repeated input, enemy ownership, and
  unconfirmed expiry are covered by focused presentation checks.
- `npm run playtest`: **32/32 passed**, exit 0. Its generated journal append
  was excluded from the change.
- Launched the rebuilt local client with `native/tools/play-native.ps1 -Local`,
  captured through `native/tools/capture-window.ps1`, and inspected at its
  original 3440x1440 display size. The original surface/silhouette/overlap
  problems remain; this patch is not visual-art acceptance.

Reproduce the complete native gate with:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios -CaptureRoot .ci-artifacts/strike-recheck
```

Local test logs and captures are `.ci-artifacts/strike-*`; the final Sweep
regression rerun logs use `strike-sweep-*`. The live capture is
`.ci-artifacts/strike-contact/verdigris-live.png`. Rendering confirms changed
phase/feedback sequencing, not a measured D2 timing match or an audio review.
The four 960x600 production frames are also retained under this study's
`evidence/strike-{before,input,contact,recovery}.png` paths for later review.

## Integration

Work is isolated on `codex/diablo-reference-study-20260908`, based on
`2b5da07b1`, in `C:/Users/Alex/Documents/ChatGPT/verdigris-diablo-study`.
The architect checkout and preexisting Codex checkout were not switched or
cleaned. This milestone is committed locally, not pushed or merged into the
program branch. Locate its commit with:

```powershell
git log -1 --format=%H codex/diablo-reference-study-20260908
```
