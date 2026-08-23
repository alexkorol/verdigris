# TASK-0096 — Campaign and zone-graph measurement audit (REPORT)

- Lane: `ox-pc-bb` · Model: `openrouter/stealth/ox-alpha`
- Branch/worktree: `worker/verdigris/pc/ox-pc-bb` @ `Z:/Code/.worktrees/verdigris/ox-pc-bb`
- Task base: `d2423873c577d299b3b39c56024d1d840993c72b` (verified ancestor of audited head)
- Claim commit: `f852254d` ("TASK-0096: claim (ox-pc-bb)", STATUS CLAIMED, pushed to origin)

## Executive summary

Mechanical, read-only audit of the current campaign/zone-graph topology
delivered as [`FINDINGS.md`](FINDINGS.md) plus machine-readable
[`captures/graph.json`](captures/graph.json). Headlines:

- **Two graphs exist.** Graph A is the headless core route table seeded in the
  constructor (`native/src/core.cpp:195-200`): 2 mandatory nodes
  (`route:tin:1:0` → `route:tin:2:0`) plus 1 optional branch (`branch:ash`,
  specialization grant at `core.cpp:461-467`). Graph B is the protocol world
  web (`native/src/networking.cpp:716-824`): 4 roads × lazily generated tiers
  of width 1–3, node ids `<road>:<tier>:<index>`, per-house deterministic via
  FNV-1a hashing; progress is session-scoped only
  (`networking.hpp:197-205`).
- **Measured traversals.** Core shortest path to campaign completion = 1 node:
  `campaign_complete` fires on the FIRST clear of any route
  (`core.cpp:802-806`; tests `core_tests.cpp:1095-1140`). Longest defined core
  traversal = 3 stops (root clear + branch interact + deep clear), then the
  graph is exhausted. Web traversal costs one expedition leg per stage tier
  (`networking.cpp:2388-2389` vs `1517-1546`) with no growth bound and no
  terminal/completion state.
- **Negative control honored.** `campaign.act_count` and
  `campaign.target_duration_hours` remain MISSING in
  `missing_authoring[]`: road names were not promoted into acts and the
  constitutional 6–30h sentence was not converted into a measurement.
- **Delta mapped without invention.** Campaign / optional branches /
  repeatable endgame / fast-travel seams are mapped to the constitution
  (FINDINGS §8) with owner-blocked parts separated from mechanical successors
  (FINDINGS §7, §9): persist web progress per House, define a protocol-path
  completion analog, reconcile dual books, centralize unlock authorization at
  `world:zone:enter`.

No code or product files were modified. Nothing was invented. Resource capsule
respected: read-only survey, no ports opened (never 6500), no servers started.

## Approach

1. Product authority first: constitution campaign/endgame section,
   OD-012 open decision, crossroads design doc (historical reference).
2. Full-text sweep of `native/src`, `native/include`, `native/tests`,
   `playtest/scenarios/world-web.mjs`, `playtest/scenarios/quest.mjs`
   (SPEC acceptance pattern); line-level reads of every hit bearing on
   routes/nodes/gates/wardens/stairs/extraction/campaign.
3. Deterministic re-execution of the published web algorithm in
   `tools/build-graph.mjs` to produce the worked example and traversal counts;
   all findings written with path:line citations.

## Changed files

```
orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/FINDINGS.md            new
orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/captures/graph.json    new
orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/captures/acceptance-rg-transcript.txt new
orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/tools/build-graph.mjs  new
orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/REPORT.md              new (this file)
orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/STATUS.md              CLAIMED -> REVIEW_REQUESTED
```

Nothing outside `orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/**`
was touched (owned_paths honored; forbidden_paths untouched).

## Public interfaces added/changed

None. Documentation/evidence only; no API, wire, schema, or behavior change.

## Acceptance commands — literal transcripts + exit codes

The four SPEC acceptance commands were executed literally from the repo root,
in order. Preparation disclosure: the evidence files are newly created, so
`git add -N orchestration/tasks/TASK-0096-campaign-graph-measurement-audit`
(intent-to-add) was applied before the diff commands so that `git diff` can
report them; intent-to-add stages no content and was not part of the
acceptance set.

### 1) rg sweep — exit code 0

Command:

```text
rg -n "road|route|node|branch|warden|waymark|stairs|extract|campaign" native/src native/include native/tests playtest/scenarios/world-web.mjs playtest/scenarios/quest.mjs
```

Outcome: exit code 0, 677 matching lines across the five swept trees (72,926
bytes of stdout). The complete unedited transcript is preserved verbatim at
[`captures/acceptance-rg-transcript.txt`](captures/acceptance-rg-transcript.txt)
(SHA256 `b88ad13d3a8150cdaa7e2daae1f1af050ac88f47743be143b1c3d53296ab89d3`;
677 lines, byte-exact `rg` stdout from the recorded execution). Repeatability
note: a second literal execution returned exit 0 with the identical 677-line
match set (verified zero-diff after sorting); `rg`'s line ordering across
multiple root paths is itself nondeterministic between runs, so the capture is
pinned to its execution by SHA256 rather than by line order. Head of
transcript:

