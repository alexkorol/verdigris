# VERDIGRIS
## Native ARPG design and parallel execution plan

**Working edition 1.0 | September 4, 2026 | Proposal for owner adoption**

**North star:** the material presence and reward clarity of Diablo II: Resurrected, the build-expression and repeatable-goal depth associated with Path of Exile, and responsive, deliberate action combat—expressed through Verdigris's own Houses, mortal Scions, Bronze Age world, Brands, Bonds and dangerous expeditions.

**What this document is:** a product design, capability roadmap and execution contract. Its companion registry contains **200 proposed atomic goals across 25 workstreams**. It is usable by human developers and coding agents, but it does not authorize work, change repository policy or claim that any goal is complete.

**What this document is not:** a clone specification, a claim of achieved parity, a fully costed schedule, or a promise that 200 tasks reproduce every asset, feature, league and service of three commercial games. Content production expands through scoped child packets after the quality slice proves the pipeline.

---

## 1. Read this first

### 1.1 Authority and evidence

The repository constitution remains the product authority. This pack is subordinate planning material until adopted through the repository's designated decision process. A new planning recommendation does not override an existing decision, acceptance test, permission or owner restriction. [R01, R03–R05]

The earlier source review used `master` at `2d3e92a524f8d46bef02adf0c8de1b81a89e8062`. The integration branch was rechecked for this expansion at `8597c654894181c52d0a45a7932cce9deaeeb0c2`. No fresh Windows/macOS gameplay, audio listening, GPU measurement, security exploit reproduction or full branch audit was performed for this pack. Prior audit findings remain attributed findings requiring current-head verification. [R07–R09, R13]

All new scope counts, score thresholds, build examples, module layouts and scheduling rules below are proposals. No issue, PR, task claim, branch modification or agent launch has been made. `VG-...` identifiers belong to this planning pack; they are not reserved repository `TASK-NNNN` identifiers.

### 1.2 How to use the pack

The master document explains product choices, architecture, gates and working rules. `backlog/tasks.json` is the canonical structured planning registry; `ATOMIC_GOALS.md` is the readable checklist; `tasks.csv` is an import projection; the Word appendix is a compact review index. Regenerate projections after changing the registry rather than maintaining four competing boards.

Use `prompts/COORDINATOR_LOOP.md` to turn current, approved goals into collision-free packets. Use `WORKER_LOOP.md` for implementation, `REVIEWER_LOOP.md` for independent review, and `INTEGRATOR_LOOP.md` for controlled integration. The included validator is read-only: it checks the plan and proposes candidate groups. It neither claims tasks nor grants permission to edit the game.

All goals start **DRAFT**. Only a current-base, policy-approved READY packet with exact file ownership, runnable acceptance commands and an exclusive claim is executable. A plausible plan is not a claim.

---

## 2. Product thesis and benchmark translation

### 2.1 The game we are making

Verdigris is a native Windows/macOS action RPG in a grounded pre-iron fantasy world. The player leads a persistent House through a succession of mortal Scions. A Scion ventures out for a specific opportunity, changes their build through consequential equipment and techniques, decides whether to push farther, and tries to bring value home. A House retains knowledge, access, selected investments and a bounded history—not a free copy of every fallen character's equipment. [R01]

**Design promise:** “Your House remembers what your Scion risks.”

The core experiential loop is:

> Choose a goal → prepare a build → enter a dangerous route → read and answer threats → acquire something meaningful → push or extract → invest in the House → continue, retire or replace the Scion.

Every production feature should improve a decision in that loop. A feature that only increases the number of panels, nouns or percentages needs a separate justification.

### 2.2 Translate the references into capabilities

Blizzard's remaster announcement describes 3D physically based rendering, dynamic lighting, revised animation/effects and upgraded sound. The useful benchmark is coherent presentation quality, not “large sprites.” GGG's official overview emphasizes character customization and an item economy; its Atlas surface establishes endgame specialization as a distinct player system. PoE2's official feature summary describes active/support gems, while official patch examples demonstrate movement/skill interaction work. We use those as capability references, not as a verified inventory of their current patches. [E01–E05]

| Reference dimension | Verdigris equivalent | Evidence needed | Deliberate exclusion |
|---|---|---|---|
| Tangible loot and readable rewards | Distinct bases, meaningful modifiers, world-visible equipment, excellent pickup/equip feedback | An earned item changes tactics and is understandable in play | Copied names, icons, item databases or drop tables |
| Build composition | Freeform techniques, passive choices, compatible transformations, bounded Arcane weaves | Several viable builds solve the same encounter differently | Matching a competitor's tree or gem count |
| Action responsiveness | Independent movement/aim, phased actions, readable attacks, consistent collision | Input traces plus unaided counterplay observations | Mandatory piano-sized hotbars |
| Endgame agency | Goal-driven expedition graph, target families, risk modules, House route investment | Different repeatable routes create different next decisions | Copying every historical league mechanic |
| Material/audio quality | Cohesive pre-iron materials, grounded actors, animation and layered sound | Ordinary-play captures and dense-fight listening review | Beauty shots disconnected from gameplay |
| Economy and multiplayer trust | Stable ownership, transactional progress, authenticated cooperative play and later escrow | Restart, duplicate-request and impaired-network conservation tests | Publicly exposing the development guest server |

