---
task: TASK-0055
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0055-browser-followups-cursor
base_commit: f71815f351ea9da9ebd6459812b8a25d87a323a5
architect_review_required: true
---

# TASK-0055 REPORT — Identity chip + server-side zone preview payload

## Executive summary

Two 0049 review debts, closed without a client-side data mirror.

1. **Identity chip**: long House/Scion labels no longer run into the HP orb.
   The chip is parked above the orb (`bottom: orb-size + 28px`), width-capped
   at `min(28vw, 240px)` with ellipsis, and the full string is on `title` so
   hover shows the untruncated name. Verified at 1366×768 and 1920×1080 with
   20-char generated names.
2. **Zone preview payload**: `player:login` and `party:update` now carry an
   additive `adventureZones` array. Boss display name, guaranteed treasure
   item-level, and delve depth come from `THEME_MONSTERS` +
   `instanceItemLevelForDepth` + solo-delve depth 1 (the same `enterFloor(1)`
   startSoloInstance already uses). `src/core/adventure-objective-data.js` is
   **deleted**. `adventure-objectives.js` reads the payload only.

## Changed files

- `server/core/party.js` — new preview builder (`zonePreviewFields`,
  `adventureZonePayload`).
- `server/player/handlers/party.js` — attaches `adventureZones` on login
  (Authentication.addPlayer wrap, restored after emit) and every
  `party:update`.
- `src/core/adventure-objectives.js` — ingest from the WS envelope; no
  mirror tables.
- `src/core/adventure-objective-data.js` — **deleted**.
- `src/components/layout/GameHUD.vue` — chip position, width cap, title,
  hover hit-testing.
- `src/components/layout/GameContainer.vue` — existing `zoneObjective(zone).line`
  wiring made a computed so the panel updates when the payload arrives
  (see scope note).
- `tests/unit/adventure-objectives.spec.js`, `adventure-zone-payload.spec.js`,
  `identity-chip.spec.js`, `party-system.spec.js`, `party-descent.spec.js`
  (map mock exports for THEME_MONSTERS).
- `orchestration/tasks/TASK-0055-browser-followups/captures/` — hard-fail
  Playwright script, 4 PNGs, evidence JSON.

No `playtest/**` assertion changes. No `native/**` changes.

## Protocol

Additive fields only. Existing login/party keys are unchanged; old clients
ignore `adventureZones`. Existing zone `id` / `name` / `template` / `layout`
/ `levelHint` are preserved on the new array entries.

## Test commands and outcomes

`npm run test:unit`:

```
 Test Files  133 passed (133)
      Tests  838 passed (838)
```

`npm run playtest` (PLAYTEST_PORT=6580, bind 127.0.0.1):

```
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":32.227327,"maxEventLoopLagMs":132.317183}
```

Browser smoke (build + Playwright critical loop; server on 127.0.0.1:6581,
never 6500 — cursor capsule):

```
  ok 1 tests\e2e\browser-critical-loop.spec.mjs:135:1 › the built game supports the browser-critical guest loop (18.5s)
  1 passed (21.4s)
```

Capture (server on 127.0.0.1:6582):

```
CAPTURES OK {"1366x768.houseIdentity":true,"1366x768.titleMatchesFullName":true,"1366x768.chipDoesNotOverlapHpOrb":true,"1366x768.serverPayloadPresent":true,"1366x768.adventureMatchesServer":true,"1920x1080.houseIdentity":true,"1920x1080.titleMatchesFullName":true,"1920x1080.chipDoesNotOverlapHpOrb":true,"1920x1080.serverPayloadPresent":true,"1920x1080.adventureMatchesServer":true}
```

PNGs: `01-1366x768-identity-chip.png`, `01-1366x768-adventure-preview.png`,
`02-1920x1080-identity-chip.png`, `02-1920x1080-adventure-preview.png`.
Evidence JSON greps the live `adventureZones` payload; The Old Barrow row
must equal `Warden of the Deep · item-level 10 gear · depth 1`.

## Authentic negative

`zoneObjective({ template: 'dungeon' })` with an empty ingest cache returns
`warden: null` and `line: ''` — it does not invent "Warden of the Deep" from
a client table. Restored ingest → the unit tests go green again.

## Scope note (GameContainer.vue)

The SPEC owned_paths list `adventure-objectives.js` as the client reader and
do not name `GameContainer.vue`. The Adventure panel is the only UI that
renders `zoneObjective(zone).line`. A one-time `const` map ran before login
payload ingest, so the panel stayed blank (first capture run: payload present
on the wire, objective DOM empty). The catalog itself is unchanged; it is
now a `computed` keyed off the ingest tick so it re-reads the **server**
payload. Flagging because it is a wiring file just outside the listed
owned_paths.

## Commits

- `2ec96b77` — claim (STATUS.md)
- implementation + captures + REVIEW_REQUESTED — this commit
