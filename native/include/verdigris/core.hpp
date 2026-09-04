#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
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

// D-114 world-scale table. Distances derive from the player's current
// walking cadence (220 world units/second, 11 units/tick), rather than being
// independent presentation guesses. Keep this table as the single source
// for the combat, expedition, and client collision envelope:
//
//   measure                  derivation                         value / time
//   player walk step         220 units/s * 50 ms                 11 u/tick
//   melee contact            13 walk ticks                      143 u / .65 s
//   thrust contact           melee * 1.5                        214 u / .97 s
//   extraction interaction   8 walk ticks                       88 u / .40 s
//   enemy spawn              melee * 5                         715 u / 3.25 s
//   arena half-extent        melee * 6                         858 u / 3.90 s
//   actor collider            melee / 5                          28 u
//   scenery collider         melee / 2                          71 u
//
// The table intentionally keeps first contact inside the owner's 0.5–0.8
// second readability target while leaving enough arena for approach,
// extraction, and grounded scenery to share the same relative scale.
namespace world_scale {
inline constexpr int kPlayerMoveSpeed = 220;
inline constexpr int kPlayerStepPerTick = movement_step_per_tick(kPlayerMoveSpeed);
inline constexpr int kMeleeContactTicks = 13;
inline constexpr int kMeleeRange = kPlayerStepPerTick * kMeleeContactTicks;
inline constexpr int kThrustRange = (kMeleeRange * 3) / 2;
inline constexpr int kExtractionContactTicks = 8;
inline constexpr int kExtractionRange = kPlayerStepPerTick * kExtractionContactTicks;
inline constexpr int kEnemySpawnDistance = kMeleeRange * 5;
inline constexpr int kArenaHalfExtent = kMeleeRange * 6;
inline constexpr int kActorColliderRadius = kMeleeRange / 5;
inline constexpr int kSceneryColliderRadius = kMeleeRange / 2;
}  // namespace world_scale

// A dash is a short, readable burst measured in ordinary movement ticks.
inline constexpr int kDashMovementTicks = 10;

// Curated gameplay constants needed by presentation.  Mechanics and the
// read-only catalog use these same definitions; clients must not mirror the
// values independently.
namespace presentation_constants {
inline constexpr int kMeleeRange = world_scale::kMeleeRange;
inline constexpr int kThrustRange = world_scale::kThrustRange;
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
  TrophyResurfaced,
  // Appended last so recorded command streams and stored event ordinals keep
  // their historical numeric codes.
  ExpeditionPhaseChanged
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
  Unequip,
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
  static Command unequip();
  static Command enter(const std::string& route_id);
  static Command extract();
};

// Authoritative first-expedition objective for an active instance: defeat
// every warden on the floor, then carry the value back to the extraction
// point. The simulation owns the transition (last living monster dies);
// presentation reads it instead of re-deriving the loop from actor scans.
// The phase is descriptive telemetry, never a command gate: extraction rules
// are unchanged. It resets to SlayWardens on every instance entry and dies
// with the instance.
enum class ExpeditionPhase { SlayWardens, ExtractCarriedValue };

struct InstanceState {
  bool active = false;
  std::string route_id;
  Vec2 extraction_point{0, 0};
  std::vector<std::string> ground_item_ids;
  std::vector<std::string> ground_trophy_ids;
  ExpeditionPhase phase = ExpeditionPhase::SlayWardens;
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

  // Wardens of the active instance that have not materialized yet. The first
  // expedition reveals its pack deterministically: when a kill leaves roster
  // entries owed, they all materialize together kTelegraphTicks later on
  // their fixed anchors. Like the rest of the live instance state, the
  // pending roster is retired at every instance boundary and is deliberately
  // absent from durable snapshots.
  const std::vector<Actor>& pending_wave() const { return pending_wave_; }

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
  void resolve_unequip();
  void resolve_enter(const std::string& route_id);
  void resolve_extract();
  void retire_instance();
  void advance_tick();
  void enemy_turn();
  Actor make_monster(Vec2 position, int level, bool elite);
  void spawn_enemy();
  void materialize_wave();
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
  // Unmaterialized warden roster of the active instance (see pending_wave()).
  std::vector<Actor> pending_wave_;
  // Tick at which the remaining owed pack materializes together; 0 when
  // nothing is scheduled. A kill inside an active instance re-arms it
  // deterministically.
  std::uint64_t wave_materialization_tick_ = 0;
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

// ────────────────────────────────────────────────────────────────────────────
// Parity wave N4: items, inventory, and Vesselforge data rules.
//
// These types mirror the browser server's item pipeline:
//  - server/core/items/vesselforge/engine.js + verdigris-pack.js (the seeded
//    Vesselforge generator, brand tables, tooltip + combat derivation)
//  - server/core/items/factory.js (catalogue -> instance, vessel attach)
//  - server/shared/inventory-footprints.js (12x7 grid, footprint resolution)
//  - server/shared/wear-slots.js + server/core/utilities/wear.js (equip seats,
//    combat totals)
//  - server/core/combat/loot.js (kill drops, Wealthy boost, loot tiles)
// All rules live here; the networking layer only maps envelopes.
// ────────────────────────────────────────────────────────────────────────────

// mulberry32 exactly as engine.js/dev.js speak it (uint32 wraparound, /2^32).
class Mulberry32 {
 public:
  explicit Mulberry32(std::uint32_t seed = 0) : state_(seed) {}
  double next();
  int rint(int min, int max) { return min + static_cast<int>(std::floor(next() * (max - min + 1))); }

