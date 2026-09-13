# REPORT — TASK-0121 Owner content approval matrix

- **Lane:** ox-pc-bb (`openrouter/stealth/ox-alpha`)
- **Claim commit:** `32049508` (STATUS CLAIMED at base `9bd689b4cebac0fe1f79ba54edcc9967a1a8f0d4`)
- **Branch:** `worker/verdigris/pc/ox-pc-bb` → origin only
- **Date:** 2026-08-23

## Executive summary

Batched every current or terminal owner-only content decision into a single
approval matrix: 15 gates across art/assets, lore, naming, magic, balance,
economy, campaign content, bosses/monsters/items/skills, music, season rules,
distribution, and irreversible accounts (plus the provisional content-scale
envelope). Each gate carries evidence prerequisite, critical-path deadline,
recommended evidence-gathering step, >=2 viable decision classes, acceptance
rubric, dependents, and fallback work. Negative control satisfied: G-04 magic
direction is parked noncritical with executable fallback (also G-07/G-09/G-10).
No canon was chosen or recommended; every gate remains
`UNRESOLVED_OWNER_ONLY`.

## Approach

1. AGENTS.md preflight (clean tree, base ancestor check, origin sync) — PASS.
2. Read product authority and orchestration sources: constitution,
   OPEN_DECISIONS, DECISIONS.md, owner-input queue OI-001..OI-009,
   PROGRAM_GRAPH.md, CONTENT_SCALE_MATRIX.md, BACKLOG_FACTORY.md,
   BOOTSTRAP.md owner-only list.
3. Ran the SPEC's evidence scan (`rg`) to bound the gate universe.
4. Mapped each domain to gates, reusing documented packet recommendations only
   as clearly-marked non-rulings; invented nothing.
5. Wrote machine artifact first (`captures/owner-gates.json`, node-validated),
   then human companion (`FINDINGS.md`), then ran all acceptance commands.

## Changed files

```
orchestration/tasks/TASK-0121-owner-content-approval-matrix/FINDINGS.md        (new)
orchestration/tasks/TASK-0121-owner-content-approval-matrix/captures/owner-gates.json (new)
orchestration/tasks/TASK-0121-owner-content-approval-matrix/STATUS.md          (claim -> REVIEW_REQUESTED)
orchestration/tasks/TASK-0121-owner-content-approval-matrix/REPORT.md          (new, this file)
```

No file outside the owned folder was touched. Public interfaces added/changed:
none (documentation-only packet).

## Acceptance commands — literal transcripts and exit codes

### 1) Evidence scan

```
$ rg -n "owner-only|OWNER|OD-[0-9]|asset|lore|naming|balance|economy|music|season|distribution" docs/product orchestration/DECISIONS.md orchestration/owner-input orchestration/PROGRAM_GRAPH.md
<full output captured above in session; 90 matching lines across>
docs/product/OPEN_DECISIONS.md (OD-001..OD-013 rows),
docs/product/VERDIGRIS_CONSTITUTION.md (66,78,155,172),
docs/product/VERDIGRIS_FEATURE_CHECKLIST.md (45,63,64),
orchestration/DECISIONS.md (14,42,47,52,61,69,82,113,145,148,153,270),
orchestration/PROGRAM_GRAPH.md (15,16,17,22,81,102,120,124,133,158,175,178,188,193,195,205,207,210,211,212),
orchestration/owner-input/OI-001..OI-009 + README.md
rg exit: 0
```

Exit code: **0**. (Full verbatim output retained in the lane session log;
representative lines shown. The scan bounded G-01..G-15.)

### 2) JSON gate artifact validation

```
$ node -e "const x=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0121-owner-content-approval-matrix/captures/owner-gates.json','utf8')); if(!Array.isArray(x.gates)||!x.gates.length) process.exit(1); console.log('owner gates: PASS')"
owner gates: PASS
node exit: 0
```

Exit code: **0**. Artifact contains 15 gates, all `UNRESOLVED_OWNER_ONLY`.

### 3) Whitespace check

```
$ git diff --check
git diff --check exit: 0
```

Exit code: **0** (no whitespace errors).

### 4) Changed-path proof

```
$ git diff --name-only
git diff --name-only exit: 0
(no tracked-file modifications; deliverables are new untracked files)

$ git status --short
?? orchestration/tasks/TASK-0121-owner-content-approval-matrix/FINDINGS.md
?? orchestration/tasks/TASK-0121-owner-content-approval-matrix/captures/
status exit: 0
```

Expected met: **only this task folder changes**.

## Manual verification

- Domain coverage table in the JSON maps all twelve SPEC domains to >=1 gate.
- Every gate: evidence prerequisite, deadline, evidence-gathering step,
  >=2 viable decision classes, rubric, dependents, fallback present.
- Negative control present: `negative_control: true` on G-04 with executable
  fallback list; corroborated by G-07/G-09/G-10 parked statuses matching the
  owner-input README states.
- No-choice-invariant audit: no field marks any decision resolved/approved;
  packet recommendations are quoted with explicit "not an owner ruling"
  markers.

## Deviations

- None of substance. Note: PROTOCOL.md says "NEVER push", but the START_HERE
  claim protocol for this lane explicitly overrides it ("commit + push it to
  origin worker/verdigris/pc/ox-pc-bb"); pushes went to the worker branch
  only, never to `codex/native-reconstitution`.
- `git diff --name-only` shows nothing for untracked new files; scope proven
  via `git status --short` (recorded above).

## Unresolved questions / risks

- G-13 (distribution/monetization) and G-14 (irreversible/account actions)
  currently lack dedicated OI packets; they are inventoried here. Creating
  those packets is architect work (this lane must not write outside its owned
  paths).
- Gate deadlines are domain-specific per existing docs; none block current
  critical-path tasks today.

## Follow-ups (for the architect)

1. When TASK-0093/0094, TASK-0105, TASK-0117, TASK-0085, or TASK-0096 evidence
   lands, promote the corresponding OI packets to READY_FOR_OWNER using this
   matrix's decision-class lists as the batched alternatives.
2. Consider spawning OI packets for distribution/monetization and the
   irreversible-actions handoff ledger (G-13/G-14).
3. An early G-15 scale-tier ruling avoids large re-decomposition of D-128
   backlog floors.

## Commits in this delivery

| Commit | Subject |
|---|---|
| `32049508` | claim(TASK-0121): ox-pc-bb claims owner content approval matrix at 9bd689b4 |
| *(this commit)* | docs(TASK-0121): owner content approval matrix findings + gate captures |
| *(next commit)* | status(TASK-0121): REVIEW_REQUESTED with frozen head |

Frozen reviewed head recorded in STATUS.md at flip time.
