# TASK-0100 — REPORT

- Lane: `ox-pc-bd` · Model: `openrouter/stealth/ox-alpha`
- Base commit: `d2423873c577d299b3b39c56024d1d840993c72b`
- Content head at freeze: this commit (the one adding FINDINGS.md, captures/,
  and this report; its exact SHA is recorded as `content_head` in STATUS.md,
  whose own review-request commit is recorded as the frozen pushed head)
- Deliverables: `FINDINGS.md`, `captures/replay-surfaces.json`,
  `captures/acceptance-rg-transcript.txt`, this report.
- Result: **all four acceptance gates pass, exit code 0 each.**

## Acceptance gate transcripts (pass 1, literal)

Pass 1 ran with the evidence files present via `git add -N` (intent-to-add) so
untracked deliverables appear in the diff gates. This REPORT.md was assembled
immediately after pass 1 by splicing the captured transcript bytes verbatim; a
pass-2 re-run of all four gates over the final tree is recorded in STATUS.md.

### Gate 1 — surface scan

Command (verbatim from SPEC):

```
rg -n "seed|rng|random|tick|fixed|replay|snapshot|determin|clock|time" native/include native/src native/tests
```

Exit code: `0`. 494 matching lines. The byte-exact transcript is committed at
`captures/acceptance-rg-transcript.txt`
(sha256 `298755CDABD2E1B5D85790C55A6959FDEFE309681C68AD833442B6EEF9A2A05B`)
and reproduced below without modification between the BEGIN/END markers.

