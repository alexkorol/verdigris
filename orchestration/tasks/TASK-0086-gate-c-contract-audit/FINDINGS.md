# FINDINGS — TASK-0086 Gate C campaign-decision contract audit

Worker: `ox-pc-c` · Branch: `codex/TASK-0086-gate-c-contract-audit-ox-pc-c` ·
Immutable code base: `42718fbc4340589e606fff94a6eaa3dfbd03ad1c` (audited at
coordination head `2ed4799a`, which touches orchestration files only).
Machine-readable companion: `captures/gate-c-contract.json`.

## Contract under audit

`docs/rebuild/NATIVE_PRODUCT_CONVERGENCE.md:68-72` — Gate C requires the
route choice to be made on concrete info: **goal, boss/danger, expected
trophy/material/item family, depth, branch consequence,
extraction/return condition**. "A route name alone fails." Per SPEC stop
conditions: evidence absent ⇒ MISSING; no campaign/reward/economy/balance/risk
value was invented anywhere in this audit.

## Surfaces audited

- Browser authoritative server: `server/player/handlers/world-web.js`,
  `server/core/world-web.js`, `server/core/services/zone-service.js`,
  `server/core/first-goal.js`.
- Native N6 server: `native/src/networking.cpp`
  (`emit_chart_screen` :1410-1443, `enter_road_node` :1445-1475,
  gate sweep :1519-1535, extraction :939-968/:2116-2124,
  event dispatch :2314-2320/:2451).
- Client consumer of the chart: `src/components/game-panes/Chart.vue`,
  `src/Delaford.vue:1575`.
- Tests: `native/tests/networking_tests.cpp`,
  `playtest/scenarios/world-web.mjs`, `playtest/scenarios/quest.mjs`.

## Field-by-field verdicts

### 1. Concrete goal — **MISSING**

Chart payloads carry no goal/objective key on either server
(`server/core/world-web.js:203-216,254-269`; native rows
`native/src/networking.cpp:1424-1437`). The only route-adjacent goal loop is
the route-agnostic first goal — Aldwyn's "put its Warden down, and come back
to me" for *any* tier-1 stretch (`server/core/first-goal.js:48-55`;
native mirror :1554-1558) — which assigns no per-road or per-node purpose.
Road `blurb`s (`world-web.js:19,31,43,55`) are terrain flavor. The Warden-clear
micro-objective is derivable from the unlock rule, but a Gate C "concrete
goal" for choosing a specific route does not exist; inventing one is owner
campaign authority.

Smallest future owner path: add a per-node/per-road goal label in the two
chart builders (`server/core/world-web.js` `buildChart`;
`native/src/networking.cpp` `emit_chart_screen`), consumed by
`src/components/game-panes/Chart.vue` and the future native chart pane.
Content is owner product authority.

### 2. Boss/danger — **AVAILABLE**