### 2.3 Resolve conflicting aspirations

**Fast clearing versus deliberate bosses.** Ordinary packs should support flow; elites and bosses should create distinct readable decisions. Do not make every normal enemy a long duel, and do not let build scaling erase every authored response. Difficulty tuning must be evaluated at low and high approved build investment.

**Depth versus comprehensibility.** The small action bar is a hard design advantage. Complexity belongs in preparation, interactions and decision quality, not in dozens of simultaneously required buttons. Add advanced explanation progressively.

**Mortality versus time respect.** Death needs stakes, but loss must be predictable and legible. House knowledge and preparation should create continuity without converting replacement Scions into a tax on playing the game. Disconnect and crash policy must be explicit before tuning mortality.

**Spectacle versus legibility.** Player VFX may be spectacular; lethal enemy information must remain visible and audible. Quality settings must not remove the only cue for a required response.

**Trade versus self-found viability.** Design the core progression loop to work without a market. Trade can broaden choices later, but is not a repair for an unrewarding loot loop. Online and offline trust domains stay separate unless an explicit import policy is approved.

---

## 3. Unique systems: design contracts, not just themes

### 3.1 House and Scion progression

A Scion owns their current combat state, equipment and expedition risk. A House owns campaign knowledge, stores, route access, craft relationships, bounded investments and historical records. The initial House investment should unlock a preparation or access choice; unbounded inherited damage is not the default. [R01]

**Required player decision:** “Which future opportunities should this expedition secure?”

**First proof:** spend one extracted material on one durable route/preparation benefit; restart; create a successor; demonstrate that benefit without restoring the previous Scion's complete build.

**Guardrails:** House state is server-owned; investments are transactional; no duplicate claim on extraction; catch-up alternatives do not invalidate early content; permanent-death rules never hide behind ambiguous UI.

### 3.2 Brands and Bonds

Brands are an intentional crafting direction already named by the constitution; Bonds are an item-history/progression direction. The exact formulas are not finalized by the reference material. The following semantics are a proposal for a bounded native implementation, not an assertion that they already exist. [R01, R06]

**Brand proposal:** a House service spends known resources to impose one legible mechanical change on an existing item. The player sees eligibility, outcome range, permanence and opportunity cost before committing. The item retains its identity.

**Bond proposal:** significant approved use creates a bounded progression milestone that unlocks a property or tradeoff. Count meaningful events, not frame time or harmless attack spam. A Bond can reinforce a style, but must not force players to use a weak item for hours without useful feedback.

**First proof:** one preview → one transactional Brand → one Bond milestone → one changed combat behavior. Repeating the command, reconnecting or cancelling cannot create free resources or reroll the result.

### 3.3 Scars, relics and history

A fallen item's eligible recovery state is an exclusive claim, not a duplicated world drop. A recovered relic retains a provenance chain and may gain an approved scar/tradeoff. Recovery must create a targetable expedition story while preserving scarcity and conservation.

**First proof:** death creates one recovery candidate; a later route reveals an attributable opportunity; recovery resolves once; the crypt and inventory agree after restart. Losing sight of the item or abandoning its floor cannot create a second claim.

Use bounded counters and milestone records. Do not save a prose line for every hit or run an unbounded world-history simulation in the combat tick. [R01, R07]

### 3.4 Arcane Lattice

The repository reference describes a fixed lattice topology and explicitly notes that its spell generator is a placeholder. A native implementation therefore needs a real, bounded compiler from an approved path to existing action/effect primitives. The reference interface is not gameplay authority. [R10]

Start with a restricted subset: legal adjacency, permitted strata/tier, one resource model and one manifestation. Each compiled plan declares action timing, targeting, effects, cost, incompatibilities and a work budget. Unsupported paths reject with an explanation; they do not improvise a spell.

**First proof:** changing one legal node changes a known property of one playable spell. The result remains deterministic, explainable, cancellable and performant. Then expand by one approved transformation at a time.

### 3.5 Targeted expeditions and living Legends

The endgame equivalent of an Atlas is not a copied tree. It is a House-readable graph of opportunities: materials, trophy families, relic clues, specializations, encounters and optional risk modules. A route card tells the player why to go, what can go wrong and how return works.

