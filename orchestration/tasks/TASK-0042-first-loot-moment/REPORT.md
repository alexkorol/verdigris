---
task: TASK-0042
state: REVIEW_REQUESTED
worker_commits:
  - 560fb265
  - 06529910
  - 296785a0
base_commit: ca0dd2d
merged_tip: e5c4367
---

## Summary

The first drop of a session is now a moment. The session's first delve
(The Old Barrow, the D-114 encounter table) guarantees one curated
Verdigris item within the first three kills — Flint Spear, Hide Wrap, or
Bronze Roundshield, drawn from existing item data. The drop lands on its
own tile beside the coin bounty, tagged `firstFind`, and the client gives
it the full beat: a gold ground ring + light beam + floating name label
in the 2.5D renderer, the loot chime, a HUD first-action prompt naming
the Take affordance, and on pickup a compact `LootMoment` toast with a
one-line stat comparison against what the scion currently holds in that
slot ("+12 stab attack — nothing held in that slot").

## Approach

- Server (`server/core/combat/loot.js`): `FIRST_FIND` named constant
  (kill window 3, frozen pool of three existing verdigris.js base ids).
  Session-scoped per player object via WeakMap; fires once, retries
  across the window only if item creation itself fails. The find is
  placed on a safe neighbouring tile that no other drop occupies, so the
  underfoot grab (Z) reaches it directly instead of the coin pile.
- Client drop beat (`src/core/player/events/world.js` +
  `loot-moment.js`): the `world:itemDropped` diff spots the newly-tagged
  item, plays the existing `sound:loot` chime, and raises the existing
  `game:context-menu:first-only` HUD prompt ("Take Flint Spear — press Z
  underfoot"). The server also sends a chat line naming the drop.
- Client pickup beat (`src/core/player/events/item.js` +
  `loot-moment.js` + `src/components/ui/LootMoment.vue`): the inventory
  refresh diff spots the tagged item crossing ground → inventory and
  opens `LootMoment` through the existing `open:screen` seam — name,
  examine text, comparison line, dismisses itself after 7s. The
  comparison is honest: dominant combat category of the find, top two
  stat deltas vs the held item, plain "Even trade" when equal. A find
  toasts once per uuid, so persisted items never re-toast on login.
- Ground presentation (`src/core/rendering/perspective-renderer.js`):
  one additive method `drawFirstFindHighlight` — pulsing gold ring,
  additive light beam, name label — called from `drawItem` when
  `item.firstFind`. Uses the existing projection/anchor vocabulary.

## QUESTION-0007 note (deviation, review adjudication requested)

The codex/Luna claim blocked on the presentation seam: no `Loot*.vue`
was mounted and the renderer is outside `owned_paths`. Implementation
showed the seam is smaller than feared: the toast mounts through the
existing `open:screen` bus seam (GameCanvas renders the passed component
with `:game`/`:data`), so no `GameContainer.vue` wiring was needed. The
ground beam/label genuinely required `perspective-renderer.js`; the edit
is one additive method plus a timestamp passthrough, ~75 lines, no
change to any existing draw path (untagged items render identically).
The spec's acceptance criteria (ground highlight/beam + name label,
D-115 "feel the beat") could not be met from owned paths alone; this is
QUESTION-0007's option 1, executed minimally. If review disagrees, the
renderer method is self-contained and reverts cleanly.

## Changed files

- `server/core/combat/loot.js` — FIRST_FIND rule + export.
- `src/core/player/events/loot-moment.js` — new: drop/pickup detection,
  comparison builder, announce/present beats.
- `src/core/player/events/world.js` — drop-beat hook on
  `world:itemDropped`.
- `src/core/player/events/item.js` — pickup-beat hook on
  `core:refresh:inventory`.
- `src/components/ui/LootMoment.vue` — new toast component.
- `src/core/rendering/perspective-renderer.js` — ground beam/ring/label
  for tagged finds (see deviation note).
- `tests/unit/loot-first-find.spec.js` — 12 tests.
- `orchestration/tasks/TASK-0042-first-loot-moment/captures/` — driver +
  three JPEG captures.
- `docs/loop-journal.md` — session-arc telemetry rows appended by the
  mandated playtest runs (harness-written; see Risks).