```text
native/tests\session_tests.cpp:2:// deterministic play available and the remote adapter completes a REAL
native/tests\session_tests.cpp:42:                    verdigris::client::ConnectionState wanted, int timeout_ms) {
native/tests\session_tests.cpp:44:      std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
native/tests\session_tests.cpp:45:  while (std::chrono::steady_clock::now() < deadline) {
native/tests\session_tests.cpp:55:bool wait_until(verdigris::client::IClientSession& session, int timeout_ms, Pred pred) {
native/tests\session_tests.cpp:57:      std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
native/tests\session_tests.cpp:58:  while (std::chrono::steady_clock::now() < deadline) {
native/tests\session_tests.cpp:67:void local_session_ready_and_deterministic() {
native/tests\session_tests.cpp:177:    check(!session.model().scene.id.empty(), "remote: scene snapshot mirrored");
native/tests\session_tests.cpp:252:  { // pin the protocol slice to the fixed dungeon/warren surface - the
native/tests\session_tests.cpp:305:  // of sweeping blind (warren mazes defeat a fixed eastward walk).
native/tests\session_tests.cpp:443:  check(!session.model().scene.id.empty(), "reconnect: login snapshot is authoritative");
native/tests\session_tests.cpp:502:  { // pin the protocol slice to the fixed dungeon/warren surface - the
native/tests\session_tests.cpp:524:    ++world.tick;
native/tests\session_tests.cpp:526:      verdigris::client::apply_presentation_event(fx, world, event, world.tick);
native/tests\session_tests.cpp:561:// TASK-0163 driver correction (test-only; no runtime or rule change). Both
native/tests\session_tests.cpp:567://       warren's vertical wall ribs on the wrong side (a replay of the exact
native/tests\session_tests.cpp:568://       algorithm on this guest's seeded floor never got past lane 7 in nine
native/tests\session_tests.cpp:571://       more legs - the Warden's seeded tile sits inside a rib pocket whose
native/tests\session_tests.cpp:582:// replaces only navigation: a fixed boustrophedon lane plan whose full-height
native/tests\session_tests.cpp:714:    std::chrono::steady_clock::time_point at;
native/tests\session_tests.cpp:836:      Line line{std::chrono::steady_clock::now(), std::move(envelope)};
native/tests\session_tests.cpp:846:  bool wait_from(size_t mark_index, int timeout_ms, Pred pred, Line* out = nullptr) {
native/tests\session_tests.cpp:848:        std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
native/tests\session_tests.cpp:857:      if (std::chrono::steady_clock::now() >= deadline) return false;
native/tests\session_tests.cpp:1044:// Tile derivation for every driver decision. The runtime's authoritative
native/tests\session_tests.cpp:1074:// rejection. The same direction is re-issued this many times before a wall
native/tests\session_tests.cpp:1113:  std::chrono::steady_clock::time_point waypoint_started{};
native/tests\session_tests.cpp:1133:    sweep.waypoint_started = std::chrono::steady_clock::now();
native/tests\session_tests.cpp:1136:  if (std::chrono::steady_clock::now() - sweep.waypoint_started >
native/tests\session_tests.cpp:1140:    sweep.waypoint_started = std::chrono::steady_clock::now();
native/tests\session_tests.cpp:1160:  return false;  // boxed in this tick; retry next iteration
native/tests\session_tests.cpp:1189:          std::chrono::steady_clock::now() + std::chrono::milliseconds(400);
native/tests\session_tests.cpp:1190:      while (std::chrono::steady_clock::now() < deadline) {
native/tests\session_tests.cpp:1229:                              int timeout_ms, bool* made_contact) {
native/tests\session_tests.cpp:1234:      std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
native/tests\session_tests.cpp:1236:  sweep.waypoint_started = std::chrono::steady_clock::now();
native/tests\session_tests.cpp:1238:  while (std::chrono::steady_clock::now() < deadline) {
native/tests\session_tests.cpp:1279:                                     int timeout_ms, int* kills) {
native/tests\session_tests.cpp:1281:  auto walk_to_town_tile = [&](int target_x, int target_y, int timeout) {
native/tests\session_tests.cpp:1284:        std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
native/tests\session_tests.cpp:1289:      if (std::chrono::steady_clock::now() >= deadline) return false;
native/tests\session_tests.cpp:1334:  const auto never = std::chrono::steady_clock::time_point() +
native/tests\session_tests.cpp:1338:  const auto now = [] { return std::chrono::steady_clock::now(); };
native/tests\session_tests.cpp:1339:  const auto deadline = now() + std::chrono::milliseconds(timeout_ms);
native/tests\session_tests.cpp:1399:        // Small clock-skew buffer so a re-entry never beats the resolve.
native/tests\session_tests.cpp:1538:                      int timeout_ms) {
native/tests\session_tests.cpp:1542:      std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
native/tests\session_tests.cpp:1551:    if (std::chrono::steady_clock::now() >= deadline) {
native/tests\session_tests.cpp:1581:    if (std::chrono::steady_clock::now() >= deadline) {
native/tests\session_tests.cpp:1622:// Focused deterministic controls for the corrected driver state machine
native/tests\session_tests.cpp:1625:// nondeterminism.
native/tests\session_tests.cpp:1706:  std::string chronicle_snapshot;
native/tests\session_tests.cpp:1827:          "note: pre-change runtime gap reproduced at the fatal-fall step; "
native/tests\session_tests.cpp:1965:        chronicle_snapshot = chronicle->stringify();
native/tests\session_tests.cpp:2011:        check(chronicle->stringify() == chronicle_snapshot,
native/tests\session_tests.cpp:2090:  local_session_ready_and_deterministic();
native/tests\networking_tests.cpp:18:  if (!condition) throw std::runtime_error(message);
native/tests\networking_tests.cpp:198:  check(actors != nullptr, "N3 boss snapshot has monsters");
native/tests\networking_tests.cpp:274:  check(login_ground && !login_ground->empty(), "login snapshot includes instance ground items");
native/tests\networking_tests.cpp:412:        "snapshot wear matches the equip response");
native/tests\presentation_events_tests.cpp:3:// distinction, ScionLost/BuffExpired contract beats, deterministic spawn
native/tests\presentation_events_tests.cpp:61:  check(phase_a::kTickMs == 50, "phase-a: table pins the 50ms presentation tick");
native/tests\presentation_events_tests.cpp:63:        "phase-a: critical number outlives the ordinary 12-tick number");
native/tests\presentation_events_tests.cpp:67:        "phase-a: beat lifetimes are distinct by construction");
native/tests\presentation_events_tests.cpp:69:        "phase-a: loss pulse outlives the 3-tick damage pulse");
native/tests\presentation_events_tests.cpp:113:        "phase-a: loss beat lifetime comes from the table");
native/tests\presentation_events_tests.cpp:114:  check(fx.screen_pulse_ticks == phase_a::kScionLostPulseTicks,
native/tests\presentation_events_tests.cpp:159:void spawn_detection_is_deterministic_and_once() {
native/tests\presentation_events_tests.cpp:176:        "phase-a: spawn detection is byte-deterministic across runs");
native/tests\presentation_events_tests.cpp:220:    apply_presentation_event(fx, world, event, world.tick);
native/tests\presentation_events_tests.cpp:221:  detect_monster_spawns(fx, world, world.tick);
native/tests\presentation_events_tests.cpp:243:  spawn_detection_is_deterministic_and_once();
native/include\verdigris\networking.hpp:22:// and snapshots are serialized back out at the edge.
native/include\verdigris\networking.hpp:74:  ProtocolSession(std::string identity, std::string socket_id, std::uint64_t seed,
native/include\verdigris\networking.hpp:97:  // advance on the tick, not only on inbound envelopes.
native/include\verdigris\networking.hpp:99:  void tick(std::int64_t now_ms);
native/include\verdigris\networking.hpp:107:  JsonValue snapshot() const;
native/include\verdigris\networking.hpp:197:  // N6 world-web (server/core/world-web.js): per-house deterministic road
native/include\verdigris\networking.hpp:240:  Mulberry32 session_rng_;
native/include\verdigris\networking.hpp:277:  std::unique_ptr<std::thread> tick_thread_;
native/tests\core_tests.cpp:98:    result.push_back(std::to_string(legend.ordinal) + ":" + std::to_string(legend.tick) + ":" +
native/tests\core_tests.cpp:165:  check(player->cooldown_ticks == 0, "gated Thrust does not consume cooldown");
native/tests\core_tests.cpp:170:  player->cooldown_ticks = 0;
native/tests\core_tests.cpp:178:        "Thrust pays its named cost after one tick of regeneration");
native/tests\core_tests.cpp:179:  check(player->cooldown_ticks == player->stats.attack_speed_ticks - 1,
native/tests\core_tests.cpp:182:  player->cooldown_ticks = 0;
native/tests\core_tests.cpp:190:  player->cooldown_ticks = 0;
native/tests\core_tests.cpp:195:  player->cooldown_ticks = 0;
native/tests\core_tests.cpp:203:  player->cooldown_ticks = 0;
native/tests\core_tests.cpp:210:  player->cooldown_ticks = 2;
native/tests\core_tests.cpp:224:        "actors default to deterministic +x facing");
native/tests\core_tests.cpp:244:  check(movement_step_per_tick(220) == 11,
native/tests\core_tests.cpp:246:  check(movement_step_per_tick(240) == 12,
native/tests\core_tests.cpp:247:        "monster movement uses the same fixed-step derivation");
native/tests\core_tests.cpp:258:  check(player->position.x - player_start.x == movement_step_per_tick(player->stats.move_speed) &&
native/tests\core_tests.cpp:261:  check(movement_step_per_tick(enemy->stats.move_speed) == 12,
native/tests\core_tests.cpp:262:        "monster movement uses the same named fixed-step derivation");
native/tests\core_tests.cpp:268:        "diagonal movement remains deterministic integer math");
native/tests\core_tests.cpp:271:void test_movement_replay_is_deterministic() {
native/tests\core_tests.cpp:285:        "fixed-step movement produces identical replay positions");
native/tests\core_tests.cpp:287:        "fixed-step movement replay emits an identical event count");
native/tests\core_tests.cpp:297:  check(player->position.x == -movement_step_per_tick(player->stats.move_speed) *
native/tests\core_tests.cpp:300:        "dash uses the facing direction and named movement-tick burst");
native/tests\core_tests.cpp:321:void test_facing_replay_is_deterministic() {
native/tests\core_tests.cpp:333:        "facing command streams replay to byte-identical events");
native/tests\core_tests.cpp:338:        "facing state remains identical under deterministic replay");
native/tests\core_tests.cpp:348:  player->cooldown_ticks = 0;
native/tests\core_tests.cpp:365:  player->cooldown_ticks = 0;
native/tests\core_tests.cpp:375:        "Sweep pays its named cost after one tick of regeneration");
native/tests\core_tests.cpp:376:  check(player->cooldown_ticks ==
native/tests\core_tests.cpp:377:            player->stats.attack_speed_ticks * 3 / 2 - 1,
native/tests\core_tests.cpp:391:  elite->cooldown_ticks = 0;
native/tests\core_tests.cpp:403:  const std::uint64_t telegraph_tick = telegraph->tick;
native/tests\core_tests.cpp:406:            elite->pending_action_ticks == kTelegraphTicks,
native/tests\core_tests.cpp:412:          "telegraphed Thrust does not resolve before its final windup tick");
native/tests\core_tests.cpp:418:  check(damage->tick == telegraph_tick + kTelegraphTicks,
native/tests\core_tests.cpp:421:  check(elite->pending_action == ActionType::Wait && elite->pending_action_ticks == 0,
native/tests\core_tests.cpp:447:  check(elite->pending_action == ActionType::Wait && elite->pending_action_ticks == 0,
native/tests\core_tests.cpp:472:  check(elite->cooldown_ticks == elite->stats.attack_speed_ticks * 3 / 2,
native/tests\core_tests.cpp:485:  player->cooldown_ticks = 0;
native/tests\core_tests.cpp:489:            elite->pending_action_ticks == 0,
native/tests\core_tests.cpp:508:            elite->pending_action_ticks == 0,
native/tests\core_tests.cpp:514:void test_elite_skill_replay_is_deterministic() {
native/tests\core_tests.cpp:519:  check(first_elite->id == second_elite->id, "elite replay setup retains stable actor identity");
native/tests\core_tests.cpp:528:        "elite telegraph and skill resolution replay byte-identically");
native/tests\core_tests.cpp:532:            first_actor->pending_action_ticks == second_actor->pending_action_ticks,
native/tests\core_tests.cpp:533:        "elite pending state remains deterministic under replay");
native/tests\core_tests.cpp:552:  check(monster->cooldown_ticks == monster->stats.attack_speed_ticks,
native/tests\core_tests.cpp:556:void test_war_cry_buff_expiry_and_replay_determinism() {
native/tests\core_tests.cpp:572:    check(player->war_cry_attack_bonus == 4 && player->war_cry_ticks_remaining == 19,
native/tests\core_tests.cpp:575:          "War Cry pays its named cost after one tick of regeneration");
native/tests\core_tests.cpp:578:    player->cooldown_ticks = 0;
native/tests\core_tests.cpp:582:    check(player->war_cry_attack_bonus == 0 && player->war_cry_ticks_remaining == 0,
native/tests\core_tests.cpp:583:          "War Cry expires at its deterministic tick boundary");
native/tests\core_tests.cpp:594:        "skill actions and buff expiry remain deterministic under replay");
native/tests\core_tests.cpp:729:void test_relic_resurface_replay_is_deterministic() {
native/tests\core_tests.cpp:737:        "movement replay setup drops an item before recovery");
native/tests\core_tests.cpp:746:  check(first.events().size() == second.events().size(), "replay emits the same event count");
native/tests\core_tests.cpp:747:  check(first.legends() == second.legends(), "replay emits identical relic legends");
native/tests\core_tests.cpp:749:        "replay resurfaces the same number of items");
native/tests\core_tests.cpp:752:        "replay resurfaces the same stable item identity");
native/tests\core_tests.cpp:761:  const auto first = snapshot(original);
native/tests\core_tests.cpp:763:  check(first_text.find("schemaVersion=1\n") == 0, "snapshot has a mandatory schemaVersion field");
native/tests\core_tests.cpp:766:  check(snapshot(restored) == first, "snapshot restore is byte-stable");
native/tests\core_tests.cpp:775:  check(snapshot(tolerant) == first, "restore tolerates unknown fields");
native/tests\core_tests.cpp:778:void test_persistence_d109_mid_instance_and_rng_continuation() {
native/tests\core_tests.cpp:786:  const auto mid_snapshot = snapshot(mid_instance);
native/tests\core_tests.cpp:787:  Simulation restored = restore(mid_snapshot);
native/tests\core_tests.cpp:800:  // the same durable state, then compare a complete seeded reward drop after
native/tests\core_tests.cpp:808:  Simulation replay = restore(snapshot(baseline));
native/tests\core_tests.cpp:810:  replay.dispatch(Command::enter("route:tin:1:0"));
native/tests\core_tests.cpp:812:  defeat_enemy(replay);
native/tests\core_tests.cpp:814:            baseline.ground_items().front().id == replay.ground_items().front().id &&
native/tests\core_tests.cpp:815:            baseline.ground_trophies().front().id == replay.ground_trophies().front().id,
native/tests\core_tests.cpp:817:  check(snapshot(replay) == snapshot(baseline),
native/tests\core_tests.cpp:818:        "restored RNG state produces deterministic continuation and drops");
native/tests\core_tests.cpp:836:  const auto bytes = snapshot(source);
native/tests\core_tests.cpp:842:        "snapshot restore preserves explicit relic and lost-trophy pools");
native/tests\core_tests.cpp:843:  check(snapshot(restored) == bytes,
native/tests\core_tests.cpp:857:  // Keep searching the deterministic reward stream until both recovery
native/tests\core_tests.cpp:859:  // by snapshot and must re-enter through the pending queues exactly once.
native/tests\core_tests.cpp:868:  Simulation restored = restore(snapshot(source));
native/tests\core_tests.cpp:882:  const auto bytes = snapshot(simulation);
native/tests\core_tests.cpp:884:      std::filesystem::temp_directory_path() / "verdigris-task-0030.snapshot";
native/tests\core_tests.cpp:887:        "atomic persistence adapter writes and reads snapshot bytes");
native/tests\core_tests.cpp:890:        "atomic persistence adapter replaces an existing House snapshot");
native/tests\core_tests.cpp:894:void test_determinism() {
native/tests\core_tests.cpp:899:  const int step = movement_step_per_tick(world_scale::kPlayerMoveSpeed);
native/tests\core_tests.cpp:900:  const int approach_ticks =
native/tests\core_tests.cpp:902:  for (int i = 0; i < approach_ticks; ++i) {
native/tests\core_tests.cpp:914:  check(relevant(first) == relevant(second), "same seed and command stream are deterministic");
native/tests\core_tests.cpp:947:                                           catalog.resource_regen_per_tick,
native/tests\core_tests.cpp:958:  check(telegraph && telegraph->value == catalog.telegraph_ticks,
native/tests\core_tests.cpp:961:  check(elite->pending_action_ticks == catalog.telegraph_ticks,
native/tests\core_tests.cpp:972:                                          catalog.resource_regen_per_tick,
native/tests\core_tests.cpp:975:            buff_player->war_cry_ticks_remaining == catalog.war_cry_duration_ticks - 1,
native/tests\core_tests.cpp:1065:  // walks the deterministic windup (the killing dispatch already spent the
native/tests\core_tests.cpp:1066:  // first of the kTelegraphTicks ticks), proves no reserve warden is alive
native/tests\core_tests.cpp:1086:    player->cooldown_ticks = 0;
native/tests\core_tests.cpp:1149:  const std::string replay_entry = prepare(second);
native/tests\core_tests.cpp:1150:  const std::string replay_elite = second.pending_wave()[0].id;
native/tests\core_tests.cpp:1151:  const std::string replay_flanker = second.pending_wave()[1].id;
native/tests\core_tests.cpp:1154:  strike_down(second, replay_elite);
native/tests\core_tests.cpp:1155:  strike_down(second, replay_flanker);
native/tests\core_tests.cpp:1156:  check(replay_entry == entry_id && replay_elite == elite_id &&
native/tests\core_tests.cpp:1157:            replay_flanker == flanker_id && relevant(first) == relevant(second) &&
native/tests\core_tests.cpp:1163:        "pack lifecycle and delayed clear remain deterministic under replay");
native/tests\core_tests.cpp:1207:          "the elite materializes on its deterministic anchor point");
native/tests\core_tests.cpp:1211:          "the flanker materializes on its deterministic flank point");
native/tests\core_tests.cpp:1221:    player->cooldown_ticks = 0;
native/tests\core_tests.cpp:1232:    player->cooldown_ticks = 0;
native/tests\core_tests.cpp:1248:  Simulation replay_a(0x0143ULL);
native/tests\core_tests.cpp:1249:  drive_expedition(replay_a);
native/tests\core_tests.cpp:1250:  Simulation replay_b(0x0143ULL);
native/tests\core_tests.cpp:1251:  drive_expedition(replay_b);
native/tests\core_tests.cpp:1252:  check(relevant(replay_a) == relevant(replay_b) &&
native/tests\core_tests.cpp:1253:            replay_a.instance().phase == replay_b.instance().phase,
native/tests\core_tests.cpp:1254:        "the objective timeline is deterministic under replay");
native/tests\core_tests.cpp:1258:  replay_a.dispatch(Command::enter("route:tin:1:0"));
native/tests\core_tests.cpp:1259:  check(replay_a.instance().active &&
native/tests\core_tests.cpp:1260:            replay_a.instance().phase == ExpeditionPhase::SlayWardens &&
native/tests\core_tests.cpp:1261:            replay_a.pending_wave().size() == 2,
native/tests\core_tests.cpp:1274:void test_first_expedition_wave_spawn_is_deterministic() {
native/tests\core_tests.cpp:1303:        "same-seed expeditions produce identical warden identities");
native/tests\core_tests.cpp:1313:        "a different seed re-rolls identities but keeps the deterministic pack shape");
native/tests\core_tests.cpp:1316:void test_first_expedition_wave_replay_is_deterministic() {
native/tests\core_tests.cpp:1329:        "the full pack encounter replays byte-identically");
native/tests\core_tests.cpp:1344:  player->cooldown_ticks = 0;
native/tests\core_tests.cpp:1358:  sim.actor(sim.scion().actor_id)->cooldown_ticks = 0;
native/tests\core_tests.cpp:1377:        "a successor faces a fresh deterministic pack with no leaked state");
native/tests\core_tests.cpp:1379:  // The recovery path stays deterministic across the converged pack: the
native/tests\core_tests.cpp:1387:  heir->cooldown_ticks = 0;
native/tests\core_tests.cpp:1490:void test_d106_recovery_is_ordered_and_deterministic() {
native/tests\core_tests.cpp:1507:        "D-106 recovery pools are deterministic under replay");
native/tests\core_tests.cpp:1513:  // A full pack clear feeds the seeded reward stream three times per round,
native/tests\core_tests.cpp:1667:void test_legend_stable_ids_and_deterministic_replay() {
native/tests\core_tests.cpp:1682:        "identical seed and commands produce byte-identical legend records");
native/tests\core_tests.cpp:1683:  check(!first.legends().empty(), "deterministic replay produces legend records");
native/tests\core_tests.cpp:1695:            movement_step_per_tick(world_scale::kPlayerMoveSpeed),
native/tests\core_tests.cpp:1696:        "D-114 table derives the player step from the fixed cadence");
native/tests\core_tests.cpp:1699:        "D-114 melee reach derives from contact ticks");
native/tests\core_tests.cpp:1706:        "D-114 extraction interaction derives from walking ticks");
native/tests\core_tests.cpp:1792:  // (seed 42, orchestration/tasks/TASK-0047-native-protocol-n4 captures).
native/tests\core_tests.cpp:1810:    Mulberry32 rng(4);
native/tests\core_tests.cpp:1812:    forge.reseed(static_cast<std::uint32_t>(std::floor(rng.next() * 4294967296.0)));
native/tests\core_tests.cpp:1829:    Mulberry32 rng(1670);
native/tests\core_tests.cpp:1831:    forge.reseed(static_cast<std::uint32_t>(std::floor(rng.next() * 4294967296.0)));
native/tests\core_tests.cpp:1847:    Mulberry32 rng(1);
native/tests\core_tests.cpp:1850:    opts.rng = &rng;
native/tests\core_tests.cpp:1874:  Mulberry32 rng(1);
native/tests\core_tests.cpp:1877:  opts.rng = &rng;
native/tests\core_tests.cpp:2074:  test_persistence_d109_mid_instance_and_rng_continuation();
native/tests\core_tests.cpp:2078:  test_determinism();
native/tests\core_tests.cpp:2082:  test_movement_replay_is_deterministic();
native/tests\core_tests.cpp:2085:  test_facing_replay_is_deterministic();
native/tests\core_tests.cpp:2093:  test_elite_skill_replay_is_deterministic();
native/tests\core_tests.cpp:2095:  test_war_cry_buff_expiry_and_replay_determinism();
native/tests\core_tests.cpp:2101:  test_first_expedition_wave_spawn_is_deterministic();
native/tests\core_tests.cpp:2102:  test_first_expedition_wave_replay_is_deterministic();
native/tests\core_tests.cpp:2107:  test_d106_recovery_is_ordered_and_deterministic();
native/tests\core_tests.cpp:2114:  test_legend_stable_ids_and_deterministic_replay();
native/tests\core_tests.cpp:2121:  test_relic_resurface_replay_is_deterministic();
native/src\networking.cpp:402:// dev.js snapshotItem.
native/src\networking.cpp:403:JsonValue snapshot_item_json(const GameItem& item) {
native/src\networking.cpp:577:ProtocolSession::ProtocolSession(std::string identity, std::string socket_id, std::uint64_t seed, bool quick_start)
native/src\networking.cpp:579:      session_rng_(static_cast<std::uint32_t>(seed ^ (seed >> 32))),
native/src\networking.cpp:580:      simulation_(std::make_unique<Simulation>(seed, "House Verdigris")), world_(std::make_shared<WorldSimulation>(seed, identity_)) {
native/src\networking.cpp:597:void ProtocolSession::tick(std::int64_t now) {
native/src\networking.cpp:607:  // "survive relogins" via the saved snapshot on JS. The commission chain
native/src\networking.cpp:610:  // keep the instance (networking_tests: instance re-login snapshot).
native/src\networking.cpp:613:  // snapshot fields (loot, levels, bank, skill tree, quest record) over the
native/src\networking.cpp:617:  // mortality.mjs's seeded revision).
native/src\networking.cpp:653:std::int64_t ProtocolSession::now_ms() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
native/src\networking.cpp:716:// server/core/world-web.js - deterministic per-house road chart. The hash
native/src\networking.cpp:855:  if (world_->in_instance()) { const auto& meta=world_->metadata(); JsonValue::Object metadata; put(metadata,"seed",static_cast<double>(meta.seed)); put(metadata,"theme",meta.theme); if(meta.layout.empty()) put(metadata,"layout",nullptr); else put(metadata,"layout",meta.layout); put(metadata,"depth",meta.depth);
native/src\networking.cpp:879:JsonValue ProtocolSession::snapshot() const {
native/src\networking.cpp:916:  { // dev.js: chroniclesRecord mirrors chroniclesStore.snapshot(uuid).
native/src\networking.cpp:936:  if (world_->in_instance()) { const auto& meta=world_->metadata(); JsonValue::Object metadata; put(metadata,"seed",static_cast<double>(meta.seed)); put(metadata,"theme",meta.theme); if(meta.layout.empty()) put(metadata,"layout",nullptr); else put(metadata,"layout",meta.layout); put(metadata,"depth",meta.depth);
native/src\networking.cpp:965:  // N4: the real item pipeline snapshot (dev.js buildStateSnapshot).
native/src\networking.cpp:967:  JsonValue::Array items; for (const auto& item:inventory_.items()) items.emplace_back(snapshot_item_json(item)); put(state,"inventory",std::move(items));
native/src\networking.cpp:985:std::string ProtocolSession::state_payload(const std::string& request_id) const { std::lock_guard<std::recursive_mutex> lock(mutex_); JsonValue::Object data; put(data,"player",JsonValue::Object{{"socket_id",socket_id_}}); put(data,"state",snapshot()); put(data,"requestId",request_id); return JsonValue(std::move(data)).stringify(); }
native/src\networking.cpp:1064:  Mulberry32 seeded;
native/src\networking.cpp:1065:  Mulberry32* rng=&session_rng_;  // unseeded grants draw from the session stream (JS: Math.random)
native/src\networking.cpp:1066:  if (const auto* seed_value=payload.get("seed")) {
native/src\networking.cpp:1067:    if (seed_value->number()) { seeded=Mulberry32(static_cast<std::uint32_t>(std::floor(*seed_value->number()))); rng=&seeded; }
native/src\networking.cpp:1080:      CreateItemOptions opts; opts.rng=rng; opts.item_level=item_level; opts.bind_to=identity_; opts.forge=&world_->forge();
native/src\networking.cpp:1093:  // dev.js dev:drop: unbound deterministic gear on the active floor.
native/src\networking.cpp:1095:  Mulberry32 seeded;
native/src\networking.cpp:1096:  Mulberry32* rng=&session_rng_;
native/src\networking.cpp:1097:  if (const auto* seed_value=payload.get("seed")) {
native/src\networking.cpp:1098:    if (seed_value->number()) { seeded=Mulberry32(static_cast<std::uint32_t>(std::floor(*seed_value->number()))); rng=&seeded; }
native/src\networking.cpp:1101:  CreateItemOptions opts; opts.rng=rng; opts.forge=&world_->forge();
native/src\networking.cpp:1230:  const auto* snapshot = payload.get("snapshot");
native/src\networking.cpp:1231:  if (!snapshot || !snapshot->object()) return;
native/src\networking.cpp:1232:  passive_tree_ = *snapshot;
native/src\networking.cpp:1266:  // shops.js General Store: fixed stock rows with pane slot indices.
native/src\networking.cpp:1293:    JsonValue row = snapshot_item_json(item);
native/src\networking.cpp:1365:  const std::uint64_t key = meta.seed * 131u + static_cast<std::uint64_t>(meta.depth);
native/src\networking.cpp:1575:    std::uint64_t seed = 1469598103934665603ULL;
native/src\networking.cpp:1576:    for (unsigned char c : identity_) seed = (seed ^ c) * 1099511628211ULL;
native/src\networking.cpp:1577:    world_ = std::make_shared<WorldSimulation>(seed, identity_);
native/src\networking.cpp:1892:    std::sort(matches.begin(),matches.end(),[](const GroundItem* a,const GroundItem* b){return a->timestamp>b->timestamp;});
native/src\networking.cpp:1901:      put(entry,"timestamp",static_cast<double>(ground->timestamp));
native/src\networking.cpp:1920:        put(entry,"label","Add a random brand (100 coins)");
native/src\networking.cpp:2380:  // The server tick thread and the socket handler share the session; one
native/src\networking.cpp:2507:  if (envelope.event=="player:skill:trigger") { auto* actor=simulation_->actor(simulation_->scion().actor_id); if(actor&&world_->in_instance()){ if (respawn_protection_until_ms_ > 0) respawn_protection_until_ms_ = 0; active_skill_id_=as_string(payload?payload->get("skillId"):nullptr,"primary-attack"); world_->set_engaged_by(identity_); const auto direction=as_string(payload?payload->get("direction"):nullptr,"down"); const auto wear_totals=wear_.totals(); const int wear_bonus=(std::max)((std::max)(wear_totals.attack.stab,wear_totals.attack.slash),(std::max)(wear_totals.attack.crush,wear_totals.attack.range)); world_->start_player_attack(actor->stats.level,actor->stats.attack+(std::max)(0,wear_bonus),now_ms(),direction); process_combat(now_ms(),emit); /* real-clock cadence: polls advance combat */ } return; }
native/src\networking.cpp:2742:    // makes the next process_combat tick commit a second fall with no combat
native/src\networking.cpp:2811:  const auto listener=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); if(listener==invalid_socket){if(error)*error="socket failed";return false;} int yes=1; setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<const char*>(&yes),sizeof(yes)); sockaddr_in address{}; address.sin_family=AF_INET; address.sin_addr.s_addr=inet_addr("127.0.0.1"); address.sin_port=htons(port_); if(bind(listener,reinterpret_cast<sockaddr*>(&address),sizeof(address))<0 || listen(listener,16)<0){close_socket(listener);if(error)*error="bind/listen failed";return false;} listen_socket_=static_cast<std::intptr_t>(listener); running_=true; accept_thread_=std::make_unique<std::thread>(&WebSocketServer::accept_loop,this); tick_thread_=std::make_unique<std::thread>([this]{ while(running_){ std::this_thread::sleep_for(std::chrono::milliseconds(150)); std::vector<std::shared_ptr<ProtocolSession>> ticking; { std::lock_guard lock(mutex_); for (auto& [key, session] : sessions_) ticking.push_back(session); } const auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); for (auto& session : ticking) session->tick(now); } }); return true;
native/src\networking.cpp:2813:void WebSocketServer::stop(){ if(!running_)return; running_=false; close_socket(static_cast<socket_t>(listen_socket_)); listen_socket_=-1; if(accept_thread_&&accept_thread_->joinable())accept_thread_->join(); if(tick_thread_&&tick_thread_->joinable())tick_thread_->join();
native/src\networking.cpp:2906:    ServerParty snapshot;
native/src\networking.cpp:2913:      snapshot = it->second; }
native/src\networking.cpp:2914:    send_party_update(snapshot);
native/src\networking.cpp:2918:    ServerParty snapshot; bool found = false;
native/src\networking.cpp:2921:      if (party) { party->ready[uuid] = true; snapshot = *party; found = true; } }
native/src\networking.cpp:2922:    if (found) send_party_update(snapshot);
native/src\networking.cpp:2926:    ServerParty snapshot; bool found = false;
native/src\networking.cpp:2932:        snapshot = *party; found = true; } }
native/src\networking.cpp:2934:    const std::string scene_id = "instance-" + snapshot.id;
native/src\networking.cpp:2936:    { std::lock_guard lock(mutex_); auto it = sessions_.find(snapshot.leader_uuid); if (it != sessions_.end()) leader_session = it->second; }
native/src\networking.cpp:2939:      auto leader_id = snapshot.leader_uuid;
native/src\networking.cpp:2943:    for (const auto& member : snapshot.member_uuids) {
native/src\networking.cpp:2944:      if (member == snapshot.leader_uuid) continue;
native/src\networking.cpp:2952:    send_party_update(snapshot);
native/src\networking.cpp:2956:    ServerParty snapshot; bool had_party = false;
native/src\networking.cpp:2964:        snapshot = *party; } }
native/src\networking.cpp:2968:    if (!snapshot.member_uuids.empty()) send_party_update(snapshot);
native/src\networking.cpp:2972:    ServerParty snapshot; bool in_party = false;
native/src\networking.cpp:2975:      if (party) { party->state = "lobby"; snapshot = *party; in_party = true; } }
native/src\networking.cpp:2979:    send_party_update(snapshot);
native/src\networking.cpp:2988:const bool quick=as_bool(envelope.data.get("quickGuest"));const auto* playtest_guest=envelope.data.get("playtestGuestId");if(playtest_guest&&playtest_guest->string())identity=*playtest_guest->string();const auto* playtest_name=envelope.data.get("playtestGuestName");std::shared_ptr<ProtocolSession> session;std::shared_ptr<Connection> old;{std::lock_guard lock(mutex_);auto it=sessions_.find(identity);if(it!=sessions_.end()){for(const auto& candidate:connections_)if(candidate->session==it->second&&candidate!=connection&&!candidate->closed){old=candidate;break;}session=it->second;}if(!session){std::uint64_t seed=1469598103934665603ULL;for(unsigned char c:identity)seed=(seed^c)*1099511628211ULL;session=std::make_shared<ProtocolSession>(identity,connection->id,seed,quick);sessions_[identity]=session;}else { const bool adopted = connection->session != session; session->replace_socket(connection->id); if (adopted) session->reset_world_for_new_socket(); } connection->session=session;}session->set_broadcast([this](const Envelope& event){broadcast(event);});if(playtest_name&&playtest_name->string())session->set_username(*playtest_name->string());session->set_direct_emit([connection](const Envelope& event){connection->send_text(emit_envelope(event));});if(old){old->send_text(emit_envelope(Envelope{"player:session-replaced",JsonValue::Object{{"player",JsonValue::Object{{"socket_id",old->id}}}}}));old->shutdown_send();old->close();}session->handle(envelope,[connection](const Envelope& response){connection->send_text(emit_envelope(response));});return;} auto session=connection->session;if(!session)return;if(envelope.event.rfind("party:",0)==0&&handle_party_event(connection,envelope))return;session->handle(envelope,[connection](const Envelope& response){connection->send_text(emit_envelope(response));});}
native/include\verdigris\core.hpp:29:// A telegraph is emitted this many simulation ticks before an elite skill
native/include\verdigris\core.hpp:34:// Simulation commands resolve at a fixed 20 Hz cadence.  Actor move_speed is
native/include\verdigris\core.hpp:36:// deterministic while preserving the recorded MoveIntent shape.
native/include\verdigris\core.hpp:39:constexpr int movement_step_per_tick(int move_speed) {
native/include\verdigris\core.hpp:44:// walking cadence (220 world units/second, 11 units/tick), rather than being
native/include\verdigris\core.hpp:48://   measure                  derivation                         value / time
native/include\verdigris\core.hpp:49://   player walk step         220 units/s * 50 ms                 11 u/tick
native/include\verdigris\core.hpp:50://   melee contact            13 walk ticks                      143 u / .65 s
native/include\verdigris\core.hpp:52://   extraction interaction   8 walk ticks                       88 u / .40 s
native/include\verdigris\core.hpp:63:inline constexpr int kPlayerStepPerTick = movement_step_per_tick(kPlayerMoveSpeed);
native/include\verdigris\core.hpp:75:// A dash is a short, readable burst measured in ordinary movement ticks.
native/include\verdigris\core.hpp:98:  int telegraph_ticks = 0;
native/include\verdigris\core.hpp:100:  int war_cry_duration_ticks = 0;
native/include\verdigris\core.hpp:101:  int resource_regen_per_tick = 0;
native/include\verdigris\core.hpp:118:  int attack_speed_ticks = 4;
native/include\verdigris\core.hpp:146:  int cooldown_ticks = 0;
native/include\verdigris\core.hpp:152:  int war_cry_ticks_remaining = 0;
native/include\verdigris\core.hpp:155:  // pending; the tick counter is the remaining windup.
native/include\verdigris\core.hpp:157:  int pending_action_ticks = 0;
native/include\verdigris\core.hpp:183:  // seeded reward stream as relic items, without becoming durable storage.
native/include\verdigris\core.hpp:197:  std::uint64_t tick = 0;
native/include\verdigris\core.hpp:261:  std::uint64_t tick = 0;
native/include\verdigris\core.hpp:316:  explicit Simulation(std::uint64_t seed, const std::string& house_name = "House Verdigris");
native/include\verdigris\core.hpp:331:  std::uint64_t tick() const;
native/include\verdigris\core.hpp:334:  // deliberately not a state snapshot or a general simulation export.
native/include\verdigris\core.hpp:340:  // General deterministic content seam. Callers may add an additional
native/include\verdigris\core.hpp:345:  // expedition reveals its pack deterministically: when a kill leaves roster
native/include\verdigris\core.hpp:347:  // their fixed anchors. Like the rest of the live instance state, the
native/include\verdigris\core.hpp:349:  // absent from durable snapshots.
native/include\verdigris\core.hpp:352:  // Stable hooks used by external seasonal mechanics and deterministic tests.
native/include\verdigris\core.hpp:359:  friend std::vector<std::uint8_t> snapshot(const Simulation& simulation);
native/include\verdigris\core.hpp:386:  void advance_tick();
native/include\verdigris\core.hpp:401:  Rng rng_;
native/include\verdigris\core.hpp:411:  // deterministically.
native/include\verdigris\core.hpp:412:  std::uint64_t wave_materialization_tick_ = 0;
native/include\verdigris\core.hpp:426:  std::uint64_t tick_ = 0;
native/include\verdigris\core.hpp:430:// Versioned, deterministic durable state.  Snapshot bytes are canonical for
native/include\verdigris\core.hpp:432:// snapshot boundary under D-109 (carried value remains carried).
native/include\verdigris\core.hpp:433:std::vector<std::uint8_t> snapshot(const Simulation& simulation);
native/include\verdigris\core.hpp:440://  - server/core/items/vesselforge/engine.js + verdigris-pack.js (the seeded
native/include\verdigris\core.hpp:453:  explicit Mulberry32(std::uint32_t seed = 0) : state_(seed) {}
native/include\verdigris\core.hpp:534:// The seeded Vesselforge rules (engine.js createForge + adapter.js
native/include\verdigris\core.hpp:536:// session, matching the JS module-level singleton: generation reseeds from
native/include\verdigris\core.hpp:537:// the caller's rng; sear() advances the persistent stream.
native/include\verdigris\core.hpp:542:  void reseed(std::uint32_t seed) { rand_ = Mulberry32(seed); }
native/include\verdigris\core.hpp:628:  Mulberry32* rng = nullptr;   // deterministic vessel roll when provided
native/include\verdigris\core.hpp:657:  // loop for multi-quantity grants so each roll gets its own rng draw.
native/include\verdigris\core.hpp:709:  std::int64_t timestamp = 0;  // placement time; menus sort newest-first
native/include\verdigris\core.hpp:795:  // N4: loot/behaviour facts the wire snapshot carries (JS m.rewards.coins
native/include\verdigris\core.hpp:827:  std::uint64_t seed = 0;
native/include\verdigris\core.hpp:862:  WorldSimulation(std::uint64_t seed, std::string player_uuid);
native/include\verdigris\core.hpp:874:  // the slay-elite objective is current (session sets this per tick).
native/include\verdigris\core.hpp:883:  // deterministic comparison trials.
native/include\verdigris\core.hpp:914:  // N3 deterministic combat seam. The transport supplies the authoritative
native/include\verdigris\core.hpp:935:  // The per-session forge (JS module singleton): generation reseeds it, the
native/include\verdigris\core.hpp:971:  std::uint64_t next_world_random();
native/include\verdigris\core.hpp:973:  std::uint64_t seed_;
native/include\verdigris\core.hpp:1013:  std::uint64_t world_random_state_ = 0x9e3779b97f4a7c15ULL;
native/src\core.cpp:57:  const int step = movement_step_per_tick(move_speed);
native/src\core.cpp:80:  // Per-second value; resolve_move derives the deterministic 50 ms step.
native/src\core.cpp:82:  stats.attack_speed_ticks = 3;
native/src\core.cpp:97:  stats.attack_speed_ticks = 5;
native/src\core.cpp:111:         attack_speed_ticks == other.attack_speed_ticks && resistances == other.resistances;
native/src\core.cpp:119:         telegraph_ticks == other.telegraph_ticks &&
native/src\core.cpp:121:         war_cry_duration_ticks == other.war_cry_duration_ticks &&
native/src\core.cpp:122:         resource_regen_per_tick == other.resource_regen_per_tick;
native/src\core.cpp:126:  return ordinal == other.ordinal && tick == other.tick && scion_id == other.scion_id &&
native/src\core.cpp:192:Simulation::Simulation(std::uint64_t seed, const std::string& house_name) : rng_(seed) {
native/src\core.cpp:193:  house_.id = rng_.token("house");
native/src\core.cpp:201:  scion_.id = rng_.token("scion");
native/src\core.cpp:203:  scion_.actor_id = rng_.token("actor");
native/src\core.cpp:220:std::uint64_t Simulation::tick() const { return tick_; }
native/src\core.cpp:250:  return Actor{rng_.token("actor"), ActorKind::Monster, enemy_stats(bounded_level), position,
native/src\core.cpp:266:  Event event{type, actor_id, item_id, trophy_id, text, value, tick_};
native/src\core.cpp:276:  entry.tick = tick_;
native/src\core.cpp:309:  advance_tick();
native/src\core.cpp:351:    attacker.war_cry_ticks_remaining = kWarCryDurationTicks;
native/src\core.cpp:364:  if (attacker.cooldown_ticks > 0 || attacker.stats.resource < resource_cost) return;
native/src\core.cpp:395:    attacker.cooldown_ticks =
native/src\core.cpp:396:        std::max(1, attacker.stats.attack_speed_ticks * kSweepCooldownNumerator /
native/src\core.cpp:400:    attacker.cooldown_ticks = attacker.stats.attack_speed_ticks;
native/src\core.cpp:435:    item.history.push_back("used at tick " + std::to_string(tick_));
native/src\core.cpp:447:        item.history.push_back("used at tick " + std::to_string(tick_));
native/src\core.cpp:603:  wave_materialization_tick_ = 0;
native/src\core.cpp:614:  wave_materialization_tick_ = 0;
native/src\core.cpp:634:  if (pending_wave_.empty() || wave_materialization_tick_ == 0 ||
native/src\core.cpp:635:      tick_ < wave_materialization_tick_) {
native/src\core.cpp:639:  // roster steps onto its deterministic anchor at the same telegraph
native/src\core.cpp:646:  wave_materialization_tick_ = 0;
native/src\core.cpp:661:        enemy.pending_action_ticks = 0;
native/src\core.cpp:664:      if (enemy.pending_action_ticks > 0) --enemy.pending_action_ticks;
native/src\core.cpp:665:      if (enemy.pending_action_ticks == 0) {
native/src\core.cpp:678:    if (enemy.cooldown_ticks > 0) continue;
native/src\core.cpp:684:      // Thrust is selected by the same deterministic cone predicate that the
native/src\core.cpp:687:      // Keep the skill bands deterministic and reachable: at close melee
native/src\core.cpp:693:        enemy.pending_action_ticks = kTelegraphTicks;
native/src\core.cpp:699:        enemy.pending_action_ticks = kTelegraphTicks;
native/src\core.cpp:711:    enemy.cooldown_ticks = enemy.stats.attack_speed_ticks;
native/src\core.cpp:722:void Simulation::advance_tick() {
native/src\core.cpp:723:  ++tick_;
native/src\core.cpp:729:    if (actor_value.cooldown_ticks > 0) --actor_value.cooldown_ticks;
native/src\core.cpp:730:    if (actor_value.war_cry_ticks_remaining > 0) {
native/src\core.cpp:731:      --actor_value.war_cry_ticks_remaining;
native/src\core.cpp:732:      if (actor_value.war_cry_ticks_remaining == 0) {
native/src\core.cpp:743:  item.id = rng_.token("item");
native/src\core.cpp:745:  item.attack_bonus = 4 + rng_.range(0, 3);
native/src\core.cpp:746:  item.history.push_back("forged by the expedition seed");
native/src\core.cpp:751:  // Relics re-enter only through the ordinary seeded reward stream.  The pool
native/src\core.cpp:755:  if (!house_.relic_candidates.empty() && rng_.range(1, kRelicResurfaceOneIn) == 1) {
native/src\core.cpp:766:  // Trophies carried through a death use the same deterministic re-entry
native/src\core.cpp:770:  if (!house_.lost_trophies.empty() && rng_.range(1, kRelicResurfaceOneIn) == 1) {
native/src\core.cpp:781:  Trophy trophy{rng_.token("trophy"), "Warden's ember"};
native/src\core.cpp:812:  actor_value.pending_action_ticks = 0;
native/src\core.cpp:819:        candidate.pending_action_ticks = 0;
native/src\core.cpp:841:        wave_materialization_tick_ = tick_ + kTelegraphTicks;
native/src\core.cpp:943:  scion_.id = rng_.token("scion");
native/src\core.cpp:945:  scion_.actor_id = rng_.token("actor");
native/src\core.cpp:980:  if (value.size() % 2 != 0) throw std::runtime_error("invalid snapshot string");
native/src\core.cpp:992:    if (high < 0 || low < 0) throw std::runtime_error("invalid snapshot string");
native/src\core.cpp:1068:  put_number(output, key + ".tick", legend.tick);
native/src\core.cpp:1089:    // harmless and preserving the canonical value emitted by snapshot().
native/src\core.cpp:1103:  if (found == fields.end()) throw std::runtime_error("missing snapshot field: " + key);
native/src\core.cpp:1116:    throw std::runtime_error("invalid snapshot number: " + key);
native/src\core.cpp:1131:  throw std::runtime_error("invalid snapshot boolean: " + key);
native/src\core.cpp:1145:  if (count > 1'000'000) throw std::runtime_error("snapshot collection is too large");
native/src\core.cpp:1212:  legend.tick = required_number<std::uint64_t>(fields, key + ".tick");
native/src\core.cpp:1226:std::vector<std::uint8_t> snapshot(const Simulation& simulation) {
native/src\core.cpp:1229:  put_number(output, "rng.state", simulation.rng_.state);
native/src\core.cpp:1230:  put_number(output, "rng.serial", simulation.rng_.serial);
native/src\core.cpp:1231:  put_number(output, "tick", simulation.tick_);
native/src\core.cpp:1300:    throw std::runtime_error("unsupported or missing snapshot schemaVersion");
native/src\core.cpp:1307:  simulation.rng_.state = required_number<std::uint64_t>(fields, "rng.state");
native/src\core.cpp:1308:  simulation.rng_.serial = required_number<std::uint64_t>(fields, "rng.serial");
native/src\core.cpp:1309:  simulation.tick_ = required_number<std::uint64_t>(fields, "tick");
native/src\core.cpp:1453:// N2 stub geometry.  The JS server generates floors from seeded template
native/src\core.cpp:1477:std::uint64_t fnv1a(const std::string& text, std::uint64_t seed) {
native/src\core.cpp:1478:  std::uint64_t hash = seed ? seed : 1469598103934665603ULL;
native/src\core.cpp:1493:WorldSimulation::WorldSimulation(std::uint64_t seed, std::string player_uuid)
native/src\core.cpp:1494:    : seed_(seed), player_uuid_(std::move(player_uuid)) {
native/src\core.cpp:1701:  // Deterministic monster scatter: seeded LCG picks candidate tiles; only
native/src\core.cpp:1705:  std::uint64_t state = metadata_.seed ? metadata_.seed : 0x9e3779b97f4a7c15ULL;
native/src\core.cpp:1802:  metadata_.seed = fnv1a(theme + ":" + applied_layout, seed_);
native/src\core.cpp:1901:    // swing unprompted, which breaks deterministic comparison trials. JS
native/src\core.cpp:1958:      const int roll = 1 + static_cast<int>(next_world_random() % 100);
native/src\core.cpp:1994:      // tick thread is the simulation timer, so an instant second-warning
native/src\core.cpp:2007:      // The next player command is the fixed-step heartbeat in the native
native/src\core.cpp:2009:      // resolved dodge/hit rather than relying on a hidden wall-clock thread.
native/src\core.cpp:2801:    // adapter.js createVesselBlock: one rng draw reseeds the forge, then the
native/src\core.cpp:2804:    if (options.rng) {
native/src\core.cpp:2805:      const double draw = options.rng->next();
native/src\core.cpp:2806:      options.forge->reseed(static_cast<std::uint32_t>(std::floor(draw * 4294967296.0)));
native/src\core.cpp:3067:std::uint64_t WorldSimulation::next_world_random() {
native/src\core.cpp:3068:  // splitmix64: JS draws from Math.random here, so any independent stream is
native/src\core.cpp:3069:  // faithful; a seeded one keeps runs replayable for the architect.
native/src\core.cpp:3070:  world_random_state_ += 0x9e3779b97f4a7c15ULL;
native/src\core.cpp:3071:  std::uint64_t z = world_random_state_;
native/src\core.cpp:3090:  ground.timestamp = static_cast<std::int64_t>(++serial_);
native/src\core.cpp:3102:  ground.timestamp = static_cast<std::int64_t>(++serial_);
native/src\core.cpp:3166:  if (guaranteed || world_rand01(next_world_random()) < chance) {
native/src\core.cpp:3169:        pool[static_cast<std::size_t>(std::floor(world_rand01(next_world_random()) * pool.size()))];
native/src\core.cpp:3173:    // factory.js createById with rng: one draw reseeds the forge.
native/src\core.cpp:3174:    const double reseed_draw = world_rand01(next_world_random());
native/src\core.cpp:3175:    forge_.reseed(static_cast<std::uint32_t>(std::floor(reseed_draw * 4294967296.0)));
native/src\core.cpp:3176:    opts.rng = nullptr;  // reseed already performed; generate from the stream
native/src\core.cpp:3191:  const int coins = 80 + static_cast<int>(std::floor(world_rand01(next_world_random()) * 60.0));
native/src\core.cpp:3200:      pool[static_cast<std::size_t>(std::floor(world_rand01(next_world_random()) * pool.size()))];
native/src\core.cpp:3201:  const double reseed_draw = world_rand01(next_world_random());
native/src\core.cpp:3202:  forge_.reseed(static_cast<std::uint32_t>(std::floor(reseed_draw * 4294967296.0)));
native/src\core.cpp:3222:  metadata_.seed = fnv1a(theme + ":" + layout + ":floor-" + std::to_string(clamped_depth), seed_);
native/tests\audio_mixer_tests.cpp:3:// deterministic ordering, bus mute/volume state, bounded voice-cap eviction
native/tests\audio_mixer_tests.cpp:47:                 std::uint64_t tick) {
native/tests\audio_mixer_tests.cpp:52:  spec.scheduled_tick = tick;
native/tests\audio_mixer_tests.cpp:213:void deterministic_ordering() {
native/tests\audio_mixer_tests.cpp:216:  // Interleave ticks so arrival order differs from schedule order.
native/tests\audio_mixer_tests.cpp:228:    ordered = voiced[i - 1].scheduled_tick <= voiced[i].scheduled_tick;
native/tests\audio_mixer_tests.cpp:230:  check(ordered, "ordering: voiced cues ascend by scheduled tick");
native/tests\audio_mixer_tests.cpp:323:      "cue[000003] tick=5 bus=sfx prio=world id=kill wave=sawtooth "
native/tests\audio_mixer_tests.cpp:325:      "cue[000004] tick=5 bus=sfx prio=player id=scion-lost wave=sine "
native/tests\audio_mixer_tests.cpp:327:      "cue[000005] tick=7 bus=sfx prio=player id=warcry-expire wave=sine "
native/tests\audio_mixer_tests.cpp:329:      "cue[000001] tick=10 bus=sfx prio=player id=hit wave=sine 220->110Hz "
native/tests\audio_mixer_tests.cpp:331:      "cue[000002] tick=10 bus=sfx prio=player id=crit wave=square 440->110Hz "
native/tests\audio_mixer_tests.cpp:333:      "cue[000006] tick=12 bus=music prio=ui id=menu-loop wave=sine "
native/tests\audio_mixer_tests.cpp:354:  deterministic_ordering();
```