A later bounded monster Legend can make a past killer recognizable in a future encounter. Its history modifies an approved template within a cap; it does not accumulate unlimited levels, states or simulation cost.

**Signature journey:** recover a storied item from a route shaped by House knowledge, alter it through a Brand, earn a Bond ability, and choose a different expedition because the build now answers a different danger.

---

## 4. Scope ladder and non-negotiable gates

### 4.1 Quality slice, then breadth

| Scope tier | Proposed content envelope | Purpose |
|---|---|---|
| G0–G1 foundations | Reproducible baseline, decisions, authoritative profile, tests, limited renderer/audio trials | Make further work safe and measurable |
| G2 first playable | One connected native loop with reliable input, loot, equipment, return and actual presentation | Eliminate isolated feature islands |
| G3 quality slice | One hub; one biome; two layout variations; one polished boss; a small mixed-role roster; three build directions; one Brand/Bond/recovery loop | Demonstrate comparable execution quality within a bounded experience |
| G4 production-depth release candidate | Second biome pipeline; repeatable expedition goals; native two-player proof; expanded approved content lots | Prove throughput and systems depth without blanket parity claims |
| G5 scoped commercial release | Owner-approved campaign/endgame scope, platform matrix, migrations, security, support and release evidence | Ship the agreed product truthfully |
| Later parity expansion | Additional approved builds, acts, enemies, bosses, online services and optional seasons | Expand breadth only while preserving quality |

The G3 route targets about 30–45 minutes of player experience, with an automated shorter journey for regression. These are content targets, not development-time estimates. The constitution's eventual campaign direction is approximately 6–30 hours depending on optional branches; reaching it requires real content lots, not more framework code. [R01]

A proposed commercial content discussion may consider three campaign movements, multiple biome families and several build families, but **no exact act, skill, boss or unique-item quantity is committed here**. GOV-007 freezes the funded scope after the first pipeline evidence. The 200 backbone tasks do not include every future art asset or content instance.

### 4.2 Gate evidence

**G0 — program truth.** Current head and authorities are recorded; duplicate work is reconciled; slice scope and reference machines are agreed; the dependency/path validator is accepted.

**G1 — trustworthy foundation.** The actual profile can survive server restart; ownership and clocks have explicit contracts; malformed input is bounded; platform and content experiments are isolated and reproducible.

**G2 — integrated first playable.** A fresh profile can enter, move, fight, loot, equip and return through the real native client/session. No developer grants, hidden fallback or direct state mutation. Incomplete polish is identified, not claimed away.

**G3 — quality slice.** Three approved builds, mortal succession, craft/recovery, restart, visual target, audible combat and performance all pass on the exact packaged build. A failed integrity or readability dimension blocks the gate even when other scores are excellent.

**G4 — production proof.** A second content lot shows designer/artist throughput, endgame choices change goals, and two-player impaired-network play is trustworthy. Online economics require their own authority and conservation proofs.

**G5 — release.** The supported platform matrix, compatibility, updates, diagnostics, signing/distribution policy, security and support ownership are accepted. Marketing scope is derived from evidence, not task count.

### 4.3 Proposed quantitative checks

Performance thresholds require named hardware and test settings. Initial targets: stable 60 FPS at 1080p in the reference dense encounter; p99 frame time at or below 25 ms; no recurring warmed-up combat stalls above 100 ms; p95 local input-to-visible-response below 80 ms under a stated measurement method. Separate CPU simulation, GPU frame work, asset loading and network delay. These values are proposals, not current measurements.

Integrity checks are invariant-based: zero duplicate committed ownership in the fault suite; no loss of acknowledged committed progress; deterministic replay within the approved build/content contract; bounded event, effect, history and session memory.

Playability checks use an observed small cohort, initially 5–8 fresh testers per iteration: can players state their goal, identify the lethal threat, equip an upgrade, choose a return and explain retained/lost value? Record raw counts and coaching. Do not claim statistical population certainty from this sample.

---

## 5. Canonical architecture and ownership

### 5.1 Target flow

```text
Platform input -> typed command -> local OR remote session adapter
             -> canonical game services -> fixed-step world/actor simulation
             -> typed events + snapshots -> presentation model
             -> animation / GPU renderer / sound / user interface

Game services -> transactional profile store
Authored definitions -> validators -> cooked versioned content
```

Local play changes transport and trust policy, not damage, items or extraction rules. Client prediction is a visual responsiveness mechanism; it does not authorize damage, pickups, death or durable rewards. Network reducers cannot create gameplay rules.

