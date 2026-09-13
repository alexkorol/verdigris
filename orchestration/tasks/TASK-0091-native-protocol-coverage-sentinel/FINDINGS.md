# TASK-0091 FINDINGS — native client protocol coverage sentinel design

- lane: ox-pc-bc (model openrouter/stealth/ox-alpha)
- task base_commit: `d2423873c577d299b3b39c56024d1d840993c72b`
- surveyed head (all file:line evidence resolves here): `6c7d48e7a00caf3254755129e157d1c69e729dde`
  - base is an ancestor of the surveyed head; `native/**` moved between them
    (+13184/−922 across 49 files), so every citation below is against the
    surveyed tip, not the older matrix snapshot lines.
- claim commit: `64fab114ded129c12c8d16e996240e39ef823414`
- source of rows: `docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md` (read-only; not edited)
- machine-readable twin: `captures/coverage.json` (same row ids, same classifications)

## Method and classification rules

Each matrix row is mapped to four columns of current evidence:

1. **native handler** — server-side emitter/dispatch in `native/src/networking.cpp`.
2. **client reducer/model** — where `RemoteProtocolSession` consumes the
   envelope into `ClientModel` (`native/client/remote_session.cpp`,
   `client_model.hpp`). `RemoteProtocolSession` is the only client socket
   owner (native/client/remote_session.hpp:23).
3. **presentation op** — `PresentationEvent`
   (`native/client/presentation_events.hpp:12`) → fx
   (`presentation_state.cpp:193`) → recorded semantic draw ops
   (`render_list.hpp:16-45`, `main.cpp` GDI paths).
4. **automated test** — literal `check()` label + line in
   `session_tests.cpp` / `networking_tests.cpp` /
   `presentation_events_tests.cpp`, or `scenario_check()` label in
   `native/client/main.cpp` scenarios (run by ctest target
   `verdigris_client_scenarios`, CMakeLists.txt:87).

Classification:

- **COVERED** — all four columns resolve to real code AND an automated test
  asserts the step through its production path.
- **PARTIAL** — the step works today but at least one column's evidence is
  indirect, synthetic, or unexercised (e.g., envelope proven server-side but
  never consumed/asserted client-side). Missing evidence stays red inside the
  affected cell.
- **RED** — a required cell is missing outright (no code or no test anywhere).

Frozen invariants honored: `{event,data}` envelope, existing scenario
assertions, D-122 journey gates, server-authoritative ownership — none were
edited; this lane owns only `orchestration/tasks/TASK-0091-*/`.

## Gate A — journey rows (18)

