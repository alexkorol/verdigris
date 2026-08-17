#include "verdigris/core.hpp"

#include <cmath>
#include <charconv>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>

#include "verdigris/seasonal.hpp"

namespace verdigris {

namespace {
// D-114 derives the expedition envelope from the same player walking cadence
// used by combat. Keep these aliases local so the simulation remains the sole
// authority while the named table stays visible to tests and presentation.
constexpr int kEnemySpawnX = world_scale::kEnemySpawnDistance;
constexpr int kExtractionRange = world_scale::kExtractionRange;
constexpr int kThrustDamageNumerator = 13;
constexpr int kThrustDamageDenominator = 10;
constexpr int kSweepDamageNumerator = 3;
constexpr int kSweepDamageDenominator = 4;
constexpr int kSweepCooldownNumerator = 3;
constexpr int kSweepCooldownDenominator = 2;
// A relic is a possibility in the drop stream, not a guaranteed inheritance.
constexpr int kRelicResurfaceOneIn = 4;

using presentation_constants::kMeleeRange;
using presentation_constants::kResourceRegenPerTick;
using presentation_constants::kSweepResourceCost;
using presentation_constants::kThrustRange;
using presentation_constants::kThrustResourceCost;
using presentation_constants::kWarCryAttackBonus;
using presentation_constants::kWarCryDurationTicks;
using presentation_constants::kWarCryResourceCost;

std::string hex_id(std::uint64_t value) {
  std::ostringstream stream;
  stream << std::hex << value;
  return stream.str();
}

int direction_component(int value) {
  return value < 0 ? -1 : value > 0 ? 1 : 0;
}

Vec2 quantize_direction(int dx, int dy) {
  return {direction_component(dx), direction_component(dy)};
}

Vec2 movement_delta(int dx, int dy, int move_speed) {
  if (dx == 0 && dy == 0) return {};
  const int length = std::max(1, std::abs(dx) + std::abs(dy));
  const int step = movement_step_per_tick(move_speed);
  return {(dx * step) / length, (dy * step) / length};
}

bool is_forward(const Vec2& facing, Vec2 delta) {
  // A strict half-plane keeps the boundary (dot == 0) out of a thrust cone.
  // All operands are bounded integer world-space values and the facing is
  // limited to -1/0/+1 components.
  return facing.x * delta.x + facing.y * delta.y > 0;
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
  // Per-second value; resolve_move derives the deterministic 50 ms step.
  stats.move_speed = world_scale::kPlayerMoveSpeed;
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

bool PresentationCatalog::operator==(const PresentationCatalog& other) const {
  return thrust_resource_cost == other.thrust_resource_cost &&
         sweep_resource_cost == other.sweep_resource_cost &&
         war_cry_resource_cost == other.war_cry_resource_cost &&
         melee_range == other.melee_range && thrust_range == other.thrust_range &&
         telegraph_ticks == other.telegraph_ticks &&
         war_cry_attack_bonus == other.war_cry_attack_bonus &&
         war_cry_duration_ticks == other.war_cry_duration_ticks &&
         resource_regen_per_tick == other.resource_regen_per_tick;
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

Command Command::aim(int x, int y) {
  return {CommandType::AimIntent, x, y, ActionType::Wait, {}};
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

PresentationCatalog Simulation::presentation_catalog() {
  return {presentation_constants::kThrustResourceCost,
          presentation_constants::kSweepResourceCost,
          presentation_constants::kWarCryResourceCost,
          presentation_constants::kMeleeRange,
          presentation_constants::kThrustRange,
          kTelegraphTicks,
          presentation_constants::kWarCryAttackBonus,
          presentation_constants::kWarCryDurationTicks,
          presentation_constants::kResourceRegenPerTick};
}

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

std::string Simulation::spawn_monster(Vec2 position, int level, bool elite) {
  const int bounded_level = std::max(1, level);
  Actor enemy{rng_.token("actor"), ActorKind::Monster, enemy_stats(bounded_level), position,
              true, 0, std::nullopt, elite};
  actors_.push_back(enemy);
  return enemy.id;
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
  if (command.type == CommandType::AimIntent) resolve_aim(command.dx, command.dy);
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
  const Vec2 direction = quantize_direction(dx, dy);
  if (direction.x != 0 || direction.y != 0) player->facing = direction;
  const Vec2 delta = movement_delta(dx, dy, player->stats.move_speed);
  player->position.x += delta.x;
  player->position.y += delta.y;
  emit(EventType::ActorMoved, player->id, {}, {}, {}, player->position.x);
}

void Simulation::resolve_aim(int dx, int dy) {
  Actor* player = actor(scion_.actor_id);
  if (!player || !player->alive || !scion_.alive) return;
  const Vec2 direction = quantize_direction(dx, dy);
  if (direction.x != 0 || direction.y != 0) player->facing = direction;
}

void Simulation::resolve_action(ActionType action) {
  Actor* player = actor(scion_.actor_id);
  if (!player || !player->alive || !scion_.alive) return;
  resolve_actor_action(*player, action);
}

void Simulation::resolve_actor_action(Actor& attacker, ActionType action) {
  if (!attacker.alive) return;
  if (action == ActionType::Dash) {
    const Vec2 delta = movement_delta(attacker.facing.x, attacker.facing.y,
                                      attacker.stats.move_speed);
    attacker.position.x += delta.x * kDashMovementTicks;
    attacker.position.y += delta.y * kDashMovementTicks;
    emit(EventType::ActorMoved, attacker.id, {}, {}, "dash", attacker.position.x);
    return;
  }

  if (action == ActionType::WarCry) {
    if (attacker.stats.resource < kWarCryResourceCost) return;
    attacker.stats.resource -= kWarCryResourceCost;
    attacker.war_cry_attack_bonus = kWarCryAttackBonus;
    attacker.war_cry_ticks_remaining = kWarCryDurationTicks;
    emit(EventType::BuffApplied, attacker.id, {}, {}, "war-cry", kWarCryAttackBonus);
    return;
  }

  if (action != ActionType::Melee && action != ActionType::Thrust &&
      action != ActionType::Sweep) {
    return;
  }

  const int resource_cost = action == ActionType::Thrust
                                ? kThrustResourceCost
                                : action == ActionType::Sweep ? kSweepResourceCost : 0;
  if (attacker.cooldown_ticks > 0 || attacker.stats.resource < resource_cost) return;

  const ActorKind target_kind = attacker.kind == ActorKind::Player ? ActorKind::Monster
                                                                     : ActorKind::Player;
  std::vector<Actor*> targets;
  Actor* nearest = nullptr;
  int best_distance = std::numeric_limits<int>::max();
  for (auto& candidate : actors_) {
    if (candidate.kind != target_kind || !candidate.alive) continue;
    const int distance = manhattan_distance(attacker.position, candidate.position);
    const bool in_range = action == ActionType::Thrust ? distance <= kThrustRange
                                                       : distance <= kMeleeRange;
    if (!in_range) continue;
    if (action == ActionType::Thrust) {
      const Vec2 delta{candidate.position.x - attacker.position.x,
                       candidate.position.y - attacker.position.y};
      if (!is_forward(attacker.facing, delta)) continue;
    }
    if (action == ActionType::Sweep) {
      targets.push_back(&candidate);
    } else if (distance < best_distance) {
      nearest = &candidate;
      best_distance = distance;
    }
  }

  if (action != ActionType::Sweep && !nearest) return;
  if (action == ActionType::Sweep && targets.empty()) return;

  attacker.stats.resource -= resource_cost;
  if (action == ActionType::Sweep) {
    attacker.cooldown_ticks =
        std::max(1, attacker.stats.attack_speed_ticks * kSweepCooldownNumerator /
                         kSweepCooldownDenominator);
  } else {
    // Melee and Thrust deliberately share the same attack cooldown.
    attacker.cooldown_ticks = attacker.stats.attack_speed_ticks;
  }
  const char* action_name = action == ActionType::Melee
                                ? "melee"
                                : action == ActionType::Thrust ? "thrust" : "sweep";
  emit(EventType::AttackStarted, attacker.id, {}, {}, action_name);

  if (action != ActionType::Sweep) targets.push_back(nearest);
  for (Actor* target : targets) {
    int damage = resolve_damage(attacker, *target,
                                attacker.kind == ActorKind::Player ? equipped_attack_bonus() : 0);
    if (action == ActionType::Thrust) {
      damage = std::max(1, damage * kThrustDamageNumerator / kThrustDamageDenominator);
    } else if (action == ActionType::Sweep) {
      damage = std::max(1, damage * kSweepDamageNumerator / kSweepDamageDenominator);
    }
    target->stats.life = std::max(0, target->stats.life - damage);
    emit(EventType::DamageApplied, target->id, {}, {}, action_name, damage);
    if (target->stats.life == 0) handle_death(*target, attacker.id);
  }
  record_equipped_item_use(attacker);
}

int Simulation::resolve_damage(const Actor& attacker, const Actor& defender, int item_bonus) {
  const int raw = attacker.stats.attack + attacker.stats.strength / 2 + item_bonus +
                  attacker.war_cry_attack_bonus;
  const int mitigated = raw - defender.stats.defense / 2;
  return std::max(1, mitigated);
}

void Simulation::record_equipped_item_use(Actor& attacker) {
  if (attacker.kind != ActorKind::Player || !scion_.alive) return;
  for (auto& item : scion_.carried_items) {
    if (!item.equipped) continue;
    item.use_count += 1;
    item.history.push_back("used at tick " + std::to_string(tick_));
    emit(EventType::ItemHistoryUpdated, attacker.id, item.id, {}, "use", item.use_count);
    break;
  }
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
  if (!instance_.active || !scion_.alive) return;

  const bool item_registered =
      std::find(instance_.ground_item_ids.begin(), instance_.ground_item_ids.end(), item_id) !=
      instance_.ground_item_ids.end();
  const bool trophy_registered =
      std::find(instance_.ground_trophy_ids.begin(), instance_.ground_trophy_ids.end(), item_id) !=
      instance_.ground_trophy_ids.end();

  // The active instance's ID lists are the authority boundary. Even if a
  // stale ground vector entry remains due to a caller retaining an old
  // reference, it is not pickable once its instance has retired.
  if (!item_registered && !trophy_registered) return;

  auto item = std::find_if(ground_items_.begin(), ground_items_.end(),
                           [&](const Item& value) { return value.id == item_id; });
  if (!item_registered || item == ground_items_.end()) {
    if (!trophy_registered) return;
    auto trophy = std::find_if(ground_trophies_.begin(), ground_trophies_.end(),
                               [&](const Trophy& value) { return value.id == item_id; });
    if (trophy == ground_trophies_.end()) return;
    scion_.carried_trophies.push_back(*trophy);
    emit(EventType::TrophyPickedUp, scion_.actor_id, {}, trophy->id);
    ground_trophies_.erase(trophy);
    instance_.ground_trophy_ids.erase(std::remove(instance_.ground_trophy_ids.begin(),
                                                  instance_.ground_trophy_ids.end(), item_id),
                                      instance_.ground_trophy_ids.end());
    resurfaced_trophy_ids_.erase(
        std::remove(resurfaced_trophy_ids_.begin(), resurfaced_trophy_ids_.end(), item_id),
        resurfaced_trophy_ids_.end());
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
  retire_instance();
  instance_ = {};
  instance_.active = true;
  instance_.route_id = route_id;
  spawn_enemy();
  for (const auto& relic : pending_relic_items_) {
    ground_items_.push_back(relic);
    instance_.ground_item_ids.push_back(relic.id);
  }
  pending_relic_items_.clear();
  for (const auto& trophy : pending_relic_trophies_) {
    ground_trophies_.push_back(trophy);
    instance_.ground_trophy_ids.push_back(trophy.id);
    resurfaced_trophy_ids_.push_back(trophy.id);
  }
  pending_relic_trophies_.clear();
  emit(EventType::InstanceEntered, scion_.actor_id, {}, {}, route_id);
  if (seasonal_mechanic_) seasonal_mechanic_->on_instance_enter(*this, instance_);
}

void Simulation::retire_instance() {
  // Floor value belongs to the instance that produced it. Leaving by
  // extraction, death, or a route transition abandons all uncollected floor
  // items/trophies; carried value is handled separately by extraction or the
  // death recovery path. Preserve route_id until the caller has recorded any
  // terminal event that needs its provenance.
  // A surfaced relic was removed from the recovery pool when it entered this
  // instance. Keep it recoverable exactly once if it is abandoned here;
  // ordinary drops have no relic marker and are simply lost. The same rule
  // applies to trophies tracked as surfaced recovery candidates. Pending
  // candidates are reattached to the next instance on entry, so this is not a
  // second pool registration or an ordinary floor-leftover escape hatch.
  for (const auto& item : ground_items_) {
    if (item.relic_candidate &&
        std::find_if(pending_relic_items_.begin(), pending_relic_items_.end(),
                     [&](const Item& candidate) { return candidate.id == item.id; }) ==
            pending_relic_items_.end()) {
      pending_relic_items_.push_back(item);
    }
  }
  for (const auto& trophy : ground_trophies_) {
    if (std::find(resurfaced_trophy_ids_.begin(), resurfaced_trophy_ids_.end(), trophy.id) ==
        resurfaced_trophy_ids_.end()) {
      continue;
    }
    if (std::find_if(pending_relic_trophies_.begin(), pending_relic_trophies_.end(),
                     [&](const Trophy& candidate) { return candidate.id == trophy.id; }) ==
        pending_relic_trophies_.end()) {
      pending_relic_trophies_.push_back(trophy);
    }
  }
  ground_items_.clear();
  ground_trophies_.clear();
  resurfaced_trophy_ids_.clear();
  instance_.ground_item_ids.clear();
  instance_.ground_trophy_ids.clear();
  instance_.active = false;
}

void Simulation::spawn_enemy() {
  const int level = instance_.route_id == "route:tin:2:0" ? 2 : 1;
  actors_.erase(std::remove_if(actors_.begin(), actors_.end(),
                               [](const Actor& value) { return value.kind == ActorKind::Monster; }),
                actors_.end());
  spawn_monster({kEnemySpawnX, 0}, level, instance_.route_id == "route:tin:2:0");
}

void Simulation::enemy_turn() {
  Actor* player = actor(scion_.actor_id);
  if (!player || !player->alive || !scion_.alive) return;
  for (auto& enemy : actors_) {
    if (enemy.kind != ActorKind::Monster || !enemy.alive) continue;

    // A scheduled elite skill owns the monster's turn until its windup
    // expires.  The target is the current player actor; if either side died
    // during the windup, discard the action rather than striking a corpse.
    if (enemy.pending_action != ActionType::Wait) {
      if (!player->alive || !scion_.alive) {
        enemy.pending_action = ActionType::Wait;
        enemy.pending_action_ticks = 0;
        continue;
      }
      if (enemy.pending_action_ticks > 0) --enemy.pending_action_ticks;
      if (enemy.pending_action_ticks == 0) {
        const ActionType pending_action = enemy.pending_action;
        enemy.pending_action = ActionType::Wait;
        resolve_actor_action(enemy, pending_action);
      }
      if (!player->alive || !scion_.alive) return;
      continue;
    }

    const Vec2 pursuit{player->position.x - enemy.position.x,
                       player->position.y - enemy.position.y};
    const Vec2 direction = quantize_direction(pursuit.x, pursuit.y);
    if (direction.x != 0 || direction.y != 0) enemy.facing = direction;
    if (enemy.cooldown_ticks > 0) continue;

    const int distance = manhattan_distance(enemy.position, player->position);
    if (enemy.elite) {
      const Vec2 delta{player->position.x - enemy.position.x,
                       player->position.y - enemy.position.y};
      // Thrust is selected by the same deterministic cone predicate that the
      // shared resolver uses.  Resource/cooldown gates are deliberately
      // checked again by resolve_actor_action at the end of the windup.
      // Keep the skill bands deterministic and reachable: at close melee
      // distance Sweep is the elite's area response, while Thrust is chosen
      // from the forward cone in the longer thrust-only band.
      if (distance > kMeleeRange && distance <= kThrustRange &&
          is_forward(enemy.facing, delta)) {
        enemy.pending_action = ActionType::Thrust;
        enemy.pending_action_ticks = kTelegraphTicks;
        emit(EventType::AttackTelegraphed, enemy.id, {}, {}, "thrust", kTelegraphTicks);
        continue;
      }
      if (distance <= kMeleeRange && enemy.stats.resource >= kSweepResourceCost) {
        enemy.pending_action = ActionType::Sweep;
        enemy.pending_action_ticks = kTelegraphTicks;
        emit(EventType::AttackTelegraphed, enemy.id, {}, {}, "sweep", kTelegraphTicks);
        continue;
      }
    }

    // Plain melee remains the original non-elite cadence. Elite monsters also
    // use it when they are in melee range but cannot fund Sweep. The shared
    // movement_delta() derivation is used by any Actor action (player WASD
    // and Dash alike); pursuit remains presentation-neutral until the native
    // collision/navigation pass owns monster locomotion.
    if (distance > kMeleeRange) continue;
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
    actor_value.stats.resource =
        std::min(actor_value.stats.resource_max,
                 actor_value.stats.resource + kResourceRegenPerTick);
    if (actor_value.cooldown_ticks > 0) --actor_value.cooldown_ticks;
    if (actor_value.war_cry_ticks_remaining > 0) {
      --actor_value.war_cry_ticks_remaining;
      if (actor_value.war_cry_ticks_remaining == 0) {
        actor_value.war_cry_attack_bonus = 0;
        emit(EventType::BuffExpired, actor_value.id, {}, {}, "war-cry", kWarCryAttackBonus);
      }
    }
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

  // Relics re-enter only through the ordinary seeded reward stream.  The pool
  // is the proof that a Scion has already died with a meaningful item, and
  // moving the oldest entry before appending it to the ground preserves the
  // single-owner invariant across pool, ground, carried, and stored state.
  if (!house_.relic_candidates.empty() && rng_.range(1, kRelicResurfaceOneIn) == 1) {
    Item relic = house_.relic_candidates.front();
    house_.relic_candidates.erase(house_.relic_candidates.begin());
    relic.history.push_back("resurfaced on route " + instance_.route_id);
    ground_items_.push_back(relic);
    instance_.ground_item_ids.push_back(relic.id);
    emit(EventType::RelicResurfaced, {}, relic.id, {}, instance_.route_id);
    record_legend("relic_resurfaced", relic.id, "route=" + instance_.route_id, {},
                  instance_.route_id);
  }

  // Trophies carried through a death use the same deterministic re-entry
  // cadence, but remain a separate recoverable pool rather than durable
  // House storage. This keeps extraction risk meaningful while preserving
  // every trophy's identity and single ownership.
  if (!house_.lost_trophies.empty() && rng_.range(1, kRelicResurfaceOneIn) == 1) {
    Trophy trophy = house_.lost_trophies.front();
    house_.lost_trophies.erase(house_.lost_trophies.begin());
    ground_trophies_.push_back(trophy);
    resurfaced_trophy_ids_.push_back(trophy.id);
    instance_.ground_trophy_ids.push_back(trophy.id);
    emit(EventType::TrophyResurfaced, {}, {}, trophy.id, instance_.route_id);
    record_legend("trophy_resurfaced", trophy.id, "route=" + instance_.route_id, {},
                  instance_.route_id);
  }

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
  actor_value.pending_action = ActionType::Wait;
  actor_value.pending_action_ticks = 0;
  if (actor_value.kind == ActorKind::Player) {
    // All elite windups target the current player.  Death cancels every
    // scheduled strike before the next enemy turn can resolve it.
    for (auto& candidate : actors_) {
      if (candidate.kind == ActorKind::Monster) {
        candidate.pending_action = ActionType::Wait;
        candidate.pending_action_ticks = 0;
      }
    }
  }
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
    if (instance_.active) {
      drop_reward();
      const bool living_monster_remains = std::any_of(
          actors_.begin(), actors_.end(), [](const Actor& candidate) {
            return candidate.kind == ActorKind::Monster && candidate.alive;
          });
      if (!living_monster_remains) clear_route_and_unlock_children();
    }
    return;
  }
  scion_.alive = false;
  scion_.level = actor_value.stats.level;
  // Every carried item remains a future possibility. Equipped gear keeps the
  // historical registration wording; pack items receive an equivalent route
  // marker so identity/history survive the enlarged recovery pool.
  for (const auto& carried : scion_.carried_items) {
    Item relic = carried;
    relic.relic_candidate = true;
    const bool was_equipped = carried.equipped ||
                              (actor_value.equipped_item_id &&
                               *actor_value.equipped_item_id == carried.id);
    relic.history.push_back(was_equipped
                                ? "registered after Scion death"
                                : "lost at " + instance_.route_id + ", awaiting recovery");
    house_.relic_candidates.push_back(relic);
    record_legend("relic_candidate", relic.id, "name=" + relic.name, killer_id,
                  instance_.route_id);
  }
  for (const auto& trophy : scion_.carried_trophies) {
    house_.lost_trophies.push_back(trophy);
    record_legend("trophy_candidate", trophy.id, "name=" + trophy.name, killer_id,
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
  retire_instance();
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
  retire_instance();
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

namespace {

// The v1 format is line-oriented rather than a dependency-heavy JSON parser.
// Every string is hex encoded, making the grammar unambiguous and the output
// byte-stable across platforms.  Unknown keys are intentionally ignored.
constexpr std::uint64_t kSnapshotSchemaVersion = 1;

using SnapshotFields = std::map<std::string, std::string>;

std::string encode_text(const std::string& value) {
  static constexpr char digits[] = "0123456789abcdef";
  std::string encoded;
  encoded.reserve(value.size() * 2);
  for (const unsigned char byte : value) {
    encoded.push_back(digits[byte >> 4U]);
    encoded.push_back(digits[byte & 0x0fU]);
  }
  return encoded;
}

std::string decode_text(const std::string& value) {
  if (value.size() % 2 != 0) throw std::runtime_error("invalid snapshot string");
  std::string decoded;
  decoded.reserve(value.size() / 2);
  auto nibble = [](char digit) -> int {
    if (digit >= '0' && digit <= '9') return digit - '0';
    if (digit >= 'a' && digit <= 'f') return digit - 'a' + 10;
    if (digit >= 'A' && digit <= 'F') return digit - 'A' + 10;
    return -1;
  };
  for (std::size_t i = 0; i < value.size(); i += 2) {
    const int high = nibble(value[i]);
    const int low = nibble(value[i + 1]);
    if (high < 0 || low < 0) throw std::runtime_error("invalid snapshot string");
    decoded.push_back(static_cast<char>((high << 4) | low));
  }
  return decoded;
}

void put_field(std::ostringstream& output, const std::string& key, const std::string& value) {
  output << key << '=' << value << '\n';
}

void put_text(std::ostringstream& output, const std::string& key, const std::string& value) {
  put_field(output, key, encode_text(value));
}

template <typename Number>
void put_number(std::ostringstream& output, const std::string& key, Number value) {
  put_field(output, key, std::to_string(value));
}

void put_bool(std::ostringstream& output, const std::string& key, bool value) {
  put_field(output, key, value ? "1" : "0");
}

void put_texts(std::ostringstream& output, const std::string& key,
               const std::vector<std::string>& values) {
  put_number(output, key + ".count", values.size());
  for (std::size_t index = 0; index < values.size(); ++index) {
    put_text(output, key + "." + std::to_string(index), values[index]);
  }
}

void put_item(std::ostringstream& output, const std::string& key, const Item& item) {
  put_text(output, key + ".id", item.id);
  put_text(output, key + ".name", item.name);
  put_number(output, key + ".attackBonus", item.attack_bonus);
  put_text(output, key + ".ownerId", item.owner_id);
  put_number(output, key + ".useCount", item.use_count);
  put_bool(output, key + ".equipped", item.equipped);
  put_bool(output, key + ".relicCandidate", item.relic_candidate);
  put_texts(output, key + ".history", item.history);
}

void put_items(std::ostringstream& output, const std::string& key,
               const std::vector<Item>& values) {
  put_number(output, key + ".count", values.size());
  for (std::size_t index = 0; index < values.size(); ++index) {
    put_item(output, key + "." + std::to_string(index), values[index]);
  }
}

void put_trophy(std::ostringstream& output, const std::string& key, const Trophy& trophy) {
  put_text(output, key + ".id", trophy.id);
  put_text(output, key + ".name", trophy.name);
}

void put_trophies(std::ostringstream& output, const std::string& key,
                  const std::vector<Trophy>& values) {
  put_number(output, key + ".count", values.size());
  for (std::size_t index = 0; index < values.size(); ++index) {
    put_trophy(output, key + "." + std::to_string(index), values[index]);
  }
}

void put_scion(std::ostringstream& output, const std::string& key, const Scion& scion) {
  put_text(output, key + ".id", scion.id);
  put_text(output, key + ".name", scion.name);
  put_number(output, key + ".level", scion.level);
  put_bool(output, key + ".alive", scion.alive);
  put_text(output, key + ".actorId", scion.actor_id);
  put_trophies(output, key + ".carriedTrophies", scion.carried_trophies);
  put_items(output, key + ".carriedItems", scion.carried_items);
  put_texts(output, key + ".deeds", scion.deeds);
}

void put_legend(std::ostringstream& output, const std::string& key, const LegendEntry& legend) {
  put_number(output, key + ".ordinal", legend.ordinal);
  put_number(output, key + ".tick", legend.tick);
  put_text(output, key + ".scionId", legend.scion_id);
  put_text(output, key + ".scionName", legend.scion_name);
  put_text(output, key + ".kind", legend.kind);
  put_text(output, key + ".subject", legend.subject);
  put_text(output, key + ".detail", legend.detail);
  put_text(output, key + ".killerId", legend.killer_id);
  put_text(output, key + ".routeId", legend.route_id);
  put_bool(output, key + ".founding", legend.founding);
}

SnapshotFields parse_fields(const std::vector<std::uint8_t>& bytes) {
  const std::string text(bytes.begin(), bytes.end());
  std::istringstream input(text);
  SnapshotFields fields;
  std::string line;
  while (std::getline(input, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    const std::size_t separator = line.find('=');
    if (separator == std::string::npos || separator == 0) continue;
    // First occurrence wins, making an appended unknown/duplicate field
    // harmless and preserving the canonical value emitted by snapshot().
    fields.emplace(line.substr(0, separator), line.substr(separator + 1));
  }
  return fields;
}

std::string field(const SnapshotFields& fields, const std::string& key,
                  const std::string& fallback = {}) {
  const auto found = fields.find(key);
  return found == fields.end() ? fallback : found->second;
}

std::string required_field(const SnapshotFields& fields, const std::string& key) {
  const auto found = fields.find(key);
  if (found == fields.end()) throw std::runtime_error("missing snapshot field: " + key);
  return found->second;
}

template <typename Number>
Number number_field(const SnapshotFields& fields, const std::string& key, Number fallback = {}) {
  const auto found = fields.find(key);
  if (found == fields.end()) return fallback;
  Number value{};
  const char* begin = found->second.data();
  const char* end = begin + found->second.size();
  const auto result = std::from_chars(begin, end, value);
  if (result.ec != std::errc{} || result.ptr != end) {
    throw std::runtime_error("invalid snapshot number: " + key);
  }
  return value;
}

template <typename Number>
Number required_number(const SnapshotFields& fields, const std::string& key) {
  required_field(fields, key);
  return number_field<Number>(fields, key);
}

bool bool_field(const SnapshotFields& fields, const std::string& key, bool fallback = false) {
  const std::string value = field(fields, key, fallback ? "1" : "0");
  if (value == "1") return true;
  if (value == "0") return false;
  throw std::runtime_error("invalid snapshot boolean: " + key);
}

bool required_bool(const SnapshotFields& fields, const std::string& key) {
  required_field(fields, key);
  return bool_field(fields, key);
}

std::string required_text(const SnapshotFields& fields, const std::string& key) {
  return decode_text(required_field(fields, key));
}

std::size_t required_count(const SnapshotFields& fields, const std::string& key) {
  const auto count = required_number<std::uint64_t>(fields, key + ".count");
  if (count > 1'000'000) throw std::runtime_error("snapshot collection is too large");
  return static_cast<std::size_t>(count);
}

std::vector<std::string> read_texts(const SnapshotFields& fields, const std::string& key) {
  std::vector<std::string> values;
  const std::size_t count = required_count(fields, key);
  values.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    values.push_back(required_text(fields, key + "." + std::to_string(index)));
  }
  return values;
}

Item read_item(const SnapshotFields& fields, const std::string& key) {
  Item item;
  item.id = required_text(fields, key + ".id");
  item.name = required_text(fields, key + ".name");
  item.attack_bonus = required_number<int>(fields, key + ".attackBonus");
  item.owner_id = required_text(fields, key + ".ownerId");
  item.use_count = required_number<int>(fields, key + ".useCount");
  item.equipped = required_bool(fields, key + ".equipped");
  item.relic_candidate = required_bool(fields, key + ".relicCandidate");
  item.history = read_texts(fields, key + ".history");
  return item;
}

std::vector<Item> read_items(const SnapshotFields& fields, const std::string& key) {
  std::vector<Item> values;
  const std::size_t count = required_count(fields, key);
  values.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    values.push_back(read_item(fields, key + "." + std::to_string(index)));
  }
  return values;
}

Trophy read_trophy(const SnapshotFields& fields, const std::string& key) {
  return {required_text(fields, key + ".id"), required_text(fields, key + ".name")};
}

std::vector<Trophy> read_trophies(const SnapshotFields& fields, const std::string& key) {
  std::vector<Trophy> values;
  const std::size_t count = required_count(fields, key);
  values.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    values.push_back(read_trophy(fields, key + "." + std::to_string(index)));
  }
  return values;
}

Scion read_scion(const SnapshotFields& fields, const std::string& key) {
  Scion scion;
  scion.id = required_text(fields, key + ".id");
  scion.name = required_text(fields, key + ".name");
  scion.level = required_number<int>(fields, key + ".level");
  scion.alive = required_bool(fields, key + ".alive");
  scion.actor_id = required_text(fields, key + ".actorId");
  scion.carried_trophies = read_trophies(fields, key + ".carriedTrophies");
  scion.carried_items = read_items(fields, key + ".carriedItems");
  scion.deeds = read_texts(fields, key + ".deeds");
  return scion;
}

LegendEntry read_legend(const SnapshotFields& fields, const std::string& key) {
  LegendEntry legend;
  legend.ordinal = required_number<std::uint64_t>(fields, key + ".ordinal");
  legend.tick = required_number<std::uint64_t>(fields, key + ".tick");
  legend.scion_id = required_text(fields, key + ".scionId");
  legend.scion_name = required_text(fields, key + ".scionName");
  legend.kind = required_text(fields, key + ".kind");
  legend.subject = required_text(fields, key + ".subject");
  legend.detail = required_text(fields, key + ".detail");
  legend.killer_id = required_text(fields, key + ".killerId");
  legend.route_id = required_text(fields, key + ".routeId");
  legend.founding = required_bool(fields, key + ".founding");
  return legend;
}

}  // namespace

std::vector<std::uint8_t> snapshot(const Simulation& simulation) {
  std::ostringstream output;
  put_number(output, "schemaVersion", kSnapshotSchemaVersion);
  put_number(output, "rng.state", simulation.rng_.state);
  put_number(output, "rng.serial", simulation.rng_.serial);
  put_number(output, "tick", simulation.tick_);
  put_number(output, "nextLegendOrdinal", simulation.next_legend_ordinal_);

  put_text(output, "house.id", simulation.house_.id);
  put_text(output, "house.name", simulation.house_.name);
  put_number(output, "house.routes.count", simulation.house_.routes.size());
  for (std::size_t index = 0; index < simulation.house_.routes.size(); ++index) {
    const auto& route = simulation.house_.routes[index];
    const std::string key = "house.routes." + std::to_string(index);
    put_text(output, key + ".id", route.id);
    put_text(output, key + ".parentId", route.parent_id);
    put_bool(output, key + ".optional", route.optional);
    put_texts(output, key + ".children", route.children);
  }
  put_texts(output, "house.unlockedRoutes", simulation.house_.unlocked_routes);
  put_texts(output, "house.clearedRoutes", simulation.house_.cleared_routes);
  put_texts(output, "house.specializations", simulation.house_.specializations);
  put_trophies(output, "house.storedTrophies", simulation.house_.stored_trophies);
  put_items(output, "house.storedItems", simulation.house_.stored_items);
  put_items(output, "house.relicCandidates", simulation.house_.relic_candidates);
  put_trophies(output, "house.lostTrophies", simulation.house_.lost_trophies);
  put_texts(output, "house.seasonalRewards", simulation.house_.seasonal_rewards);
  put_number(output, "house.legends.count", simulation.house_.legends.size());
  for (std::size_t index = 0; index < simulation.house_.legends.size(); ++index) {
    put_legend(output, "house.legends." + std::to_string(index), simulation.house_.legends[index]);
  }
  put_bool(output, "house.campaignComplete", simulation.house_.campaign_complete);

  put_scion(output, "scion", simulation.scion_);
  put_number(output, "fallenScions.count", simulation.fallen_scions_.size());
  for (std::size_t index = 0; index < simulation.fallen_scions_.size(); ++index) {
    put_scion(output, "fallenScions." + std::to_string(index), simulation.fallen_scions_[index]);
  }

  // Active floor state is deliberately absent.  Relic/trophy recovery
  // candidates that were already surfaced are returned to the pending pools,
  // exactly as retire_instance() does at an abandonment boundary.
  std::vector<Item> pending_items = simulation.pending_relic_items_;
  for (const auto& item : simulation.ground_items_) {
    if (!item.relic_candidate ||
        std::find_if(pending_items.begin(), pending_items.end(), [&](const Item& candidate) {
          return candidate.id == item.id;
        }) != pending_items.end()) {
      continue;
    }
    pending_items.push_back(item);
  }
  std::vector<Trophy> pending_trophies = simulation.pending_relic_trophies_;
  for (const auto& trophy : simulation.ground_trophies_) {
    if (std::find(simulation.resurfaced_trophy_ids_.begin(),
                  simulation.resurfaced_trophy_ids_.end(), trophy.id) ==
            simulation.resurfaced_trophy_ids_.end() ||
        std::find_if(pending_trophies.begin(), pending_trophies.end(),
                     [&](const Trophy& candidate) { return candidate.id == trophy.id; }) !=
            pending_trophies.end()) {
      continue;
    }
    pending_trophies.push_back(trophy);
  }
  put_items(output, "pendingRelicItems", pending_items);
  put_trophies(output, "pendingRelicTrophies", pending_trophies);

  const std::string text = output.str();
  return {text.begin(), text.end()};
}

Simulation restore(const std::vector<std::uint8_t>& bytes) {
  const SnapshotFields fields = parse_fields(bytes);
  if (required_number<std::uint64_t>(fields, "schemaVersion") != kSnapshotSchemaVersion) {
    throw std::runtime_error("unsupported or missing snapshot schemaVersion");
  }

  // Construct once to obtain the regular player actor shape, then replace all
  // state with the explicitly serialized durable fields.  No constructor RNG
  // output or event survives this reset.
  Simulation simulation(0);
  simulation.rng_.state = required_number<std::uint64_t>(fields, "rng.state");
  simulation.rng_.serial = required_number<std::uint64_t>(fields, "rng.serial");
  simulation.tick_ = required_number<std::uint64_t>(fields, "tick");
  simulation.next_legend_ordinal_ = required_number<std::uint64_t>(fields, "nextLegendOrdinal");

  simulation.house_ = {};
  simulation.house_.id = required_text(fields, "house.id");
  simulation.house_.name = required_text(fields, "house.name");
  const std::size_t route_count = required_count(fields, "house.routes");
  simulation.house_.routes.reserve(route_count);
  for (std::size_t index = 0; index < route_count; ++index) {
    const std::string key = "house.routes." + std::to_string(index);
    RouteNode route;
    route.id = required_text(fields, key + ".id");
    route.parent_id = required_text(fields, key + ".parentId");
    route.optional = required_bool(fields, key + ".optional");
    route.children = read_texts(fields, key + ".children");
    simulation.house_.routes.push_back(std::move(route));
  }
  simulation.house_.unlocked_routes = read_texts(fields, "house.unlockedRoutes");
  simulation.house_.cleared_routes = read_texts(fields, "house.clearedRoutes");
  simulation.house_.specializations = read_texts(fields, "house.specializations");
  simulation.house_.stored_trophies = read_trophies(fields, "house.storedTrophies");
  simulation.house_.stored_items = read_items(fields, "house.storedItems");
  simulation.house_.relic_candidates = read_items(fields, "house.relicCandidates");
  simulation.house_.lost_trophies = read_trophies(fields, "house.lostTrophies");
  simulation.house_.seasonal_rewards = read_texts(fields, "house.seasonalRewards");
  const std::size_t legend_count = required_count(fields, "house.legends");
  simulation.house_.legends.reserve(legend_count);
  for (std::size_t index = 0; index < legend_count; ++index) {
    simulation.house_.legends.push_back(
        read_legend(fields, "house.legends." + std::to_string(index)));
  }
  simulation.house_.campaign_complete = required_bool(fields, "house.campaignComplete");

  simulation.scion_ = read_scion(fields, "scion");
  const std::size_t fallen_count = required_count(fields, "fallenScions");
  simulation.fallen_scions_.reserve(fallen_count);
  for (std::size_t index = 0; index < fallen_count; ++index) {
    simulation.fallen_scions_.push_back(
        read_scion(fields, "fallenScions." + std::to_string(index)));
  }
  simulation.pending_relic_items_ = read_items(fields, "pendingRelicItems");
  simulation.pending_relic_trophies_ = read_trophies(fields, "pendingRelicTrophies");

  simulation.instance_ = {};
  simulation.ground_items_.clear();
  simulation.ground_trophies_.clear();
  simulation.resurfaced_trophy_ids_.clear();
  simulation.events_.clear();
  simulation.seasonal_mechanic_ = nullptr;
  simulation.actors_.clear();
  Actor player{simulation.scion_.actor_id,
               ActorKind::Player,
               player_stats(),
               {0, 0},
               simulation.scion_.alive,
               0,
               std::nullopt};
  player.stats.level = std::max(1, simulation.scion_.level);
  for (const auto& item : simulation.scion_.carried_items) {
    if (item.equipped) {
      player.equipped_item_id = item.id;
      break;
    }
  }
  simulation.actors_.push_back(std::move(player));
  return simulation;
}

}  // namespace verdigris