 private:
  std::uint32_t state_;
};

struct VesselBrand {
  std::string id;
  std::string mod_id;
  int tier = 1;
  int value = 0;
};

struct VesselBond {
  std::string id;
  std::string mod_id;
  std::string theme_id;
  int base = 0;
  int tier = 1;
};

struct VesselTrophy {
  std::string id;
  std::string trophy_id;
};

struct VesselTrophyDefinition {
  std::string id;
  std::string name;
  int fragments = 0;
  std::vector<std::string> kinds;
  std::string stat_id;
  int value = 0;
  std::string label;
  std::string completion_bonus;
};

// WIZARD verdigris-pack trophy data in its authored declaration order.
const std::vector<VesselTrophyDefinition>& vessel_trophy_definitions();

struct VesselAttunement {
  int xp = 0;
  int next = 80;
  std::map<std::string, int> theme_counts;
};

struct VesselAwakened {
  std::string name;
  std::string theme_id;
  std::string power;
  std::string flavor;
};

struct VesselEvolutionEvent {
  std::string kind;  // bond | awake
  std::string text;
};

struct VesselItem {
  std::string id;
  std::string form_id;
  std::string material_id;
  std::string kind;
  int w = 1;
  int h = 1;
  int ilvl = 10;
  int vessel = 0;
  int scars = 0;
  int patience = 0;
  int patience_max = 0;
  std::vector<VesselBrand> brands;
  std::vector<VesselBond> bonds;
  std::vector<VesselTrophy> trophies;
  VesselAttunement attunement;
  int evolutions = 0;
  std::string epithet_name;
  std::optional<VesselAwakened> awakened;
};

struct TooltipLine {
  std::string section;
  std::string text;
  std::string tone = "normal";
};

struct ChannelRatings {
  int stab = 0;
  int slash = 0;
  int crush = 0;
  int range = 0;
};

struct CombatModifiers {
  int block_chance = 0;
  int critical_chance = 0;
  int attack_speed_percent = 0;
  int goods_found = 0;
  int damage_against_beasts = 0;
  int bleed_chance = 0;
  int reach_percent = 0;
  int projectile_range_percent = 0;
  int armour_penetration_percent = 0;
  int movement_speed_percent = 0;
  int ember_resistance = 0;
  int river_resistance = 0;
  // Living-item Bond powers. These remain separate from always-on Brands so
  // the authoritative combat simulation can evaluate their real triggers.
  int health_on_kill_percent = 0;
  int attack_speed_on_kill_percent = 0;
  int critical_against_bleeding_percent = 0;
  int health_on_block = 0;
  int stationary_block_chance = 0;
  int armour_on_hit_percent = 0;
  int ability_power_high_resource_percent = 0;
  int resource_on_kill_percent = 0;
  int curse_avoid_percent = 0;
  int movement_speed_on_kill_percent = 0;
  int thrown_avoid_while_moving_percent = 0;
  int health_regen_while_moving = 0;
  bool awakened_echoing_kill = false;
  bool awakened_last_stand = false;
  bool awakened_twinned_voice = false;
  bool awakened_untraceable = false;
};

struct VesselCombat {
  ChannelRatings attack;
  ChannelRatings defense;
  CombatModifiers modifiers;
  bool has_attributes = false;
  int attributes = 0;
  int resource_health = 0;
  int resource_mana = 0;
  // Weapon sheet (adapter.js combat.damage) and ward total.
  bool has_damage = false;
  int damage_min = 0;
  int damage_max = 0;
  double attacks_per_second = 0;
  int dps = 0;
  int rating = 0;
  std::string channel;
  int ward = 0;
};

struct VesselBlock {
  VesselItem item;
  std::string pack_id = "verdigris-1";
  std::string material;
  int material_tier = 1;
  std::string form;
  std::string display_name;
  std::vector<TooltipLine> lines;
  VesselCombat combat;
};

// The seeded Vesselforge rules (engine.js createForge + adapter.js
// createVesselBlock/refreshVesselBlock/deriveVesselCombat). One instance per
// session, matching the JS module-level singleton: generation reseeds from
// the caller's rng; sear() advances the persistent stream.
class VesselForge {
 public:
  VesselForge();