| id | Journey step | Classification | Native handler | Client reducer/model | Presentation op | Automated test |
|---|---|---|---|---|---|---|
| A01 | Connect | COVERED | `WebSocketServer::handle_connection` networking.cpp:2828 (101 upgrade) | `connect_transport` remote_session.cpp:248-297; `Connected` :293; `ConnectionEstablished` :294-295 | `ConnectionEstablished` arm presentation_state.cpp:296-301 (events.hpp:13) | session_tests.cpp:172 `remote: connect + upgrade + login sent`; :183 SessionReady |
| A02 | Guest login | COVERED | handle_message login branch networking.cpp:2984-2988; `player:login` :2753; `emit_login` :2374 | auto-login remote_session.cpp:299-318; reducer :637-675 (uuid/x/y/facing/inventory); `SessionReady` :669-674 | `SessionReady` presentation_state.cpp:297 (events.hpp:15) | session_tests.cpp:177 `remote: scene snapshot mirrored`; :183 `remote: SessionReady presentation event emitted` |
| A03 | Dead endpoint (negative) | COVERED | — (refusal) | start() fail remote_session.cpp:373-392 → `Rejected`; fail() :971-975 emits ConnectionLost | ConnectionLost fx presentation_state.cpp:291-295 | session_tests.cpp:155/:157/:158 `remote-negative: …not a silent local fallback` |
| A04 | Session replaced | COVERED | replacement flush networking.cpp:2988 (old conn gets `player:session-replaced`) | remote_session.cpp:721-726 → Disconnected, suppress retry | ConnectionLost via fail() :974 | session_tests.cpp:480/:481/:482 `replaced: …` |
| A05 | Zone entry | COVERED | `world:zone:enter` networking.cpp:2386 → world:scene:transition + ground change | submit EnterZone remote_session.cpp:455-458; transition reducer :745-754; stairsUp parse :179-189; ClientScene client_model.hpp:61-68 | scene HUD/stairs pad: Extraction op main.cpp:3285, presentation_state.cpp:374 | networking_tests.cpp:134/:140/:144; journey session_tests.cpp:265/:266 |
| A06 | Movement | COVERED | `player:move` networking.cpp:2388 → `emit_movement` :2377 | submit Move remote_session.cpp:418-428; reducer player:movement :740-744 | model→world sync presentation_state.cpp:80; Player/Monster ops record_world_ops :368 | networking_tests.cpp:111/:114; journey session_tests.cpp:276 |
| A07 | Aim | COVERED | — presentation-local by design | Aim case remote_session.cpp:430-439 sets facing, sends nothing (comment :431-432) | facing drives swing direction | session_tests.cpp:280 `journey: aim updates facing` |
| A08 | Primary action | COVERED | `player:skill:trigger` networking.cpp:2507 | submit UseAction remote_session.cpp:440-444; outgoing branch :797-850 | Swing op presentation_state.cpp:401 / main.cpp:1641; Impact main.cpp:1685; Damage :1728 | session_tests.cpp:300 `journey: outgoing combat:hit reached the client`; render-list Swing :538 |
| A09 | Telegraph | COVERED | telegraph arm networking.cpp:2061-2066 (emit :2066) | remote_session.cpp:756-769 upsert_monster + Telegraph event | Telegraph fx presentation_state.cpp:229-242; op :394; GDI main.cpp:1527/:1582 | session_tests.cpp:370 `journey: monster:telegraph reached the client` |
| A10 | Damage in/out | COVERED | process_combat :2079; combat:hit emitter :2077 (parity critical/attackStyle) | combat:hit branches remote_session.cpp:770-851 (incoming life/last_incoming_hit :780-796; outgoing :797-850) | DamageApplied fx :205-228; Damage/TargetFlash/ScreenPulse ops :414/:423/:430/:438/:446; GDI main.cpp:1728/:1768/:1784/:1821 | session_tests.cpp:300 outgoing; :369 incoming; phase-a contract presentation_events_tests.cpp:86-101 |
| A11 | Enemy death | COVERED | died flag on combat:hit networking.cpp:2070-2077 | died branch remote_session.cpp:831-848: kills++ :832, ActorDied :835-840 | Death ring fx :243-250; Death op :419; GDI main.cpp:1692 | session_tests.cpp:301 `journey: enemy death reached the client`; Drop-from-loot :539 |
| A12 | Item drop | **PARTIAL** | `emit_ground_change` networking.cpp:995-998 (`world:itemDropped` :997, `item:change` :998) | **no consumer for either envelope**; ground arrives via dev:state poll (request remote_session.cpp:514-520; ingest :895-910); kill-drop beat synthesized locally :841-847 | ItemDropped fx :270-282; Drop op :389; Loot draw renders model.ground main.cpp:3497-3504 — automated Drop-op proof (:539) covers only synthesized kill loot | wire-only: networking_tests.cpp:263 `floor treasure emits item:change and world:itemDropped`; :266 ground fields. **No test drives either envelope into RemoteProtocolSession.** See negative control N-1 |
| A13 | Pickup | COVERED | `player:take:underfoot` networking.cpp:2564 → core:refresh:inventory emit :989; relic flip mark_relic_recovered :1674 (call :1721) | submit PickUp remote_session.cpp:446-448; refresh reducer :920-968; ItemPickedUp diff :937-949 | ItemPickedUp fx :283-286 | session_tests.cpp:334/:335 (ground-walk driver :306-325 proves dev:state ground sync feeds pickup targeting); relic recovery :1948 + scenario main.cpp:5005 |
| A14 | Equip | COVERED (deviation noted) | `item:equip` networking.cpp:2522 → `player:equippedAnItem` emit :1009 (wear projection) | submit Equip remote_session.cpp:449-454 (pending_equip_uuid_); completion inferred from refresh diff :950-966 → model_.equipped :959-960; ItemEquipped :961-963. **Client never parses `player:equippedAnItem`** (see N-2 grep) | ItemEquipped arm presentation_state.cpp:296-301; PaneWeapon op render_list.hpp:39 | networking_tests.cpp:402/:405/:412 (wire+wear); session_tests.cpp:346-349 (model().equipped asserted; backpack removal) |
| A15 | Extraction | COVERED | stairs-up inside player:move networking.cpp:2388 (surface message + finish_extraction); finish_extraction :1011-1040 (bank summary :1040); explicit handler :2523; flag :1541 | Extract command message-only guidance remote_session.cpp:459-464 (no envelope sent); surface-message reducer game:send:message :727-739 → extracted=true + ExtractionCompleted :733-736 | ExtractionCompleted fx :287-290; Extraction pad op :374/main.cpp:3285; PaneBanked main.cpp:2278 | session_tests.cpp:389/:392 `journey: ExtractionCompleted from surface message`; networking_tests.cpp:374/:378/:385 |
| A16 | Disconnect | COVERED | transport close (opcode 8 break, networking.cpp:2828) | reader peer-drop remote_session.cpp:628-633 → begin_retry :333-352 (Retrying :349; ConnectionLost once :340-343); fail() :971-975 | ConnectionLost fx :291-295 | session_tests.cpp:427 `unexpected drop enters Retrying`; :428 `ConnectionLost is visible (no silent local fallback)`; :435 position frozen offline |
| A17 | Reconnect | COVERED | identity-keyed session reuse networking.cpp:2988 (replace_socket / reset_world_for_new_socket) | pump_retry backoff remote_session.cpp:354-371; re-login :299-318 | Retrying→Ready surfaced via state label | session_tests.cpp:441/:442/:443 `reconnect: …` |
| A18 | Persistence | **RED** | required: N5 durable House restore — absent | absent (no durable store) | absent | absent — negative control N-4 |

