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

# THE MISSION, OWNER-RULED (D-116, ~23:10): C++ PARITY OR BETTER

The orchestration's purpose is the C++ conversion — native at the web
version's level or better, with multi-layer regression sweeps. Strategy:
the C++ server speaks the existing WS protocol so the unchanged client
and the unchanged 31-scenario playtest harness become the parity bar.
Roadmap: `docs/rebuild/PARITY_ROADMAP.md` (waves N1–N7).

- **TASK-0039 (critical, READY): parity wave N1** — C++ protocol server
  passing `quickstart` + `single-session` against the unchanged harness;
  includes ADR-003 (networking lib) draft for architect ratification.
- The browser playability wave (0032–0038) CONTINUES — it raises the
  reference bar; both lanes run in parallel with disjoint paths.
- Sweep layers standing: protocol dual-run matrix, UI pane sweeps, core
  determinism replays, D-115 play gate.

# OWNER VERDICT ON THE NATIVE EXE + PROCESS CORRECTIONS (~22:45)

The owner played the exe: "a total mess." Root causes confirmed and
owned by the architect: (1) per-task green reviews with NO whole-product
play gate; (2) the movement rescale left all combat ranges incoherent
(D-114 born from this); (3) debug scaffolding presented as game UI;
(4) no art-direction ruling so scale/style drifted; (5) effort split
across two "playable" surfaces so neither reached quality.

New process rules now binding: **D-114 feel-coherence tables** and
**D-115 architect play gate**. Product rulings: **D-112 the browser game
is THE product; the exe is demoted to core testbed** (standing architect
recommendation unless the owner inverts), **D-113 webchat/vector-first
art with a scale chart**.

Critical path unchanged in spirit, updated in content:
1. TASK-0034 playability evaluation (browser) — now seeded with the
   owner's exe findings (OWNER-SEED.md in the task folder).
2. TASK-0032 death/disconnect rules (in flight).
3. TASK-0035 native exe triage (range table + debug UI behind F3) —
   replaces further native feature work entirely.
4. TASK-0033 daytime default.
Everything else remains HELD.

# PRIORITY RESET — PLAYABLE-FIRST (owner-directed, 2026-08-16 ~22:15)

The owner's directive: "I want to have a playable game." Recorded as
**D-110**: play-session friction outranks all feature/infra/renderer
work from here. New critical path:

1. **TASK-0034 (critical)** — actually PLAY the game for a session and
   produce the ranked friction inventory. This document sets the next
   wave. Suggested: kimi.
2. **TASK-0032 (critical, in flight)** — death/disconnect fairness IS
   playability; continues.
3. **TASK-0033 (high)** — boot in permanent daytime; cycle behind an
   off-by-default toggle (D-111) until the owner rules on it.

Everything else HOLDS until the 0034 friction list exists. Do not spec
or claim new feature work past these three.

# KIMI RETURN BRIEF (2026-08-16 ~21:40 — quota restored)

Welcome back, kimi and kimi-work. While you were out: your 0027 claim was
released per protocol and completed by codex USING YOUR QUESTION-0005
analysis — the NaN theory was proven correct and your investigation is
credited in the accepted review. The mud-kill plan you surveyed (0019) is
now COMPLETE and shipped. Do not resume 0017/0024/0027 — all closed.

Claimable RIGHT NOW (first STATUS-write wins, pull first):
- **TASK-0031** browser D-106 death/relic audit — read-only deep-read,
  ideal for kimi (same shape as your excellent 0019 survey).
- **TASK-0030** native persistence (C++ snapshot/restore per ADR-002).
Check `orchestration/tasks/*/SPEC.md` state==READY with no STATUS.md.

# MUD-KILL COMPLETE + EVENING LEDGER (2026-08-16 ~21:20)

**The 25D overhaul plan is complete — all five phases accepted and
shipped to master**, with real defects found and fixed along the way
(capture-clock NaN killing night orbs, grade double-darkening, dash
tunneling through scenery). Live on master tonight: 2.5D Phases 1–5,
movement-feel fix, expedition-boundary enforcement, pack-clear rule,
D-109 disconnect safety, D-106 death recoverability, native scenery
billboards + catalog single-sourcing, hardened CI/build/denylist.

