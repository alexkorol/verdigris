# TASK-0104 — REPORT (itemization, extraction, and item-history gap audit)

Worker lane: `ox-sw-g` · Track: read-only audit (BOUNDED-DESIGN) · Branch:
`codex/TASK-0104-itemization-history-gap-audit-ox-sw-g`

## Executive summary

Mapped the full item lifecycle across both native item systems — the headless
core `Simulation` model (`Item`/`Trophy`/`House` pools, seed-stable ids,
durable history vectors, snapshot/restore) and the wire-facing N4/N5
`GameItem` pipeline (`PlayerInventory`, `WearSet`, `GroundItem`,
`VesselForge`, circulation pool) — across `native/include`, `native/src`,
`native/client`, `native/tests`, and `docs/product`. Produced ranked
findings: **10 content-neutral lifecycle gaps** and 7 owner-dependent content
gaps, plus a frozen-invariant risk table. Headline results:

- D-106 recoverability is green within a process but **red across restarts**
  (the wire relic-circulation pool is a process static; nothing on the server
  persists recovery state).
- Significant-item history is durable in the core model but **invisible on
  the wire**, and the unequip transition emits a history event without a
  history line — the audit's negative control (no automated test covers it).
- Extraction has two divergent risk gates (core proximity gate vs wire
  "anywhere in instance") and House storage is split across two pools that
  never converge (`house_store_` vs `bank_`).

No production code was modified; all work is confined to the task folder.

## Approach

1. Preflight + claim commit (STATUS.md).
2. Read binding sources: task brief (SPEC.md absent at base — see
   Deviations), AGENTS.md, VERDIGRIS_CONSTITUTION.md, OPEN_DECISIONS.md,
   config/legacy-denylist.json.
3. Full reads of core.hpp; lifecycle regions of core.cpp (commands, pickup/
   equip/unequip, enter/retire, drops, death, extract, snapshot/restore,
   forge/catalogue/inventory/wearset/world loot); networking.cpp (session
   admission, wire projections, give/drop/equip/take/gold, bank/wagon/shop,
   brand service, combat loot+relic surfacing, extract/final-death/respawn,
   release-relic); persistence adapter; client model/session/presentation.
4. Test inventory of all four suites for lifecycle coverage.
5. Wrote FINDINGS.md (ranked gaps, red risks, negative control, proposed
   locking tests) and captures/item-lifecycle.json (machine-readable surface
   map with file:line citations).
6. Ran all acceptance commands verbatim; recorded outcomes below.

## Changed files

- `orchestration/tasks/TASK-0104-itemization-history-gap-audit/STATUS.md` (new)
- `orchestration/tasks/TASK-0104-itemization-history-gap-audit/FINDINGS.md` (new)
- `orchestration/tasks/TASK-0104-itemization-history-gap-audit/captures/item-lifecycle.json` (new)
- `orchestration/tasks/TASK-0104-itemization-history-gap-audit/REPORT.md` (new)

Nothing outside `orchestration/tasks/TASK-0104-itemization-history-gap-audit/**`
was created or modified.

## Test commands + outcomes (exit codes)

Run from worktree root, PowerShell:

| Command | Exit | Outcome |
|---|---|---|
| `rg -n "item\|inventory\|equip\|drop\|pickup\|extract\|relic\|scar\|brand\|bond\|forge\|stable.*id" native/include native/src native/client native/tests docs/product` | 0 | Matches produced as expected across all five roots (full output captured by harness); confirms citation coverage |
| `node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0104-itemization-history-gap-audit/captures/item-lifecycle.json','utf8')); console.log('item lifecycle: PASS')"` | 0 | Printed `item lifecycle: PASS` |
| `git diff --check` | 0 | No whitespace/conflict-marker problems |
| `git diff --name-only` | 0 | Empty output — working tree clean; only task-folder paths exist in the branch diff |

## Key findings (top-level; full detail in FINDINGS.md)