## Gate B — Chronicles wire-contract rows (5)

The frozen matrix text says every chronicles step's test column was RED ("no
native test label") at TASK-0081's base `986264f4`. At this surveyed tip that
is stale: `rg -n 'chronicles' native/tests/session_tests.cpp` finds 41
matches (gate-b labels :1712-2074), and the production-client scenario
`scenario_chronicles_gate_b` (native/client/main.cpp:4802, ctest
`verdigris_client_scenarios`) drives `RemoteProtocolSession` with
`quick_guest=false` (main.cpp:4832-4834) end-to-end. Rows are classified on
current evidence; the matrix document itself was not edited.

| id | Envelope/journey | Classification | Native handler | Client reducer/model | Presentation op | Automated test |
|---|---|---|---|---|---|---|
| B01 | House lifecycle | COVERED | `chronicles:house:found` networking.cpp:2570-2579 (state emit :2578); alt mutate :2635; awaitChronicles login branch :2753-2771 | FoundHouse submit remote_session.cpp:465-468; trio reducer chronicles:state/ready/update :676-698 → apply_chronicle_object :196-237 → ClientChronicle client_model.hpp:117-124 | Op::Chronicles front door render_list.hpp:42; emitted main.cpp:3000/:3015 | session_tests.cpp:1723 `gate-b: non-quick guest login admits chronicles:state`; :1733 `house:found emits chronicles:state`; scenario main.cpp:4839 front door opens; :4851 roster renders |
| B02 | Scion lifecycle | COVERED | create :2581-2589 (createdScionId); set-out :2592+; select admission :2718+ → player:login | CreateScion :469-477; SelectScion :479-500 (roster house resolution); SetOut :502-505; admission reducer = player:login chronicles{} :642-649 + Ready :669-674 | HouseChip render_list.hpp:44 emitted main.cpp:3731; oath states main.cpp:4868-4876 | session_tests.cpp:1757 createdScionId; :1767 set-out admits; scenario main.cpp:4864 scion joins; :4885 mortal select lands in world; :4966 heirship select admits |
| B03 | Death | COVERED (sibling-envelope note) | trigger :2185 (+guard :2460); handle_final_death :2200-2269: scion-fallen :2262, scion-witnessed broadcast :2269 | chronicles:scion-fallen reducer remote_session.cpp:699-720 → fallen struct, alive=false :712, ScionDied :713-715, Message :716-718 | ScionDied arm presentation_state.cpp:243-250; front-door fallen/crypt lines main.cpp:4929-4932 | wire gate-b session_tests.cpp:1821-1822 fallen observed; production-client main.cpp:4924 `consequence: scion-fallen names the fallen Scion`; :4926 fall returns owner to chronicles |
| B04 | Successor | **PARTIAL** | return leg `player:chronicles:return` networking.cpp:2672-2715; recovery chain dev:release-relic :2483, chroniclesRelic ground field :479, mark_relic_recovered :1674 | recovery leg green (relic ingest :903-907 with provenance; pickup flip tested); **return leg has NO client seam**: no ClientCommand type (session.hpp:40-55), zero references to `"player:chronicles:return"` in native/client (negative control N-3); production succession goes straight through select (main.cpp:4946-4966), which carries a RECORDED RED workaround comment (main.cpp:4967+, dev:heal) | Op::Chronicles succession actions main.cpp:4934/:4955; relic toast main.cpp:3015/:5007; Hud relic line | wire: session_tests.cpp:1846 `player:chronicles:return readies succession`; :1861 successor created; :1874 heir admitted; :1937 elite surfaces heirloom; :1948 exact relic recovered; scenario main.cpp:5005 crypt flips lost→recovered |
| B05 | Persistence | **RED on durable leg** (in-process legs green) | in-process reuse networking.cpp:2988 adoption path; durable store **absent** | reconnect restores ClientChronicle via ready/state reducers :676-698 | roster renders on return (scenario) | in-process: session_tests.cpp:1993/:2006/:2009/:2010-2011 identical record after reconnect; :2043-2074 roster/crypt/oath/carried; scenario main.cpp:5033 `reconnect: House, heir, and crypt render on return`. Durable: nothing — see N-4 |

