#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace verdigris {

struct Vec2 {
  int x = 0;
  int y = 0;
};

int manhattan_distance(Vec2 a, Vec2 b);

class Simulation;
class SeasonalMechanic;

enum class ActorKind { Player, Monster };

// Keep the original values stable for recorded command streams; new skills
// are appended to the action vocabulary.
enum class ActionType { Melee, Dash, Wait, Thrust, Sweep, WarCry };

// A telegraph is emitted this many simulation ticks before an elite skill
// resolves.  It is part of the simulation contract so every presentation can
// render the same warning window.
inline constexpr int kTelegraphTicks = 3;

// Simulation commands resolve at a fixed 20 Hz cadence.  Actor move_speed is
// expressed in world units per second; this named derivation keeps movement
// deterministic while preserving the recorded MoveIntent shape.
inline constexpr int kSimulationTickMs = 50;

constexpr int movement_step_per_tick(int move_speed) {
  return std::max(1, move_speed * kSimulationTickMs / 1000);
}

// A dash is a short, readable burst measured in ordinary movement ticks.
inline constexpr int kDashMovementTicks = 10;

// Curated gameplay constants needed by presentation.  Mechanics and the
// read-only catalog use these same definitions; clients must not mirror the
// values independently.
namespace presentation_constants {
inline constexpr int kMeleeRange = 1100;
inline constexpr int kThrustRange = (kMeleeRange * 3) / 2;
inline constexpr int kThrustResourceCost = 10;
inline constexpr int kSweepResourceCost = 15;
inline constexpr int kWarCryResourceCost = 20;
inline constexpr int kResourceRegenPerTick = 2;
inline constexpr int kWarCryAttackBonus = 4;
inline constexpr int kWarCryDurationTicks = 20;
}  // namespace presentation_constants

struct PresentationCatalog {
  int thrust_resource_cost = 0;
  int sweep_resource_cost = 0;
  int war_cry_resource_cost = 0;
  int melee_range = 0;
  int thrust_range = 0;
  int telegraph_ticks = 0;
  int war_cry_attack_bonus = 0;
  int war_cry_duration_ticks = 0;
  int resource_regen_per_tick = 0;

  bool operator==(const PresentationCatalog& other) const;
};

struct ActorStats {
  int level = 1;
  int strength = 10;
  int dexterity = 10;
  int intelligence = 10;
  int life_max = 100;
  int life = 100;
  int resource_max = 50;
  int resource = 50;
  int attack = 10;
  int defense = 5;
  int move_speed = 100;
  int attack_speed_ticks = 4;
  int resistances = 0;

  bool operator==(const ActorStats& other) const;
};

struct Item {
  std::string id;
  std::string name;
  int attack_bonus = 0;
  std::string owner_id;
  int use_count = 0;
  bool equipped = false;
  bool relic_candidate = false;
  std::vector<std::string> history;
};

struct Trophy {
  std::string id;
  std::string name;
};

struct Actor {
  std::string id;
  ActorKind kind = ActorKind::Player;
  ActorStats stats;
  Vec2 position;
  bool alive = true;
  int cooldown_ticks = 0;
  std::optional<std::string> equipped_item_id;
  bool elite = false;
  // Temporary combat effects are actor state so the same action/damage
  // pipeline can be used for players and monsters.
  int war_cry_attack_bonus = 0;
  int war_cry_ticks_remaining = 0;
  // Elite monster skills are scheduled by the simulation and resolved through
  // the same action pipeline as player skills.  Wait means no action is
  // pending; the tick counter is the remaining windup.
  ActionType pending_action = ActionType::Wait;
  int pending_action_ticks = 0;
  // Deterministic 8-way facing. Each component is -1, 0, or +1; the
  // direction is intentionally not normalized with floating-point math.
  Vec2 facing{1, 0};
};

struct RouteNode {
  std::string id;
  std::string parent_id;
  std::vector<std::string> children;
  bool optional = false;
};

struct LegendEntry;

struct House {
  std::string id;
  std::string name;
  std::vector<RouteNode> routes;
  std::vector<std::string> unlocked_routes;
  std::vector<std::string> cleared_routes;
  std::vector<std::string> specializations;
  std::vector<Trophy> stored_trophies;
  std::vector<Item> stored_items;
  std::vector<Item> relic_candidates;
  // Trophies carried by a fallen Scion remain recoverable through the same
  // seeded reward stream as relic items, without becoming durable storage.
  std::vector<Trophy> lost_trophies;
  std::vector<std::string> seasonal_rewards;
  std::vector<LegendEntry> legends;
  bool campaign_complete = false;

  bool route_unlocked(const std::string& route_id) const;
  bool route_cleared(const std::string& route_id) const;
};

inline constexpr std::size_t kLegendCapacity = 64;

struct LegendEntry {
  std::uint64_t ordinal = 0;
  std::uint64_t tick = 0;
  std::string scion_id;
  std::string scion_name;
  std::string kind;
  std::string subject;
  std::string detail;
  std::string killer_id;
  std::string route_id;
  bool founding = false;

  bool operator==(const LegendEntry& other) const;
};

struct Scion {
  std::string id;
  std::string name;
  int level = 1;
  bool alive = true;
  std::string actor_id;
  std::vector<Trophy> carried_trophies;
  std::vector<Item> carried_items;
  std::vector<std::string> deeds;
};

enum class EventType {
  HouseCreated,
  ScionCreated,
  InstanceEntered,
  ActorMoved,
  AttackStarted,
  DamageApplied,
  ActorDied,
  ItemDropped,
  TrophyDropped,
  ItemPickedUp,
  TrophyPickedUp,
  ItemEquipped,
  ItemHistoryUpdated,
  ItemExtracted,
  TrophyExtracted,
  HouseStoreChanged,
  RouteUnlocked,
  BranchUnlocked,
  SeasonalObjectiveAdded,
  SeasonalRewardGranted,
  ScionLost,
  LegendRecorded,
  RelicResurfaced,
  BuffApplied,
  BuffExpired,
  AttackTelegraphed,
  TrophyResurfaced
};

