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

## Wave 3 accepted + integrated; wave 4 (2026-08-16, ~10:15)

- TASK-0007 (`e7505ad` → integrated `a832b2b`) and TASK-0008 (`56ac8f0` →
  integrated `e7c6d28`) ACCEPTED after independent verification;
  integration tip `0c51439` merged here and gates rerun green.
- LEGACY_MATRIX revised with the audit's file-level provenance addendum
  (architect debt cleared). Remaining debt: ADR-002 persistence seam.
- TASK-0009 client Q/E/R bindings — promoted **READY**, base `0c51439`
  (sole in-flight client task).
- Noted for a future core task: a real facing field (Thrust is +x-only).

In flight: TASK-0009 only. Next after it: facing-field core task, then
ADR-002.

## TASK-0009 accepted; TASK-0010 issued (2026-08-16, ~10:40)

- TASK-0009 (`629a1c0`) ACCEPTED — verified live: skill strip states,
  WarCry aura + tick countdown, resource drain/regen on HUD. Watch item
  logged: client display-costs mirror core constants by value; a future
  snapshot/query seam should own them.
- TASK-0010 actor facing (core+client, sequential, sole in-flight) —
  **READY**, base = 0009 integration tip (Codex records SHA).
- Architect debt remaining: ADR-002 persistence/serialization seam draft.

## TASK-0010 accepted; wave 6 issued (2026-08-16, ~11:05)

- TASK-0010 (`7e066aa`) ACCEPTED — 8-way integer facing, AimIntent
  appended, strict half-plane Thrust cone, monster pursuit facing, replay
  determinism; gates independently rerun green.
- Wave 6: TASK-0011 monster skill AI with `AttackTelegraphed` windups
  (core; READY) and TASK-0012 camera-preset evidence pack (task-folder
  only; READY) — disjoint. DRAFT idea queued: skill-cost query seam
  (0009 watch item) after 0011.
- Architect debt: ADR-002 draft next quiet cycle.

## Wave 6 reviewed; ADR-002 drafted (2026-08-16, ~11:40)

- TASK-0011 (`8c68aed`) **ACCEPTED** — elite Thrust/Sweep bands via the
  shared resolver, honest `AttackTelegraphed`, thorough cancellation;
  gates independently green. Natural follow-up: client telegraph
  rendering.
- TASK-0012 (`310b76d`) **REVISE** — methodology exemplary, but the nine
  ~1.4MB PNGs (~12.5MB) must be re-encoded lossy (≤250KB each) before
  integration; one numbered correction.
- `docs/rebuild/ADR-002-persistence-seam.md` drafted, status PROPOSED —
  snapshot-based persistence with RNG state, mid-instance non-durability
  as the honest extraction-risk reading, flat-file v1, SQLite deferred.
  Owner decisions flagged inside.
- Owner morning-review queue: camera evidence pack (after 0012 rev),
  ADR-002 proposal, D-101/D-102/D-103 provisional decisions, and the
  0006 unequipped-relic observation.

# OVERNIGHT SUMMARY FOR THE OWNER (2026-08-16, session end ~12:45)

Thirteen tasks specced, twelve implemented and ACCEPTED after independent
verification, all integrated (TASK-0013 integration pending Codex's next
pass). The queue is deliberately EMPTY for your review. What the native
lab can now do, none of which existed yesterday morning:

- Bounded deterministic **Legends** history (0001); **relic re-entry**
  closing death→loss→rediscovery (0006).
- **Skill actions** Thrust/Sweep/WarCry with a live resource economy
  (0007), bound to Q/E/R with HUD (0009); real **8-way facing** with mouse
  aim as a simulation input (0010); **elite monsters using the same
  skills** with telegraphed windups (0011) rendered as honest ground
  warnings in the client (0013).
- **Build/CI hardened** (0002: vswhere discovery, define guard, presets
  v2, native.yml) and the **denylist firewall hardened** against the
  evasion classes the archaeology audit proved (0005, 0008).
- The **founding-slice** has its own one-command verification harness
  (0003) and produced a **camera-preset evidence pack** for your D-102
  judgment (0012, compressed, in the task folder).
- Architect docs: LEGACY_MATRIX provenance addendum, **ADR-002
  persistence seam (PROPOSED — your call)**, DECISIONS.md D-001..D-104.

Decisions awaiting YOU (nothing blocked on them overnight):
1. ADR-002 persistence proposal (accept/amend).
2. D-101/D-102/D-103 provisional calls (player base-life, camera
   envelope via the 0012 evidence pack, slice banking).
3. The 0006 observation: should notable *carried* (unequipped) relics
   also re-register on death, or stay lost?
4. Owner-only items D-O1..D-O5 (seasons, assets, magic/WIZARD lattice,
   naming/lore, economy) — untouched, as required.

Suggested next waves when you resume: snapshot/query seam (fixes the
0009/0013 display-constant drift watch items and starts ADR-002 if
accepted), campaign-graph deepening (needs your lore input to avoid
invented canon), and the slice→native billboard-renderer experiment
(Milestone E follow-on).

## Watch items

- Enemy melee cadence in the slice felt lethal for an idle level-1; base
  player life raised to 52. Feel data, not canon.
- The slice's "bank on node completion" simplification weakens the
  extraction-risk fantasy relative to the constitution; acceptable for the
  demo, must NOT leak into native specs.
- Asset plates: downscaled magenta-keyed derivatives are vendored under
  prototypes only. Full-resolution provenance/packaging for native remains
  an owner decision (D-O2).
