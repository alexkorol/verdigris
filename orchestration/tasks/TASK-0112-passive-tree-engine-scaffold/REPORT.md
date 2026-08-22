# TASK-0112 REPORT — ox-pc-c

## Executive summary

Delivered the content-neutral, machine-readable authority contract for the
future native passive tree: `passive-tree-contract.json` (versioned graph,
node, edge, allocation, budget, validation-result, migration, and persistence
envelopes), `VALIDATION.md` (nine deterministic error codes with fixed ranks,
tie-breaks, and stack anchors), and `fixtures/negative-cases.json` (ten
invalid shapes covering all eight required error families plus envelope
malformation and a case that encodes the TASK-0105 native raw-snapshot trust
gap). The two point ledgers are declared structurally distinct with an
explicit `COUNTER_CONFUSION` failure mode; the native +2/axis walk and the
raw-snapshot save path are recorded as named negative controls. Zero topology,
nodes, effects, awards, caps, migration outcomes, or balance were chosen;
every such surface is marked OWNER_PENDING routed through OI-004. All five
literal acceptance commands were run; transcripts and exit codes below.

## Approach

1. Preflight per START_HERE: root/branch/routed HEAD/base/clean/origin proven;
   base confirmed ancestor; no competing claim, RELEASE, or superseding route
   (task folder held only SPEC.md locally and on origin).
2. Claimed via committed+pushed STATUS-only commit inside the window
   (`d14c69d4`).
3. Evidence pass: constitution; TASK-0105 SPEC/REPORT/FINDINGS/REVIEW;
   OI-004; browser authority (`server/core/passives/verdigris-authority.js`);
   native approximation (`native/src/networking.cpp:815-828,1092-1166`,
   `native/include/verdigris/networking.hpp:174-180`).
4. Authored the three artifacts inside the owned path only, then ran every
   literal acceptance command, preserving literal output and exit codes.

## Changed files (worker commits `d14c69d4`, `d302ac32`, plus final REPORT/STATUS)

- `orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/STATUS.md`
- `orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/passive-tree-contract.json`
- `orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/VALIDATION.md`
- `orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/fixtures/negative-cases.json`
- `orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/REPORT.md`

Boundary proof: `git diff --name-only d14c69d4^..HEAD` lists only these task
files. Nothing outside the task folder was created or modified by this worker;
forbidden paths untouched; ports 6660–6679 unused (no servers started); port
6500 untouched.

## Public interfaces added/changed

None in code. This packet adds contract documents only; no production,
browser, native, test, or build surface changed.

## Provenance mapping (contract field/error → current authority / owner-pending)

Legend: **BA** = current browser authority; **NA** = current native
approximation; **OP** = explicitly owner-pending (OI-004 or architect ruling).

| Contract surface | BA anchor | NA anchor | Classification |
|---|---|---|---|
| `graph.graph_version` | `VERDIGRIS_PASSIVE_TREE_SCHEMA_VERSION = 2`, src/core/passives/verdigris-geometric-tree.js (consumed at verdigris-authority.js:32) | hardcoded `schemaVersion 2` networking.cpp:1147 | value OP; observed current = 2 (both stacks) |
| `graph.origin` + mandatory-origin rule | `'The passive tree must include its origin.'` verdigris-authority.js:39; origin id `'0,0'` | same id assumed networking.cpp:1106,1144 | rule CONTRACT; literal id OP |
| `graph.nodes[].node_id` opaque identity | string ids in set membership checks verdigris-authority.js:38-40 | parsed for axes networking.cpp:1102-1112 (negative control) | shape CONTRACT; values OP |
| `node.content` (attributes/effects/cost/visibility/display) | computed by engine computeStats; subtree visibility verdigris-authority.js:86-91 | approximated (+2/axis walk networking.cpp:1092-1119; STUB NOTE 818-821) | entirely OP |
| `edge` (id/from/to/variants) | conduit `{id, variant}` validation verdigris-authority.js:44-54 | conduits echoed unvalidated networking.cpp:1137-1140 | shape CONTRACT; variants/values OP |
| edge cost convention (1 point/choice) | spent formula verdigris-authority.js:73 | mirrored networking.cpp:1135-1139 | observed convention recorded as provenance; binding costs OP |
| `allocation.allocated_nodes/edge_choices` wire | `player:skilltree:save {snapshot}` handler chain | `player:skilltree:save` dispatch networking.cpp:2453 → raw store :1157-1166 | shape CONTRACT (renamed snake_case); wire key mapping documented here |
| `allocation.selected_node` fallback-to-origin | verdigris-authority.js:81,101 | networking.cpp:1129,1141 | rule CONTRACT; semantics preserved |
| `allocation.calling_order` (browser `classOrder`) | verdigris-authority.js:58-65 | echoed networking.cpp:1142 | shape CONTRACT; ordering semantics OP |
| `budget.persistent_commission_points` | `quests.questPoints` chain record (quest-service; survives relogin per TASK-0105 QP-1) | `quest_points_` networking.cpp:1082 | ledger separation CONTRACT; increments/caps/reset OP (QP-2) |
| `budget.live_tree_points` | top-level live counter feeding earned (first-goal-fed, persisted) | `tree_quest_points_` networking.hpp:174-177; login-reset networking.cpp:553 | ledger separation CONTRACT; canonical lifecycle OP (QP-2) |
| `budget.earned/spent/unspent` + OVERSPENT invariant | earnedVerdigrisPoints + spent>earned reject verdigris-authority.js:10-19,73-74 | formula comment networking.cpp:1121-1126; silent clamp :1150 | invariant CONTRACT; numeric constants OP (observed 140/117/23 cited as provenance only) |
| `validation_result.*` codes/ranks | reason strings mapped per code in VALIDATION.md table | absent natively (trust gap) | CONTRACT (new deterministic surface) |
| `migration.full_refund_reset` strategy | resolvePersistedVerdigrisTree one-time refund verdigris-authority.js:121-128 | absent | precedent recorded; per-transition choice OP |
| `migration.unsupported → UNSUPPORTED_MIGRATION` | no explicit analogue (fail-closed is new) | absent | CONTRACT |
| `persistence.validation_provenance` + trust boundary | "snapshots are no longer trusted save blobs" verdigris-authority.js:23-27 | verbatim store networking.cpp:1157-1166 — forbidden negative control | CONTRACT |
| `COUNTER_CONFUSION` | prevented structurally, not coded | divergence documented networking.hpp:175-177 | CONTRACT (new detection) |

