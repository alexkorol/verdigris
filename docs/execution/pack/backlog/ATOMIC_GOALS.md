# Atomic goal catalogue

Date: 2026-09-04. **All 200 goals are DRAFT proposals, not authorized task claims.**

Each entry is one bounded outcome. Current-base promotion must replace proposed locations with exact owned paths, test commands and authority records. New directories shown here are proposed architecture, not claims about existing files.

Read `docs/MASTER_DESIGN_AND_EXECUTION.md` and `prompts/WORKER_LOOP.md` before use. See `docs/SOURCES.json` for ordinary source references.

## GOV — Program truth and decisions

Primary discipline: Architect / producer. Integration seam to reserve: `orchestration authority`.

### [ ] VG-GOV-001 — Freeze a reproducible baseline

**State:** DRAFT | **Gate:** G0 | **Priority:** P0 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Record actual branch heads, build commands and package hash in a baseline manifest
**Acceptance:** A second checkout resolves the same commits and reproduces the documented baseline result
**Negative control:** A mismatched commit or missing package is reported, not silently accepted
**Dependencies:** None; baseline bootstrap.
**Candidate owned paths:** `docs/execution/decisions/freeze-a-reproducible-baseline.md`
**Integration reservation:** `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R03, R04, R05, R06; task content is a new proposal.

### [ ] VG-GOV-002 — Resolve orchestration precedence

**State:** DRAFT | **Gate:** G0 | **Priority:** P0 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Obtain one recorded ruling for conflicting push, review and claim policies
**Acceptance:** A policy table names the authoritative writer, target and approval for every state change
**Negative control:** Separate successful lane-branch pushes never count as exclusive claims
**Dependencies:** VG-GOV-001
**Candidate owned paths:** `docs/execution/decisions/resolve-orchestration-precedence.md`
**Integration reservation:** `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R03, R04, R05, R06; task content is a new proposal.

### [ ] VG-GOV-003 — Freeze the parity scorecard

**State:** DRAFT | **Gate:** G0 | **Priority:** P0 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Approve benchmark dimensions, reference captures and proposed pass thresholds
**Acceptance:** Every dimension has a journey, measurement and accountable approver
**Negative control:** A feature count alone cannot produce a parity pass
**Dependencies:** VG-GOV-001
**Candidate owned paths:** `docs/execution/decisions/freeze-the-parity-scorecard.md`
**Integration reservation:** `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R03, R04, R05, R06; task content is a new proposal.

### [ ] VG-GOV-004 — Crosswalk existing task packets

**State:** DRAFT | **Gate:** G0 | **Priority:** P0 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Map each planning ID to reuse, extend, verify or new against current task records
**Acceptance:** Existing TASK-0108 and Owner Demo work have explicit dispositions before new tasks are issued
**Negative control:** Two planning goals mapping to one implementation are flagged for reconciliation
**Dependencies:** VG-GOV-001
**Candidate owned paths:** `docs/execution/decisions/crosswalk-existing-task-packets.md`
**Integration reservation:** `orchestration authority`
**Existing packets:** TASK-0166, TASK-0205
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R03, R04, R05, R06; task content is a new proposal.

### [ ] VG-GOV-005 — Choose the renderer trial boundary

**State:** DRAFT | **Gate:** G0 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Approve one limited GPU proof and actor representation comparison
**Acceptance:** Decision records platform targets, dependency approval, scene and stop criteria
**Negative control:** No full-engine migration is authorized by a successful triangle demo
**Dependencies:** VG-GOV-003
**Candidate owned paths:** `docs/execution/decisions/choose-the-renderer-trial-boundary.md`
**Integration reservation:** `orchestration authority`
**Existing packets:** TASK-0114
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R03, R04, R05, R06; task content is a new proposal.

### [ ] VG-GOV-006 — Rule on death and disconnect

**State:** DRAFT | **Gate:** G0 | **Priority:** P0 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Approve a state-transition table for mortality, quitting, connection loss and crashes
**Acceptance:** Every transition states carried-value, House-value and recovery consequences
**Negative control:** Disconnect cannot silently acknowledge uncommitted extraction
**Dependencies:** VG-GOV-003
**Candidate owned paths:** `docs/execution/decisions/rule-on-death-and-disconnect.md`
**Integration reservation:** `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R03, R04, R05, R06; task content is a new proposal.

### [ ] VG-GOV-007 — Approve bounded content budgets

**State:** DRAFT | **Gate:** G0 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Freeze slice quantities, candidate build examples and prohibited scope growth
**Acceptance:** Every approved content lot has a named owner and completion gate
**Negative control:** A new skill or biome without a budget replaces scope or stays deferred
**Dependencies:** VG-GOV-003
**Candidate owned paths:** `docs/execution/decisions/approve-bounded-content-budgets.md`
**Integration reservation:** `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R03, R04, R05, R06; task content is a new proposal.

### [ ] VG-GOV-008 — Audit dependency and path scheduling

**State:** DRAFT | **Gate:** G0 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Add a read-only validator for missing dependencies, cycles and path conflicts
**Acceptance:** Valid graph passes and intentional cycle, overlap and absent-ID fixtures fail
**Negative control:** Wildcard overlap and shared integration hooks are not treated as independent
**Dependencies:** VG-GOV-002, VG-GOV-004
**Candidate owned paths:** `docs/execution/decisions/audit-dependency-and-path-scheduling.md`
**Integration reservation:** `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R03, R04, R05, R06; task content is a new proposal.

## CORE — Canonical simulation and authority

Primary discipline: Core engineer. Integration seam to reserve: `native/src/core.cpp;native/include/verdigris/core.hpp`.

### [ ] VG-CORE-001 — Characterize the two simulation paths

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Create equivalent movement, attack and extraction fixtures for both existing models
**Acceptance:** A report records matching behavior and approved divergence by fixture
**Negative control:** Tests do not force accidental browser behavior into the native contract
**Dependencies:** VG-GOV-001, VG-GOV-004
**Candidate owned paths:** `native/systems/core/characterize-the-two-simulation-paths.hpp`, `native/tests/roadmap/core-001_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R02, R07; task content is a new proposal.

### [ ] VG-CORE-002 — Define typed identity and unit contracts

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Introduce agreed actor, item, instance, tick and duration boundary types
**Acceptance:** Invalid unit or identity substitution is caught at compile time or validation
**Negative control:** A millisecond value cannot be interpreted as a tick by magnitude
**Dependencies:** VG-GOV-001, VG-CORE-001
**Candidate owned paths:** `native/systems/core/define-typed-identity-and-unit-contracts.hpp`, `native/tests/roadmap/core-002_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R02, R07; task content is a new proposal.

### [ ] VG-CORE-003 — Own simulation time explicitly

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Separate queued commands from one fixed-step advancement call
**Acceptance:** Extra aim or duplicate packets do not advance cooldowns or movement time
**Negative control:** Flooding harmless commands cannot speed up a character
**Dependencies:** VG-GOV-001, VG-CORE-002
**Candidate owned paths:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`, `native/tests/roadmap/core-003_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R02, R07; task content is a new proposal.

### [ ] VG-CORE-004 — Extract account game-service state

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Move one profile aggregate boundary out of transport without changing rules
**Acceptance:** Local and remote adapters read the same typed profile-service interface
**Negative control:** Transport reducers cannot mutate the profile outside service commands
**Dependencies:** VG-GOV-001, VG-CORE-002, VG-SAVE-001
**Candidate owned paths:** `native/include/verdigris/networking.hpp`, `native/src/networking.cpp`, `native/tests/roadmap/core-004_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R02, R07; task content is a new proposal.

### [ ] VG-CORE-005 — Version gameplay random streams

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Separate and record combat, loot and generation RNG streams
**Acceptance:** Rendering or audio changes leave authoritative reward results unchanged
**Negative control:** Consuming cosmetic randomness cannot alter the next loot roll
**Dependencies:** VG-GOV-001, VG-CORE-002
**Candidate owned paths:** `native/systems/core/version-gameplay-random-streams.hpp`, `native/tests/roadmap/core-005_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R02, R07; task content is a new proposal.

### [ ] VG-CORE-006 — Freeze typed event envelopes

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Add event ID, simulation tick, instance ID and typed duration to the event contract
**Acceptance:** Consumer tests reject malformed duration and stale-instance events
**Negative control:** Reordered duplicate events cannot generate a second reward or death effect
**Dependencies:** VG-GOV-001, VG-CORE-002
**Candidate owned paths:** `native/systems/core/freeze-typed-event-envelopes.hpp`, `native/tests/roadmap/core-006_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R02, R07; task content is a new proposal.

### [ ] VG-CORE-007 — Route both modes through one command service

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Adapt one representative expedition to the canonical service
**Acceptance:** Local and remote replay resolve equivalent authoritative outcomes
**Negative control:** Remote mode cannot instantiate a second authoritative world
**Dependencies:** VG-GOV-001, VG-CORE-003, VG-CORE-004, VG-CORE-006
**Candidate owned paths:** `native/client/local_session.cpp`, `native/src/networking.cpp`, `native/tests/roadmap/core-007_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R02, R07; task content is a new proposal.

### [ ] VG-CORE-008 — Retire the superseded production path

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Remove one migrated duplicate rules path after equivalence approval
**Acceptance:** Build and journey evidence show no production caller remains
**Negative control:** A compatibility test may remain but cannot be reachable by release play
**Dependencies:** VG-GOV-001, VG-CORE-007, VG-QA-003
**Candidate owned paths:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`, `native/tests/roadmap/core-008_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R02, R07; task content is a new proposal.

## MOVE — Input, locomotion and collision

Primary discipline: Gameplay engineer. Integration seam to reserve: `native/client/remote_session.cpp;native/client/main.cpp`.

### [ ] VG-MOVE-001 — Preserve diagonal remote input

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Correct direction encoding for the eight movement vectors
**Acceptance:** All eight submitted directions produce the intended authoritative displacement
**Negative control:** Diagonal input must not collapse to the vertical component
**Dependencies:** VG-GOV-001, VG-GOV-004
**Candidate owned paths:** `native/client/remote_session.cpp`, `native/tests/session_tests.cpp`, `native/tests/roadmap/move-001_contract_test.cpp`
**Integration reservation:** `native/client/remote_session.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R11; task content is a new proposal.

### [ ] VG-MOVE-002 — Make aim independent of motion

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Preserve aim intent separately from locomotion input in the command adapter
**Acceptance:** Moving west while aiming east yields the expected attack direction
**Negative control:** A movement update does not overwrite a held aim direction
**Dependencies:** VG-GOV-001, VG-CORE-002, VG-MOVE-001
**Candidate owned paths:** `native/client/remote_session.cpp`, `native/client/local_session.cpp`, `native/tests/roadmap/move-002_contract_test.cpp`
**Integration reservation:** `native/client/remote_session.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R11; task content is a new proposal.

### [ ] VG-MOVE-003 — Normalize movement deterministically

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Implement the approved direction-speed rule in the canonical simulation
**Acceptance:** Cardinal and diagonal travel match the approved distance tolerance over equal ticks
**Negative control:** Packet count and render FPS do not change travel distance
**Dependencies:** VG-GOV-001, VG-CORE-003, VG-MOVE-001
**Candidate owned paths:** `native/client/input/normalize-movement-deterministically.hpp`, `native/tests/roadmap/move-003_contract_test.cpp`
**Integration reservation:** `native/client/remote_session.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R11; task content is a new proposal.

### [ ] VG-MOVE-004 — Sweep dash against collision

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Resolve a dash segment against the canonical collision query
**Acceptance:** A thin wall, corner and actor obstruction obey the approved dash rule
**Negative control:** Dash cannot tunnel through a wall at high movement speed
**Dependencies:** VG-GOV-001, VG-MOVE-003, VG-WORLD-002
**Candidate owned paths:** `native/client/input/sweep-dash-against-collision.hpp`, `native/tests/roadmap/move-004_contract_test.cpp`
**Integration reservation:** `native/client/remote_session.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R11; task content is a new proposal.

### [ ] VG-MOVE-005 — Suppress gameplay through focused panes

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Integrate existing focus arbitration for inventory and modal input
**Acceptance:** Click, drag and text-entry scenarios leave gameplay input suppressed as specified
**Negative control:** Closing a pane does not release a buffered attack into the world
**Dependencies:** VG-GOV-001, VG-GOV-004, VG-UI-001
**Candidate owned paths:** `native/client/main.cpp`, `native/tests/roadmap/move-005_contract_test.cpp`
**Integration reservation:** `native/client/remote_session.cpp`, `native/client/main.cpp`
**Existing packets:** TASK-0165
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R11; task content is a new proposal.

### [ ] VG-MOVE-006 — Persist remapped controls

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Add versioned binding serialization and conflict feedback
**Acceptance:** Remap, restart and restore defaults work without inaccessible actions
**Negative control:** Duplicate bindings and invalid device codes fail visibly
**Dependencies:** VG-GOV-001, VG-MOVE-005, VG-SHIP-001
**Candidate owned paths:** `native/client/input/persist-remapped-controls.hpp`, `native/tests/roadmap/move-006_contract_test.cpp`
**Integration reservation:** `native/client/remote_session.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R11; task content is a new proposal.

### [ ] VG-MOVE-007 — Bound action input buffering

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Add one explicit bounded action buffer around recovery windows
**Acceptance:** Early input executes once inside the approved window and expires outside it
**Negative control:** Stale buffered actions never fire after death or zone change
**Dependencies:** VG-GOV-001, VG-ACT-002, VG-MOVE-005
**Candidate owned paths:** `native/client/input/bound-action-input-buffering.hpp`, `native/tests/roadmap/move-007_contract_test.cpp`
**Integration reservation:** `native/client/remote_session.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R11; task content is a new proposal.

