---
task: TASK-0093
title: Native typography and text-rendering contract audit
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: b954e3ec
reviewed_at: 2026-08-23T20:10:00Z
revision: 1
---

# Review — TASK-0093 (native typography and text-rendering contract audit)

## Verdict: ACCEPTED

Frozen head `b954e3ec` (worker branch `worker/verdigris/pc/ox-pc-bc`),
content head `ccd876cf`, reviewed in detached worktree
`review-task0093-b954e3ec`.

## Scope

Worker-only delta `0a8aa40e..b954e3ec` touches only
`orchestration/tasks/TASK-0093-native-typography-contract-audit/**`
(FINDINGS.md, REPORT.md, STATUS.md, captures/coverage.json, 4 acceptance
transcript captures). No font downloaded/generated/licensed/selected; no
renderer chosen; read-only capsule honored. `git diff --check` clean.

## Acceptance gates

1. `rg -n "Text|Label|font|DrawText|text" native/client src/components src/assets src --glob ...`
   → 1635 lines, exit 0.
2. `rg -n "1920x1080|1366x768|typography|panel" orchestration/benchmarks orchestration/tasks/TASK-0079-browser-panel-inventory`
   → 13 lines, exit 0.
3. `git diff --check` → clean, exit 0.
4. `git diff --name-only` → owned additions only, exit 0.

## Evidence quality

- FINDINGS.md is excellent and comprehensive: primitive inventory (all text via
  `TextOutA` in main.cpp only), explicit-font sites, browser text-role
  inventory, resolution/DPI analysis, glyph-range requirements, wrapping/
  alignment/clipping/contrast current-vs-required tables, measured WCAG 2.1
  contrast ratios, offscreen-capture determinism, Windows/macOS backend needs,
  a clean backend-neutral text contract (roles, TextOp, Surface API with
  invariants I1-I6), proposed locking tests T1-T8, and owner-only choices.
- **Negative control verified genuine:** persistent chat log — browser renders
  a scrolling wrapped Chatbox (`Chatbox.vue:439-441,468,494`), but
  `render::Op` (render_list.hpp:16-45) carries no chat op and `ClientState`
  has no chat buffer/input seam; `rg Chat render_list.hpp` → no matches.
- **DPI claim verified:** `rg -i dpi native/client` → zero calls (exit 1); GDI
  text scales with system DPI virtualization, blurring on scaled displays.
- **Text isolation verified:** `rg -l "TextOutA|DrawText" native/client` →
  main.cpp only; all draws use `TextOutA` (ANSI codepage), confirming the
  latent em-dash/latin-1 mangling risk.
- Contrast analysis is measured from literal RGB pairs (quickbar-unavailable
  at 3.54:1 below AA; player damage ~3.45 worst-case over terrain with no
  outline). Proposed contract and tests preserve the existing
  measure→place→wrap→draw discipline and render-list determinism lock.
- Machine-readable twin `captures/coverage.json` parses.

## Capsule

Read-only audit respected: no font/renderer chosen, no code patched, no ports
bound, port 6500 untouched, only owned task-folder paths changed.

## Follow-up

Successor should implement the backend-neutral text contract (Roles + TextOp +
Surface API), add the locking tests T1-T8, and address the three concrete
defects the audit surfaced: quickbar-unavailable contrast, missing outline on
world-anchored text, and the ANSI-only pipeline. Font/backend/contrast-target
remain owner decisions.