Error-code anchors are tabulated in VALIDATION.md ("Stack anchors").

## Acceptance gate transcripts (literal commands from SPEC, run from repo root)

### Gate 1

```
PS Z:\Code\.worktrees\verdigris\ox-pc-c> node -e "const fs=require('fs');const p='orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/passive-tree-contract.json';...console.log('passive-tree contract: PASS')"
passive-tree contract: PASS
EXIT_CODE_GATE1=0
```

Note: first run failed with `Error: counters collapsed` (exit 1) because both
counter descriptors were nested under `budget.properties`, making the probe's
direct reads `undefined === undefined`. Fixed by hoisting
`persistent_commission_points` / `live_tree_points` to direct `budget` keys
with distinct objects; rerun passes. Final state exit code: **0**.

### Gate 2

```
PS> node -e "…fixtures/negative-cases.json…"
passive-tree negative fixtures: PASS
EXIT_CODE_GATE2=0
```

Exit code: **0**. All eight required expected_error values present among cases.

### Gate 3

```
PS> rg -n 'resolveVerdigrisTree|validateSnapshot|questPoints|live_tree_points|player:skilltree:save|\+2|STUB NOTE' server/core/passives/verdigris-authority.js server/game/verdigris-skill-tree.js native/src/networking.cpp native/include/verdigris/networking.hpp
rg: server/game/verdigris-skill-tree.js: native/include/verdigris/networking.hpp:175:  // JS Player.questPoints: the LIVE session tree budget. Unlike the chain
server/core/passives/verdigris-authority.js:10:export const earnedVerdigrisPoints = (level, questPoints = 0) => Math.min(
server/core/passives/verdigris-authority.js:16:    Math.max(0, Math.floor(Number(questPoints) || 0)),
server/core/passives/verdigris-authority.js:28:export const resolveVerdigrisTree = (incoming, level = 1, questPoints = 0) => {
server/core/passives/verdigris-authority.js:36:  const earned = earnedVerdigrisPoints(level, questPoints);
server/core/passives/verdigris-authority.js:113:export const resetVerdigrisTree = (level = 1, questPoints = 0) => resolveVerdigrisTree({
server/core/passives/verdigris-authority.js:119:}, level, questPoints);
server/core/passives/verdigris-authority.js:124:export const resolvePersistedVerdigrisTree = (incoming, level = 1, questPoints = 0) => (
server/core/passives/verdigris-authority.js:126:    ? resolveVerdigrisTree(incoming, level, questPoints)
server/core/passives/verdigris-authority.js:127:    : resetVerdigrisTree(level, questPoints)
server/core/passives/verdigris-authority.js:134:  resolveVerdigrisTree,
native/src/networking.cpp:435: … (base64/hex helper lines matching the \+2 pattern)
native/src/networking.cpp:455: … (bit-shift helpers matching \+2)
native/src/networking.cpp:553:  tree_quest_points_ = 0;  // JS: a rebuilt Player starts with questPoints 0
native/src/networking.cpp:815:  put(state,"questPoints",tree_quest_points_);
native/src/networking.cpp:818:  { // stats-manager attributes: base 10s plus the tree path. STUB NOTE:
native/src/networking.cpp:820:    // as +2/attr per allocated node beyond the root until the geometric
native/src/networking.cpp:1082:  put(quests, "questPoints", quest_points_);
native/src/networking.cpp:1122:  // min(140, min(max(2, level), 117) + min(questPoints, 23)).
native/src/networking.cpp:1542:  put(data, "questPoints", tree_quest_points_);
native/src/networking.cpp:2453:  if (envelope.event=="player:skilltree:save") { if (payload) handle_skilltree_save(*payload, emit); return; }
The system cannot find the path specified. (os error 3)
EXIT_CODE_GATE3=2
```