### [ ] VG-MOVE-008 — Measure native input response

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Add timestamped input-to-present markers and a manual measurement protocol
**Acceptance:** Report p50/p95 response on approved machines with method and uncertainty
**Negative control:** Headless command time alone is not labeled input-to-photon latency
**Dependencies:** VG-GOV-001, VG-MOVE-007, VG-PERF-001, VG-GPU-004
**Candidate owned paths:** `native/client/input/measure-native-input-response.hpp`, `native/tests/roadmap/move-008_contract_test.cpp`
**Integration reservation:** `native/client/remote_session.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R11; task content is a new proposal.

## ACT — Actions, targeting and impact

Primary discipline: Combat engineer. Integration seam to reserve: `native/src/core.cpp;native/client/presentation_state.cpp`.

### [ ] VG-ACT-001 — Define one action specification

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Author the schema for costs, phases, hit geometry and cancellation
**Acceptance:** One current melee action validates with no renderer-owned gameplay fields
**Negative control:** Undefined hit windows or negative costs fail validation
**Dependencies:** VG-GOV-001, VG-CORE-002, VG-GOV-007
**Candidate owned paths:** `native/systems/actions/define-one-action-specification.hpp`, `native/tests/roadmap/act-001_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ACT-002 — Implement phased melee resolution

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Run windup, active and recovery for the approved reference melee
**Acceptance:** Damage occurs only on the active phase at the correct tick
**Negative control:** Animation completion cannot independently award a hit
**Dependencies:** VG-GOV-001, VG-ACT-001, VG-CORE-003
**Candidate owned paths:** `native/systems/actions/implement-phased-melee-resolution.hpp`, `native/tests/roadmap/act-002_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ACT-003 — Resolve directional hit geometry

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Implement one authoritative thrust or arc shape with stable target ordering
**Acceptance:** Front, edge, rear and multiple-target fixtures match the authored shape
**Negative control:** A target behind a thrust cannot be hit by a presentation cone
**Dependencies:** VG-GOV-001, VG-ACT-002, VG-WORLD-002
**Candidate owned paths:** `native/systems/actions/resolve-directional-hit-geometry.hpp`, `native/tests/roadmap/act-003_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ACT-004 — Implement one bounded projectile

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Resolve one approved projectile with swept collision and lifetime
**Acceptance:** Target, wall, expiry and source attribution match a deterministic replay
**Negative control:** Projectile cannot hit twice after despawn or pass through thin cover
**Dependencies:** VG-GOV-001, VG-ACT-001, VG-WORLD-002, VG-CORE-006
**Candidate owned paths:** `native/systems/actions/implement-one-bounded-projectile.hpp`, `native/tests/roadmap/act-004_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** TASK-0108
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ACT-005 — Publish telegraph timing and geometry

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Replace a guessed telegraph duration with typed authoritative data
**Acceptance:** Local and remote consumers render the same warning window and footprint
**Negative control:** Expired or cancelled attacks leave no damaging invisible telegraph
**Dependencies:** VG-GOV-001, VG-CORE-006, VG-ACT-002
**Candidate owned paths:** `native/systems/actions/publish-telegraph-timing-and-geometry.hpp`, `native/tests/roadmap/act-005_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ACT-006 — Add one authoritative interrupt rule

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Implement the approved stagger or guard interruption for one action
**Acceptance:** The interrupt cancels scheduled hits and resolves costs per contract
**Negative control:** A cancelled action cannot still damage on its original active tick
**Dependencies:** VG-GOV-001, VG-ACT-002, VG-STAT-006
**Candidate owned paths:** `native/systems/actions/add-one-authoritative-interrupt-rule.hpp`, `native/tests/roadmap/act-006_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ACT-007 — Wire one complete attack presentation beat

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Connect authoritative attack, impact and cancel events to animation and sound
**Acceptance:** Real native play shows anticipation, impact and aftermath on the correct events
**Negative control:** Removing the event bridge makes the integration test fail
**Dependencies:** VG-GOV-001, VG-ACT-005, VG-ART-003, VG-SOUND-003, VG-GPU-004
**Candidate owned paths:** `native/systems/actions/wire-one-complete-attack-presentation-beat.hpp`, `native/tests/roadmap/act-007_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ACT-008 — Converge one local and remote combat journey

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Prove melee, projectile, cancellation and death through both session modes
**Acceptance:** Same seeded intents yield the accepted outcomes with independent visual evidence
**Negative control:** A green local scenario cannot substitute for the networked journey
**Dependencies:** VG-GOV-001, VG-ACT-003, VG-ACT-004, VG-ACT-006, VG-CORE-007, VG-QA-003
**Candidate owned paths:** `native/systems/actions/converge-one-local-and-remote-combat-journey.hpp`, `native/tests/roadmap/act-008_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

## STAT — Stats, damage and status algebra

Primary discipline: Systems engineer. Integration seam to reserve: `native/src/core.cpp;native/include/verdigris/core.hpp`.

### [ ] VG-STAT-001 — Specify modifier evaluation order

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Freeze flat, additive, multiplicative, conversion and cap order
**Acceptance:** Worked examples and executable fixtures produce identical totals
**Negative control:** Modifier container iteration order cannot change the result
**Dependencies:** VG-GOV-001, VG-CORE-002, VG-GOV-003
**Candidate owned paths:** `native/systems/stats/specify-modifier-evaluation-order.hpp`, `native/tests/roadmap/stat-001_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-STAT-002 — Implement the reference damage breakdown

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Return named intermediate values from one shared damage resolver
**Acceptance:** Character and monster fixtures use the same calculation and explain output
**Negative control:** Client-supplied final damage is ignored or rejected
**Dependencies:** VG-GOV-001, VG-STAT-001
**Candidate owned paths:** `native/systems/stats/implement-the-reference-damage-breakdown.hpp`, `native/tests/roadmap/stat-002_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-STAT-003 — Consume one resistance channel

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Apply a declared elemental resistance and approved cap
**Acceptance:** Zero, positive, capped and invalid resistance cases have pinned results
**Negative control:** Unrelated physical damage is not reduced by an elemental resistance
**Dependencies:** VG-GOV-001, VG-STAT-002
**Candidate owned paths:** `native/systems/stats/consume-one-resistance-channel.hpp`, `native/tests/roadmap/stat-003_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-STAT-004 — Make defensive equipment effective

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Feed one equipment defense or block property into the resolver
**Acceptance:** Swapping the test item changes survivability by the authored rule
**Negative control:** A tooltip-only defensive bonus is not counted as implemented
**Dependencies:** VG-GOV-001, VG-STAT-002, VG-ITEM-004
**Candidate owned paths:** `native/systems/stats/make-defensive-equipment-effective.hpp`, `native/tests/roadmap/stat-004_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-STAT-005 — Generalize timed effects

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Replace one hard-coded buff with a typed effect instance
**Acceptance:** Two simultaneous effects expire in deterministic order with defined stacking
**Negative control:** Expired effects never survive a lifecycle boundary accidentally
**Dependencies:** VG-GOV-001, VG-CORE-003, VG-STAT-001
**Candidate owned paths:** `native/systems/stats/generalize-timed-effects.hpp`, `native/tests/roadmap/stat-005_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-STAT-006 — Add bounded stagger accumulation

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Implement one approved poise/stagger rule for players and enemies
**Acceptance:** Threshold, decay and immunity-window fixtures use common actor rules
**Negative control:** Repeated weak hits cannot create an unbounded permanent lock
**Dependencies:** VG-GOV-001, VG-STAT-005, VG-GOV-007
**Candidate owned paths:** `native/systems/stats/add-bounded-stagger-accumulation.hpp`, `native/tests/roadmap/stat-006_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-STAT-007 — Bound proc chains and conversion

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Enforce max depth and per-tick effect work for triggered interactions
**Acceptance:** A cyclic trigger fixture terminates with deterministic rejection or truncation
**Negative control:** A proc loop cannot stall the authoritative tick
**Dependencies:** VG-GOV-001, VG-STAT-005, VG-BUILD-002
**Candidate owned paths:** `native/systems/stats/bound-proc-chains-and-conversion.hpp`, `native/tests/roadmap/stat-007_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-STAT-008 — Explain live character totals

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Feed one stat-breakdown view directly from derived authoritative values
**Acceptance:** Displayed and applied damage/defense match an integration fixture
**Negative control:** Displayed DPS does not include dormant or unsupported modifiers
**Dependencies:** VG-GOV-001, VG-STAT-003, VG-STAT-004, VG-UI-004
**Candidate owned paths:** `native/systems/stats/explain-live-character-totals.hpp`, `native/tests/roadmap/stat-008_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

## BUILD — Freeform builds and Arcane Lattice

Primary discipline: Systems designer / gameplay engineer. Integration seam to reserve: `native/src/core.cpp;native/client/main.cpp`.

### [ ] VG-BUILD-001 — Freeze three slice build fixtures

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Define distinct reach, pressure and magic build configurations
**Acceptance:** Each build lists tactics, weakness, gear dependency and expected encounter answers
**Negative control:** Three differently colored copies of one damage loop fail design review
**Dependencies:** VG-GOV-001, VG-GOV-007, VG-STAT-001
**Candidate owned paths:** `native/systems/builds/freeze-three-slice-build-fixtures.hpp`, `native/tests/roadmap/build-001_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R10; task content is a new proposal.

### [ ] VG-BUILD-002 — Define skill-transform compatibility

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Specify allowed action tags and exclusions for one bounded support layer
**Acceptance:** Compatible and incompatible transformations validate without runtime guessing
**Negative control:** An unsupported transformation never consumes resources silently
**Dependencies:** VG-GOV-001, VG-ACT-001, VG-BUILD-001
**Candidate owned paths:** `native/systems/builds/define-skill-transform-compatibility.hpp`, `native/tests/roadmap/build-002_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R10; task content is a new proposal.

### [ ] VG-BUILD-003 — Implement one shape-changing transform

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Add one approved support that changes hit form rather than only damage
**Acceptance:** With and without the transform show distinct target geometry and cost
**Negative control:** Removing the transform restores the original event stream
**Dependencies:** VG-GOV-001, VG-BUILD-002, VG-ACT-003
**Candidate owned paths:** `native/systems/builds/implement-one-shape-changing-transform.hpp`, `native/tests/roadmap/build-003_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R10; task content is a new proposal.

### [ ] VG-BUILD-004 — Integrate passive allocation validation

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Connect budget, adjacency and respec validation to canonical profile state
**Acceptance:** Valid allocation survives restart and invalid allocation leaves state untouched
**Negative control:** UI-local point spending cannot change authoritative actor stats
**Dependencies:** VG-GOV-001, VG-CORE-004, VG-SAVE-003, VG-STAT-001
**Candidate owned paths:** `native/systems/builds/integrate-passive-allocation-validation.hpp`, `native/tests/roadmap/build-004_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** TASK-0162, TASK-0193, TASK-0194
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R10; task content is a new proposal.

### [ ] VG-BUILD-005 — Validate bounded Arcane paths

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Reuse the reference lattice topology for a limited approved native subset
**Acceptance:** Invalid adjacency, disallowed tier and cyclic paths fail with readable errors
**Negative control:** A reference UI path is not assumed to be an implemented spell
**Dependencies:** VG-GOV-001, VG-GOV-004, VG-BUILD-002
**Candidate owned paths:** `native/systems/builds/validate-bounded-arcane-paths.hpp`, `native/tests/roadmap/build-005_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** TASK-0195, TASK-0196
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R10; task content is a new proposal.

### [ ] VG-BUILD-006 — Compile one Arcane path to an effect plan

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Translate one approved weave into existing action/effect primitives
**Acceptance:** Plan includes resource, timing, work budget and deterministic outcome
**Negative control:** Unknown manifestations fail closed rather than inventing behavior
**Dependencies:** VG-GOV-001, VG-BUILD-005, VG-ACT-004, VG-STAT-005
**Candidate owned paths:** `native/systems/builds/compile-one-arcane-path-to-an-effect-plan.hpp`, `native/tests/roadmap/build-006_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R10; task content is a new proposal.

### [ ] VG-BUILD-007 — Prevent loadout-swap exploits

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Validate equipment/skill swap transitions and resource reservations
**Acceptance:** Repeated swaps do not reset costs, duplicate effects or grant free resources
**Negative control:** A cooldown cannot be bypassed by swapping away and back
**Dependencies:** VG-GOV-001, VG-BUILD-003, VG-BUILD-004, VG-ITEM-004
**Candidate owned paths:** `native/systems/builds/prevent-loadout-swap-exploits.hpp`, `native/tests/roadmap/build-007_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R10; task content is a new proposal.

### [ ] VG-BUILD-008 — Compare three complete build journeys

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Run the three approved configurations against the same expedition
**Acceptance:** Evidence records different tactics, survivability and completion outcomes
**Negative control:** Completion by one dominant build does not approve all three
**Dependencies:** VG-GOV-001, VG-BUILD-006, VG-BUILD-007, VG-ENEMY-007, VG-QA-004
**Candidate owned paths:** `native/systems/builds/compare-three-complete-build-journeys.hpp`, `native/tests/roadmap/build-008_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R10; task content is a new proposal.

## ITEM — Item identity, equipment and loot

Primary discipline: Item systems engineer. Integration seam to reserve: `native/src/core.cpp;native/include/verdigris/core.hpp`.

### [ ] VG-ITEM-001 — Freeze canonical item ownership

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Separate definition, instance ID, roll seed, location and owner
**Acceptance:** Each live item has exactly one valid owner/location and a stable identity
**Negative control:** Two containers cannot own the same instance simultaneously
**Dependencies:** VG-GOV-001, VG-CORE-002, VG-GOV-004
**Candidate owned paths:** `native/systems/items/freeze-canonical-item-ownership.hpp`, `native/tests/roadmap/item-001_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R08; task content is a new proposal.

### [ ] VG-ITEM-002 — Validate base and modifier definitions

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Add schema checks for one item family and affix group rules
**Acceptance:** Valid definitions cook; unknown stats and conflicting groups fail
**Negative control:** A display-only mod cannot claim a gameplay effect without a binding
**Dependencies:** VG-GOV-001, VG-ITEM-001, VG-STAT-001, VG-TOOLS-001
**Candidate owned paths:** `native/systems/items/validate-base-and-modifier-definitions.hpp`, `native/tests/roadmap/item-002_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R08; task content is a new proposal.

### [ ] VG-ITEM-003 — Roll one deterministic loot family

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Produce one approved base/affix pool through the loot RNG stream
**Acceptance:** Fixed seed reproduces legal rolls and distribution checks report sample limits
**Negative control:** A cosmetic RNG call cannot reroll the item
**Dependencies:** VG-GOV-001, VG-ITEM-002, VG-CORE-005
**Candidate owned paths:** `native/systems/items/roll-one-deterministic-loot-family.hpp`, `native/tests/roadmap/item-003_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R08; task content is a new proposal.