The reviewed architecture has separate local/world simulation surfaces and important profile responsibilities in the networking layer. Convergence must be incremental: characterize behavior, approve canonical rules, migrate one boundary, verify the journey, then retire the superseded production path. A large rewrite or automatic browser port is not authorized by this plan. [R02, R07, R08, R11, R12]

### 5.2 Contract ownership

| Contract | Single accountable owner | Parallel consumers | Approval trigger |
|---|---|---|---|
| IDs, units and clock | Core lead | Movement, combat, world, persistence | Changed semantics or replay behavior |
| Action phases and event schema | Combat/core lead | Animation, sound, UI, networking | New fields, timing or cancellation semantics |
| Item location and transaction record | Item/persistence lead | Inventory, crafting, trade, House | Ownership, save schema or economic effect |
| Content IDs and version | Tools lead | Every content/art/runtime lane | Version compatibility and identifier changes |
| Render packet and resource lifecycle | Rendering lead | UI, world, VFX, capture | Backend/dependency and packet changes |
| Profile schema and migrations | Persistence lead | Services, networking, lifecycle | Every schema migration or durability policy |

Interface owners publish small contract fixtures before parallel consumers implement. A header file split does not eliminate semantic coupling. Claim both write paths and the logical contract resource when modifying a shared interface.

### 5.3 Incremental module extraction

Candidate new module locations in the registry are proposed—not existing paths. At READY promotion, inspect the current tree and replace them with exact owned paths. Keep shared `main.cpp`, `core.cpp`, `networking.cpp`, public headers, CMake, schema registries and CI entry points under explicit integration reservations.

A sidecar model can land with its tests, but it remains **NOT_INTEGRATED** until an accepted task connects it to the production client. There must be a named integration successor and a gate that fails when the connection is removed. Avoid another layer of unused foundation code.

---

## 6. Progress, mortality and economy integrity

Persist the actual profile aggregate, not only the small prototype simulation. Account for House, Scion, inventory, wear, stores, passives, route knowledge, lifecycle, XP, pending recovery and required random state. Classify all live fields as durable, transient or reconstructible. The prior durability audit specifically distinguishes snapshot helpers from production profile persistence. [R07]

### 6.1 Transaction boundaries

Extraction, crafting, death/recovery and trade use explicit request/transaction IDs, precondition revisions and committed outcomes. Acknowledgement follows durable commit. A retry returns the recorded result rather than applying the mutation again. Inventory locations must conserve instances before and after every transition.

For a first local/server profile store, compare a tested embedded transaction store with a rigorously tested atomic-file design. The decision is Tier C. The requirement is crash-consistent profile semantics, migrations and write ownership—not a particular database brand.

### 6.2 Failure policy requiring a ruling

| Event | Required recorded decision |
|---|---|
| Voluntary quit in danger | Does the expedition remain live, retire, or impose a bounded consequence? |
| Short connection loss | What grace/rejoin behavior applies, and who owns the actor meanwhile? |
| Server crash | Which committed state is restored and what happens to the live expedition? |
| Confirmed mortal death | What is lost, retained, eligible for recovery and visible to the successor? |
| Disconnect during extraction | Which durable transaction state determines the result? |

A test harness must terminate disposable processes around commit boundaries, not merely reconnect to a live in-memory session. Never test destructive recovery against a real owner profile. Backups and migrations must preserve evidence when recovery fails; silently replacing a damaged profile with a new House is unacceptable.

### 6.3 Public-service boundary

Do not expose a guest/loopback prototype by changing a bind address. Static audit candidates in parser recursion, work amplification and economic inputs should be verified and fixed in isolated tests. Online access needs authenticated identities, bounded resources, encrypted transport/deployment policy, server-derived prices and quantities, and removal of release dev commands. [R09]

Asynchronous trading is a later, separate milestone: listing escrow, cancellation, two-party settlement, crash replay, auditability and offline/online separation. A global market is not a prerequisite for a good first expedition.

---

## 7. Combat and build implementation policy

The action model owns request validation, resource cost, windup, active geometry/projectile, recovery and cancellation. Simulation events drive animation, VFX, sounds and UI. A renderer's animation completion must not decide damage.

Publish explicit geometry and duration units. The current remote adapter and prior combat audit justify first addressing input mapping, supported state replication, mechanical role behavior and event consistency. Findings should be reverified on the chosen READY base; do not retest a stale report as though it were a current executable result. [R08, R11]

### 7.1 Build algebra

Freeze modifier order before adding dozens of stats. Distinguish base values, additive modifiers, multiplicative factors, conversion, caps, conditions, proc triggers and derived display values. One resolver explains the applied result for players and monsters. Unsupported properties remain explicitly inactive rather than being advertised as functional.

