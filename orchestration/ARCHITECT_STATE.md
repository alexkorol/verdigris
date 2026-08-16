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

## Wave 2 (2026-08-16, after wave-1 integration merge at bc73ce0)

Wave-1 integrations (Legends `5487778`, harness `8517bf5`+`403e3a3`) merged
into the architect checkout and verified green (denylist + core tests).
Review pass 2 results:

- TASK-0005 archaeology audit — **ACCEPTED** (`ff2ea30`); its report is the
  standing legacy evidence base. A LEGACY_MATRIX revision and a
  denylist-hardening tooling task will be specced from it in a later wave.
- TASK-0002 — **REVISE rev 2** (verified: presets v2 validate on bundled
  CMake 3.20, define guard works). Two corrections: (5) drop the
  `Visual Studio 16 2019` generator pin that would fail on windows-latest,
  (6) actually suppress the vcvars-internal vswhere noise (item 5 of the
  original scope).
- TASK-0004 client direct-control — promoted **READY**, base `bc73ce0`.
- TASK-0006 relic re-entry — new, **READY**, base `bc73ce0` (core sources;
  closes death→loss→rediscovery per D-004, exercises `relic_extracted`).

In flight: 0002-rev2 (build files), 0004 (client), 0006 (core) — three
READY tasks with disjoint ownership.

## Wave 2 review complete (2026-08-16, ~09:30)

All three wave-2 tasks **ACCEPTED** after independent verification:

- TASK-0002 (rev 2, `f9c979b`): generator pin replaced with NMake +
  `msvc-dev-cmd` in CI; vswhere noise eliminated via PATH injection
  (verified clean run); presets v2 validate on bundled CMake 3.20.
- TASK-0004 (`7ac51d4`): D-007 contract verified end to end with a driven
  PostMessage pass — X pickup, I-overlay equip, F extraction (stored 1/1,
  carried 0), Z labels, Q/E/R disabled stubs.
- TASK-0006 (`f542a04`): relic re-entry verified; death→loss→rediscovery
  loop closed. Logged observation: unequipped carried relics are lost
  forever at death (baseline rule) — future Brands & Bonds question.

## Wave 3 (issued 2026-08-16)

- TASK-0007 core skill actions (Thrust/Sweep/WarCry + resource economy) —
  **READY**, base = wave-2 integration tip (Codex records SHA).
- TASK-0008 denylist hardening from the 0005 audit's §4 gap evidence —
  **READY**, same base convention. Disjoint paths from 0007.
- TASK-0009 client Q/E/R bindings — DRAFT, promotes after 0007.

Architect debt (mine, not delegable): revise `docs/rebuild/LEGACY_MATRIX.md`
from the 0005 report; draft ADR-002 (serialization/persistence seam) when
the loop stabilizes.

## Watch items

- Enemy melee cadence in the slice felt lethal for an idle level-1; base
  player life raised to 52. Feel data, not canon.
- The slice's "bank on node completion" simplification weakens the
  extraction-risk fantasy relative to the constitution; acceptable for the
  demo, must NOT leak into native specs.
- Asset plates: downscaled magenta-keyed derivatives are vendored under
  prototypes only. Full-resolution provenance/packaging for native remains
  an owner decision (D-O2).