### [ ] VG-ITEM-004 — Make equipment swaps atomic

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Apply equip, displaced-item return and derived stats as one command
**Acceptance:** Full bag, two-handed conflict and repeated request preserve all instances
**Negative control:** Failed equip cannot destroy the old item or duplicate the new one
**Dependencies:** VG-GOV-001, VG-ITEM-001, VG-CORE-004
**Candidate owned paths:** `native/systems/items/make-equipment-swaps-atomic.hpp`, `native/tests/roadmap/item-004_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** TASK-0171, TASK-0172, TASK-0184
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R08; task content is a new proposal.

### [ ] VG-ITEM-005 — Implement named ground pickup authority

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Validate distance, binding and instance membership before transfer
**Acceptance:** Two simultaneous pickup requests produce exactly one winner
**Negative control:** Stale-instance or distant item IDs cannot be collected
**Dependencies:** VG-GOV-001, VG-ITEM-001, VG-WORLD-002, VG-CORE-004
**Candidate owned paths:** `native/systems/items/implement-named-ground-pickup-authority.hpp`, `native/tests/roadmap/item-005_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R08; task content is a new proposal.

### [ ] VG-ITEM-006 — Expose loot-filter facts

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Publish safe item facts and filter ordering without gameplay mutation
**Acceptance:** Filter hides/shows categories and preserves manual accessibility
**Negative control:** A hidden item cannot change ownership or droprate
**Dependencies:** VG-GOV-001, VG-ITEM-003, VG-UI-001
**Candidate owned paths:** `native/systems/items/expose-loot-filter-facts.hpp`, `native/tests/roadmap/item-006_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R08; task content is a new proposal.

### [ ] VG-ITEM-007 — Add one behavior-changing relic

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Implement one approved item that changes reach, cost or action shape
**Acceptance:** Equipped and unequipped combat fixtures demonstrate the promised behavior
**Negative control:** A rarity color or large flat damage number alone does not pass
**Dependencies:** VG-GOV-001, VG-ITEM-004, VG-BUILD-003, VG-GOV-007
**Candidate owned paths:** `native/systems/items/add-one-behavior-changing-relic.hpp`, `native/tests/roadmap/item-007_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R08; task content is a new proposal.

### [ ] VG-ITEM-008 — Audit loot progression distribution

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Simulate the slice loot table over a versioned seed sample
**Acceptance:** Report time-to-upgrade quantiles, dry streaks and unusable-item rate
**Negative control:** Mean reward alone cannot approve severe tail failures
**Dependencies:** VG-GOV-001, VG-ITEM-003, VG-ITEM-007, VG-QA-004
**Candidate owned paths:** `native/systems/items/audit-loot-progression-distribution.hpp`, `native/tests/roadmap/item-008_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/include/verdigris/core.hpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R08; task content is a new proposal.

## FORGE — Brands, Bonds and crafting

Primary discipline: Economy designer / gameplay engineer. Integration seam to reserve: `native/src/core.cpp;native/client/main.cpp`.

### [ ] VG-FORGE-001 — Specify one House crafting recipe

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Define inputs, output mutation, eligibility and failure semantics
**Acceptance:** Recipe names one player decision and every resource source/sink
**Negative control:** No agent invents irreversible Brand costs beyond approved values
**Dependencies:** VG-GOV-001, VG-GOV-007, VG-ITEM-002
**Candidate owned paths:** `native/systems/crafting/specify-one-house-crafting-recipe.hpp`, `native/tests/roadmap/forge-001_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-FORGE-002 — Preview one Brand transformation

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Compute the exact eligible outcome range without mutating an item
**Acceptance:** Preview uses the same recipe and derived stat definitions as commit
**Negative control:** Cancelling preview consumes nothing and changes no RNG state
**Dependencies:** VG-GOV-001, VG-FORGE-001, VG-STAT-002
**Candidate owned paths:** `native/systems/crafting/preview-one-brand-transformation.hpp`, `native/tests/roadmap/forge-002_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-FORGE-003 — Commit Brand crafting transactionally

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Consume materials and mutate the existing item in one durable transaction
**Acceptance:** Retry, insufficient materials and server restart preserve exactly one result
**Negative control:** Duplicate request cannot spend twice or reroll a Brand
**Dependencies:** VG-GOV-001, VG-FORGE-002, VG-SAVE-004, VG-ITEM-004
**Candidate owned paths:** `native/systems/crafting/commit-brand-crafting-transactionally.hpp`, `native/tests/roadmap/forge-003_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** TASK-0198
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-FORGE-004 — Accumulate bounded Bond progress

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Count only approved meaningful events toward one Bond milestone
**Acceptance:** Capped progress and qualifying event rules replay deterministically
**Negative control:** Idle time or harmless action spam cannot farm unlimited Bond credit
**Dependencies:** VG-GOV-001, VG-CORE-006, VG-ITEM-001, VG-GOV-007
**Candidate owned paths:** `native/systems/crafting/accumulate-bounded-bond-progress.hpp`, `native/tests/roadmap/forge-004_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** TASK-0199
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-FORGE-005 — Activate one Bond milestone

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Apply one approved Bond ability or tradeoff at the threshold
**Acceptance:** Activation is durable, exactly once and reflected in derived behavior
**Negative control:** Reconnect cannot trigger the unlock twice
**Dependencies:** VG-GOV-001, VG-FORGE-004, VG-SAVE-004, VG-STAT-005
**Candidate owned paths:** `native/systems/crafting/activate-one-bond-milestone.hpp`, `native/tests/roadmap/forge-005_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-FORGE-006 — Define scar and recovery mutation

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Apply one approved scar to a recovered item while preserving provenance
**Acceptance:** The item keeps identity and a legible mechanical tradeoff
**Negative control:** Recovery never restores a dead Scion's complete inventory by accident
**Dependencies:** VG-GOV-001, VG-HOUSE-004, VG-ITEM-007, VG-GOV-006
**Candidate owned paths:** `native/systems/crafting/define-scar-and-recovery-mutation.hpp`, `native/tests/roadmap/forge-006_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-FORGE-007 — Wire the Brand and Bond pane

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Integrate existing models into the native inventory/House flow
**Acceptance:** A player previews, commits and inspects the changed item through real controls
**Negative control:** A model-only demo cannot satisfy the runtime gate
**Dependencies:** VG-GOV-001, VG-FORGE-003, VG-FORGE-005, VG-UI-003
**Candidate owned paths:** `native/systems/crafting/wire-the-brand-and-bond-pane.hpp`, `native/tests/roadmap/forge-007_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-FORGE-008 — Check crafting conservation and sinks

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Run a fixed recipe cycle through generation, spending and recovery
**Acceptance:** Material and item ledger balances except at named sources/sinks
**Negative control:** Repeated craft/salvage cannot generate net value without approved intent
**Dependencies:** VG-GOV-001, VG-FORGE-003, VG-FORGE-006, VG-ITEM-008
**Candidate owned paths:** `native/systems/crafting/check-crafting-conservation-and-sinks.hpp`, `native/tests/roadmap/forge-008_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

## SAVE — Durability and transactional progress

Primary discipline: Persistence engineer. Integration seam to reserve: `native/include/verdigris/networking.hpp;native/src/networking.cpp;native/src/server_main.cpp`.

### [ ] VG-SAVE-001 — Inventory the real durable profile

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Classify each live field as durable, reconstructible or transient
**Acceptance:** House, Scion, wear, bag, bank, passives, XP and recovery state are accounted for
**Negative control:** Saving only the small local Simulation cannot pass
**Dependencies:** VG-GOV-001, VG-GOV-004, VG-GOV-006
**Candidate owned paths:** `native/persistence/profile/inventory-the-real-durable-profile.hpp`, `native/tests/roadmap/save-001_contract_test.cpp`
**Integration reservation:** `native/include/verdigris/networking.hpp`, `native/src/networking.cpp`, `native/src/server_main.cpp`
**Existing packets:** TASK-0097
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R07; task content is a new proposal.

### [ ] VG-SAVE-002 — Freeze the versioned profile schema

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Define schema version, revision, identifiers and migration boundaries
**Acceptance:** A canonical fixture represents every approved durable field
**Negative control:** Unknown required versions fail without overwriting a valid save
**Dependencies:** VG-GOV-001, VG-SAVE-001, VG-ITEM-001
**Candidate owned paths:** `native/persistence/profile/freeze-the-versioned-profile-schema.hpp`, `native/tests/roadmap/save-002_contract_test.cpp`
**Integration reservation:** `native/include/verdigris/networking.hpp`, `native/src/networking.cpp`, `native/src/server_main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R07; task content is a new proposal.

### [ ] VG-SAVE-003 — Load and save one actual server profile

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Connect the profile repository to startup and authoritative state
**Acceptance:** Earned item and allocated passive survive full server termination/restart
**Negative control:** Same-process reconnect is not accepted as durability proof
**Dependencies:** VG-GOV-001, VG-SAVE-002, VG-CORE-004
**Candidate owned paths:** `native/persistence/profile/load-and-save-one-actual-server-profile.hpp`, `native/tests/roadmap/save-003_contract_test.cpp`
**Integration reservation:** `native/include/verdigris/networking.hpp`, `native/src/networking.cpp`, `native/src/server_main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R07; task content is a new proposal.

### [ ] VG-SAVE-004 — Add idempotent transaction records

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Record transaction ID, precondition revision and committed result
**Acceptance:** Replayed transaction returns the recorded result with no extra mutation
**Negative control:** Duplicate extraction/craft commands cannot award or spend twice
**Dependencies:** VG-GOV-001, VG-SAVE-003
**Candidate owned paths:** `native/persistence/profile/add-idempotent-transaction-records.hpp`, `native/tests/roadmap/save-004_contract_test.cpp`
**Integration reservation:** `native/include/verdigris/networking.hpp`, `native/src/networking.cpp`, `native/src/server_main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R07; task content is a new proposal.

### [ ] VG-SAVE-005 — Commit extraction as one durable operation

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Transfer carried value and House progression atomically
**Acceptance:** Crash around commit yields either complete old state or complete new state
**Negative control:** No carried-and-stored duplicate ownership is observable
**Dependencies:** VG-GOV-001, VG-SAVE-004, VG-GOV-006, VG-ITEM-005
**Candidate owned paths:** `native/persistence/profile/commit-extraction-as-one-durable-operation.hpp`, `native/tests/roadmap/save-005_contract_test.cpp`
**Integration reservation:** `native/include/verdigris/networking.hpp`, `native/src/networking.cpp`, `native/src/server_main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R07; task content is a new proposal.

### [ ] VG-SAVE-006 — Commit permanent death and succession

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Persist mortal death, item disposition and successor eligibility together
**Acceptance:** Restart cannot resurrect the fallen Scion or erase the crypt consequence
**Negative control:** Duplicate death events do not create duplicate relic candidates
**Dependencies:** VG-GOV-001, VG-SAVE-004, VG-GOV-006, VG-ITEM-001
**Candidate owned paths:** `native/persistence/profile/commit-permanent-death-and-succession.hpp`, `native/tests/roadmap/save-006_contract_test.cpp`
**Integration reservation:** `native/include/verdigris/networking.hpp`, `native/src/networking.cpp`, `native/src/server_main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R07; task content is a new proposal.

### [ ] VG-SAVE-007 — Add corrupt-save and migration recovery

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Implement backup selection and one tested schema migration
**Acceptance:** Truncation, invalid revision and prior-version fixtures recover or fail visibly
**Negative control:** Recovery never silently replaces progress with a fresh profile
**Dependencies:** VG-GOV-001, VG-SAVE-003
**Candidate owned paths:** `native/persistence/profile/add-corrupt-save-and-migration-recovery.hpp`, `native/tests/roadmap/save-007_contract_test.cpp`
**Integration reservation:** `native/include/verdigris/networking.hpp`, `native/src/networking.cpp`, `native/src/server_main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R07; task content is a new proposal.

### [ ] VG-SAVE-008 — Fault-inject the durable loop

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Kill the process at controlled transaction checkpoints on disposable profiles
**Acceptance:** Extraction, death and crafting preserve declared invariants across restart
**Negative control:** Never run destructive fault tests on real owner saves
**Dependencies:** VG-GOV-001, VG-SAVE-005, VG-SAVE-006, VG-SAVE-007, VG-FORGE-003
**Candidate owned paths:** `native/persistence/profile/fault-inject-the-durable-loop.hpp`, `native/tests/roadmap/save-008_contract_test.cpp`
**Integration reservation:** `native/include/verdigris/networking.hpp`, `native/src/networking.cpp`, `native/src/server_main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R07; task content is a new proposal.

## HOUSE — House, Scions and living history

Primary discipline: Progression engineer / designer. Integration seam to reserve: `native/src/networking.cpp;native/client/main.cpp`.

### [ ] VG-HOUSE-001 — Define horizontal House investment

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Specify one durable access or preparation unlock with bounded power
**Acceptance:** Benefit, cost and successor inheritance are explicit and approved
**Negative control:** Unlimited cumulative damage is not the default House reward
**Dependencies:** VG-GOV-001, VG-GOV-007, VG-GOV-006
**Candidate owned paths:** `native/systems/house/define-horizontal-house-investment.hpp`, `native/tests/roadmap/house-001_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R07; task content is a new proposal.

### [ ] VG-HOUSE-002 — Integrate one House investment

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Apply one approved investment using the canonical profile service
**Acceptance:** Cost and access survive restart and affect a real route or preparation option
**Negative control:** UI-only purchased state cannot unlock gameplay
**Dependencies:** VG-GOV-001, VG-HOUSE-001, VG-SAVE-004, VG-CORE-004
**Candidate owned paths:** `native/systems/house/integrate-one-house-investment.hpp`, `native/tests/roadmap/house-002_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/main.cpp`
**Existing packets:** TASK-0200, TASK-0201
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R07; task content is a new proposal.

### [ ] VG-HOUSE-003 — Bound the Legends event record

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Store milestone records and counters with a fixed retention policy
**Acceptance:** Repeated combat keeps history within its budget while retaining key events
**Negative control:** One string per hit cannot grow the save without bound
**Dependencies:** VG-GOV-001, VG-CORE-006, VG-SAVE-002
**Candidate owned paths:** `native/systems/house/bound-the-legends-event-record.hpp`, `native/tests/roadmap/house-003_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R07; task content is a new proposal.

### [ ] VG-HOUSE-004 — Create one relic recovery claim

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Represent a fallen item's recoverable eligibility and provenance
**Acceptance:** One candidate surfaces at most once and remains attributable to its owner
**Negative control:** Abandoning a spawned relic cannot duplicate its recovery eligibility
**Dependencies:** VG-GOV-001, VG-SAVE-006, VG-HOUSE-003, VG-ITEM-001
**Candidate owned paths:** `native/systems/house/create-one-relic-recovery-claim.hpp`, `native/tests/roadmap/house-004_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/main.cpp`
**Existing packets:** TASK-0202
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R07; task content is a new proposal.

