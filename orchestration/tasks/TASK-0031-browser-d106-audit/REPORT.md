---
task: TASK-0031
state: REVIEW_REQUESTED
coordinator: kimi
worker: kimi-code-cli
branch: codex/TASK-0031-browser-d106-audit
base_commit: fe43dbd
---

# TASK-0031 report — browser death/relic behavior vs D-106/D-109 (read-only audit)

Read-only audit. No files outside this task folder were touched;
`git status --short` shows only this folder. Every claim carries path:line
evidence from the checkout at base `fe43dbd`.

## 1. Current browser death path — walkthrough

### 1.1 Shared lifecycle (all modes)

Damage entry: monster hits route through
`server/core/entities/monster/combat-controller.js:339` →
`Player.applyDamage` in `server/shared/stats/index.js:355-395`. At 0 HP the
player gets one cheat-death chance (1 charge, 300 s cooldown,
`shared/stats/index.js:255-299`, defaults `:32-37`), then `markDeath`
(`:301-353`):

- `lifecycle.mode 'soft'` (the default, `:28`): state → `awaiting-respawn`,
  `respawn.at = now + 10 000 ms` (`:330-333`, default `:41`). **No item, wear,
  bank, or currency is touched — nothing is dropped, destroyed, or moved.**
  The only item-drop function in the game is player-initiated
  (`server/player/handlers/actions/index.js:454-476`). No corpse/gravestone
  mechanic exists. `respawn.penaltyMultiplier` (`shared/stats/index.js:44`)
  is defined but never read — no death penalty of any kind.
- `lifecycle.mode 'hard'` with no extra lives: state → `permadead`
  (`:342-344`), force-persisted immediately
  (`server/core/entities/player/stats-manager.js:256-259`) so reconnect
  cannot revive; the killing monster calls `entombFallenScion`
  (`combat-controller.js:363-370`).

Respawn (soft): `server/core/combat/index.js:856-894` — HP/mana to 50 %
(`shared/stats/index.js:446-460`), 5 s anti-corpse-camp ward
(`combat/index.js:27,874`), teleport to `scene.metadata.spawnPoints[0]`
(instance entry) if defined, else rise where you fell (`:876-888`). Client
learns of death via `player:stats:update`
(`src/core/player/events/player.js:301-378`, permadeath branch `:357-360`)
and the `combat:hit` `died` flag (`src/core/player/events/monster.js:11`);
there is no dedicated `player:died` event.

### 1.2 By login mode

**Guest (JSON save, token `none`)** — always soft mode unless the save
carries a `chronicles.mortal` identity (`guest-save-store.js:84-86`).
Death: keep everything, respawn in place/at entry, no forced save (120 s
autosave, `server/Delaford.js:38,303`). Fully D-106 compliant already, by
having no loss mechanic at all.

**Local login account (token `local:<uuid>`)** — same soft path; profile in
`login_accounts.profile_json` (`identity-registry.js:107-129`).

**Chronicles-Scion (SQLite, `guestId`/`quickGuest`/`resumeScionId`)** —
`beginScionSession` forces hard mode
(`server/core/services/chronicles.js:156-165`). Final death →
`entombFallenScion` (`chronicles.js:206-244`) →
`ChroniclesRepository.entombScion`
(`server/core/repositories/chronicles-repository.js:467-506`):
`snapshot_json = NULL` (`:479`) — **the entire character (wear, inventory,
bank, currency) is destroyed except notable gear**, which is first copied
into `chronicle_relics` (`collectNotableGear`, `chronicles.js:60-89` —
jewelry / vessel-bearing / brand-or-bond affix items only), each stamped
with `legacy` provenance and `eligible_run = run_count + 3` (`:472,485-496`).
Relics re-enter the world deterministically-ish via a 12 %-per-kill draw
(`RELIC_DROP_CHANCE`, `server/core/combat/loot.js:16,209-223`,
`drawEligibleRelic` `chronicles-repository.js:508-542`), must be picked up
(`items/pickup.js:48-54` → `claimRelic` `:583-591`), and are reset to
`circulating` on server restart (`:88-91`). Death witnesses get cross-house
relic access after 3 own runs (`chronicles.js:230-242`,
`grantHouseRelicAccess` `:544-557`).

