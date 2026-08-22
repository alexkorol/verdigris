# TASK-0116 REPORT — Native animation and VFX contract audit

- **Worker:** ox-pc-s (OpenCode Ox Alpha) · provider `openrouter` · model `stealth/ox-alpha`
- **Branch:** `codex/TASK-0116-animation-vfx-contract-audit-ox-pc-s` (worktree `Z:\Code\.worktrees\verdigris\ox-pc-s`)
- **Base:** `9fe673b66ffc082e865e0f0fb66f454ec1984949` (routed; spec base `9bd689b4…` noted in STATUS)
- **State at handoff:** REVIEW_REQUESTED

## Executive summary

The native animation/VFX contract is audited end-to-end in `FINDINGS.md` with a
machine-readable `captures/animation-vfx-matrix.json` (18 rows: 7 COMPLETE,
9 PARTIAL, 4 MISSING, 1 OWNER-ASSET; 5 negative controls; 4 successor phases).
Headline: the **contract skeleton is sound and locked** (simulation telegraph
timing, D-119 render list, byte-deterministic captures, scenario/test layers),
but there is **no actual animation yet** — no walk cycle, no actor
interpolation, no particle layer, no crit distinction, no camera-shake/l-lerp
coverage. Every gap is mapped to a concrete Phase A–D boundary so TASK-0122 can
be promoted immediately (Phases A–C need no owner input under D-113).

## Approach

1. Preflight per AGENTS.md; verified no existing claim (`STATUS.md` absent,
   no remote branch, no `RELEASE.md`) before writing the claim.
2. Read constitution + D-113/D-114/D-115/D-118 decisions as frozen authority.
3. Deep-read the animation/VFX surface: `core.hpp/core.cpp` (timing/events),
   `networking.cpp` (wire), `presentation_events/presentation_state/
   local_session/remote_session` (seam), `main.cpp` (paint/FX/camera/scenarios),
   `render_list.hpp`, `camera2d.hpp`, all four test suites, checked-in capture
   folders, side-by-side benchmark, TASK-0141/0142/0147 context.
4. Ran every literal acceptance command; classified each contract area;
   documented negative controls; derived phased successor boundaries.

## Changed files

Only inside `orchestration/tasks/TASK-0116-animation-vfx-contract-audit/`:

- `STATUS.md` (claim → REVIEW_REQUESTED)
- `FINDINGS.md`
- `captures/animation-vfx-matrix.json`

No game source, SPEC, REVIEW, other task folders, other worktrees, or port
6500 were touched. `native/`, `src/`, `server/` untouched (read-only capsule).

## Public interfaces added/changed

None (audit-only). The matrix proposes future interface changes for TASK-0122
(Phase A event mappings, telegraph geometry fields, windup-unit unification)
but changes nothing today.

## Test commands + outcomes

| Command (literal from SPEC) | Outcome |
|---|---|
| `rg -n "animation\|frame\|facing\|swing\|telegraph\|impact\|death\|dash\|effect\|particle\|aura\|orb\|camera" native/client native/include native/src native/tests orchestration/benchmarks` | PASS — ran verbatim twice (pre/post deliverables); 719 matching lines; clusters: main.cpp 304, core_tests.cpp 118, core.cpp 71, presentation_state.cpp 49, remote_session.cpp 38. Full output retained in session tool logs. |
| `node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0116-animation-vfx-contract-audit/captures/animation-vfx-matrix.json','utf8')); console.log('animation/VFX matrix: PASS')"` | PASS — printed `animation/VFX matrix: PASS`. |
| `git diff --check` | PASS — exit 0, no whitespace/conflict markers. |
| `git diff --name-only` | PASS — empty working-tree diff outside staged task-folder files; `git status --short` showed only the two audit files before commit. |

Negative control satisfied: five documented (matrix `negative_controls`);
primary NC-01 = critical hits (`combat:hit.critical` computed at
`core.cpp:1953-1971`, shipped at `networking.cpp:1998-2005`, ignored by client
`remote_session.cpp:744-808`, no visual, no capture).

## Manual verification

Not applicable to simulation behavior (no code changed). Evidence quality was
verified by re-opening every cited file/line during matrix authoring; checked-in
capture inventories were listed directly (TASK-0070: 15 files present;
triage-captures: 4 files; side-by-side benchmark tree enumerated).

## Commit SHAs

- `297b4e9d` — claim (`STATUS.md` CLAIMED)
- `6d6fcf4b` — FINDINGS.md + animation-vfx-matrix.json
- `<final>` — REPORT.md + STATUS REVIEW_REQUESTED (this commit)

## Deviations

- None from SPEC scope. Note: pre-commit hook (`lint-staged`) initially failed
  because this fresh worktree had no `node_modules`; resolved by running
  `npm ci` rather than skipping hooks.
- Spec base commit differs from routed base (`9bd689b4…` vs `9fe673b6…`);
  both recorded in STATUS/matrix. All citations are against the routed base.

## Unresolved questions

None requiring an architect ruling now. Flagged for TASK-0122 design:
hit-stop authority (sim freeze ticks vs presentation-only), renderer-seam
extract-now vs defer, windup unit unification choice (ticks vs ms).

## Risks

- Audit reflects base `9fe673b6`; if the program branch moves before review,
  line numbers may drift slightly (structure unlikely to change — no other
  task owns these files).
- TASK-0147 polish remains quarantined/unintegrated; its RELEASE.md warnings
  are restated in FINDINGS F-PROV so the successor doesn't resume dirty work.

## Follow-ups (successor promotion package)

TASK-0122 can be promoted immediately with:
1. Phase A (contract mappings, zero feel risk) as milestone 1 — closes NC-01…NC-05.
2. Phase B (named timing constants + multi-tick capture goldens + missing assertions).
3. Phase C (procedural juice under D-113/D-114/D-115).
4. Phase D terminal gate (owner asset decisions).
Also route the `core.cpp:1942` debug `fprintf("[swing]…")` removal into the
successor's first owned-source commit (this capsule could not touch it).