Initial build fixtures are illustrative, pending approval: a reach/control fighter, a close-range pressure fighter, and a bounded magic configuration. Each needs a weakness, gear dependency, meaningful defensive choice and a different answer to a mixed encounter. This is not a class declaration.

Protect against proc recursion, reservation/loadout-swap abuse, cooldown resets and unlimited effect work. Gameplay magnitude and complexity are distinct budgets: low damage does not make an unbounded loop safe.

### 7.2 Enemy composition

Implement distinct behavior before multiplying skins. A minimum useful pack demonstrates pursuit, ranged pressure and support; add a heavy threat and a boss pattern with clear responses. Existing TASK-0108 is the first reuse candidate for ranged behavior. [R06, R08]

Every dangerous action must expose source, anticipation, legal response and consequence. Enemy placement must preserve spawn safety, movement space and readable threat overlap. Elite composition needs incompatibility rules, not random unrestricted stacking.

---

## 8. Presentation, audio and accessibility

Choose one bounded renderer proof with ordinary gameplay scale. The existing build graph and renderer work should be reconciled before dependencies are adopted. Preserve semantic render-list tests, but add actual pixel captures and listening review. A GPU backend is a means to coherent materials, animation and frame pacing, not a quality certificate by itself. [R06, R12]

Compare billboard/atlas, hybrid and real-time 3D actors using the same scene, equipment changes, lights, memory limits and art-authoring effort. The constitution's 2.5D direction stays authoritative until an explicit visual ruling changes it. Decide before producing large quantities of incompatible art.

**Art contract:** camera, actor proportions, scale, pivots, attachment positions, material maps, source files, animation phases, collision proxies and provenance. The approved target must be in-game, not a mood board alone.

**Audio contract:** real device output, packaged cues, event attribution, variation, spatial rules, voice priority, category controls and dense-fight mix review. Scheduler tests prove selection; they do not prove audible quality.

**Accessibility contract:** scalable text/HUD, remappable controls, non-color-only danger cues, reduced flashes, independently controllable camera shake, readable tooltips, focus-safe panes and clear hold/toggle behavior. Controller support has its own full-journey gate.

**Capture discipline:** show representative combat, inventory, route choice, death/succession and extraction at approved resolutions. Store exact build/content/seed, render settings and platform. Stage-only beauty shots cannot certify the quality slice.

---

## 9. Content factory and expansion templates

Framework completion is not content completion. Once the first slice works, content expansion uses repeatable lots with unique identities and evidence.

| Lot type | Required child sequence | Completion boundary |
|---|---|---|
| One active technique | Approved behavior -> definition -> mechanic fixture -> animation/VFX/audio -> runtime binding -> playtest | Earn/use it through ordinary play |
| One item/relic | Purpose and source -> legal modifiers -> art -> behavior -> drop/craft placement -> tooltip/history -> balance review | Find, equip and understand its consequence |
| One enemy | Tactical role -> AI/action -> model/sprite -> animation/audio -> encounter placement -> counterplay review | Readable in a mixed pack |
| One room | Encounter purpose -> blockout -> collision/navigation -> final kit -> spawn/exit tests -> playtest | Useful route chunk, not empty geometry |
| One boss | Pattern decision -> one action per packet -> phase rules -> audiovisual tells -> reward -> three-build tests | Legitimate completion and failure are explainable |
| One biome | Individual room/enemy/art/audio lots -> route assembly -> variation -> profiling -> fidelity review | Complete second route without invasive core edits |

Use the included content-lot generator to create DRAFT child records from an approved manifest. Each actual asset and behavior remains a separate reviewable unit. Large generated lists are planning aids, not automatic authorizations or completed work.

Every reusable lot records authoring effort, rework, integration time and blocked time. Use those observations to estimate the next lot. Do not infer art/content throughput from the number of coding agents available.

---

## 10. Parallel work model

### 10.1 Roles and lane capacity

Begin with up to eight disjoint implementation lanes, consistent with the older documented concurrency convention, plus designated review/integration capacity. Increasing this limit requires an adopted policy and evidence that review and shared contracts are not the bottleneck. [R03]

Suggested disciplines are core/persistence, combat/systems, input/UI, rendering/platform, content/tools, technical art/animation, audio, and QA/security/performance. A person may cover multiple disciplines, but their active ownership remains one task at a time. Human judgement remains necessary for product rulings, art direction, feel and release claims.

Keep separate seats for coordinator, implementer, reviewer and integrator. An implementer may not self-approve. Different model families can strengthen review, but reviewer independence is not established merely by a different chat window when both used the same unverified assumptions.

### 10.2 Eligibility and conflict rules

A task is eligible only when required predecessors are **INTEGRATED on the accepted program head**, relevant decisions are approved, exact paths/commands are stamped, and no active claim overlaps its resources. ACCEPTED on a worker branch is not enough.

