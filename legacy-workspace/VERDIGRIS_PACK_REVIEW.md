# Verdigris Parallel Execution Pack — review, reconciliation, and execution status

**Written:** 2026-09-05 ~23:00 PDT by Kimi Work K3 (`coordinator: kimi-work`)
**Pack:** `VERDIGRIS_PARALLEL_EXECUTION_PACK/verdigris_execution_pack` (Edition 1.0, 2026-09-04)
**Scope of this document:** (A) critical review of the pack, (B) reconciliation of its 200-goal
registry against the live repository, (C) the owner decisions that still gate execution, and
(D) execution started under the owner's "start implementing" directive.

---

## A. Review assessment of the pack

**Overall: adopt it as the planning backbone — it is unusually honest and structurally sound —
but treat its repo-state assumptions as already superseded.**

Strengths:

1. **Honest scope framing.** Every count is labeled proposal, not parity claim; the evidence
   ladder (Defined → Unit-proven → Integrated → Perceptually accepted → Packaged) matches the
   hard-won lesson recorded in the repo's own D-114/D-115 era ("green correctness gates shipped
   an unplayable game"). The anti-theater rules (§15) are exactly the failure modes this
   repository has already burned once.
2. **The gate ladder (G0–G5) is the right shape** and converges correctly on one integrated
   player journey instead of task counts — consistent with OWNER_DEMO_RUNWAY's "no task count
   substitutes for the integrated journey and npm run playtest".
3. **Dependency tooling works.** `roadmap.py validate` + 20/20 unit tests pass locally; the DAG
   (200 goals / 689 edges) is acyclic with no dangling IDs. The read-only-by-design stance
   (tools cannot mint claims) is correct discipline.
4. **The claim-mechanism critique (§10.3) correctly identified a real defect** — first-push-wins
   branch claims are not exclusive locks — which the repo has since partially addressed (see B4).

Weaknesses / risks:

1. **Stale baselines.** The pack's audit baselines (`2d3e92a5` origin/master, `8597c654`
   origin/codex/native-reconstitution) are superseded. The live program head is
   `origin/codex/goal-aaa-systems` @ `e7b65360` (2026-09-04), 59 commits ahead of
   native-reconstitution, containing the architect checkout's `486058f3` plus a large
   "AAA systems" lane (account persistence, remote skills/HUD, guided House/Scion creation,
   charted expeditions — several of which overlap pack goals in SAVE/HOUSE/END/NET).
   **Any READY stamping must be re-done against `e7b65360` or later, not the pack's refs.**
2. **The 8-lane first wave table (§13.2) is partly obsolete:** VG-MOVE-001's target files are
   under another agent's active lease; VG-SOUND-001's deliverable substantially exists
   (waveOut sink shipped `fdbcb5a9`, TASK-0157 seam integrated); VG-GPU-001 is answered in
   direction by D-118 (2D top-down now) and TASK-0114 (INTEGRATED). Per the pack's own rule,
   these need "verify/extend" reframing, not fresh implementation.
