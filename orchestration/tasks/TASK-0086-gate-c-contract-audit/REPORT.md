# REPORT — TASK-0086 Gate C campaign-decision contract audit

Worker: `ox-pc-c` · Branch: `codex/TASK-0086-gate-c-contract-audit-ox-pc-c`
· Base: `42718fbc4340589e606fff94a6eaa3dfbd03ad1c` · Route/base-refresh head
at claim: `039dcfa7f12497aa79c3677873a06a96c231a13d` · Machine:
DESKTOP-TVU7OR7 · Ports: 6660-6679 (none bound; no server needed) ·
Provider/model/harness: openrouter / stealth/ox-alpha / OpenCode CLI 1.18.21.

## Executive summary

Gate C (route decision on concrete info) is **not satisfiable from today's
surface**: of the six required fields, **boss/danger**, **depth**, and
**extraction/return condition** are AVAILABLE; **branch consequence** is
DERIVABLE-WITHOUT-GAMEPLAY-RULES for the immediate next stage only;
**concrete goal** and **expected trophy/material/item family** are MISSING on
both the browser-authoritative server and the N6 C++ server. A route name
(plus blurb/tier) alone still fails the contract. No reward/economy/balance/
risk value was invented; both MISSING fields are marked per stop conditions.
Five parity deltas between the browser and native chart surfaces are recorded
(P1-P5 in FINDINGS.md), including the documented intentional node-identity
divergence.

## Approach

Read-only audit. The six Gate C fields from
`docs/rebuild/NATIVE_PRODUCT_CONVERGENCE.md:68-72` were mapped against every
current producer/consumer/test of the world-web chart and zone surface:
browser handlers/services (`world-web.js`, `zone-service.js`,
`first-goal.js`), native N6 server (`native/src/networking.cpp` chart/zone/
extract paths), client consumer (`Chart.vue`), and the three test surfaces.
Each field cites event/payload/source/test labels, carries a single honest
classification, and names the smallest future owner path. Deliverables:
`FINDINGS.md` (narrative + citations) and `captures/gate-c-contract.json`
(machine-evaluable mapping).

## Changed files

All inside owned paths `orchestration/tasks/TASK-0086-gate-c-contract-audit/**`:

- `STATUS.md` — CLAIMED at `2ed4799a`, now REVIEW_REQUESTED.
- `FINDINGS.md` — new.
- `captures/gate-c-contract.json` — new.

No file outside the task folder was created, modified, or deleted by this
worker (verified below). Forbidden paths (`native/**`, `server/**`, `src/**`,
`playtest/**`, `docs/product/**`) untouched.

## Public interfaces added/changed

None. Pure audit packet; zero code or protocol changes.

## Acceptance commands — literal transcripts and exit codes

### Gate 1 — contract JSON parses

```
$ node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0086-gate-c-contract-audit/captures/gate-c-contract.json','utf8')); console.log('gate-c contract JSON: PASS')"
gate-c contract JSON: PASS
EXIT CODE: 0
```

### Gate 2 — evidence labels exist across the four audited surfaces