  void reseed(std::uint32_t seed) { rand_ = Mulberry32(seed); }
  Mulberry32& rand() { return rand_; }
  // engine.js generateItem with explicit formId (+ optional materialId).
  VesselItem generate_item(int ilvl, const std::string& form_id,
                           const std::string& material_id = "");
  // engine.js sear: spend 1 patience, roll + append a brand. False when the
  // vessel cannot take another brand.
  bool sear(VesselItem& item);
  // engine.js socketTrophy: spend one complete House fragment set, consume
  // one compatible Vessel slot, and keep both inputs untouched on failure.
  bool socket_trophy(VesselItem& item, const std::string& trophy_id,
                     std::map<std::string, int>& fragment_stash,
                     std::string* error = nullptr);
  // WIZARD living-item progression adapted to classless Scions: completed
  // expeditions contribute theme memory, cross 80 + 55*evolution thresholds,
  // form/deepen Bonds, and eventually awaken a sufficiently capacious item.
  std::vector<VesselEvolutionEvent> attune(
      VesselItem& item, int xp,
      const std::map<std::string, int>& theme_weights,
      const std::string& scion_name);
  int used_slots(const VesselItem& item) const;
  bool is_sated(const VesselItem& item) const;
  // adapter.js honest tooltip (dormant marking for inactive lines).
  std::vector<TooltipLine> tooltip(const VesselItem& item) const;
  // adapter.js deriveVesselCombat.
  VesselCombat derive_combat(const VesselItem& item) const;
  // adapter.js refreshVesselBlock: packId/material/form/displayName/lines/combat.
  VesselBlock make_block(const VesselItem& item) const;

  // engine.js pickWeighted entry shape; public so the free helper can
  // operate on pools built by the forge.
  struct WeightedEntry {
    std::string id;
    double weight = 0;
  };

 private:
  std::string gen_id();
  std::vector<WeightedEntry> brand_pool(const VesselItem& item) const;
  bool roll_brand(VesselItem& item, VesselBrand* out);
  bool roll_bond(VesselItem& item, VesselBond* out);
  std::string dominant_bond_theme(const VesselItem& item) const;
  std::optional<VesselEvolutionEvent> evolve(
      VesselItem& item, const std::string& scion_name);

  Mulberry32 rand_;
  std::uint64_t id_counter_ = 0;
};

// Catalogue row (server/core/data/items/*): the curated bases the N4
// scenarios touch plus the 13 Vesselforge-native vessel-* entries.
struct ItemDef {
  std::string id;
  std::string name;
  std::string type;  // weapon, armor, jewelry, currency
  std::string slot;  // base equip slot ("" = not equippable)
  bool stackable = false;
  bool two_handed = false;
  ChannelRatings attack;
  ChannelRatings defense;
  int size_w = 0;  // 0 = derive via the footprint rules
  int size_h = 0;
  std::string vessel_form;      // vesselForm / vesselforge.formId hint
  std::string vessel_material;  // vesselMaterial hint ("" = roll from pool)
};

const ItemDef* item_def(const std::string& id);
// loot.js GEAR_DROP_POOL order.
const std::vector<std::string>& gear_drop_pool();

struct ItemSize {
  int width = 1;
  int height = 1;
};

// A consumable endgame chart. The complete roll travels with the item so a
// tablet shown in the backpack is the exact expedition the server opens;
// neither the transport nor presentation layer reconstructs its rules.
struct ExpeditionMapBlock {
  int tier = 1;
  std::string family;
  std::string objective_key;
  std::string theme;
  std::string layout;
  int monster_level_bonus = 0;
  int monster_life_percent = 0;
  int monster_damage_percent = 0;
  int extra_monsters = 0;
  int goods_found_percent = 0;
  std::vector<std::string> modifiers;
};

// Recharting changes only a tablet's rolled risk/reward clauses. Keeping the
// cost shared prevents the native Framekit hint from inventing economy truth.
inline constexpr int kExpeditionRechartCost = 50;

// inventory-footprints.js resolveItemSize (explicit size first, then the
// weapon/armour id rules, then equipment-slot defaults).
ItemSize resolve_item_size(const ItemDef& def, const VesselBlock* vessel);

// A live item instance (factory.js createFromBase output shape).
struct GameItem {
  std::string id;
  std::string uuid;
  std::string name;
  std::string display_name;
  int qty = 1;
  int slot = -1;  // backpack cell index; -1 outside the backpack grid
  ItemSize size;
  bool stackable = false;
  bool two_handed = false;
  std::string equip_slot;
  ChannelRatings attack;
  ChannelRatings defense;
  CombatModifiers combat_bonuses;
  int bonus_health = 0;
  int bonus_mana = 0;
  int bonus_attributes = 0;
  std::optional<VesselBlock> vessel;
  std::optional<ExpeditionMapBlock> expedition_map;
  std::string bound_to;

