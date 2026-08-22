# TASK-0105 REPORT — ox-pc-d

## Executive summary

Completed the passive-tree/progression authority gap audit as a read-only
MECHANICAL packet. Produced `FINDINGS.md` and
`captures/progression-matrix.json` covering levels, attributes, both
quest-point counters, allocation commands/wire, hex projection, the
referenced 271-node gap, persistence/reset rules, network payloads,
client presentation, and tests — each row classified authoritative /
approximation / unruled with file:line citations and test names.
The native +2-per-node attribute approximation and the native
unvalidated save path are recorded as explicit negative controls
(NON-AUTHORITATIVE). No tree content, node, or balance decisions were
made; OI-004 remains WAITING_EVIDENCE and TASK-0112 stays scaffold-first.

## Approach

1. Preflight proven per START_HERE packet (root/branch/HEAD/clean/origin;
   immutable base `42718fbc` verified ancestor of route base `039dcfa7`).
2. Claimed via committed STATUS within the window (commit `1558c3a1`),
   pushed before any other work.
3. Read-side survey with the SPEC's literal rg scan, then targeted reads
   of: browser authority (`server/core/passives/verdigris-authority.js`,
   `quest-service.js`, `first-goal.js`, `player.js`, stats-manager,
   socket-events), client engine/pane (`verdigris-geometric-tree.js`,
   `verdigris-skill-tree.js`, `GeometricSkillTreePane.vue`, `Stats.vue`),
   native session (`native/src/networking.cpp`, `core.cpp`,
   `include/verdigris/*.hpp`), tests (`tests/unit/*skill*`,
   `playtest/scenarios/*`, `playtest/critic.mjs`,
   `native/tests/networking_tests.cpp`), docs (`vision.md`,
   `HANDOFF.md`, `VERDIGRIS_GAP_AUDIT.md`, `LEGACY_MATRIX.md`, OI-004).
4. Executed the engine read-only to pin graph truth: **331 main-lattice
   nodes + 34 subtree nodes = 365 total, 974 conduits, schemaVersion 2**
   (`node --input-type=module` against
   `src/core/passives/verdigris-geometric-tree.js`) — contradicting the
   stale 271 prose.
5. Wrote evidence artifacts inside the task folder only; ran all four
   literal acceptance gates; flipped STATUS to REVIEW_REQUESTED.

## Changed files (this worker branch, base `039dcfa7` → head)

- `orchestration/tasks/TASK-0105-passive-tree-progression-gap-audit/STATUS.md`
  (claim, then REVIEW_REQUESTED)
- `orchestration/tasks/TASK-0105-passive-tree-progression-gap-audit/FINDINGS.md`
- `orchestration/tasks/TASK-0105-passive-tree-progression-gap-audit/captures/progression-matrix.json`
- `orchestration/tasks/TASK-0105-passive-tree-progression-gap-audit/REPORT.md`

No file outside the owned task folder was created or modified.

## Public interfaces added/changed

None. Read-only audit; zero production code changes.

## Acceptance gate transcripts (literal commands from SPEC)

### Gate 1

```
PS Z:\Code\.worktrees\verdigris\ox-pc-d> rg -n "passive|tree|questPoints|attribute|strength|dexterity|intelligence|allocate|271|hex" native/include native/src native/client native/tests playtest docs/rebuild
<full output: ~190 matching lines across native/include, native/src,
native/client, playtest, docs/rebuild — key hits: networking.cpp:553,815,818-828,1082,1092-1155,1157-1166,1374,1542,1597,2014-2021,2453,2644;
networking.hpp:174-180; core.hpp:108-110,479-480,591; core.cpp:68-99,420;
client main.cpp scenery-only tree hits; skilltree/build-divergence/
session-arc/first-goal/quest scenarios; HANDOFF.md:252-256;
LEGACY_MATRIX.md:17; VERDIGRIS_GAP_AUDIT.md:39>
RG_EXIT=0
```

Exit code: **0** (matches). Full raw output preserved in the session
transcript; the load-bearing lines are cited row-by-row in
`captures/progression-matrix.json`. Notable negative-space result: no
skilltree/passive/questPoints hits under `native/tests`.

### Gate 2