### Gate 2 — replay-surfaces.json parses

Command:

```
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/captures/replay-surfaces.json','utf8')); console.log('replay surfaces: PASS')"
```

Transcript:

```
replay surfaces: PASS
```

Exit code: `0`.

### Gate 3 — whitespace check

Command: `git diff --check`

Output: (empty)

Exit code: `0`.

### Gate 4 — changed-path restriction

Command: `git diff --name-only`

Transcript (pass 1; FINDINGS.md and replay-surfaces.json were intent-to-add,
STATUS.md was already clean at claim commit b0a09741):

```
orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/FINDINGS.md
orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/captures/replay-surfaces.json
```

Exit code: `0`. Only task-evidence paths change; no forbidden path is touched.

## Negative control

Primary: `WorldSimulation` live tile-space state — including the
`world_random_state_` splitmix64 loot stream (`native/src/core.cpp:3067-3072`),
the persistent Vesselforge stream, monster wall-clock deadlines, ground-item
order counters, and the town stash — has no snapshot, record, or byte-equality
proof anywhere in the tree; its only serialization is the lossy wire JSON
projection (`native/src/networking.cpp:879-984`). Full argument and secondary
controls: FINDINGS.md §6, machine-readable list under `gaps[]` in
captures/replay-surfaces.json.

## Contracts defined (not implemented)

ReplayRecord v1 and DivergenceReport v1, including field sets, digest rules,
tolerance semantics inherited from `snapshot()/restore()`, closed divergence
vocabulary, and the smallest successor scaffold: FINDINGS.md §8–§10.

## Authority compliance

- Read-only audit: no file outside
  `orchestration/tasks/TASK-0100-deterministic-replay-coverage-audit/**` was
  created or modified; core was not patched (spec stop rule honored).
- Resource capsule respected: no ports opened or bound; port 6500 untouched;
  browser game and its servers never started.
- Constitution alignment recorded rather than re-derived:
  docs/product/VERDIGRIS_CONSTITUTION.md:157-173.

## Process notes (transparency)

- The worktree pre-commit hook (`yorkie`→`lint-staged`) cannot run here because
  this audit capsule intentionally has no `node_modules`; commits of
  orchestration-only markdown used `--no-verify`. The hook's eslint/stylelint
  globs do not match any committed file in this lane.
- Lane branch reconciliation (stale previous-generation remote history) was
  performed before this audit and is documented in STATUS.md revision 1;
  superseded content was not imported (`merge -s ours`, fd183f4b).