  int item_level() const {
    if (vessel) return vessel->item.ilvl;
    return expedition_map ? expedition_map->tier : 0;
  }
};

struct CreateItemOptions {
  Mulberry32* rng = nullptr;   // deterministic vessel roll when provided
  int item_level = 0;          // 0 = engine default (10)
  int quantity = 1;
  std::string bind_to;         // factory shouldBindOnPickup rules apply
  VesselForge* forge = nullptr;
};

// factory.js createById/createFromBase. Returns nullopt for unknown ids.
std::optional<GameItem> create_game_item(const std::string& item_id,
                                         const CreateItemOptions& options);
// Replaces every projection derived from one vessel roll as an atomic model
// update. Crafting/attunement callers must not refresh only tooltip text.
void apply_vessel_block(GameItem& item, VesselBlock block);

// The 12x7 spatial backpack (inventory-footprints.js + inventory.js add()).
class PlayerInventory {
 public:
  static constexpr int kColumns = 12;
  static constexpr int kRows = 7;
  static constexpr int kSlotCount = kColumns * kRows;

  struct AddResult {
    int added = 0;
    std::vector<GameItem> overflow;  // instances that did not fit
  };

  const std::vector<GameItem>& items() const { return items_; }
  std::vector<GameItem>& items() { return items_; }
  void clear() { items_.clear(); }

  // inventory.js add(): currency merges into the existing stack and never
  // overflows; other items place first-fit. One instance per call — callers
  // loop for multi-quantity grants so each roll gets its own rng draw.
  AddResult add(GameItem item);
  bool remove_by_uuid(const std::string& uuid, GameItem* out);
  GameItem* find_by_uuid(const std::string& uuid);
  const GameItem* find_by_uuid(const std::string& uuid) const;
  int coin_total() const;
  bool spend_coins(int amount);  // false when coin_total() < amount

 private:
  bool fits_at(const GameItem& item, int slot) const;
  int first_fit(const GameItem& item) const;  // -1 when no placement exists

  std::vector<GameItem> items_;
};

// wear-slots.js + wear.js: physical seats and combat totals.
class WearSet {
 public:
  static const std::vector<std::string>& physical_slots();
  // Base item slot -> seats in fill order (ring -> [ring, ring2]).
  static std::vector<std::string> seats_for_base(const std::string& base_slot);
  static bool can_use_seat(const std::string& base_slot, const std::string& seat);
  // resolveEquipSlot: honour a valid explicit seat, else first empty, else
  // the last seat (a swap).
  std::string resolve_seat(const std::string& base_slot,
                           const std::string& preferred = "") const;

  const std::map<std::string, GameItem>& slots() const { return slots_; }
  std::map<std::string, GameItem>& mutable_slots() { return slots_; }
  void clear() { slots_.clear(); }
  const GameItem* in_seat(const std::string& seat) const;
  // Place an item into a seat; returns the displaced item when swapping.
  std::optional<GameItem> equip(GameItem item, const std::string& seat);
  std::optional<GameItem> unequip(const std::string& seat);

  struct Totals {
    ChannelRatings attack;
    ChannelRatings defense;
    CombatModifiers modifiers;  // defensive caps 75; offensive/utility caps 100
  };
  Totals totals() const;