1. **GAP-2 / D-106 red risk**: `circulation_pool()` is a function-local
   static (networking.cpp:748-750); final-death commits
   (networking.cpp:2133-2143) and elite-kill surfacing (:2083-2108) are
   memory-only. A restart destroys unrecovered heirlooms and strands the
   crypt's `relic.status="lost"` promise. The legacy core's
   snapshot()/restore() persists its pools correctly but is never wired into
   server_main.cpp.
2. **GAP-1 / negative control**: `resolve_unequip` (core.cpp:522-534) emits
   `ItemHistoryUpdated("unequip")` without appending an `item.history` line;
   every sibling transition appends one. Uncovered by tests
   (core_tests.cpp:1228-1233 asserts flags only).
3. **GAP-4/GAP-5 storage/risk seams**: extraction banks into `house_store_`
   (networking.cpp:939-972) while the bank UI trades out of a separate
   `bank_` vector (:1217-1243, :1908-1944) — no convergence; and
   `handle_extract` requires only `world_->in_instance()` (:2116-2127),
   dropping the core's extraction-point proximity gate (core.cpp:854-857).
4. **GAP-3 identity**: wire uuids come from a process-global serial
   (core.cpp:2608-2618) — unique per process, not seed-stable, not persisted,
   while core `Item.id` is replay-stable.
5. Owner-dependent (ranked separately, unresolved by design): scars have no
   producer (core.cpp:2353/:2434 read-only), bonds are always null
   (networking.cpp:371), "notable gear" = everything except coins +
   bronze-dagger (:2133-2137), cadence constants cited not judged (OD-006).

## Negative control

Transition: **item history across unequip** — `resolve_unequip`
(native/src/core.cpp:522-534) emits `EventType::ItemHistoryUpdated` with text
"unequip" but never appends to `item.history`; no automated test exercises
history content around unequip, so the suite stays green whether or not the
durable record matches the event stream. Recorded in FINDINGS.md §D and in
captures/item-lifecycle.json (`negative_control`). Secondary uncovered
recovery transitions noted there: restart survival of `circulation_pool()`
and the crypt `relic.status lost→recovered` flip via `mark_relic_recovered`
(networking.cpp:1602-1631), which has no native automated test.

## Commit SHAs (this lane)

| SHA | Subject |
|---|---|
| `e2487bbe` | chore(TASK-0104): claim lane ox-sw-g |
| `637e03cb` | audit(TASK-0104): itemization/extraction/history gap findings + lifecycle map |
| *(final)* | docs(TASK-0104): report + REVIEW_REQUESTED status — see git log for SHA |

Base: `d2423873c577d299b3b39c56024d1d840993c72b`. Local only; nothing pushed.

## Deviations

- **SPEC.md absent**: `orchestration/tasks/TASK-0104-itemization-history-gap-audit/SPEC.md`
  does not exist at the base commit (folder did not exist). The task brief in
  the lane assignment was treated as binding; scope, acceptance commands, and
  deliverables follow it verbatim.
- No build/test binaries were run: this is a read-only audit lane producing
  evidence documents; the acceptance commands above are grep/JSON/git checks
  and all pass. No native code was changed, so build gates were not required.

## Risks

- Findings cite exact lines at base `d2423873`; future merges may shift line
  numbers before owners triage GAP-1..GAP-10.
- GAP-2 (restart volatility) interacts with any future persistence task;
  until then D-106 holds only within a server lifetime — flagged RED in
  captures/item-lifecycle.json so it cannot be mistaken for green.
- Two denylist exceptions (`bronze-dagger`, `legacyRelicId`) remain live in
  native item paths under documented owner review; this audit adds no new
  exception.

## Follow-ups (proposed, owner-discretionary)

1. Owner ruling + smallest locking test for unequip history contract
   (FINDINGS §E.1).
2. Persistence seam for the circulation pool (or explicit D-106 scope note
   that recoverability is process-lifetime-bounded).
3. Decide single House-store authority (`house_store_` vs `bank_`).
4. Wire projection decision for item history/use_count (GAP-7) and
   uuid stability policy (GAP-3).
5. Extraction risk-model unification between core and wire surfaces (GAP-5).
