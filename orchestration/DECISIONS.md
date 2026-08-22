# Canonical decisions

Settled = binding until the owner reverses it. Provisional = architect's
call, cheap to reverse, flagged for owner attention. Owner-only = no agent
may decide it.

## Settled

- **D-001 Language**: native runtime is C++20 (ADR-001,
  `docs/rebuild/ADR-001-native-language.md`). MSVC on Windows now; CMake is
  the cross-platform path. No general-purpose engine (no Godot/Unreal/
  Unity/Bevy-as-engine).
- **D-002 Simulation boundary**: deterministic fixed-timestep headless core;
  commands in, events out; no windowing/GPU/sockets/DB/DOM/assets in the
  core. Presentation may only consume events and snapshots.
- **D-003 Actor symmetry**: one stat schema and one damage pipeline for
  players, monsters, future mercenaries. Elites differ by level, build,
  equipment, and abilities — never by a separate stat universe.
- **D-004 House/Scion model**: the House is the persistent identity
  (lineage/tribe, not a building); Scions are mortal individuals. Campaign
  progress (routes, branches, knowledge) is House-owned. Unextracted value
  is lost on death; significant carried items enter a House relic pool and
  can re-enter the loot stream with history.
- **D-005 Setting**: Bronze Age / pre-iron fantasy. The Delaford legacy
  denylist (`config/legacy-denylist.json`) governs what may enter native
  production code. No medieval defaults, no Delaford starter kits.
- **D-006 Track isolation**: `prototypes/founding-slice/` is a disposable
  feel lab. Its code and numbers carry no design authority; findings enter
  canon only through this file or the constitution.
- **D-007 Native client control contract** (unblocks the client task; the
  E-key conflict raised by Codex is resolved as follows):
  - WASD movement (continuous), mouse aiming.
  - LMB primary attack, RMB weapon skill.
  - Space dodge/dash.
  - Q / E / R: additional skills (E is a SKILL slot, not equip).
  - X: pick up nearest item; Z: toggle loot-name filter; gold-like
    currency (if/when it exists) auto-picks.
  - F: contextual interact (extraction standard, shrines, doors).
  - I or Tab: gear/inventory; no context-menu dependency; no piano bar.
  Equip moves to the inventory UI, not a world key.

- **D-106 Death recoverability (OWNER-RULED 2026-08-16)**: items are never
  destroyed by Scion death. Everything carried (equipped, pack, trophies)
  returns to a recoverable pool — significant items to the relic pool,
  the rest to the wider loot pool at minimum. Supersedes the baseline
  "unequipped carried items are lost forever" behavior (TASK-0018).
- **D-107 Camera direction (OWNER-RULED 2026-08-16, resolves D-102)**:
  ARPG preset is the primary camera (pitch ~62, moderate perspective,
  no tilt-shift). Miniature-style treatment applies when the player zooms
  in with the wheel (blend toward stronger perspective/tilt at close
  zoom). High Table is rejected. Both clients default to the ARPG values.
- **D-108 Look/feel acceptance target (OWNER-SUPPLIED 2026-08-16)**: the
  webchat demo vendored at `docs/reference/25d-overhaul/` (playable
  `dist/songs-of-the-mire.html`, math in `docs/ARCHITECTURE.md`) is the
  acceptance target for rendering look and feel. Its gameplay is
  throwaway; its projection/terrain/lighting design is authoritative
  reference. The phased integration brief in its HANDOFF.md targets the
  browser game as the near-term shippable product while the native
  rebuild continues.

- **D-109 Forgiving persistence (OWNER-RULED 2026-08-16)**: logout,
  disconnect, or crash never loses items or progress — the Scion keeps
  everything and returns to town (House) on next login; the instance is
  simply left. Death is the only loss event, and networked play must
  prevent disconnect-caused deaths (safe pull-out on connection loss).
  ADR-002 is ACCEPTED as amended by this ruling. If logout-as-escape
  proves abusable, the fix is an in-danger logout delay, never item loss.

- **D-110 Playable-first (OWNER-DIRECTED 2026-08-16 evening)**: the
  measure of progress is a real player having a good session — first
  minutes clear, combat satisfying, loot exciting, death fair,
  progression legible. All further feature/infra/renderer work queues
  BEHIND fixing actual play friction. Evidence of friction comes from
  played sessions (TASK-0034), not harness scenarios.

## Provisional (architect's call, owner may override)

- **D-111 Day/night default**: owner is unsure the cycle belongs in the
  game. Until ruled: the game DEFAULTS to full daytime (most readable),
  with the cycle kept behind a settings toggle (off by default). Cheap to
  flip either way when the owner decides (TASK-0033).