Every node generates `wardenName = "Warden of <node>"`
(`world-web.js:215`; native :713, emitted at :1429); zone entry pins it as the
floor boss via `set_boss_name_override` (:1462) and the browser generator
renames its elite accordingly (`zone-service.js:146-152`). Difficulty band:
`levelHintForTier` (`world-web.js:169-172,:212`), painted on in-zone gates
(`zone-service.js:186`) and rendered as `Lv <hint>` (`Chart.vue:32`). Danger
flavor per road via direction+blurb (`world-web.js:14-63`). Tests:
`playtest/scenarios/world-web.mjs:39-41` ("the ${wardenName} keeps the
ground"); `native/tests/networking_tests.cpp:201,212` ("N3 names the Old
Barrow boss", telegraph). Exact pack composition is visible only after entry.

### 3. Expected trophy/material/item family — **MISSING**

No chart/node key describes expected drops, trophies, materials, or item
families on either surface; the only drop guarantees live inside the
commission chain and are discovered in-scene, not pre-announced per route
(`playtest/scenarios/quest.mjs:175-178`). Native drop pipeline comment even
retires the legacy synthetic trophy event (`networking.cpp:1996-1997`).
STOP condition honored: nothing invented.

Smallest future owner path: an owner-owned loot-table/family source keyed by
road/template/tier surfaced through both chart builders. Candidate future
owners per RUN_STATUS routing (routing suggestions, not claims): TASK-0104
(itemization/history audit), TASK-0103 (monster/encounter gap audit).

### 4. Depth — **AVAILABLE**

`tier` on every node row (`world-web.js:208`; native :1426); instances
generate at `depth = node.tier` (`zone-service.js:139`); delve triggers carry
`metadata().depth` (native :1474,:2314-2317);
`sceneMetadata.tier === 2` asserted (`world-web.mjs:76`);
`sceneMetadata.depth === 2` asserted after stairsDown descent
(`quest.mjs:255-264`); stairs coordinates on every snapshot
(native :784-785; test label "both stairs exist",
`networking_tests.cpp:143-144`).

### 5. Branch consequence — **DERIVABLE-WITHOUT-GAMEPLAY-RULES** (immediate stage only)

The unlock rule and statuses (`cleared/open/barred`) are fully on the wire
(`world-web.js:254-261,272-277`; native :1414-1432), parent/child links exist
(`parentId`/`childIds`, `world-web.js:213-217`), onward gates name their child
node + levelHint (`zone-service.js:164-189`), and the refusal message states
the rule verbatim while a Warden lives (`zone-service.js:351-358`; native
:1527). A client can compute "clearing X opens Y and Z" with no new gameplay
rules. Bounded gaps recorded honestly: the frontier caps visibility at deepest
cleared tier + 1 (`world-web.js:246-252`; native :1414-1418), so multi-stage
or terminal consequences are absent (owner ruling required to change); and
C++ rows omit `childIds` (parity P1 below).

### 6. Extraction/return condition — **AVAILABLE**

Entry waymark (stairsUp) returns to the Crossroads
(`zone-service.js:212,:288-325,:341-345`; native pins
`stairs_up_returns_to_town` :1469 and routes stairs-up through
`finish_extraction` → `player:extract` bank summary :939-968,:2116-2124).
Persistence window communicated in the chart footnote ("a quarter hour… then
the green closes over your footprints", `Chart.vue:40-43`) and implemented via
`ZONE_LINGER_MS` default 15 min with 24 h respawn suppression
(`zone-service.js:34,38`). Test labels: "entry stairs return to town"
(`networking_tests.cpp:170-175`), "stairs-up emits the same player:extract
bank summary" (:374), "entry waymark returns to the Crossroads" /
"dead stays dead" / "the Warden stays down inside the linger window"
(`world-web.mjs:92-110`). Protocol matrix extraction row is already green
(`docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md:25`).

## Parity observations (cited, unfixed — outside owned paths)

- **P1** Native chart rows omit `levelHint` and `childIds`; browser payload
  also carries `houseName`. (`networking.cpp:1424-1432` vs
  `world-web.js:212,214,217`, `zone-service.js:125`, `Chart.vue:32`.)
- **P2** Node identity is intentionally not cross-server stable — C++
  documents "The hash need not match JS bit-for-bit" (:644-646) and uses
  FNV-1a bit slices (:674-678,:701-709) vs xmur3/mulberry32
  (`world-web.js:96-122`); name pools differ (C++ 6×6 compounds vs JS 12×10 +
  standalones). The same nodeId string resolves to different
  name/template/layout/branching per implementation, so browser-captured
  concrete info will not describe the native journey until reconciled.
- **P3** Copper Road pairs diverge (browser volcanic/sand `world-web.js:56-61`
  vs C++ crypt/wilds :655-656); all four C++ blurbs are truncated prefixes of
  the browser blurbs (:647-657).
- **P4** Browser pushes `world:chart:updated` on Warden death
  (`zone-service.js:404-411`); the native server emits no such event.
- **P5** No chart pane exists in the native client (no matches for "chart"
  under `native/client/`) — Gate C's deciding surface is client-pending,
  consistent with `NATIVE_CLIENT_PROTOCOL_MATRIX.md:7`.

## Verdict

`gateCRouteDecisionReadyToday: false`. Two of six fields are outright MISSING
(**goal**, **trophy/material/item family**), one is derivable-only
(**branch consequence**, immediate stage), three are AVAILABLE
(**boss/danger**, **depth**, **extraction/return condition**). A route name
plus today's blurb/tier alone therefore still fails the Gate C contract.
Machine-readable detail: `captures/gate-c-contract.json`.