### [ ] VG-HOUSE-005 — Reward a targeted recovery expedition

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Bind one disclosed recovery opportunity to route and reward rules
**Acceptance:** Real expedition recovery updates item ownership and crypt status together
**Negative control:** A client cannot choose an arbitrary lost item ID as a reward
**Dependencies:** VG-GOV-001, VG-HOUSE-004, VG-WORLD-005, VG-SAVE-004
**Candidate owned paths:** `native/systems/house/reward-a-targeted-recovery-expedition.hpp`, `native/tests/roadmap/house-005_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R07; task content is a new proposal.

### [ ] VG-HOUSE-006 — Complete the successor front door

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Wire death summary, successor creation and preparation into native UI
**Acceptance:** A player understands retained/lost value and returns to play with a successor
**Negative control:** The old Scion's complete bag cannot silently transfer
**Dependencies:** VG-GOV-001, VG-SAVE-006, VG-HOUSE-002, VG-UI-006
**Candidate owned paths:** `native/systems/house/complete-the-successor-front-door.hpp`, `native/tests/roadmap/house-006_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/main.cpp`
**Existing packets:** TASK-0145, TASK-0197
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R07; task content is a new proposal.

### [ ] VG-HOUSE-007 — Bound a named monster Legend

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Persist one escaped killer's bounded modifier/history record for later use
**Acceptance:** Replay produces a capped future encounter variant with readable provenance
**Negative control:** A past monster cannot gain unbounded levels from repeated records
**Dependencies:** VG-GOV-001, VG-HOUSE-003, VG-ENEMY-006, VG-GOV-007
**Candidate owned paths:** `native/systems/house/bound-a-named-monster-legend.hpp`, `native/tests/roadmap/house-007_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R07; task content is a new proposal.

### [ ] VG-HOUSE-008 — Prove a multigeneration House journey

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Complete expedition, craft, mortal death, successor and targeted recovery
**Acceptance:** Three-session test preserves history and creates meaningful changed choices
**Negative control:** A disconnected client cannot skip the approved loss policy
**Dependencies:** VG-GOV-001, VG-HOUSE-005, VG-HOUSE-006, VG-FORGE-006, VG-QA-003
**Candidate owned paths:** `native/systems/house/prove-a-multigeneration-house-journey.hpp`, `native/tests/roadmap/house-008_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R07; task content is a new proposal.

## WORLD — World layout and traversal

Primary discipline: World-generation engineer. Integration seam to reserve: `native/src/core.cpp;native/client/presentation_state.cpp`.

### [ ] VG-WORLD-001 — Freeze world coordinate conversion

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Specify tile, world, render and navigation conversions
**Acceptance:** Round-trip and boundary fixtures agree at walls, stairs and screen picking
**Negative control:** UI scaling must not alter authoritative collision coordinates
**Dependencies:** VG-GOV-001, VG-CORE-002
**Candidate owned paths:** `native/content/world/freeze-world-coordinate-conversion.hpp`, `native/tests/roadmap/world-001_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-WORLD-002 — Expose one canonical collision query

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Provide solid/line/swept queries over a validated instance layout
**Acceptance:** Collision and navigation agree on a fixed corridor/corner fixture
**Negative control:** A visually blocked wall cannot be traversable by the canonical query
**Dependencies:** VG-GOV-001, VG-WORLD-001, VG-CORE-001
**Candidate owned paths:** `native/content/world/expose-one-canonical-collision-query.hpp`, `native/tests/roadmap/world-002_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-WORLD-003 — Integrate one Cartographer seed

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Load one generated layout through the native content boundary
**Acceptance:** Same version and seed yield identical topology and metadata
**Negative control:** A content adapter cannot mutate the simulation RNG stream unexpectedly
**Dependencies:** VG-GOV-001, VG-GOV-004, VG-WORLD-001, VG-TOOLS-002, VG-CORE-005
**Candidate owned paths:** `native/content/world/integrate-one-cartographer-seed.hpp`, `native/tests/roadmap/world-003_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** TASK-0191, TASK-0192
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-WORLD-004 — Validate objective and exit reachability

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Add graph checks for entrance, exit, mandatory objective and safe spawn
**Acceptance:** A large seed corpus reports exact failing seeds and paths
**Negative control:** Intentionally blocked exits and unreachable bosses fail the gate
**Dependencies:** VG-GOV-001, VG-WORLD-003, VG-WORLD-002
**Candidate owned paths:** `native/content/world/validate-objective-and-exit-reachability.hpp`, `native/tests/roadmap/world-004_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-WORLD-005 — Publish a route decision contract

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Describe goal, danger, reward family, branch and return condition
**Acceptance:** Route UI consumes authored facts rather than names alone
**Negative control:** Missing reward or extraction semantics are visible validation failures
**Dependencies:** VG-GOV-001, VG-WORLD-003, VG-GOV-007
**Candidate owned paths:** `native/content/world/publish-a-route-decision-contract.hpp`, `native/tests/roadmap/world-005_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-WORLD-006 — Implement instance retirement rules

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Apply approved reentry, floor-drop and pending-recovery semantics
**Acceptance:** Reentering and retiring an instance follow the same authoritative policy
**Negative control:** Leaving a floor cannot duplicate ground drops or retire durable claims
**Dependencies:** VG-GOV-001, VG-GOV-006, VG-CORE-007, VG-ITEM-005
**Candidate owned paths:** `native/content/world/implement-instance-retirement-rules.hpp`, `native/tests/roadmap/world-006_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** TASK-0176, TASK-0189
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-WORLD-007 — Integrate one portal or gate interaction

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Connect the existing gate model to validated entry/return commands
**Acceptance:** Real controls enter and leave the correct instance with explicit failure feedback
**Negative control:** A presentation-only gate cannot grant route access
**Dependencies:** VG-GOV-001, VG-WORLD-005, VG-WORLD-006, VG-UI-006
**Candidate owned paths:** `native/content/world/integrate-one-portal-or-gate-interaction.hpp`, `native/tests/roadmap/world-007_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** TASK-0175, TASK-0188
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-WORLD-008 — Separate visual dressing from topology

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Apply a versioned decoration pass to one approved layout
**Acceptance:** Art changes leave collision, spawn and reward seeds unchanged
**Negative control:** A tree visual cannot create an unreported authoritative obstacle
**Dependencies:** VG-GOV-001, VG-WORLD-003, VG-ART-004, VG-GPU-005
**Candidate owned paths:** `native/content/world/separate-visual-dressing-from-topology.hpp`, `native/tests/roadmap/world-008_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

## ENEMY — Enemy roles and encounter direction

Primary discipline: AI engineer / encounter designer. Integration seam to reserve: `native/src/core.cpp;native/client/presentation_state.cpp`.

### [ ] VG-ENEMY-001 — Implement one pursuit role

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Give one enemy a bounded pursue, attack and disengage state machine
**Acceptance:** Enemy reaches legal melee contact and stops at authored leash conditions
**Negative control:** It cannot chase through walls or attack outside its geometry
**Dependencies:** VG-GOV-001, VG-WORLD-002, VG-ACT-002
**Candidate owned paths:** `native/systems/encounters/implement-one-pursuit-role.hpp`, `native/tests/roadmap/enemy-001_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ENEMY-002 — Finish the existing ranged-role packet

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Reuse or extend TASK-0108 for one visible ranged attack
**Acceptance:** Damage is preceded by an attributable warning and legal ranged hit
**Negative control:** The same enemy labeled melee does not perform the ranged attack
**Dependencies:** VG-GOV-001, VG-GOV-004, VG-ACT-004, VG-ACT-005
**Candidate owned paths:** `native/systems/encounters/finish-the-existing-ranged-role-packet.hpp`, `native/tests/roadmap/enemy-002_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** TASK-0108
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ENEMY-003 — Implement one support role

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Add one approved buff/heal action with target choice and cooldown
**Acceptance:** Support changes a pack interaction through the shared effects system
**Negative control:** Dead or out-of-range allies cannot receive the effect
**Dependencies:** VG-GOV-001, VG-STAT-005, VG-ENEMY-001
**Candidate owned paths:** `native/systems/encounters/implement-one-support-role.hpp`, `native/tests/roadmap/enemy-003_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ENEMY-004 — Add bounded local avoidance

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Resolve one multi-enemy congestion case around a doorway
**Acceptance:** A fixed pack does not stack into one point or permanently block itself
**Negative control:** Increased pack count must remain within the per-tick work budget
**Dependencies:** VG-GOV-001, VG-ENEMY-001, VG-WORLD-002, VG-PERF-002
**Candidate owned paths:** `native/systems/encounters/add-bounded-local-avoidance.hpp`, `native/tests/roadmap/enemy-004_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ENEMY-005 — Cook one mixed-role encounter

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Author one melee/ranged/support pack with spawn and escape constraints
**Acceptance:** The pack validates and produces more than one tactical priority
**Negative control:** Random spawn ordering cannot place the player under unavoidable instant damage
**Dependencies:** VG-GOV-001, VG-ENEMY-001, VG-ENEMY-002, VG-ENEMY-003, VG-WORLD-004
**Candidate owned paths:** `native/systems/encounters/cook-one-mixed-role-encounter.hpp`, `native/tests/roadmap/enemy-005_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ENEMY-006 — Constrain elite modifier composition

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Validate and instantiate one permitted elite combination
**Acceptance:** Budget, incompatibility and telegraph visibility rules are enforced
**Negative control:** An invalid combination cannot produce unavoidable overlapping threats
**Dependencies:** VG-GOV-001, VG-ENEMY-005, VG-STAT-007
**Candidate owned paths:** `native/systems/encounters/constrain-elite-modifier-composition.hpp`, `native/tests/roadmap/enemy-006_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ENEMY-007 — Implement one boss phase transition

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Add one approved boss phase using existing action primitives
**Acceptance:** Threshold transition cancels stale attacks and changes a readable pattern
**Negative control:** Phase changes cannot duplicate rewards or trigger dead attacks
**Dependencies:** VG-GOV-001, VG-ENEMY-005, VG-ACT-006, VG-GOV-007
**Candidate owned paths:** `native/systems/encounters/implement-one-boss-phase-transition.hpp`, `native/tests/roadmap/enemy-007_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