Next frontier issued: TASK-0030 native persistence (ADR-002/D-109
snapshot+restore with RNG state) and TASK-0031 browser death/relic audit
vs D-106 (audit-first before touching server gameplay).

**Flagged for the owner (not agent-decidable): campaign/instance-graph
deepening needs your lore input** — the mechanical graph exists in both
native and browser, but node names, places, and branch meanings are
yours. A short voice-note's worth of direction unlocks a big wave.

# OWNER PLAYTEST GUIDE (2026-08-16 evening)

**Browser game** (the ship target):
```
npm run build
npm run smoke:browser     # proves the built game boots + plays the guest loop
```
or run it persistently: `pm2 start ecosystem.config.cjs` then open
http://localhost:6500 (client + WS on one process).

**Native lab client**: `powershell -File native/build.ps1 -RunClient`
builds and smoke-runs; then launch `native/build/verdigris_client.exe`
for the window — WASD/mouse, Q/E/R skills, X pickup, I gear, F extract,
your ARPG camera, billboards from your plates, elite telegraphs, and the
fixed movement pace.

**Founding slice**: the claude.ai artifact link, or serve
`prototypes/founding-slice/` statically and open index.html.

**Shipped to your evening (since PR #4)**: movement fix (verified ~2.2
tiles/s), 2.5D Phases 1–2 (blur/wash/haze fixed), D-109 disconnect
safety (race-fixed, 31/31), D-106 death recoverability, expedition
boundary enforcement + pack-clear fix (bot-review findings), CI path
fix, ARPG camera in slice + native. In revision: Phase-3 atmosphere
(sent back — it over-darkened midday; numeric luminance bar set).

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

Post-summary addendum (~13:00): TASK-0013 integrated by Codex; merged and
gates rerun green. One exception to the queue hold: Codex's
QUESTION-0003 exposed that the documented `smoke:browser` gate never
boots its server — answered as D-105 (adopt the test:e2e lifecycle) and
issued as tiny TASK-0014 (package.json script line only), since a broken
documented gate should not greet your morning.

## Owner active again; hold lifted (2026-08-16, ~13:35)

- TASK-0014 **ACCEPTED** (`b068964`) — smoke:browser verified end to end
  (1/1 browser spec green, port 6500 released). QUESTION-0003 closable.
- Queue hold lifted at the owner's prompt. New wave:
  - TASK-0015 core presentation catalog (single-source constants for
    costs/ranges/telegraphs — kills the 0009/0013 drift watch items) —
    **READY** (core).
  - TASK-0016 native billboard experiment (client renders the vendored
    keyed plates read-only, capsule fallback; slice→native visual
    bridge, D-O2 untouched) — **READY** (client). Disjoint pair.
- Still with the owner: ADR-002, D-101..103, carried-relic question,
  D-O1..D-O5, and the camera evidence pack review.

## Owner directives received (2026-08-16, ~13:40) — priority reset

Owner rulings recorded: **D-106** (death never destroys items — all
carried items recoverable), **D-107** (ARPG camera primary; Miniature
treatment at close zoom; High Table rejected), **D-108** (the vendored
webchat demo at `docs/reference/25d-overhaul/` is the look/feel
acceptance target; its HANDOFF brief targets the browser game as the
near-term shippable). Owner reports the native client movement is
fast/jumpy — root cause confirmed (full-tile displacement per 50ms
MoveIntent) — and **Kimi Code joins as a second implementation
coordinator** (PROTOCOL amended: coordinator identity + first-claim-wins).

Ship pressure is explicit. Queue now:

- In flight: TASK-0015 (core catalog), TASK-0016 (billboards).
- TASK-0017 movement feel + D-107 camera defaults — **READY** (critical;
  after 0015+0016).
- TASK-0018 death recoverability per D-106 — **READY** (after 0015).
- TASK-0019 browser 2.5D Phase-0 survey per the reference brief —
  **READY** (critical; suggested for Kimi; plan doc only).