Exit code: **2** — the SPEC's third scan path
`server/game/verdigris-skill-tree.js` does not exist in this repository; rg
still scanned the other three paths and printed their matches before failing
on the missing path (stderr/stdout interleaved above exactly as captured; the
two `networking.cpp:435/455` hits are bit-arithmetic coincidences of the
literal `\+2` alternative). Observed reality: the browser skill-tree economy
module lives at `src/core/passives/verdigris-skill-tree.js` (imported by
verdigris-authority.js:5-8). Command run verbatim as required; no path was
substituted. Substantively the gate confirms every load-bearing anchor the
contract cites exists where cited.

### Gate 4

```
PS> git diff --check
EXIT_CODE_GATE4=0
```

Exit code: **0** (no whitespace/conflict-marker problems).

### Gate 5

```
PS> git diff --name-only cab50d62cb121ab6a88fa513257e645447226959..HEAD
orchestration/REENTRY-OX-ALPHA-PC.md
orchestration/RUN_STATUS.md
orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/SPEC.md
orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/STATUS.md
orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/VALIDATION.md
orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/fixtures/negative-cases.json
orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/passive-tree-contract.json
EXIT_CODE_GATE5=0
```

Exit code: **0**. Attribution: `cab50d62..b3599c80` is exactly one
pre-existing control-plane commit (`b3599c80` "orchestration: expand PC
OpenRouter fleet to eight lanes" — REENTRY/RUN_STATUS updates plus new SPECs
TASK-0130..TASK-0134 and this task's SPEC), which predates my claim and is not
my work. My worker commits (`d14c69d4`, `d302ac32`, plus the final
REPORT/STATUS commit) touch only
`orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/**`, proven by
`git diff --name-only d14c69d4^..HEAD`.

## Manual verification

- Executed both JSON gates after each edit until green; fixtures assert all
  eight required codes (plus MALFORMED_ALLOCATION and NEG-010 trust-gap case).
- Cross-read every VALIDATION.md anchor line against the live files during the
  evidence pass (verdigris-authority.js quoted lines verified at :29-34, :39,
  :40, :44-50, :66-71, :73-74, :84-91, :121-128; networking.cpp at :553, :815,
  :818-821, :1082, :1092-1119, :1121-1126, :1147, :1157-1166, :2453;
  networking.hpp at :174-180).

## Commits (this branch, in order)

- `d14c69d4` — claim-only STATUS push (within 10-minute window).
- `d302ac32` — contract + VALIDATION + negative fixtures + STATUS→IMPLEMENTED.
- (this commit) — REPORT + STATUS→REVIEW_REQUESTED.

Pushed only to `codex/TASK-0112-passive-tree-authority-schema-ox-pc-c`; never
merged, never force-pushed.

## Deviations

1. **Gate 3 exits 2** because the SPEC names a nonexistent scan path
   (`server/game/verdigris-skill-tree.js`). Run literally and reported
   verbatim; actual module location noted above. No files were moved or
   created to make the gate exit 0, since doing so would write outside the
   owned path.
2. **Gate 5 lists control-plane files** from the controller's own routed-head
   commit between immutable base and claim; attribution shown above; no worker
   writes occurred outside the task folder.
3. None otherwise. No env overrides, no watch servers, ports 6660–6679 and
   6500 untouched.

## Unresolved questions

1. **QP-2 (routed, must not be guessed):** canonical increment sources, caps,
   and reset rules for `persistent_commission_points` vs `live_tree_points`.
   Until ruled, every conforming implementation must keep the ledgers separate
   and raise COUNTER_CONFUSION on collapse; the earned derivation remains
   OWNER_PENDING.
2. Which migration strategy applies to which historical version transition
   (refund vs revalidate) — OI-004/owner.
3. Whether the future native wire reuses browser camelCase keys
   (`selectedNodeId`, `classOrder`) or adopts the contract's snake_case;
   presentation/wire decision intentionally out of scope here.

## Risks / follow-ups

- The successor implementation task must port the deterministic engine behind
  a flag, parity-test against `resolveVerdigrisTree` outputs, and delete the
  +2 walk only when green (per TASK-0105 FINDINGS scaffold proposal); combat
  power currently consumes the approximation, so the swap is an intentional,
  reviewable behavior change.
- `MALFORMED_ALLOCATION` is a ninth code beyond the eight required families,
  justified by the browser's distinct container-level rejection
  (verdigris-authority.js:29-31); reviewer may collapse it if undesirable.
- Fixture numbers (e.g., `earned: 2`) are synthetic context for shape testing
  only and assert no balance.

## Status

STATE → REVIEW_REQUESTED. Branch pushed at the reviewed head; awaiting Tier-A
mechanical review at exact head/base.