### [ ] VG-ENEMY-008 — Playtest encounter counterplay

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Record fixed-build runs against mixed pack and boss
**Acceptance:** Review identifies threat source, usable response and failure cause
**Negative control:** A successful damage sponge test does not prove readable counterplay
**Dependencies:** VG-GOV-001, VG-ENEMY-006, VG-ENEMY-007, VG-ACT-007, VG-QA-004
**Candidate owned paths:** `native/systems/encounters/playtest-encounter-counterplay.hpp`, `native/tests/roadmap/enemy-008_contract_test.cpp`
**Integration reservation:** `native/src/core.cpp`, `native/client/presentation_state.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R08; task content is a new proposal.

## STORY — Campaign, goals and authored spaces

Primary discipline: Level designer / narrative designer. Integration seam to reserve: `native/content;native/client/main.cpp`.

### [ ] VG-STORY-001 — Author the first goal card

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Write one concise expedition objective with explicit stakes and reward
**Acceptance:** New players can state why they are leaving and what safe return means
**Negative control:** The objective is not a checklist with unexplained nouns
**Dependencies:** VG-GOV-001, VG-GOV-007, VG-WORLD-005
**Candidate owned paths:** `native/content/campaign/author-the-first-goal-card.md`
**Integration reservation:** `native/content`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-STORY-002 — Author one combat room chunk

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Build one approved room with sightlines, retreat and collision metadata
**Acceptance:** Room runs with the reference pack without invalid spawn or blocked exit
**Negative control:** A decorative chokepoint cannot become an unavoidable kill trap
**Dependencies:** VG-GOV-001, VG-WORLD-002, VG-ART-004, VG-GOV-007
**Candidate owned paths:** `native/content/campaign/author-one-combat-room-chunk.md`
**Integration reservation:** `native/content`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-STORY-003 — Author one optional branch payoff

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Define one optional encounter and durable reward-access consequence
**Acceptance:** Taking or skipping the branch produces a documented difference
**Negative control:** The branch cannot become mandatory without the campaign graph declaring it
**Dependencies:** VG-GOV-001, VG-STORY-002, VG-HOUSE-002, VG-WORLD-005
**Candidate owned paths:** `native/content/campaign/author-one-optional-branch-payoff.md`
**Integration reservation:** `native/content`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-STORY-004 — Integrate one hub service interaction

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Connect an approved NPC/service prompt to House preparation
**Acceptance:** Real input opens the correct service and returns focus to gameplay
**Negative control:** A dialogue shell with no service command does not count
**Dependencies:** VG-GOV-001, VG-GOV-004, VG-UI-006, VG-HOUSE-002
**Candidate owned paths:** `native/content/campaign/integrate-one-hub-service-interaction.md`
**Integration reservation:** `native/content`, `native/client/main.cpp`
**Existing packets:** TASK-0177, TASK-0190
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-STORY-005 — Persist one House campaign edge

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Store the completion/unlock of one route edge at House scope
**Acceptance:** A successor retains that edge without replaying the mandatory node
**Negative control:** Scion replacement cannot reset the House campaign graph
**Dependencies:** VG-GOV-001, VG-SAVE-004, VG-WORLD-005
**Candidate owned paths:** `native/content/campaign/persist-one-house-campaign-edge.md`
**Integration reservation:** `native/content`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-STORY-006 — Add successor catch-up choice

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Implement one approved leveling or preparation alternative
**Acceptance:** A new Scion has a meaningful route that uses prior House knowledge
**Negative control:** Catch-up cannot grant another character's complete equipped build for free
**Dependencies:** VG-GOV-001, VG-HOUSE-006, VG-STORY-005, VG-GOV-007
**Candidate owned paths:** `native/content/campaign/add-successor-catch-up-choice.md`
**Integration reservation:** `native/content`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-STORY-007 — Assemble the first expedition route

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Connect approved rooms, objective, boss and return into one route
**Acceptance:** Fresh-profile native journey completes without developer grants
**Negative control:** A menu shortcut cannot bypass unimplemented traversal or rewards
**Dependencies:** VG-GOV-001, VG-STORY-001, VG-STORY-002, VG-ENEMY-007, VG-WORLD-007, VG-STORY-004
**Candidate owned paths:** `native/content/campaign/assemble-the-first-expedition-route.md`
**Integration reservation:** `native/content`, `native/client/main.cpp`
**Existing packets:** TASK-0203, TASK-0205
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-STORY-008 — Instantiate the second-biome content lot

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Create scoped child packets from the approved content manifest
**Acceptance:** Every room, enemy, audio and art asset has an individual integration gate
**Negative control:** A biome epic cannot be marked finished by creating its folders or tasks
**Dependencies:** VG-GOV-001, VG-QA-006, VG-TOOLS-005, VG-GOV-007
**Candidate owned paths:** `native/content/campaign/instantiate-the-second-biome-content-lot.md`
**Integration reservation:** `native/content`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

## END — Endgame, expedition choice and risk

Primary discipline: Endgame designer / systems engineer. Integration seam to reserve: `native/content;native/src/core.cpp`.

### [ ] VG-END-001 — Define one repeatable expedition contract

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Specify selectable target, risk modifier, reward pool and exit rule
**Acceptance:** Contract gives a concrete reason to run again beyond a larger level number
**Negative control:** Unbounded difficulty and reward scaling are excluded from the initial contract
**Dependencies:** VG-GOV-001, VG-GOV-007, VG-WORLD-005
**Candidate owned paths:** `native/systems/expeditions/define-one-repeatable-expedition-contract.hpp`, `native/tests/roadmap/end-001_contract_test.cpp`
**Integration reservation:** `native/content`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E03; task content is a new proposal.

### [ ] VG-END-002 — Implement one risk-reward modifier

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Apply an approved threat change and explicit reward consequence
**Acceptance:** The modifier changes encounter decisions and reports its reward adjustment
**Negative control:** More enemy life alone does not satisfy the interaction requirement
**Dependencies:** VG-GOV-001, VG-END-001, VG-ENEMY-006, VG-ITEM-003
**Candidate owned paths:** `native/systems/expeditions/implement-one-risk-reward-modifier.hpp`, `native/tests/roadmap/end-002_contract_test.cpp`
**Integration reservation:** `native/content`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E03; task content is a new proposal.

### [ ] VG-END-003 — Add target-family reward weighting

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Expose a bounded targeted item/material objective in the drop generator
**Acceptance:** Seeded samples show the documented conditional distribution
**Negative control:** Targeting cannot guarantee unrestricted best-in-slot outcomes
**Dependencies:** VG-GOV-001, VG-END-001, VG-ITEM-008
**Candidate owned paths:** `native/systems/expeditions/add-target-family-reward-weighting.hpp`, `native/tests/roadmap/end-003_contract_test.cpp`
**Integration reservation:** `native/content`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E03; task content is a new proposal.

### [ ] VG-END-004 — Unlock one endgame specialization choice

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Spend a House resource on one reversible or explicitly irreversible route modifier
**Acceptance:** The choice changes eligible expedition content and survives restart
**Negative control:** Respec cannot duplicate rewards or refund more than was spent
**Dependencies:** VG-GOV-001, VG-END-002, VG-HOUSE-002, VG-SAVE-004
**Candidate owned paths:** `native/systems/expeditions/unlock-one-endgame-specialization-choice.hpp`, `native/tests/roadmap/end-004_contract_test.cpp`
**Integration reservation:** `native/content`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E03; task content is a new proposal.

### [ ] VG-END-005 — Implement one relic-hunt chain

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Link a disclosed clue, encounter and recovery reward
**Acceptance:** Chain completion updates the unique recovery claim exactly once
**Negative control:** Abandon/retry cannot surface the same item in two expeditions
**Dependencies:** VG-GOV-001, VG-HOUSE-005, VG-END-003
**Candidate owned paths:** `native/systems/expeditions/implement-one-relic-hunt-chain.hpp`, `native/tests/roadmap/end-005_contract_test.cpp`
**Integration reservation:** `native/content`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E03; task content is a new proposal.

### [ ] VG-END-006 — Attach one optional mechanic module

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Reuse the seasonal boundary for one opt-in risk/reward demonstration
**Acceptance:** Module observes events, owns bounded state and detaches cleanly
**Negative control:** Removing the module leaves base combat and extraction semantics intact
**Dependencies:** VG-GOV-001, VG-CORE-006, VG-END-002, VG-TOOLS-002
**Candidate owned paths:** `native/systems/expeditions/attach-one-optional-mechanic-module.hpp`, `native/tests/roadmap/end-006_contract_test.cpp`
**Integration reservation:** `native/content`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E03; task content is a new proposal.

### [ ] VG-END-007 — Simulate endgame sources and sinks

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Model one approved expedition/crafting cycle over a seed cohort
**Acceptance:** Report material supply, craft consumption and runaway wealth conditions
**Negative control:** An average positive loop is not dismissed when exploit replay compounds it
**Dependencies:** VG-GOV-001, VG-END-003, VG-FORGE-008
**Candidate owned paths:** `native/systems/expeditions/simulate-endgame-sources-and-sinks.hpp`, `native/tests/roadmap/end-007_contract_test.cpp`
**Integration reservation:** `native/content`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E03; task content is a new proposal.

### [ ] VG-END-008 — Accept one repeatable endgame session

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Playtest target selection, modifier, reward and follow-on decision
**Acceptance:** Players can explain the next goal after two distinct runs
**Negative control:** Repeating the same corridor with inflated health does not pass
**Dependencies:** VG-GOV-001, VG-END-004, VG-END-005, VG-END-006, VG-END-007, VG-QA-004
**Candidate owned paths:** `native/systems/expeditions/accept-one-repeatable-endgame-session.hpp`, `native/tests/roadmap/end-008_contract_test.cpp`
**Integration reservation:** `native/content`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E03; task content is a new proposal.

## GPU — GPU renderer and platform surface

Primary discipline: Rendering engineer. Integration seam to reserve: `native/CMakeLists.txt;native/client/main.cpp`.

### [ ] VG-GPU-001 — Build an isolated cross-platform GPU sample

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Create the approved backend experiment outside the production core
**Acceptance:** Window, textured quad and clean shutdown run on Windows and macOS
**Negative control:** A Windows-only proof cannot settle the cross-platform decision
**Dependencies:** VG-GOV-001, VG-GOV-005
**Candidate owned paths:** `native/renderer/gpu/build-an-isolated-cross-platform-gpu-sample.hpp`, `native/tests/roadmap/gpu-001_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12, E01; task content is a new proposal.

### [ ] VG-GPU-002 — Preserve semantic render commands

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Adapt one recorded draw class to a backend-neutral render packet
**Acceptance:** Existing headless semantics remain deterministic without GPU state
**Negative control:** Backend resources cannot enter an authoritative snapshot
**Dependencies:** VG-GOV-001, VG-GPU-001, VG-CORE-006
**Candidate owned paths:** `native/client/render_list.hpp`, `native/tests/roadmap/gpu-002_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12, E01; task content is a new proposal.

### [ ] VG-GPU-003 — Cook shaders and resource bindings

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Produce a versioned shader artifact for each selected backend
**Acceptance:** Clean builds compile/load matching shaders without runtime source paths
**Negative control:** Wrong backend or stale layout fails explicitly rather than rendering silently
**Dependencies:** VG-GOV-001, VG-GPU-001, VG-TOOLS-002
**Candidate owned paths:** `native/renderer/gpu/cook-shaders-and-resource-bindings.hpp`, `native/tests/roadmap/gpu-003_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12, E01; task content is a new proposal.

### [ ] VG-GPU-004 — Render the native reference scene

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Draw actors, world, effects and HUD through the GPU presentation bridge
**Acceptance:** Packaged scene displays the approved composition on both platforms
**Negative control:** A standalone GPU demo disconnected from session events cannot pass
**Dependencies:** VG-GOV-001, VG-GPU-002, VG-GPU-003, VG-ART-001
**Candidate owned paths:** `native/renderer/gpu/render-the-native-reference-scene.hpp`, `native/tests/roadmap/gpu-004_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12, E01; task content is a new proposal.

### [ ] VG-GPU-005 — Implement grounding and occlusion

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Add one depth/height and contact-shadow policy for actors and scenery
**Acceptance:** Actor feet, walls and scenery sort consistently through movement
**Negative control:** A foreground wall cannot hide mandatory threat telegraphs completely
**Dependencies:** VG-GOV-001, VG-GPU-004, VG-WORLD-001
**Candidate owned paths:** `native/renderer/gpu/implement-grounding-and-occlusion.hpp`, `native/tests/roadmap/gpu-005_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12, E01; task content is a new proposal.

### [ ] VG-GPU-006 — Add one dynamic material-light interaction

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Implement one approved normal/emissive or 3D material effect
**Acceptance:** A moving light visibly affects the reference material without erasing contrast
**Negative control:** Overbright additive effects cannot conceal damage zones
**Dependencies:** VG-GOV-001, VG-GPU-005, VG-ART-002
**Candidate owned paths:** `native/renderer/gpu/add-one-dynamic-material-light-interaction.hpp`, `native/tests/roadmap/gpu-006_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12, E01; task content is a new proposal.

### [ ] VG-GPU-007 — Capture actual rendered output

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Add deterministic scene setup and backend-specific image readback
**Acceptance:** Captures include build/content/platform provenance and usable image files
**Negative control:** A semantic draw log is never substituted for pixel evidence
**Dependencies:** VG-GOV-001, VG-GPU-004, VG-QA-002
**Candidate owned paths:** `native/renderer/gpu/capture-actual-rendered-output.hpp`, `native/tests/roadmap/gpu-007_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12, E01; task content is a new proposal.

### [ ] VG-GPU-008 — Recover renderer resource loss

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Handle resize, scale change and backend recreation for the selected APIs
**Acceptance:** Repeated recreation preserves resources and displays visible errors on failure
**Negative control:** Minimize/restore cannot leak textures indefinitely or crash
**Dependencies:** VG-GOV-001, VG-GPU-004, VG-PERF-004
**Candidate owned paths:** `native/renderer/gpu/recover-renderer-resource-loss.hpp`, `native/tests/roadmap/gpu-008_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12, E01; task content is a new proposal.

## ART — Art direction, animation and asset quality

Primary discipline: Art director / technical artist. Integration seam to reserve: `native/client/assets;native/client/main.cpp`.

### [ ] VG-ART-001 — Approve one visual target sheet

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Establish camera, adult proportions, material palette and contrast hierarchy
**Acceptance:** Owner and art reviewer approve a named in-game composition target
**Negative control:** External concept images alone cannot count as in-game fidelity
**Dependencies:** VG-GOV-001, VG-GOV-003, VG-GOV-005, VG-GOV-007
**Candidate owned paths:** `native/client/assets/production/approve-one-visual-target-sheet.md`
**Integration reservation:** `native/client/assets`, `native/client/main.cpp`
**Existing packets:** TASK-0206
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06, E01; task content is a new proposal.

### [ ] VG-ART-002 — Create one production material family

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Deliver source and cooked maps for the reference bronze/stone family
**Acceptance:** Scale, lighting response and provenance match the asset specification
**Negative control:** Placeholder color fills do not pass as finished materials
**Dependencies:** VG-GOV-001, VG-ART-001, VG-TOOLS-003
**Candidate owned paths:** `native/client/assets/production/create-one-production-material-family.md`
**Integration reservation:** `native/client/assets`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06, E01; task content is a new proposal.

### [ ] VG-ART-003 — Produce one complete attack animation

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Deliver windup, active, recovery and cancel poses for one approved action
**Acceptance:** Poses align to simulation phase markers and remain readable at game scale
**Negative control:** Frame count alone cannot approve weight or timing
**Dependencies:** VG-GOV-001, VG-ART-001, VG-ACT-001, VG-TOOLS-003
**Candidate owned paths:** `native/client/assets/production/produce-one-complete-attack-animation.md`
**Integration reservation:** `native/client/assets`, `native/client/main.cpp`
**Existing packets:** TASK-0173, TASK-0186, TASK-0187
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06, E01; task content is a new proposal.

### [ ] VG-ART-004 — Produce one environment kit chunk

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Deliver one room/terrain kit with pivots, collision proxies and source files
**Acceptance:** Parts assemble without seams, scale drift or missing references
**Negative control:** Collision may not be baked only into an artist's local scene
**Dependencies:** VG-GOV-001, VG-ART-001, VG-TOOLS-003
**Candidate owned paths:** `native/client/assets/production/produce-one-environment-kit-chunk.md`
**Integration reservation:** `native/client/assets`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06, E01; task content is a new proposal.

### [ ] VG-ART-005 — Show one equipped item on the actor

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Bind one weapon/armor appearance to authoritative equipment identity
**Acceptance:** Equip/unequip changes the correct actor layer or attachment in real play
**Negative control:** A paper-doll icon alone does not satisfy world appearance
**Dependencies:** VG-GOV-001, VG-ART-003, VG-ITEM-004, VG-GPU-004
**Candidate owned paths:** `native/client/assets/production/show-one-equipped-item-on-the-actor.md`
**Integration reservation:** `native/client/assets`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06, E01; task content is a new proposal.

### [ ] VG-ART-006 — Create one coherent spell VFX family

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Deliver cast, travel, impact and cancellation effects for the reference weave
**Acceptance:** Effects share identity, fit budgets and preserve enemy-warning visibility
**Negative control:** A large screen-filling effect cannot pass on spectacle alone
**Dependencies:** VG-GOV-001, VG-BUILD-006, VG-ART-001, VG-GPU-006
**Candidate owned paths:** `native/client/assets/production/create-one-coherent-spell-vfx-family.md`
**Integration reservation:** `native/client/assets`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06, E01; task content is a new proposal.

### [ ] VG-ART-007 — Finish one mixed-pack silhouette review

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Review reference enemies at normal camera distance with low-color evidence
**Acceptance:** Each role remains identifiable by silhouette, motion or shape cues
**Negative control:** Color alone cannot distinguish critical enemy roles
**Dependencies:** VG-GOV-001, VG-ENEMY-005, VG-ART-003, VG-GPU-004
**Candidate owned paths:** `native/client/assets/production/finish-one-mixed-pack-silhouette-review.md`
**Integration reservation:** `native/client/assets`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06, E01; task content is a new proposal.

### [ ] VG-ART-008 — Approve one in-game fidelity capture set

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Compare ordinary-play captures with the approved target, scoring weaknesses
**Acceptance:** Lighting, actors, materials, VFX and UI each receive an explicit verdict
**Negative control:** Off-path beauty shots cannot substitute for the playable route
**Dependencies:** VG-GOV-001, VG-ART-004, VG-ART-005, VG-ART-006, VG-ART-007, VG-GPU-007, VG-UI-007
**Candidate owned paths:** `native/client/assets/production/approve-one-in-game-fidelity-capture-set.md`
**Integration reservation:** `native/client/assets`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06, E01; task content is a new proposal.

## UI — Interface, navigation and accessibility

Primary discipline: UI engineer / UX designer. Integration seam to reserve: `native/client/main.cpp`.

### [ ] VG-UI-001 — Integrate one pane ownership model

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Adopt current pane/focus foundations in the native shell
**Acceptance:** Open, close, stack and Escape behavior are deterministic through real input
**Negative control:** A pane-model unit test alone does not prove native integration
**Dependencies:** VG-GOV-001, VG-GOV-004
**Candidate owned paths:** `native/client/main.cpp`, `native/tests/roadmap/ui-001_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`
**Existing packets:** TASK-0158, TASK-0170
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-UI-002 — Integrate spatial inventory moves

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Connect grid drag and placement preview to authoritative move requests
**Acceptance:** Valid placement works and invalid placement returns the unchanged item
**Negative control:** Failed drag cannot lose, duplicate or equip an item silently
**Dependencies:** VG-GOV-001, VG-UI-001, VG-ITEM-004
**Candidate owned paths:** `native/client/ui/integrate-spatial-inventory-moves.hpp`, `native/tests/roadmap/ui-002_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`
**Existing packets:** TASK-0171, TASK-0184
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-UI-003 — Integrate equipment and comparison

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Connect paper doll, equip action and comparison tooltip to live data
**Acceptance:** Tooltip and actor stats update from the acknowledged equip result
**Negative control:** Optimistic UI state cannot pretend a rejected equip succeeded
**Dependencies:** VG-GOV-001, VG-UI-002, VG-ITEM-004
**Candidate owned paths:** `native/client/ui/integrate-equipment-and-comparison.hpp`, `native/tests/roadmap/ui-003_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`
**Existing packets:** TASK-0172, TASK-0184
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-UI-004 — Add readable stat explanations

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Show one expandable stat source breakdown and inactive-effect label
**Acceptance:** Player can distinguish base, gear, passive and conditional contributions
**Negative control:** Dormant values cannot be included as active damage
**Dependencies:** VG-GOV-001, VG-UI-001, VG-STAT-002
**Candidate owned paths:** `native/client/ui/add-readable-stat-explanations.hpp`, `native/tests/roadmap/ui-004_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-UI-005 — Implement map and route explanation

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Integrate side map, overlay and a route card with return/risk facts
**Acceptance:** Zoom, placement and opacity work without changing world semantics
**Negative control:** Map settings cannot reveal server-hidden targets unintentionally
**Dependencies:** VG-GOV-001, VG-WORLD-005, VG-UI-001, VG-GPU-004
**Candidate owned paths:** `native/client/ui/implement-map-and-route-explanation.hpp`, `native/tests/roadmap/ui-005_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-UI-006 — Integrate Chronicles and service navigation

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Connect House/Scion selection, service panes and error feedback
**Acceptance:** Keyboard/mouse flow creates, selects and prepares the authoritative character
**Negative control:** Local widget state cannot grant a House or select a dead Scion
**Dependencies:** VG-GOV-001, VG-UI-001, VG-CORE-004
**Candidate owned paths:** `native/client/ui/integrate-chronicles-and-service-navigation.hpp`, `native/tests/roadmap/ui-006_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-UI-007 — Validate scalable UI and non-color cues

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Test the slice at approved resolutions, DPI and accessible contrast settings
**Acceptance:** Text and tooltips fit; danger has shape or sound cues; controls remain usable
**Negative control:** Shrinking fonts below the agreed minimum does not solve overflow
**Dependencies:** VG-GOV-001, VG-UI-003, VG-UI-005, VG-UI-006, VG-GPU-004
**Candidate owned paths:** `native/client/ui/validate-scalable-ui-and-non-color-cues.hpp`, `native/tests/roadmap/ui-007_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

### [ ] VG-UI-008 — Complete one controller interaction path

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Map movement, targeting and inventory focus for one supported controller
**Acceptance:** The full reference loop works with controller hotplug and clear glyphs
**Negative control:** Mouse emulation alone does not count as controller UX acceptance
**Dependencies:** VG-GOV-001, VG-MOVE-006, VG-UI-007, VG-GOV-007
**Candidate owned paths:** `native/client/ui/complete-one-controller-interaction-path.hpp`, `native/tests/roadmap/ui-008_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, R06; task content is a new proposal.