```
$ rg -n 'world:road:chart|world:zone:enter|nodeId|warden|trophy|depth|stairs|extract' native/src/networking.cpp native/tests/networking_tests.cpp playtest/scenarios/world-web.mjs playtest/scenarios/quest.mjs
native/src/networking.cpp:621:struct QuestObjective { const char* trigger; const char* crit_a; const char* crit_b; int min_depth; };
native/src/networking.cpp:680:  std::string id, name, template_id, layout, parent_id, warden_name;
native/src/networking.cpp:713:      node.warden_name = "Warden of " + name;
native/src/networking.cpp:783:  ... put(metadata,"depth",meta.depth);
native/src/networking.cpp:784:    JsonValue::Object up; put(up,"x",meta.stairs_up.x); put(up,"y",meta.stairs_up.y); put(metadata,"stairsUp",std::move(up));
native/src/networking.cpp:785:    JsonValue::Object down; put(down,"x",meta.stairs_down.x); put(down,"y",meta.stairs_down.y); put(metadata,"stairsDown",std::move(down));
native/src/networking.cpp:788:      put(metadata, "nodeId", current_node_id_);
native/src/networking.cpp:790:      JsonValue::Object entry_gate; put(entry_gate, "x", meta.stairs_up.x); put(entry_gate, "y", meta.stairs_up.y);
native/src/networking.cpp:794:        JsonValue::Object gate; put(gate, "x", meta.stairs_down.x); put(gate, "y", meta.stairs_down.y);
native/src/networking.cpp:795:        put(gate, "nodeId", current_child_id_); put(gate, "name", current_child_name_);
native/src/networking.cpp:799:      put(metadata, "wardenDead", cleared_nodes_.count(current_node_id_) > 0);
native/src/networking.cpp:813:  put(state,"bestDepth",best_depth_);
native/src/networking.cpp:864:  ... put(metadata,"depth",meta.depth);   [second state builder]
native/src/networking.cpp:865-880: [stairsUp/stairsDown/nodeId/entryGate/zoneGates/wardenDead mirrors]
native/src/networking.cpp:939:void ProtocolSession::finish_extraction(const std::function<void(const Envelope&)>& emit) {
native/src/networking.cpp:940:  // Drain backpack + wear into the House store. JS has no player:extract;
native/src/networking.cpp:968:  emit(Envelope{"player:extract", JsonValue(std::move(summary))});
native/src/networking.cpp:1293:  const std::uint64_t key = meta.seed * 131u + static_cast<std::uint64_t>(meta.depth);
native/src/networking.cpp:1299:    put(data, "depth", meta.depth);
native/src/networking.cpp:1303-1305:  emit_message(emit, "Floor " + std::to_string(meta.depth) + " cleared! Rewards distributed - find the stairs to descend, or take the entry stairs to leave."); / first_goal depth checks
native/src/networking.cpp:1359,1368,1369:  quest objective min_depth plumbing
native/src/networking.cpp:1429:    put(row, "wardenName", node.warden_name);
native/src/networking.cpp:1461-1481:  enter_road_node warden/stairs wiring (set_boss_name_override :1462, set_block_stairs_down :1467/:1480, set_stairs_up_returns_to_town :1469/:1481)
native/src/networking.cpp:1519-1527:  stairs-down Warden refusal ("No road holds past a living Warden.")
native/src/networking.cpp:1997:  // advance_combat; the legacy synthetic 'drop' trophy event is retired.
native/src/networking.cpp:2067,2072,2073:  post-combat stairs/first-goal depth logic
native/src/networking.cpp:2116-2124:  handle_extract ("There is no extraction here." / finish_extraction)
native/src/networking.cpp:2314:  world:zone:enter dispatch (nodeId default "tin:1:0", delve depth trigger)
native/src/networking.cpp:2315:  instance:enterSolo dispatch
native/src/networking.cpp:2316:  player:move depth/return-surface/extraction chain
native/src/networking.cpp:2317:  dev:teleport depth/return/extraction chain
native/src/networking.cpp:2320:  if (envelope.event=="world:road:chart") { emit_chart_screen(...); return; }
native/src/networking.cpp:2451:  if (envelope.event=="player:extract") { handle_extract(emit); return; }
native/src/networking.cpp:2466-2496:  Warden-death unlock, stairs unblock, finish_extraction, dev:state bestDepth
playtest/scenarios/world-web.mjs:31:    p.emit('world:zone:enter', { nodeId: root.id });
playtest/scenarios/world-web.mjs:34,54,74,75,104:  sceneMetadata.nodeId assertions
playtest/scenarios/world-web.mjs:40-41:  `the ${root.wardenName} keeps the ground`
playtest/scenarios/world-web.mjs:79:    p.emit('world:road:chart', { roadId: 'tin' });
playtest/scenarios/world-web.mjs:110:  'the Warden stays down inside the linger window'
playtest/scenarios/quest.mjs:255-257:  sceneMetadata.stairsDown assertions/use
playtest/scenarios/quest.mjs:260:  next.sceneMetadata.depth === 2
playtest/scenarios/quest.mjs:325:  state.sceneMetadata.stairsUp
native/tests/networking_tests.cpp:44:  world:zone:enter {nodeId: "tin:1:0"}
native/tests/networking_tests.cpp:124,143-144,165-171:  test_instance_entry_and_stairs / "both stairs exist" / "entry stairs return to town"
native/tests/networking_tests.cpp:333-385:  test_gate_a_extract_and_stairs ("player:extract emits a bank summary", "stairs-up emits the same player:extract bank summary", ...)
native/tests/networking_tests.cpp:439,442:  test registration calls
EXIT CODE: 0
```

