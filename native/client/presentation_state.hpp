#pragma once

// Shared presentation world + event-to-FX mapping (TASK-0064).
// paint_scene in the native client and the remote render-list session test
// consume the same WorldView / EffectFx records. Neither path may construct
// a Simulation in remote mode.

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "camera2d.hpp"
#include "client_model.hpp"
#include "presentation_events.hpp"
#include "render_list.hpp"
#include "verdigris/core.hpp"

namespace verdigris::client {

struct EffectFx {
  enum class Kind {
    Swing,
    SweepArc,
    WarCryAura,
    Impact,
    DeathRing,
    Dust,
    Sparkle,
    DamageNumber,
    TargetFlash,
    // TASK-0122 Phase A beats. All lifetimes come from the phase_a constants
    // table in presentation_events.hpp; none of them touch simulation state.
    Materialize,    // deterministic first-sighting spawn beat
    WarCryFade,     // BuffExpired("war-cry") contract beat
    ScionLostBeat,  // ScionLost contract beat
  };
  Kind kind = Kind::Impact;
  double wx = 0.0;
  double wy = 0.0;
  double angle = 0.0;
  int age = 0;
  int ttl = 8;
  int value = 0;
  bool damage_to_player = false;
  // TASK-0122 Phase A: critical/style parity data copied from the event.
  bool critical = false;
  std::string style;
};

struct ActiveTelegraph {
  std::string actor_id;
  std::string action;
  verdigris::Vec2 position;
  verdigris::Vec2 facing{1, 0};
  std::uint64_t start_tick = 0;
  int windup_ticks = 1;
};

struct WorldActor {
  std::string id;
  verdigris::Vec2 position{};
  verdigris::Vec2 facing{1, 0};
  int life = 0;
  int life_max = 0;
  int resource = 0;
  int resource_max = 0;
  int attack = 0;
  int defense = 0;
  int level = 1;
  int cooldown_ticks = 0;
  int war_cry_ticks_remaining = 0;
  bool alive = true;
  bool elite = false;
};

struct WorldCarriedItem {
  std::string id;
  std::string name;
  int attack_bonus = 0;
  bool equipped = false;
};

// TASK-0153: owner-facing expedition phase. The local path reads the core's
// authoritative Simulation::instance().phase directly; the remote path
// mirrors it from the already-authoritative session snapshot (living foes).
// It is descriptive telemetry for the objective strip — never a client-side
// rule engine, and Unknown wherever no authoritative scene exists.
enum class ExpeditionPhaseView { Unknown, SlayWardens, ExtractCarriedValue };

struct WorldView {
  WorldActor player;
  std::vector<WorldActor> monsters;
  verdigris::Vec2 extraction{};
  bool has_extraction = false;
  std::string house_name = "House Verdigris";
  std::string scion_name;
  std::uint64_t tick = 0;
  std::vector<WorldCarriedItem> carried;
  std::size_t stored_items = 0;
  std::size_t stored_trophies = 0;
  std::size_t carried_trophies = 0;
  std::string route_id;
  ExpeditionPhaseView expedition_phase = ExpeditionPhaseView::Unknown;
  std::unordered_map<std::string, std::string> loot_names;
  // TASK-0156: verbatim mirror of the authoritative passive-tree progression
  // payload. Default (present=false) means no payload has arrived; local
  // play stays absent because the core carries no tree authority.
  ClientPassiveProgression progression;
};

struct PresentationFx {
  std::vector<EffectFx> effects;
  std::unordered_map<std::string, ActiveTelegraph> telegraphs;
  std::unordered_map<std::string, verdigris::Vec2> loot_positions;
  verdigris::Vec2 last_death_pos{};
  int loot_scatter = 0;
  int screen_pulse_ticks = 0;
  std::vector<std::string> event_log;
  std::string hint;
  int hint_ticks = 0;
  // TASK-0122 Phase A: monster ids already seen by the presentation, so the
  // spawn/materialization beat fires exactly once per foe. Presentation
  // bookkeeping only — it never creates, moves, or damages an actor.
  std::unordered_set<std::string> known_monsters;
};

verdigris::Vec2 facing_vector(const std::string& facing);

// TASK-0122 Phase A: push a deterministic materialization beat for every
// monster present in `world` that the presentation has never seen before.
// Pure presentation bookkeeping over the already-authoritative snapshot.
void detect_monster_spawns(PresentationFx& fx, const WorldView& world,
                           std::uint64_t now_tick);

// Protocol scene coordinates are tile-sized; native presentation uses D-114
// world units. One protocol tile equals one ground-grid tile.
double protocol_to_world(double protocol_units);

void sync_world_from_simulation(WorldView& world, const verdigris::Simulation& sim);
void sync_world_from_model(WorldView& world, const ClientModel& model);

// TASK-0153 mode-aware extraction contract: the one owner-facing action
// phrase for reaching the exit. Local play resolves an F interaction at the
// pad; the remote protocol has no extract handler and extracts by walking
// onto stairs-up. The objective strip must never show the wrong one.
const char* extraction_action_hint(bool remote_session);

void apply_presentation_event(PresentationFx& fx, const WorldView& world,
                              const PresentationEvent& event, std::uint64_t now_tick);

void age_presentation_fx(PresentationFx& fx);

// Semantic recorder used by the remote render-list session test. Positions
// match camera2d::project so ops are the same vocabulary paint_scene records.
void record_world_ops(render::List& rl, const WorldView& world, const PresentationFx& fx,
                      const camera2d::Camera& camera, int width, int height);

}  // namespace verdigris::client