Totals: **19 COVERED · 2 PARTIAL · 2 RED** across 23 rows.

## Negative controls (literal evidence preventing false greens)

N-1 — A12 Item drop: the frozen response envelopes are dead letters to the native client.

```
PS> rg -n '"world:itemDropped"|"item:change"' native/client
(no output)
exit code: 1
```

while native/src/networking.cpp:997-998 emits both and networking_tests.cpp:263/:266 assert
them server-side only. No test feeds them to RemoteProtocolSession.

N-2 — A14 Equip deviation: authoritative response unconsumed by name.

```
PS> rg -n '"player:equippedAnItem"' native/client
(no output)
exit code: 1
```

Client equip outcome is inferred from core:refresh:inventory diffing
(remote_session.cpp:950-966). Both halves are individually labeled green
(networking_tests.cpp:402-412 wire; session_tests.cpp:346-349 model), so the
row holds COVERED, but a silent rename of the response would pass every
client-side test — sentinel watch item.

N-3 — B04 Successor: named return envelope unreachable from any client input path.

```
PS> rg -n '"player:chronicles:return"' native/client
(no output)
exit code: 1
```

ClientCommand::Type (native/client/session.hpp:40-55) defines no return
intent; the server handler (networking.cpp:2672) is only exercised wire-level
via LoopbackClient (session_tests.cpp:1846).

N-4 — A18/B05 Persistence: no durable cross-process store exists.

```
PS> rg -n 'ofstream|ifstream|fopen|fwrite' native/src
(no output)
exit code: 1
```

Server restart loses Houses/Scions/crypt; no quit/logout envelope exists;
no test asserts a persisted House across a restart. In-process reuse is
separately green (labels above).

Watch list (emitted but never consumed/asserted anywhere):
`chronicles:scion-witnessed` (networking.cpp:2269; rg over native/client +
native/tests → no matches) and direct `player:stats:update` lifecycle push
(client mirrors lifecycle from dev:state instead, remote_session.cpp:858-859).

## Matrix drift observed (documented; matrix NOT edited)

- Stale line citations: most session_tests.cpp references moved (e.g., matrix
  cites :105/:121/:123/:243-244/:314-317/:365-368/:405-406; current labels
  live at :155/:172/:183/:300-301/:389-392/:441-443/:480-481).
