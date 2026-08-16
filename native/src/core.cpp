#include "verdigris/core.hpp"

#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

#include "verdigris/seasonal.hpp"

namespace verdigris {

namespace {
constexpr int kEnemySpawnX = 2000;
constexpr int kMeleeRange = 1100;
constexpr int kExtractionRange = 250;

std::string hex_id(std::uint64_t value) {
  std::ostringstream stream;
  stream << std::hex << value;
  return stream.str();
}

ActorStats player_stats() {
  ActorStats stats;
  stats.level = 1;
  stats.strength = 10;
  stats.dexterity = 10;
  stats.intelligence = 10;
  stats.life_max = 100;
  stats.life = stats.life_max;
  stats.resource_max = 50;
  stats.resource = stats.resource_max;
  stats.attack = 12;
  stats.defense = 5;
  stats.move_speed = 300;
  stats.attack_speed_ticks = 3;
  return stats;
}

ActorStats enemy_stats(int level) {
  ActorStats stats = player_stats();
  stats.level = level;
  stats.strength = 10 + (level - 1) * 2;
  stats.dexterity = 10 + (level - 1) * 2;
  stats.intelligence = 10 + (level - 1) * 2;
  stats.life_max = 35 + (level - 1) * 15;
  stats.life = stats.life_max;
  stats.attack = 8 + (level - 1) * 3;
  stats.defense = 4 + (level - 1) * 2;
  stats.move_speed = 240;
  stats.attack_speed_ticks = 5;
  return stats;
}
}  // namespace

int manhattan_distance(Vec2 a, Vec2 b) {
  return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

bool ActorStats::operator==(const ActorStats& other) const {
  return level == other.level && strength == other.strength && dexterity == other.dexterity &&
         intelligence == other.intelligence && life_max == other.life_max && life == other.life &&
         resource_max == other.resource_max && resource == other.resource &&
         attack == other.attack && defense == other.defense && move_speed == other.move_speed &&
         attack_speed_ticks == other.attack_speed_ticks && resistances == other.resistances;
}

bool LegendEntry::operator==(const LegendEntry& other) const {
  return ordinal == other.ordinal && tick == other.tick && scion_id == other.scion_id &&
         scion_name == other.scion_name && kind == other.kind && subject == other.subject &&
         detail == other.detail && killer_id == other.killer_id && route_id == other.route_id &&
         founding == other.founding;
}

bool House::route_unlocked(const std::string& route_id) const {
  return std::find(unlocked_routes.begin(), unlocked_routes.end(), route_id) != unlocked_routes.end();
}

bool House::route_cleared(const std::string& route_id) const {
  return std::find(cleared_routes.begin(), cleared_routes.end(), route_id) != cleared_routes.end();
}

std::uint64_t Simulation::Rng::next() {
  std::uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
  z = (z ^ (z >> 30U)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27U)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31U);
}

int Simulation::Rng::range(int min, int max) {
  if (max <= min) return min;
  return min + static_cast<int>(next() % static_cast<std::uint64_t>(max - min + 1));
}

std::string Simulation::Rng::token(const std::string& prefix) {
  return prefix + "-" + hex_id(next()) + "-" + std::to_string(serial++);
}

Command Command::move(int x, int y) {
  return {CommandType::MoveIntent, x, y, ActionType::Wait, {}};
}

Command Command::action_use(ActionType action) {
  return {CommandType::UseAction, 0, 0, action, {}};
}

Command Command::interact(const std::string& target) {
  return {CommandType::Interact, 0, 0, ActionType::Wait, target};
}

Command Command::pick_up(const std::string& item_id) {
  return {CommandType::PickUp, 0, 0, ActionType::Wait, item_id};
}

Command Command::equip(const std::string& item_id) {
  return {CommandType::Equip, 0, 0, ActionType::Wait, item_id};
}

Command Command::enter(const std::string& route_id) {
  return {CommandType::EnterInstance, 0, 0, ActionType::Wait, route_id};
}

Command Command::extract() {
  return {CommandType::ExtractToHouse, 0, 0, ActionType::Wait, {}};
}

Simulation::Simulation(std::uint64_t seed, const std::string& house_name) : rng_(seed) {
  house_.id = rng_.token("house");
  house_.name = house_name;
  house_.routes = {
      {"route:tin:1:0", "", {"route:tin:2:0"}, false},
      {"route:tin:2:0", "route:tin:1:0", {}, false},
      {"branch:ash", "route:tin:1:0", {}, true},
  };
  house_.unlocked_routes.push_back("route:tin:1:0");
  scion_.id = rng_.token("scion");
  scion_.name = "First Scion";
  scion_.actor_id = rng_.token("actor");
  Actor player{scion_.actor_id, ActorKind::Player, player_stats(), {0, 0}, true, 0, std::nullopt};
  actors_.push_back(player);
  emit(EventType::HouseCreated, {}, {}, {}, house_.name);
  emit(EventType::ScionCreated, player.id, {}, {}, scion_.name);
  record_legend("scion_created", scion_.id, scion_.name);
}

const House& Simulation::house() const { return house_; }
const Scion& Simulation::scion() const { return scion_; }
const std::vector<Scion>& Simulation::fallen_scions() const { return fallen_scions_; }
const std::vector<Actor>& Simulation::actors() const { return actors_; }
const InstanceState& Simulation::instance() const { return instance_; }
const std::vector<Item>& Simulation::ground_items() const { return ground_items_; }
const std::vector<Trophy>& Simulation::ground_trophies() const { return ground_trophies_; }
const std::vector<Event>& Simulation::events() const { return events_; }
const std::vector<LegendEntry>& Simulation::legends() const { return house_.legends; }
std::uint64_t Simulation::tick() const { return tick_; }

const Actor* Simulation::actor(const std::string& id) const {
  for (const auto& candidate : actors_) {
    if (candidate.id == id) return &candidate;
  }
  return nullptr;
}

Actor* Simulation::actor(const std::string& id) {
  for (auto& candidate : actors_) {
    if (candidate.id == id) return &candidate;
  }
  return nullptr;
}

void Simulation::set_seasonal_mechanic(SeasonalMechanic* mechanic) {
  seasonal_mechanic_ = mechanic;
}

void Simulation::emit(EventType type, const std::string& actor_id, const std::string& item_id,
                      const std::string& trophy_id, const std::string& text, int value) {
  Event event{type, actor_id, item_id, trophy_id, text, value, tick_};
  events_.push_back(event);
  if (seasonal_mechanic_) seasonal_mechanic_->on_event(*this, event);
}

void Simulation::record_legend(const std::string& kind, const std::string& subject,
                               const std::string& detail, const std::string& killer_id,
                               const std::string& route_id, bool founding) {
  LegendEntry entry;
  entry.ordinal = next_legend_ordinal_++;
  entry.tick = tick_;
  entry.scion_id = scion_.id;
  entry.scion_name = scion_.name;
  entry.kind = kind;
  entry.subject = subject;
  entry.detail = detail;
  entry.killer_id = killer_id;
  entry.route_id = route_id;
  entry.founding = founding;

  if (house_.legends.size() >= kLegendCapacity) {
    auto victim = std::find_if(
        house_.legends.begin(), house_.legends.end(),
        [](const LegendEntry& candidate) { return !candidate.founding; });
    if (victim == house_.legends.end()) victim = house_.legends.begin();
    house_.legends.erase(victim);
  }
  house_.legends.push_back(entry);
  emit(EventType::LegendRecorded, scion_.actor_id,
       kind == "relic_extracted" ? subject : std::string{}, {}, kind,
       static_cast<int>(entry.ordinal));
}

void Simulation::dispatch(const Command& command) {
  if (command.type == CommandType::MoveIntent) resolve_move(command.dx, command.dy);
  if (command.type == CommandType::UseAction) resolve_action(command.action);
  if (command.type == CommandType::Interact) resolve_interact(command.target);
  if (command.type == CommandType::PickUp) resolve_pickup(command.target);
  if (command.type == CommandType::Equip) resolve_equip(command.target);
  if (command.type == CommandType::EnterInstance) resolve_enter(command.target);
  if (command.type == CommandType::ExtractToHouse) resolve_extract();
  advance_tick();
}

void Simulation::resolve_move(int dx, int dy) {
  Actor* player = actor(scion_.actor_id);
  if (!player || !player->alive || !scion_.alive) return;
  const int length = std::max(1, std::abs(dx) + std::abs(dy));
  player->position.x += (dx * player->stats.move_speed) / length;
  player->position.y += (dy * player->stats.move_speed) / length;
  emit(EventType::ActorMoved, player->id, {}, {}, {}, player->position.x);
}

void Simulation::resolve_action(ActionType action) {
  Actor* player = actor(scion_.actor_id);
  if (!player || !player->alive || !scion_.alive) return;
  if (action == ActionType::Dash) {
    player->position.x += player->stats.move_speed * 2;
    emit(EventType::ActorMoved, player->id, {}, {}, "dash", player->position.x);
    return;
  }
  if (action != ActionType::Melee || player->cooldown_ticks > 0) return;
  Actor* target = nullptr;
  int best_distance = std::numeric_limits<int>::max();
  for (auto& candidate : actors_) {
    if (candidate.kind != ActorKind::Monster || !candidate.alive) continue;
    const int distance = manhattan_distance(player->position, candidate.position);
    if (distance <= kMeleeRange && distance < best_distance) {
      target = &candidate;
      best_distance = distance;
    }
  }
  if (!target) return;
  player->cooldown_ticks = player->stats.attack_speed_ticks;
  emit(EventType::AttackStarted, player->id, {}, {}, "melee");
  const int damage = resolve_damage(*player, *target, equipped_attack_bonus());
  target->stats.life = std::max(0, target->stats.life - damage);
  emit(EventType::DamageApplied, target->id, {}, {}, "melee", damage);
  if (!scion_.carried_items.empty()) {
    for (auto& item : scion_.carried_items) {
      if (item.equipped) {
        item.use_count += 1;
        item.history.push_back("used at tick " + std::to_string(tick_));
        emit(EventType::ItemHistoryUpdated, player->id, item.id, {}, "use", item.use_count);
        break;
      }
    }
  }
  if (target->stats.life == 0) handle_death(*target, player->id);
}

int Simulation::resolve_damage(const Actor& attacker, const Actor& defender, int item_bonus) {
  const int raw = attacker.stats.attack + attacker.stats.strength / 2 + item_bonus;
  const int mitigated = raw - defender.stats.defense / 2;
  return std::max(1, mitigated);
}

void Simulation::resolve_interact(const std::string& target) {
  if (target.rfind("use:", 0) == 0 && scion_.alive) {
    const std::string item_id = target.substr(4);
    for (auto& item : scion_.carried_items) {
      if (item.id == item_id) {
        item.use_count += 1;
        item.history.push_back("used at tick " + std::to_string(tick_));
        emit(EventType::ItemHistoryUpdated, scion_.actor_id, item.id, {}, "use", item.use_count);
        return;
      }
    }
  }
  if (target == "hazard:death" && scion_.alive) {
    Actor* player = actor(scion_.actor_id);
    if (player) {
      player->stats.life = 0;
      handle_death(*player, "hazard:death");
    }
    return;
  }
  if (target == "branch:ash" && house_.route_cleared("route:tin:1:0") &&
      std::find(house_.specializations.begin(), house_.specializations.end(), "ash") ==
          house_.specializations.end()) {
    house_.specializations.push_back("ash");
    emit(EventType::BranchUnlocked, scion_.actor_id, {}, {}, "ash");
    record_legend("branch_unlocked", target, "specialization=ash");
  }
}

void Simulation::resolve_pickup(const std::string& item_id) {
  auto item = std::find_if(ground_items_.begin(), ground_items_.end(),
                           [&](const Item& value) { return value.id == item_id; });
  if (item == ground_items_.end() || !scion_.alive) {
    auto trophy = std::find_if(ground_trophies_.begin(), ground_trophies_.end(),
                               [&](const Trophy& value) { return value.id == item_id; });
    if (trophy == ground_trophies_.end() || !scion_.alive) return;
    scion_.carried_trophies.push_back(*trophy);
    emit(EventType::TrophyPickedUp, scion_.actor_id, {}, trophy->id);
    ground_trophies_.erase(trophy);
    instance_.ground_trophy_ids.erase(std::remove(instance_.ground_trophy_ids.begin(),
                                                  instance_.ground_trophy_ids.end(), item_id),
                                      instance_.ground_trophy_ids.end());
    return;
  }
  item->owner_id = scion_.id;
  item->history.push_back("picked up");
  scion_.carried_items.push_back(*item);
  emit(EventType::ItemPickedUp, scion_.actor_id, item->id);
  ground_items_.erase(item);
  instance_.ground_item_ids.erase(
      std::remove(instance_.ground_item_ids.begin(), instance_.ground_item_ids.end(), item_id),
      instance_.ground_item_ids.end());
}

void Simulation::resolve_equip(const std::string& item_id) {
  auto item = std::find_if(scion_.carried_items.begin(), scion_.carried_items.end(),
                           [&](const Item& value) { return value.id == item_id; });
  if (item == scion_.carried_items.end() || !scion_.alive) return;
  for (auto& carried : scion_.carried_items) carried.equipped = false;
  item->equipped = true;
  item->history.push_back("equipped");
  Actor* player = actor(scion_.actor_id);
  if (player) player->equipped_item_id = item->id;
  emit(EventType::ItemEquipped, scion_.actor_id, item->id);
  emit(EventType::ItemHistoryUpdated, scion_.actor_id, item->id, {}, "equip");
}

void Simulation::resolve_enter(const std::string& route_id) {
  if (!house_.route_unlocked(route_id) || !scion_.alive) return;
  instance_ = {};
  instance_.active = true;
  instance_.route_id = route_id;
  spawn_enemy();
  emit(EventType::InstanceEntered, scion_.actor_id, {}, {}, route_id);
  if (seasonal_mechanic_) seasonal_mechanic_->on_instance_enter(*this, instance_);
}

void Simulation::spawn_enemy() {
  const int level = instance_.route_id == "route:tin:2:0" ? 2 : 1;
  Actor enemy{rng_.token("actor"), ActorKind::Monster, enemy_stats(level),
              {kEnemySpawnX, 0}, true, 0, std::nullopt};
  enemy.elite = instance_.route_id == "route:tin:2:0";
  actors_.erase(std::remove_if(actors_.begin(), actors_.end(),
                               [](const Actor& value) { return value.kind == ActorKind::Monster; }),
                actors_.end());
  actors_.push_back(enemy);
}

void Simulation::enemy_turn() {
  Actor* player = actor(scion_.actor_id);
  if (!player || !player->alive || !scion_.alive) return;
  for (auto& enemy : actors_) {
    if (enemy.kind != ActorKind::Monster || !enemy.alive) continue;
    if (enemy.cooldown_ticks > 0) continue;
    if (manhattan_distance(enemy.position, player->position) > kMeleeRange) continue;
    enemy.cooldown_ticks = enemy.stats.attack_speed_ticks;
    const int damage = resolve_damage(enemy, *player);
    player->stats.life = std::max(0, player->stats.life - damage);
    emit(EventType::DamageApplied, player->id, {}, {}, "enemy-melee", damage);
    if (player->stats.life == 0) {
      handle_death(*player, enemy.id);
      return;
    }
  }
}

void Simulation::advance_tick() {
  ++tick_;
  for (auto& actor_value : actors_) {
    if (actor_value.cooldown_ticks > 0) --actor_value.cooldown_ticks;
  }
  enemy_turn();
}

void Simulation::drop_reward() {
  Item item;
  item.id = rng_.token("item");
  item.name = "Ember-edged axe";
  item.attack_bonus = 4 + rng_.range(0, 3);
  item.history.push_back("forged by the expedition seed");
  ground_items_.push_back(item);
  instance_.ground_item_ids.push_back(item.id);
  emit(EventType::ItemDropped, {}, item.id, {}, item.name, item.attack_bonus);

  Trophy trophy{rng_.token("trophy"), "Warden's ember"};
  ground_trophies_.push_back(trophy);
  instance_.ground_trophy_ids.push_back(trophy.id);
  emit(EventType::TrophyDropped, {}, {}, trophy.id, trophy.name);
}

void Simulation::clear_route_and_unlock_children() {
  const bool first_clear =
      std::find(house_.cleared_routes.begin(), house_.cleared_routes.end(), instance_.route_id) ==
      house_.cleared_routes.end();
  if (first_clear) {
    house_.cleared_routes.push_back(instance_.route_id);
    record_legend("route_cleared", instance_.route_id, "first_clear");
  }
  for (const auto& route : house_.routes) {
    if (route.parent_id == instance_.route_id && !house_.route_unlocked(route.id)) {
      house_.unlocked_routes.push_back(route.id);
      emit(EventType::RouteUnlocked, {}, {}, {}, route.id);
      record_legend("route_unlocked", route.id, "parent=" + instance_.route_id);
    }
  }
  if (!house_.campaign_complete) {
    house_.campaign_complete = true;
    record_legend("campaign_complete", house_.id, "route=" + instance_.route_id, {},
                  instance_.route_id, true);
  }
}

void Simulation::handle_death(Actor& actor_value, const std::string& killer_id) {
  actor_value.alive = false;
  if (actor_value.kind == ActorKind::Monster) {
    emit(EventType::ActorDied, actor_value.id, {}, {}, "monster");
    const Actor* player = actor(scion_.actor_id);
    const bool is_elite = actor_value.elite ||
                          (player && actor_value.stats.level >= player->stats.level + 2);
    if (is_elite) {
      record_legend("elite_kill", actor_value.id,
                    "killer=" + killer_id + ";route=" + instance_.route_id, killer_id,
                    instance_.route_id);
    }
    drop_reward();
    clear_route_and_unlock_children();
    return;
  }
  scion_.alive = false;
  scion_.level = actor_value.stats.level;
  // One stable item becomes a future possibility; the rest of carried value is lost.
  if (!scion_.carried_items.empty()) {
    Item relic = scion_.carried_items.front();
    relic.relic_candidate = true;
    relic.history.push_back("registered after Scion death");
    house_.relic_candidates.push_back(relic);
    record_legend("relic_candidate", relic.id, "name=" + relic.name, killer_id,
                  instance_.route_id);
  }
  scion_.carried_items.clear();
  scion_.carried_trophies.clear();
  actor_value.equipped_item_id.reset();
  emit(EventType::ActorDied, actor_value.id, {}, {}, "scion");
  emit(EventType::ScionLost, actor_value.id, {}, {}, scion_.name);
  record_legend("scion_death", scion_.id,
                "killer=" + (killer_id.empty() ? std::string("unknown") : killer_id) +
                    ";route=" + instance_.route_id,
                killer_id, instance_.route_id);
}

void Simulation::resolve_extract() {
  Actor* player = actor(scion_.actor_id);
  if (!player || !player->alive || !instance_.active || !at_extraction()) return;
  for (const auto& item : scion_.carried_items) {
    house_.stored_items.push_back(item);
    emit(EventType::ItemExtracted, player->id, item.id);
    if (item.relic_candidate) {
      record_legend("relic_extracted", item.id, "name=" + item.name, player->id,
                    instance_.route_id);
    }
  }
  for (const auto& trophy : scion_.carried_trophies) {
    house_.stored_trophies.push_back(trophy);
    emit(EventType::TrophyExtracted, player->id, {}, trophy.id);
  }
  scion_.carried_items.clear();
  scion_.carried_trophies.clear();
  instance_.active = false;
  emit(EventType::HouseStoreChanged, {}, {}, {}, "extraction");
}

bool Simulation::at_extraction() const {
  const Actor* player = actor(scion_.actor_id);
  return player && manhattan_distance(player->position, instance_.extraction_point) <= kExtractionRange;
}

int Simulation::equipped_attack_bonus() const {
  for (const auto& item : scion_.carried_items) {
    if (item.equipped) return item.attack_bonus;
  }
  return 0;
}

void Simulation::grant_seasonal_reward(const std::string& reward) {
  house_.seasonal_rewards.push_back(reward);
  emit(EventType::SeasonalRewardGranted, {}, {}, {}, reward);
}

void Simulation::create_successor(const std::string& name) {
  if (scion_.alive) return;
  fallen_scions_.push_back(scion_);
  scion_ = {};
  scion_.id = rng_.token("scion");
  scion_.name = name;
  scion_.actor_id = rng_.token("actor");
  Actor player{scion_.actor_id, ActorKind::Player, player_stats(), {0, 0}, true, 0, std::nullopt};
  actors_.clear();
  actors_.push_back(player);
  emit(EventType::ScionCreated, player.id, {}, {}, scion_.name);
  record_legend("scion_created", scion_.id, scion_.name);
}

void Simulation::add_seasonal_objective(const std::string& description) {
  instance_.seasonal_objective = true;
  instance_.seasonal_objective_text = description;
  emit(EventType::SeasonalObjectiveAdded, {}, {}, {}, description);
}

}  // namespace verdigris