Treat these as conflicts: the same file; a file within another lane's owned directory; overlapping glob patterns; one shared public contract; a schema/ID registry; one generated output; a CMake/CI entry point; or the same external resource such as a port range or test profile. Read-only access is allowed, but shared interfaces must be version-pinned.

A 200-task DAG is not permission to launch 200 agents. Dispatch only the currently eligible, conflict-free frontier. A scheduled wave is a conservative grouping, not a calendar estimate.

### 10.3 Claims: a required correction

The repository's older protocol says not to push, while the later BUS requires pushed coordination; they also differ in role permissions. GOV-002 must obtain a recorded precedence ruling before autonomous dispatch adopts either interpretation. [R03–R05]

There is a second technical issue: two agents can both push different lane branches successfully. That does **not** create an exclusive lock. This pack proposes a single authoritative coordinator-written claim ledger, or a transactional compare-and-swap claim service approved by the owner. For a Git-backed ledger, every claim must update the same designated ref against an expected old revision; a rejected update must be reread and revalidated. No force-push is allowed.

Each claim contains task ID, lane, worktree, base SHA, contract versions, resource locks, claim token, lease/revision and evidence location. A lease expiry marks work STALE; it does not allow a second writer until the coordinator revokes/releases the prior token and reconciles WIP. Integration checks that the submitted claim token is still current.

The included tools do not implement this shared claim authority. They remain deliberately read-only until such a mechanism is approved and implemented.

### 10.4 Worktree and branch safety

Use one owned clone/worktree per lane. Never switch or modify another lane's working tree. Create separate build, capture, log, port and disposable-profile locations. Do not bind the protected development port by default; allocate and record a task-owned range after inspecting current policy.

Do not stash, reset, delete or commit another worker's files. Do not remove a stale worktree until its owner or coordinator has preserved its commits. No direct pushes to protected branches, force-pushes, blanket staging or automatic release publication.

---

## 11. Atomic task contract and evidence levels

### 11.1 Definition of an atomic goal

One goal has one player/system outcome, one acceptance boundary and one reviewable implementation. It may include the tests and data necessary to prove that outcome. It is too large when it introduces unrelated behavior, spans independent contracts, needs multiple designers' rulings, or cannot be independently reverted/disabled safely.

Split an oversized task into contract, implementation and integration children with explicit dependencies. Do not split merely by arbitrary line counts; tests and generated art can be large while behavior remains small. S/M size hints in the registry are qualitative and are not time estimates.

### 11.2 Required READY fields

Task ID and approved existing-packet mapping; exact base and authority hashes; outcome/non-goals; integrated dependencies; owned/forbidden paths; logical resource locks; contract inputs/outputs; acceptance commands with named assertions; a negative control; required ordinary-play or art evidence; rollback/forward-recovery plan; reviewer tier; stop conditions; and integration owner.

A packet containing “TBD,” guessed paths or commands to nonexistent tests stays DRAFT. A worker may implement new tests, but the packet must state how the new target is built/run and who owns any build-system wiring.

### 11.3 Evidence ladder

| Level | What it proves | What it does not prove |
|---|---|---|
| Defined | Design or contract exists | Runtime behavior |
| Unit-proven | Isolated implementation matches assertions | Native integration |
| Integrated | Real production caller exercises the behavior | Visual/audio usability |
| Perceptually accepted | Ordinary play looks, sounds and communicates correctly | Durability or release packaging |
| Packaged and gate-passed | Exact distributable passes relevant platform, journey and failure gates | Unlimited future compatibility |

Keep the repository lifecycle: DRAFT -> AUTO_RELEASE -> READY -> CLAIMED -> IMPLEMENTED -> REVIEW_REQUESTED -> ACCEPTED -> INTEGRATED, with REVISE/BLOCKED/SUPERSEDED as appropriate. The evidence ladder is a set of separate facts, not a replacement task state machine. [R03]

### 11.4 Definition of done

A task is done only when independent review accepts the exact implementation, integration occurs on the approved head, its production/evidence boundary is satisfied, and the relevant combined gates remain green. A screenshot, unit test or merged header alone cannot stand in for a complete player journey.

The report must record changed files, added contracts, commands and exit codes, failure output, manual observations, exact head/artifact hashes, limitations and any NOT_INTEGRATED status. Never fabricate test runs. Do not weaken an existing test without an explicit approved behavior change.

---

## 12. Review, integration and recovery

Tier C includes core contracts, wire semantics, persistence formats, production dependencies, irreversible economy behavior and release decisions. Such tasks wait for the designated architecture/product authority. Bounded tasks follow the approved independent review policy; routine mechanics may use a lighter path only when a standing policy names it. [R05]