Next after these: browser overhaul phases 1+ (from the 0019 plan), native
scenery billboards, client catalog adoption.

## BOARD ATTENTION (2026-08-16 ~16:15)

**TASK-0017 (native movement feel fix, CRITICAL) has been unclaimed for
~2 hours** while the browser track advances. The owner personally
reported this bug (character "moves very very fast, jumping around").
Codex: with 8 workers available, claim 0017 in parallel with the browser
phases — its paths (native core+client) are disjoint from everything in
flight. Kimi: 0024 Phase-3 or 0017 are both open to you; first
STATUS-write wins.

Progress ledger today: Phases 1–2 of the mud-kill ACCEPTED and
integrated; D-109 disconnect safety live; 0016 billboards, 0018 death
recoverability, 0021 slice camera all integrated.

## Queue widened; D-109 recorded (2026-08-16, ~14:05)

- Owner: Codex/Luna can run up to 8 workers — PROTOCOL cap widened to 8
  for unquestionably disjoint tasks.
- **D-109 (owner-ruled)**: forgiving persistence — logout/disconnect/crash
  keeps everything, returns the character to town; death is the only loss
  event; disconnect deaths must be prevented. ADR-002 ACCEPTED as
  amended.
- Kimi onboarding written to `orchestration/ONBOARDING-KIMI.md` (owner
  communicates with Kimi via repo files); AGENTS.md points coordinators
  at the protocol.
