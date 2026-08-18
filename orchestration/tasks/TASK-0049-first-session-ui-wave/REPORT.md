---
task: TASK-0049
state: REVIEW_REQUESTED
coordinator: deepseek
worker_branch: codex/TASK-0049-first-session-ui-wave-deepseek
base_commit: 34b7069f12930933b66fde0e81c27e2cb44007e8
architect_review_required: true
---

# TASK-0049 REPORT — First-session UI wave

## Executive summary

All five accepted TASK-0046 SURVIVES items are closed with client-only
presentation changes. No `server/**`, `native/**`, or playtest-assertion
changes were made. The five deliverables:

1. **House/Scion identity in the world HUD** — a compact chip beside the HP
   orb reads `House Ember — Asha (Mortal oath)` and stays visible after Set
   Out.
2. **Directive mana rejection** — the bare `Not enough mana.` chat line is
   rewritten to `Need N more mana — recovering X every 2s.` using the live
   resource state and the last-attempted skill's cost.
3. **Tutorial ticker legibility** — Aldwyn's first-session beats surface as a
   larger transient banner while combat lines keep flowing through the ticker.
4. **Zone objective preview** — each Adventure row states a concrete draw
   (`Warden of the Deep · item-level 10 gear · depth 1`).
5. **Skill tree first-allocation hint** — with unspent points and nothing
   allocated, the pane names and highlights a data-driven starter node.

Gates are green on the final code (transcripts below), and a hard-fail
Playwright capture produced five rendered screenshots with on-screen text
assertions, all passing.

## Approach

1. **House identity** (`src/Delaford.vue`, `GameContainer.vue`, `GameHUD.vue`).
   A `houseIdentity` computed reads `game.player.houseName`/`username` and the
   `chronicles.mortal` flag; because the direct-admission Chronicles flow
   carries the scion name on `player.username` but not the house name, the
   house name falls back to the account-scoped Chronicles cache
   (`loadHouses(Socket.chroniclesAccountId)`). `GameHUD` renders the label
   `House <H> — <S> (Mortal oath | Soft return)`.

2. **Mana directive** (`src/core/mana-directive.js`, `resource.js`,
   `GameCanvas.vue`, `Delaford.vue`). `recordSkillAttempt` is called at both
   client skill-dispatch points. The `game:send:message` handler rewrites the
   exact `Not enough mana.` text using `formatManaRejection(player)`, which
   reads `stats.resources.mana` and the last skill's `resourceCost.mana`.

3. **Guide banner** (`src/core/tutorial-beats.js`,
   `src/components/ui/world/GuideBanner.vue`, `GameContainer.vue`). Aldwyn
   beats are detected by the `Aldwyn the Guide:` prefix, re-emitted on a
   `tutorial:beat` bus event, and rendered as a 9-second transient banner
   capped at the first 12 beats.

4. **Zone objectives** (`src/core/adventure-objective-data.js`,
   `src/core/adventure-objectives.js`, `GameContainer.vue`). Each solo zone's
   `template` maps to its generated theme boss, and the guaranteed treasure
   item level comes from the first-delve depth. The row renders
   `zoneObjective(zone).line`.

5. **Skill-tree hint** (`verdigris-geometric-tree.js`,
   `GeometricSkillTreePane.vue`). `recommendFirstAllocation()` walks the tree,
   requires unspent points + no non-origin allocations, and picks the
   strongest starter node directly off the Origin (highest authored `amount`,
   deterministic tie-break). `toState()` exposes it as
   `firstAllocationHint`; the pane renders a "Start here" callout and the SVG
   renderer adds a `recommended` ring to that node.

## Changed files

New:

- `src/core/mana-directive.js` — directive formatter + skill-attempt tracking
- `src/core/tutorial-beats.js` — guide-beat detection + surfacing cap
- `src/core/adventure-objective-data.js` — mirrored server display constants
- `src/core/adventure-objectives.js` — per-zone objective line
- `src/components/ui/world/GuideBanner.vue` — transient first-session banner
- `tests/unit/mana-directive.spec.js` (6), `tutorial-beats.spec.js` (3),
  `adventure-objectives.spec.js` (3), `skill-tree-hint.spec.js` (5),
  `first-session-ui.spec.js` (4)
- `orchestration/tasks/TASK-0049-first-session-ui-wave/captures/` — capture
  script, five rendered PNGs, checks JSON

Modified:

- `src/Delaford.vue` — `houseIdentity` computed, `recordSkillAttempt` fallback,
  `:house-identity` pass-through
- `src/components/GameCanvas.vue` — `recordSkillAttempt` on skill dispatch
- `src/components/layout/GameContainer.vue` — GuideBanner mount + bus listener,
  zone objective line, `house-identity` pass-through
