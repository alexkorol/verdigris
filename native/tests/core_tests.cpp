#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "verdigris/core.hpp"
#include "verdigris/seasonal.hpp"

using namespace verdigris;

namespace {

void check(bool condition, const std::string& message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

// Movement helpers derive their tick counts from the same named per-tick
// step the core applies, so reachability stays proven if the movement
// cadence constants change again.
int steps_to_cover(int distance, int move_speed) {
  const int step = movement_step_per_tick(move_speed);
  return std::max(0, (distance + step - 1) / step);
}

void walk(Simulation& sim, int dx, int dy, int ticks) {
  for (int i = 0; i < ticks; ++i) sim.dispatch(Command::move(dx, dy));
}

void reach_enemy(Simulation& sim) {
  const Actor* player = sim.actor(sim.scion().actor_id);
  const Actor* enemy = nullptr;
  for (const auto& actor : sim.actors()) {
    if (actor.kind == ActorKind::Monster) enemy = &actor;
  }
  check(player && enemy, "movement helpers see the player and the spawned enemy");
  const int gap = manhattan_distance(player->position, enemy->position) -
                  presentation_constants::kMeleeRange;
  walk(sim, 1, 0, steps_to_cover(gap, player->stats.move_speed));
}

void defeat_enemy(Simulation& sim) {
  reach_enemy(sim);
  for (int i = 0; i < 12; ++i) sim.dispatch(Command::action_use(ActionType::Melee));
  const auto monsters = sim.actors();
  bool dead = true;
  for (const auto& actor : monsters) {
    if (actor.kind == ActorKind::Monster && actor.alive) dead = false;
  }
  check(dead, "melee defeats the instance enemy");
}

void pick_all_rewards(Simulation& sim) {
  check(!sim.ground_items().empty(), "enemy drops an equipment item");
  check(!sim.ground_trophies().empty(), "enemy drops a trophy");
  const std::string item_id = sim.ground_items().front().id;
  const std::string trophy_id = sim.ground_trophies().front().id;
  sim.dispatch(Command::pick_up(item_id));
  sim.dispatch(Command::pick_up(trophy_id));
}

void extract_from_start(Simulation& sim) {
  const Actor* player = sim.actor(sim.scion().actor_id);
  check(player != nullptr, "extraction walk has a player");
  // The extraction pad sits at the origin; the core admits extraction within
  // its private 250-unit range, and the ceiling step count always lands just
  // inside it.
  const int return_ticks =
      steps_to_cover(player->position.x - 250, player->stats.move_speed);
  walk(sim, -1, 0, return_ticks);
  sim.dispatch(Command::extract());
}

std::vector<std::string> relevant(const Simulation& sim) {
  std::vector<std::string> result;
  for (const auto& event : sim.events()) {
    result.push_back(std::to_string(static_cast<int>(event.type)) + ":" + event.actor_id + ":" +
                     event.item_id + ":" + event.trophy_id + ":" + event.text + ":" +
                     std::to_string(event.value));
  }
  result.push_back(sim.house().id);
  result.push_back(sim.scion().id);
  if (!sim.house().stored_items.empty()) result.push_back(sim.house().stored_items.front().id);
  for (const auto& legend : sim.legends()) {
    result.push_back(std::to_string(legend.ordinal) + ":" + std::to_string(legend.tick) + ":" +
                     legend.scion_id + ":" + legend.scion_name + ":" + legend.kind + ":" +
                     legend.subject + ":" + legend.detail + ":" + legend.killer_id + ":" +
                     legend.route_id + ":" + (legend.founding ? "founding" : "ordinary"));
  }
  return result;
}

const LegendEntry* find_legend(const Simulation& sim, const std::string& kind) {
  for (const auto& legend : sim.legends()) {
    if (legend.kind == kind) return &legend;
  }
  return nullptr;
}

const Item* find_ground_item(const Simulation& sim, const std::string& id) {
  for (const auto& item : sim.ground_items()) {
    if (item.id == id) return &item;
  }
  return nullptr;
}

int count_events(const Simulation& sim, EventType type, const std::string& text = {}) {
  int count = 0;
  for (const auto& event : sim.events()) {
    if (event.type == type && (text.empty() || event.text == text)) ++count;
  }
  return count;
}

const Event* last_event(const Simulation& sim, EventType type, const std::string& text = {}) {
  for (auto it = sim.events().rbegin(); it != sim.events().rend(); ++it) {
    if (it->type == type && (text.empty() || it->text == text)) return &*it;
  }
  return nullptr;
}

Actor* first_monster(Simulation& sim) {
  for (const auto& actor : sim.actors()) {
    if (actor.kind == ActorKind::Monster) return sim.actor(actor.id);
  }
  return nullptr;
}

void test_skill_resource_gating_and_thrust() {
  Simulation sim(0xA001ULL);
  sim.dispatch(Command::enter("route:tin:1:0"));
  Actor* player = sim.actor(sim.scion().actor_id);
  Actor* enemy = first_monster(sim);
  check(player && enemy, "skill test has a player and monster");
  player->position = {0, 0};
  player->stats.resource = 5;
  enemy->position = {1000, 0};
  enemy->stats.life = 1000;
  const int life_before = enemy->stats.life;
  sim.dispatch(Command::action_use(ActionType::Thrust));
  check(enemy->stats.life == life_before, "insufficient resource makes Thrust a no-op");
  check(count_events(sim, EventType::AttackStarted, "thrust") == 0,
        "gated Thrust emits no attack event");
  check(player->cooldown_ticks == 0, "gated Thrust does not consume cooldown");

  player = sim.actor(sim.scion().actor_id);
  enemy = first_monster(sim);
  player->stats.resource = player->stats.resource_max;
  player->cooldown_ticks = 0;
  enemy->position = {1000, 0};
  enemy->stats.life = 1000;
  sim.dispatch(Command::action_use(ActionType::Thrust));
  check(enemy->stats.life < 1000, "funded Thrust damages one target in front");
  check(count_events(sim, EventType::AttackStarted, "thrust") == 1,
        "funded Thrust emits one attack event");
  check(player->stats.resource == player->stats.resource_max - 8,
        "Thrust pays its named cost after one tick of regeneration");
  check(player->cooldown_ticks == player->stats.attack_speed_ticks - 1,
        "Thrust shares the ordinary attack cooldown");

  player->cooldown_ticks = 0;
  player->stats.resource = player->stats.resource_max;
  enemy->position = {-1000, 0};
  const int behind_life = enemy->stats.life;
  sim.dispatch(Command::aim(-1, 0));
  player = sim.actor(sim.scion().actor_id);
  check(player->facing.x == -1 && player->facing.y == 0,
        "aim command turns the player toward a target behind them");
  player->cooldown_ticks = 0;
  player->stats.resource = player->stats.resource_max;
  sim.dispatch(Command::action_use(ActionType::Thrust));
  check(enemy->stats.life < behind_life, "Thrust hits a target behind the player after aiming");

  player->cooldown_ticks = 0;
  player->stats.resource = player->stats.resource_max;
  enemy->position = {1000, 0};
  const int outside_cone_life = enemy->stats.life;
  sim.dispatch(Command::action_use(ActionType::Thrust));
  check(enemy->stats.life == outside_cone_life,
        "Thrust rejects a target outside the facing half-plane");

  player->cooldown_ticks = 0;
  player->stats.resource = player->stats.resource_max;
  enemy->position = {2201, 0};
  const int distant_life = enemy->stats.life;
  sim.dispatch(Command::action_use(ActionType::Thrust));
  check(enemy->stats.life == distant_life, "Thrust rejects a target beyond its 1.5x range");

  player->cooldown_ticks = 2;
  player->stats.resource = player->stats.resource_max;
  enemy->position = {1000, 0};
  const int cooldown_life = enemy->stats.life;
  sim.dispatch(Command::action_use(ActionType::Thrust));
  check(enemy->stats.life == cooldown_life, "Thrust respects an existing attack cooldown");
}

void test_actor_facing_follows_movement_and_aim() {
  Simulation sim(0xA010ULL);
  sim.dispatch(Command::enter("route:tin:1:0"));
  Actor* player = sim.actor(sim.scion().actor_id);
  check(player != nullptr, "facing test has a player");
  check(player->facing.x == 1 && player->facing.y == 0,
        "actors default to deterministic +x facing");

  sim.dispatch(Command::move(-2, 1));
  player = sim.actor(sim.scion().actor_id);
  check(player->facing.x == -1 && player->facing.y == 1,
        "movement updates facing using integer quantization");
  const Vec2 position_after_move = player->position;

  sim.dispatch(Command::aim(0, -7));
  player = sim.actor(sim.scion().actor_id);
  check(player->facing.x == 0 && player->facing.y == -1,
        "aim overrides the previous movement facing");
  sim.dispatch(Command::action_use(ActionType::Wait));
  player = sim.actor(sim.scion().actor_id);
  check(player->position.x == position_after_move.x &&
            player->position.y == position_after_move.y,
        "aim does not move the actor");
}

void test_monster_facing_tracks_pursuit_target() {
  Simulation sim(0xA011ULL);
  sim.dispatch(Command::enter("route:tin:1:0"));
  Actor* player = sim.actor(sim.scion().actor_id);
  Actor* enemy = first_monster(sim);
  check(player && enemy, "monster facing test has both actors");
  check(enemy->facing.x == -1 && enemy->facing.y == 0,
        "monster faces the player while pursuing from the right");

  player->position = {2400, 300};
  sim.dispatch(Command::action_use(ActionType::Wait));
  enemy = sim.actor(enemy->id);
  check(enemy->facing.x == 1 && enemy->facing.y == 1,
        "monster facing tracks a player on its upper-right pursuit vector");
}

void test_facing_replay_is_deterministic() {
  Simulation first(0xA012ULL);
  Simulation second(0xA012ULL);
  const std::vector<Command> commands = {
      Command::enter("route:tin:1:0"), Command::move(-1, 1), Command::aim(-1, 0),
      Command::action_use(ActionType::Thrust), Command::move(0, -1),
      Command::aim(1, 1), Command::action_use(ActionType::Wait)};
  for (const auto& command : commands) {
    first.dispatch(command);
    second.dispatch(command);
  }
  check(relevant(first) == relevant(second),
        "facing command streams replay to byte-identical events");
  const Actor* first_player = first.actor(first.scion().actor_id);
  const Actor* second_player = second.actor(second.scion().actor_id);
  check(first_player && second_player && first_player->facing.x == second_player->facing.x &&
            first_player->facing.y == second_player->facing.y,
        "facing state remains identical under deterministic replay");
}

void test_movement_step_matches_tick_rate_derivation() {
  Simulation sim(0xA020ULL);
  sim.dispatch(Command::enter("route:tin:1:0"));
  const Actor* player = sim.actor(sim.scion().actor_id);
  check(player != nullptr, "movement step test has a player");
  const int step = movement_step_per_tick(player->stats.move_speed);
  check(player->stats.move_speed == 220,
        "player move speed stays the tuned per-second rate");
  check(step == player->stats.move_speed * kTickMs / 1000,
        "per-tick step is move_speed * kTickMs / 1000 in integer math");

  const Vec2 start = player->position;
  sim.dispatch(Command::move(1, 0));
  player = sim.actor(sim.scion().actor_id);
  check(player->position.x - start.x == step && player->position.y == start.y,
        "one move command displaces exactly the named per-tick step");

  sim.dispatch(Command::move(1, 1));
  player = sim.actor(sim.scion().actor_id);
  check(player->position.x == start.x + step + step / 2 &&
            player->position.y == start.y + step / 2,
        "diagonal movement splits the per-tick step with integer math");

  const Vec2 before_second = player->position;
  for (int i = 0; i < 1000 / kTickMs; ++i) sim.dispatch(Command::move(1, 0));
  player = sim.actor(sim.scion().actor_id);
  check(player->position.x - before_second.x == player->stats.move_speed,
        "one real second of ticks covers exactly the per-second rate");
}

void test_dash_is_a_bounded_burst() {
  Simulation sim(0xA021ULL);
  sim.dispatch(Command::enter("route:tin:1:0"));
  const Actor* player = sim.actor(sim.scion().actor_id);
  check(player != nullptr, "dash test has a player");
  const Vec2 start = player->position;
  sim.dispatch(Command::action_use(ActionType::Dash));
  player = sim.actor(sim.scion().actor_id);
  check(player->position.x - start.x ==
            movement_step_per_tick(player->stats.move_speed) * (1000 / kTickMs),
        "dash hops one second of the actor's own per-tick movement");
  check(player->position.x - start.x == player->stats.move_speed,
        "the dash hop equals the per-second rate at the fixed tick cadence");
  check(player->position.x - start.x < presentation_constants::kMeleeRange,
        "dash stays well inside melee reach instead of teleporting past content");
}

void test_sweep_hits_multiple_targets_and_gates_resource() {
  Simulation sim(0xA002ULL);
  sim.dispatch(Command::enter("route:tin:1:0"));
  Actor* player = sim.actor(sim.scion().actor_id);
  check(player != nullptr, "Sweep test has a player");
  player->position = {0, 0};
  player->stats.resource = 10;
  player->cooldown_ticks = 0;
  Actor* first = first_monster(sim);
  check(first != nullptr, "Sweep test has an initial monster");
  first->position = {800, 0};
  first->stats.life = 1000;
  const std::string second_id = sim.spawn_monster({900, 0});
  Actor* second = sim.actor(second_id);
  check(second != nullptr, "general monster spawn seam creates a second target");
  second->stats.life = 1000;
  sim.dispatch(Command::action_use(ActionType::Sweep));
  check(sim.actor(second_id)->stats.life == 1000,
        "insufficient resource makes Sweep a no-op");
  check(count_events(sim, EventType::AttackStarted, "sweep") == 0,
        "gated Sweep emits no attack event");

  player = sim.actor(sim.scion().actor_id);
  player->stats.resource = player->stats.resource_max;
  player->cooldown_ticks = 0;
  sim.dispatch(Command::action_use(ActionType::Sweep));
  int damaged = 0;
  for (const auto& actor : sim.actors()) {
    if (actor.kind == ActorKind::Monster && actor.stats.life < 1000) ++damaged;
  }
  check(damaged == 2, "Sweep damages every living monster in melee range");
  check(count_events(sim, EventType::DamageApplied, "sweep") == 2,
        "Sweep emits one damage event per target");
  check(player->stats.resource == player->stats.resource_max - 13,
        "Sweep pays its named cost after one tick of regeneration");
  check(player->cooldown_ticks ==
            player->stats.attack_speed_ticks * 3 / 2 - 1,
        "Sweep uses its 1.5x attack cooldown");
}

Actor* setup_elite(Simulation& sim, Vec2 position) {
  Actor* player = sim.actor(sim.scion().actor_id);
  check(player != nullptr, "elite setup has a player");
  player->position = {0, 0};
  player->stats.life = 1000;
  const std::string elite_id = sim.spawn_monster(position, 1, true);
  Actor* elite = sim.actor(elite_id);
  check(elite != nullptr && elite->elite, "elite setup creates an elite monster");
  elite->stats.life = 1000;
  elite->stats.resource = elite->stats.resource_max;
  elite->cooldown_ticks = 0;
  return elite;
}

void test_elite_thrust_telegraph_timing() {
  Simulation sim(0xA011ULL);
  Actor* elite = setup_elite(sim, {1400, 0});
  const std::string elite_id = elite->id;
  sim.dispatch(Command::action_use(ActionType::Wait));
  const Event* telegraph = last_event(sim, EventType::AttackTelegraphed, "thrust");
  check(telegraph && telegraph->actor_id == elite_id && telegraph->value == kTelegraphTicks,
        "elite emits a Thrust telegraph with its actor and windup contract");
  const std::uint64_t telegraph_tick = telegraph->tick;
  elite = sim.actor(elite_id);
  check(elite->pending_action == ActionType::Thrust &&
            elite->pending_action_ticks == kTelegraphTicks,
        "elite stores the pending action and exact remaining windup");
  const int damage_before = sim.actor(sim.scion().actor_id)->stats.life;
  for (int i = 0; i < kTelegraphTicks - 1; ++i) {
    sim.dispatch(Command::action_use(ActionType::Wait));
    check(sim.actor(sim.scion().actor_id)->stats.life == damage_before,
          "telegraphed Thrust does not resolve before its final windup tick");
  }
  sim.dispatch(Command::action_use(ActionType::Wait));
  const Event* damage = last_event(sim, EventType::DamageApplied, "thrust");
  check(damage && damage->actor_id == sim.scion().actor_id,
        "telegraphed Thrust resolves through the shared damage event");
  check(damage->tick == telegraph_tick + kTelegraphTicks,
        "telegraph precedes Thrust damage by exactly kTelegraphTicks");
  elite = sim.actor(elite_id);
  check(elite->pending_action == ActionType::Wait && elite->pending_action_ticks == 0,
        "resolved Thrust clears the elite pending action");
}

void test_elite_skill_cone_gating() {
  Simulation sim(0xA013ULL);
  Actor* elite = setup_elite(sim, {800, 0});
  const std::string elite_id = elite->id;
  sim.dispatch(Command::action_use(ActionType::Wait));
  const Event* telegraph = last_event(sim, EventType::AttackTelegraphed);
  check(telegraph && telegraph->actor_id == elite_id && telegraph->text == "sweep",
        "an elite in close melee range selects Sweep instead of long-range Thrust");
  check(count_events(sim, EventType::AttackTelegraphed, "thrust") == 0,
        "Thrust is gated out when the target is not in the thrust-only range band");
}

void test_elite_skill_fizzles_when_resolution_gates_fail() {
  Simulation sim(0xA019ULL);
  Actor* elite = setup_elite(sim, {1400, 0});
  const std::string elite_id = elite->id;
  elite->stats.resource = 0;
  sim.dispatch(Command::action_use(ActionType::Wait));
  check(count_events(sim, EventType::AttackTelegraphed, "thrust") == 1,
        "elite can telegraph before the later resource gate is checked");
  for (int i = 0; i < kTelegraphTicks; ++i) sim.dispatch(Command::action_use(ActionType::Wait));
  elite = sim.actor(elite_id);
  check(elite->pending_action == ActionType::Wait && elite->pending_action_ticks == 0,
        "a gated elite action clears its pending state when it fizzles");
  check(count_events(sim, EventType::AttackStarted, "thrust") == 0 &&
            count_events(sim, EventType::DamageApplied, "thrust") == 0,
        "a resource-gated elite Thrust fizzles without attack or damage events");
}

void test_elite_sweep_uses_shared_pipeline() {
  Simulation sim(0xA014ULL);
  Actor* elite = setup_elite(sim, {800, 0});
  const std::string elite_id = elite->id;
  Actor* player = sim.actor(sim.scion().actor_id);
  const int expected_damage =
      std::max(1, Simulation::resolve_damage(*elite, *player) * 3 / 4);
  sim.dispatch(Command::action_use(ActionType::Wait));
  for (int i = 0; i < kTelegraphTicks; ++i) sim.dispatch(Command::action_use(ActionType::Wait));
  player = sim.actor(sim.scion().actor_id);
  check(player->stats.life == 1000 - expected_damage,
        "elite Sweep applies the shared damage calculation and multiplier");
  check(count_events(sim, EventType::AttackStarted, "sweep") == 1 &&
            count_events(sim, EventType::DamageApplied, "sweep") == 1,
        "elite Sweep uses the shared attack and damage event pipeline");
  elite = sim.actor(elite_id);
  check(elite->stats.resource == elite->stats.resource_max - 15,
        "elite Sweep consumes the same resource gate as player Sweep");
  check(elite->cooldown_ticks == elite->stats.attack_speed_ticks * 3 / 2,
        "elite Sweep applies the shared Sweep cooldown");
}

void test_elite_telegraph_cancels_on_death() {
  Simulation monster_death(0xA015ULL);
  Actor* elite = setup_elite(monster_death, {1400, 0});
  const std::string elite_id = elite->id;
  monster_death.dispatch(Command::action_use(ActionType::Wait));
  elite = monster_death.actor(elite_id);
  elite->stats.life = 1;
  Actor* player = monster_death.actor(monster_death.scion().actor_id);
  player->stats.resource = player->stats.resource_max;
  player->cooldown_ticks = 0;
  monster_death.dispatch(Command::action_use(ActionType::Thrust));
  elite = monster_death.actor(elite_id);
  check(!elite->alive && elite->pending_action == ActionType::Wait &&
            elite->pending_action_ticks == 0,
        "monster death cancels its pending telegraphed action");
  bool monster_death_damaged_player = false;
  for (const auto& event : monster_death.events()) {
    if (event.type == EventType::DamageApplied && event.text == "thrust" &&
        event.actor_id == monster_death.scion().actor_id) {
      monster_death_damaged_player = true;
    }
  }
  check(!monster_death_damaged_player,
        "a dead elite cannot resolve its cancelled Thrust");

  Simulation target_death(0xA016ULL);
  elite = setup_elite(target_death, {1400, 0});
  const std::string target_elite_id = elite->id;
  target_death.dispatch(Command::action_use(ActionType::Wait));
  target_death.dispatch(Command::interact("hazard:death"));
  elite = target_death.actor(target_elite_id);
  check(!target_death.scion().alive && elite->pending_action == ActionType::Wait &&
            elite->pending_action_ticks == 0,
        "target death cancels every pending elite action");
  check(count_events(target_death, EventType::DamageApplied, "thrust") == 0,
        "a telegraphed skill never damages a dead target");
}

void test_elite_skill_replay_is_deterministic() {
  Simulation first(0xA017ULL);
  Simulation second(0xA017ULL);
  Actor* first_elite = setup_elite(first, {1400, 0});
  Actor* second_elite = setup_elite(second, {1400, 0});
  check(first_elite->id == second_elite->id, "elite replay setup retains stable actor identity");
  const std::vector<Command> commands = {
      Command::action_use(ActionType::Wait), Command::action_use(ActionType::Wait),
      Command::action_use(ActionType::Wait), Command::action_use(ActionType::Wait)};
  for (const auto& command : commands) {
    first.dispatch(command);
    second.dispatch(command);
  }
  check(relevant(first) == relevant(second),
        "elite telegraph and skill resolution replay byte-identically");
  const Actor* first_actor = first.actor(first_elite->id);
  const Actor* second_actor = second.actor(second_elite->id);
  check(first_actor && second_actor && first_actor->pending_action == second_actor->pending_action &&
            first_actor->pending_action_ticks == second_actor->pending_action_ticks,
        "elite pending state remains deterministic under replay");
}

void test_non_elite_melee_cadence_is_unchanged() {
  Simulation sim(0xA018ULL);
  Actor* player = sim.actor(sim.scion().actor_id);
  player->position = {0, 0};
  player->stats.life = 1000;
  const std::string monster_id = sim.spawn_monster({800, 0}, 1, false);
  Actor* monster = sim.actor(monster_id);
  check(monster && !monster->elite, "non-elite cadence test creates a plain monster");
  sim.dispatch(Command::action_use(ActionType::Wait));
  player = sim.actor(sim.scion().actor_id);
  monster = sim.actor(monster_id);
  check(count_events(sim, EventType::AttackTelegraphed) == 0,
        "non-elite melee emits no telegraph");
  check(count_events(sim, EventType::DamageApplied, "enemy-melee") == 1,
        "non-elite monster still performs its ordinary melee attack");
  check(monster->cooldown_ticks == monster->stats.attack_speed_ticks,
        "non-elite melee cooldown cadence remains unchanged");
}

void test_war_cry_buff_expiry_and_replay_determinism() {
  Simulation first(0xA003ULL);
  Simulation second(0xA003ULL);
  for (Simulation* sim : {&first, &second}) {
    sim->dispatch(Command::enter("route:tin:1:0"));
    Actor* player = sim->actor(sim->scion().actor_id);
    check(player != nullptr, "War Cry test has a player");
    player->position = {0, 0};
    player->stats.life = player->stats.life_max;
    Actor* first_actor = first_monster(*sim);
    check(first_actor != nullptr, "War Cry test has an initial monster");
    first_actor->position = {800, 0};
    first_actor->stats.life = 1000;
    player->stats.resource = player->stats.resource_max;
    sim->dispatch(Command::action_use(ActionType::WarCry));
    player = sim->actor(sim->scion().actor_id);
    check(player->war_cry_attack_bonus == 4 && player->war_cry_ticks_remaining == 19,
          "War Cry applies its named attack bonus and duration");
    check(player->stats.resource == player->stats.resource_max - 18,
          "War Cry pays its named cost after one tick of regeneration");
    check(count_events(*sim, EventType::BuffApplied, "war-cry") == 1,
          "War Cry emits BuffApplied");
    player->cooldown_ticks = 0;
    sim->dispatch(Command::action_use(ActionType::Melee));
    for (int i = 0; i < 18; ++i) sim->dispatch(Command::action_use(ActionType::Wait));
    player = sim->actor(sim->scion().actor_id);
    check(player->war_cry_attack_bonus == 0 && player->war_cry_ticks_remaining == 0,
          "War Cry expires at its deterministic tick boundary");
    check(count_events(*sim, EventType::BuffExpired, "war-cry") == 1,
          "War Cry emits one BuffExpired event");

    player->stats.resource = 19;
    const int applied_before = count_events(*sim, EventType::BuffApplied, "war-cry");
    sim->dispatch(Command::action_use(ActionType::WarCry));
    check(count_events(*sim, EventType::BuffApplied, "war-cry") == applied_before,
          "insufficient resource makes War Cry a no-op");
  }
  check(relevant(first) == relevant(second),
        "skill actions and buff expiry remain deterministic under replay");
  const Event* applied = last_event(first, EventType::BuffApplied, "war-cry");
  const Event* expired = last_event(first, EventType::BuffExpired, "war-cry");
  check(applied && expired && applied->actor_id == expired->actor_id,
        "War Cry buff events retain the actor identity");
}

void force_relic_resurface(Simulation& sim, const std::string& route,
                           const std::string& target_id = {}) {
  for (int attempt = 0;
       attempt < 64 &&
       (target_id.empty() ? sim.house().relic_candidates.size() == 1
                           : find_ground_item(sim, target_id) == nullptr);
       ++attempt) {
    Actor* player = sim.actor(sim.scion().actor_id);
    if (player) {
      player->position = {0, 0};
      player->stats.life = player->stats.life_max;
    }
    sim.dispatch(Command::enter(route));
    defeat_enemy(sim);
    if (target_id.empty()) {
      for (const auto& item : sim.ground_items()) {
        if (item.relic_candidate) return;
      }
      sim.dispatch(Command::interact("hazard:death"));
      sim.create_successor("Resurfacing Successor " + std::to_string(attempt));
    }
  }
  if (!target_id.empty())
    check(find_ground_item(sim, target_id) != nullptr,
          "target relic resurfaces from the reward stream");
}

bool ground_has_trophy(const Simulation& sim, const std::string& id) {
  for (const auto& trophy : sim.ground_trophies()) {
    if (trophy.id == id) return true;
  }
  return false;
}

void force_trophy_resurface(Simulation& sim, const std::string& route,
                            const std::string& trophy_id) {
  for (int attempt = 0; attempt < 64 && !ground_has_trophy(sim, trophy_id); ++attempt) {
    Actor* player = sim.actor(sim.scion().actor_id);
    if (player) {
      player->position = {0, 0};
      player->stats.life = player->stats.life_max;
    }
    sim.dispatch(Command::enter(route));
    defeat_enemy(sim);
  }
  check(ground_has_trophy(sim, trophy_id), "recoverable trophy resurfaces from the reward stream");
}

std::vector<std::string> recoverable_signature(const Simulation& sim) {
  std::vector<std::string> signature;
  for (const auto& item : sim.house().relic_candidates) {
    signature.push_back("item:" + item.id + ":" +
                        (item.history.empty() ? std::string{} : item.history.back()));
  }
  for (const auto& trophy : sim.house().lost_trophies) {
    signature.push_back("trophy:" + trophy.id);
  }
  return signature;
}

void test_relic_resurface_round_trip() {
  Simulation sim(0xD00DFEEDULL);
  sim.dispatch(Command::enter("route:tin:1:0"));
  defeat_enemy(sim);
  pick_all_rewards(sim);
  const std::string relic_id = sim.scion().carried_items.front().id;
  sim.dispatch(Command::interact("hazard:death"));
  check(sim.house().relic_candidates.size() == 1, "death registers one relic candidate");
  sim.create_successor("Relic Successor");

  force_relic_resurface(sim, "route:tin:1:0");
  const Item* surfaced = nullptr;
  for (const auto& item : sim.ground_items()) {
    if (item.relic_candidate) surfaced = &item;
  }
  check(surfaced != nullptr, "a later reward stream resurfaces a relic");
  check(surfaced->id == relic_id, "resurfacing preserves stable item identity");
  check(sim.house().relic_candidates.empty(), "resurfacing removes the oldest item from the pool");
  check(surfaced->history.size() >= 3 &&
            surfaced->history[surfaced->history.size() - 1] ==
                "resurfaced on route route:tin:1:0",
        "resurfacing appends a route history line");
  bool saw_resurfaced_event = false;
  for (const auto& event : sim.events()) {
    if (event.type == EventType::RelicResurfaced && event.item_id == relic_id &&
        event.text == "route:tin:1:0") {
      saw_resurfaced_event = true;
      break;
    }
  }
  check(saw_resurfaced_event, "resurfacing emits the named event with item and route");
  check(find_legend(sim, "relic_resurfaced") != nullptr,
        "resurfacing records a relic legend");

  sim.dispatch(Command::pick_up(relic_id));
  check(sim.scion().carried_items.size() == 1 && sim.scion().carried_items.front().id == relic_id,
        "resurfaced relic can be picked up");
  extract_from_start(sim);
  check(sim.house().stored_items.size() == 1 && sim.house().stored_items.front().id == relic_id,
        "resurfaced relic extracts into House storage");
  check(find_legend(sim, "relic_extracted") != nullptr,
        "extraction records the existing relic legend");
  check(sim.house().stored_items.front().history.back() == "picked up",
        "pickup history is preserved through extraction");
}

void test_relic_loss_again_returns_once() {
  Simulation sim(0xD00DFEEDULL);
  sim.dispatch(Command::enter("route:tin:1:0"));
  defeat_enemy(sim);
  pick_all_rewards(sim);
  const std::string relic_id = sim.scion().carried_items.front().id;
  sim.dispatch(Command::interact("hazard:death"));
  sim.create_successor("Loss Successor");
  force_relic_resurface(sim, "route:tin:1:0");
  sim.dispatch(Command::pick_up(relic_id));
  check(sim.scion().carried_items.size() == 1, "successor carries the resurfaced relic");
  sim.dispatch(Command::interact("hazard:death"));
  check(sim.house().relic_candidates.size() == 1, "lost resurfaced relic returns to the pool once");
  check(sim.house().relic_candidates.front().id == relic_id,
        "loss-again round trip preserves relic identity");
  check(find_ground_item(sim, relic_id) == nullptr, "lost relic is not duplicated on the ground");
  check(sim.house().stored_items.empty(), "lost resurfaced relic was not stored");
  check(sim.house().relic_candidates.front().history.back() ==
            "lost at route:tin:1:0, awaiting recovery",
        "pack relic death records its route recovery history line");
}

void test_relic_resurface_replay_is_deterministic() {
  Simulation first(0xD00DFEEDULL);
  Simulation second(0xD00DFEEDULL);
  first.dispatch(Command::enter("route:tin:1:0"));
  second.dispatch(Command::enter("route:tin:1:0"));
  // The approach walk derives from the named per-tick step so the stream
  // still reaches and kills the enemy at the current movement cadence.
  reach_enemy(first);
  reach_enemy(second);
  const std::vector<Command> setup = {
      Command::action_use(ActionType::Melee), Command::action_use(ActionType::Melee),
      Command::action_use(ActionType::Melee), Command::action_use(ActionType::Melee),
      Command::action_use(ActionType::Melee), Command::action_use(ActionType::Melee),
      Command::action_use(ActionType::Melee),
      Command::pick_up(""), Command::interact("hazard:death")};
  for (const auto& command : setup) {
    first.dispatch(command);
    second.dispatch(command);
  }
  first.create_successor("Replay Successor");
  second.create_successor("Replay Successor");
  force_relic_resurface(first, "route:tin:1:0");
  force_relic_resurface(second, "route:tin:1:0");
  check(first.events().size() == second.events().size(), "replay emits the same event count");
  check(first.legends() == second.legends(), "replay emits identical relic legends");
  check(first.ground_items().size() == second.ground_items().size(),
        "replay resurfaces the same number of items");
  check(!first.ground_items().empty() && first.ground_items().back().id ==
            second.ground_items().back().id,
        "replay resurfaces the same stable item identity");
}

void test_determinism() {
  Simulation first(0xBADC0FFEEULL);
  Simulation second(0xBADC0FFEEULL);
  first.dispatch(Command::enter("route:tin:1:0"));
  second.dispatch(Command::enter("route:tin:1:0"));
  reach_enemy(first);
  reach_enemy(second);
  for (int i = 0; i < 8; ++i) {
    first.dispatch(Command::action_use(ActionType::Melee));
    second.dispatch(Command::action_use(ActionType::Melee));
  }
  pick_all_rewards(first);
  pick_all_rewards(second);
  extract_from_start(first);
  extract_from_start(second);
  check(relevant(first) == relevant(second), "same seed and command stream are deterministic");
}

void test_actor_symmetry() {
  Simulation sim(7);
  sim.dispatch(Command::enter("route:tin:1:0"));
  const Actor* player = sim.actor(sim.scion().actor_id);
  const Actor* enemy = nullptr;
  for (const auto& actor : sim.actors()) {
    if (actor.kind == ActorKind::Monster) enemy = &actor;
  }
  check(player && enemy, "player and monster use the same actor container");
  check(player->stats.level == enemy->stats.level, "same-level actors share a stat schema");
  check(Simulation::resolve_damage(*player, *enemy) > 0, "player uses the shared damage pipeline");
  check(Simulation::resolve_damage(*enemy, *player) > 0, "monster uses the shared damage pipeline");
}

void test_presentation_catalog_is_authoritative_and_stable() {
  const PresentationCatalog catalog = Simulation::presentation_catalog();
  check(catalog == Simulation::presentation_catalog(),
        "presentation catalog is stable across repeated reads");

  Simulation thrust_sim(0xA019ULL);
  Actor* thrust_player = thrust_sim.actor(thrust_sim.scion().actor_id);
  const std::string thrust_player_id = thrust_player->id;
  thrust_player->position = {0, 0};
  thrust_player->stats.resource = thrust_player->stats.resource_max;
  const std::string thrust_enemy = thrust_sim.spawn_monster({1400, 0});
  thrust_sim.dispatch(Command::action_use(ActionType::Thrust));
  thrust_player = thrust_sim.actor(thrust_player_id);
  check(thrust_player->stats.resource == thrust_player->stats.resource_max -
                                           catalog.thrust_resource_cost +
                                           catalog.resource_regen_per_tick,
        "catalogued Thrust cost matches the resource deduction");
  check(count_events(thrust_sim, EventType::AttackStarted, "thrust") == 1,
        "catalogued Thrust still resolves through the normal attack path");
  check(thrust_sim.actor(thrust_enemy) != nullptr, "catalog test retains the enemy actor");

  Simulation elite_sim(0xA01AULL);
  Actor* elite = setup_elite(elite_sim, {1400, 0});
  const std::string elite_id = elite->id;
  elite_sim.dispatch(Command::action_use(ActionType::Wait));
  const Event* telegraph = last_event(elite_sim, EventType::AttackTelegraphed, "thrust");
  check(telegraph && telegraph->value == catalog.telegraph_ticks,
        "catalogued telegraph duration matches the emitted event");
  elite = elite_sim.actor(elite_id);
  check(elite->pending_action_ticks == catalog.telegraph_ticks,
        "catalogued telegraph duration matches pending state");

  Simulation buff_sim(0xA01BULL);
  Actor* buff_player = buff_sim.actor(buff_sim.scion().actor_id);
  const std::string buff_player_id = buff_player->id;
  buff_player->stats.resource = buff_player->stats.resource_max;
  buff_sim.dispatch(Command::action_use(ActionType::WarCry));
  buff_player = buff_sim.actor(buff_player_id);
  check(buff_player->stats.resource == buff_player->stats.resource_max -
                                          catalog.war_cry_resource_cost +
                                          catalog.resource_regen_per_tick,
        "catalogued War Cry cost matches the resource deduction");
  check(buff_player->war_cry_attack_bonus == catalog.war_cry_attack_bonus &&
            buff_player->war_cry_ticks_remaining == catalog.war_cry_duration_ticks - 1,
        "catalogued War Cry bonus and duration match applied state");
}

void test_extraction() {
  Simulation sim(11);
  sim.dispatch(Command::enter("route:tin:1:0"));
  defeat_enemy(sim);
  pick_all_rewards(sim);
  extract_from_start(sim);
  check(!sim.instance().active, "extraction closes the instance");
  check(sim.house().stored_trophies.size() == 1, "extracted trophy enters durable House storage");
  check(sim.house().stored_items.size() == 1, "extracted item enters durable House storage");
  check(sim.scion().carried_trophies.empty(), "extracted trophy leaves the Scion");
}

void test_death_and_successor() {
  Simulation sim(12);
  sim.dispatch(Command::enter("route:tin:1:0"));
  defeat_enemy(sim);
  pick_all_rewards(sim);
  const std::string carried_item = sim.scion().carried_items.front().id;
  const std::string route = "route:tin:2:0";
  sim.dispatch(Command::interact("hazard:death"));
  check(!sim.scion().alive, "Scion dies in the expedition");
  check(sim.house().stored_trophies.empty(), "unextracted trophy is not durable House storage");
  check(sim.house().lost_trophies.size() == 1,
        "unextracted trophy enters the recoverable pool");
  check(sim.house().relic_candidates.size() == 1, "one meaningful item becomes a relic candidate");
  check(sim.house().relic_candidates.front().id == carried_item, "relic candidate retains stable item identity");
  check(sim.house().route_unlocked(route), "House route progress survives Scion death");
  const std::string old_scion = sim.scion().id;
  sim.create_successor("Second Scion");
  check(sim.scion().id != old_scion, "successor is a new individual character");
  check(sim.scion().level == 1 && sim.scion().carried_items.empty(),
        "successor does not inherit the dead Scion's full progression");
  check(sim.fallen_scions().size() == 1, "dead Scion remains in House history");
  const LegendEntry* death = find_legend(sim, "scion_death");
  check(death && death->scion_id == old_scion, "death legend retains the fallen Scion identity");
  check(death->killer_id == "hazard:death" && death->route_id == "route:tin:1:0",
        "death legend records a stable killer and route");
  check(find_legend(sim, "relic_candidate") != nullptr,
        "death legend records the relic candidate transition");
  check(find_legend(sim, "scion_created") != nullptr,
        "successor creation is retained in the shared House history");
}

void test_d106_all_carried_value_is_recoverable() {
  Simulation sim(0xD106ULL);
  sim.dispatch(Command::enter("route:tin:1:0"));
  defeat_enemy(sim);
  pick_all_rewards(sim);
  const std::string equipped_id = sim.scion().carried_items.front().id;
  const std::string first_trophy_id = sim.scion().carried_trophies.front().id;
  sim.dispatch(Command::equip(equipped_id));

  // A second cleared instance supplies a pack item and a second trophy while
  // the first pair remains carried by the same Scion.
  sim.actor(sim.scion().actor_id)->stats.life = sim.actor(sim.scion().actor_id)->stats.life_max;
  sim.dispatch(Command::enter("route:tin:1:0"));
  defeat_enemy(sim);
  pick_all_rewards(sim);
  check(sim.scion().carried_items.size() == 2 && sim.scion().carried_trophies.size() == 2,
        "D-106 setup carries multiple items and trophies");
  const std::string pack_id = sim.scion().carried_items.back().id;
  const std::string second_trophy_id = sim.scion().carried_trophies.back().id;

  sim.dispatch(Command::interact("hazard:death"));
  check(sim.scion().carried_items.empty() && sim.scion().carried_trophies.empty(),
        "death clears the fallen Scion inventory without deleting its contents");
  check(sim.house().stored_items.empty() && sim.house().stored_trophies.empty(),
        "death does not bypass extraction into durable House storage");
  check(sim.house().relic_candidates.size() == 2,
        "every carried item enters the recoverable relic pool exactly once");
  check(sim.house().relic_candidates[0].id == equipped_id &&
            sim.house().relic_candidates[1].id == pack_id,
        "recoverable items retain carried order and stable identity");
  check(sim.house().relic_candidates[0].history.back() == "registered after Scion death",
        "equipped item retains the established death registration history");
  check(sim.house().relic_candidates[1].history.back() ==
            "lost at route:tin:1:0, awaiting recovery",
        "pack item receives an equivalent route recovery history");
  check(sim.house().lost_trophies.size() == 2 &&
            sim.house().lost_trophies[0].id == first_trophy_id &&
            sim.house().lost_trophies[1].id == second_trophy_id,
        "every carried trophy enters the ordered recoverable trophy pool");
  check(count_events(sim, EventType::TrophyResurfaced) == 0,
        "death itself does not prematurely resurface a trophy");
  check(count_events(sim, EventType::LegendRecorded, "trophy_candidate") == 2,
        "trophy recovery transitions are recorded in the House legend");

  sim.create_successor("D-106 Successor");
  check(sim.scion().carried_items.empty() && sim.scion().carried_trophies.empty(),
        "successor starts empty while the recoverable pools persist");
}

void test_d106_recovery_is_ordered_and_deterministic() {
  Simulation first(0xD106ULL);
  Simulation second(0xD106ULL);
  auto prepare = [](Simulation& sim) {
    sim.dispatch(Command::enter("route:tin:1:0"));
    defeat_enemy(sim);
    pick_all_rewards(sim);
    sim.actor(sim.scion().actor_id)->stats.life = sim.actor(sim.scion().actor_id)->stats.life_max;
    sim.dispatch(Command::enter("route:tin:1:0"));
    defeat_enemy(sim);
    pick_all_rewards(sim);
    sim.dispatch(Command::interact("hazard:death"));
    sim.create_successor("Recovery Successor");
  };
  prepare(first);
  prepare(second);
  check(recoverable_signature(first) == recoverable_signature(second),
        "D-106 recovery pools are deterministic under replay");
  const std::string first_item = first.house().relic_candidates.front().id;
  const std::string second_item = first.house().relic_candidates.back().id;
  const std::string first_trophy = first.house().lost_trophies.front().id;
  force_relic_resurface(first, "route:tin:1:0", first_item);
  force_trophy_resurface(first, "route:tin:1:0", first_trophy);
  check(first.house().relic_candidates.size() == 1 &&
            first.house().relic_candidates.front().id == second_item,
        "item recovery consumes only the oldest item candidate");
  if (find_ground_item(first, second_item) == nullptr) {
    force_relic_resurface(first, "route:tin:1:0", second_item);
  }
  const Item* recovered_pack = find_ground_item(first, second_item);
  check(recovered_pack != nullptr && !recovered_pack->history.empty() &&
            recovered_pack->history.back() == "resurfaced on route route:tin:1:0",
        "pack item eventually resurfaces with ordered identity and history");
  check(first.house().lost_trophies.empty() ||
            first.house().lost_trophies.front().id != first_trophy,
        "trophy recovery consumes the oldest trophy candidate");
  const bool saw_trophy_event =
      count_events(first, EventType::TrophyResurfaced) >= 1;
  check(saw_trophy_event, "trophy resurfacing emits a dedicated recovery event");
  if (!first.house().lost_trophies.empty()) {
    force_trophy_resurface(first, "route:tin:1:0", first.house().lost_trophies.front().id);
  }
  check(first.house().lost_trophies.empty(),
        "repeated reward streams eventually recover the complete trophy pool");
}

void test_item_identity_and_branch() {
  Simulation sim(13);
  sim.dispatch(Command::enter("route:tin:1:0"));
  defeat_enemy(sim);
  const std::string item_id = sim.ground_items().front().id;
  sim.dispatch(Command::pick_up(item_id));
  sim.dispatch(Command::equip(item_id));
  sim.dispatch(Command::interact("use:" + item_id));
  check(sim.scion().carried_items.front().id == item_id, "pickup/equip/use preserve item identity");
  check(sim.scion().carried_items.front().use_count == 1, "use history increments");
  sim.dispatch(Command::interact("branch:ash"));
  check(sim.house().specializations.size() == 1 && sim.house().specializations.front() == "ash",
        "optional branch grants a House access unlock, not a rigid character class");
}

void test_campaign_and_seasonal_extension() {
  Simulation sim(14);
  EmberHunt seasonal;
  sim.set_seasonal_mechanic(&seasonal);
  sim.dispatch(Command::enter("route:tin:1:0"));
  check(sim.instance().seasonal_objective, "external mechanic attaches an objective");
  defeat_enemy(sim);
  check(seasonal.reward_granted(), "external mechanic observes the combat event");
  check(sim.house().seasonal_rewards.size() == 1, "external mechanic feeds a distinct reward to the House");
  check(sim.house().route_cleared("route:tin:1:0"), "campaign graph ownership is House-level");
  sim.dispatch(Command::interact("branch:ash"));
  check(sim.house().specializations.size() == 1, "optional branch grants a House specialization");
  check(sim.house().route_unlocked("route:tin:2:0"), "clearing a route unlocks its child");
}

void test_elite_uses_same_universe() {
  Simulation sim(15);
  sim.dispatch(Command::enter("route:tin:1:0"));
  defeat_enemy(sim);
  sim.dispatch(Command::enter("route:tin:2:0"));
  const Actor* elite = nullptr;
  for (const auto& actor : sim.actors()) {
    if (actor.kind == ActorKind::Monster) elite = &actor;
  }
  const Actor* player = sim.actor(sim.scion().actor_id);
  check(elite && player, "elite and player are actors");
  check(elite->stats.level > player->stats.level, "elite difficulty comes from level/build");
  check(Simulation::resolve_damage(*player, *elite) > 0, "elite still uses the shared damage pipeline");
}

void test_legends_cover_unlocks_and_campaign_milestone() {
  Simulation sim(16);
  sim.dispatch(Command::enter("route:tin:1:0"));
  defeat_enemy(sim);
  const LegendEntry* route = find_legend(sim, "route_cleared");
  const LegendEntry* unlock = find_legend(sim, "route_unlocked");
  const LegendEntry* campaign = find_legend(sim, "campaign_complete");
  check(route && route->subject == "route:tin:1:0", "first route clear records its stable route id");
  check(unlock && unlock->subject == "route:tin:2:0", "route unlock records its stable route id");
  check(campaign && campaign->founding && campaign->subject == sim.house().id,
        "campaign completion is a founding-equivalent legend");
  sim.dispatch(Command::interact("branch:ash"));
  const LegendEntry* branch = find_legend(sim, "branch_unlocked");
  check(branch && branch->subject == "branch:ash", "branch unlock records its stable branch id");
  check(find_legend(sim, "LegendRecorded") == nullptr, "legend kinds remain domain values");
}

void test_elite_kill_and_recorded_event() {
  Simulation sim(17);
  sim.dispatch(Command::enter("route:tin:1:0"));
  defeat_enemy(sim);
  sim.dispatch(Command::enter("route:tin:2:0"));
  defeat_enemy(sim);
  const LegendEntry* elite = find_legend(sim, "elite_kill");
  check(elite && elite->route_id == "route:tin:2:0", "elite kill records the route");
  check(!elite->killer_id.empty() && elite->subject.rfind("actor-", 0) == 0,
        "elite legend references stable actor ids");
  bool saw_recorded_event = false;
  for (const auto& event : sim.events()) {
    if (event.type == EventType::LegendRecorded) {
      saw_recorded_event = true;
      break;
    }
  }
  check(saw_recorded_event, "recording a legend emits a LegendRecorded event");
}

void test_legends_are_bounded_and_evict_oldest_non_founding() {
  Simulation sim(18);
  sim.dispatch(Command::enter("route:tin:1:0"));
  defeat_enemy(sim);
  const LegendEntry* campaign = find_legend(sim, "campaign_complete");
  check(campaign != nullptr, "cap test has a founding milestone to preserve");
  const std::uint64_t campaign_ordinal = campaign->ordinal;
  for (int i = 0; i < 40; ++i) {
    sim.dispatch(Command::interact("hazard:death"));
    sim.create_successor("Successor " + std::to_string(i));
  }
  check(sim.legends().size() == kLegendCapacity, "legend history enforces its cap");
  check(sim.legends().front().founding, "founding milestone survives non-founding eviction");
  check(sim.legends().front().ordinal == campaign_ordinal,
        "oldest ordinary records are evicted before the founding record");
  for (std::size_t index = 1; index < sim.legends().size(); ++index) {
    check(sim.legends()[index - 1].ordinal < sim.legends()[index].ordinal,
          "legend ordinals remain strictly increasing after eviction");
  }
}

void test_legend_stable_ids_and_deterministic_replay() {
  Simulation first(19);
  Simulation second(19);
  const std::vector<Command> commands = {
      Command::enter("route:tin:1:0"), Command::move(1, 0), Command::move(1, 0),
      Command::move(1, 0), Command::move(1, 0), Command::action_use(ActionType::Melee),
      Command::action_use(ActionType::Melee), Command::action_use(ActionType::Melee),
      Command::action_use(ActionType::Melee), Command::action_use(ActionType::Melee),
      Command::action_use(ActionType::Melee), Command::action_use(ActionType::Melee),
  };
  for (const auto& command : commands) {
    first.dispatch(command);
    second.dispatch(command);
  }
  check(first.legends() == second.legends(),
        "identical seed and commands produce byte-identical legend records");
  check(!first.legends().empty(), "deterministic replay produces legend records");
  for (const auto& legend : first.legends()) {
    check(!legend.scion_id.empty() && !legend.scion_name.empty(),
          "legend records retain the Scion identity");
    if (legend.kind == "route_cleared") {
      check(legend.subject.rfind("route:", 0) == 0, "route legend uses a stable route id");
    }
  }
}

}  // namespace

int main() {
  test_determinism();
  test_actor_symmetry();
  test_actor_facing_follows_movement_and_aim();
  test_monster_facing_tracks_pursuit_target();
  test_facing_replay_is_deterministic();
  test_movement_step_matches_tick_rate_derivation();
  test_dash_is_a_bounded_burst();
  test_skill_resource_gating_and_thrust();
  test_sweep_hits_multiple_targets_and_gates_resource();
  test_elite_thrust_telegraph_timing();
  test_elite_skill_cone_gating();
  test_elite_skill_fizzles_when_resolution_gates_fail();
  test_elite_sweep_uses_shared_pipeline();
  test_elite_telegraph_cancels_on_death();
  test_elite_skill_replay_is_deterministic();
  test_non_elite_melee_cadence_is_unchanged();
  test_war_cry_buff_expiry_and_replay_determinism();
  test_presentation_catalog_is_authoritative_and_stable();
  test_extraction();
  test_death_and_successor();
  test_d106_all_carried_value_is_recoverable();
  test_d106_recovery_is_ordered_and_deterministic();
  test_item_identity_and_branch();
  test_campaign_and_seasonal_extension();
  test_elite_uses_same_universe();
  test_legends_cover_unlocks_and_campaign_milestone();
  test_elite_kill_and_recorded_event();
  test_legends_are_bounded_and_evict_oldest_non_founding();
  test_legend_stable_ids_and_deterministic_replay();
  test_relic_resurface_round_trip();
  test_relic_loss_again_returns_once();
  test_relic_resurface_replay_is_deterministic();
  std::cout << "verdigris core tests: PASS\n";
  return 0;
}
