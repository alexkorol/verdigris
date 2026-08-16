# TASK-0019 report — Phase 0 survey for the browser 2.5D overhaul

## Executive summary

Surveyed the browser client against the vendored reference
(`docs/reference/25d-overhaul/`) and rewrote `docs/25d-overhaul-plan.md` as
rev 2. Key framing finding: the brief's phases 1–5 already have a landed first
pass (the `src/core/rendering/perspective-*` stack implements the reference
camera, GL terrain, lightmap, and DoF), so the plan re-scopes phases 1–5 as
conformance/remediation passes. The "muddy" look is root-caused to six named
sources with file/line evidence (plan §4), led by non-zero playfield DoF
(`perspective-camera.js:56-60`), a compensating canvas-wide CSS filter
(`GameCanvas.vue:634-639`), and terrain-fetch grading
(`terrain-renderer.js:124-135`). Elevation decision: keep flat `h = 0`,
formalize the existing `heightAt` seam as the single contract (plan §5).
Camera plan translates D-107 into the browser's HOR/FOCUS/zoom/dofStrength
knob space with the miniature treatment as a close-zoom blend (plan §6).

## Approach

Read the reference `ARCHITECTURE.md` in full (§8 treated as binding), the
brief's phases 1–5, D-106..D-108, and the TASK-0012 camera evidence pack
(ARPG preset values cited in the plan). Surveyed the client render loop,
perspective stack, map/tile format, picking, and toggle infrastructure with
file/line evidence; personally re-verified the load-bearing citations for the
gap analysis (`renderer-mode.js`, `perspective-camera.js`,
`terrain-renderer.js:115-136`, `GameCanvas.vue:630-641`,
`perspective-renderer.js:484-501`).

## Changed files

- `docs/25d-overhaul-plan.md` — rewritten (the task's sole owned path).
- `orchestration/tasks/TASK-0019-browser-25d-survey/STATUS.md`,
  `REPORT.md` — protocol bookkeeping.

## Public interfaces

None (plan document only; no code changes).

## Verification

- Acceptance commands: none in the spec (survey task).
- `git status` shows only the plan file plus orchestration bookkeeping.
- Plan covers every spec scope item: engine/stack findings, render-loop entry
  points with file/line refs, coordinate/depth/anchor concept mapping, camera
  integration plan, named elevation decision, phase 1–5 file-touch forecast,
  risks, feature-flag design.

## Deviations

None from the spec. One framing observation for the architect: the brief's
phase language ("implement the projection…") predates the landed perspective
stack; the plan keeps the brief's phase numbering but redefines each phase's
scope as conformance. If the architect would rather have greenfield-style
phase specs, say so in review and the plan can be re-cut.

## Risks / follow-ups

Listed in plan §9. The biggest: D-107's slice camera scalars and the browser's
HOR/FOCUS parameterization are different models of the same feel; the §6
mapping needs visual validation in phase 1, and the plan proposes a
non-persisted `?camera=` debug override for that A/B loop.