## SOUND — Audio playback, feedback and mix

Primary discipline: Audio engineer / sound designer. Integration seam to reserve: `native/CMakeLists.txt;native/client/main.cpp`.

### [ ] VG-SOUND-001 — Choose and test one audio device adapter

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Implement the approved output backend behind the existing scheduler
**Acceptance:** A packaged generated test tone plays and shuts down on both platforms
**Negative control:** A scheduled cue without audible output does not pass
**Dependencies:** VG-GOV-001, VG-GOV-005
**Candidate owned paths:** `native/audio/runtime/choose-and-test-one-audio-device-adapter.hpp`, `native/tests/roadmap/sound-001_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** TASK-0157
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SOUND-002 — Package one legal sound family

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Add licensed/source-traceable swing, impact and warning assets
**Acceptance:** Manifest and cooked bank contain the approved variants and metadata
**Negative control:** Missing provenance blocks shipping even when playback works
**Dependencies:** VG-GOV-001, VG-SOUND-001, VG-TOOLS-003, VG-GOV-007
**Candidate owned paths:** `native/audio/runtime/package-one-legal-sound-family.hpp`, `native/tests/roadmap/sound-002_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SOUND-003 — Map authoritative combat beats to sound

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Connect attack, impact, warning and death events to actual playback
**Acceptance:** An ordinary fight emits the intended cues with no duplicated event playback
**Negative control:** Replaying the same event ID cannot double-play an impact
**Dependencies:** VG-GOV-001, VG-SOUND-002, VG-CORE-006
**Candidate owned paths:** `native/audio/runtime/map-authoritative-combat-beats-to-sound.hpp`, `native/tests/roadmap/sound-003_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** TASK-0204
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SOUND-004 — Enforce voice limits and priorities

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Bound voices and reserve priority for danger-critical warnings
**Acceptance:** Dense encounter test keeps warnings audible within the declared voice budget
**Negative control:** Low-priority cosmetic sounds cannot starve critical cues
**Dependencies:** VG-GOV-001, VG-SOUND-003, VG-PERF-002
**Candidate owned paths:** `native/audio/runtime/enforce-voice-limits-and-priorities.hpp`, `native/tests/roadmap/sound-004_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SOUND-005 — Add one environment ambience layer

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Integrate region ambience with spatial/zone transition rules
**Acceptance:** Enter/exit and crossfade have no clicks, runaway loops or missing assets
**Negative control:** Rapid zone reentry cannot accumulate additional ambient loops
**Dependencies:** VG-GOV-001, VG-SOUND-001, VG-WORLD-007
**Candidate owned paths:** `native/audio/runtime/add-one-environment-ambience-layer.hpp`, `native/tests/roadmap/sound-005_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SOUND-006 — Persist audio accessibility controls

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Save master/category volumes and relevant cue options
**Acceptance:** Relaunch preserves controls and zero-volume categories remain silent
**Negative control:** A critical-setting change cannot reset unrelated user preferences
**Dependencies:** VG-GOV-001, VG-SOUND-003, VG-SHIP-001, VG-UI-001
**Candidate owned paths:** `native/audio/runtime/persist-audio-accessibility-controls.hpp`, `native/tests/roadmap/sound-006_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SOUND-007 — Score one dense-combat mix

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Capture and review a repeatable mixed pack plus boss encounter
**Acceptance:** Review records audibility, dynamic range and cue attribution issues
**Negative control:** An isolated sound preview cannot prove the combat mix works
**Dependencies:** VG-GOV-001, VG-SOUND-004, VG-SOUND-005, VG-ENEMY-007, VG-QA-004
**Candidate owned paths:** `native/audio/runtime/score-one-dense-combat-mix.hpp`, `native/tests/roadmap/sound-007_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SOUND-008 — Add one music state transition

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Implement exploration/combat/recovery transitions for approved music
**Acceptance:** Phase changes transition cleanly with explicit restart and mute behavior
**Negative control:** Paused or unloaded scenes cannot leave competing music states active
**Dependencies:** VG-GOV-001, VG-SOUND-006, VG-STORY-007, VG-GOV-007
**Candidate owned paths:** `native/audio/runtime/add-one-music-state-transition.hpp`, `native/tests/roadmap/sound-008_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/client/main.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

## NET — Replication and cooperative play

Primary discipline: Network engineer. Integration seam to reserve: `native/src/networking.cpp;native/client/remote_session.cpp`.

### [ ] VG-NET-001 — Version the production snapshot contract

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Define an authenticated gameplay snapshot/delta surface to replace dev polling
**Acceptance:** Contract names sequence, tick, entity lifetime and profile revision
**Negative control:** dev:state is not the required release synchronization API
**Dependencies:** VG-GOV-001, VG-CORE-006, VG-CORE-004
**Candidate owned paths:** `native/networking/replication/version-the-production-snapshot-contract.hpp`, `native/tests/roadmap/net-001_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R11; task content is a new proposal.

### [ ] VG-NET-002 — Implement input acknowledgement

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Stamp accepted inputs with sequence and authoritative tick
**Acceptance:** Duplicate and out-of-window inputs are rejected without additional motion
**Negative control:** Packet replay cannot grant more simulation steps
**Dependencies:** VG-GOV-001, VG-NET-001, VG-CORE-003
**Candidate owned paths:** `native/networking/replication/implement-input-acknowledgement.hpp`, `native/tests/roadmap/net-002_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R11; task content is a new proposal.

### [ ] VG-NET-003 — Predict and reconcile local movement

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Add bounded non-authoritative prediction for the player actor
**Acceptance:** Delayed acknowledgements converge without persistent position drift
**Negative control:** Prediction cannot authorize hits, pickups or extraction
**Dependencies:** VG-GOV-001, VG-NET-002, VG-MOVE-003
**Candidate owned paths:** `native/networking/replication/predict-and-reconcile-local-movement.hpp`, `native/tests/roadmap/net-003_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R11; task content is a new proposal.

### [ ] VG-NET-004 — Interpolate remote actor snapshots

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Buffer and interpolate other actors with explicit discontinuity rules
**Acceptance:** Jittered updates remain smooth and teleport/death reset interpolation
**Negative control:** Interpolation never resurrects a despawned actor
**Dependencies:** VG-GOV-001, VG-NET-001, VG-GPU-004
**Candidate owned paths:** `native/networking/replication/interpolate-remote-actor-snapshots.hpp`, `native/tests/roadmap/net-004_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R11; task content is a new proposal.

### [ ] VG-NET-005 — Resynchronize after connection loss

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Restore a full authoritative snapshot with event/revision deduplication
**Acceptance:** Reconnect preserves durable state and discards stale prediction
**Negative control:** Silent local fallback and duplicate reward presentation fail the test
**Dependencies:** VG-GOV-001, VG-NET-001, VG-SAVE-003, VG-CORE-006
**Candidate owned paths:** `native/client/remote_session.cpp`, `native/tests/roadmap/net-005_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R11; task content is a new proposal.

### [ ] VG-NET-006 — Isolate two players in a shared instance

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Move per-player combat/input state out of shared mutable singleton fields
**Acceptance:** Two clients retain independent targets, resources, cooldowns and death states
**Negative control:** Player two cannot inherit player one's active target or attacks
**Dependencies:** VG-GOV-001, VG-CORE-007, VG-NET-002, VG-ACT-008
**Candidate owned paths:** `native/networking/replication/isolate-two-players-in-a-shared-instance.hpp`, `native/tests/roadmap/net-006_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R11; task content is a new proposal.

### [ ] VG-NET-007 — Resolve cooperative loot ownership

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Apply the approved loot allocation rule to simultaneous party pickup
**Acceptance:** Exactly one ownership transfer occurs and every client sees the result
**Negative control:** Repeated pickup or disconnect cannot duplicate a party item
**Dependencies:** VG-GOV-001, VG-NET-006, VG-ITEM-005, VG-SAVE-004, VG-GOV-007
**Candidate owned paths:** `native/networking/replication/resolve-cooperative-loot-ownership.hpp`, `native/tests/roadmap/net-007_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R11; task content is a new proposal.

### [ ] VG-NET-008 — Run an impaired-network party journey

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Exercise two native clients under delay, jitter, disconnect and restart
**Acceptance:** Fight, pickup, equip, extraction and reconnect pass with recorded impairment
**Negative control:** Loopback-only success cannot certify wide-area playability
**Dependencies:** VG-GOV-001, VG-NET-003, VG-NET-004, VG-NET-005, VG-NET-007, VG-SEC-007, VG-QA-005
**Candidate owned paths:** `native/networking/replication/run-an-impaired-network-party-journey.hpp`, `native/tests/roadmap/net-008_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R11; task content is a new proposal.

## SEC — Authority, abuse resistance and security

Primary discipline: Security-minded systems engineer. Integration seam to reserve: `native/src/networking.cpp;native/client/remote_session.cpp`.

### [ ] VG-SEC-001 — Bound JSON nesting and allocation

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Add explicit depth, token and byte budgets to the existing parser
**Acceptance:** Deep and wide malformed inputs fail without crash or state mutation
**Negative control:** A frame-size limit alone cannot satisfy the recursion-depth requirement
**Dependencies:** VG-GOV-001, VG-GOV-004
**Candidate owned paths:** `native/src/networking.cpp`, `native/tests/roadmap/sec-001_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R09; task content is a new proposal.

### [ ] VG-SEC-002 — Bound route identifiers and generation work

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Validate tier/index against allowed frontier and compute within fixed limits
**Acceptance:** Extreme and invalid node IDs are rejected before generation
**Negative control:** Client-supplied depth cannot trigger unbounded recursion
**Dependencies:** VG-GOV-001, VG-GOV-004, VG-WORLD-005
**Candidate owned paths:** `native/src/networking.cpp`, `native/tests/roadmap/sec-002_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R09; task content is a new proposal.