No new items, affixes, Vesselforge, or economy changes (stop conditions
respected). `server/core/data/items/verdigris.js` was listed as owned
but needed no change — the pool references existing ids.

## Evidence

- `npm run test:unit` — 124 files, 800/800 tests PASS at the merged tip
  (includes the 12-test first-find suite: guarantee/window/retry/scene
  gating, comparison content, client seams, world-event wiring).
- `npm run playtest` — 32/32 scenarios PASS at the merged tip
  (p99 event-loop lag 32.3ms). Two earlier attempts flaked: one run
  died mid-session-arc because a `vite build` was running concurrently
  (resource contention — session-arc passes alone, and the full suite
  passes when isolated); one run hit marginal zone-transition timeouts
  (8.05s vs 8s authored) that also reproduce-then-clear at the pure
  program tip (verified via a clean worktree at e5c4367: zones+respawn
  2/2), so neither is caused by this diff.
- `npm run build` — PASS (11.29s, pre-existing chunk-size warning only).
- ESLint + stylelint on all changed files — PASS (also enforced by the
  lint-staged pre-commit hook).
- Browser gate: default `npm run smoke:browser` cannot run on this
  machine — port 6500 is owned by the owner's persistent server (PID
  17960), the same environmental conflict TASK-0041 recorded. Per that
  precedent the alternate-port gate ran instead: own development server
  on 127.0.0.1:6512 + `PLAYWRIGHT_BASE_URL=http://127.0.0.1:6512 npx
  playwright test tests/e2e/browser-critical-loop.spec.mjs` — 1/1 PASS
  (19.2s). An earlier default-port attempt failed because it exercised
  the owner's older server, not this branch.
- Stale-base check: `git diff origin/codex/native-reconstitution HEAD`
  over the touched areas is empty except this task's own additions; the
  branch was merged up to program tip e5c4367 and every gate rerun at
  the merged state.

## Captures (real, this branch's build, driven in a real browser)

- `captures/01-drop-moment.jpg` — the tagged Flint Spear on the ground
  with gold ring, beam, and floating name label; HUD prompt top-left.
- `captures/02-pickup-prompt.jpg` — standing on the find; HUD prompt
  "Take Flint Spear — press Z underfoot".
- `captures/03-comparison-toast.jpg` — the LootMoment toast: name,
  examine, "+12 stab attack — nothing held in that slot", equip hint.
- `captures/_driver.mjs` — reproducible driver (dev events for
  movement/combat; the pickup itself is the real Z key).

## Manual verification

Played end to end in a real Chromium via `_driver.mjs` against this
branch's production build served by this branch's server: guest login →
Chronicles onboarding → Adventure → The Old Barrow → first kill drops
the tagged find (gold beam + label visible) → walk on, HUD prompt shows
→ Z picks it up → toast with comparison renders and auto-dismisses.

## Deviations

1. `src/core/rendering/perspective-renderer.js` is outside
   `owned_paths` — see the QUESTION-0007 note above.
2. `docs/loop-journal.md` rows appended by the playtest gate itself.

## Risks / watch items

- The session-arc critic score in `docs/loop-journal.md` reads 80 on
  this branch's runs (was 100 in the 2026-08-14 rows). The failed
  criterion is the TTK L1/L5 ratio (0.13s → 0.25s), a combat-timing
  metric this task does not touch; first-drop time is unchanged (5.7s).
  The shift was already present in the 2026-08-18T00:42 row before this
  session's client work. Flagged for the architect rather than chased.
- The find is session-scoped (WeakMap keyed on the player object): a
  relog earns a fresh first find. That is the intended reading of "the
  first drop of a session"; if the architect wants once-per-scion
  persistence, that is a Chronicles-storage follow-up.
- `dev:state` strips ad-hoc item fields, so the harness cannot see the
  `firstFind` tag; the capture driver watches the real
  `world:itemDropped` broadcast instead. If a future playtest scenario
  should assert the tag, add `firstFind` to the dev:state groundItems
  projection (server/player/handlers/dev.js, outside this task's paths).

## Follow-ups

- D-115 play gate: the architect's own session should feel the beat —
  the three captures are the proxy evidence.
- Optional: extend the ground highlight vocabulary to elite/relic drops
  (separate task; the method takes any tagged item).