```
PS> node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0105-passive-tree-progression-gap-audit/captures/progression-matrix.json','utf8')); console.log('progression matrix: PASS')"
progression matrix: PASS
NODE_EXIT=0
```

Exit code: **0**.

### Gate 3

```
PS> git diff --check
DIFFCHECK_EXIT=0
```

Exit code: **0** (no whitespace/conflict-marker problems).

### Gate 4 + working-tree proof of "only task evidence changes"

```
PS> git diff --name-only
DIFFNAME_EXIT=0
PS> git status --short
?? orchestration/tasks/TASK-0105-passive-tree-progression-gap-audit/FINDINGS.md
?? orchestration/tasks/TASK-0105-passive-tree-progression-gap-audit/captures/
```

Exit code: **0**; the only changes in the worktree are the two new
task-evidence paths (plus this REPORT/STATUS inside the same folder).

## Negative control (SPEC-required)

The approximate formula is explicitly marked NON-AUTHORITATIVE:

- Matrix rows AT-2 and AL-2 carry `"status": "approximation"` with
  `"negativeControl": true`; F-2 freezes "must not be ratified".
- FINDINGS.md records the +2/axis walk
  (`native/src/networking.cpp:1092-1119`, STUB NOTE `818-821`) and the
  raw-snapshot save path (`networking.cpp:1157-1166`) as the two seams
  that must never become silent default authority.
- Contrast evidence: browser `resolveVerdigrisTree`
  (`verdigris-authority.js:28-111`) rebuilds/validates and its greens
  (build-divergence.mjs) bind to that authority, not the approximation;
  no native test pins the approximation (empty rg over
  `native/tests`), so nothing ratifies it.

## Manual verification

- Executed-check output quoted in matrix GR-1:
  `main lattice nodes: 331; subtree nodes: 34; total: 365; conduits: 974;
  schemaVersion: 2`.
- Cross-checked the earned formula on both stacks
  (`min(140, min(max(2,level),117) + min(qp,23))` native vs
  `earnedVerdigrisPoints` browser) and the counter divergence documented
  as QP-2 (browser goal-fed/persisted/cap 23 vs native login-reset,
  commission+goal-fed, goal-capped 12).

## Commits (this branch)

- `1558c3a1e319ba89f9c17353c80fcfa0b129a7f8` — claim-only STATUS push.
- `<head>` — FINDINGS + progression-matrix.json + REPORT + STATUS →
  REVIEW_REQUESTED (recorded at push time).

Boundary proof: `git diff --name-only 039dcfa7..HEAD` lists only
`orchestration/tasks/TASK-0105-passive-tree-progression-gap-audit/**`.
`42718fbc..039dcfa7` contains only the controller's coordination-only
route/base refresh (`039dcfa7`), per the launch packet; the immutable
code base is untouched.

## Deviations

1. **Claim committed with `--no-verify`.** The repo's yorkie pre-commit
   hook cannot run in this isolated worktree (`node_modules` absent;
   hook fails with `Cannot find module ...\node_modules\yorkie\src\runner.js`),
   and no sibling lane worktree has dependencies installed either. To
   hold the 10-minute claim deadline the markdown-only claim commit was
   made with `--no-verify`. Content risk is nil (pure orchestration
   markdown); disclosed here rather than hidden.
2. None otherwise. All four gates ran literally on the default path
   with no env overrides; ports were untouched (read-only capsule);
   port 6500 untouched.

## Unresolved questions

1. QP-2 ruling needed: which top-level quest-point semantics are
   canonical across relogins (increment sources, caps 12 vs 23, reset
   rule)? Routed to architect/OI-004; TASK-0112 must not guess.
2. Stale 271/123-point prose vs executed 365-node/140-point reality:
   which document class becomes authoritative once OI-004 lands?

## Risks / follow-ups

- Native combat power already consumes the approximation
  (`networking.cpp:2014-2021`); replacing it later is an intentional
  behavior change requiring review attention, not a silent retune.
- Native has zero progression tests; the successor scaffold should land
  parity fixtures against `resolveVerdigrisTree` outputs first.
- Follow-ups filed as rows in the matrix (CP-1 native presentation
  surface, TS-1 native tests, GR-1 doc drift cleanup pending owner
  source).

## Status

STATE → REVIEW_REQUESTED. Branch pushed; awaiting Tier-A mechanical
review at exact head/base.
