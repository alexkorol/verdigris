---
task: TASK-0115
title: Browser panel and typography inventory
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 16816293
reviewed_at: 2026-08-23T22:05:00Z
revision: 1
---

# Review — TASK-0115 (browser panel and typography inventory)

## Verdict: ACCEPTED

Frozen head `16816293` (worker branch `worker/verdigris/pc/ox-pc-bc`), evidence
head `086cefa4`, reviewed in detached worktree `review-task0115-16816293`.

## Scope

Worker-only delta `bd3bde95..16816293` touches only
`orchestration/tasks/TASK-0115-browser-panel-typography-inventory/**`
(FINDINGS.md, REPORT.md, STATUS.md, captures/panels.json, capture driver +
38 panel PNGs + evidence/negative-control/run logs + rg sweeps). No code,
redesign, font selection, or native work; browser behavior unchanged.
Read-only capsule honored (one disposable loopback port 6620, 6500 untouched).
`git diff --check` clean.

## Acceptance gates

1. `rg -n "font-family|font-size|font-weight|color" src --glob "*.vue" --glob "*.css"` → 827 lines, exit 0.
2. `rg -n "component|panel|pane|chat|quest|expedition|guide|context" src/components --glob "*.vue"` → 626 lines, exit 0.
3. `node -e "...panels.json...; console.log('panel inventory: PASS')"` → prints
   `panel inventory: PASS`, exit 0. **19 panels** with full per-row metadata.
4. `git diff --check` → clean, exit 0.
5. `git diff --name-only` → owned additions only, exit 0.

## Evidence quality

- FINDINGS.md is excellent: **19 persistent/situational panels captured GREEN**
  at both 1920×1080 and 1366×768 (38 PNGs), each with measured bounding box,
  trigger, anchor, mounted Vue path, font family/size/weight/color, gameplay
  load, and native phasing rank. The typography contract is frozen with the
  fluid clamp (16px @1920 vs 15.026px @1366), the GameFont/ChatFont/UIFont
  roles, and 5 deviations worth freezing before native porting (Georgia serif
  "moment" overlays, orphaned UIFont, 12px pins).
- **Negative control verified genuine:** `captures/capture-0115-negative-control.log`
  shows `CAPTURE FAILED: negative-control.false-visibility-inventory` with a
  nonzero exit (injected false visibility assertion against a disposable output
  path), satisfying SPEC's requirement. Disposable output removed afterward.
- Capture evidence protocol is sound (hard-fail helper, `boxOf` null-box proof,
  real production paths via hotkeys/nav/bus hooks, `CAPTURES OK` 38/38 exit 0);
  context-menu note about Shift+RMB D-007 preservation is honest.
- Exclusions correctly cited (legacy game-panes, auth surfaces, tooltips).
- Native phasing proposal is an ordering recommendation only; font selection
  correctly reserved to owner.
- Machine-readable twin `captures/panels.json` parses (19 panels).

## Capsule

Read-only audit respected: one disposable loopback port (6620), port 6500
untouched, no code/font/behavior changes, only owned task-folder paths changed.

## Follow-up

Use the 19-row phasing rank + frozen typography contract as the native panel
reconstitution order; final font selection remains owner-only.
