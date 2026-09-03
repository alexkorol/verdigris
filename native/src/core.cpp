#include "verdigris/core.hpp"

#include <cmath>
#include <cctype>
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

Command Command::unequip() {
  return {CommandType::Unequip, 0, 0, ActionType::Wait, {}};
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

Actor Simulation::make_monster(Vec2 position, int level, bool elite) {
  const int bounded_level = std::max(1, level);
  return Actor{rng_.token("actor"), ActorKind::Monster, enemy_stats(bounded_level), position,
               true, 0, std::nullopt, elite};
}

std::string Simulation::spawn_monster(Vec2 position, int level, bool elite) {
  Actor enemy = make_monster(position, level, elite);
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
  if (command.type == CommandType::Unequip) resolve_unequip();
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

// Remove the equipped item without swapping in another: clear the item flag
// and the actor's equipped reference so the stats readout drops the bonus.
void Simulation::resolve_unequip() {
  if (!scion_.alive) return;
  Actor* player = actor(scion_.actor_id);
  if (!player) return;
  const std::string removed_id = player->equipped_item_id.value_or(std::string{});
  for (auto& carried : scion_.carried_items) carried.equipped = false;
  player->equipped_item_id.reset();
  if (!removed_id.empty()) {
    emit(EventType::ItemHistoryUpdated, scion_.actor_id, removed_id, {}, "unequip");
  }
}

void Simulation::resolve_enter(const std::string& route_id) {
  if (!house_.route_unlocked(route_id) || !scion_.alive) return;
  retire_instance();
  instance_ = {};
  instance_.active = true;
  instance_.route_id = route_id;
  instance_.phase = ExpeditionPhase::SlayWardens;
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
  // Unmaterialized pack wardens belong to the instance that summoned them.
  // Leaving by extraction, death, or a route transition discards the roster
  // exactly like the other floor state; a fresh entry rebuilds it.
  pending_wave_.clear();
  wave_materialization_tick_ = 0;
  instance_.active = false;
}

void Simulation::spawn_enemy() {
  const bool deep = instance_.route_id == "route:tin:2:0";
  const int level = deep ? 2 : 1;
  actors_.erase(std::remove_if(actors_.begin(), actors_.end(),
                               [](const Actor& value) { return value.kind == ActorKind::Monster; }),
                actors_.end());
  pending_wave_.clear();
  wave_materialization_tick_ = 0;
  spawn_monster({kEnemySpawnX, 0}, level, deep);
  // The first expedition is a Warden pack, not a single sentry. The entry
  // warden holds the D-114 spawn point; its two pack mates wait in the
  // roster and materialize together one telegraph window after the entry
  // warden falls, so the player faces a converged normal/elite pack. Every
  // offset reuses the shared melee range, so the pack stays on
  // the authoritative world-scale table with no new balance numbers: the
  // elite anchors one melee range deeper on the approach line, and the last
  // normal flanks one melee range off it. The deep route keeps its single
  // level-2 elite identity.
  if (!deep) {
    pending_wave_.push_back(
        make_monster({kEnemySpawnX + kMeleeRange, 0}, level, true));
    pending_wave_.push_back(
        make_monster({kEnemySpawnX + kMeleeRange, kMeleeRange}, level, false));
  }
}

void Simulation::materialize_wave() {
  if (pending_wave_.empty() || wave_materialization_tick_ == 0 ||
      tick_ < wave_materialization_tick_) {
    return;
  }
  // The owed pack arrives as one body: every entry still waiting in the
  // roster steps onto its deterministic anchor at the same telegraph
  // deadline, so the reinforcement reads as a converging pack rather than a
  // queue of serial single-target duels.
  for (const Actor& monster : pending_wave_) {
    actors_.push_back(monster);
  }
  pending_wave_.clear();
  wave_materialization_tick_ = 0;
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
  materialize_wave();
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
      // A fallen warden alerts the rest of its pack: while roster entries
      // remain, the entire remaining pack materializes together one
      // telegraph window after this death. Pack-clear progression therefore
      // waits for both the living wardens and any unmaterialized roster
      // before flipping the objective.
      if (!pending_wave_.empty()) {
        wave_materialization_tick_ = tick_ + kTelegraphTicks;
      }
      const bool pack_remains = !pending_wave_.empty() ||
                                std::any_of(actors_.begin(), actors_.end(),
                                            [](const Actor& candidate) {
                                              return candidate.kind == ActorKind::Monster &&
                                                     candidate.alive;
                                            });
      if (!pack_remains) clear_route_and_unlock_children();
      // The expedition objective flips to extraction only once the floor is
      // empty of living wardens and no roster entry is still owed; a later
      // spawn_monster() seam call restores the slay objective on the next
      // resolved kill.
      const ExpeditionPhase next_phase =
          pack_remains ? ExpeditionPhase::SlayWardens : ExpeditionPhase::ExtractCarriedValue;
      if (instance_.phase != next_phase) {
        instance_.phase = next_phase;
        emit(EventType::ExpeditionPhaseChanged, scion_.actor_id, {}, {},
             next_phase == ExpeditionPhase::SlayWardens ? "slay-wardens"
                                                        : "extract-carried-value",
             static_cast<int>(next_phase));
      }
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

// ────────────────────────────────────────────────────────────────────────────
// Parity wave N2: tile-space world rules.
// ────────────────────────────────────────────────────────────────────────────

namespace tile_movement {

namespace {
// server/shared/movement.js DIRECTION_VECTORS.  Diagonals are normalised with
// the same hypot() the JS uses, then scaled by PLAYER_MOVE_DISTANCE.
const std::map<std::string, Vec2>& direction_vectors() {
  static const std::map<std::string, Vec2> vectors = {
      {"right", {1, 0}},  {"left", {-1, 0}},   {"up", {0, -1}},  {"down", {0, 1}},
      {"up-right", {1, -1}}, {"down-right", {1, 1}}, {"up-left", {-1, -1}}, {"down-left", {-1, 1}},
  };
  return vectors;
}
}  // namespace

std::optional<WorldPosition> movement_delta(const std::string& direction) {
  const auto it = direction_vectors().find(direction);
  if (it == direction_vectors().end()) return std::nullopt;
  const double length = std::hypot(static_cast<double>(it->second.x), static_cast<double>(it->second.y));
  return WorldPosition{it->second.x / length * kMoveDistance,
                       it->second.y / length * kMoveDistance};
}

double round_position(double value) {
  const double scale = 1000000.0;  // 10 ** POSITION_PRECISION
  const double rounded = std::round(value * scale) / scale;
  const double nearest = std::round(rounded);
  return std::abs(rounded - nearest) <= 2e-6 ? nearest : rounded;
}

Vec2 occupied_tile(const WorldPosition& position) {
  return Vec2{static_cast<int>(std::round(position.x)),
              static_cast<int>(std::round(position.y))};
}

}  // namespace tile_movement

bool TileGrid::in_bounds(int x, int y) const {
  return x >= 0 && y >= 0 && x < width && y < height;
}

bool TileGrid::walkable_at(int x, int y) const {
  if (!in_bounds(x, y)) return false;
  return walkable[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
                  static_cast<std::size_t>(x)] != 0;
}

const std::vector<ZoneDescriptor>& adventure_zones() {
  // party.js ADVENTURE_ZONES, row for row.
  static const std::vector<ZoneDescriptor> zones = {
      {"old-barrow", "The Old Barrow", "dungeon", "warren"},
      {"verdant-grove", "Verdant Grove", "grove", "clearings"},
      {"sunken-colonnade", "Sunken Colonnade", "crypt", "gauntlet"},
      {"weir-crypt", "Weir Crypt", "crypt", "warren"},
      {"the-wilds", "The Wilds", "wilds", "clearings"},
      {"marsh-of-reeds", "Marsh of Reeds", "marsh", "clearings"},
  };
  return zones;
}

bool is_zone_template(const std::string& template_id) {
  for (const auto& zone : adventure_zones()) {
    if (zone.template_id == template_id) return true;
  }
  return false;
}

bool is_zone_layout(const std::string& layout) {
  return layout == "warren" || layout == "clearings" || layout == "gauntlet";
}

namespace {

// N2 stub geometry.  The JS server generates floors from seeded template
// tables; the playtest scenarios only exercise walkability around the spawn,
// both stair tiles, and monster population, so the generated floor keeps a
// protected spawn clearing and layout-shaped obstacle bodies.  N3+ replaces
// this with the real generator.
constexpr int kInstanceWidth = 40;
constexpr int kInstanceHeight = 40;
constexpr Vec2 kStairsUp{5, 20};
constexpr Vec2 kStairsDown{34, 20};
constexpr Vec2 kSpawn{6, 20};
// D-114 N3 combat table: trash life 30, player strike 18, 350 ms cadence;
// boss telegraph radius 2 tiles / 1000 ms readable window. These values are
// intentionally named here so scenario feel changes cannot hide as literals.
constexpr int kInstanceMonsterCount = 20;
constexpr int kN3TrashLife = 30;
constexpr int kN3PlayerDamage = 18;
constexpr int kN3PlayerAttackIntervalMs = 350;
constexpr int kN3SweepAttackIntervalMs = 525;
// The local presentation's 143-unit melee reach spans roughly three 48-unit
// protocol tiles; thrust keeps its authored 1.5x reach rounded outward.
constexpr int kN3MeleeRangeTiles = 3;
constexpr int kN3ThrustRangeTiles = 5;
constexpr int kN3SweepRangeTiles = 3;
constexpr int kN3MonsterDamage = 5;
constexpr int kN3BossLife = 120;
constexpr int kN3BossDamage = 12;
constexpr int kN3BossTelegraphRadius = 2;
constexpr int kN3BossTelegraphWindowMs = 1000;
constexpr int kTownSize = 200;

std::uint64_t fnv1a(const std::string& text, std::uint64_t seed) {
  std::uint64_t hash = seed ? seed : 1469598103934665603ULL;
  for (unsigned char ch : text) {
    hash = (hash ^ ch) * 1099511628211ULL;
  }
  return hash;
}

bool in_spawn_clearing(int x, int y) {
  // Protected bubble around the entry: no obstacles and no monsters, so the
  // first steps in any direction behave exactly like town.
  return x >= 2 && x <= 10 && y >= 16 && y <= 24;
}

}  // namespace

WorldSimulation::WorldSimulation(std::uint64_t seed, std::string player_uuid)
    : seed_(seed), player_uuid_(std::move(player_uuid)) {
  // N2 stub: town collision geometry is an open field.  The town login spawn
  // area the scenarios walk (38,115 +/- a few tiles) is open in the real map
  // too; porting the town tile tables is N3+ work documented in the report.
  grid_.width = kTownSize;
  grid_.height = kTownSize;
  grid_.walkable.assign(static_cast<std::size_t>(kTownSize) * kTownSize, 1);
}

bool WorldSimulation::can_move_to(double target_x, double target_y) const {
  const int tile_x = static_cast<int>(std::round(target_x));
  const int tile_y = static_cast<int>(std::round(target_y));
  if (!grid_.in_bounds(tile_x, tile_y)) return false;
  for (const auto& monster : monsters_) {
    if (monster.alive && monster.x == tile_x && monster.y == tile_y) return false;
  }
  return grid_.walkable_at(tile_x, tile_y);
}

bool WorldSimulation::is_blocked(const WorldPosition& origin, const WorldPosition& delta) const {
  if (!can_move_to(origin.x + delta.x, origin.y + delta.y)) return true;
  const bool diagonal = delta.x != 0.0 && delta.y != 0.0;
  if (diagonal) {
    // movement-handler.js: a diagonal is blocked only when BOTH orthogonal
    // neighbours are unwalkable.
    const bool horizontal = can_move_to(origin.x + delta.x, origin.y);
    const bool vertical = can_move_to(origin.x, origin.y + delta.y);
    if (!horizontal && !vertical) return true;
  }
  return false;
}

void WorldSimulation::register_step(const std::string& direction, int duration_ms, bool blocked,
                                    std::int64_t now_ms) {
  last_step_.sequence += 1;
  last_step_.started_at_ms = now_ms;
  last_step_.duration_ms = duration_ms;
  last_step_.direction = direction;
  last_step_.blocked = blocked;
}

bool WorldSimulation::apply_movement_sample(const std::string& direction, std::int64_t now_ms) {
  const auto delta = tile_movement::movement_delta(direction);
  if (!delta) return false;

  // movement-handler.js setFacing: diagonals collapse onto their horizontal
  // component so the run animation has a stable left/right read.
  if (direction.find("left") != std::string::npos) facing_ = "left";
  else if (direction.find("right") != std::string::npos) facing_ = "right";
  else facing_ = direction;

  if (is_blocked(position_, *delta)) {
    register_step(direction, 0, true, now_ms);
    return false;
  }

  const Vec2 previous_tile = tile_movement::occupied_tile(position_);
  position_.x = tile_movement::round_position(position_.x + delta->x);
  position_.y = tile_movement::round_position(position_.y + delta->y);
  register_step(direction, static_cast<int>(tile_movement::kSampleMs), false, now_ms);

  const Vec2 current_tile = tile_movement::occupied_tile(position_);
  if (current_tile.x != previous_tile.x || current_tile.y != previous_tile.y) {
    check_stair_transition();
  }
  return true;
}

bool WorldSimulation::apply_dash(const std::string& direction, std::int64_t now_ms) {
  bool moved = false;
  const std::string starting_scene = scene_id_;
  for (int step = 0; step < kDashMovementTicks; ++step) {
    if (!apply_movement_sample(
            direction, now_ms + step * static_cast<std::int64_t>(tile_movement::kSampleMs))) {
      break;
    }
    moved = true;
    // A stair transition owns the remainder of the action; never carry dash
    // momentum into a newly-created floor or back through the town gate.
    if (scene_id_ != starting_scene) break;
  }
  return moved;
}

void WorldSimulation::teleport(int x, int y, std::int64_t now_ms) {
  // dev.js dev:teleport floors onto the target tile and outranks any
  // in-flight client interpolation with a fresh zero-duration step.
  position_.x = static_cast<double>(x);
  position_.y = static_cast<double>(y);
  register_step("", 0, false, now_ms);
  check_stair_transition();
}

void WorldSimulation::check_stair_transition() {
  if (!in_instance()) return;
  const Vec2 tile = tile_movement::occupied_tile(position_);
  // party.js checkStairTransitions: stairsDown descends a floor, stairsUp
  // climbs — and only floor 1's climb returns to the surface.
  if (tile.x == metadata_.stairs_down.x && tile.y == metadata_.stairs_down.y) {
    if (block_stairs_down_) return;  // "No road holds past a living Warden."
    transition_floor(metadata_.depth + 1);
    return;
  }
  if (tile.x == metadata_.stairs_up.x && tile.y == metadata_.stairs_up.y) {
    if (metadata_.depth <= 1 || stairs_up_returns_to_town_) {
      // world-web: a node's entry waymark walks back to the Crossroads from
      // any stage of the road.
      return_to_town();
    } else {
      transition_floor(metadata_.depth - 1);
    }
  }
}

void WorldSimulation::return_to_surface() {
  if (!in_instance()) return;
  return_to_town();
}

void WorldSimulation::reset_to_town() {
  has_pre_instance_ = false;
  pre_instance_scene_id_.clear();
  return_to_town();
}

void WorldSimulation::return_to_town() {
  scene_type_ = "town";
  scene_id_ = "town:verdigris";
  scene_name_ = "Verdigris";
  metadata_ = InstanceMetadata{};
  monsters_.clear();
  // N4: floor ground lists retire with the floor (JS scene retirement); the
  // town list was stashed on entry and comes back exactly as left.
  ground_items_.clear();
  ground_items_ = std::move(town_ground_items_);
  town_ground_items_.clear();
  active_target_.clear();
  pending_player_skill_.clear();
  auto_player_melee_ = false;
  grid_.width = kTownSize;
  grid_.height = kTownSize;
  grid_.walkable.assign(static_cast<std::size_t>(kTownSize) * kTownSize, 1);
  if (has_pre_instance_) {
    position_ = pre_instance_position_;
    scene_id_ = pre_instance_scene_id_.empty() ? scene_id_ : pre_instance_scene_id_;
  } else {
    position_ = WorldPosition{38.0, 115.0};
  }
  has_pre_instance_ = false;
  pre_instance_scene_id_.clear();
  clear_expedition_tuning();
}

std::string WorldSimulation::zone_display_name(const std::string& template_id,
                                               const std::string& layout, int depth) {
  std::string name;
  for (const auto& zone : adventure_zones()) {
    if (zone.template_id == template_id && zone.layout == layout) {
      name = zone.name;
      break;
    }
  }
  if (name.empty()) {
    for (const auto& zone : adventure_zones()) {
      if (zone.template_id == template_id) {
        name = zone.name;
        break;
      }
    }
  }
  if (name.empty()) {
    name = template_id.empty() ? "Dungeon" : template_id;
    name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
  }
  if (depth > 1) {
    name += " · Floor " + std::to_string(depth);
  }
  return name;
}

void WorldSimulation::generate_instance() {
  const std::string& layout = metadata_.layout;
  const std::string effective = layout.empty() ? "warren" : layout;

  grid_.width = kInstanceWidth;
  grid_.height = kInstanceHeight;
  grid_.walkable.assign(static_cast<std::size_t>(kInstanceWidth) * kInstanceHeight, 1);

  auto block = [&](int x, int y) {
    if (grid_.in_bounds(x, y) && !in_spawn_clearing(x, y)
        && !(x == kStairsUp.x && y == kStairsUp.y)
        && !(x == kStairsDown.x && y == kStairsDown.y)) {
      grid_.walkable[static_cast<std::size_t>(y) * kInstanceWidth + x] = 0;
    }
  };

  // Border walls.
  for (int x = 0; x < kInstanceWidth; ++x) { block(x, 0); block(x, kInstanceHeight - 1); }
  for (int y = 0; y < kInstanceHeight; ++y) { block(0, y); block(kInstanceWidth - 1, y); }

  if (effective == "warren") {
    // Tight dungeon: vertical wall ribs with staggered gaps.
    for (int rib = 12; rib <= 32; rib += 6) {
      for (int y = 3; y < kInstanceHeight - 3; ++y) {
        const bool gap = (y >= 10 && y <= 12) || (y >= 26 && y <= 28) || (y >= 19 && y <= 21);
        if (!gap) block(rib, y);
      }
    }
  } else if (effective == "clearings") {
    // Open field: a few scattered 2x2 thickets.
    const Vec2 thickets[] = {{14, 9}, {25, 13}, {17, 28}, {29, 30}, {13, 33}};
    for (const auto& thicket : thickets) {
      for (int dx = 0; dx < 2; ++dx)
        for (int dy = 0; dy < 2; ++dy) block(thicket.x + dx, thicket.y + dy);
    }
  } else {  // gauntlet: a linear push down a walled corridor.
    for (int x = 2; x < kInstanceWidth - 2; ++x) {
      block(x, 14);
      block(x, 26);
    }
  }

  metadata_.stairs_up = kStairsUp;
  metadata_.stairs_down = kStairsDown;
  metadata_.spawn_points = {kSpawn};

  // Deterministic monster scatter: seeded LCG picks candidate tiles; only
  // walkable tiles well away from the entry clearing and stairs are used so
  // population never interferes with movement parity.
  monsters_.clear();
  std::uint64_t state = metadata_.seed ? metadata_.seed : 0x9e3779b97f4a7c15ULL;
  auto next = [&]() {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
  };
  const int level = (metadata_.theme == "crypt" ? 4
                   : metadata_.theme == "wilds" ? 6
                   : metadata_.theme == "marsh" ? 8
                   : 2) + expedition_level_bonus_;
  const int monster_count =
      kInstanceMonsterCount + std::min(12, expedition_extra_monsters_);
  int placed = 0;
  int attempts = 0;
  while (!spawn_suppressed_ && placed < monster_count && attempts < 4000) {
    ++attempts;
    const int x = static_cast<int>(next() % kInstanceWidth);
    const int y = static_cast<int>(next() % kInstanceHeight);
    if (!grid_.walkable_at(x, y) || in_spawn_clearing(x, y)) continue;
    if (std::abs(x - kStairsUp.x) + std::abs(y - kStairsUp.y) < 5) continue;
    if (std::abs(x - kStairsDown.x) + std::abs(y - kStairsDown.y) < 3) continue;
    bool occupied = false;
    for (const auto& monster : monsters_) {
      if (monster.x == x && monster.y == y) { occupied = true; break; }
    }
    if (occupied) continue;
    WorldMonster monster;
    monster.uuid = "monster-" + std::to_string(serial_) + "-" + std::to_string(placed);
    monster.id = metadata_.theme + "-lurker";
    monster.name = zone_display_name(metadata_.theme, "", 1) + " Lurker";
    // Named per-theme roster (owner content ruling 2026-08-31): each road
    // fields melee/ranged/buffer kinds with their own names and ids so the
    // bestiary reads as fauna, not one renamed lurker. Ids are stable
    // (theme-role); rigs and future drops key off them.
    struct RosterRow { const char* melee; const char* ranged; const char* buffer; };
    const RosterRow roster =
        metadata_.theme == "grove" ? RosterRow{"Thorn Stalker", "Sling Poacher", "Sapbinder"}
        : metadata_.theme == "crypt" ? RosterRow{"Barrow Wight", "Grave Archer", "Candle Priest"}
        : metadata_.theme == "wilds" ? RosterRow{"Ridge Wolf", "Crag Slinger", "Herd Caller"}
        : metadata_.theme == "marsh" ? RosterRow{"Mire Ghast", "Bog Spitter", "Rot Shaman"}
                                     : RosterRow{"Stone Lurker", "Flint Slinger", "Warden Caller"};
    monster.x = x;
    monster.y = y;
    // map.js: level = max(1, floor(1 + index*0.14)) + (depth-1)*2 + theme
    // bonus. Deeper floors are the authoritative difficulty wall.
    monster.level = level + (metadata_.depth - 1) * 2 + placed / 7;
    monster.life = (kN3TrashLife + (level - 2) * 5) *
                   (100 + expedition_life_percent_) / 100;
    monster.life_max = monster.life;
    // Authored pack recipes mirror map.js: crypt is melee-heavy, marsh adds
    // ranged pressure, and every biome has one support buffer. The final
    // dungeon room is replaced below by the named Old Barrow boss.
    const int role_index = placed % 6;
    if (metadata_.theme == "crypt") {
      monster.behaviour_type = (role_index == 5) ? "buffer" : (role_index == 4 ? "ranged" : "melee");
    } else if (metadata_.theme == "marsh") {
      monster.behaviour_type = (role_index == 5) ? "buffer" : (role_index % 2 ? "ranged" : "melee");
    } else {
      monster.behaviour_type = (role_index == 5) ? "buffer" : (role_index % 3 ? "melee" : "ranged");
    }
    if (monster.behaviour_type == "ranged") {
      monster.id = metadata_.theme + "-ranged";
      monster.name = roster.ranged;
      monster.life = (std::max)(6, monster.life * 8 / 10);
      monster.life_max = monster.life;
    } else if (monster.behaviour_type == "buffer") {
      monster.id = metadata_.theme + "-buffer";
      monster.name = roster.buffer;
      monster.life = (std::max)(6, monster.life * 9 / 10);
      monster.life_max = monster.life;
    } else {
      monster.id = metadata_.theme + "-melee";
      monster.name = roster.melee;
    }
    if (placed == 0 && metadata_.theme == "marsh") {
      monster.rarity = "rare";
      monster.modifiers = {"empowered"};
    }
    if (metadata_.theme == "marsh" && monster.behaviour_type != "buffer" && placed % 4 == 1) {
      monster.empowered = true;
    }
    if (placed == monster_count - 1) {
      // Every theme fields its boss (server/core/map.js THEME_MONSTERS);
      // native "dungeon" mirrors the JS "stone" theme.
      monster.boss = true;
      monster.rarity = "elite";
      if (!boss_name_override_.empty()) monster.name = boss_name_override_;
      else if (metadata_.theme == "grove") monster.name = "The Elder Oak";
      else if (metadata_.theme == "crypt") monster.name = "The Pale Sovereign";
      else if (metadata_.theme == "wilds") monster.name = "Alpha of the Wilds";
      else if (metadata_.theme == "marsh") monster.name = "The Rotfather";
      else monster.name = "Warden of the Deep";
      monster.life = (kN3BossLife + expedition_level_bonus_ * 12) *
                     (100 + expedition_life_percent_) / 100;
      monster.life_max = monster.life;
      monster.behaviour_type = "melee";
    }
    // N4 loot facts (map.js monster rewards/tags): the grove fields beasts
    // (the Beastbane scenario reads these tags); coin bounties are an
    // authored stand-in until the JS reward tables are ported (N5 note).
    if (metadata_.theme == "grove" && !monster.boss) {
      monster.tags = {"beast"};
    }
    monster.coins = 10 + monster.level * 5;
    if (monster.rarity == "elite") monster.coins *= 3;
    monsters_.push_back(std::move(monster));
    ++placed;
  }
  scatter_floor_treasure();
}

void WorldSimulation::enter_solo_instance(const std::string& template_id, const std::string& layout) {
  const std::string theme = is_zone_template(template_id) ? template_id : "dungeon";
  const std::string applied_layout = is_zone_layout(layout) ? layout : "";

  if (!in_instance() && !has_pre_instance_) {
    // First entry only: instance -> instance hops keep the original surface
    // position so the stair return lands where the party left.
    pre_instance_position_ = position_;
    pre_instance_scene_id_ = scene_id_;
    has_pre_instance_ = true;
  }

  serial_ += 1;
  metadata_ = InstanceMetadata{};
  metadata_.seed = fnv1a(theme + ":" + applied_layout, seed_);
  metadata_.theme = theme;
  metadata_.layout = applied_layout;
  metadata_.depth = 1;
  // N4: leaving town stashes its ground items; instance floors retire theirs.
  if (scene_type_ == "town") {
    town_ground_items_ = std::move(ground_items_);
    ground_items_.clear();
  } else {
    ground_items_.clear();  // instance -> instance hop retires the old floor
  }
  active_target_.clear();
  pending_player_skill_.clear();
  auto_player_melee_ = false;
  boss_warning_seen_ = false;
  next_boss_telegraph_ms_ = 0;
  generate_instance();

  scene_type_ = "instance";
  scene_id_ = "instance:" + theme + ":" + (applied_layout.empty() ? "default" : applied_layout);
  scene_name_ = zone_display_name(theme, applied_layout, 1);
  position_.x = static_cast<double>(metadata_.spawn_points.front().x);
  position_.y = static_cast<double>(metadata_.spawn_points.front().y);
}

void WorldSimulation::set_level(int level) {
  player_level_ = std::max(1, level);
}

void WorldSimulation::heal_player(int& player_life, int player_life_max) {
  player_life = player_life_max;
}

bool WorldSimulation::start_player_attack(int player_level, int player_attack,
                                          std::int64_t now_ms,
                                          const std::string& direction,
                                          const std::string& requested_skill) {
  player_level_ = std::max(1, player_level);
  const auto now = static_cast<std::uint64_t>(std::max<std::int64_t>(0, now_ms));
  if (now < next_player_attack_ms_ || !pending_player_skill_.empty()) return false;

  std::string skill = requested_skill;
  if (skill.empty() || skill == "primary-attack") skill = "melee";
  if (skill != "melee" && skill != "thrust" && skill != "sweep") return false;

  const Vec2 here = tile_movement::occupied_tile(position_);
  int aim_dx = 0, aim_dy = 0;
  if (direction.find("left") != std::string::npos) aim_dx = -1;
  if (direction.find("right") != std::string::npos) aim_dx = 1;
  if (direction.find("up") != std::string::npos) aim_dy = -1;
  if (direction.find("down") != std::string::npos) aim_dy = 1;
  const int range = skill == "thrust" ? kN3ThrustRangeTiles
                                       : skill == "sweep" ? kN3SweepRangeTiles
                                                          : kN3MeleeRangeTiles;
  WorldMonster* chosen = nullptr;
  int best = std::numeric_limits<int>::max();
  int best_priority = -1;
  int best_aim = std::numeric_limits<int>::min();
  for (auto& monster : monsters_) {
    if (!monster.alive) continue;
    const int dx = monster.x - here.x;
    const int dy = monster.y - here.y;
    const int distance = std::abs(dx) + std::abs(dy);
    if (distance > range) continue;
    const int aim = aim_dx * dx + aim_dy * dy;
    if (skill == "thrust" && aim <= 0) continue;
    const int priority = monster.boss ? 2 : monster.empowered ? 1 : 0;
    if (priority > best_priority ||
        (priority == best_priority &&
         (distance < best || (distance == best && aim > best_aim)))) {
      chosen = &monster;
      best = distance;
      best_aim = aim;
      best_priority = priority;
    }
  }
  if (!chosen) return false;
  active_target_ = chosen->uuid;
  pending_player_skill_ = skill;
  // Primary click-to-attack keeps swinging at its chosen target; named
  // abilities are discrete casts. The cooldown timestamp is never moved by
  // rejected repeat input, preserving both responsive play and anti-spam.
  auto_player_melee_ = skill == "melee";
  next_player_attack_ms_ = now +
      (skill == "sweep" ? kN3SweepAttackIntervalMs : kN3PlayerAttackIntervalMs);
  (void)player_attack;
  return true;
}

int WorldSimulation::player_cooldown_remaining_ms(std::int64_t now_ms) const {
  const auto now = static_cast<std::uint64_t>(std::max<std::int64_t>(0, now_ms));
  if (now >= next_player_attack_ms_) return 0;
  return static_cast<int>(next_player_attack_ms_ - now);
}

std::vector<WorldCombatEvent> WorldSimulation::advance_combat(int player_level,
                                                              int player_attack,
                                                              int& player_life,
                                                              int player_life_max,
                                                              std::int64_t now_ms) {
  std::vector<WorldCombatEvent> events;
  (void)player_level;
  const auto now = static_cast<std::uint64_t>(std::max<std::int64_t>(0, now_ms));
  if (active_target_.empty()) {
    const Vec2 here = tile_movement::occupied_tile(position_);
    // N5: a scion standing beside a pack member is engaged even without
    // swinging (JS monster AI). Prefer a boss, then any in-range monster.
    for (const auto& monster : monsters_) {
      if (monster.alive && monster.boss && std::abs(monster.x - here.x) <= 2
          && std::abs(monster.y - here.y) <= 2) { active_target_ = monster.uuid; break; }
    }
    // (N5 draft auto-targeted any adjacent monster; that made the PLAYER
    // swing unprompted, which breaks deterministic comparison trials. JS
    // parity: monsters engage the player; the player's swings are inputs.)
  }
  // JS monster AI: pack members adjacent to the player strike on their own
  // cooldown whether or not the player is fighting back. This is what makes
  // standing in a pack lethal (mortality / final-death flows).
  {
    const Vec2 here = tile_movement::occupied_tile(position_);
    for (auto& monster : monsters_) {
      if (!monster.alive || monster.boss) continue;
      if (std::abs(monster.x - here.x) > 1 || std::abs(monster.y - here.y) > 1) continue;
      if (monster.next_attack_ms == 0) {
        // First contact: a short, per-monster staggered windup instead of
        // the whole adjacent pack landing its opening hit on the same
        // millisecond. The synchronised burst deleted a level-1 scion
        // before the first telegraph could even read (owner ruling: the
        // first stretch must be survivable and readable).
        std::uint32_t stagger_hash = 2166136261u;
        for (const char c : monster.uuid)
          stagger_hash = (stagger_hash ^ static_cast<std::uint8_t>(c)) * 16777619u;
        monster.next_attack_ms = now + 400 + stagger_hash % 900;
        continue;
      }
      if (now < monster.next_attack_ms) continue;
      monster.next_attack_ms = now + 1200;
      // Owner balance ruling 2026-08-31: 4 + level*2 outpaced level-1 life
      // by the third simultaneous attacker; contact pressure now scales at
      // half the slope so early floors threaten without deleting.
      const int damage = std::max(
          1, (2 + monster.level) * (100 + expedition_damage_percent_) / 100);
      player_life = std::max(0, player_life - damage);
      WorldCombatEvent impact;
      impact.type = "hit";
      impact.attacker_id = monster.uuid;
      impact.attacker_name = monster.name;
      impact.target_id = player_uuid_;
      impact.amount = damage;
      impact.health = player_life;
      impact.health_max = player_life_max;
      impact.died = player_life == 0;
      events.push_back(impact);
      if (player_life == 0) break;
    }
  }
  if (active_target_.empty()) {
    pending_player_skill_.clear();
    auto_player_melee_ = false;
    return events;
  }
  WorldMonster* target = nullptr;
  for (auto& monster : monsters_) if (monster.uuid == active_target_ && monster.alive) { target = &monster; break; }
  if (!target) { active_target_.clear(); pending_player_skill_.clear(); auto_player_melee_ = false; return events; }
  { // JS combat: walking out of melee reach disengages - the swing loop must
    // not chase a target across the map (build-comparison parking relies on it).
    const Vec2 here = tile_movement::occupied_tile(position_);
    if (std::abs(target->x - here.x) > kN3ThrustRangeTiles ||
        std::abs(target->y - here.y) > kN3ThrustRangeTiles) {
      active_target_.clear();
      pending_player_skill_.clear();
      auto_player_melee_ = false;
      return events;
    }
  }
  if (pending_player_skill_.empty() && auto_player_melee_ &&
      now >= next_player_attack_ms_) {
    const Vec2 here = tile_movement::occupied_tile(position_);
    const int distance = std::abs(target->x - here.x) +
                         std::abs(target->y - here.y);
    if (distance <= kN3MeleeRangeTiles) {
      pending_player_skill_ = "melee";
      next_player_attack_ms_ = now + kN3PlayerAttackIntervalMs;
    }
  }
  if (player_attack > 0 && !pending_player_skill_.empty()) {
    const std::string skill = pending_player_skill_;
    pending_player_skill_.clear();  // one accepted request resolves once
    const Vec2 here = tile_movement::occupied_tile(position_);
    std::vector<WorldMonster*> struck;
    if (skill == "sweep") {
      for (auto& monster : monsters_) {
        if (!monster.alive) continue;
        const int distance = std::abs(monster.x - here.x) +
                             std::abs(monster.y - here.y);
        if (distance <= kN3SweepRangeTiles) struck.push_back(&monster);
      }
    } else {
      const int range = skill == "thrust" ? kN3ThrustRangeTiles
                                           : kN3MeleeRangeTiles;
      const int distance = std::abs(target->x - here.x) +
                           std::abs(target->y - here.y);
      if (distance <= range) struck.push_back(target);
    }

    bool active_target_died = false;
    for (WorldMonster* struck_target : struck) {
      // N4 hit pipeline (server/core/combat/index.js applyHitToMonster):
      // skill coefficient -> Beastbane -> critical multiplier. Sweep rolls
      // each target independently; a forced critical is consumed by the
      // first body hit, matching the single-use modifier contract.
      int base = std::max(1, player_attack > 0 ? player_attack : kN3PlayerDamage);
      if (skill == "thrust") base = std::max(1, base * 13 / 10);
      else if (skill == "sweep") base = std::max(1, base * 3 / 4);
      const bool beast = std::find(struck_target->tags.begin(),
                                   struck_target->tags.end(), "beast") !=
                         struck_target->tags.end();
      const int beastbane_percent = beast
          ? std::max(0, std::min(100, player_mods_.damage_against_beasts)) : 0;
      const int beastbane_damage = static_cast<int>(std::lround(
          base * (1.0 + beastbane_percent / 100.0)));
      bool critical = false;
      if (player_mods_.force_critical) {
        player_mods_.force_critical = false;
        critical = true;
      } else if (player_mods_.critical_chance > 0) {
        const int roll = 1 + static_cast<int>(next_world_random() % 100);
        critical = roll <= std::max(0, std::min(75, player_mods_.critical_chance));
      }
      const int damage = critical
          ? std::max(beastbane_damage + 1,
                     static_cast<int>(std::lround(beastbane_damage * 1.5)))
          : beastbane_damage;
      struck_target->life = std::max(0, struck_target->life - damage);
      WorldCombatEvent hit;
      hit.type = "hit"; hit.attacker_id = player_uuid_;
      hit.attacker_name = "Adventurer";
      hit.target_id = struck_target->uuid; hit.target_name = struck_target->name;
      hit.skill_id = skill;
      hit.amount = damage; hit.health = struck_target->life;
      hit.health_max = struck_target->life_max; hit.died = struck_target->life == 0;
      hit.base_amount = base; hit.beastbane_amount = beastbane_damage;
      hit.beastbane_percent = beastbane_percent;
      hit.beastbane = beastbane_percent > 0;
      hit.critical = critical; hit.attack_style = player_mods_.attack_style;
      events.push_back(hit);
      if (struck_target->life == 0) {
        struck_target->alive = false;
        WorldCombatEvent death = hit; death.type = "death"; events.push_back(death);
        drop_monster_loot(*struck_target, player_mods_.goods_found);
        if (struck_target->uuid == active_target_) active_target_died = true;
      }
    }
    if (active_target_died) {
      active_target_.clear();
      auto_player_melee_ = false;
      return events;
    }
  }
  // Boss mechanic: announce once, then resolve at the authored window. The
  // player's current tile is authoritative, so dev teleport genuinely dodges.
  if (target->boss) {
    if (target->telegraph_until_ms == 0 && now >= next_boss_telegraph_ms_) {
      target->telegraph_until_ms = now + kN3BossTelegraphWindowMs;
      WorldCombatEvent warning; warning.type = "telegraph"; warning.attacker_id = target->uuid;
      warning.attacker_name = target->name; warning.target_id = player_uuid_; warning.skill_id = "boss:ground-slam";
      warning.radius = kN3BossTelegraphRadius; warning.duration_ms = kN3BossTelegraphWindowMs; warning.x = target->x; warning.y = target->y;
      events.push_back(warning);
      // Every warning resolves at its authored window below - the server
      // tick thread is the simulation timer, so an instant second-warning
      // resolution would punish a player who already left the circle.
      boss_warning_seen_ = true;
    } else if (target->telegraph_until_ms != 0 && now >= target->telegraph_until_ms) {
      const Vec2 p = tile_movement::occupied_tile(position_);
      if (std::abs(p.x - target->x) <= kN3BossTelegraphRadius && std::abs(p.y - target->y) <= kN3BossTelegraphRadius) {
        player_life = std::max(0, player_life - kN3BossDamage);
        WorldCombatEvent impact; impact.type = "hit"; impact.attacker_id = target->uuid; impact.attacker_name = target->name;
        impact.target_id = player_uuid_; impact.target_name = "Adventurer"; impact.skill_id = "boss:ground-slam";
        impact.amount = kN3BossDamage; impact.health = player_life; impact.health_max = player_life_max; impact.died = player_life == 0;
        events.push_back(impact);
      }
      target->telegraph_until_ms = 0;
      // The next player command is the fixed-step heartbeat in the native
      // protocol slice; make the repeat eligible immediately after the
      // resolved dodge/hit rather than relying on a hidden wall-clock thread.
      next_boss_telegraph_ms_ = now;
    }
  } else if (now >= target->next_attack_ms && std::abs(target->x - tile_movement::occupied_tile(position_).x) <= 2
             && std::abs(target->y - tile_movement::occupied_tile(position_).y) <= 2) {
    const int damage = target->empowered ? kN3MonsterDamage + 2 : kN3MonsterDamage;
    player_life = std::max(0, player_life - damage);
    WorldCombatEvent impact; impact.type = "hit"; impact.attacker_id = target->uuid; impact.attacker_name = target->name;
    impact.target_id = player_uuid_; impact.target_name = "Adventurer"; impact.skill_id = "monster:attack";
    impact.amount = damage; impact.health = player_life; impact.health_max = player_life_max; impact.died = player_life == 0;
    events.push_back(impact); target->next_attack_ms = now + 1500;
  }
  return events;
}

}  // namespace verdigris

namespace verdigris {

// ────────────────────────────────────────────────────────────────────────────
// Parity wave N4 implementation: items, inventory, Vesselforge.
// Mirrors server/core/items/vesselforge/{engine,verdigris-pack,adapter}.js,
// server/core/items/factory.js, server/shared/inventory-footprints.js,
// server/shared/wear-slots.js, server/core/utilities/wear.js, and
// server/core/combat/loot.js. See the task NOTES/REPORT for the survey.
// ────────────────────────────────────────────────────────────────────────────

double Mulberry32::next() {
  // engine.js mulberry32: 32-bit state, Math.imul semantics.  All arithmetic
  // is mod 2^32; the >>> shifts are logical on the unsigned view.
  state_ = state_ + 0x6D2B79F5u;
  std::uint32_t t = state_;
  auto imul = [](std::uint32_t a, std::uint32_t b) { return a * b; };
  t = imul(t ^ (t >> 15), t | 1u);
  t = (t + imul(t ^ (t >> 7), t | 61u)) ^ t;
  return static_cast<double>(t ^ (t >> 14)) / 4294967296.0;
}

namespace {

// verdigris-pack.js data tables.  Iteration order is load-bearing for the
// brand pool (JS object insertion order), so these live in ordered vectors.
struct PackMaterial {
  const char* id;
  const char* name;
  int tier;
  double stat_mult;
  int vessel_lo, vessel_hi;
  int patience_lo, patience_hi;
  double drop_weight;
  std::vector<std::pair<const char*, double>> weights;
};

struct PackImplicit {
  const char* label;
  const char* stat_id;  // nullptr = flavour-only implicit
  int stat_value = 0;
};

struct PackForm {
  const char* id;
  const char* name;
  const char* kind;
  const char* kind_label;
  int w, h;
  bool weapon = false;
  int dmg_lo = 0, dmg_hi = 0;
  double aps = 0;
  int armor = 0;
  bool has_armor = false;  // JS `if (f.armor)` — 0 armour prints no stat line
  std::vector<const char*> tags;
  std::vector<std::pair<const char*, double>> weights;
  std::vector<const char*> materials;  // pack order
  PackImplicit implicit{nullptr, nullptr, 0};
  bool no_vessel = false;
};

struct BrandTier {
  int roll_lo, roll_hi;
  int min_ilvl = 0;
};

struct PackBrandMod {
  const char* id;
  const char* label;
  const char* shape;  // flat | scalar
  std::vector<const char*> tags;
  std::vector<const char*> kinds;
  double weight;
  std::vector<BrandTier> tiers;
};

const std::vector<PackMaterial>& pack_materials() {
  static const std::vector<PackMaterial> data = {
      {"flint", "Flint", 1, 1.0, 2, 3, 2, 3, 30, {{"blade", 1.4}, {"blood", 1.2}, {"ward", 0.6}}},
      {"bone", "Bone", 1, 0.9, 2, 4, 2, 4, 25, {{"beast", 1.6}, {"spirit", 1.3}, {"ember", 0.5}}},
      {"hide", "Hide", 1, 0.9, 2, 3, 3, 4, 30, {{"beast", 1.3}, {"swift", 1.3}, {"ward", 0.8}}},
      {"quilted", "Quilted", 2, 1.2, 2, 4, 3, 5, 16, {{"ward", 1.3}, {"life", 1.3}, {"blade", 0.6}}},
      {"copper", "Copper", 2, 1.25, 2, 4, 3, 5, 18, {{"river", 1.3}, {"fortune", 1.3}}},
      {"bronze", "Bronze", 3, 1.6, 3, 5, 4, 6, 9, {{"blade", 1.2}, {"ward", 1.2}, {"blunt", 1.2}}},
      {"obsidian", "Obsidian", 3, 1.55, 3, 5, 2, 4, 8, {{"blade", 1.8}, {"blood", 1.8}, {"ward", 0.4}}},
      {"jade", "Jade", 4, 1.7, 4, 5, 4, 6, 4, {{"spirit", 1.8}, {"ward", 1.5}, {"blood", 0.5}}},
      {"amber", "Amber", 4, 1.65, 4, 5, 4, 6, 4, {{"ember", 1.7}, {"fortune", 1.5}, {"spirit", 1.3}}},
      {"bronzescale", "Bronze-scale", 4, 1.9, 4, 5, 4, 6, 3, {{"ward", 1.6}, {"life", 1.3}}},
      {"skymetal", "Skymetal", 5, 2.3, 4, 6, 5, 7, 1, {{"blade", 1.4}, {"spirit", 1.4}, {"ember", 1.3}}},
      {"rivetmail", "Riveted Mail", 6, 2.6, 5, 6, 5, 8, 0.3, {{"ward", 2.0}, {"life", 1.5}}},
  };
  return data;
}

const std::vector<PackForm>& pack_forms() {
  static const std::vector<PackForm> data = {
      {"handaxe", "Handaxe", "weapon", "One-hand weapon", 1, 2, true, 7, 13, 1.3, 0, false,
       {"blade"}, {{"blade", 1.3}}, {"flint", "copper", "bronze", "obsidian", "skymetal"},
       {"15% increased Physical Damage", "phys_pct", 15}},
      {"spear", "Spear", "weapon", "Reach weapon", 1, 4, true, 10, 22, 1.0, 0, false,
       {"reach"}, {{"reach", 1.5}}, {"flint", "bone", "copper", "bronze", "obsidian", "skymetal"},
       {"Strikes keep foes at reach", nullptr, 0}},
      {"macuahuitl", "Macuahuitl", "weapon", "Edged club", 2, 3, true, 14, 30, 0.85, 0, false,
       {"blade", "blunt", "blood"}, {{"blood", 1.6}}, {"obsidian", "flint", "bone"},
       {"Hits cause Bleeding", nullptr, 0}},
      {"atlatl", "Atlatl", "weapon", "Dart-thrower", 1, 3, true, 8, 18, 1.1, 0, false,
       {"reach", "swift"}, {{"swift", 1.4}}, {"bone", "copper", "bronze"},
       {"+20% Projectile Range", nullptr, 0}},
      {"khopesh", "Khopesh", "weapon", "Sickle-sword", 1, 3, true, 11, 20, 1.2, 0, false,
       {"blade"}, {{"blade", 1.2}, {"fortune", 1.2}}, {"flint", "copper", "bronze", "skymetal"},
       {"+10% Attack Speed", "atk_speed", 10}},
      {"sling", "Sling", "weapon", "Sling", 1, 2, true, 5, 16, 1.15, 0, false,
       {"reach", "swift"}, {{"swift", 1.3}}, {"hide", "quilted"},
       {"Ignores half of Armour", nullptr, 0}},
      {"hideshield", "Shield", "shield", "Shield", 2, 3, false, 0, 0, 0, 45, true,
       {"ward"}, {{"ward", 1.5}}, {"hide", "bronze", "bronzescale", "rivetmail"},
       {"+4% Chance to Block", "block", 4}},
      {"wrap", "Wrap", "body", "Body wrap", 2, 3, false, 0, 0, 0, 60, true,
       {"ward", "life"}, {{"life", 1.2}}, {"hide", "quilted", "bronzescale", "rivetmail"},
       {"+15 to Maximum Health", "life", 15}},
      {"crest", "Crest", "helmet", "Headpiece", 2, 2, false, 0, 0, 0, 25, true,
       {"ward", "spirit"}, {{"spirit", 1.2}}, {"bone", "hide", "copper", "bronze", "jade"},
       {"+10 to Maximum Mana", "spirit", 10}},
      {"grips", "Grips", "gloves", "Handwraps", 2, 2, false, 0, 0, 0, 18, true,
       {"blade", "swift"}, {}, {"hide", "quilted", "bronzescale"},
       {"+8% Attack Speed", "atk_speed", 8}},
      {"sandals", "Sandals", "boots", "Footwear", 2, 2, false, 0, 0, 0, 14, true,
       {"swift"}, {{"swift", 1.6}}, {"hide", "quilted"},
       {"+10% Movement Speed", "move", 10}},
      {"girdle", "Girdle", "belt", "Waistband", 2, 1, false, 0, 0, 0, 0, false,
       {"life"}, {{"life", 1.3}}, {"hide", "quilted", "copper"},
       {"+12 to Maximum Health", "life", 12}},
      {"gorget", "Gorget", "amulet", "Neckpiece", 1, 1, false, 0, 0, 0, 0, false,
       {"spirit", "ward"}, {{"spirit", 1.4}}, {"jade", "amber", "bone", "copper"},
       {"+8 to All Attributes", "attrs", 8}},
      {"ring", "Ring", "ring", "Ring", 1, 1, false, 0, 0, 0, 0, false,
       {"fortune"}, {{"fortune", 1.2}}, {"bone", "copper", "jade", "amber"},
       {"+12 to Maximum Health", "life", 12}},
      {"curio", "Curio", "curio", "Curio", 1, 1, false, 0, 0, 0, 0, false,
       {}, {}, {"bone", "jade", "amber"}, {nullptr, nullptr, 0}, true},
  };
  return data;
}

const std::vector<PackBrandMod>& pack_brand_mods() {
  static const std::vector<PackBrandMod> data = {
      {"keen", "+{v}% increased Physical Damage", "scalar", {"blade"}, {"weapon"}, 12,
       {{8, 14}, {15, 22, 20}, {23, 32, 50}}},
      {"heavy", "+{v} flat Damage", "flat", {"blunt", "blade"}, {"weapon"}, 12,
       {{2, 4}, {5, 8, 25}, {9, 14, 55}}},
      {"swift_haft", "+{v}% Attack Speed", "scalar", {"swift"}, {"weapon"}, 9,
       {{5, 8}, {9, 13, 25}, {14, 18, 55}}},
      {"bloodgroove", "+{v}% chance to Bleed on hit", "scalar", {"blood"}, {"weapon"}, 7,
       {{10, 15}, {16, 25, 30}}},
      {"long_reach", "+{v}% increased Reach", "scalar", {"reach"}, {"weapon"}, 7,
       {{6, 10}, {11, 16, 30}}},
      {"warded", "+{v}% increased Armour", "scalar", {"ward"},
       {"shield", "body", "helmet", "gloves", "boots"}, 12,
       {{10, 18}, {19, 28, 20}, {29, 40, 50}}},
      {"hale", "+{v} to Maximum Health", "flat", {"life"},
       {"weapon", "shield", "body", "helmet", "gloves", "boots", "belt", "amulet", "ring"}, 12,
       {{10, 20}, {21, 35, 25}, {36, 55, 55}}},
      {"spirited", "+{v} to Maximum Mana", "flat", {"spirit"},
       {"helmet", "amulet", "ring", "weapon"}, 10,
       {{8, 15}, {16, 26, 25}, {27, 40, 55}}},
      {"emberkiss", "Adds {v} Fire Damage", "flat", {"ember"}, {"weapon", "amulet"}, 6,
       {{3, 7}, {8, 14, 30}, {15, 22, 60}}},
      {"riverblessed", "+{v}% to Cold Resistance", "scalar", {"river", "ward"},
       {"shield", "body", "helmet", "belt", "amulet", "ring"}, 8,
       {{8, 15}, {16, 25, 25}}},
      {"emberward", "+{v}% to Fire Resistance", "scalar", {"ember", "ward"},
       {"shield", "body", "helmet", "belt", "amulet", "ring"}, 8,
       {{8, 15}, {16, 25, 25}}},
      {"surefoot", "+{v}% Movement Speed", "scalar", {"swift"}, {"boots"}, 9,
       {{5, 9}, {10, 15, 30}}},
      {"keen_eye", "+{v}% Critical Chance", "scalar", {"blade", "swift"}, {"weapon", "ring"}, 7,
       {{8, 14}, {15, 22, 35}}},
      {"wealthy", "+{v}% Item Find", "scalar", {"fortune"},
       {"weapon", "shield", "body", "helmet", "gloves", "boots", "belt", "amulet", "ring"}, 8,
       {{6, 12}, {13, 20, 30}}},
      {"beastbane", "+{v}% Damage against Beasts", "scalar", {"beast"}, {"weapon"}, 7,
       {{10, 18}, {19, 30, 30}}},
      {"strongback", "+{v} to All Attributes", "flat", {"life", "spirit"},
       {"amulet", "ring", "belt"}, 6,
       {{3, 6}, {7, 11, 35}}},
  };
  return data;
}

const std::vector<const char*>& pack_name_pre() {
  static const std::vector<const char*> data = {
      "Grim", "Sable", "Ashen", "Reed", "Ember", "Frost", "Dusk", "Copper", "Thorn", "Vesper"};
  return data;
}
const std::vector<const char*>& pack_name_post() {
  static const std::vector<const char*> data = {
      "Whisper", "Ward", "Bite", "Song", "Pledge", "Shard", "Gloam", "Tithe"};
  return data;
}

// engine.js settings not overridden by the pack.
constexpr int kMaxVessel = 6;
constexpr double kBrandTierWeights[] = {10, 5, 2};
constexpr double kBrandCountWeights[] = {30, 25, 25, 15, 5};
constexpr int kAttuneBase = 80;

const PackMaterial* pack_material(const std::string& id) {
  for (const auto& m : pack_materials()) if (id == m.id) return &m;
  return nullptr;
}
const PackForm* pack_form(const std::string& id) {
  for (const auto& f : pack_forms()) if (id == f.id) return &f;
  return nullptr;
}
const PackBrandMod* pack_brand_mod(const std::string& id) {
  for (const auto& b : pack_brand_mods()) if (id == b.id) return &b;
  return nullptr;
}

double weight_for(const std::vector<std::pair<const char*, double>>& weights, const char* tag) {
  for (const auto& [key, value] : weights) if (std::string(tag) == key) return value;
  return 0.0;  // JS `if (mat.weights[tag])` — absent means unchanged
}

bool list_has(const std::vector<const char*>& list, const std::string& value) {
  for (const auto* entry : list) if (value == entry) return true;
  return false;
}

std::string to_base36(std::uint64_t value) {
  static constexpr char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  if (value == 0) return "0";
  std::string out;
  while (value) {
    out.insert(out.begin(), digits[value % 36]);
    value /= 36;
  }
  return out;
}

std::string format_label(const std::string& templ, int value) {
  std::string out = templ;
  const std::string replacement = std::to_string(value);
  std::string::size_type at = 0;
  while ((at = out.find("{v}", at)) != std::string::npos) {
    out.replace(at, 3, replacement);
    at += replacement.size();
  }
  return out;
}

// adapter.js ACTIVE_BRAND_MODS — brands presented as live, not dormant.
bool is_active_brand_mod(const std::string& mod_id) {
  static const char* kActive[] = {"keen", "heavy", "swift_haft", "warded", "hale",
                                  "spirited", "emberkiss", "strongback", "keen_eye",
                                  "wealthy", "beastbane"};
  for (const auto* id : kActive) if (mod_id == id) return true;
  return false;
}

}  // namespace

VesselForge::VesselForge() : rand_(Mulberry32(0)) {}

std::string VesselForge::gen_id() {
  id_counter_ += 1;
  return "vf" + to_base36(id_counter_) +
         to_base36(static_cast<std::uint64_t>(std::floor(rand_.next() * 1e6)));
}

std::vector<VesselForge::WeightedEntry> VesselForge::brand_pool(const VesselItem& item) const {
  const PackMaterial* mat = pack_material(item.material_id);
  const PackForm* f = pack_form(item.form_id);
  std::vector<WeightedEntry> pool;
  if (!mat || !f) return pool;
  for (const auto& mod : pack_brand_mods()) {
    if (!list_has(mod.kinds, item.kind)) continue;
    bool used = false;
    for (const auto& brand : item.brands) if (brand.mod_id == mod.id) { used = true; break; }
    if (used) continue;
    double w = mod.weight;
    for (const auto* tag : mod.tags) {
      const double material_weight = weight_for(mat->weights, tag);
      if (material_weight != 0.0) w *= material_weight;
      const double form_weight = weight_for(f->weights, tag);
      if (form_weight != 0.0) w *= form_weight;
    }
    if (w > 0) pool.push_back({mod.id, w});
  }
  return pool;
}

namespace {
// engine.js pickWeighted: roll = rnd*total; subtract in order; first <=0 wins.
std::string pick_weighted(Mulberry32& rand, const std::vector<VesselForge::WeightedEntry>& entries) {
  double total = 0;
  for (const auto& entry : entries) total += entry.weight;
  if (total <= 0 || entries.empty()) return {};
  double roll = rand.next() * total;
  for (const auto& entry : entries) {
    roll -= entry.weight;
    if (roll <= 0) return entry.id;
  }
  return entries.back().id;
}
}  // namespace

bool VesselForge::roll_brand(VesselItem& item, VesselBrand* out) {
  const auto pool = brand_pool(item);
  if (pool.empty()) return false;
  const std::string mod_id = pick_weighted(rand_, pool);  // 1 rand even on failure paths
  if (mod_id.empty()) return false;
  const PackBrandMod* mod = pack_brand_mod(mod_id);
  if (!mod) return false;
  std::vector<WeightedEntry> gated;
  for (std::size_t i = 0; i < mod->tiers.size(); ++i) {
    if (mod->tiers[i].min_ilvl <= item.ilvl) {
      gated.push_back({std::to_string(i), kBrandTierWeights[i]});
    }
  }
  if (gated.empty()) return false;
  const std::string tier_index = pick_weighted(rand_, gated);
  const int tier = std::stoi(tier_index);  // zero-based index, tier = index + 1
  const BrandTier& tier_def = mod->tiers[static_cast<std::size_t>(tier)];
  VesselBrand brand;
  brand.id = gen_id();
  brand.mod_id = mod_id;
  brand.tier = tier + 1;
  brand.value = rand_.rint(tier_def.roll_lo, tier_def.roll_hi);
  if (out) *out = brand;
  return true;
}

VesselItem VesselForge::generate_item(int ilvl, const std::string& form_id,
                                      const std::string& material_id) {
  const PackForm* f = pack_form(form_id);
  VesselItem item;
  if (!f) return item;  // caller validates form ids against the catalogue
  item.ilvl = ilvl;
  item.form_id = form_id;
  item.kind = f->kind;
  item.w = f->w;
  item.h = f->h;

  std::string mat_id = material_id;
  if (mat_id.empty()) {
    // materialPoolFor: form materials in pack order, tier <= 1 + ilvl/15,
    // weighted by dropWeight.
    std::vector<WeightedEntry> pool;
    const int max_tier = 1 + ilvl / 15;
    for (const auto* candidate : f->materials) {
      const PackMaterial* m = pack_material(candidate);
      if (m && m->tier <= max_tier) pool.push_back({candidate, m->drop_weight});
    }
    mat_id = pick_weighted(rand_, pool);
    if (mat_id.empty() && !f->materials.empty()) mat_id = f->materials.front();
  }
  item.material_id = mat_id;
  const PackMaterial* mat = pack_material(mat_id);

  item.id = gen_id();
  item.vessel = mat ? std::min(kMaxVessel, rand_.rint(mat->vessel_lo, mat->vessel_hi)) : 0;
  item.patience_max = mat ? rand_.rint(mat->patience_lo, mat->patience_hi) : 0;
  item.patience = item.patience_max;

  if (!f->no_vessel) {
    std::vector<WeightedEntry> counts;
    for (int i = 0; i < 5; ++i) counts.push_back({std::to_string(i), kBrandCountWeights[i]});
    int n = std::stoi(pick_weighted(rand_, counts));
    n = std::min(n, item.vessel);
    for (int i = 0; i < n; ++i) {
      VesselBrand brand;
      if (roll_brand(item, &brand)) item.brands.push_back(brand);
    }
  } else {
    item.vessel = 0;
    item.patience = 0;
    item.patience_max = 0;
  }

  // maybeName: three or more marks earn an epithet from the pack tables.
  // NB: draw order matters — pre first, then post, in separate statements
  // (operator+ operand order is unspecified in C++).
  if (item.epithet_name.empty() && item.brands.size() >= 3) {
    const auto& pre = pack_name_pre();
    const auto& post = pack_name_post();
    const double first = rand_.next();
    const double second = rand_.next();
    item.epithet_name = std::string(pre[static_cast<std::size_t>(std::floor(first * pre.size()))]) +
        " " + post[static_cast<std::size_t>(std::floor(second * post.size()))];
  }
  return item;
}

bool VesselForge::sear(VesselItem& item) {
  if (!item.vessel) return false;
  const int used = static_cast<int>(item.brands.size()) + item.scars;
  if (item.vessel - used <= 0) return false;
  if (item.patience < 1) return false;
  // engine.js sear rolls on a clone: a failed roll leaves the item untouched.
  VesselItem copy = item;
  VesselBrand brand;
  if (!roll_brand(copy, &brand)) return false;
  item.patience -= 1;
  item.brands.push_back(brand);
  if (item.epithet_name.empty() && item.brands.size() >= 3) {
    const auto& pre = pack_name_pre();
    const auto& post = pack_name_post();
    const double first = rand_.next();
    const double second = rand_.next();
    item.epithet_name = std::string(pre[static_cast<std::size_t>(std::floor(first * pre.size()))]) +
        " " + post[static_cast<std::size_t>(std::floor(second * post.size()))];
  }
  return true;
}

namespace {
std::string vessel_display_name(const VesselItem& item) {
  if (!item.epithet_name.empty()) return item.epithet_name;
  const PackMaterial* mat = pack_material(item.material_id);
  const PackForm* f = pack_form(item.form_id);
  return std::string(mat ? mat->name : "?") + " " + (f ? f->name : "?");
}

std::string format_aps(double aps) {
  std::ostringstream out;
  out << aps;  // JS prints the literal (1, 1.3, 0.85)
  return out.str();
}
}  // namespace

std::vector<TooltipLine> VesselForge::tooltip(const VesselItem& item) const {
  // engine.js tooltip — bonds/trophies/awakening are out of N4's scope (no
  // flow produces them), so their sections never appear here.
  std::vector<TooltipLine> lines;
  const PackMaterial* mat = pack_material(item.material_id);
  const PackForm* f = pack_form(item.form_id);
  if (!mat || !f) return lines;

  lines.push_back({"name", vessel_display_name(item),
                   item.brands.empty() ? "plain" : "branded"});
  if (!item.epithet_name.empty()) {
    lines.push_back({"base", std::string(mat->name) + " " + f->name, "normal"});
  }
  lines.push_back({"kind", std::string(f->kind_label) + " · " + mat->name +
                               " (tier " + std::to_string(mat->tier) + ") · Item Level " +
                               std::to_string(item.ilvl),
                   "normal"});
  if (item.vessel) {
    lines.push_back({"vessel", "Vessel " + std::to_string(item.vessel) + " · Patience " +
                                   std::to_string(item.patience) + "/" +
                                   std::to_string(item.patience_max),
                     "normal"});
  }
  if (f->weapon) {
    lines.push_back({"stat", "Damage " +
                                 std::to_string(static_cast<int>(std::lround(f->dmg_lo * mat->stat_mult))) +
                                 "–" +
                                 std::to_string(static_cast<int>(std::lround(f->dmg_hi * mat->stat_mult))) +
                                 " · Speed " + format_aps(f->aps),
                     "normal"});
  }
  if (f->has_armor && f->armor > 0) {
    lines.push_back({"stat", "Armour " +
                                 std::to_string(static_cast<int>(std::lround(f->armor * mat->stat_mult))),
                     "normal"});
  }
  if (f->implicit.label) {
    lines.push_back({"implicit", f->implicit.label, "normal"});
  }
  for (const auto& brand : item.brands) {
    const PackBrandMod* mod = pack_brand_mod(brand.mod_id);
    if (!mod) continue;
    lines.push_back({"brand", "✦ " + format_label(mod->label, brand.value) +
                                  " (T" + std::to_string(brand.tier) + ")",
                     "normal"});
  }
  if (item.scars) {
    lines.push_back({"scar", "✕ " + std::to_string(item.scars) + " scarred slot" +
                                 (item.scars > 1 ? "s" : ""),
                     "normal"});
  }
  const int used = static_cast<int>(item.brands.size()) + item.scars;
  if (item.vessel && item.vessel - used > 0) {
    lines.push_back({"attune", "Attunement 0/" + std::to_string(kAttuneBase), "normal"});
  }
  return lines;
}

VesselCombat VesselForge::derive_combat(const VesselItem& item) const {
  // adapter.js deriveVesselCombat over the engine aggregate of this one item.
  VesselCombat combat;
  const PackMaterial* mat = pack_material(item.material_id);
  const PackForm* f = pack_form(item.form_id);
  if (!mat || !f) return combat;

  std::map<std::string, double> sums;
  if (f->implicit.stat_id) sums[f->implicit.stat_id] += f->implicit.stat_value;
  for (const auto& brand : item.brands) {
    const PackBrandMod* mod = pack_brand_mod(brand.mod_id);
    if (!mod) continue;
    if (std::string(mod->shape) == "flat" || std::string(mod->shape) == "scalar") {
      sums[brand.mod_id] += brand.value;
    }
  }
  auto sum = [&](const char* id) { auto it = sums.find(id); return it == sums.end() ? 0.0 : it->second; };

  if (f->has_armor && f->armor > 0) {
    combat.ward = std::max(0, static_cast<int>(std::lround(f->armor * mat->stat_mult)));
  }

  if (f->weapon) {
    const double flat = sum("heavy") + sum("emberkiss");
    const double physical = 1 + (sum("phys_pct") + sum("keen")) / 100.0;
    const double speed = f->aps * (1 + sum("atk_speed") / 100.0);
    const int minimum = std::max(1, static_cast<int>(std::lround((f->dmg_lo * mat->stat_mult + flat) * physical)));
    const int maximum = std::max(minimum, static_cast<int>(std::lround((f->dmg_hi * mat->stat_mult + flat) * physical)));
    const int dps = std::max(1, static_cast<int>(std::lround(((minimum + maximum) / 2.0) * speed)));
    const int rating = std::max(1, static_cast<int>(std::lround(dps / 2.0)));
    const char* channel = "crush";
    if (item.form_id == "handaxe" || item.form_id == "khopesh") channel = "slash";
    else if (item.form_id == "spear") channel = "stab";
    else if (item.form_id == "atlatl" || item.form_id == "sling") channel = "range";
    combat.has_damage = true;
    combat.damage_min = minimum;
    combat.damage_max = maximum;
    combat.attacks_per_second = std::lround(speed * 100) / 100.0;
    combat.dps = dps;
    combat.rating = rating;
    combat.channel = channel;
    if (combat.channel == "stab") combat.attack.stab = rating;
    else if (combat.channel == "slash") combat.attack.slash = rating;
    else if (combat.channel == "range") combat.attack.range = rating;
    else combat.attack.crush = rating;
  }

  if (combat.ward > 0) {
    const int rating = std::max(1, static_cast<int>(std::lround(combat.ward / 8.0)));
    combat.defense.stab = rating;
    combat.defense.slash = rating;
    combat.defense.crush = rating;
    combat.defense.range = rating;
  }

  const int all_attributes = std::max(0, static_cast<int>(std::lround(sum("attrs") + sum("strongback"))));
  if (all_attributes > 0) {
    combat.has_attributes = true;
    combat.attributes = all_attributes;
  }
  combat.resource_health = std::max(0, static_cast<int>(std::lround(sum("life") + sum("hale"))));
  combat.resource_mana = std::max(0, static_cast<int>(std::lround(sum("spirit") + sum("spirited"))));

  combat.modifiers.block_chance = std::max(0, std::min(75, static_cast<int>(sum("block"))));
  combat.modifiers.critical_chance = std::max(0, std::min(75, static_cast<int>(sum("keen_eye"))));
  combat.modifiers.goods_found = std::max(0, std::min(100, static_cast<int>(sum("wealthy"))));
  combat.modifiers.damage_against_beasts = std::max(0, std::min(100, static_cast<int>(sum("beastbane"))));
  return combat;
}

VesselBlock VesselForge::make_block(const VesselItem& item) const {
  // adapter.js refreshVesselBlock: derived layers + honest dormant marking.
  VesselBlock block;
  const PackMaterial* mat = pack_material(item.material_id);
  const PackForm* f = pack_form(item.form_id);
  block.item = item;
  if (!mat || !f) return block;
  block.material = mat->name;
  block.material_tier = mat->tier;
  block.form = f->name;

  auto lines = tooltip(item);
  std::size_t brand_index = 0;
  for (auto& line : lines) {
    if (line.section == "implicit") {
      const char* stat_id = f->implicit.stat_id ? f->implicit.stat_id : "";
      const bool active = stat_id == std::string("life") || stat_id == std::string("spirit") ||
                          stat_id == std::string("attrs") || stat_id == std::string("block") ||
                          (f->weapon && (stat_id == std::string("phys_pct") ||
                                         stat_id == std::string("atk_speed")));
      if (!active) {
        line.section = "dormant";
        line.text = "Dormant · " + line.text;
        line.tone = "inactive";
      }
    } else if (line.section == "brand") {
      const bool active = brand_index < item.brands.size() &&
                          is_active_brand_mod(item.brands[brand_index].mod_id);
      ++brand_index;
      if (!active) {
        line.section = "dormant";
        line.text = "Dormant · " + line.text;
        line.tone = "inactive";
      }
    }
  }
  block.lines = std::move(lines);
  for (const auto& line : block.lines) {
    if (line.section == "name") {
      block.display_name = line.text;
      break;
    }
  }
  if (block.display_name.empty()) block.display_name = f->name;
  block.combat = derive_combat(item);
  return block;
}

// ── Item catalogue (server/core/data/items/{general,jewelry,belts,weapons,
// verdigris,vessels}.js) ─────────────────────────────────────────────────
namespace {

const ItemDef kItemCatalogue[] = {
    // general.js: carried balance. Stackable, never binds, no grid cells.
    {"coins", "Coins", "currency", "", true, false, {}, {}, 0, 0, "", ""},
    // jewelry.js rings + belts.js waist the N4 scenarios exercise.
    {"ring", "Ring", "armor", "ring", false, false, {1, 1, 1, 1}, {1, 0, 1, 1}, 0, 0, "", ""},
    {"gold-ring", "Gold Ring", "armor", "ring", false, false, {3, 3, 3, 3}, {3, 3, 3, 3}, 0, 0, "", ""},
    {"hide-girdle", "Hide Girdle", "armor", "belt", false, false, {0, 0, 0, 0}, {1, 1, 1, 0}, 0, 0, "", ""},
    // jewelry.js amulets: the single-session regression grants garnet-amulet.
    {"garnet-amulet", "Garnet Amulet", "armor", "necklace", false, false, {23, 22, 13, 1}, {24, 25, 13, 4}, 0, 0, "", ""},
    // weapons.js / verdigris.js curated bases.
    {"bronze-sword", "Bronze Sword", "weapon", "right_hand", false, false, {4, 3, -2, 0}, {0, 2, 1, 0}, 0, 0, "", ""},
    {"bronze-pike", "Bronze Pike", "weapon", "right_hand", false, true, {13, 5, 0, 0}, {1, 1, 0, 0}, 1, 4, "spear", "bronze"},
    // vessels.js: the 13 Vesselforge-native rows. Material, footprint and
    // combat profile are rolled by the forge (disableAffixes in JS).
    {"vessel-handaxe", "Handaxe", "weapon", "right_hand", false, false, {}, {}, 0, 0, "handaxe", ""},
    {"vessel-spear", "Spear", "weapon", "right_hand", false, true, {}, {}, 0, 0, "spear", ""},
    {"vessel-macuahuitl", "Macuahuitl", "weapon", "right_hand", false, true, {}, {}, 0, 0, "macuahuitl", ""},
    {"vessel-atlatl", "Atlatl", "weapon", "right_hand", false, true, {}, {}, 0, 0, "atlatl", ""},
    {"vessel-khopesh", "Khopesh", "weapon", "right_hand", false, false, {}, {}, 0, 0, "khopesh", ""},
    {"vessel-sling", "Sling", "weapon", "right_hand", false, false, {}, {}, 0, 0, "sling", ""},
    {"vessel-shield", "Shield", "armor", "left_hand", false, false, {}, {}, 0, 0, "hideshield", ""},
    {"vessel-wrap", "Wrap", "armor", "armor", false, false, {}, {}, 0, 0, "wrap", ""},
    {"vessel-crest", "Crest", "armor", "head", false, false, {}, {}, 0, 0, "crest", ""},
    {"vessel-grips", "Grips", "armor", "gloves", false, false, {}, {}, 0, 0, "grips", ""},
    {"vessel-sandals", "Sandals", "armor", "feet", false, false, {}, {}, 0, 0, "sandals", ""},
    {"vessel-gorget", "Gorget", "armor", "necklace", false, false, {}, {}, 0, 0, "gorget", ""},
    {"vessel-ring", "Ring", "armor", "ring", false, false, {}, {}, 0, 0, "ring", ""},
    // weapons.js iron weaponry: the mortality successor grants iron-sword.
    {"iron-sword", "Iron Sword", "weapon", "right_hand", false, false, {6, 4, -2, 0}, {0, 3, 2, 0}, 0, 0, "", ""},
    // weapons.js steel-battleaxe (session-arc reward drop).
    {"steel-battleaxe", "Steel Battleaxe", "weapon", "right_hand", false, false, {-2, 19, 13, 0}, {0, 1, 2, 2}, 0, 0, "", ""},
    // town-amenities starter kit + small armor footprints.
    {"bronze-dagger", "Bronze Dagger", "weapon", "right_hand", false, false, {4, 2, -1, 0}, {0, 1, 0, 0}, 0, 0, "", ""},
    {"bronze-med-helm", "Bronze Med Helm", "armor", "head", false, false, {0, 0, 0, 0}, {3, 4, 3, 0}, 0, 0, "", ""},
    {"bronze-gloves", "Bronze Gloves", "armor", "gloves", false, false, {0, 0, 0, 0}, {1, 2, 1, 0}, 0, 0, "", ""},
    {"bronze-boots", "Bronze Boots", "armor", "feet", false, false, {0, 0, 0, 0}, {1, 2, 2, 0}, 0, 0, "", ""},
    {"knife", "Knife", "sharp", "", false, false, {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, "", ""},
    {"wooden-shield", "Wooden Shield", "armor", "left_hand", false, false, {0, 0, 0, 0}, {2, 1, 3, 0}, 0, 0, "", ""},
    // Endgame charted tablets: one compact inventory item is consumed to
    // open one rolled expedition. Theme/layout are authored by the base;
    // tier and modifiers are rolled into the live GameItem below.
    {"charted-tablet-barrow", "Barrow Charted Tablet", "map", "", false, false, {}, {}, 1, 1, "", ""},
    {"charted-tablet-reeds", "Reeds Charted Tablet", "map", "", false, false, {}, {}, 1, 1, "", ""},
    {"charted-tablet-crown", "Crown Charted Tablet", "map", "", false, false, {}, {}, 1, 1, "", ""},
    {"charted-tablet-thorns", "Thorns Charted Tablet", "map", "", false, false, {}, {}, 1, 1, "", ""},
};

// Process-wide instance identity source (factory.js uuid v4): uniqueness is
// the only contract the wire and the take/equip verbs rely on.
std::uint64_t g_item_uuid_serial = 0;

std::string next_item_uuid() {
  const std::uint64_t value = ++g_item_uuid_serial;
  char buffer[40];
  std::snprintf(buffer, sizeof(buffer), "00000000-0000-4000-8000-%012llx",
                static_cast<unsigned long long>(value & 0xffffffffffffULL));
  return buffer;
}

std::string lower_copy(const std::string& value) {
  std::string out = value;
  for (auto& ch : out) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  return out;
}

// inventory-footprints.js idContains: substring match over id + name.
bool id_contains(const ItemDef& def, const GameItem* instance,
                 std::initializer_list<const char*> needles) {
  const std::string id = lower_copy(def.id);
  const std::string name = lower_copy(instance && !instance->display_name.empty()
                                          ? instance->display_name
                                          : def.name);
  for (const char* needle : needles) {
    const std::string n = lower_copy(needle);
    if (id.find(n) != std::string::npos || name.find(n) != std::string::npos) return true;
  }
  return false;
}

ItemSize normalise_size(int width, int height) {
  auto clamp_int = [](int value, int lo, int hi) { return std::max(lo, std::min(hi, value)); };
  ItemSize size{clamp_int(width, 1, 4), clamp_int(height, 1, 8)};
  while (size.width * size.height > 8 && size.height > 1) size.height -= 1;
  while (size.width * size.height > 8 && size.width > 1) size.width -= 1;
  return size;
}

ItemSize weapon_size(const ItemDef& def, const GameItem* instance, bool two_handed) {
  if (two_handed || id_contains(def, instance, {"halberd", "spear"})) return {2, 4};
  if (id_contains(def, instance, {"longbow", "shortbow", "battleaxe", "warhammer"})) return {2, 3};
  if (id_contains(def, instance, {"dagger", "knife"})) return {1, 2};
  if (id_contains(def, instance, {"axe", "mace", "sword"})) return {1, 3};
  return {1, 2};
}

ItemSize armor_size(const ItemDef& def, const GameItem* instance) {
  if (id_contains(def, instance, {"pavise"})) return {2, 3};
  if (id_contains(def, instance, {"shield"})) return {2, 2};
  if (id_contains(def, instance, {"chainmail", "armor", "body", "robe"})) return {2, 3};
  if (id_contains(def, instance, {"cape"})) return {2, 3};
  if (id_contains(def, instance, {"helm", "hat", "cowl"})) return {2, 2};
  if (id_contains(def, instance, {"boots", "gloves"})) return {2, 2};
  // SLOT_SIZE_BY_EQUIPMENT_SLOT.
  const std::string& slot = def.slot;
  if (slot == "ring" || slot == "necklace") return {1, 1};
  if (slot == "head" || slot == "gloves" || slot == "feet" || slot == "left_hand") return {2, 2};
  if (slot == "back" || slot == "armor") return {2, 3};
  return {1, 1};
}

}  // namespace

const ItemDef* item_def(const std::string& id) {
  for (const auto& def : kItemCatalogue) {
    if (def.id == id) return &def;
  }
  return nullptr;
}

const std::vector<std::string>& gear_drop_pool() {
  // loot.js GEAR_DROP_POOL, in declaration order (pool index is rolled).
  static const std::vector<std::string> pool = {
      "vessel-handaxe", "vessel-spear", "vessel-macuahuitl", "vessel-atlatl",
      "vessel-khopesh", "vessel-sling", "vessel-shield", "vessel-wrap",
      "vessel-crest", "vessel-grips", "vessel-sandals", "vessel-gorget",
      "vessel-ring",
  };
  return pool;
}

ItemSize resolve_item_size(const ItemDef& def, const VesselBlock* vessel) {
  // inventory-footprints.js resolveItemSize. Two JS branches are omitted as
  // unreachable for this catalogue (and retired-vocabulary): the loose
  // resource id rule (currency is caught by the stackable branch first) and
  // the retired tool id rule.
  const std::string& slot = def.slot;
  if (slot == "head" || slot == "gloves" || slot == "feet") return {2, 2};
  if (vessel && vessel->item.w > 0 && vessel->item.h > 0) {
    return normalise_size(vessel->item.w, vessel->item.h);
  }
  if (def.size_w > 0 || def.size_h > 0) return normalise_size(def.size_w, def.size_h);
  if (def.stackable || def.type == "currency") return {1, 1};
  if (def.type == "weapon" || slot == "right_hand" ||
      id_contains(def, nullptr, {"sword", "axe", "mace", "dagger", "bow", "halberd", "spear", "warhammer"})) {
    return weapon_size(def, nullptr, def.two_handed);
  }
  if (def.type == "armor" || slot == "ring" || slot == "necklace" || slot == "back" ||
      slot == "armor" || slot == "left_hand") {
    return armor_size(def, nullptr);
  }
  if (def.type == "jewelry" || id_contains(def, nullptr, {"ring", "amulet"})) return {1, 1};
  return {1, 1};
}

std::optional<GameItem> create_game_item(const std::string& item_id,
                                         const CreateItemOptions& options) {
  // factory.js createById/createFromBase.
  const ItemDef* def = item_def(item_id);
  if (!def) return std::nullopt;

  GameItem item;
  item.id = def->id;
  item.name = def->name;
  item.display_name = def->name;
  item.uuid = next_item_uuid();
  item.stackable = def->stackable;
  item.two_handed = def->two_handed;
  item.equip_slot = def->slot;
  item.attack = def->attack;
  item.defense = def->defense;

  if (def->type == "map") {
    ExpeditionMapBlock map;
    map.tier = std::clamp(options.item_level > 0 ? options.item_level : 1, 1, 16);
    if (item_id == "charted-tablet-reeds") {
      map.theme = "marsh";
      map.layout = "clearings";
    } else if (item_id == "charted-tablet-crown") {
      map.theme = "crypt";
      map.layout = "gauntlet";
    } else if (item_id == "charted-tablet-thorns") {
      map.theme = "grove";
      map.layout = "clearings";
    } else {
      map.theme = "dungeon";
      map.layout = "warren";
    }

    // Tier is the dependable difficulty ladder; rolled clauses add variance
    // on top of it. A higher-tier tablet must never be merely a richer copy
    // of the same low-level floor.
    map.monster_level_bonus = map.tier;

    // Every tablet carries two distinct, inspectable risk/reward clauses.
    // A supplied RNG makes drops reproducible; factory callers without one
    // still receive a stable roll derived from the already-unique uuid.
    std::uint32_t fallback_seed = 2166136261u;
    for (unsigned char ch : item.uuid)
      fallback_seed = (fallback_seed ^ ch) * 16777619u;
    Mulberry32 fallback(fallback_seed);
    Mulberry32& rng = options.rng ? *options.rng : fallback;
    const int first = static_cast<int>(std::floor(rng.next() * 4.0)) % 4;
    const int second = (first + 1 +
                        static_cast<int>(std::floor(rng.next() * 3.0)) % 3) % 4;
    map.goods_found_percent = 15 + map.tier * 5;
    const auto apply_modifier = [&](int index) {
      if (index == 0) {
        const int value = 15 + map.tier * 3;
        map.monster_damage_percent += value;
        map.goods_found_percent += value;
        map.modifiers.push_back("Furious: monsters deal " +
                                std::to_string(value) + "% more damage");
      } else if (index == 1) {
        const int value = 3 + map.tier / 3;
        map.extra_monsters += value;
        map.goods_found_percent += 18 + value * 3;
        map.modifiers.push_back("Teeming: " + std::to_string(value) +
                                " additional foes");
      } else if (index == 2) {
        const int value = 24 + map.tier * 4;
        map.monster_life_percent += value;
        map.goods_found_percent += value / 2;
        map.modifiers.push_back("Ironbound: monsters have " +
                                std::to_string(value) + "% more life");
      } else {
        const int value = 1 + map.tier / 4;
        map.monster_level_bonus += value;
        map.goods_found_percent += 20 + value * 8;
        map.modifiers.push_back("Highborn: monsters gain " +
                                std::to_string(value) + " level" +
                                (value == 1 ? "" : "s"));
      }
    };
    apply_modifier(first);
    apply_modifier(second);
    item.name = "Tier " + std::to_string(map.tier) + " " + def->name;
    item.display_name = item.name;
    item.expedition_map = std::move(map);
  }

  if (!def->vessel_form.empty() && options.forge) {
    // adapter.js createVesselBlock: one rng draw reseeds the forge, then the
    // engine stream drives generation. The material hint is honoured only
    // when the form's material list admits it (adapter.js vesselHints).
    if (options.rng) {
      const double draw = options.rng->next();
      options.forge->reseed(static_cast<std::uint32_t>(std::floor(draw * 4294967296.0)));
    }
    const int ilvl = options.item_level > 0 ? std::min(80, options.item_level) : 10;
    std::string material_hint;
    if (!def->vessel_material.empty()) {
      if (const PackForm* form = pack_form(def->vessel_form)) {
        for (const auto* candidate : form->materials) {
          if (def->vessel_material == candidate) {
            material_hint = def->vessel_material;
            break;
          }
        }
      }
    }
    VesselItem vessel_item = options.forge->generate_item(ilvl, def->vessel_form, material_hint);
    VesselBlock block = options.forge->make_block(vessel_item);
    if (!block.display_name.empty()) {
      item.name = block.display_name;
      item.display_name = block.display_name;
    }
    // factory.js: vessel combat replaces the base stat sheet.
    item.attack = block.combat.attack;
    item.defense = block.combat.defense;
    item.combat_bonuses = block.combat.modifiers;
    if (block.combat.has_attributes) item.bonus_attributes = block.combat.attributes;
    item.bonus_health = block.combat.resource_health;
    item.bonus_mana = block.combat.resource_mana;
    item.vessel = std::move(block);
  }

  item.size = resolve_item_size(*def, item.vessel ? &*item.vessel : nullptr);
  item.qty = item.stackable ? std::max(1, options.quantity) : 1;

  // factory.js shouldBindOnPickup: weapons, armour and jewelry bind on
  // admission; currency never does. (The bindOnPickup catalogue flag is not
  // modelled — no N4 row uses it; noted as a stub in the task report.)
  if (!options.bind_to.empty() && !item.stackable &&
      (def->type == "weapon" || def->type == "armor" || def->type == "jewelry")) {
    item.bound_to = options.bind_to;
  }
  return item;
}

// ── PlayerInventory (inventory.js + inventory-footprints.js) ─────────────

bool PlayerInventory::fits_at(const GameItem& item, int slot) const {
  const int x0 = slot % kColumns;
  const int y0 = slot / kColumns;
  for (int dy = 0; dy < item.size.height; ++dy) {
    for (int dx = 0; dx < item.size.width; ++dx) {
      const int x = x0 + dx;
      const int y = y0 + dy;
      if (x >= kColumns || y >= kRows) return false;
      for (const auto& other : items_) {
        if (other.slot < 0) continue;  // unplaced stacks block nothing
        const int ox = other.slot % kColumns;
        const int oy = other.slot / kColumns;
        if (x >= ox && x < ox + other.size.width && y >= oy && y < oy + other.size.height) {
          return false;
        }
      }
    }
  }
  return true;
}

int PlayerInventory::first_fit(const GameItem& item) const {
  // findOpenInventorySlot: row-major scan, no rotation for server grants.
  for (int slot = 0; slot < kSlotCount; ++slot) {
    if (fits_at(item, slot)) return slot;
  }
  return -1;
}

PlayerInventory::AddResult PlayerInventory::add(GameItem item) {
  AddResult result;
  if (item.stackable) {
    // inventory.js: an existing stack of the same id absorbs the quantity;
    // the balance never needs a free cell and never overflows.
    for (auto& existing : items_) {
      if (existing.id == item.id) {
        existing.qty += item.qty;
        result.added = item.qty;
        return result;
      }
    }
    // JS parity: a new stack occupies a real backpack cell like any item;
    // -1 only when the grid is genuinely full (the balance still counts).
    item.slot = first_fit(item);
    result.added = item.qty;
    items_.push_back(std::move(item));
    return result;
  }
  const int slot = first_fit(item);
  if (slot < 0) {
    result.overflow.push_back(std::move(item));
    return result;
  }
  item.slot = slot;
  result.added = 1;
  items_.push_back(std::move(item));
  return result;
}

bool PlayerInventory::remove_by_uuid(const std::string& uuid, GameItem* out) {
  for (auto it = items_.begin(); it != items_.end(); ++it) {
    if (it->uuid == uuid) {
      if (out) *out = *it;
      items_.erase(it);
      return true;
    }
  }
  return false;
}

GameItem* PlayerInventory::find_by_uuid(const std::string& uuid) {
  for (auto& item : items_) {
    if (item.uuid == uuid) return &item;
  }
  return nullptr;
}

const GameItem* PlayerInventory::find_by_uuid(const std::string& uuid) const {
  for (const auto& item : items_) {
    if (item.uuid == uuid) return &item;
  }
  return nullptr;
}

int PlayerInventory::coin_total() const {
  int total = 0;
  for (const auto& item : items_) {
    if (item.id == "coins") total += item.qty;
  }
  return total;
}

bool PlayerInventory::spend_coins(int amount) {
  if (amount < 0 || coin_total() < amount) return false;
  int remaining = amount;
  for (auto it = items_.begin(); it != items_.end() && remaining > 0;) {
    if (it->id != "coins") {
      ++it;
      continue;
    }
    const int take = std::min(remaining, it->qty);
    it->qty -= take;
    remaining -= take;
    if (it->qty <= 0) {
      it = items_.erase(it);
    } else {
      ++it;
    }
  }
  return true;
}

// ── WearSet (wear-slots.js + wear.js) ────────────────────────────────────

const std::vector<std::string>& WearSet::physical_slots() {
  static const std::vector<std::string> slots = {
      "right_hand", "left_hand", "armor", "head", "back", "belt",
      "gloves", "feet", "ring", "ring2", "necklace",
  };
  return slots;
}

std::vector<std::string> WearSet::seats_for_base(const std::string& base_slot) {
  if (base_slot.empty()) return {};
  if (base_slot == "ring") return {"ring", "ring2"};
  return {base_slot};
}

bool WearSet::can_use_seat(const std::string& base_slot, const std::string& seat) {
  const auto seats = seats_for_base(base_slot);
  return std::find(seats.begin(), seats.end(), seat) != seats.end();
}

std::string WearSet::resolve_seat(const std::string& base_slot,
                                  const std::string& preferred) const {
  // wear-slots.js resolveEquipSlot.
  const auto seats = seats_for_base(base_slot);
  if (seats.empty()) return base_slot;
  if (!preferred.empty() &&
      std::find(seats.begin(), seats.end(), preferred) != seats.end()) {
    return preferred;
  }
  for (const auto& seat : seats) {
    if (slots_.count(seat) == 0) return seat;
  }
  return seats.back();
}

const GameItem* WearSet::in_seat(const std::string& seat) const {
  const auto it = slots_.find(seat);
  return it == slots_.end() ? nullptr : &it->second;
}

std::optional<GameItem> WearSet::equip(GameItem item, const std::string& seat) {
  std::optional<GameItem> displaced;
  const auto it = slots_.find(seat);
  if (it != slots_.end()) {
    displaced = it->second;
    slots_.erase(it);
  }
  item.slot = -1;  // worn items live outside the backpack grid
  slots_.emplace(seat, std::move(item));
  return displaced;
}

std::optional<GameItem> WearSet::unequip(const std::string& seat) {
  const auto it = slots_.find(seat);
  if (it == slots_.end()) return std::nullopt;
  GameItem item = it->second;
  slots_.erase(it);
  return item;
}

WearSet::Totals WearSet::totals() const {
  // wear.js calculateCombat: channel sums plus combatBonuses, capped.
  Totals out;
  auto clamp = [](int value, int hi) { return std::max(0, std::min(hi, value)); };
  for (const auto& [seat, item] : slots_) {
    out.attack.stab += item.attack.stab;
    out.attack.slash += item.attack.slash;
    out.attack.crush += item.attack.crush;
    out.attack.range += item.attack.range;
    out.defense.stab += item.defense.stab;
    out.defense.slash += item.defense.slash;
    out.defense.crush += item.defense.crush;
    out.defense.range += item.defense.range;
    out.modifiers.block_chance += item.combat_bonuses.block_chance;
    out.modifiers.critical_chance += item.combat_bonuses.critical_chance;
    out.modifiers.goods_found += item.combat_bonuses.goods_found;
    out.modifiers.damage_against_beasts += item.combat_bonuses.damage_against_beasts;
  }
  out.modifiers.block_chance = clamp(out.modifiers.block_chance, 75);
  out.modifiers.critical_chance = clamp(out.modifiers.critical_chance, 75);
  out.modifiers.goods_found = clamp(out.modifiers.goods_found, 100);
  out.modifiers.damage_against_beasts = clamp(out.modifiers.damage_against_beasts, 100);
  return out;
}

// ── loot.js helpers ──────────────────────────────────────────────────────

int apply_goods_found_to_coins(int coins, int goods_found_percent) {
  const int percent = std::max(0, std::min(100, goods_found_percent));
  return std::max(0, static_cast<int>(std::floor(std::max(0, coins) * (1.0 + percent / 100.0))));
}

double apply_goods_found_to_gear_chance(double chance, int goods_found_percent) {
  const int percent = std::max(0, std::min(100, goods_found_percent));
  return std::min(0.75, std::max(0.0, chance) * (1.0 + percent / 100.0));
}

int instance_item_level_for_depth(int depth) {
  return std::min(80, 10 + (std::max(1, depth) - 1) * 10);
}

// ── WorldSimulation N4: ground items, loot, depth chaining ───────────────

std::uint64_t WorldSimulation::next_world_random() {
  // splitmix64: JS draws from Math.random here, so any independent stream is
  // faithful; a seeded one keeps runs replayable for the architect.
  world_random_state_ += 0x9e3779b97f4a7c15ULL;
  std::uint64_t z = world_random_state_;
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}

namespace {

double world_rand01(std::uint64_t draw) {
  return static_cast<double>(draw >> 11) * (1.0 / 9007199254740992.0);
}

}  // namespace

void WorldSimulation::add_ground_item(GameItem item, double x, double y) {
  GroundItem ground;
  ground.item = std::move(item);
  ground.x = x;
  ground.y = y;
  ground.timestamp = static_cast<std::int64_t>(++serial_);
  ground_items_.push_back(std::move(ground));
}

void WorldSimulation::add_relic_ground_item(GameItem item, double x, double y,
                                            const std::string& relic_id,
                                            const std::string& source_scion_id,
                                            const std::string& source_scion_name) {
  GroundItem ground;
  ground.item = std::move(item);
  ground.x = x;
  ground.y = y;
  ground.timestamp = static_cast<std::int64_t>(++serial_);
  ground.relic_record_id = relic_id;
  ground.relic_source_scion_id_field = source_scion_id;
  ground.relic_source_scion_name_field = source_scion_name;
  ground_items_.push_back(std::move(ground));
}

bool WorldSimulation::take_ground_item(const std::string& uuid, GameItem* out) {
  for (auto it = ground_items_.begin(); it != ground_items_.end(); ++it) {
    if (it->item.uuid == uuid) {
      if (out) *out = it->item;
      ground_items_.erase(it);
      return true;
    }
  }
  return false;
}

Vec2 WorldSimulation::resolve_loot_tile(int x, int y) const {
  // loot.js resolveLootLocation: never on stairs/portals or blocked tiles;
  // spiral outward (top row, bottom row, left column, right column per
  // radius), falling back to the origin.
  auto safe = [&](int tx, int ty) {
    if (!grid_.walkable_at(tx, ty)) return false;
    if (tx == metadata_.stairs_up.x && ty == metadata_.stairs_up.y) return false;
    if (tx == metadata_.stairs_down.x && ty == metadata_.stairs_down.y) return false;
    return true;
  };
  if (safe(x, y)) return {x, y};
  for (int radius = 1; radius <= 6; ++radius) {
    for (int offset = -radius; offset <= radius; ++offset) {
      const Vec2 candidates[] = {
          {x + offset, y - radius},
          {x + offset, y + radius},
          {x - radius, y + offset},
          {x + radius, y + offset},
      };
      for (const auto& candidate : candidates) {
        if (safe(candidate.x, candidate.y)) return candidate;
      }
    }
  }
  return {x, y};
}

void WorldSimulation::drop_monster_loot(const WorldMonster& monster, int goods_found_percent) {
  // loot.js dropMonsterLoot: coin bounty always (Wealthy-boosted), then a
  // rarity-gated gear roll. Relic/trophy circulation and the first-find
  // grant are Chronicles/encounter features — N5 stubs (see the report).
  const Vec2 tile = resolve_loot_tile(monster.x, monster.y);
  const int coins = apply_goods_found_to_coins(monster.coins, goods_found_percent);
  if (coins > 0) {
    CreateItemOptions opts;
    opts.quantity = coins;
    auto coin_item = create_game_item("coins", opts);
    if (coin_item) add_ground_item(std::move(*coin_item), tile.x, tile.y);
  }

  double base_chance = 0.05;
  if (monster.rarity == "uncommon") base_chance = 0.1;
  if (monster.rarity == "rare") base_chance = 0.2;
  if (monster.rarity == "elite") base_chance = 0.5;
  const double chance = apply_goods_found_to_gear_chance(base_chance, goods_found_percent);
  const bool guaranteed = guaranteed_elite_gear_ && monster.rarity == "elite";
  if (guaranteed || world_rand01(next_world_random()) < chance) {
    const auto& pool = gear_drop_pool();
    const std::string& gear_id =
        pool[static_cast<std::size_t>(std::floor(world_rand01(next_world_random()) * pool.size()))];
    CreateItemOptions opts;
    opts.item_level = std::min(80, monster.level * 2);
    opts.forge = &forge_;
    // factory.js createById with rng: one draw reseeds the forge.
    const double reseed_draw = world_rand01(next_world_random());
    forge_.reseed(static_cast<std::uint32_t>(std::floor(reseed_draw * 4294967296.0)));
    opts.rng = nullptr;  // reseed already performed; generate from the stream
    auto gear = create_game_item(gear_id, opts);
    if (gear) add_ground_item(std::move(*gear), tile.x, tile.y);
  }
}

void WorldSimulation::scatter_floor_treasure() {
  // map.js guaranteed per-floor treasure hoard: a coin purse plus one gear
  // piece whose item level scales with depth. The JS server scatters these
  // at treasure-room centres from gearPoolForDepth; this port uses the map
  // centre and the shared drop pool (documented stub in the task report).
  if (grid_.width <= 0 || grid_.height <= 0) return;
  const int cx = grid_.width / 2;
  const int cy = grid_.height / 2;

  const int coins = 80 + static_cast<int>(std::floor(world_rand01(next_world_random()) * 60.0));
  const Vec2 coin_tile = resolve_loot_tile(cx, cy);
  CreateItemOptions coin_opts;
  coin_opts.quantity = coins;
  auto coin_item = create_game_item("coins", coin_opts);
  if (coin_item) add_ground_item(std::move(*coin_item), coin_tile.x, coin_tile.y);

  const auto& pool = gear_drop_pool();
  const std::string& gear_id =
      pool[static_cast<std::size_t>(std::floor(world_rand01(next_world_random()) * pool.size()))];
  const double reseed_draw = world_rand01(next_world_random());
  forge_.reseed(static_cast<std::uint32_t>(std::floor(reseed_draw * 4294967296.0)));
  CreateItemOptions gear_opts;
  gear_opts.item_level = instance_item_level_for_depth(metadata_.depth);
  gear_opts.forge = &forge_;
  auto gear = create_game_item(gear_id, gear_opts);
  if (gear) {
    const Vec2 gear_tile = resolve_loot_tile(cx, cy + 1);
    add_ground_item(std::move(*gear), gear_tile.x, gear_tile.y);
  }
}

void WorldSimulation::transition_floor(int depth) {
  // party.js transitionFloor: same template/layout, regenerated at the new
  // depth; the player re-enters at the floor's spawn. Floor ground items
  // retire with the old floor, exactly like JS scene retirement.
  const std::string theme = metadata_.theme;
  const std::string layout = metadata_.layout;
  serial_ += 1;
  const int clamped_depth = std::max(1, depth);
  metadata_ = InstanceMetadata{};
  metadata_.seed = fnv1a(theme + ":" + layout + ":floor-" + std::to_string(clamped_depth), seed_);
  metadata_.theme = theme;
  metadata_.layout = layout;
  metadata_.depth = clamped_depth;
  ground_items_.clear();
  active_target_.clear();
  boss_warning_seen_ = false;
  next_boss_telegraph_ms_ = 0;
  generate_instance();
  scene_type_ = "instance";
  scene_id_ = "instance:" + theme + ":" + (layout.empty() ? "default" : layout);
  scene_name_ = zone_display_name(theme, layout, clamped_depth);
  position_.x = static_cast<double>(metadata_.spawn_points.front().x);
  position_.y = static_cast<double>(metadata_.spawn_points.front().y);
}

}  // namespace verdigris