**Legacy JSON chronicles-store scion (direct admission with `scionName`)** —
hard mode iff `mortal` (`socket-events/index.js:111-131`). Permadeath →
client-driven `player:chronicles:return` entomb
(`socket-events/index.js:435-508`): exactly ONE relic is selected by
equipment-slot priority (`server/core/services/scion-relics.js:4-14,48-71`)
and removed from the corpse (`:476-482`); it waits `queued` → `circulating`
on elite kill (`chronicles-store.js:447-521`) → `recovered` on pickup
(`actions/index.js:1242`). Everything else is lost indirectly: the fresh
successor's forced save (`socket-events/index.js:377-379`) overwrites the
shared guest save keyed by `player.uuid`.

### 1.3 Disconnect / crash (D-109 surface)

Already compliant. `Delaford.close` (`server/Delaford.js:201-283`) persists
before removing (the "D-109 safety boundary", `:233,238-240`), sets
`disconnecting` so monsters can't target and movement is rejected
(`combat-controller.js:37`, `movement-handler.js:317`), and cancels
pathfinding/auto-attack. Nothing dies or is lost on disconnect; crash is
covered by the 120 s autosave. Return-to-town happens at next login (scion
sessions spawn at the House wagon, `chronicles.js:175-180`; guests at saved
surface position or town fallback, `guest-save-store.js:61-71`).

## 2. Delta table — browser vs D-106/D-109 vs native semantics

Native reference semantics (TASK-0018/0025, `native/src/core.cpp`):
ALL carried items → `house_.relic_candidates`; carried trophies →
`house_.lost_trophies`; seeded 1-in-4 resurface per drop, oldest-first;
surfaced-but-unpicked items return to their pools once on instance
retirement; ordinary floor drops are lost on retirement; pickup gated by the
active instance boundary; successor starts empty (D-004).

| Behavior | Browser today | Native / ruling | Delta |
| --- | --- | --- | --- |
| Soft-death item loss | none (nothing dropped) | D-106: nothing destroyed | compliant |
| Hard-death carried items, SQLite | only "notable gear" becomes relics; `snapshot_json=NULL` destroys the rest incl. bank (`chronicles-repository.js:479`) | every carried item enters the recoverable pool | **GAP — the core D-106 violation** |
| Hard-death carried items, JSON store | exactly 1 relic (`scion-relics.js:48-71`); rest lost via successor overwrite (`socket-events/index.js:377-379`) | every carried item | **GAP** |
| Bank contents of dead scion | die with the snapshot (`chronicles.js:45-58`, `:479`) | D-106 says "everything carried"; bank is arguably house-stored, not carried | needs architect ruling — see questions below |
| Recovery delay | 3-run eligibility (`chronicles-repository.js:472`) + 12 %/kill draw | seeded 1-in-4 per drop, oldest-first | different cadence, same spirit; browser cadence is fine to keep |
| Recovery ordering | oldest eligible first (`:508-542`) | oldest-first | compliant in spirit |
| Surfaced-not-picked-up | ground items have no TTL, but zone teardown (`world.js:476-484`) and restart discard them silently; relics alone are rescued (boot reset `:88-91`, store requeue `chronicles-store.js:235-237`) | surfaced candidates return to pool once on retirement (TASK-0025) | **partial GAP — relic-specific rescue exists, non-relic pool returns don't** |
| Ordinary ground drops on teardown/restart | discarded | lost on retirement | compliant (matches native) |
| Successor start | fresh character, starts empty (`socket-events/index.js:133-142,173`) | starts empty (D-004) | compliant |
| Disconnect/crash loss | none; persist-before-remove (`Delaford.js:233-240`) | D-109 | compliant |
| Trophies | no trophy concept in the browser game | `lost_trophies` pool | n/a — browser has no trophies to protect |
| Client notification | no listener for `chronicles:scion-fallen`/`scion-witnessed` in `src/` (only the playtest harness consumes them) | — | UX gap, orthogonal to D-106 |

## 3. Minimal change-set forecast for the implementation task

Goal: on hard death, everything the scion carried (wear + inventory, and
bank if the architect rules it "carried" — see §4) becomes recoverable
instead of destroyed, in both Chronicles stacks.

**SQLite flow (primary — the account path):**

1. `server/core/services/chronicles.js` — widen `collectNotableGear`
   (`:60-89`) or add a sibling `collectCarriedGear` that stamps ALL carried
   items with `legacy` provenance (reuse `prepareRelic`, `:68-81`).
   Notable-vs-ordinary grading can be preserved by keeping the existing
   filter for naming/renown purposes while pooling everything.
2. `server/core/repositories/chronicles-repository.js` — `entombScion`
   (`:467-506`): insert every carried item into `chronicle_relics` before
   nulling the snapshot. No schema change needed — `item_json` already
   stores full durable snapshots; possibly add a `notable`/`grade` column via
   the existing idempotent ALTER pattern (`:161-170`) if grading matters.