 private:
  std::map<std::string, GameItem> slots_;
};

// A ground item in a scene (loot.js toWorldInstance shape). Coordinates are
// the raw continuous drop position (dev drops/overflow land exactly at the
// player's feet); monster loot is spiralled onto integer tiles first.
struct GroundItem {
  GameItem item;
  double x = 0;
  double y = 0;
  std::int64_t timestamp = 0;  // placement time; menus sort newest-first
  // N5 relic circulation provenance (server/core/services/chronicles.js
  // drawCirculatingRelic): set when this drop is a recovered heirloom.
  std::string relic_record_id;
  std::string relic_source_scion_id_field;
  std::string relic_source_scion_name_field;
};

// Player combat modifiers the equip pipeline feeds into tile-space combat.
struct PlayerCombatMods {
  int block_chance = 0;
  int armour_rating = 0;
  int critical_chance = 0;
  int attack_speed_percent = 0;
  int goods_found = 0;
  int damage_against_beasts = 0;
  int bleed_chance = 0;
  int reach_percent = 0;
  int projectile_range_percent = 0;
  int armour_penetration_percent = 0;
  int movement_speed_percent = 0;
  int ember_resistance = 0;
  int river_resistance = 0;
  int health_on_kill_percent = 0;
  int attack_speed_on_kill_percent = 0;
  int critical_against_bleeding_percent = 0;
  int health_on_block = 0;
  int stationary_block_chance = 0;
  int armour_on_hit_percent = 0;
  int ability_power_high_resource_percent = 0;
  int resource_on_kill_percent = 0;
  int curse_avoid_percent = 0;
  int movement_speed_on_kill_percent = 0;
  int thrown_avoid_while_moving_percent = 0;
  int health_regen_while_moving = 0;
  bool awakened_echoing_kill = false;
  bool awakened_last_stand = false;
  bool awakened_twinned_voice = false;
  bool awakened_untraceable = false;
  bool force_critical = false;
  std::string attack_style = "slash";
};

// loot.js applyGoodsFoundToCoins / applyGoodsFoundToGearChance.
int apply_goods_found_to_coins(int coins, int goods_found_percent);
double apply_goods_found_to_gear_chance(double chance, int goods_found_percent);
// map.js instanceItemLevelForDepth: min(80, 10 + (depth-1)*10).
int instance_item_level_for_depth(int depth);

// ────────────────────────────────────────────────────────────────────────────
// Parity wave N2: tile-space world rules.
//
// These rules mirror the browser server's continuous movement contract
// (server/shared/movement.js + movement-handler.js): one player:move sample
// advances the player 1/3 tile along an 8-way normalised vector, positions
// round to 6 decimals with an integer snap, and blocking is checked against
// the rounded target tile (with the both-orthogonal-neighbours diagonal
// rule).  The combat Simulation above keeps the D-114 world-unit envelope;
// this section is the tile-space world the wire protocol speaks.
// ────────────────────────────────────────────────────────────────────────────

struct WorldPosition {
  double x = 0;
  double y = 0;
};

namespace tile_movement {
// Browser feel constants (post-0037): one tile takes 150 ms, a held key
// samples every 50 ms, so each sample moves exactly 1/3 tile.
inline constexpr double kTileTravelMs = 150.0;
inline constexpr double kSampleMs = 50.0;
inline constexpr double kMoveDistance = kSampleMs / kTileTravelMs;
inline constexpr int kPositionPrecision = 6;

// Normalised 8-way sample delta (PLAYER_MOVE_DISTANCE along the vector).
// Returns nullopt for unknown direction names.
std::optional<WorldPosition> movement_delta(const std::string& direction);
// Number(v).toFixed(6) with the JS integer snap (within 2e-6 of a tile).
double round_position(double value);
// Math.round per axis (positions are non-negative in practice).
Vec2 occupied_tile(const WorldPosition& position);
}  // namespace tile_movement

struct TileGrid {
  int width = 0;
  int height = 0;
  // Row-major, 1 = walkable, 0 = blocked.  N2 stub: the JS server derives
  // this from tileset/object tables; generated instance maps and the town
  // field carry their walkability directly until N3+ ports the tile tables.
  std::vector<std::uint8_t> walkable;