- New READY (disjoint): TASK-0020 browser disconnect-safety per D-109
  (server/** — critical, ship path) and TASK-0021 slice ARPG camera
  default per D-107 (slice files only).
- Claimable board now: 0016 (client, in flight), 0018 (core, after 0015),
  0017 (core+client, after 0016+0018 chain), 0019 (survey, Kimi),
  0020 (server), 0021 (slice). Core-file tasks remain strictly
  sequential via dependencies.

## Watch items

- Enemy melee cadence in the slice felt lethal for an idle level-1; base
  player life raised to 52. Feel data, not canon.
- The slice's "bank on node completion" simplification weakens the
  extraction-risk fantasy relative to the constitution; acceptable for the
  demo, must NOT leak into native specs.
- Asset plates: downscaled magenta-keyed derivatives are vendored under
  prototypes only. Full-resolution provenance/packaging for native remains
  an owner decision (D-O2).

## BOARD NUDGE (2026-08-17 ~06:45)

0043's ten-run proof is inherently serial, but it does NOT need the
whole fleet. TASK-0044 (parity wave N2 — the mission's critical path),
TASK-0038 (rebinding), and TASK-0042 (first-loot) are claimable NOW with
disjoint paths from 0043 (playtest/** vs native/** vs src+server loot
paths vs input/settings). Codex: parallelize. Kimi instances: any of the
three is yours if you claim first.

## 0043 REVIEW (2026-08-17 ~08:20)

REVISE. Ten-run loaded proof checked out, but architect's own
default-mode (no flag) full run at the merged tip failed session-arc
30/31 under ambient 122ms lag; solo rerun passed. Correction: adaptive
guard must key off measured lag, not the env flag; proof = 3 consecutive
default-mode full runs green + architect rerun. Harness-only changes
provisionally merged to program branch (documented in REVIEW.md) to
avoid revert-of-merge hazard; task stays open. Board otherwise: 0044
(N2, critical) / 0038 / 0042 still unclaimed - Kimis re-entering.

## 0043 ACCEPTED + INTEGRATED (2026-08-17 ~09:05)

Rev1 verified: lag-keyed default-mode guard; coordinator 3x default full
runs green; architect gate 31/31 at 130ms peak ambient lag. Integrated
at 1f470e3; shipping to master next. One 10/31 architect run between
rounds was Windows Firewall consent dialogs blocking fresh server
binaries (owner clicked Allow) - environmental, documented in REVIEW.

## STANDING GUIDANCE - loopback binds (2026-08-17)

All test/dev servers (playtest server/index.js boot, native
verdigris_server, any harness listener) should bind 127.0.0.1
explicitly, not 0.0.0.0. Loopback binds never trigger Windows Firewall
consent prompts; wildcard binds prompt once per new binary, which
stalls sockets until a human clicks Allow (this collapsed a full
playtest run today, and every fresh C++ server build will re-prompt).
Fold into 0044 implementation (native server default bind) and the next
harness touch. LAN play later can make the bind address a config knob.

## BOARD (post-0043)

0044 parity N2: CLAIMED by kimi-work (branch codex/TASK-0044-native-protocol-n2,
base 32d7b6e, own clone KimiWork). 0038 rebinding + 0042 first-loot:
still open - kimi (console) or codex/Luna may claim. 0043: done.

## 0044 ACCEPTED + INTEGRATED (2026-08-17 ~09:30)

Parity wave N2 landed: world/movement/zones over the C++ server.
Architect rebuilt the branch, ran native gates green, and attached the
UNCHANGED post-0043 harness to my own server build: movement + zones +
quickstart + single-session 4/4. Movement constants literally mirror
server/shared/movement.js. Honest 8-item stub inventory carried into
the N3 spec notes (real generator, town tiles, depth>1, monsters as
Simulation actors, stdin idle fix). kimi-work delivered the mission
critical path within ~90 min of re-entry. Shipping 0043+0044 to master.

Board: 0038 (LMB/RMB + rebinding) and 0042 (first-loot moment) remain
open for kimi (console) or codex/Luna. N3 spec is the architect's next
authoring task.

## N3 SHIPPED + BOARD (2026-08-17 ~18:10)

0045 ACCEPTED+INTEGRATED, shipped to master as PR #17 (master 86688c1f).
Architect rebuilt and reran 7/7 attach incl combat/encounter-variety/
boss-mechanic. C++ parity now covers N1 transport/login, N2 world/
movement/zones, N3 combat/packs/boss. Next rung: TASK-0047 N4
items/inventory/Vesselforge (spec READY, critical) - closes N3 depth
stub, graduates drops to real items, 13/13 attach bar. 0046 REVISE
stands (re-request added no arcs). In flight: 0038 kimi-work, 0042
kimi. Reviewer ops note: stop detached review servers via the task
runner (TaskStop), not process kills.

## EVENING SWEEP (2026-08-17 ~18:50)

- 0038 ACCEPTED + SHIPPED (PR #18, master d687e004): LMB/RMB attacks +
  six-action rebinding UI with persistence. Architect verified real
  captures + WS frame log, reran unit 788/788 + playtest 31/31.
- 0046 rev2 ACCEPTED: two played arcs with per-arc wire proofs + full
  disposition table. Supersedes 0034 as the "playable" definition.
  Key: guest loop WORKS (3 kills, XP, gold); B2 zone-bounce FIXED.
- NEW CRITICAL: TASK-0048 Chronicles silent combat (mortal-oath scion
  cannot connect a single hit; guest path fine on same build). Specs
  a real-path diagnosis + a Chronicles-login playtest scenario.
- 0047 N4 items/inventory CLAIMED by kimi-work (base 05c3f46).
- Board: 0048 open (codex free - claim it), 0042 in flight (kimi),
  0047 in flight (kimi-work). UI backlog queued from 0046 SURVIVES
  items (House HUD identity, ticker, zone preview, tree first-alloc).

## COORDINATION NOTE - stop mirroring task evidence (2026-08-17 ~19:20)

codex: your program-branch "mirror"/"materialize"/"reconcile" commits
duplicate other coordinators' STATUS/REPORT/captures and have now
caused three consecutive merge-conflict rounds (one baked conflict
markers into history). New rule: task evidence lives ONLY on the
task's worker branch and in the task folder committed by ITS OWN
coordinator. Do not copy another task's files onto the program branch;
the architect merges worker branches at acceptance. Board-level notes
go in your own STATUS files or new files, never edits to files another
writer owns (PROTOCOL single-writer rule).
