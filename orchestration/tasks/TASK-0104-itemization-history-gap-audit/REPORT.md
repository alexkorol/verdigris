# REPORT — TASK-0104: Itemization, extraction, and item-history gap audit

Lane `ox-pc-bb` · model `openrouter/stealth/ox-alpha` · read-only BOUNDED-DESIGN
audit. Claim pushed at `2cfe40cf` on `worker/verdigris/pc/ox-pc-bb`. Evidence
head submitted for review is recorded in STATUS.md (`frozen_review_head`).

## Deliverables

- `FINDINGS.md` — two-universe lifecycle map (core simulation + tile-space
  parity world) across stable IDs, generation, equipment behavior change,
  drops, pickup, inventory, extraction, death recovery (D-106), relic
  candidacy/circulation, scars/history, persistence, wire payloads,
  presentation, and tests; 22-edge coverage matrix; ranked content-neutral gaps
  R1–R8/G-01..G-08; negative controls NC-1 (ItemHistoryUpdated emissions have
  zero test assertions) and NC-2 (snapshot schemaVersion rejection untested).
  Every claim cites file:line.
- `captures/item-lifecycle.json` — machine-readable twin (validated below).

## Acceptance gate transcripts (run literally)

### Gate 1 — mapping grep

```
$ rg -n "item|inventory|equip|drop|pickup|extract|relic|scar|brand|bond|forge|stable.*id" native/include native/src native/client native/tests docs/product
<large transcript; full output captured by the harness at
 C:\Users\Alex\.local\share\opencode\tool-output\tool_02f86ce75001p6dnzcySButvwj
 — spans core.hpp/core.cpp item+inventory+equip+relic+scar+brand lines,
 networking wire shapes, client pickup/extract/relic-toast surfaces, all four
 native test files, and docs/product constitution/checklist/open-decision rows>
EXIT=0
```

Exit code: **0** (matches expected non-empty mapping output).

Negative-control proof of absence (supporting evidence for NC-1):

```
$ rg -n "ItemHistoryUpdated" native/tests
EXIT=1   # no matches — the history event emission is asserted by no test
```

### Gate 2 — machine-readable capture parses

```
$ node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0104-itemization-history-gap-audit/captures/item-lifecycle.json','utf8')); console.log('item lifecycle: PASS')"
item lifecycle: PASS
EXIT=0
```

Exit code: **0**, printed exactly `item lifecycle: PASS`.

### Gate 3 — whitespace/conflict check

```
$ git diff --check
EXIT=0
```

Exit code: **0** (silent — no whitespace errors, no conflict markers).

### Gate 4 — modified-path enumeration

```
$ git diff --name-only
EXIT=0
```

Exit code: **0** with empty output — no tracked file modified. The evidence is
untracked task-folder additions only, confirmed by:

```
$ git status --short
?? orchestration/tasks/TASK-0104-itemization-history-gap-audit/FINDINGS.md
?? orchestration/tasks/TASK-0104-itemization-history-gap-audit/captures/
```

Exactly the owned paths (`orchestration/tasks/TASK-0104-itemization-history-gap-audit/**`)
— nothing else in the tree was touched.

## Expected-result check

- Only task evidence changes: PASS (gate 4 + status above).
- Negative control identified: NC-1 — the *equipped-item use / equip /
  unequip → `EventType::ItemHistoryUpdated` emission* history transition
  (native/src/core.cpp:436,448,523,536) has no automated test anywhere under
  `native/tests` (grep exit 1 above), even though the underlying state is
  tested (native/tests/core_tests.cpp:1557-1563). Secondary recovery-gate gap:
  NC-2 — restore()'s schemaVersion rejection path (core.cpp:1299-1301) is
  untested (positive header check only, core_tests.cpp:763).
- Stopped at formula/economy/content choices: no affix math, economy rates,
  drop rates, or item names proposed; gaps are stated as neutral seams
  (G-02 scar production seam, G-03 seeded uuid stream, G-01 durability
  grammar). Brands/Bonds formula and relic probability tuning remain
  owner-only per FINDINGS "Frozen invariants honored".

## Capsule compliance

Read-only audit: zero writes outside
`orchestration/tasks/TASK-0104-itemization-history-gap-audit/**`; no process
launched against any port; port 6500 never referenced or used; no real save
profile touched (no runtime execution of the game beyond compile-free static
reading and the acceptance commands above).

## Handoff notes for the reviewer/validator

1. Validate at the frozen evidence head recorded in STATUS.md.
2. Highest-value successor work order: G-05 (smallest locking test L1, pure
   test addition), G-07 (schemaVersion rejection test), then the three red
   seams G-01/G-02/G-03 which need design sign-off on identity spine before
   implementation (G-04 decision gates them).
3. Known-flaky note inherited from RUN_STATUS: none of this task's evidence
   depends on the session-test gate-b hunt leg; no binaries were built or run.
