---
task: TASK-0118
title: Native accessibility, options, and input audit
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 6b87d49c
reviewed_at: 2026-08-23T21:45:00Z
revision: 1
---

# Review — TASK-0118 (native accessibility, options, and input audit)

## Verdict: ACCEPTED

Frozen head `6b87d49c` (worker branch `worker/verdigris/pc/ox-pc-bd`) reviewed
in detached worktree `review-task0118-6b87d49c`.

## Scope

Worker-only delta `fb69ba32..6b87d49c` touches only
`orchestration/tasks/TASK-0118-accessibility-options-audit/**`
(FINDINGS.md, REPORT.md, STATUS.md, captures/accessibility-matrix.json).
Read-only capsule honored (no ports, no settings mutation, D-007 preserved);
no option default chosen, no balance touched. `git diff --check` clean.

## Acceptance gates

1. `rg -n "rebind|keybind|sensitivity|focus|contrast|color|motion|flash|subtitle|caption|volume|scale|accessibility|setting" native/client native/tests src tests docs/product`
   → 1327 lines, exit 0.
2. `node -e "...accessibility-matrix.json...; console.log('accessibility matrix: PASS')"`
   → prints `accessibility matrix: PASS`, exit 0.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → owned additions only, exit 0.

## Evidence quality

- FINDINGS.md is excellent and comprehensive: covers all SPEC dimensions
  (key/mouse rebinding, hold/toggle, sensitivity, focus, text scale, contrast,
  color independence, reduced motion/flash, subtitles, audio, minimap/pane,
  persistence, reset, keyboard-only navigation, test coverage), each with
  current native state + browser reference + content-neutral gap contract.
- **Negative control verified genuine:** the elite attack telegraph is conveyed
  only by a translucent red cone/wedge (`draw_thrust_telegraph`,
  main.cpp:1519-1543) with no text label, no legend entry, no audio cue, no
  colorblind-safe alternative. Verified: `AudioMixer` not referenced in
  main.cpp (exit 1), so the client plays no audio; `beat_legend` is populated
  only in the capture scenario. A red-deficient player receives dodge-critical
  info solely through red hue + geometry.
- **Key library seams verified unwired:** `input_focus.hpp` (pure focus
  reducer) has zero integration with main.cpp (exit 1); `AudioMixer` (bus
  volume/mute) exists and is unit-tested but wired into neither main.cpp nor
  any UI.
- Also verified: native has no settings UI, no options persistence (only
  capture artifacts), no minimap second mode, and the largest product gap is
  the single fixed minimap vs the constitution's two modes with
  transparency/zoom/placement options.
- The standards-first continuation path (wire input_focus, port browser
  binding-map, add options surface with persistence/reset, extend scenario
  ladder, caption seam when audio arrives) is concrete and content-neutral.
- Machine-readable twin `captures/accessibility-matrix.json` parses.

## Capsule

Read-only audit respected throughout: no ports, no settings mutation, no balance
change, D-007 preserved, port 6500 untouched, only owned task-folder paths
changed.

## Follow-up

Successor should start with wiring `input_focus.hpp` and the audio mixer into
main.cpp, then port the browser binding-map mechanics, then add an options
surface with persistence/reset (volume, text scale, sensitivity, telegraph/
minimap redundancy, reduced-flash). Final defaults remain owner play verdicts.
