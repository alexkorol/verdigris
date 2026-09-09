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

## Limits and remaining work

D2R launched, but the Windows capture helper failed twice with E_NOINTERFACE.
There is no measured D2R play sequence, audio mix, input latency or screen-pixel
layout in this milestone. The extracted data and public source research are
real evidence; they do not substitute for a future controlled play comparison.

The separately identified visible-windup/contact mismatch and remote duplicate
swing need a focused presentation fix with recorded-frame acceptance. Broader
HUD, materials, lighting, animation, audio and encounter changes are not made
by this patch. Prioritized experiments and their exact source seams are in
the reference README.

## Integration

Work is isolated on `codex/diablo-reference-study-20260908`, based on
`2b5da07b1`, in `C:/Users/Alex/Documents/ChatGPT/verdigris-diablo-study`.
The architect checkout and preexisting Codex checkout were not switched or
cleaned. This milestone is committed locally, not pushed or merged into the
program branch. Locate its commit with:

```powershell
git log -1 --format=%H codex/diablo-reference-study-20260908
```