### [ ] VG-SEC-003 — Derive economic values on the server

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Ignore echoed prices and validate positive bounded quantities
**Acceptance:** Invalid purchases and bank transfers leave the ledger unchanged
**Negative control:** Negative quantity or zero client price cannot mint value
**Dependencies:** VG-GOV-001, VG-ITEM-001, VG-CORE-004
**Candidate owned paths:** `native/src/networking.cpp`, `native/tests/roadmap/sec-003_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R09; task content is a new proposal.

### [ ] VG-SEC-004 — Remove release access to dev mutation

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Gate developer endpoints behind explicit non-release configuration
**Acceptance:** Packaged release rejects grants, teleports and artificial death commands
**Negative control:** A normal guest cannot enable developer mode through an envelope
**Dependencies:** VG-GOV-001, VG-NET-001, VG-SHIP-002
**Candidate owned paths:** `native/src/networking.cpp`, `native/tests/roadmap/sec-004_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R09; task content is a new proposal.

### [ ] VG-SEC-005 — Add connection and session work limits

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Bound per-client rate, queue, memory and disconnected-session retention
**Acceptance:** Flood fixtures are throttled and inactive sessions follow safe eviction
**Negative control:** Session eviction cannot discard uncommitted durable state
**Dependencies:** VG-GOV-001, VG-SAVE-003, VG-NET-002
**Candidate owned paths:** `native/networking/validation/add-connection-and-session-work-limits.hpp`, `native/tests/roadmap/sec-005_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R09; task content is a new proposal.

### [ ] VG-SEC-006 — Authenticate online profile access

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Bind session identity to an approved credential and authorization policy
**Acceptance:** Wrong account cannot replace or load another profile by replaying an ID
**Negative control:** A guestId string is not accepted as public-server authentication
**Dependencies:** VG-GOV-001, VG-GOV-007, VG-CORE-004, VG-SEC-005
**Candidate owned paths:** `native/networking/validation/authenticate-online-profile-access.hpp`, `native/tests/roadmap/sec-006_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R09; task content is a new proposal.

### [ ] VG-SEC-007 — Approve secure network deployment

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Add encrypted transport/deployment policy and an isolated hosting test
**Acceptance:** Credential handling, transport and environment configuration pass review
**Negative control:** The loopback prototype is never exposed by merely changing bind address
**Dependencies:** VG-GOV-001, VG-SEC-001, VG-SEC-002, VG-SEC-003, VG-SEC-004, VG-SEC-006
**Candidate owned paths:** `native/networking/validation/approve-secure-network-deployment.hpp`, `native/tests/roadmap/sec-007_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R09; task content is a new proposal.

### [ ] VG-SEC-008 — Fuzz state-changing boundaries

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Run bounded parser/command/save fuzzers on disposable fixtures
**Acceptance:** Crashes, leaks and invariant failures retain minimized repro inputs
**Negative control:** A fuzzer finding cannot be silenced by weakening the invariant
**Dependencies:** VG-GOV-001, VG-SEC-001, VG-SEC-003, VG-SAVE-007, VG-QA-007
**Candidate owned paths:** `native/networking/validation/fuzz-state-changing-boundaries.hpp`, `native/tests/roadmap/sec-008_contract_test.cpp`
**Integration reservation:** `native/src/networking.cpp`, `native/client/remote_session.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R09; task content is a new proposal.

## TOOLS — Content compiler and authoring tools

Primary discipline: Tools engineer. Integration seam to reserve: `native/content;native/CMakeLists.txt`.

### [ ] VG-TOOLS-001 — Unify content IDs and schema validation

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Add a shared ID/reference policy for one content manifest
**Acceptance:** Duplicate IDs, dangling references and unknown fields are reported precisely
**Negative control:** Two unrelated asset packs cannot silently reuse one runtime ID
**Dependencies:** VG-GOV-001, VG-GOV-004
**Candidate owned paths:** `native/tools/content/unify-content-ids-and-schema-validation.py`, `native/tests/roadmap/tools-001_contract_test.cpp`
**Integration reservation:** `native/content`, `native/CMakeLists.txt`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-TOOLS-002 — Cook a versioned runtime bundle

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Compile one validated content set into a version/hash-addressed package
**Acceptance:** Same source inputs and tool version reproduce the same bundle manifest
**Negative control:** Client and server mismatched content versions fail explicitly
**Dependencies:** VG-GOV-001, VG-TOOLS-001
**Candidate owned paths:** `native/tools/content/cook-a-versioned-runtime-bundle.py`, `native/tests/roadmap/tools-002_contract_test.cpp`
**Integration reservation:** `native/content`, `native/CMakeLists.txt`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-TOOLS-003 — Require asset provenance at cooking

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Record source, license/permission, authoring recipe and output hash
**Acceptance:** Missing or stale provenance blocks the distributable asset package
**Negative control:** An asset filename is not proof of permission or origin
**Dependencies:** VG-GOV-001, VG-GOV-004, VG-TOOLS-001
**Candidate owned paths:** `native/tools/content/require-asset-provenance-at-cooking.py`, `native/tests/roadmap/tools-003_contract_test.cpp`
**Integration reservation:** `native/content`, `native/CMakeLists.txt`
**Existing packets:** TASK-0166, TASK-0167, TASK-0168, TASK-0169, TASK-0179
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-TOOLS-004 — Provide one content preview utility

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Preview a selected item, actor or room in the same runtime definitions
**Acceptance:** Editing approved content changes the preview without touching combat code
**Negative control:** Preview cannot introduce rules unavailable to the production client
**Dependencies:** VG-GOV-001, VG-TOOLS-002, VG-GPU-004
**Candidate owned paths:** `native/tools/content/provide-one-content-preview-utility.py`, `native/tests/roadmap/tools-004_contract_test.cpp`
**Integration reservation:** `native/content`, `native/CMakeLists.txt`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-TOOLS-005 — Generate scoped content-lot packets

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Expand one approved asset manifest into bounded task cards
**Acceptance:** Each asset has spec, implementation/art, integration and review dependencies
**Negative control:** Creating child tasks never marks their parent content lot complete
**Dependencies:** VG-GOV-001, VG-GOV-008, VG-TOOLS-001
**Candidate owned paths:** `native/tools/content/generate-scoped-content-lot-packets.py`, `native/tests/roadmap/tools-005_contract_test.cpp`
**Integration reservation:** `native/content`, `native/CMakeLists.txt`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-TOOLS-006 — Reload one development content category

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Implement safe development-only reload with version boundaries
**Acceptance:** Reload updates the selected category or reports a visible validation error
**Negative control:** Existing sessions never mix two incompatible gameplay definition versions
**Dependencies:** VG-GOV-001, VG-TOOLS-002, VG-CORE-004
**Candidate owned paths:** `native/tools/content/reload-one-development-content-category.py`, `native/tests/roadmap/tools-006_contract_test.cpp`
**Integration reservation:** `native/content`, `native/CMakeLists.txt`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-TOOLS-007 — Create a deterministic build sandbox

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Load an approved build/seed encounter for comparison without production grants
**Acceptance:** Sandbox records build/content/seed and cannot alter real profiles
**Negative control:** A debug sandbox result is labeled separately from ordinary-play evidence
**Dependencies:** VG-GOV-001, VG-BUILD-001, VG-TOOLS-002, VG-QA-001
**Candidate owned paths:** `native/tools/content/create-a-deterministic-build-sandbox.py`, `native/tests/roadmap/tools-007_contract_test.cpp`
**Integration reservation:** `native/content`, `native/CMakeLists.txt`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-TOOLS-008 — Cook a package without the source checkout

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Validate every runtime dependency from a clean staged directory
**Acceptance:** Reference route loads with no absolute developer paths or missing files
**Negative control:** A file available only beside the repository cannot mask packaging failure
**Dependencies:** VG-GOV-001, VG-TOOLS-002, VG-TOOLS-003, VG-SHIP-002
**Candidate owned paths:** `native/tools/content/cook-a-package-without-the-source-checkout.py`, `native/tests/roadmap/tools-008_contract_test.cpp`
**Integration reservation:** `native/content`, `native/CMakeLists.txt`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

## QA — Journeys, tests and evidence

Primary discipline: QA / automation engineer. Integration seam to reserve: `native/tests;native/tools/ci-native.ps1`.

### [ ] VG-QA-001 — Standardize evidence manifests

**State:** DRAFT | **Gate:** G0 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Define build, content, seed, commands, machine and artifact hash records
**Acceptance:** A test result can be traced to an exact implementation and environment
**Negative control:** Screenshots without provenance cannot certify a current task
**Dependencies:** VG-GOV-001
**Candidate owned paths:** `native/tests/journeys/standardize-evidence-manifests.py`, `native/tests/roadmap/qa-001_contract_test.cpp`
**Integration reservation:** `native/tests`, `native/tools/ci-native.ps1`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R06; task content is a new proposal.

### [ ] VG-QA-002 — Preserve headless presentation contracts

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Add a fixture mapping one authoritative event to a semantic render/audio intent
**Acceptance:** The fixture fails when the intended bridge is removed
**Negative control:** A mocked event not emitted by the game cannot prove the whole journey
**Dependencies:** VG-GOV-001, VG-CORE-006, VG-QA-001
**Candidate owned paths:** `native/tests/journeys/preserve-headless-presentation-contracts.py`, `native/tests/roadmap/qa-002_contract_test.cpp`
**Integration reservation:** `native/tests`, `native/tools/ci-native.ps1`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R06; task content is a new proposal.

### [ ] VG-QA-003 — Run the real guest expedition

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Drive login, move, combat, loot, equip and return over the supported session
**Acceptance:** Fresh disposable profile completes without direct state mutation
**Negative control:** Dev grants and preloaded success states invalidate the run
**Dependencies:** VG-GOV-001, VG-CORE-007, VG-ITEM-004, VG-ITEM-005, VG-ACT-005
**Candidate owned paths:** `native/tests/journeys/run-the-real-guest-expedition.py`, `native/tests/roadmap/qa-003_contract_test.cpp`
**Integration reservation:** `native/tests`, `native/tools/ci-native.ps1`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R06; task content is a new proposal.

### [ ] VG-QA-004 — Specify fresh-player playtest scoring

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Create a repeatable protocol for comprehension, danger, reward and agency
**Acceptance:** Observer records behavior and errors, not only satisfaction comments
**Negative control:** Developer coaching is logged and cannot be hidden as independent success
**Dependencies:** VG-GOV-001, VG-GOV-003, VG-GOV-007
**Candidate owned paths:** `native/tests/journeys/specify-fresh-player-playtest-scoring.py`, `native/tests/roadmap/qa-004_contract_test.cpp`
**Integration reservation:** `native/tests`, `native/tools/ci-native.ps1`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R06; task content is a new proposal.

### [ ] VG-QA-005 — Inject repeatable network impairment

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Add a test proxy or harness for latency, jitter, loss and disconnect
**Acceptance:** Scenario records exact impairment schedule and failure recovery evidence
**Negative control:** Network success without the enabled impairment fails the test setup
**Dependencies:** VG-GOV-001, VG-NET-001, VG-QA-001
**Candidate owned paths:** `native/tests/journeys/inject-repeatable-network-impairment.py`, `native/tests/roadmap/qa-005_contract_test.cpp`
**Integration reservation:** `native/tests`, `native/tools/ci-native.ps1`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R06; task content is a new proposal.

### [ ] VG-QA-006 — Accept the persistent quality slice

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Combine ordinary-play, restart, fidelity, audio and three-build evidence
**Acceptance:** All slice dimensions pass independently on the named build and content
**Negative control:** No averaged score can hide a zero in integrity or readability
**Dependencies:** VG-GOV-001, VG-QA-003, VG-SAVE-008, VG-HOUSE-008, VG-BUILD-008, VG-STORY-007, VG-ART-008, VG-SOUND-007, VG-PERF-008, VG-SHIP-004
**Candidate owned paths:** `native/tests/journeys/accept-the-persistent-quality-slice.py`, `native/tests/roadmap/qa-006_contract_test.cpp`
**Integration reservation:** `native/tests`, `native/tools/ci-native.ps1`
**Existing packets:** TASK-0205, TASK-0206, TASK-0207, TASK-0208
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R06; task content is a new proposal.

### [ ] VG-QA-007 — Enable sanitizers and boundary tests

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Add supported sanitizer builds and parser/save regression corpora
**Acceptance:** Clean instrumented runs record toolchain and unsupported-platform limitations
**Negative control:** Release-only test success does not substitute for instrumented checks
**Dependencies:** VG-GOV-001, VG-CORE-002
**Candidate owned paths:** `native/tests/journeys/enable-sanitizers-and-boundary-tests.py`, `native/tests/roadmap/qa-007_contract_test.cpp`
**Integration reservation:** `native/tests`, `native/tools/ci-native.ps1`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R06; task content is a new proposal.

### [ ] VG-QA-008 — Freeze a release-candidate evidence bundle

**State:** DRAFT | **Gate:** G5 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Gather signed-off journeys, defects, migrations and package hashes
**Acceptance:** Every must-pass gate references the exact candidate and blockers are closed
**Negative control:** Accepted work on an earlier SHA cannot certify a changed candidate
**Dependencies:** VG-GOV-001, VG-QA-006, VG-NET-008, VG-SEC-008, VG-SHIP-006, VG-LIVE-008
**Candidate owned paths:** `native/tests/journeys/freeze-a-release-candidate-evidence-bundle.py`, `native/tests/roadmap/qa-008_contract_test.cpp`
**Integration reservation:** `native/tests`, `native/tools/ci-native.ps1`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R02, R06; task content is a new proposal.

## PERF — Performance, budgets and scaling

Primary discipline: Performance engineer. Integration seam to reserve: `native/client/main.cpp;native/src/core.cpp`.

### [ ] VG-PERF-001 — Name reference machines and trace schema