struct Event {
  EventType type;
  std::string actor_id;
  std::string item_id;
  std::string trophy_id;
  std::string text;
  int value = 0;
  std::uint64_t tick = 0;
};

enum class CommandType {
  MoveIntent,
  UseAction,
  Interact,
  PickUp,
  Equip,
  EnterInstance,
  ExtractToHouse,
  AimIntent
};

struct Command {
  CommandType type;
  int dx = 0;
  int dy = 0;
  ActionType action = ActionType::Wait;
  std::string target;

  static Command move(int x, int y);
  static Command aim(int x, int y);
  static Command action_use(ActionType action);
  static Command interact(const std::string& target);
  static Command pick_up(const std::string& item_id);
  static Command equip(const std::string& item_id);
  static Command enter(const std::string& route_id);
  static Command extract();
};

struct InstanceState {
  bool active = false;
  std::string route_id;
  Vec2 extraction_point{0, 0};
  std::vector<std::string> ground_item_ids;
  std::vector<std::string> ground_trophy_ids;
  bool seasonal_objective = false;
  std::string seasonal_objective_text;
};

class Simulation {
 public:
  explicit Simulation(std::uint64_t seed, const std::string& house_name = "House Verdigris");

  void dispatch(const Command& command);
  void set_seasonal_mechanic(SeasonalMechanic* mechanic);
  void create_successor(const std::string& name);

  const House& house() const;
  const Scion& scion() const;
  const std::vector<Scion>& fallen_scions() const;
  const std::vector<Actor>& actors() const;
  const InstanceState& instance() const;
  const std::vector<Item>& ground_items() const;
  const std::vector<Trophy>& ground_trophies() const;
  const std::vector<Event>& events() const;
  const std::vector<LegendEntry>& legends() const;
  std::uint64_t tick() const;

  // Curated, read-only gameplay constants for presentation clients. This is
  // deliberately not a state snapshot or a general simulation export.
  static PresentationCatalog presentation_catalog();

  const Actor* actor(const std::string& id) const;
  Actor* actor(const std::string& id);

  // General deterministic content seam. Callers may add an additional
  // opponent without changing the combat implementation or test-only state.
  std::string spawn_monster(Vec2 position, int level = 1, bool elite = false);

  // Stable hooks used by external seasonal mechanics and deterministic tests.
  void grant_seasonal_reward(const std::string& reward);
  void add_seasonal_objective(const std::string& description);
  static int resolve_damage(const Actor& attacker, const Actor& defender, int item_bonus = 0);

  // Durable persistence is deliberately a free-function boundary.  The
  // serializer owns no I/O and does not expose live instance state.
  friend std::vector<std::uint8_t> snapshot(const Simulation& simulation);
  friend Simulation restore(const std::vector<std::uint8_t>& bytes);

 private:
  struct Rng {
    explicit Rng(std::uint64_t value) : state(value) {}
    std::uint64_t next();
    int range(int min, int max);
    std::string token(const std::string& prefix);

    std::uint64_t state;
    std::uint64_t serial = 0;
  };

  void emit(EventType type, const std::string& actor_id = {}, const std::string& item_id = {},
            const std::string& trophy_id = {}, const std::string& text = {}, int value = 0);
  void resolve_move(int dx, int dy);
  void resolve_aim(int dx, int dy);
  void resolve_action(ActionType action);
  void resolve_actor_action(Actor& attacker, ActionType action);
  void resolve_interact(const std::string& target);
  void resolve_pickup(const std::string& item_id);
  void resolve_equip(const std::string& item_id);
  void resolve_enter(const std::string& route_id);
  void resolve_extract();
  void retire_instance();
  void advance_tick();
  void enemy_turn();
  void spawn_enemy();
  void record_equipped_item_use(Actor& attacker);
  void drop_reward();
  void clear_route_and_unlock_children();
  void handle_death(Actor& actor, const std::string& killer_id = {});
  void record_legend(const std::string& kind, const std::string& subject,
                     const std::string& detail = {}, const std::string& killer_id = {},
                     const std::string& route_id = {}, bool founding = false);
  int equipped_attack_bonus() const;
  bool at_extraction() const;

  Rng rng_;
  House house_;
  Scion scion_;
  std::vector<Scion> fallen_scions_;
  std::vector<Actor> actors_;
  InstanceState instance_;
  std::vector<Item> ground_items_;
  std::vector<Trophy> ground_trophies_;
  // A surfaced recovery candidate remains recoverable across an instance
  // retirement. It is reattached to the next active instance, while ordinary
  // floor drops are discarded.
  std::vector<Item> pending_relic_items_;
  std::vector<Trophy> pending_relic_trophies_;
  // Trophy IDs currently borrowed from House recovery. They return to that
  // pool if the active instance retires before pickup; ordinary floor drops
  // remain lost on retirement.
  std::vector<std::string> resurfaced_trophy_ids_;
  std::vector<Event> events_;
  SeasonalMechanic* seasonal_mechanic_ = nullptr;
  std::uint64_t tick_ = 0;
  std::uint64_t next_legend_ordinal_ = 1;
};

// Versioned, deterministic durable state.  Snapshot bytes are canonical for
// identical House/Scion/RNG state; an active instance is retired at the
// snapshot boundary under D-109 (carried value remains carried).
std::vector<std::uint8_t> snapshot(const Simulation& simulation);
Simulation restore(const std::vector<std::uint8_t>& bytes);

}  // namespace verdigris
