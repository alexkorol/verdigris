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

void force_relic_resurface(Simulation& sim, const std::string& route) {
  for (int attempt = 0; attempt < 32 && sim.house().relic_candidates.size() == 1; ++attempt) {
    sim.dispatch(Command::enter(route));
    defeat_enemy(sim);
    for (const auto& item : sim.ground_items()) {
      if (item.relic_candidate) return;
    }
    sim.dispatch(Command::interact("hazard:death"));
    sim.create_successor("Resurfacing Successor " + std::to_string(attempt));
  }
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
  check(sim.house().relic_candidates.front().history.back() == "registered after Scion death",
        "loss-again death appends the death history line");
}

void test_relic_resurface_replay_is_deterministic() {
  Simulation first(0xD00DFEEDULL);
  Simulation second(0xD00DFEEDULL);
  const std::vector<Command> setup = {
      Command::enter("route:tin:1:0"), Command::move(1, 0), Command::move(1, 0),
      Command::move(1, 0), Command::move(1, 0), Command::action_use(ActionType::Melee),
      Command::action_use(ActionType::Melee), Command::action_use(ActionType::Melee),
      Command::action_use(ActionType::Melee), Command::action_use(ActionType::Melee),
      Command::action_use(ActionType::Melee), Command::action_use(ActionType::Melee),
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
  const LegendEntry* death = find_legend(sim, "scion_death");
  check(death && death->scion_id == old_scion, "death legend retains the fallen Scion identity");
  check(death->killer_id == "hazard:death" && death->route_id == "route:tin:1:0",
        "death legend records a stable killer and route");
  check(find_legend(sim, "relic_candidate") != nullptr,
        "death legend records the relic candidate transition");
  check(find_legend(sim, "scion_created") != nullptr,
        "successor creation is retained in the shared House history");
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
  test_extraction();
  test_death_and_successor();
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