3. `server/core/combat/loot.js:209-223` — the 12 % draw stays; consider
   excluding ordinary items from the rename/"Relic of <scion>" flourish
   (`chronicles.js:261-263`) or rename all — product call.
4. Retirement analog: on `world.destroyZoneScene`/`destroyInstance`
   (`world.js:476-484,532-545`) and on boot, any `dropped` relic already
   resets (`chronicles-repository.js:88-91`); extend the same reset to cover
   the enlarged pool (it does automatically — status-based) and decide
   whether ordinary resurfaced items get the same once-return rule.

**JSON chronicles-store flow (legacy guest path):**

5. `server/core/services/scion-relics.js:48-71` — replace single-relic
   selection with select-all-carried; `chronicles-store.js:447-472` entomb
   stores an array (schema v3 crypt relic is singular — needs a versioned
   sanitize-on-load bump at `chronicles-store.js:224-254`, the de-facto
   migration point).
6. `socket-events/index.js:463-508` entomb and `:369-372`
   `pruneUnrecoveredRelics` must handle N relics (prune already works on UUID
   sets, `scion-relics.js:99-126` — likely trivially generalizes).
7. Alternative worth considering (cheaper): deprecate the JSON-store hard
   mode in favor of the SQLite flow rather than doubling the relic schema —
   the two-stack unification is already open work (root HANDOFF.md "known
   seams"). Flag for the architect.

**Constraining tests/playtests (must be updated deliberately):**

- `playtest/scenarios/mortality.mjs:121-141` — asserts the crypt preserves
  EXACTLY one heirloom and the successor can't duplicate it; becomes
  N-relic assertions.
- `playtest/scenarios/chronicles.mjs:37-63` — relic returns after three
  Set Out runs; unchanged in shape, more items in pool.
- `tests/unit/scion-relics.spec.js:35-87` — selection priority, exact-UUID
  removal, pruning.
- `tests/unit/chronicles-store.spec.js:146-239`, `chronicles-repository.spec.js:55-143`
  — entomb/circulation invariants.
- `tests/unit/death-cancels-walk.spec.js:73` — force-persist ordering.
- `playtest/scenarios/respawn.mjs` — soft-death path untouched; guards
  against regressions while editing the death pipeline.
- E2E has no death assertions (verified) — unit + playtest are the gates.

## 4. Risks and open questions

1. **Bank on death** — D-106 says "everything carried"; the browser bank is
   account-side storage inside the same snapshot that gets nulled
   (`chronicles.js:52`). Is bank "carried"? Recommend: bank is NOT carried
   (it's House-side), but it should transfer to the House/successor rather
   than be destroyed. Needs architect ruling.
2. **Live-save compatibility** — no schema version on guest saves
   (tolerant hydration instead, `inventory-manager.js:18-24`,
   `inventory.js:23-26`); chronicles-store has versioned sanitize-on-load
   (`chronicles-store.js:224-254`); SQLite has idempotent ALTER migrations
   (`chronicles-repository.js:161-170`). Enlarging relic pools is
   backward-compatible (old saves simply have fewer relics); changing the
   JSON-store crypt relic from object to array REQUIRES a sanitize rule.
   Live `chronicle_relics` rows with `status='dropped'` are already rescued
   on boot (`:88-91`).
3. **Economic dilution** — pooling ALL carried items makes relic drops
   mostly mundane loot; the 12 % draw cadence may need a notable-first
   ordering or a separate mundane pool. Product call, flagged not decided.
4. **Duplication windows** — the enlarged pool widens the
   prune-unrecovered surface; `pruneUnrecoveredRelics`
   (`scion-relics.js:118-126`) and the SQLite claim path
   (`chronicles-repository.js:583-591`) are the dup guards to extend and
   test.
5. **Two-stack divergence** — implementing D-106 twice (SQLite + JSON
   store) doubles the migration/testing surface; consider sequencing with
   the known chronicles-stack unification work.

## Verification of this audit

- Read-only: `git status --short` shows only
  `orchestration/tasks/TASK-0031-browser-d106-audit/`.
- Evidence gathered by two independent survey passes over `server/`,
  `src/`, `tests/`, `playtest/`, cross-checked for contradictions (none
  found on the death path; the two relic stacks were independently
  confirmed by both passes).
- Native semantics verified against `native/src/core.cpp` at base and the
  TASK-0018/0025 reports.
