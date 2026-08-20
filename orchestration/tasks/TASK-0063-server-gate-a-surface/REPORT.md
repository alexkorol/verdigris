---
task: TASK-0063
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0063-server-gate-a-surface-cursor
base_commit: 5c41a04821695e38261fdb52f2e86b2dea67f21d
architect_review_required: true
---

# TASK-0063 REPORT — Server-side Gate A protocol surface

## Executive summary

The four 0061 server gaps are first-class envelopes on `ProtocolSession`.
Kill loot and floor treasure emit the JS ground-item pair (`item:change` +
`world:itemDropped`). `player:extract` and walking the depth-1 stairs-up
converge on the same House bank + surface return. Successful `item:equip`
answers with wear-slot state and derived combat totals. Login and
`dev:state` both carry the current ground list.

N1–N4 attach against this build is **13/13 PASS**, harness unchanged.
`native/client/**` was not touched (0064 owns it).

## Approach (JS names, checked)

Grep of `server/` + `src/core/player/events/`:

| Gap | JS name | Native emit |
|---|---|---|
| Ground drops | `item:change` and `world:itemDropped` (both; client reads `data.data`) | same pair; `data` wrapped as `{data: [...]}` because native `parse_envelope` rejects array payloads |
| Extract | **no JS `player:extract`** | SPEC-named inbound `player:extract`; response reuses that name as the bank summary |
| Equip totals | `player:equippedAnItem` (public projection `wear`) | same event, additive `wearDetails` + `combat` |
| Equip miss | `game:send:message` "That item is no longer in your inventory." | mirrored; inventory unchanged |
| Login ground | `droppedItems` on the login block and on `scene` | `login.droppedItems`, `scene.droppedItems`, `dev:state.groundItems` |

`item:ground` was not used. House-bank summary has no JS event; the
response is `player:extract` `{items, trophies, storedItems, storedTrophies}`
plus a `game:send:message` line. Trophy circulation remains an N5 stub
(`trophies: 0`).

Stairs-up at depth 1 and `player:extract` both call
`WorldSimulation::return_to_surface()` then `finish_extraction()` (drain
backpack + wear into `house_store_`). Inter-floor stairs-up (depth > 1)
does not bank.

## Changed files

- `native/include/verdigris/core.hpp` — public `WorldSimulation::return_to_surface()`.
- `native/include/verdigris/networking.hpp` — extract/ground/equip helpers, `house_store_`.
- `native/src/core.cpp` — `return_to_surface()`.
- `native/src/networking.cpp` — envelopes listed above.
- `native/tests/networking_tests.cpp` — coverage for each envelope + unknown-uuid negative.

## Verification

1. `powershell -File native/build.ps1 -RunTests` (2026-08-20 04:03 PDT) —
   denylist / core / networking / camera2d / session tests PASS.
   Literal transcript: `captures/build-runtests.log`.
2. Attach, cursor port **6587**, harness **unchanged** (no `playtest/**`
   edits; last `playtest/` commit on this tip is `68af057e`, 0062
   diagnostics already on program base `5c41a048`):

```
$env:PLAYTEST_WS_URL='ws://127.0.0.1:6587'
node playtest/run.mjs --attach quickstart single-session movement zones combat encounter-variety boss-mechanic loot equipment-slots depth-loot overflow vesselforge vesselforge-brand
```

**13/13 PASS.** Literal transcript: `captures/attach-n1-n4.log`.
Server stopped after the run.

3. Authentic negative (C++): `item:equip` of uuid `missing-uuid` emits
   `game:send:message` "That item is no longer in your inventory.",
   refreshes inventory, and leaves wear + backpack unchanged
   (`test_gate_a_equip_totals_and_unknown_uuid`).

New networking coverage also asserts:

- floor treasure and kill-loot `item:change` carry uuid, id, name, x, y
- login + `dev:state` ground lists match after instance entry
- `player:extract` banks `garnet-amulet` and returns to town
- stairs-up emits the same `player:extract` summary and banks `bronze-sword`
- successful equip emits `player:equippedAnItem` with `wear.necklace` and
  raised `combat.attack.stab`

## Deviations / follow-ups

- Native `item:change` / `world:itemDropped` wrap the array at `data.data`
  (JS Socket.broadcast sends the array as `data` directly). Required for
  `parse_envelope`; JS client handlers already read `data.data`.
- Protocol House store lives on `ProtocolSession::house_store_`, not
  `Simulation::house()` (that API is const from this layer). Snapshot field
  `houseStoredItems` is additive.
- `player:extract` from anywhere in an instance returns to town (does not
  require standing on the stairs). Stairs-up at depth 1 still works.
- 0064 remote client still infers monster/loot positions until it consumes
  these envelopes; this task did not edit `native/client/**`.
- Architect should extend the 0061 drive script for live drop labels and
  rescore Gate A.

## Risks

Banking on stairs-up is a behavior change vs 0061 ("returning to town keeps
session inventory"). N1–N4 attach stays green because those scenarios do
not inspect backpack contents after a surface return. A later scenario that
dives, walks stairs, then expects starting coins in the backpack would
need the House store instead.