The reviewer first verifies base/head and evidence provenance, then reads load-bearing code, reruns acceptance and negative controls, and checks player-visible integration. The reviewer records ACCEPTED, REVISE or BLOCKED with numbered reproducible findings. The author addresses findings in a new frozen head; acceptance does not transfer automatically.

Only the designated integrator updates the program branch. Rebase/merge against the current accepted head, resolve conflicts in the owned integration slot, rerun impacted gates, record the exact merge SHA and evidence, then mark INTEGRATED. Shared-file modifications from different accepted tasks are not assumed composable.

A failed combined gate stops integration. Preserve the failing head/logs, bisect or revert the specific candidate safely, and route a bounded correction. For persistence/content format transitions, use the approved compatibility or forward-recovery procedure rather than blindly rolling back user data.

---

## 13. First dispatch and critical-path management

### 13.1 Bootstrap before implementation

Complete GOV-001 through GOV-008 as applicable, including current-head reuse mapping, claim-policy reconciliation, renderer/scope rulings and graph validation. Establish QA-001 evidence and PERF-001 hardware/trace conventions. These are setup prerequisites, not background tasks silently completed by this document.

### 13.2 First candidate parallel wave

After the required bootstrap decisions are integrated, these are useful candidates—not pre-approved claims:

| Lane | Goal | Why it can proceed separately | Reserved integration concern |
|---|---|---|---|
| Core | VG-CORE-001 | Characterization fixtures before rules migration | Core test/build entry point |
| Input | VG-MOVE-001 | Narrow remote direction correction | remote_session.cpp and session tests |
| Persistence | VG-SAVE-001 | Actual profile inventory/contract preparation | Profile and networking public contract |
| Tools | VG-TOOLS-001 | Content ID and schema validator | Content registry/version policy |
| Rendering | VG-GPU-001 | Isolated approved GPU experiment | CMake and native shell later |
| Art | VG-ART-001 | In-game target and representation specification | Camera/asset contract approval |
| Audio | VG-SOUND-001 | Isolated real device-output adapter | Backend dependency and client linkage |
| Security | VG-SEC-001 | Bounded JSON parser correction | networking.cpp writer reservation |

Review the exact READY paths again before dispatch. Audio/render builds cannot both edit CMake concurrently merely because their subsystem source files differ. The coordinator can stage their build integration serially.

### 13.3 Critical paths

The integrity path runs through profile inventory/schema -> canonical profile service -> production save/load -> idempotent transactions -> extraction/death/crafting -> failure tests.

The presentation path runs through visual/backend ruling -> resource/content pipeline -> real native scene -> animation/effects/audio -> capture/performance -> fidelity acceptance.

The playability path runs through clock/input -> action lifecycle -> meaningful stats/items -> mixed encounters/build choices -> expedition assembly -> unaided playtest.

G3 requires all three. Do not keep the fastest path busy by manufacturing low-value tasks while the blocking path has no reviewer, contract ruling or integration slot.

---

## 14. Agent operating loops

The full copy/paste prompts live in `prompts/`. Every prompt begins with the same boundaries: inspect current authority, reconcile existing work, use an owned worktree, obtain a real exclusive claim, operate only within an approved packet, and report evidence honestly.

**Coordinator loop:** observe one current snapshot -> reconcile task states and owner rulings -> compute integrated dependency frontier -> reject path/contract/resource collisions -> promote only validated packets -> dispatch with claim tokens -> route independent review -> integrate through the authorized seat -> reevaluate the blocked frontier. It is event-driven and bounded, not a perpetual chat promise.

**Worker loop:** verify packet/base/claim -> reproduce baseline -> add the smallest failing assertion -> implement one outcome -> run acceptance and negative controls -> capture required integration proof -> freeze head -> report -> stop at REVIEW_REQUESTED. Pick another task only through a fresh claim.

**Reviewer loop:** verify exact head -> read load-bearing changes -> rerun evidence -> inspect negative control and integration -> assess risk tier -> record a testable verdict. Never self-review.

**Integrator loop:** verify acceptance and live claim/permissions -> stage onto current head -> reserve shared files -> run combined gates -> record integration or restore a safe head. No automatic publication or irreversible account action.

**Playtest/art loop:** use approved build/seed/scene -> observe real player-facing behavior -> score each required dimension separately -> capture failures -> route specific corrections. AI-generated assets or developer assertions cannot self-certify perceptual quality.

---

## 15. Capacity, metrics and anti-theater rules

Measure completed player journeys, defects escaping review, review queue age, accepted-to-integrated delay, content rework, save invariant failures, frame-time regressions and observed player confusion. Do not reward raw code volume, task count or “green” snapshots without provenance.