- `src/components/layout/GameHUD.vue` — identity chip (prop + computed + CSS)
- `src/components/passives/GeometricSkillTreePane.vue` — hint callout +
  `recommended` node highlight
- `src/core/passives/verdigris-geometric-tree.js` — `recommendFirstAllocation()`
  + `firstAllocationHint` in `toState()`
- `src/core/player/events/resource.js` — mana rewrite + `tutorial:beat` emit

## Public interfaces added

- `mana-directive.js`: `recordSkillAttempt`, `lastAttemptedSkill`,
  `manaRegenAmount`, `readMana`, `formatManaRejection`, `MANA_REGEN_INTERVAL_S`,
  `MANA_REGEN_FRACTION`.
- `tutorial-beats.js`: `isGuideMessage`, `stripGuidePrefix`,
  `shouldSurfaceGuideBeat`, `GUIDE_PREFIX`, `DEFAULT_MAX_GUIDE_BEATS`.
- `adventure-objectives.js`: `zoneObjective(zone)` → `{ warden, itemLevel,
  depth, line }`.
- `VerdigrisGeometricTree.recommendFirstAllocation()` → starter-node descriptor
  or `null`; surfaced in `toState().firstAllocationHint`.
- New bus event `tutorial:beat` (client-internal, no protocol change).

## Test commands and outcomes

`npm run test:unit` — green (final code):

```
 Test Files  128 passed (128)
      Tests  809 passed (809)
```

`npm run build` — green (379 → 128 modules transformed; build succeeded).

`npm run smoke:browser` — green:

```
  ok 1 tests\e2e\browser-critical-loop.spec.mjs:135:1 › the built game supports the browser-critical guest loop (17.8s)
  1 passed (20.1s)
```

`npm run playtest` — green on a calm run:

```
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":32.243711,"maxEventLoopLagMs":116.195327}
```

Note: this task changes no `server/**` code and no playtest scenario; the
harness boots its own isolated server on :6510 with hermetic saves. During
concurrent npm/build/capture activity a few full runs hit the pre-existing
timing flakes documented in the board watch item (`first-goal` 8s authored
deadline, `house-treasury` carried-gold assert, `zones`/`encounter-variety`
8s transition at ~130ms peak lag); the same command re-run alone is 32/32.

## Manual verification (captures/)

`orchestration/tasks/TASK-0049-first-session-ui-wave/captures/capture-0049.mjs`
(Playwright, real Chromium, real server on 127.0.0.1:6500; Chronicles flow
→ Set Out → world). Hard-fail: exits non-zero unless every check passes.
Output: `CAPTURES OK {"guideBanner":true,"houseIdentity":true,"zoneObjective":true,"skillTreeHint":true,"manaDirective":true}`.

- `01-house-identity.png` — HUD chip text `House Ember-… — Asha-… (Mortal oath)`
- `02-mana-directive.png` — chat shows `Need 2 more mana — recovering 2 every 2s.`
- `03-guide-banner.png` — banner text `First things first: use W, A, S and D …`
- `04-zone-objective.png` — `Warden of the Deep · item-level 10 gear · depth 1`
- `05-skill-tree-hint.png` — `Start here … Light Step … +24 to Evasion …`
- `capture-0049-checks.json` — assertions + captured text + final mana (4).

## Deviations

1. **Display constants mirrored by value.** `MANA_REGEN_FRACTION`/interval
   (deliverable 2), the theme-boss names, and `instanceItemLevelForDepth`
   (deliverable 4) live in `server/core/**`, which is forbidden for this task,
   and are not present in any client payload. They are mirrored in
   `adventure-objective-data.js` / `mana-directive.js` with a pointer to the
   server source, the same pattern as the 0009/0013 quickbar display-cost
   watch item. These must be re-synced if the core constants ever change.
2. **House-name fallback.** The direct-admission Chronicles path does not put
   `houseName` on the self player; the chip reads the account-scoped Chronicles
   cache for it (client-only, no protocol change).
3. **"depth" in the objective line** is the solo delve's opening floor (1), per
   `startSoloInstance → enterFloor(party, 1)`; the boss name is the actual
   generated theme boss, so no new canon is invented.

## Risks / follow-ups

- The mirrored display constants are the standing 0009/0013 drift risk; a
  future snapshot/query seam (ADR-002 direction) would remove them.
- The banner and mana copy are client rewrites of server-authored strings; if
  the server copy ever changes, `isGuideMessage`/`formatManaRejection` need a
  matching update.
- The playtest timing flakes are environmental and pre-date this task; no
  scenario or assertion was touched.

## Commits

- `896a8ed` — claim (STATUS.md)
- (implementation + report commit; SHA recorded at push)
