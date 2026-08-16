# Architect state — 2026-08-15

## Program shape

Two source briefs are reconciled into two tracks with disjoint paths:

1. **Native reconstruction (primary)** — `native/`. C++20, dependency-free
   deterministic core (fixed timestep, command/event boundary), Win32
   placeholder client. Milestones A–E of the reconstitution brief are
   implemented, committed, and verified on Windows (MSVC build, 11 core
   tests, denylist gate, headless loop, driven-input window pass).
   Key fix this session: `build.ps1` never defined `VERDIGRIS_NATIVE_WINDOWS`,
   so the windowed client had silently been the console fallback.
2. **Front-end feel prototype (secondary)** — `prototypes/founding-slice/`.
   Self-contained browser slice ("Founding of a House") answering camera,
   billboard, combat-feel, drop, House-founding, and graph-presentation
   questions. Verified end to end (intro → crisis → combat → death → relic →
   successor → node clears → founding) via browser automation + Playwright
   captures. It is a laboratory, NOT production architecture: its stat
   formulas, item tables, and node graph are demo conveniences, not canon.

## Reconciliation of the two briefs

- Shared requirements (Houses/Scions, actor symmetry, extraction risk, item
  identity/history, relics, instance graph, Bronze Age identity, WASD+mouse
  controls, procedural effects) are canonical in
  `docs/product/VERDIGRIS_CONSTITUTION.md`; both tracks serve them.
- The vertical-slice brief's "fresh project, assume nothing" framing is
  resolved by isolation: the slice lives entirely under
  `prototypes/founding-slice/` and may not reach into `native/`, `src/`,
  or `server/`.
- Duplicated loop implementations are intentional: the native core is the
  system of record; the slice exists to answer feel questions cheaply.
  Findings flow slice → decisions → native specs, never as code ports.
- The web demo must not redefine native architecture; the native track must
  not stall on abstractions before a playable loop (it already has one).

## Current baseline

- Branch `codex/native-reconstitution` at `f5b4b72`, clean, ahead of
  origin/master by the milestone commits (local; owner pushes).
- Passing: `./native/build.ps1 -RunTests -RunClient` (build, denylist,
  11 core tests, headless loop). Slice builds via
  `node prototypes/founding-slice/build.mjs`.
- Historical browser game untouched and still gated by `npm run playtest`.
- WIP from another stream preserved on `codex/recover-merge-refinements`
  (commit `5f829ac`, identity-services work) — not part of this program.

## Wave 1 (issued now)

- TASK-0001 native Legends slice — READY
- TASK-0002 build/CI hardening — READY
- TASK-0003 slice verification harness — READY
- TASK-0004 native client direct-control pass — DRAFT (depends on D-007,
  now decided, and reviews of wave 1; promoted next wave)
- TASK-0005 legacy archaeology audit — DRAFT (research; promoted when a
  Luna reader is free)

Paths are disjoint across the three READY tasks (core sources / build+CI
files / prototype folder). See each SPEC for exact ownership.

## Wave 1 review (2026-08-16 morning)

Codex coordinator runs from a separate clone at
`C:\Users\Alex\Documents\ChatGPT\verdigris` with per-task worktrees under
`.codex/worktrees/`; work is exchanged via origin pushes.

- TASK-0001 Legends records — **ACCEPTED** (`7ed844d`; diff reviewed, tests
  independently rerun green). Codex may integrate.
- TASK-0002 build/CI — **REVISE** (rev 1): QUESTION-0001 answered with
  D-104 — presets schema v2 (CMake 3.20-compatible); everything else in the
  original scope stands.
- TASK-0003 slice harness — **ACCEPTED** (`278f7dd`+`e25336d`; harness
  independently rerun, 4/4 green). Codex may integrate.
- TASK-0004 client direct-control — stays DRAFT; promotes once wave-1
  integration lands on origin (base_commit = integrated tip) and the
  TASK-0002 define guard exists.
- TASK-0005 legacy archaeology audit — promoted to **READY** (read-only,
  base_commit 9eadfbd).

In flight after this pass: 0002 revision + 0005 (disjoint: build/CI files
vs report-only). Integration of accepted work is Codex-owned; after it
lands, the next architect pass promotes TASK-0004 and specs a relic
re-entry core task to exercise the dormant `relic_extracted` path.

## Watch items

- Enemy melee cadence in the slice felt lethal for an idle level-1; base
  player life raised to 52. Feel data, not canon.
- The slice's "bank on node completion" simplification weakens the
  extraction-risk fantasy relative to the constitution; acceptable for the
  demo, must NOT leak into native specs.
- Asset plates: downscaled magenta-keyed derivatives are vendored under
  prototypes only. Full-resolution provenance/packaging for native remains
  an owner decision (D-O2).