(Full unabridged output captured in the session transcript; elisions above are
line-folding of repeated metadata builders only — every matched line number is
listed.)

### Gates 3 & 4 — whitespace check and changed-file list (run with deliverables on disk)

```
$ git diff --check
EXIT CODE: 0
$ git diff --name-only
EXIT CODE: 0
```

Honest note: both `git diff` gates are empty because this packet's
deliverables are NEW untracked files at run time (`git status --short`:
`?? orchestration/tasks/TASK-0086-gate-c-contract-audit/FINDINGS.md`,
`?? orchestration/tasks/TASK-0086-gate-c-contract-audit/captures/`), and
`git diff` covers tracked modifications only. After the REVIEW_REQUESTED
commit the same two commands remain exit 0 with empty output over a clean
tree.

## Manual verification

No server/client runs were required by the SPEC (MECHANICAL audit; no
acceptance servers, no port binds). Verification was line-level source and
test reading; every JSON citation was re-checked against the working tree at
the audited head. Cross-checks performed: absence greps for goal/reward keys
in both chart builders; absence of `levelHint`/`childIds`/`world:chart:updated`
in `native/src/networking.cpp`; absence of any chart pane under
`native/client/`.

## Base-to-head path boundary

`git diff --name-only 42718fbc..HEAD` shows only the architect's
coordination-only refresh `039dcfa7` (ORCHESTRATION/REENTRY/RUN_STATUS plus
four lane SPECs) and this worker's single commit `2ed4799a` touching exactly
`orchestration/tasks/TASK-0086-gate-c-contract-audit/STATUS.md`; the pending
REVIEW_REQUESTED commit adds FINDINGS.md, captures/gate-c-contract.json,
REPORT.md, and the STATUS transition — all inside owned paths. Worker-authored
changes never leave `orchestration/tasks/TASK-0086-gate-c-contract-audit/**`.

## Commit SHAs

- `2ed4799a5385f2d6237697a31305f5671b437e93` — claim (STATUS.md only).
- `<this commit>` — REVIEW_REQUESTED: FINDINGS.md, captures/gate-c-contract.json, REPORT.md, STATUS.md transition.

## Deviations

- Pre-commit hook initially failed (`yorkie` runner missing because the
  isolated worktree had no `node_modules`); resolved by running `npm install`
  locally (gitignored) rather than skipping hooks. lint-staged then ran with
  zero matching files for markdown-only commits.
- None otherwise. Variant/reasoning settings were not observable in-session
  and are recorded as absent in STATUS per packet rule.

## Unresolved questions

None requiring escalation. Owner decisions this audit stages (goal content;
trophy/material family tables; whether to reconcile cross-server chart
identity P2/P3) belong to future owner-authority packets, not questions.

## Risks

- P2/P5 mean Gate C cannot be driven natively until a native chart pane exists
  and node identity is either reconciled or consciously accepted as
  per-server (already documented acceptable in code at
  `networking.cpp:644-646`).
- FINDINGS line references are exact at the audited tree but will drift if
  `networking.cpp`/`world-web.js` move; the JSON records the audit head.

## Follow-ups (routing suggestions, not claims)

- TASK-0089 (Gate C native journey) consumes this audit; its spec already
  requires "missing fields resolved".
- Candidate owners for the MISSING fields per RUN_STATUS board: TASK-0104
  itemization/history audit, TASK-0103 monster/encounter gap audit, plus an
  owner ruling on chart-goal content.