- **D-116 THE MISSION (OWNER-RULED 2026-08-16 ~23:05)**: the point of
  this orchestration is the C++ conversion — the native version must
  reach the current web version's level OR BETTER, with multi-layer
  regression sweeps throughout. Strategy (architect): the C++ server
  speaks the EXISTING `{event,data}` WebSocket protocol so the current
  Vue client connects to it unchanged — which makes the existing
  31-scenario playtest harness the PARITY BAR and regression suite for
  the native server (run the same scenarios against JS and C++;
  divergence = regression). Parity lands endpoint-by-endpoint per
  `docs/rebuild/PARITY_ROADMAP.md`. Sweep layers: (1) protocol/playtest
  scenarios dual-run, (2) UI pane sweeps (0036 pattern), (3) core
  determinism replays, (4) D-115 play gate. The browser game stays the
  living reference and keeps improving (current playability wave
  continues) — every improvement raises the parity bar deliberately.
- **D-112 Two horizons, one product at a time (amended 2026-08-16 late
  after owner sustainability question; SUPERSEDED IN PART by D-116 —
  the native conversion is not deferred; it proceeds now via protocol
  parity)**: the BROWSER game is the
  near-term playable and fun-finding vehicle ONLY — not the launch
  platform (own measurement: ~43ms mean frame at 1440×1000; browsers
  fight a mature ARPG's density). The NATIVE build is the launch
  platform, per the constitution. Sequence: exe triage → browser reaches
  "actually fun" via the 0034 friction list → the native client then
  rebuilds the browser-VALIDATED design system by system under the
  D-115 play gate until parity, and becomes the shipped product. No
  parallel design invention across surfaces; browser perf work capped at
  "smooth enough to judge fun."
- **D-113 Art direction (from owner feedback)**: near-term aesthetic
  follows the webchat/slice approach — procedural/vector-first with
  minimal image inputs, consistent RELATIVE SCALE (a documented scale
  chart: player height as the unit; scenery sized against it). No return
  to Delaford-era assets. Owner supplies/approves any new image assets.
- **D-114 Feel-coherence rule (process)**: any change to movement,
  range, or speed constants must re-derive ALL related distance/time
  constants together against seconds-to-contact metrics, documented in
  one table in the diff. Reviews reject isolated constant changes.
- **D-115 Play gate (process)**: feel-affecting work is not ACCEPTED
  until the architect has driven a full session of the actual build and
  judged it as an experience, not per-feature evidence.

- **D-101 Player base-life offset**: the slice gives player-kind actors a
  modestly higher base-life constant within the shared formula. If adopted
  natively, express it as starting equipment/traits instead of a kind check.
- **D-102 Camera envelope**: 2.5D perspective-billboard, pitch ~52–62°,
  mild depth perspective, tilt-shift optional. The slice's "Miniature"
  preset is the current directional target; final projection stays an open
  experiment (camera lab preserved in both clients).
- **D-103 Slice banking**: the demo banks pack items at node completion for
  pacing. Native keeps true extraction risk per the constitution.

- **D-104 CMake presets schema v2**: `native/CMakePresets.json` targets
  presets schema version 2 (CMake ≥3.20, including the MSVC-bundled
  binary), not v3. Configure/build/test presets only; no v3-only fields.
  Revisit only when a demonstrated v3-only capability is needed
  (see TASK-0002 REVIEW, QUESTION-0001).

- **D-105 smoke:browser lifecycle**: QUESTION-0003 resolved with its
  option 1 — `smoke:browser` keeps its documented name and adopts the
  same `start-server-and-test` lifecycle as `test:e2e` (TASK-0014).
  Port 6500 stays pinned.

## Owner-only (do not decide by agent)

- **D-O1 Seasonal inheritance rule** — what survives a season reset
  (see `docs/product/OPEN_DECISIONS.md`).
- **D-O2 Asset pipeline/provenance** — vendoring full-resolution plates,
  packaging, and any generated-asset policy for the native game.
- **D-O3 Magic system** — production spell design waits on the actual
  WIZARD Spell/Arcane Lattice material
  (`docs/product/WIZARD_ARCANE_LATTICE_REFERENCE.md`); no generic mana
  wizard in the meantime.
- **D-O4 Monetization/distribution, final naming, lore canon.**
- **D-O5 Economy scope** — trade, passive income, currency exchange
  (flagged in the feature checklist; seams only until decided).

## D-117 — Client-visible progress every wave (owner-ruled, 2026-08-18)

Server-first sequencing starved the visible surface for ~36 hours of
work. Standing rule: every parity/infra wave ships a client-visible
increment alongside it, and the architect's acceptance gate for any
native wave includes PLAYING the built exe, not only attaching the
harness. Wave plans must name their owner-visible deliverable up front.

## D-118 — Native client: 2D top-down now, correct projection later (owner-ruled, 2026-08-18)

The current billboard/parallax projection is buggy (scenery slides
against movement). Ruling: switch the native client to a clean 2D
top-down presentation NOW; reintroduce the 2.5D projection later as a
separate, carefully-verified wave modeled on the webchat-Fable demo
(D-108 reference, docs/reference/25d-overhaul/). Correct and plain
beats broken and fancy.

## D-119 — Native client test harness (owner-ruled, 2026-08-18)

The client gets its own automated harness, mirroring the playtest
philosophy: scripted input sequences driving the REAL client loop with
assertions on authoritative state and presentation output, run as a
gate on every client change. Test the whole pipeline as we build - we
are not going to let both layers regress.

## D-120 - Tiered delegation + architect scaffolding (owner-ruled, 2026-08-18)

The architect (Fable, most capable model in the fleet) architects AND
writes code scaffolding for implementers: interfaces, the hard
math/algorithms, and test skeletons that lock correctness in before
delegation. Task detail is matched to model capability (weak = exact
steps + scaffolding; medium = pinned interfaces). The living record is
orchestration/ORCHESTRATION-LEARNINGS.md, updated by the architect
automatically after every notable review or failure - no owner prompt
needed. This amends the architect-does-not-implement rule: scaffolding
and reference math are architect work; feature implementation remains
coordinator work.

## D-121 - Orchestration operating system v2 (owner research, 2026-08-18)

Adopted from the owner-compiled Multi-Harness Orchestration Field
Guide (Downloads/multi_harness_orchestration_field_guide.md), scaled
to current fleet size. New canonical docs: ORCHESTRATION.md (short
constitution: prime directive, topology-first dispatch + spawn gate,
authority-narrower-than-capability, delegation contract, resource
capsules, G0-G6 validation ladder, rule lifecycle), RUN_STATUS.md
(rewritten snapshot of current truth - replaces ARCHITECT_STATE's
diary role), INCIDENTS.md (append-only, INC-001..010 migrated),
ACCEPTANCE.md (gate registry), MODEL_SCORECARD.md (empirical
per-model calibration; tiers describe the JOB PACKET, not the model).
ARCHITECT_STATE.md and ORCHESTRATION-LEARNINGS.md are frozen history.
Key doctrine shifts: topology before model; team must beat strongest
solo on accepted-outcome-per-dollar; greens are revision+environment
bound; consensus is not truth (preserve dissent, select don't blend);
every adopted lesson becomes a regression or experiment. Enforcement
backlog and EXP-1 (packet-type A/B) recorded in the new docs.

## D-122 - Three-axis parity + C3 now (owner research, 2026-08-20)

Owner-supplied program correction (chatgpt_pro writeup, verified at
tip: verdigris_client links ONLY verdigris_core - the launched exe
never talks to the C++ server the N-waves prove). Parity has three
axes: (1) server/rules parity - browser harness vs C++ server, the
N1-N6 waves; (2) native journey parity - the native client completes
real player journeys against the C++ server over WS, no in-process
sim, no dev grants; (3) presentation/feel parity - readability and
cohesion via a scored quality gate. "Full native parity" may only
describe all three passed. N6 is renamed "full SERVER parity".
Sequencing: C3 (client<->server connect) starts NOW against the N4
surface, parallel to N5 - NOT after N6. N6 releases only after Gate A
(networked guest expedition) is real. Canon lives in
docs/rebuild/NATIVE_PRODUCT_CONVERGENCE.md (gates A/B/C, quality
rubric, session architecture, single-writer main.cpp rule, protocol
matrix). Throughput metric: complete player journeys moved red->green
in the native executable - not merged handlers/effects/tasks.

## D-123 - The READY queue never runs dry (owner-ruled, 2026-08-20)

Owner: "this should never be a thing that the specced packet runs dry."
Standing rule: the board carries a MINIMUM of 3 claimable READY packets
at all times, restocked at every architect sweep from the strategic
backlog (convergence gates, parity waves, N7 better-than, infra debt).
Spec-ahead is architect work of first rank - equal priority with
reviews. Enforcement: sweep checklist step; RUN_STATUS lists the READY
count; a sweep that leaves READY < 3 must say why.

## D-124 - Aggressive daytime run (owner-ruled, 2026-08-20 ~08:20)

Owner (leaving for work): no more completing a tiny goal and pausing
for hours; be ambitious and aggressive in scope. Day target: TRUE
three-axis parity, plus side-by-side screenshots of the native client
vs the browser version showing near-identical presentation. Operating
changes: (1) architect works OWN tracked tasks continuously between
reviews - idle sweeps are a defect; (2) takeover threshold - a
critical-path lane that produces no commit for ~2 sweeps gets its task
taken over by the architect as a tracked implementation task; (3)
sweep cadence tightens to ~30 min while the owner is away; (4) token
rationing is subordinate to this directive for today (owner accepted
the spend by issuing it).

## D-125 - Durable queue runway (owner-ruled, 2026-08-20 evening)

The owner identified that the morning fleet outage was compounded by a queue
that would have drained quickly even if every model had stayed connected. A raw
count of immutable SPEC headers is not runway: integrated historical tasks,
claimed work, HOLD items, and overlapping owned paths do not count.

Standing rule: before three implementation lanes are active, the board carries
at least **8 effective READY packets** with no unresolved owned-path collision,
plus at least **4 sequenced DRAFT/PIPELINED successors**. During ordinary
operation the reserve floor is the greater of 8 or two effective READY packets
per live coordinator. A claim consumes reserve and triggers restocking in the
same architect sweep. If the floor cannot be met without inventing filler or
crossing an owner-only decision, `RUN_STATUS.md` states the reason and the
architect stages lane revival instead of pretending the queue is healthy.

`RUN_STATUS.md` is the authoritative list of effective READY work. Machine
enforcement is TASK-0080. D-123 remains the original never-dry law; D-125
strengthens its depth and counting semantics.

## D-126 - PC Ox Alpha single-lane surge topology (owner-ruled, 2026-08-21)

Verdigris currently registers one planned PC OpenCode/Ox Alpha implementation
worker: `ox-pc-a`, Windows implementation, ports 6620-6639. Route one
Verdigris task at a time until the owner explicitly adds independent workers.
The stopped `ox-pc-b` and `ox-pc-c` tabs shared one OpenCode project, made no
claim or write, and are not lanes, stalls, dark capacity, or incidents. Other
Ox capacity belongs to separate owner-run projects and Verdigris does not
direct, monitor, or depend on it.

Surge ambition remains a queue/graph requirement despite single-lane dispatch:
at least 24 effective dependency-free, path-disjoint READY packets, at least 12
sequenced successors, and a deeper whole-program graph. Queue depth prepares
future lanes but never authorizes concurrent claims without explicit owner
registration or relaxes acceptance and owner-authority boundaries.

## D-127 - Provisioned is not active; activation alarms are binding (owner-ruled, 2026-08-21)

A worktree, branch, port reservation, launch packet, process, or visible tab is
not fleet capacity. A registered lane becomes live only after the exact worker
branch carries a protocol-valid committed claim, and becomes active only after
fresh post-claim execution evidence. Supervisors record launch requests with
timestamp plus exact repository/worktree/branch/base/task/identity.

A requested local worker still unclaimed after 10 minutes raises P1
`PROVISIONED_UNCLAIMED` and an owner notification on the first observing
sweep. At 30 minutes or two sweeps it escalates to P0 `ACTIVATION_FAILED`.
Wrong repository, worktree, branch, base, task, or identity is immediate P0
`MISROUTED`. Notifications deduplicate by lane/task/state and repeat only on
escalation or changed evidence. A parked or stopped lane with no launch request
is neither dark capacity nor an incident. The supervisor may repair routing
and provisioning, but never absorbs the worker's implementation task.

## D-128 - Permanent full-product backlog factory (owner-ruled, 2026-08-21)

The 24 READY + 12 successor target is only an emergency anti-stall floor. It is
not adequate runway, planning completion, or a reportable success by itself.
Primary queue health is autonomous runway hours: target at least 72, warn below
48, and treat below 24 as a critical queue incident. Required packet count is
derived from trailing accepted throughput by full experimental unit and packet
type, never human developer estimates.

Maintain three layers continuously: an initial 2,000-node concrete terminal
product graph, an initial 500-packet detailed DRAFT/AUTO_RELEASE reserve, and a
validated 72-hour READY/AUTO_RELEASE runway. These are rolling floors, not
caps. Every accepted packet receives successor analysis; audits must feed
implementation. Runnable consumption targets at least 60% implementation,
integration, content, presentation, polish, or release work and normally no
more than 25% pure audit/research/inventory/evaluation.

The graph covers the complete ARPG through full-product and release gates, not
only parity. `BACKLOG_FACTORY.md` is the operating contract and
`CONTENT_SCALE_MATRIX.md` is the provisional owner-review envelope, with
blockbuster-scale capacity used for planning until the owner selects another
tier. Distant mutable packets do not receive stale immutable bases; exact
dependency predicates release validated successors without owner prose. Low
runway is repaired by decomposition and packet production, never by architect
implementation takeover.