Initial WIP policy proposal: one active implementation per worker, one active integration train, and a review queue no larger than the available reviewers can address in one planned cycle. Pause new dispatch to an overloaded integration hotspot. Use specialists on critical feedback and art rather than forcing every lane to emit code.

No calendar schedule is asserted because team composition, working time and asset throughput are not known. Estimate later using actual completed atomic goals and content lots, with separate engineering/art/review capacity. Count human art, design and QA capacity explicitly; model invocations are not equivalent people-months.

**Anti-theater rules:** no acceptance through empty test scaffolds; no reducing benchmark load to make a regression disappear; no direct simulation mutation in ordinary-play gates; no hidden local fallback; no polishing only staged scenes; no task closure for unused modules; no source-tree dependencies in release packages; no restoring the old browser's accidental behavior just to reuse a test.

---

## 16. Decision register and stop conditions

| Planning decision | Required output before affected work |
|---|---|
| DRAFT-D01 orchestration precedence | Owner-approved policy for push, claims, review and integration |
| DRAFT-D02 exclusive claims | Single coordinator/CAS authority, revocation and resource-lock rules |
| DRAFT-D03 visual representation | Approved billboard/hybrid/3D actor boundary and camera target |
| DRAFT-D04 production dependencies | Backend/storage/audio choice with supported-platform trial |
| DRAFT-D05 mortality and disconnect | State-transition table for death, quit, crash, rejoin and carried value |
| DRAFT-D06 slice and funded scope | Builds, route, content lots, online scope and explicit deferrals |
| DRAFT-D07 Brands/Bonds/Arcane semantics | Approved narrow recipes, thresholds, costs and effect examples |
| DRAFT-D08 online economy trust | Authentication, offline import, party loot, trade and reset policy |
| DRAFT-D09 release support contract | Machines/OS/input, accessibility, diagnostics, update and service ownership |

These are planning labels, not repository D-numbers. Workers do not invent those decisions. A blocked decision pauses only the affected branch of work; another approved disjoint task may proceed with a new claim.

Stop immediately for ambiguous authority, active path/contract overlap, expired/revoked claim, incompatible public interface, non-disposable destructive testing, repeated unexplained failures, or a task that cannot be proven within one reviewable change. Preserve evidence and route a question rather than silently expanding scope.

---

## 17. Existing program crosswalk

| Existing work | Planning continuation |
|---|---|
| TASK-0108 ranged combat | ACT/ENEMY projectile and warning goals; one implementation, not two forks |
| TASK-0097 durability audit | SAVE profile, transaction and fault-injection goals |
| TASK-0114 renderer evaluation | GOV renderer ruling and GPU proof, not another unbounded literature review |
| TASK-0162 passive payload work | BUILD allocation validation; verify current behavior before new implementation |
| TASK-0166–0169 and 0179 asset packs | TOOLS provenance/cooking and ART/UI integration |
| TASK-0170–0176 models | UI/MOVE/ART/WORLD runtime integration, preserving accepted contracts |
| TASK-0177–0178 content seeds | STORY/WORLD validated native content |
| TASK-0180–0192 render/world integrations | GPU/UI/ART/WORLD current-head reuse and completion |
| TASK-0193–0196 trees/lattice | BUILD bounded native rules, real effect plan and player-facing choices |
| TASK-0197–0204 House/crafting/recovery/prologue/audio | HOUSE/FORGE/STORY/SOUND complete signature journey |
| TASK-0205–0208 journey/fidelity/performance/release | G3 gate spine; use existing packet authority instead of a parallel release board |

This crosswalk is a starting map based on the recorded runway, not a claim that every listed packet is currently incomplete or complete. GOV-004 refreshes each against the real program head and records reuse/extend/verify/new/superseded. [R06]

---

## 18. The 200-goal backbone and expansion rule

The Word appendix lists all 200 goals with acceptance and dependencies. The companion catalogue adds negative controls, candidate paths, integration reservations, evidence, existing packet mappings and stop conditions. The structured registry is the authoritative planning source.

The graph has 689 dependency edges at this edition. It is acyclic and has no missing IDs under the included static checks. That validates graph structure—not game feasibility, task completion, code quality or live exclusivity. All 200 goals remain DRAFT.

Content-lot descendants may increase the number of goals substantially. Their completion requires implementation, assets, native integration and acceptance. Closing an enabling backbone goal never closes its unimplemented content children.

**Final scope test:** can a player pick a goal, use a distinct build, read and answer threats, earn a consequential item, make a real extraction decision, persist House value, and experience a meaningful successor/recovery story? Then can the team produce a second equally coherent region and support real cooperative play without losing those qualities? Those outcomes—not a repository full of component names—are the route to credible parity.
