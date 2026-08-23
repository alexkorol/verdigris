# TASK-0091 REPORT — native client protocol coverage sentinel design

- lane: `ox-pc-bc` · model: `openrouter/stealth/ox-alpha`
- task base SHA: `d2423873c577d299b3b39c56024d1d840993c72b`
- surveyed head (evidence + acceptance runs resolve here): `6c7d48e7a00caf3254755129e157d1c69e729dde`
- claim commit: `64fab114ded129c12c8d16e996240e39ef823414` (STATUS.md CLAIMED, pushed)
- branch: `worker/verdigris/pc/ox-pc-bc`; push target: `origin/worker/verdigris/pc/ox-pc-bc` only
- owned scope respected: every changed path is under
  `orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/`
- resource capsule: read-only; no ports bound, no servers started, port 6500 untouched;
  matrix/native sources/tests read but never edited

## Deliverables

| File | Purpose |
|---|---|
| `STATUS.md` | claim/review state machine (CLAIMED → REVIEW_REQUESTED) |
| `FINDINGS.md` | full coverage map for all 23 matrix rows (native handler / client reducer-model / presentation op / automated test per row), COVERED-PARTIAL-RED classifications with file:line + label citations, negative controls, drift log vs the frozen matrix, read-only sentinel interface proposal (design only), successor recommendation |
| `captures/coverage.json` | machine-readable twin of FINDINGS.md; same row ids/classifications; negative-control probes with literal commands + observed outputs + exit codes; sentinel row contract fields (`server_emits`, `client_consumes`, `deviation_note`, `test_labels`) ready for the phase-1 static audit |
| `captures/acceptance-transcripts.txt` | verbatim UTF-8 transcript of all five spec acceptance commands (349 lines, byte-faithful incl. matrix glyphs) |

## Classification summary (23 rows)

- Gate A (18 rows): **16 COVERED** · **A12 Item drop PARTIAL** · **A18 Persistence RED**
- Gate B (5 rows): **B01 House lifecycle COVERED** · **B02 Scion lifecycle COVERED** ·
  **B03 Death COVERED** (witness broadcast on watch list) · **B04 Successor PARTIAL** ·
  **B05 Persistence RED on the durable leg** (in-process legs green)

Negative controls required by spec are present:

- N-1 (grounds A12 PARTIAL): `rg -n '"world:itemDropped"|"item:change"' native/client`
  → no matches, exit 1 — while networking.cpp:997-998 emits both and
  networking_tests.cpp:263/:266 prove them server-side only.
- N-2 (grounds A14 deviation note): `rg -n '"player:equippedAnItem"' native/client`
  → no matches, exit 1 — equip completion inferred from core:refresh:inventory diff
  (remote_session.cpp:950-966); both halves individually labeled green.
- N-3 (grounds B04 PARTIAL): `rg -n '"player:chronicles:return"' native/client`
  → no matches, exit 1 — ClientCommand::Type (session.hpp:40-55) has no return intent;
  server handler networking.cpp:2672 exercised only wire-level (session_tests.cpp:1846).
- N-4 (grounds A18 RED, B05 durable cell RED): `rg -n 'ofstream|ifstream|fopen|fwrite' native/src`
  → no matches, exit 1 — no durable store exists; no restart-survival test.

## Acceptance commands — literal transcripts and exit codes

Run verbatim from repo root; complete byte-faithful capture committed at
`captures/acceptance-transcripts.txt` (line references below point into it).
Staging note: the two new files were registered via `git add -N` (intent-to-add)
so CMD 5's plain `git diff --name-only` lists them; this changes no content.

### CMD 1 — exit code 0 (transcript lines 4-20)