  bool in_bounds(int x, int y) const;
  bool walkable_at(int x, int y) const;  // out of bounds = blocked
};

struct WorldMonster {
  std::string uuid;
  std::string id;
  std::string name;
  int x = 0;
  int y = 0;
  int level = 1;
  int life = 30;
  int life_max = 30;
  int armour = 0;
  bool alive = true;
  std::string behaviour_type = "melee";
  std::string rarity = "common";
  std::vector<std::string> modifiers;
  bool empowered = false;
  bool boss = false;
  std::uint64_t telegraph_until_ms = 0;
  std::uint64_t next_attack_ms = 0;
  std::uint64_t next_move_ms = 0;
  // Authoritative role action state. Ranged foes sample a destination tile
  // when their warning begins; resolution checks that tile rather than the
  // player's old position so movement can genuinely dodge the volley.
  std::string pending_attack_skill;
  int pending_target_x = 0;
  int pending_target_y = 0;
  std::string damage_channel = "physical";
  std::uint64_t bleed_until_ms = 0;
  std::uint64_t next_bleed_tick_ms = 0;
  int bleed_damage = 0;
  // N4: loot/behaviour facts the wire snapshot carries (JS m.rewards.coins
  // and m.tags).
  std::vector<std::string> tags;
  int coins = 0;
};

struct WorldCombatEvent {
  std::string type; // move, hit, heal, death, telegraph, interrupt, drop
  std::string attacker_id;
  std::string attacker_name;
  std::string target_id;
  std::string target_name;
  std::string skill_id;
  int amount = 0;
  int health = 0;
  int health_max = 0;
  bool died = false;
  int radius = 0;
  int inner_radius = 0;
  int duration_ms = 0;
  int x = 0;
  int y = 0;
  std::string telegraph_shape;
  std::string item_id;
  // N4: combat:hit parity fields (server/core/combat/index.js).
  int base_amount = 0;
  int beastbane_amount = 0;
  int beastbane_percent = 0;
  bool beastbane = false;
  bool critical = false;
  std::string attack_style = "slash";
  // Server-resolved primary-attack cadence. Named skills carry step 0.
  // The third step is the finisher and may stagger a non-boss target.
  int combo_step = 0;
  int combo_window_ms = 0;
  int stagger_ms = 0;
  std::string damage_channel = "physical";
  int resistance_percent = 0;
  int armour_rating = 0;
  int armour_prevented = 0;
  int armour_penetration_percent = 0;
};

// One authoritative Warden mechanic. Road and tablet content select a
// profile before the instance is generated; the simulation owns warning
// placement, hit geometry, timing, mitigation, and cooldown resolution.
struct BossAbilityProfile {
  std::string skill_id = "boss:ground-slam";
  std::string telegraph_shape = "circle";  // circle or ring
  std::string damage_channel = "physical";
  int radius = 2;
  int inner_radius = 0;
  int windup_ms = 1000;
  int cooldown_ms = 1400;
  int damage = 12;
  bool targets_player = false;
};

struct InstanceMetadata {
  std::uint64_t seed = 0;
  std::string theme;    // zone template (dungeon/grove/crypt/wilds/marsh)
  std::string layout;   // warren/clearings/gauntlet, empty = theme default
  int depth = 1;
  Vec2 stairs_up{0, 0};
  Vec2 stairs_down{0, 0};
  std::vector<Vec2> spawn_points;
};

struct MovementStepInfo {
  std::uint64_t sequence = 0;
  std::int64_t started_at_ms = 0;
  int duration_ms = 0;
  std::string direction;
  bool blocked = false;
};

struct ZoneDescriptor {
  std::string id;
  std::string name;
  std::string template_id;
  std::string layout;
};

// The Adventure-menu table (party.js ADVENTURE_ZONES): art template + layout
// shape pair with the display name the HUD/transition payload carries.
const std::vector<ZoneDescriptor>& adventure_zones();
bool is_zone_template(const std::string& template_id);
bool is_zone_layout(const std::string& layout);

// Deterministic tile-space world for one player: town scene, solo instance
// scenes, continuous movement, and stair portals.  Transport-agnostic; the
// networking layer maps envelopes onto these verbs.
class WorldSimulation {
 public:
  WorldSimulation(std::uint64_t seed, std::string player_uuid);

  // Scene state.
  const std::string& scene_id() const { return scene_id_; }
  const std::string& scene_type() const { return scene_type_; }
  const std::string& scene_name() const { return scene_name_; }
  WorldPosition position() const { return position_; }
  const std::string& facing() const { return facing_; }
  const MovementStepInfo& last_step() const { return last_step_; }
  const InstanceMetadata& metadata() const { return metadata_; }
  const std::vector<WorldMonster>& monsters() const { return monsters_; }
  // loot.js: Proof of Temper guarantees the first elite gear drop while
  // the slay-elite objective is current (session sets this per tick).
  void set_guaranteed_elite_gear(bool value) { guaranteed_elite_gear_ = value; }
  // world-web node instances: the boss carries the node warden name; a
  // cleared node spawns no monsters at all (dead stays dead).
  void set_boss_name_override(const std::string& name) { boss_name_override_ = name; }
  void set_boss_ability_override(const BossAbilityProfile& profile);
  void clear_boss_ability_override();
  void set_spawn_suppressed(bool value) { spawn_suppressed_ = value; }
  // Applied before enter_solo_instance for a consumed charted tablet. These
  // values affect only the generated instance and are reset on return.
  void set_expedition_tuning(int level_bonus, int life_percent,
                             int damage_percent, int extra_monsters) {
    expedition_level_bonus_ = std::max(0, level_bonus);
    expedition_life_percent_ = std::max(0, life_percent);
    expedition_damage_percent_ = std::max(0, damage_percent);
    expedition_extra_monsters_ = std::max(0, extra_monsters);
  }
  void clear_expedition_tuning() {
    expedition_level_bonus_ = 0;
    expedition_life_percent_ = 0;
    expedition_damage_percent_ = 0;
    expedition_extra_monsters_ = 0;
  }
  void set_scene_name(const std::string& name) { scene_name_ = name; }
  void set_scene_id(const std::string& id) { scene_id_ = id; }
  // dev:monster:reset - revive one monster at a chosen max health for
  // deterministic comparison trials.
  bool reset_monster(const std::string& uuid, int max_health,
                     int armour = -1) {
    for (auto& monster : monsters_) {
      if (monster.uuid != uuid) continue;
      monster.alive = true;
      if (max_health > 0) monster.life_max = max_health;
      monster.life = monster.life_max;
      if (armour >= 0) monster.armour = armour;
      monster.telegraph_until_ms = 0;
      monster.next_attack_ms = 0;
      monster.next_move_ms = 0;
      monster.pending_attack_skill.clear();
      monster.bleed_until_ms = 0;
      monster.next_bleed_tick_ms = 0;
      monster.bleed_damage = 0;
      return true;
    }
    return false;
  }
  void kill_all_monsters() { for (auto& monster : monsters_) { monster.alive = false; monster.life = 0; } active_target_.clear(); pending_player_skill_.clear(); pending_player_combo_step_ = 0; player_combo_step_ = 0; player_combo_expires_ms_ = 0; auto_player_melee_ = false; }
  const TileGrid& grid() const { return grid_; }
  bool in_instance() const { return scene_type_ == "instance"; }

