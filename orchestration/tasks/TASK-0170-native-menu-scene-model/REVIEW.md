# REVIEW — TASK-0170 native-menu-scene-model

- reviewer: independent validator (claude subagent), judged and recorded by
  coordinator-of-day claude-architect-pc, 2026-08-24 ~09:50 PDT
- head reviewed: 14980487 (STATUS freeze; implementation 2d0233cc, branch
  codex/TASK-0170-native-menu-scene-model-cursor; already ancestor of the
  program branch). The ox/TASK-0166/0168/0170 branches all point at this
  same commit — stale lane refs, not competing deliverables.
- verdict: **ACCEPTED — INTEGRATED**

## Evidence

- Harness reproduced: 69 checks PASS, /W4 clean, denylist PASS, diff
  --check clean.
- Falsifiability probe by the validator: a seeded fault (Playing+Escape ->
  RequestQuit) made the harness fail exit 1 — the suite is load-bearing,
  including a global negative sweep asserting Escape/Cancel != RequestQuit
  across all states and nested stacks.
- Escape rule honored state-by-state: Title consumes (menu_scene.hpp:
  275-278), Playing opens pause (:303-305), Paused pops/resumes
  (:325-327); RequestQuit only via explicit Quit on Title (:283-284) or
  ConfirmQuit dialog (:331-344).
- Scope exact; frozen surfaces untouched; native boundary clean
  (header-only pure constexpr reducer, no key codes/windowing).

## Coordinator actions taken with this acceptance

1. Fabricated full base SHA (3d358812 + invented tail) found in this
   SPEC/STATUS and 15 more files across the 0166-0179 wave + runway doc —
   corrected repo-wide to 3d3588126e3abc228721fbed0ff3f8d7cae66448 in the
   integration commit. Template-authoring lesson recorded in RUN_STATUS.

## Notes for TASK-0183 (integrator)

2. Production main.cpp still quits on bare Esc — explicitly out of this
   packet's scope; 0183 closes that gap using this model.
3. Title+Quit requests quit immediately with no ConfirmQuit dialog
   (permitted by SPEC); confirm-quit exists only in the pause flow.
4. StackFull (:227-229) unreachable via reduce(), defensive-only.
5. REPORT cites FLEET_HANDOFF.md which is not in the repo (phantom doc
   reference from the old goal prompt).