```text
playtest/scenarios/quest.mjs:223:    assert(state.sceneName === 'Weir Crypt', 'the named campaign delve enters Weir Crypt');
playtest/scenarios/quest.mjs:251:    }, { timeoutMs: 30000, intervalMs: 350, label: 'Pale Sovereign campaign boss' });
playtest/scenarios/quest.mjs:253:      'only the named crypt sovereign breaks the campaign seal');
playtest/scenarios/quest.mjs:255:    const stairsDown = state.sceneMetadata.stairsDown;
playtest/scenarios/quest.mjs:256:    assert(stairsDown, 'the broken seal exposes stairs to the deeper realm');
playtest/scenarios/quest.mjs:257:    p.devTeleport(stairsDown.x, stairsDown.y);
```

Tail of transcript:

```text
native/tests\core_tests.cpp:2056:  const Vec2 up = world.metadata().stairs_up;
native/tests\core_tests.cpp:2058:  check(world.in_instance() && world.metadata().depth == 4, "N4 stairs climb one floor");
native/tests\core_tests.cpp:2060:  const Vec2 again = world.metadata().stairs_up;
native/tests\core_tests.cpp:2063:  const Vec2 surface = world.metadata().stairs_up;
native/tests\core_tests.cpp:2104:  test_extraction();
native/tests\core_tests.cpp:2108:  test_item_identity_and_branch();
native/tests\core_tests.cpp:2109:  test_campaign_and_seasonal_extension();
native/tests\core_tests.cpp:2111:  test_legends_cover_unlocks_and_campaign_milestone();
```

### 2) JSON parse gate — exit code 0

Command:

```text
node -e "const g=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/captures/graph.json','utf8')); if(!g.nodes||!g.edges) process.exit(1); console.log('campaign graph: PASS')"
```

Output:

```text
campaign graph: PASS
```

### 3) whitespace gate — exit code 0

Command: `git diff --check`

Output: (empty — no whitespace/conflict-marker errors)

### 4) change-set gate — exit code 0

Command: `git diff --name-only`

Output:

```text
orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/FINDINGS.md
orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/captures/graph.json
orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/tools/build-graph.mjs
```

Expected result met: only task evidence changes. (REPORT.md, the acceptance
transcript capture, and the STATUS flip are part of the closing
REVIEW_REQUESTED commit itself, so they cannot appear in a pre-commit working
tree diff.)

## Manual verification

Not applicable to runtime behavior (no code changed). Performed instead:

- Every citation spot-read at the audited head (constructor seeds, gate
  predicates, chart frontier/status logic, warden-death insertion paths,
  stairs transition rules, extraction range math).
- The worked example was regenerated by replaying
  `networking.cpp:746-804` constants exactly (`web_hash`, width recursion,
  parent assignment, name dedupe) and spot-checked against the source tables.
- Negative-control fields confirmed absent from any derived field.

## Commit SHAs

- Claim: `f852254d` — pushed to `origin/worker/verdigris/pc/ox-pc-bb`.
- Review head: this closing commit (FINDINGS.md, captures/graph.json,
  captures/acceptance-rg-transcript.txt, tools/build-graph.mjs, REPORT.md,
  STATUS → REVIEW_REQUESTED), pushed to the same worker branch. Branch history
  is append-only from the claim; no force-push, no program-branch writes.

## Deviations

- None from owned_paths/forbidden_paths or stop rules.
- `git add -N` used solely to make new evidence files visible to the literal
  `git diff --check` / `git diff --name-only` acceptance commands; disclosed above.
- The full rg transcript (72,926 bytes, 677 lines) is preserved byte-for-byte in
  `captures/acceptance-rg-transcript.txt` rather than inlined into this report;
  head/tail excerpts plus SHA256 are provided above for verification.

## Unresolved questions / risks

- Owner-only campaign choices (acts, duration targets, branch density, endgame
  content, fast-travel risk model OD-012) remain open; FINDINGS §7 preserves
  them as MISSING rather than deriving values.
- Measured anomalies parked for successors (FINDINGS §9): single onward-gate
  exposure, missing unlock authorization on direct zone entry, dual graph
  bookkeeping reconciling only at two tin ids, placeholder first-clear
  completion rule, session-scoped web progress.

## Follow-ups (mechanical successor candidates)

1. Centralize unlock authorization in `world:zone:enter` (barred-status check
   against cleared parents) — hardens the existing seam without content.
2. Persist per-House road charts natively or record the session-scope decision.
3. Expose all sibling children as onward gates (or record the one-gate rule).
4. Reconcile core route bookkeeping with web node ids (or scope the dispatch).
5. Add playtime telemetry hooks on instance enter/extract beats so future
   pacing work has measured hours instead of intent sentences.

Successor rule restated: all NEW campaign content (zones, acts, rewards,
durations, travel risk) stays owner-authored; this audit changes no contract
and invents none of it.