  // One player:move sample.  Returns true when the step was applied.
  bool apply_movement_sample(const std::string& direction, std::int64_t now_ms);
  // A dash is resolved by the same tile collision/portal rules as ordinary
  // movement, but advances up to the authored ten movement samples in one
  // authoritative action. It never enters the attack pipeline.
  bool apply_dash(const std::string& direction, std::int64_t now_ms);
  // dev:teleport: floors onto the target tile, then runs the portal check
  // (landing on the entry stairs returns to town, like the JS game loop).
  void teleport(int x, int y, std::int64_t now_ms);
  // Fresh world admission lands at the town spawn (JS direct-admission: every
  // login re-enters the world at the plaza, whatever a prior session left).
  void reset_to_town();
  // instance:enterSolo: validates template/layout against the Adventure table
  // (unknown template -> dungeon, unknown layout -> theme default), saves the
  // the surface context on first entry, and places the player at a spawn.
  void enter_solo_instance(const std::string& template_id,
                           const std::string& layout, int depth = 1);
  // Same resolution as walking the depth-1 entry stairs: retire the instance
  // and restore the stashed town scene. player:extract and stairs-up both
  // converge here (TASK-0063).
  void return_to_surface();
  // N3 deterministic combat seam. The transport supplies the authoritative
  // player actor's level/life; this world owns tile-space targets and emits
  // protocol-ready facts without putting networking into the core.
  // Queues exactly one accepted combat action. Cooldown and range are owned
  // here so repeated wire inputs cannot reset the cadence or manufacture
  // extra hits. The caller deducts the skill's authoritative resource cost
  // only when this returns true.
  bool start_player_attack(int player_level, int player_attack,
                           std::int64_t now_ms, const std::string& direction,
                           const std::string& skill_id = "melee");
  std::vector<WorldCombatEvent> advance_combat(int player_level, int player_attack,
                                               int& player_life, int player_life_max,
                                               std::int64_t now_ms,
                                               int* player_resource = nullptr,
                                               int player_resource_max = 0);
  int player_cooldown_remaining_ms(std::int64_t now_ms) const;
  int player_combo_step(std::int64_t now_ms) const;
  int player_combo_window_remaining_ms(std::int64_t now_ms) const;
  void set_level(int level);
  void heal_player(int& player_life, int player_life_max);
  // Display name for a template/layout pair (falls back to template-only,
  // then a capitalised template), matching the JS zone naming.
  static std::string zone_display_name(const std::string& template_id, const std::string& layout,
                                       int depth = 1);

  // ── N4: items, inventory, and depth ─────────────────────────────────────
  // Ground items for the CURRENT scene. The town list is stashed across
  // instance hops; per-floor lists retire with the floor, exactly like JS
  // scene retirement (documented in the N4 report).
  const std::vector<GroundItem>& ground_items() const { return ground_items_; }
  // The per-session forge (JS module singleton): generation reseeds it, the
  // brand service advances its persistent stream.
  VesselForge& forge() { return forge_; }
  // dev:drop / world-drop / overflow spill: place an item on the current
  // scene at the raw (x, y) position.
  void add_ground_item(GameItem item, double x, double y);
  // N5 relic circulation: place a recovered heirloom with its fallen-scion
  // provenance (drawCirculatingRelic in server/core/services/chronicles.js).
  void add_relic_ground_item(GameItem item, double x, double y,
                             const std::string& relic_id,
                             const std::string& source_scion_id,
                             const std::string& source_scion_name);
  // Remove a ground item by uuid (take). Returns false when absent.
  bool take_ground_item(const std::string& uuid, GameItem* out);
  // Kill rewards: coins always (Wealthy-boosted) + rarity-gated gear roll.
  // Mirrors dropMonsterLoot in server/core/combat/loot.js.
  void drop_monster_loot(const WorldMonster& monster, int goods_found_percent);
  // Equip-aware combat modifiers for the next advance_combat calls.
  void set_player_combat_mods(const PlayerCombatMods& mods) { player_mods_ = mods; }
  PlayerCombatMods& player_combat_mods() { return player_mods_; }
  int bond_attack_speed_remaining_ms(std::int64_t now_ms) const;
  int bond_movement_speed_remaining_ms(std::int64_t now_ms) const;
  int bond_old_grudge_remaining_ms(std::int64_t now_ms) const;
  bool bond_last_stand_ready() const {
    return player_mods_.awakened_last_stand && last_stand_available_;
  }
  bool bond_untraceable_ready() const {
    return player_mods_.awakened_untraceable && untraceable_available_;
  }

