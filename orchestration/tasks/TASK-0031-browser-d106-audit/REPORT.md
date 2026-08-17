---
task: TASK-0031
state: REVIEW_REQUESTED
track: research
base_commit: 8e4a977c104686781b03f6ed5f22cfa8f3b6b32d
branch: codex/TASK-0031-browser-d106-audit
scope: read-only browser death/relic audit
---

# TASK-0031 — browser death/relic audit

## Executive summary

The browser has two materially different death paths. A direct guest or
authenticated non-Chronicles player is soft-mode: lethal damage moves the
player to `awaiting-respawn`; no death handler removes wear, inventory, or
embedded trophy data, and the respawn code moves the same player back to an
instance entry ([`server/shared/stats/index.js:301-352`](../../../../server/shared/stats/index.js#L301-L352), [`server/core/combat/index.js:852-893`](../../../../server/core/combat/index.js#L852-L893)). A mortal Chronicles Scion is hard-mode: the first lethal hit marks it `permadead`, and the server commits only filtered “notable” gear as a Chronicle relic ([`server/player/handlers/socket-events/index.js:111-127`](../../../../server/player/handlers/socket-events/index.js#L111-L127), [`server/core/entities/monster/combat-controller.js:331-369`](../../../../server/core/entities/monster/combat-controller.js#L331-L369), [`server/core/services/chronicles.js:60-89`](../../../../server/core/services/chronicles.js#L60-L89)).

For hard death, ordinary weapons/armor, stackables, and any other inventory or
wear entries failing the notable predicate are not recoverable. The selected
relic is removed only when the socket returns to Chronicles; then the dead
player is removed from the world and a successor is created from the fresh
Scion profile ([`server/player/handlers/socket-events/index.js:435-507`](../../../../server/player/handlers/socket-events/index.js#L435-L507), [`server/core/entities/player/fresh-scion-profile.js:21-46`](../../../../server/core/entities/player/fresh-scion-profile.js#L21-L46)). Thus the remaining carried state disappears with the dead in-memory player. D-106 requires every carried item and trophy to remain recoverable, so the hard path is a direct product violation.

There is no browser `relic_candidates`/`lost_trophies` equivalent. Browser relics are individual crypt rows with a run-delay and kill-based circulation; the JSON Chronicle path and SQLite Chronicle path coexist and are wired through different services ([`server/core/repositories/chronicles-repository.js:467-505`](../../../../server/core/repositories/chronicles-repository.js#L467-L505), [`server/core/services/chronicles-store.js:423-487`](../../../../server/core/services/chronicles-store.js#L423-L487), [`server/player/handlers/socket-events/index.js:5-30`](../../../../server/player/handlers/socket-events/index.js#L5-L30)). Standalone Vesselforge trophy fragments live in a separate engine character object, not the browser `Player` snapshot ([`server/core/items/vesselforge/engine.js:443-452`](../../../../server/core/items/vesselforge/engine.js#L443-L452), [`server/core/items/vesselforge/engine.js:478-490`](../../../../server/core/items/vesselforge/engine.js#L478-L490)); socketed trophies are merely fields inside an item clone and are preserved only when that item itself survives ([`server/core/items/vesselforge/engine.js:175-194`](../../../../server/core/items/vesselforge/engine.js#L175-L194), [`server/core/repositories/guest-save-store.js:25-42`](../../../../server/core/repositories/guest-save-store.js#L25-L42)).

The native comparison below uses the accepted implementation reports
[`TASK-0018 REPORT`](../TASK-0018-death-recoverability/REPORT.md) and
[`TASK-0025 REPORT`](../TASK-0025-instance-lifecycle-correctness/REPORT.md),
plus binding D-106/D-109 in `orchestration/DECISIONS.md`: all carried values
enter ordered `relic_candidates`/`lost_trophies` pools; recovery is deterministic
and delayed; successors start empty; extraction remains the only durable-store
path; and instance retirement requeues an unclaimed surfaced candidate once
while ordinary floor leftovers are lost.

## Evidence walkthrough

### Admission and lifecycle mode

* Direct account login constructs a `Player` from the authentication profile;
  direct guest login overlays a file snapshot over the guest template
  ([`server/player/handlers/socket-events/index.js:281-305`](../../../../server/player/handlers/socket-events/index.js#L281-L305)). Both are soft unless a Chronicles identity is selected.
* `resolveScionIdentity`/`applyScionIdentity` attach `houseId`, `scionId`, and
  `mortal`; the latter maps `mortal: true` to lifecycle mode `hard` and all
  other Scions to `soft` ([`server/player/handlers/socket-events/index.js:111-175`](../../../../server/player/handlers/socket-events/index.js#L111-L175)).
* Chronicle-auth sessions use the separate SQLite-backed `beginScionSession`
  path and restore a saved snapshot into town ([`server/player/handlers/socket-events/index.js:236-279`](../../../../server/player/handlers/socket-events/index.js#L236-L279), [`server/core/services/chronicles.js:140-194`](../../../../server/core/services/chronicles.js#L140-L194)). This is important for migration: the repository and the JSON store are not interchangeable records.

### Soft guest/account death

* `markDeath` decrements the lifecycle counter and allows respawn whenever the
  mode is not hard (or extra lives remain); it sets `awaiting-respawn` and a
  timer instead of clearing player state ([`server/shared/stats/index.js:301-352`](../../../../server/shared/stats/index.js#L301-L352)).
* The periodic server path calls `tryRespawn`, restores the instance spawn
  point, grants five-second protection, and broadcasts the same player's stats
  ([`server/core/combat/index.js:852-893`](../../../../server/core/combat/index.js#L852-L893)). There is no item or trophy mutation in either path.
* Guest snapshots include complete durable wear, inventory, bank, lifecycle,
  and resources; account/Chronicles living snapshots include the same wear and
  inventory fields ([`server/core/repositories/guest-save-store.js:45-87`](../../../../server/core/repositories/guest-save-store.js#L45-L87), [`server/core/services/chronicles.js:45-58`](../../../../server/core/services/chronicles.js#L45-L58)).

### Hard mortal Chronicles death

* Monster damage always asks for cheat-death, but when the result is
  `permadeath` the combat controller invokes `entombFallenScion`
  ([`server/core/entities/monster/combat-controller.js:331-369`](../../../../server/core/entities/monster/combat-controller.js#L331-L369)). The hard-death save is also requested immediately in the player stats manager ([`server/core/entities/player/stats-manager.js:238-261`](../../../../server/core/entities/player/stats-manager.js#L238-L261)); this creates an ordering/migration risk noted below.
* `collectNotableGear` scans both inventory and worn values, but rejects
  stackables and accepts only jewelry, vessels, or items with brand/bond
  affixes ([`server/core/services/chronicles.js:60-89`](../../../../server/core/services/chronicles.js#L60-L89)). It does not enumerate a separate trophy collection, a lost-trophy pool, or all carried entries.
* The SQLite entomb transaction marks the Scion dead, nulls its snapshot, and
  inserts only the deduplicated filtered relic list as `circulating` rows with
  `eligible_run = current_run + 3` ([`server/core/repositories/chronicles-repository.js:467-505`](../../../../server/core/repositories/chronicles-repository.js#L467-L505)).
* On `player:chronicles:return`, the JSON Chronicle handler chooses at most one
  eligible item (wear priority, then first eligible inventory item), removes
  that UUID from the live player, removes the player from the world, and parks
  the socket in the Chronicle screen ([`server/core/services/scion-relics.js:43-70`](../../../../server/core/services/scion-relics.js#L43-L70), [`server/player/handlers/socket-events/index.js:435-507`](../../../../server/player/handlers/socket-events/index.js#L435-L507)). The remaining carried state is not transferred to a recovery pool.
* Client death handling mirrors this: it entombs the local cached House, emits
  `player:chronicles:return`, and refuses to continue building the game when
  lifecycle is `permadead` ([`src/Delaford.vue:691-734`](../../../../src/Delaford.vue#L691-L734), [`src/Delaford.vue:1715-1731`](../../../../src/Delaford.vue#L1715-L1731)).
* A fresh successor starts with the starter wear/inventory and no inherited
  carried state ([`server/core/entities/player/fresh-scion-profile.js:21-46`](../../../../server/core/entities/player/fresh-scion-profile.js#L21-L46)). This correctly prevents duplicate inheritance, but does not make the dead Scion's other carried values recoverable.

### Relics, trophies, and instance boundaries

* SQLite relic re-entry is not native-style FIFO pool resurface. `drawEligibleRelic`
  selects the oldest eligible circulating row, and the loot path exposes a
  separate 12% circulation chance plus an elite-only legacy-store branch
  ([`server/core/repositories/chronicles-repository.js:508-541`](../../../../server/core/repositories/chronicles-repository.js#L508-L541), [`server/core/combat/loot.js:13-23`](../../../../server/core/combat/loot.js#L13-L23), [`server/core/combat/loot.js:170-223`](../../../../server/core/combat/loot.js#L170-L223)).
* On pickup, the browser claims a Chronicle relic only after saving the player,
  then marks the row recovered ([`server/player/handlers/actions/index.js:1228-1253`](../../../../server/player/handlers/actions/index.js#L1228-L1253)).
* Vesselforge generated items have an embedded `trophies: []` field, while
  venture trophy fragments increment `character.fragments`; neither is a
  `Player`-level carried-trophy vector ([`server/core/items/vesselforge/engine.js:175-194`](../../../../server/core/items/vesselforge/engine.js#L175-L194), [`server/core/items/vesselforge/engine.js:443-452`](../../../../server/core/items/vesselforge/engine.js#L443-L452), [`server/core/items/vesselforge/engine.js:478-490`](../../../../server/core/items/vesselforge/engine.js#L478-L490)). An embedded trophy survives a normal durable item clone because the clone removes placement/session fields only ([`server/core/repositories/guest-save-store.js:25-42`](../../../../server/core/repositories/guest-save-store.js#L25-L42)); it is lost with an unselected item on hard death.
* Instance teardown deletes the instance scene and its floor arrays without
  requeueing a surfaced candidate ([`server/core/world.js:532-545`](../../../../server/core/world.js#L532-L545)). Party teardown and route changes call that destruction ([`server/player/handlers/party.js:175-180`](../../../../server/player/handlers/party.js#L175-L180), [`server/player/handlers/party.js:342-349`](../../../../server/player/handlers/party.js#L342-L349), [`server/player/handlers/party.js:431-438`](../../../../server/player/handlers/party.js#L431-L438)). A server restart happens to reset SQLite rows in `dropped` state to `circulating`, but ordinary instance retirement has no equivalent recovery ([`server/core/repositories/chronicles-repository.js:87-90`](../../../../server/core/repositories/chronicles-repository.js#L87-L90)).

### D-109 disconnect path

* Disconnect/logout marks the player `disconnecting`, stops combat/pathing,
  awaits `player.update()`, and only then removes the player from the world
  ([`server/Delaford.js:201-240`](../../../../server/Delaford.js#L201-L240)). Snapshot builders move an instance player to its saved pre-instance surface position while retaining wear/inventory ([`server/core/repositories/guest-save-store.js:59-87`](../../../../server/core/repositories/guest-save-store.js#L59-L87), [`server/core/services/chronicles.js:33-58`](../../../../server/core/services/chronicles.js#L33-L58)). This is substantially aligned with D-109 for successful saves.
* Save failures are logged and teardown continues ([`server/Delaford.js:230-240`](../../../../server/Delaford.js#L230-L240)); guest file failures similarly return `null` after warning ([`server/core/repositories/guest-save-store.js:89-103`](../../../../server/core/repositories/guest-save-store.js#L89-L103)). A failed persistence operation can therefore still lose live progress, which is a D-109 live-compatibility risk rather than a death-rule match.

## Delta table

| Required semantic | Native reference (TASK-0018/TASK-0025 and D-106/D-109) | Current browser evidence | Delta / implementation consequence |
|---|---|---|---|
| Death scope | Every equipped item, pack item, and carried trophy becomes recoverable; items go to `House::relic_candidates`, trophies to `House::lost_trophies`; successor starts empty. See TASK-0018 report and D-106. | Soft mode keeps all values immediately through respawn; hard mode filters through `collectNotableGear` and `selectScionRelic` ([`server/core/services/chronicles.js:60-89`](../../../../server/core/services/chronicles.js#L60-L89), [`server/core/services/scion-relics.js:43-70`](../../../../server/core/services/scion-relics.js#L43-L70)). | Hard death destroys/losses ordinary carried values and has no trophy pool. Implement one authoritative death transfer before removing the player. |
| Relic resurface | Native recovery candidates resurface oldest-first on deterministic seeded 1-in-4 cadence; recovered floor candidates remain recoverable through retirement. See TASK-0018/0025 reports. | Browser SQLite rows wait `run_count + 3`, then `drawEligibleRelic` oldest-first behind kill circulation; another JSON store uses `status: queued/circulating/recovered` ([`server/core/repositories/chronicles-repository.js:467-541`](../../../../server/core/repositories/chronicles-repository.js#L467-L541), [`server/core/services/chronicles-store.js:490-565`](../../../../server/core/services/chronicles-store.js#L490-L565)). | Cadence, storage, and event semantics differ; choose the product-authoritative pool and migrate both stores without duplicate UUIDs. |
| Trophies | Native has a separate ordered `lost_trophies` pool and dedicated resurface event. | Browser has no Player-level trophy vector; Vesselforge fragments are engine-local and socketed trophies are nested in items ([`server/core/items/vesselforge/engine.js:443-490`](../../../../server/core/items/vesselforge/engine.js#L443-L490)). | Add a durable trophy representation and death/re-entry/claim paths; explicitly decide whether existing socketed trophies remain with their recovered item. |
| Instance retirement boundary | TASK-0025 retires ground state, rejects stale pickups, requeues an unclaimed surfaced relic/trophy exactly once, and loses ordinary leftovers. | `destroyInstance` drops scene arrays wholesale; only restart resets SQLite `dropped` rows ([`server/core/world.js:532-545`](../../../../server/core/world.js#L532-L545), [`server/core/repositories/chronicles-repository.js:87-90`](../../../../server/core/repositories/chronicles-repository.js#L87-L90)). | Add an explicit browser retirement routine with active-instance membership and candidate requeue before destruction. |
| Logout/disconnect | D-109: no item/progress loss; disconnect must be a safe pull-out, never a death. | Successful close saves before removal, but failed saves are swallowed and removal continues ([`server/Delaford.js:201-240`](../../../../server/Delaford.js#L201-L240)). | Preserve current ordering, but make failure a retry/blocking boundary or durable queue; test disconnect during combat and instance teardown. |
| Chronicle ownership | Native House owns recovery pools; successor does not inherit the dead Scion's full inventory. | Browser has JSON `chroniclesStore` and SQLite `chroniclesRepository` paths in the same handler/loot surface ([`server/player/handlers/socket-events/index.js:5-30`](../../../../server/player/handlers/socket-events/index.js#L5-L30), [`server/core/combat/loot.js:1-5`](../../../../server/core/combat/loot.js#L1-L5)). | Migration must converge authority or define an adapter; otherwise a relic can be committed in one store and surfaced/claimed in another. |

## Follow-up implementation change-set forecast

This audit predicts the following minimum implementation task. It intentionally
does not edit these paths.

1. **Authoritative death transfer:** extend the server Chronicle/relic service
   and repository (`server/core/services/chronicles.js`,
   `server/core/services/chronicles-store.js`,
   `server/core/repositories/chronicles-repository.js`) with versioned
   recoverable item/trophy records. At the hard-death seam in
   `server/core/entities/monster/combat-controller.js`, transfer every
   `wear` value, every `inventory.slots` value, and the chosen durable trophy
   representation exactly once before the player is removed. Keep successor
   construction in `fresh-scion-profile.js` empty except for the intentional
   starter contract.
2. **Resurface and claim:** refactor `server/core/combat/loot.js` to use one
   seeded cadence and oldest-first pool draw; add item/trophy world metadata,
   claim/recovery status transitions, and an explicit requeue hook. Update
   `server/player/handlers/actions/index.js` so item and trophy claims are
   atomic with persistence rather than “save then claim” races.
3. **Lifecycle boundary:** add a browser instance retirement helper near
   `server/core/world.js`/`server/player/handlers/party.js`. It must clear
   ordinary floor leftovers, return surfaced unclaimed item/trophy candidates
   once, and reject stale IDs from inactive scenes, matching TASK-0025.
4. **Persistence migration:** version guest snapshots and Chronicle rows in
   `server/core/repositories/guest-save-store.js`, the SQLite migration in
   `chronicles-repository.js`, and any JSON store adapter. Read old snapshots
   with no pools as empty; deduplicate by UUID; make already-dead/queued/
   circulating/recovered records idempotent across reconnect and restart.
5. **D-109 failure handling:** retain the disconnect save-before-remove order in
   `server/Delaford.js`, but add a durable retry/ack boundary for save failure
   and a test that a disconnect cannot create a death or discard carried state.
6. **Verification:** add focused unit coverage for all carried categories,
   trophies, FIFO/deterministic re-entry, instance retirement/requeue,
   duplicate death/reconnect, old-save migration, and save failure. Extend
   `playtest/scenarios/respawn.mjs`, `session-arc.mjs`, `chronicles.mjs`, and
   `mortality.mjs` (or add a dedicated death-recovery scenario) to assert the
   live protocol, not only helper functions.

## Existing playtest constraints

* `respawn` establishes the soft contract: one death, stable pending timer,
  instance-entry spawn, five-second ward, and action consumption
  ([`playtest/scenarios/respawn.mjs:1-59`](../../../../playtest/scenarios/respawn.mjs#L1-L59)). Any death-transfer change must leave this scenario's inventory-preserving respawn behavior intact.
* `session-arc` persists a looted/equipped weapon across relog, enters several
  zones, forces one final death, and later recovers the developed weapon as an
  heirloom ([`playtest/scenarios/session-arc.mjs:158-180`](../../../../playtest/scenarios/session-arc.mjs#L158-L180), [`playtest/scenarios/session-arc.mjs:256-304`](../../../../playtest/scenarios/session-arc.mjs#L256-L304)). The new behavior must broaden recovery without breaking this exact identity/history assertion.
* `chronicles` deliberately waits through later runs before releasing a relic
  and checks origin history ([`playtest/scenarios/chronicles.mjs:37-64`](../../../../playtest/scenarios/chronicles.mjs#L37-L64)). Its run-delay expectations must be intentionally revised if D-106's cadence replaces the current rule.
* `mortality` tests a hard mortal Scion, a disconnect before client
  acknowledgement, crypt entry, an empty successor, and eventual exact-item
  recovery ([`playtest/scenarios/mortality.mjs:89-142`](../../../../playtest/scenarios/mortality.mjs#L89-L142), [`playtest/scenarios/mortality.mjs:177-205`](../../../../playtest/scenarios/mortality.mjs#L177-L205)). It is the primary regression harness for the hard path.
* `zones` enters every menu zone and returns via stairs to the pre-entry tile
  ([`playtest/scenarios/zones.mjs:1-45`](../../../../playtest/scenarios/zones.mjs#L1-L45)). Add retirement/requeue assertions at the same zone boundary; keep the server pinned to port 6500 for browser gates and run the real `npm run playtest` before claiming browser gameplay.

## Risks, compatibility, and unresolved questions

* **Dual Chronicle authority:** SQLite `chroniclesRepository` is used by the
  newer service while `chroniclesStore` still owns the socket return and one
  relic-drop branch. A migration that updates only one can strand or duplicate
  relics; the follow-up must define one source of truth or an explicit bridge
  ([`server/core/services/chronicles.js:206-218`](../../../../server/core/services/chronicles.js#L206-L218), [`server/player/handlers/socket-events/index.js:462-480`](../../../../server/player/handlers/socket-events/index.js#L462-L480)).
* **Death/save race:** hard death requests an immediate living snapshot before
  the combat controller entombs the Scion ([`server/core/entities/player/stats-manager.js:253-259`](../../../../server/core/entities/player/stats-manager.js#L253-L259), [`server/core/entities/monster/combat-controller.js:363-369`](../../../../server/core/entities/monster/combat-controller.js#L363-L369)). The new transfer must be transactional and idempotent, not a second asynchronous snapshot.
* **Legacy data:** guest JSON snapshots have no recovery pools, and existing
  SQLite/JSON rows may already be `queued`, `circulating`, `dropped`, or
  `recovered`. Migration must preserve UUIDs and provenance; startup currently
  changes `dropped` rows only as a crash repair ([`server/core/repositories/chronicles-repository.js:87-90`](../../../../server/core/repositories/chronicles-repository.js#L87-L90)).
* **Live players:** disconnect save errors currently do not stop removal
  ([`server/Delaford.js:230-240`](../../../../server/Delaford.js#L230-L240)). Changing that boundary can affect socket replacement and party cleanup; test reconnect, explicit logout, abrupt close, and duplicate close.
* **Trophy product question:** should a trophy socketed into an item remain
  attached to that item when the item resurfaces, and how should standalone
  Vesselforge fragments be represented in the House pool? The constitution and
  D-106 require recoverability but do not settle this data-shape detail.
* **Cadence product question:** D-106/TASK-0018 names the native deterministic
  1-in-4 cadence, while the browser currently uses run-delay and kill chance.
  Owner/architect confirmation is needed before changing live Chronicle
  timing, especially for already-circulating relics.

## Verification and read-only proof

No browser, server, native, orchestration-status, or test files were changed.
The only intended file is this report. No acceptance command was specified by
the task (`acceptance_commands: []`); no server/watch process was started.

Read-only preflight and final proof:

```text
git status --short                         # clean before audit
git remote -v                              # origin configured
git fetch --prune origin                   # completed
git status -sb                             # codex/TASK-0031-browser-d106-audit
git rev-parse HEAD                         # 8e4a977c104686781b03f6ed5f22cfa8f3b6b32d
git rev-list --left-right --count 'HEAD...@{upstream}'
                                           # no upstream configured for this worker branch
git status --short                         # after audit: report only before commit
```

The worker branch was clean and had no configured upstream; this is a branch
configuration fact, not a code divergence. The report was then committed as
the sole worktree change.

## Commit

Committed on `codex/TASK-0031-browser-d106-audit`; the final commit SHA is
provided in the worker handoff.