**State:** DRAFT | **Gate:** G0 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Record CPU/GPU/OS/display and per-frame trace fields
**Acceptance:** Capture includes simulation, rendering, upload, UI and network timing
**Negative control:** Unnamed hardware cannot establish a portable performance target
**Dependencies:** VG-GOV-001, VG-GOV-003
**Candidate owned paths:** `native/tools/performance/name-reference-machines-and-trace-schema.py`, `native/tests/roadmap/perf-001_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-PERF-002 — Budget per-tick gameplay work

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Define and instrument actor, projectile, effect and query work limits
**Acceptance:** Dense fixture reports p50/p95/p99 tick cost and budget breaches
**Negative control:** An entity-count-only benchmark cannot claim GPU performance
**Dependencies:** VG-GOV-001, VG-CORE-003, VG-PERF-001
**Candidate owned paths:** `native/tools/performance/budget-per-tick-gameplay-work.py`, `native/tests/roadmap/perf-002_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-PERF-003 — Batch one expensive draw category

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Optimize one measured sprite/effect category without changing appearance
**Acceptance:** Before/after traces show improvement with equivalent visual output
**Negative control:** An optimization that drops required warnings or actors fails
**Dependencies:** VG-GOV-001, VG-GPU-004, VG-PERF-001
**Candidate owned paths:** `native/tools/performance/batch-one-expensive-draw-category.py`, `native/tests/roadmap/perf-003_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-PERF-004 — Bound renderer resource lifetime

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Track texture/buffer counts across load, unload and recreation
**Acceptance:** Repeated transitions return to the expected resource envelope
**Negative control:** Stable FPS does not excuse steadily growing GPU memory
**Dependencies:** VG-GOV-001, VG-GPU-004, VG-PERF-001
**Candidate owned paths:** `native/tools/performance/bound-renderer-resource-lifetime.py`, `native/tests/roadmap/perf-004_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-PERF-005 — Bound a loot-heavy label scene

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Limit layout work and prioritize legible actionable labels
**Acceptance:** Dense drops remain selectable within the approved UI/frame budget
**Negative control:** Culling labels must not make an eligible item impossible to pick up
**Dependencies:** VG-GOV-001, VG-ITEM-006, VG-UI-003, VG-GPU-004
**Candidate owned paths:** `native/tools/performance/bound-a-loot-heavy-label-scene.py`, `native/tests/roadmap/perf-005_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-PERF-006 — Warm assets and shaders predictably

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Remove one measured first-use combat hitch with explicit preparation
**Acceptance:** Cold and warm traces show loading policy and remaining hitches
**Negative control:** Hiding cold traces cannot produce a passing report
**Dependencies:** VG-GOV-001, VG-GPU-003, VG-TOOLS-008, VG-PERF-001
**Candidate owned paths:** `native/tools/performance/warm-assets-and-shaders-predictably.py`, `native/tests/roadmap/perf-006_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-PERF-007 — Run a long-session memory soak

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Repeat zone, combat, inventory and reconnect transitions
**Acceptance:** CPU/GPU/session memory reaches a bounded envelope with diagnostic traces
**Negative control:** A short scene cannot establish multi-hour stability
**Dependencies:** VG-GOV-001, VG-GPU-008, VG-NET-005, VG-PERF-004
**Candidate owned paths:** `native/tools/performance/run-a-long-session-memory-soak.py`, `native/tests/roadmap/perf-007_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`, `native/src/core.cpp`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

### [ ] VG-PERF-008 — Accept dense-play frame pacing

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Measure the full mixed encounter in the packaged native build
**Acceptance:** Meets approved frame/tick/input budgets or records a blocking regression
**Negative control:** Headless logs and average FPS alone cannot certify the gate
**Dependencies:** VG-GOV-001, VG-PERF-002, VG-PERF-003, VG-PERF-005, VG-PERF-006, VG-PERF-007, VG-MOVE-008
**Candidate owned paths:** `native/tools/performance/accept-dense-play-frame-pacing.py`, `native/tests/roadmap/perf-008_contract_test.cpp`
**Integration reservation:** `native/client/main.cpp`, `native/src/core.cpp`
**Existing packets:** TASK-0152, TASK-0207
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R06; task content is a new proposal.

## SHIP — Packaging, configuration and platforms

Primary discipline: Build / release engineer. Integration seam to reserve: `native/CMakeLists.txt;native/tools/ci-native.ps1;.github/workflows/native.yml`.

### [ ] VG-SHIP-001 — Choose user-writable data paths

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Resolve saves, config, logs and captures by platform policy
**Acceptance:** Application starts from a read-only install location with isolated test profiles
**Negative control:** Real owner profiles are never the default test destination
**Dependencies:** VG-GOV-001, VG-GOV-007
**Candidate owned paths:** `native/tools/package/choose-user-writable-data-paths.py`, `native/tests/roadmap/ship-001_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/tools/ci-native.ps1`, `.github/workflows/native.yml`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SHIP-002 — Create a standalone development package

**State:** DRAFT | **Gate:** G1 | **Priority:** P0 | **Review:** Tier B | **Size hint:** S (not a time estimate)

**Outcome:** Stage executables, assets, notices and a launch entry point
**Acceptance:** Clean machine launches without source checkout or build tools
**Negative control:** Missing runtime dependencies cannot be fetched invisibly at launch
**Dependencies:** VG-GOV-001, VG-GOV-004, VG-TOOLS-002
**Candidate owned paths:** `native/tools/package/create-a-standalone-development-package.py`, `native/tests/roadmap/ship-002_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/tools/ci-native.ps1`, `.github/workflows/native.yml`
**Existing packets:** TASK-0092, TASK-0160
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SHIP-003 — Package the macOS presentation client

**State:** DRAFT | **Gate:** G2 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Build and stage the actual graphical client with approved dependencies
**Acceptance:** Clean macOS launch shows the reference scene and accepts input
**Negative control:** Console compilation does not count as macOS game support
**Dependencies:** VG-GOV-001, VG-GPU-004, VG-SHIP-002, VG-GOV-005
**Candidate owned paths:** `native/tools/package/package-the-macos-presentation-client.py`, `native/tests/roadmap/ship-003_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/tools/ci-native.ps1`, `.github/workflows/native.yml`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SHIP-004 — Run a clean-install native journey

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Execute the expedition from staged Windows and macOS packages
**Acceptance:** Both packages use correct assets/data paths and finish the ordinary loop
**Negative control:** Developer environment variables cannot hide missing package configuration
**Dependencies:** VG-GOV-001, VG-SHIP-003, VG-TOOLS-008, VG-QA-003
**Candidate owned paths:** `native/tools/package/run-a-clean-install-native-journey.py`, `native/tests/roadmap/ship-004_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/tools/ci-native.ps1`, `.github/workflows/native.yml`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SHIP-005 — Collect bounded crash diagnostics

**State:** DRAFT | **Gate:** G3 | **Priority:** P1 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Capture symbols/version and opted-in diagnostics without sensitive profile data
**Acceptance:** A controlled crash produces a useful, scrubbed report
**Negative control:** Secrets, chat or full saves are not uploaded by default
**Dependencies:** VG-GOV-001, VG-SHIP-001, VG-QA-001
**Candidate owned paths:** `native/tools/package/collect-bounded-crash-diagnostics.py`, `native/tests/roadmap/ship-005_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/tools/ci-native.ps1`, `.github/workflows/native.yml`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SHIP-006 — Test update and rollback policy

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Upgrade a disposable profile and validate supported rollback constraints
**Acceptance:** Compatible update preserves state; incompatible rollback fails visibly
**Negative control:** Old binaries cannot silently overwrite a newer save schema
**Dependencies:** VG-GOV-001, VG-SAVE-007, VG-SHIP-004
**Candidate owned paths:** `native/tools/package/test-update-and-rollback-policy.py`, `native/tests/roadmap/ship-006_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/tools/ci-native.ps1`, `.github/workflows/native.yml`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SHIP-007 — Prepare signing and distribution checks

**State:** DRAFT | **Gate:** G5 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Document and test approved signing/notarization/release steps in a safe channel
**Acceptance:** Credentials stay outside repo and staged artifacts have verified provenance
**Negative control:** A local unsigned build is not labeled a public release
**Dependencies:** VG-GOV-001, VG-SHIP-006, VG-SEC-007, VG-GOV-007
**Candidate owned paths:** `native/tools/package/prepare-signing-and-distribution-checks.py`, `native/tests/roadmap/ship-007_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/tools/ci-native.ps1`, `.github/workflows/native.yml`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

### [ ] VG-SHIP-008 — Publish the supported-device matrix

**State:** DRAFT | **Gate:** G5 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Record tested OS, GPU, input and accessibility configurations
**Acceptance:** Release notes distinguish tested, unsupported and unverified combinations
**Negative control:** One high-end machine cannot certify the entire support range
**Dependencies:** VG-GOV-001, VG-SHIP-004, VG-UI-008, VG-PERF-008
**Candidate owned paths:** `native/tools/package/publish-the-supported-device-matrix.py`, `native/tests/roadmap/ship-008_contract_test.cpp`
**Integration reservation:** `native/CMakeLists.txt`, `native/tools/ci-native.ps1`, `.github/workflows/native.yml`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R12; task content is a new proposal.

## LIVE — Economy services and long-term parity

Primary discipline: Online/economy engineer / producer. Integration seam to reserve: `native/networking;native/persistence;orchestration authority`.

### [ ] VG-LIVE-001 — Separate offline and online progression

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Define import/export and trust policy between local and authenticated profiles
**Acceptance:** Offline progression cannot enter the public economy without an explicit rule
**Negative control:** Local save editing is not confused with secured online authority
**Dependencies:** VG-GOV-001, VG-GOV-007, VG-SEC-006
**Candidate owned paths:** `native/services/online/separate-offline-and-online-progression.hpp`, `native/tests/roadmap/live-001_contract_test.cpp`
**Integration reservation:** `native/networking`, `native/persistence`, `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E02; task content is a new proposal.

### [ ] VG-LIVE-002 — Specify bounded asynchronous trade

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier C | **Size hint:** S (not a time estimate)

**Outcome:** Define one listing lifecycle, escrow policy and cancellation semantics
**Acceptance:** Ownership, currency, expiration and failure outcomes are fully enumerated
**Negative control:** Trading scope cannot precede durable ownership and authentication
**Dependencies:** VG-GOV-001, VG-LIVE-001, VG-SAVE-004, VG-END-007
**Candidate owned paths:** `native/services/online/specify-bounded-asynchronous-trade.hpp`, `native/tests/roadmap/live-002_contract_test.cpp`
**Integration reservation:** `native/networking`, `native/persistence`, `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E02; task content is a new proposal.

### [ ] VG-LIVE-003 — Implement listing escrow transaction

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Move one eligible item into escrow under an authenticated account
**Acceptance:** Retry and cancellation conserve ownership with one active listing
**Negative control:** The same item cannot be equipped and sold simultaneously
**Dependencies:** VG-GOV-001, VG-LIVE-002, VG-ITEM-004
**Candidate owned paths:** `native/services/online/implement-listing-escrow-transaction.hpp`, `native/tests/roadmap/live-003_contract_test.cpp`
**Integration reservation:** `native/networking`, `native/persistence`, `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E02; task content is a new proposal.

### [ ] VG-LIVE-004 — Commit one two-party trade atomically

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Settle purchase, currencies and ownership across two profiles
**Acceptance:** Crash/retry yields a complete committed trade or no trade
**Negative control:** Partial seller payment or duplicated buyer delivery fails
**Dependencies:** VG-GOV-001, VG-LIVE-003, VG-SEC-003, VG-SAVE-004
**Candidate owned paths:** `native/services/online/commit-one-two-party-trade-atomically.hpp`, `native/tests/roadmap/live-004_contract_test.cpp`
**Integration reservation:** `native/networking`, `native/persistence`, `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E02; task content is a new proposal.

### [ ] VG-LIVE-005 — Test anti-abuse economy telemetry

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Add bounded anomaly counters and audited administrative intervention
**Acceptance:** Synthetic duplicate/velocity anomalies are observable without leaking private data
**Negative control:** Telemetry is not used as a substitute for prevention invariants
**Dependencies:** VG-GOV-001, VG-LIVE-004, VG-SHIP-005
**Candidate owned paths:** `native/services/online/test-anti-abuse-economy-telemetry.hpp`, `native/tests/roadmap/live-005_contract_test.cpp`
**Integration reservation:** `native/networking`, `native/persistence`, `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E02; task content is a new proposal.

### [ ] VG-LIVE-006 — Prototype one seasonal reset migration

**State:** DRAFT | **Gate:** G5 | **Priority:** P2 | **Review:** Tier C | **Size hint:** M (not a time estimate)

**Outcome:** Apply an owner-approved reset to copied profiles with retained history policy
**Acceptance:** Seasonal and historical state partition as specified with reversible rehearsal
**Negative control:** No live account reset is authorized by a successful test
**Dependencies:** VG-GOV-001, VG-END-006, VG-SAVE-007, VG-GOV-007
**Candidate owned paths:** `native/services/online/prototype-one-seasonal-reset-migration.hpp`, `native/tests/roadmap/live-006_contract_test.cpp`
**Integration reservation:** `native/networking`, `native/persistence`, `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E02; task content is a new proposal.

### [ ] VG-LIVE-007 — Reassess the parity evidence ledger

**State:** DRAFT | **Gate:** G4 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Compare approved D2R/PoE capability goals with current shipped evidence
**Acceptance:** Remaining breadth, content, services and quality gaps are explicitly unscored
**Negative control:** The 200-task backbone alone never implies complete feature-count parity
**Dependencies:** VG-GOV-001, VG-QA-006, VG-END-008, VG-TOOLS-005
**Candidate owned paths:** `native/services/online/reassess-the-parity-evidence-ledger.hpp`, `native/tests/roadmap/live-007_contract_test.cpp`
**Integration reservation:** `native/networking`, `native/persistence`, `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E02; task content is a new proposal.

### [ ] VG-LIVE-008 — Approve launch scope and operating limits

**State:** DRAFT | **Gate:** G5 | **Priority:** P2 | **Review:** Tier B | **Size hint:** M (not a time estimate)

**Outcome:** Freeze service capacity, support, incident and deferred-feature commitments
**Acceptance:** Named owners accept rollback, economy incidents and public scope claims
**Negative control:** Public release cannot proceed with unowned P0 risks or missing gate evidence
**Dependencies:** VG-GOV-001, VG-LIVE-005, VG-LIVE-006, VG-LIVE-007, VG-SHIP-007, VG-SHIP-008, VG-END-008
**Candidate owned paths:** `native/services/online/approve-launch-scope-and-operating-limits.hpp`, `native/tests/roadmap/live-008_contract_test.cpp`
**Integration reservation:** `native/networking`, `native/persistence`, `orchestration authority`
**Existing packets:** Current-head search and crosswalk required; no absence claim.
**Readiness:** Approve crosswalk and necessary owner rulings; verify dependencies INTEGRATED; refresh interfaces at current head; stamp exact paths, commands and base; obtain exclusive claim via approved authority.
**Evidence:** exact base/head and changed-path list; named acceptance assertions and negative-control result; command, exit code and log hash; integration evidence or explicit NOT_INTEGRATED; artifact provenance and known limitations.
**Stop:** split if the change crosses independent behaviors or outgrows one reviewable PR; do not change frozen contracts or tests to force a pass.
**Basis:** R01, E02; task content is a new proposal.