```
$ rg -n "Gate A|Gate B|Gate C|chronicles:|world:|player:" docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md
12:| Guest login | auto after connect | `player:login` {guestId, quickGuest} | handle_message login branch | `player:login` {player, scene} | uuid/x/y/facing/scene/inventory | SessionReady event | ✅ `native/tests/session_tests.cpp:123` — `remote: player:login acknowledged -> ready` |
14:| Session replaced | second login same guest | — | session-replacement flush | `player:session-replaced` | Disconnected | ConnectionLost event | ✅ `native/tests/session_tests.cpp:405-406` — `replaced: first session is disconnected`; `replaced: ConnectionLost from player:session-replaced` |
15:| Zone entry | E / route select | `world:zone:enter` {nodeId} | handler exists | `world:scene:transition` | scene + stairs_up | scene name / stairs pad | ✅ `native/tests/networking_tests.cpp:134-145` — `solo entry emits a scene transition`; `state reports instance`; `both stairs exist` |
16:| Movement | WASD | `player:move` {direction} | handler exists | `player:movement` | x/y/facing | actor motion | ✅ `native/tests/networking_tests.cpp:111-114` — `applied sample broadcasts player:movement`; `right sample moves east`; `both stairs exist` |
18:| Primary action | click/space | `player:skill:trigger` | handler exists | combat events | last_outgoing_hit | swing/impact fx | ✅ `native/tests/session_tests.cpp:243` — `journey: outgoing combat:hit reached the client` |
22:| Item drop | — | — | drop pipeline | `item:change`, `world:itemDropped` | ground item list | drop sparkle + log | ✅ `native/tests/networking_tests.cpp:262-266` — `floor treasure emits item:change and world:itemDropped`; `ground envelope has uuid, id, name, x, y` |
23:| Pickup | X | `player:take:underfoot` | handler exists | `core:refresh:inventory` | inventory slots | pickup log + backpack | ✅ `native/tests/session_tests.cpp:259-260` — `journey: named item entered inventory (pickup)`; `journey: ItemPickedUp or inventory growth` |
24:| Equip | 1-9 | `item:equip` {item.uuid} | handler exists | `player:equippedAnItem` | equipped slot + item stats | equipped HUD | ✅ `native/tests/networking_tests.cpp:402-412` — `item:equip emits player:equippedAnItem`; `equip response includes wear-slot state`; `snapshot wear matches the equip response` |
25:| Extraction | walk stairs-up | `player:move` onto stairs | stair transition | `player:extract` / surface message | extracted + town scene | banked banner | ✅ `native/tests/session_tests.cpp:314-317` — `walking onto stairs-up returns to the surface (extract)`; `ExtractionCompleted from surface message` |
30:## Gate B — Chronicles wire-contract freeze (TASK-0081)
45:| House lifecycle | Found: `chronicles:house:found` {name} → ... (`ProtocolSession::handle` :2498-2508 ...) Alt existing mutate path: `player:chronicles:mutate` ... Restore: `player:login` {guestId} → `chronicles:state`; `{awaitChronicles:true}` → `player:chronicles:ready` ... | 🧩 | 🔴 RED: no native test label |
46:| Scion lifecycle | Create: `chronicles:scion:create` ... Select/admission: `player:chronicles:select` → `player:login` ... Set-out: `chronicles:scion:set-out` {scionId} → `player:login` (+ one-shot `game:send:message` daily road-purse notice :2530-2534 ...) | 🧩 | 🔴 RED: no native test label |
47:| Death | Server-initiated fatal fall ... `ProtocolSession::handle_final_death` :2128-2207 emits `chronicles:scion-fallen` ... broadcasts `chronicles:scion-witnessed` ... direct `player:stats:update` ... Fallen scion moves houses[].scions[] → houses[].crypt[] ... | 🧩 | 🔴 RED: no native test label |
48:| Successor | No dedicated successor event: reuse `chronicles:scion:create` → `player:chronicles:select` ... Return path: `player:chronicles:return` {} → `player:chronicles:ready` ... pickup `player:take:underfoot` → `core:refresh:inventory` + `world:itemDropped`/`item:change` and `mark_relic_recovered` flips crypt relic lost→recovered ... | 🧩 | 🔴 RED: no native test label anywhere on the chain |
exit code: 0
```

(Verbatim including all glyphs is in the transcript file; long Gate B cells are
wrapped above only for readability.)

### CMD 2 — exit code 0 (transcript lines 23-330; 305 matching lines)

```
$ rg -n "RemoteProtocolSession|ClientSnapshot|PresentationEvent|render_list" native/client native/tests
native/tests\session_tests.cpp:90:    if (event.type == verdigris::client::PresentationEventType::SessionReady) saw_ready = true;
native/tests\session_tests.cpp:118:    auto* remote = dynamic_cast<verdigris::client::RemoteProtocolSession*>(&session);
[...299 further matching lines omitted here; verbatim in captures/acceptance-transcripts.txt lines 24-329...]
native/client\remote_session.cpp:846:        pending_events_.push_back({PresentationEventType::ItemDropped,
native/client\remote_session.cpp:946:        pending_events_.push_back({PresentationEventType::ItemPickedUp, model_.player.uuid,
native/client\remote_session.cpp:961:        pending_events_.push_back({PresentationEventType::ItemEquipped, model_.player.uuid,
exit code: 0
```

Note: `ClientSnapshot` contributes zero of those matches — the client model type is named
`ClientModel` (`native/client/client_model.hpp:131`). Recorded so a future symbol rename is
not mistaken for coverage.

### CMD 3 — exit code 0 (transcript lines 333-336)

```
$ node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/captures/coverage.json','utf8')); console.log('coverage JSON: PASS')"
coverage JSON: PASS
exit code: 0
```

### CMD 4 — exit code 0 (transcript lines 339-341)

```
$ git diff --check
(no output — no whitespace errors)
exit code: 0
```

### CMD 5 — exit code 0 (transcript lines 344-348)

```
$ git diff --name-only
orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/FINDINGS.md
orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/captures/coverage.json
exit code: 0
```

Only owned task-folder paths appear. `REPORT.md`, this flip's STATUS edit, and
`captures/acceptance-transcripts.txt` are written after the transcript was captured and stay
inside the owned folder (see changed-file list below).

## Changed-file list (complete, entire lane)

1. `orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/STATUS.md` (new; CLAIMED → REVIEW_REQUESTED)
2. `orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/FINDINGS.md` (new)
3. `orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/captures/coverage.json` (new)
4. `orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/captures/acceptance-transcripts.txt` (new)
5. `orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/REPORT.md` (this file)

## Stop/fallback status

Not blocked. All 23 provable rows completed and classified; smallest owner paths for each
red/partial seam recorded (FINDINGS.md §Successor recommendation).

## Handoff

STATUS.md flips to `state: REVIEW_REQUESTED` recording the frozen content commit (the
deliverables commit this report ships in). Reviewer can re-run the five acceptance commands
verbatim from the repo root and diff against `captures/acceptance-transcripts.txt`.
