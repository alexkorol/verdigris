---
id: TASK-0022
title: Browser 2.5D Phase 1 — camera + focus conformance (kill the mud)
state: READY
track: web-demo
priority: critical
base_commit: current program tip after 0019 integration (coordinator records the SHA)
dependencies: [TASK-0019]
parallel_safe: true
owned_paths:
  - src/core/rendering/**
  - src/components/GameCanvas.vue
forbidden_paths:
  - server/**
  - native/**
  - prototypes/**
  - src/core/chronicles/**
  - package.json
acceptance_commands:
  - npm run test:unit
  - npm run playtest
  - npm run smoke:browser
---

Suggested coordinator: **Kimi Code** (continuity with its accepted 0019
plan). First STATUS-write wins.

## Goal

Execute exactly Phase 1 of the governing plan
(`docs/25d-overhaul-plan.md` §7 "Phase 1 — Camera + focus conformance"),
burning down gap items 1, 2, 3, and 5 of §4: playfield DoF ≈ 0 at the
D-107 ARPG default, canvas-wide CSS grade removed at its source, neutral
terrain fetch with grading moved to the lighting pass, sprite shadowBlur
dropped in favor of foot ellipses.

## Why this task exists

Owner verdict: the current browser 2.5D "looks and plays awful… muddy,
heavy fog/blur, low contrast, washed-out tiles." The accepted 0019 plan
identifies the causes with file:line precision; Phase 1 removes the
biggest ones. D-108 makes `docs/reference/25d-overhaul/dist/
songs-of-the-mire.html` the acceptance look.

## Product and architectural invariants

- Follow the plan's §4 conformance targets and §6 camera integration
  verbatim; the reference ARCHITECTURE §8 solved-bugs list is binding.
- Renderer toggle stays functional (legacy path untouched and
  switchable).
- D-107: ARPG camera primary; close-zoom miniature blend is where blur
  may live.
- No gameplay/server changes.

## Scope

The plan's Phase-1 file-touch forecast. Where the plan and code disagree,
the plan's conformance TARGET governs and the deviation is documented in
REPORT.md.

## Non-goals

Phases 2–5 (terrain/horizon, lighting/atmosphere retune, DoF coupling,
perf), elevation content, asset changes.

## Deliverables

Rendering changes + before/after evidence, coherent commit(s).

## Acceptance criteria

- All three acceptance commands green (playtest 31/31 — note TASK-0020's
  revision is fixing an unrelated regression; base your work on a tip
  where playtest is green, or coordinate integration order).
- Before/after screenshots at the ARPG default over the same scene, plus
  a side-by-side against the reference demo's default view, committed to
  the task folder (lossy ≤250KB each).
- The canvas CSS filter is gone (or provably legacy-scoped) and DoF is
  ≈0 at default zoom (parameter dump or code cite in REPORT.md).

## Review focus

Visual delta against the reference, no legacy-renderer regression, §8
fidelity, and that grading moved rather than doubled.

## Stop conditions

Any Phase-2+ temptation (haze/horizon/atmosphere retunes beyond items
1/2/3/5) → stop at the Phase-1 boundary. Conflicts with TASK-0020's
in-revision server work are impossible by path, but if playtest is red on
your base for its reasons, say so in STATUS and proceed with unit+smoke
plus a note.