- networking.cpp handler lines moved wholesale since the 0081 freeze (e.g.,
  chronicles:house:found cited :2498-2508, now :2570-2579).
- The Gate B blanket "no native test label" claim is outdated at this tip:
  41 chronicle matches in session_tests.cpp plus the production-client
  scenario exist.
- Minor: client comment remote_session.cpp:460 claims "No player:extract
  handler exists on the native server"; an explicit handler exists at
  networking.cpp:2523. Behavior unaffected (walking stairs remains the
  shipped extraction surface).

A docs-owned task should refresh NATIVE_CLIENT_PROTOCOL_MATRIX.md; this lane
may not edit it.

## Proposed read-only sentinel interface (design only — NOT implemented)

Purpose: prevent a green journey row from silently losing a required wire
step. Phase 1 is static and read-only; it binds no ports and starts no
servers (resource capsule respected).

```cpp
// proposed new file: native/tools/protocol_coverage_sentinel.cpp
// (successor task implements; data contract already captured in captures/coverage.json)
namespace verdigris::sentinel {

struct SentinelRow {
  std::string row_id;                       // "A01".."A18", "B01".."B05"
  std::vector<std::string> server_emits;    // envelope names that must appear in native/src/**
  std::vector<std::string> client_consumes; // envelope names that must appear in native/client/**
                                            // empty => deviation_note is mandatory
  std::string deviation_note;               // e.g. A12 dev:state seam; B04 no-return-seam
  std::vector<std::string> test_labels;     // literal check()/scenario_check() strings
                                            // that must exist in native/tests/** or
                                            // native/client/main.cpp
};

struct Violation {
  std::string row_id;
  // "missing-server-emit" | "missing-client-consume" | "missing-test-label"
  std::string kind;
  std::string detail;
};

struct Report {
  std::vector<Violation> violations;
};

// Reads coverage JSON + repo sources read-only; prints findings to stdout.
// Returns 0 iff violations.empty(); nonzero otherwise. Never writes files,
// never opens sockets, never spawns processes.
int run_sentinel(const std::string& coverage_json_abs_path,
                 const std::string& repo_root,
                 Report* out);

}  // namespace verdigris::sentinel
```

Design notes:

- Match by literal strings (envelope names, test labels), not line numbers —
  the drift table above shows why line pins rot.
- Input contract is exactly this task's `captures/coverage.json` schema
  (`rows[].required.server_emits / client_consumes / test_labels /
  deviation_note`), so the map and the sentinel cannot disagree silently.
- Wiring (successor): one CMake test target
  `add_test(NAME verdigris_protocol_sentinel ...)` reading
  `orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/captures/coverage.json`;
  runs anywhere `ctest` runs; no Windows-specific bits.
- Optional phase 2 (same owner): dynamic tier reusing the existing ephemeral
  loopback capsule pattern (session tests bind 7159-7179 in-process; client
  scenarios bind 6780-6799) driving `RemoteProtocolSession` through one
  scripted journey requiring ≥1 PresentationEvent/model delta per COVERED
  row. Converts "label exists" into "step actually fires".

## Successor recommendation (smallest owner paths)

1. A12: consume `item:change`/`world:itemDropped` in `RemoteProtocolSession`
   (map onto `model_.ground` + `ItemDropped`) or formally amend the matrix
   row to name `dev:state.groundItems` as the canonical native channel.
   Owner family: TASK-0077/TASK-0061 client work.
2. A18/B05 durable leg: implement behind the existing
   `native/persistence/adapter.hpp` stub + decide a quit/logout envelope +
   one restart test asserting House/Scion/crypt survival. Owner family:
   TASK-0056 (N5). Until then these stay honestly RED.
3. B04: add `ClientCommand::Type::ReturnToHouse` translating to
   `player:chronicles:return` plus one journey assertion — or declare
   select-direct succession the shipped path and demote the return envelope
   in the matrix.
4. Adopt the phase-1 sentinel wired to `captures/coverage.json` so any of
   the above renames/removals flips CI red instead of drifting silently.
5. Watch-list decisions (consume-or-document):
   `chronicles:scion-witnessed`, direct `player:stats:update`.