 private:
  bool can_move_to(double target_x, double target_y) const;
  bool is_blocked(const WorldPosition& origin, const WorldPosition& delta) const;
  void register_step(const std::string& direction, int duration_ms, bool blocked,
                     std::int64_t now_ms);
  void generate_instance();
  void return_to_town();
  // Portal check after a step/teleport changed the occupied tile.
  void check_stair_transition();
  // N4: depth>1 floor chaining (party.js transitionFloor).
  void transition_floor(int depth);
  // N4: guaranteed per-floor treasure (map.js treasure hoard scatter).
  void scatter_floor_treasure();
  // N4: safe loot tile spiral (loot.js resolveLootLocation).
  Vec2 resolve_loot_tile(int x, int y) const;
  std::uint64_t next_world_random();
  std::uint64_t next_combat_random();

  std::uint64_t seed_;
  std::string player_uuid_;
  std::uint64_t serial_ = 0;

  WorldPosition position_{38.0, 115.0};  // Crossroads fountain / arrival anchor
  std::string facing_ = "down";
  std::string scene_type_ = "town";
  std::string scene_id_ = "town:verdigris";
  std::string scene_name_ = "Verdigris";
  TileGrid grid_;
  InstanceMetadata metadata_;
  std::vector<WorldMonster> monsters_;
  std::string active_target_;
  bool guaranteed_elite_gear_ = false;
  std::string boss_name_override_;
  BossAbilityProfile boss_ability_{};
  bool spawn_suppressed_ = false;
  bool block_stairs_down_ = false;
  bool stairs_up_returns_to_town_ = false;
  int expedition_level_bonus_ = 0;
  int expedition_life_percent_ = 0;
  int expedition_damage_percent_ = 0;
  int expedition_extra_monsters_ = 0;
  std::string engaged_by_;
public:
  void set_block_stairs_down(bool value) { block_stairs_down_ = value; }
  void set_stairs_up_returns_to_town(bool value) { stairs_up_returns_to_town_ = value; }
  // shared party worlds: swings resolve only on the session that engaged.
  void set_engaged_by(const std::string& identity) { engaged_by_ = identity; }
  const std::string& engaged_by() const { return engaged_by_; }
private:
  std::uint64_t next_player_attack_ms_ = 0;
  std::string pending_player_skill_;
  int pending_player_combo_step_ = 0;
  int player_combo_step_ = 0;
  std::uint64_t player_combo_expires_ms_ = 0;
  bool auto_player_melee_ = false;
  std::uint64_t next_boss_telegraph_ms_ = 0;
  bool boss_warning_seen_ = false;
  int player_level_ = 1;
  MovementStepInfo last_step_;
  // Where the player entered the current instance chain from (first entry
  // only, not instance->instance hops), restored on stair return.
  bool has_pre_instance_ = false;
  WorldPosition pre_instance_position_{};
  std::string pre_instance_scene_id_;
  // N4 state.
  std::vector<GroundItem> ground_items_;
  std::vector<GroundItem> town_ground_items_;
  PlayerCombatMods player_mods_;
  std::uint64_t last_player_move_ms_ = 0;
  std::uint64_t bond_attack_speed_until_ms_ = 0;
  std::uint64_t bond_movement_speed_until_ms_ = 0;
  std::uint64_t bond_old_grudge_until_ms_ = 0;
  std::uint64_t next_moving_regen_ms_ = 0;
  bool last_stand_available_ = true;
  bool untraceable_available_ = true;
  std::uint64_t world_random_state_ = 0x9e3779b97f4a7c15ULL;
  std::uint64_t combat_random_state_ = 0xd1b54a32d192ed03ULL;
  VesselForge forge_;
};

}  // namespace verdigris