3. **Candidate owned paths are fictitious layouts** (`native/systems/items/*.hpp`,
   `native/tests/roadmap/*_contract_test.cpp` do not exist). The pack admits this ("candidate
   new module locations … proposed—not existing paths"), but every packet promotion must pay
   the path-stamping cost against the real tree.
4. **200 goals ≠ dispatchable work.** Only a handful of VG goals are near-eligible; GOV-001–008
   bootstrap and owner rulings (DRAFT-D01..D09) gate almost everything. This is by design, but
   anyone reading "200 goals" as a work backlog will over-dispatch.
5. **Content-lot expansion** can inflate the registry by an order of magnitude; the pack says
   so, but the estimator warning ("do not infer throughput from agent count") deserves more
   prominence given this fleet's history.

Verdict: adopt as planning authority subordinate to the constitution and DECISIONS.md;
reconcile per §B before any further dispatch.

## B. Reconciliation against the live repository

### B1. True program head

| Ref | SHA | Status |
|---|---|---|
| origin/master | `2d3e92a5` | pack baseline; stale |
| origin/codex/native-reconstitution | `8597c654` | pack integration ref; stale |
| architect checkout (delaford_game) | `486058f3` | 20 unpushed commits ahead of native-reconstitution; live Cursor HUD work uncommitted on top |
| **origin/codex/goal-aaa-systems** | **`e7b65360`** | **current program head (2026-09-04)** — contains 486058f3 + 39-commit AAA lane |

Kimi Work's clone: `Z:\Code\Games\delaford\kimiwork_verdigris`, branch `kimiwork/program`
tracking the aaa tip. Per-repo protocol, the architect checkout is never touched by me
(read-only inspection only).

### B2. Task board (STATUS.md is truth; SPEC frontmatter stale)

- 16 tasks INTEGRATED in the 0160–0204 range; 17 at REVIEW_REQUESTED (review queue is the
  program bottleneck per RUN_STATUS); REVISE verdicts open on TASK-0166, 0172, 0182.
- **TASK-0108 rev 3 was the one READY, claimable, P0 packet** — now CLAIMED by kimi-work
  (see §D).
- TASK-0095 / TASK-0097 (which the pack crosswalk feeds into VG-TOOLS-001 / VG-SAVE-001) are
  **SUPERSEDED** — their FINDINGS exist and should be absorbed, not re-audited.
- Owner Demo chain 0205–0207 is unclaimed AUTO_RELEASE; 0208 DRAFT. The lane map forbids both
  Cursor and me from duplicating that chain.

### B3. Pack crosswalk §17 vs reality

- TASK-0108 → now in flight (kimi-work), one implementation as the pack demands.
- TASK-0097/0114/0162 → audits INTEGRATED or SUPERSEDED; VG-SAVE-*/VG-GPU-*/VG-BUILD-*
  successors must read their FINDINGS first.
- TASK-0193–0196 (trees/lattice), 0197–0204 (House/forge/recovery/audio) → mostly
  REVIEW_REQUESTED or BRIDGE_PREP under Cursor's lease; the pack's BUILD/HOUSE/FORGE goals
  must extend those packets, not fork them (lane-map hard rule 1).

### B4. The push/claim policy conflict (pack DRAFT-D01/D02 = repo GOV-002)

Partially resolved in-repo since the pack was written: commit `e3ad60da` (2026-09-03) rewrote
PROTOCOL step 5 to "do not push by default; push when the owner explicitly asks"; the standing
workspace rule remains "commit locally, the owner pushes." Cursor's `CURSOR_KIMI_LANES.md`
establishes **path+resource leases** as the on-machine exclusivity mechanism. Remaining gap:
leases are only visible to agents on this machine; origin-visible claims still wait for owner
pushes. **Still needs the owner's stamp (GOV-002).**

### B5. Baseline gates at e7b65360 (run today, real)

- `npm run playtest` — **PASS, 32/32 scenarios, exit 0** (~3 min, port-isolated).
- `native/build.ps1 -RunTests` — **PASS, exit 0** (379 PASS / 0 FAIL lines; denylist, core,
  networking, camera2d, session, presentation-events, audio all green).
- Spot-ran `verdigris_core_tests.exe`, `verdigris_networking_tests.exe`,
  `verdigris_session_tests.exe` — all exit 0.
- Environment note: from Git Bash, `build.ps1` needs `ProgramFiles(x86)` injected; `npm` is
  `npm.cmd` (bundled Node v24.15.0 / npm 11.12.1).

## C. Owner decisions still gating execution

| Decision | Needed for | Current state |
|---|---|---|
| GOV-002 / DRAFT-D01/D02 — claim & push precedence | any multi-agent dispatch beyond this machine | protocol amended; lease file in use; **owner stamp outstanding** |
| DRAFT-D03 visual representation | VG-ART-*, VG-GPU-* | D-118 (2D now) + vector-art era + D2R benchmarks on aaa branch; owner confirmation of target sheet pending |
| DRAFT-D05 mortality/disconnect table | VG-SAVE/HOUSE/LIVE tuning | partially ruled (D-106, D-109); expedition-state table not recorded |
| DRAFT-D06 funded slice scope | G3 content lots | open |
| DRAFT-D07 Brands/Bonds/Arcane semantics | VG-FORGE/BUILD | open (TASK-0193–0199 in review may answer parts) |
| DRAFT-D08/D09 online economy + release support | G4/G5 | open, not yet blocking |

## D. Execution started (owner directive "start implementing")

Wave 1, all inside my own clone, one worktree per lane, nothing pushed:

1. **Baseline verification** — done, green (§B5).
2. **TASK-0108 rev 3 (ranged combat, P0)** — CLAIMED, core+wire slice implemented on
   `kimiwork/TASK-0108-ranged-rev3` (commits `bebb1aba`, `72b25d85`, `3b929637`):
   - ranged volley windup emits a distinct `projectile` combat event with origin/target tiles;
   - networking routes it to the `world:projectile` envelope with the exact JS payload keys
     (`{fromX, fromY, toX, toY, travelMs, kind}`), never `monster:telegraph` (D-129);
   - locking tests: ranged damage beyond contact, melee twin cannot, every hit preceded by a
     warning, byte-identical replay, envelope + negative control;
   - gates green: full `-RunTests` + `-RunClientScenarios` (38 scenario blocks, 0 failures),
     diff-check clean, `state.xp` block untouched.
   - **Deferred (Cursor lease):** the client-visible Telegraph render op in
     `presentation_state.cpp`/`render_list.hpp`/`main.cpp` + its `presentation_events_tests`
     lock. STATUS stays CLAIMED until that lands; interim window (wire event without
     client-visible warning) recorded in REPORT.md.
   - One honest flake noted: gate-b "successor fell to ordinary combat" failed once on a full
     run, green on standalone rerun and the full second run — documented adds-during-pickup
     flake mode, unrelated to this diff.
3. **Collaboration protocol live:** acknowledged Cursor's `orchestration/CURSOR_KIMI_LANES.md`
   and appended the dated "Kimi Work claim" section (my lanes: `native/src/**`,
   `native/include/**`, `native/tests/**` minus frozen/leased regions, `native/tools/**`;
   Cursor keeps `native/client/**`, `docs/execution/**`, the `state.xp` block).

### Next frontier (in order, as blockers clear)

1. TASK-0108 client presentation stage — after Cursor's `native/client/**` lease releases.
2. VG-TOOLS-001 content ID/schema validator — after Cursor's GOV-001/004 artifacts land in
   `docs/execution/`; absorbs SUPERSEDED TASK-0095 findings.
3. VG-SAVE-001 durable-profile inventory (Tier C) — absorbs SUPERSEDED TASK-0097 findings;
   needs GOV-001 and the architect/owner Tier-C eye.
4. REVISE queue help (0166/0172/0182) is Cursor/architect-owned; not mine unless reassigned.
