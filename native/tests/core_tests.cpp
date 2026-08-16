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

void reach_enemy(Simulation& sim) {
  for (int i = 0; i < 4; ++i) sim.dispatch(Command::move(1, 0));
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
  for (int i = 0; i < 4; ++i) sim.dispatch(Command::move(-1, 0));
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
  return result;
}

void test_determinism() {
  Simulation first(0xBADC0FFEEULL);
  Simulation second(0xBADC0FFEEULL);
  first.dispatch(Command::enter("route:tin:1:0"));
  second.dispatch(Command::enter("route:tin:1:0"));
  for (int i = 0; i < 4; ++i) {
    first.dispatch(Command::move(1, 0));
    second.dispatch(Command::move(1, 0));
  }
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
  check(sim.house().stored_trophies.empty(), "unextracted trophy is not preserved");
  check(sim.house().relic_candidates.size() == 1, "one meaningful item becomes a relic candidate");
  check(sim.house().relic_candidates.front().id == carried_item, "relic candidate retains stable item identity");
  check(sim.house().route_unlocked(route), "House route progress survives Scion death");
  const std::string old_scion = sim.scion().id;
  sim.create_successor("Second Scion");
  check(sim.scion().id != old_scion, "successor is a new individual character");
  check(sim.scion().level == 1 && sim.scion().carried_items.empty(),
        "successor does not inherit the dead Scion's full progression");
  check(sim.fallen_scions().size() == 1, "dead Scion remains in House history");
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

}  // namespace

int main() {
  test_determinism();
  test_actor_symmetry();
  test_extraction();
  test_death_and_successor();
  test_item_identity_and_branch();
  test_campaign_and_seasonal_extension();
  test_elite_uses_same_universe();
  std::cout << "verdigris core tests: PASS\n";
  return 0;
}
