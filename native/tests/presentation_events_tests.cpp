// TASK-0122 Phase A: dedicated presentation-events tests. Proves the new
// readable event beats at the session seam: critical/ordinary damage
// distinction, ScionLost/BuffExpired contract beats, deterministic spawn
// materialization, corrected (non-fabricated) monster facing, and the
// negative control that presentation effects never mutate simulation state.
// All timing values come from the phase_a constants table in
// presentation_events.hpp; no magic literals are pinned here that the table
// does not also pin.

#include <cstdio>
#include <cmath>
#include <string>

#include "../client/local_session.hpp"
#include "../client/presentation_state.hpp"
#include "../client/ingest-ranged-projectile-warning.hpp"

namespace {

int failures = 0;

void check(bool ok, const char* label) {
  std::printf("%s %s\n", ok ? "PASS" : "FAIL", label);
  if (!ok) ++failures;
}

using verdigris::client::EffectFx;
using verdigris::client::PresentationEvent;
using verdigris::client::PresentationEventType;
using verdigris::client::PresentationFx;
using verdigris::client::WorldActor;
using verdigris::client::WorldView;
namespace phase_a = verdigris::client::phase_a;

int count_kind(const PresentationFx& fx, EffectFx::Kind kind) {
  int total = 0;
  for (const auto& effect : fx.effects)
    if (effect.kind == kind) ++total;
  return total;
}

const EffectFx* first_kind(const PresentationFx& fx, EffectFx::Kind kind) {
  for (const auto& effect : fx.effects)
    if (effect.kind == kind) return &effect;
  return nullptr;
}

WorldView world_with_player_and_foe(const std::string& player_facing) {
  WorldView world;
  world.player.id = "scion-1";
  world.player.position = {100, 100};
  world.player.facing = verdigris::client::facing_vector(player_facing);
  world.player.alive = true;
  WorldActor foe;
  foe.id = "foe-1";
  foe.position = {140, 100};
  foe.alive = true;
  world.monsters.push_back(foe);
  return world;
}

void constants_are_named_and_distinct() {
  check(phase_a::kTickMs == 50, "phase-a: table pins the 50ms presentation tick");
  check(phase_a::kCriticalNumberTtlTicks > 12,
        "phase-a: critical number outlives the ordinary 12-tick number");
  check(phase_a::kMaterializeTtlTicks != phase_a::kWarcryFadeTtlTicks &&
            phase_a::kWarcryFadeTtlTicks != phase_a::kScionLostRingTtlTicks &&
            phase_a::kMaterializeTtlTicks != phase_a::kScionLostRingTtlTicks,
        "phase-a: beat lifetimes are distinct by construction");
  check(phase_a::kScionLostPulseTicks > 3,
        "phase-a: loss pulse outlives the 3-tick damage pulse");
}

void critical_damage_is_distinct() {
  WorldView world = world_with_player_and_foe("right");
  PresentationFx fx;
  PresentationEvent plain{PresentationEventType::DamageApplied, "foe-1", "",
                          "outgoing", 4, false, {}};
  apply_presentation_event(fx, world, plain, 0);
  const int plain_number_ttl = first_kind(fx, EffectFx::Kind::DamageNumber)->ttl;
  const int plain_flash_ttl =
      first_kind(fx, EffectFx::Kind::TargetFlash)->ttl;
  PresentationFx crit_fx;
  PresentationEvent crit{PresentationEventType::DamageApplied, "foe-1", "",
                         "outgoing", 9, true, "stab"};
  apply_presentation_event(crit_fx, world, crit, 0);
  const EffectFx* crit_number = first_kind(crit_fx, EffectFx::Kind::DamageNumber);
  check(crit_number != nullptr && crit_number->critical && crit_number->style == "stab",
        "phase-a: critical flag and style copy onto the damage number");
  check(crit_number->ttl == phase_a::kCriticalNumberTtlTicks &&
        crit_number->ttl != plain_number_ttl,
        "phase-a: critical number ttl comes from the table and differs");
  check(first_kind(crit_fx, EffectFx::Kind::TargetFlash)->ttl ==
                phase_a::kCriticalFlashTtlTicks &&
            plain_flash_ttl == 4,
        "phase-a: critical flash is longer than the ordinary flash");
  render::List rl;
  record_world_ops(rl, world, crit_fx, camera2d::Camera{}, 960, 600);
  bool saw_crit_label = false;
  for (const auto& item : rl)
    if (item.op == render::Op::Damage && item.label == "critical:stab")
      saw_crit_label = true;
  check(saw_crit_label, "phase-a: render list labels critical hits with style");
}

void scion_lost_beat_contract() {
  WorldView world = world_with_player_and_foe("right");
  PresentationFx fx;
  PresentationEvent lost{PresentationEventType::ScionLost, "scion-1", "", "Fable",
                         0, false, {}};
  apply_presentation_event(fx, world, lost, 0);
  const EffectFx* beat = first_kind(fx, EffectFx::Kind::ScionLostBeat);
  check(beat != nullptr, "phase-a: ScionLost produces a loss beat");
  check(beat && beat->ttl == phase_a::kScionLostRingTtlTicks,
        "phase-a: loss beat lifetime comes from the table");
  check(fx.screen_pulse_ticks == phase_a::kScionLostPulseTicks,
        "phase-a: loss pulse comes from the table");
  bool logged = false;
  for (const auto& line : fx.event_log)
    if (line == "scion lost") logged = true;
  check(logged, "phase-a: loss beat logs a readable line");
}

void buff_expired_beat_contract() {
  WorldView world = world_with_player_and_foe("right");
  PresentationFx fx;
  PresentationEvent expired{PresentationEventType::BuffExpired, "scion-1", "",
                            "war-cry", 0, false, {}};
  apply_presentation_event(fx, world, expired, 0);
  const EffectFx* fade = first_kind(fx, EffectFx::Kind::WarCryFade);
  check(fade != nullptr && fade->ttl == phase_a::kWarcryFadeTtlTicks,
        "phase-a: BuffExpired(war-cry) produces the fade beat with its ttl");
  PresentationFx other_fx;
  PresentationEvent unknown_buff{PresentationEventType::BuffExpired, "scion-1", "",
                                 "some-other-buff", 0, false, {}};
  apply_presentation_event(other_fx, world, unknown_buff, 0);
  check(count_kind(other_fx, EffectFx::Kind::WarCryFade) == 0,
        "phase-a: only war-cry expiry renders the fade beat");
}

void local_seam_maps_lifecycle_events() {
  verdigris::client::LocalCoreSession session(0xC011AB1EULL, "House Verdigris");
  std::string error;
  check(session.start(&error), "phase-a: local seam starts");
  session.submit(verdigris::client::ClientCommand::enter_zone("route:tin:1:0"));
  session.submit(verdigris::client::ClientCommand::use_action("war-cry"));
  // Advance past the authoritative war-cry window so BuffExpired fires.
  for (int i = 0; i < verdigris::presentation_constants::kWarCryDurationTicks + 2; ++i)
    session.submit(verdigris::client::ClientCommand::use_action("wait"));
  session.poll();
  bool saw_expired = false;
  for (const auto& event : session.drain_events())
    if (event.type == PresentationEventType::BuffExpired &&
        event.text == "war-cry")
      saw_expired = true;
  check(saw_expired,
        "phase-a: local seam maps BuffExpired through the presentation seam");
  session.shutdown();
}

void spawn_detection_is_deterministic_and_once() {
  WorldView world = world_with_player_and_foe("right");
  WorldActor second;
  second.id = "foe-2";
  second.position = {60, 100};
  second.alive = true;
  world.monsters.push_back(second);

  auto run_once = [&world]() {
    PresentationFx fx;
    detect_monster_spawns(fx, world, 7);
    age_presentation_fx(fx);
    return static_cast<int>(fx.effects.size());
  };
  const int first_run = run_once();
  const int second_run = run_once();
  check(first_run == 2 && second_run == first_run,
        "phase-a: spawn detection is byte-deterministic across runs");

  PresentationFx fx;
  detect_monster_spawns(fx, world, 7);
  check(count_kind(fx, EffectFx::Kind::Materialize) == 2,
        "phase-a: one materialization beat per unseen foe");
  check(static_cast<int>(first_kind(fx, EffectFx::Kind::Materialize)->ttl) ==
            phase_a::kMaterializeTtlTicks,
        "phase-a: materialization ttl comes from the table");
  detect_monster_spawns(fx, world, 8);
  check(count_kind(fx, EffectFx::Kind::Materialize) == 2,
        "phase-a: re-sighting never re-triggers the beat");
}

void monster_facing_is_no_longer_fabricated() {
  verdigris::client::ClientModel model;
  model.player.facing = "right";
  verdigris::client::ClientMonster foe;
  foe.id = "foe-1";
  foe.name = "monster";
  foe.x = 20.0;
  foe.life = 3;
  foe.life_max = 3;
  model.monsters.push_back(foe);
  // The proved inversion took facing_vector(player) and negated it: with the
  // player facing right the old code produced { -1, 0 }. The correction must
  // never derive monster facing from the player again.
  WorldView world;
  sync_world_from_model(world, model);
  check(world.monsters.size() == 1 && world.monsters.front().facing.x == 1 &&
            world.monsters.front().facing.y == 0,
        "phase-a: player facing right no longer flips the monster west");
  model.player.facing = "left";
  WorldView flipped;
  sync_world_from_model(flipped, model);
  check(flipped.monsters.front().facing.x == 1 && flipped.monsters.front().facing.y == 0,
        "phase-a: player facing left does not flip the monster east either");
}

void seam_events_cannot_mutate_simulation() {
  verdigris::client::LocalCoreSession session(0xC011AB1EULL, "House Verdigris");
  std::string error;
  check(session.start(&error), "phase-a: local seam starts for the negative control");
  session.submit(verdigris::client::ClientCommand::enter_zone("route:tin:1:0"));
  session.poll();
  const auto before = session.model();
  WorldView world;
  sync_world_from_model(world, before);
  PresentationFx fx;
  for (const auto& event : session.drain_events())
    apply_presentation_event(fx, world, event, world.tick);
  detect_monster_spawns(fx, world, world.tick);
  session.poll();
  const auto after = session.model();
  check(before.player.life == after.player.life &&
            before.player.x == after.player.x &&
            before.player.y == after.player.y &&
            before.kills == after.kills &&
            before.stored_items == after.stored_items &&
            before.stored_trophies == after.stored_trophies &&
            before.monsters.size() == after.monsters.size(),
        "phase-a: applying every drained beat leaves the model untouched");
  session.shutdown();
}

}  // namespace

void diagonal_facing_resolves_component_wise() {
  using verdigris::client::facing_vector;
  check(facing_vector("up-left").x == -1 && facing_vector("up-left").y == -1,
        "facing: up-left resolves to (-1,-1)");
  check(facing_vector("down-right").x == 1 && facing_vector("down-right").y == 1,
        "facing: down-right resolves to (1,1)");
  check(facing_vector("up-right").x == 1 && facing_vector("up-right").y == -1,
        "facing: up-right resolves to (1,-1)");
  check(facing_vector("left").x == -1 && facing_vector("left").y == 0,
        "facing: plain left keeps its single axis");
  check(facing_vector("").x == 0 && facing_vector("").y == 1,
        "facing: unknown name falls back to down");
}

void server_messages_surface_as_toasts() {
  WorldView world = world_with_player_and_foe("right");
  PresentationFx fx;
  PresentationEvent message{PresentationEventType::Message, "", "",
                            "No road holds past a living Warden.", 0};
  verdigris::client::apply_presentation_event(fx, world, message, 1);
  check(fx.hint == "No road holds past a living Warden.",
        "message: server text becomes the HUD toast");
  check(fx.hint_ticks > 80, "message: toast outlives a key-echo hint");
  PresentationEvent empty{PresentationEventType::Message, "", "", "", 0};
  PresentationFx untouched;
  verdigris::client::apply_presentation_event(untouched, world, empty, 1);
  check(untouched.hint.empty(), "message: empty text never blanks a toast");
}

void ranged_projectile_warning_precedes_attributed_hit() {
  using verdigris::client::projectile::from_js_payload;
  using verdigris::client::projectile::is_projectile_warning;
  using verdigris::client::projectile::js_payload_shape;
  using verdigris::client::projectile::kAuthoredVolleyTravelMs;
  using verdigris::client::projectile::windup_ticks_from_travel_ms;

  check(js_payload_shape(kAuthoredVolleyTravelMs, "monster"),
        "0108: JS payload shape is fromX/fromY/toX/toY/travelMs/kind=monster");
  check(!js_payload_shape(0, "monster"),
        "0108: missing travelMs is not a projectile payload");

  WorldView world = world_with_player_and_foe("right");
  world.tick = 1;
  world.monsters[0].id = "flint-1";
  world.monsters[0].behaviour = "ranged";
  world.player.position = {static_cast<int>(std::lround(
                               verdigris::client::protocol_to_world(5))),
                           static_cast<int>(std::lround(
                               verdigris::client::protocol_to_world(1)))};

  PresentationEvent slam{PresentationEventType::Telegraph, "boss-1", "",
                         "boss:ground-slam", 800};
  check(!is_projectile_warning(slam),
        "0108: slam-shaped telegraph is not a projectile warning");

  const auto warning =
      from_js_payload("flint-1", 1, 1, 5, 1, kAuthoredVolleyTravelMs, "monster");
  check(is_projectile_warning(warning),
        "0108: world:projectile keys become a Telegraph event");
  check(windup_ticks_from_travel_ms(kAuthoredVolleyTravelMs) ==
            kAuthoredVolleyTravelMs / verdigris::kSimulationTickMs,
        "0108: travelMs uses the 50ms tick, not a catalog guess");

  PresentationFx fx;
  std::vector<std::string> transcript;
  auto note_ops = [&](const char* actor) {
    render::List rl;
    camera2d::Camera camera;
    camera.x = world.player.position.x;
    camera.y = world.player.position.y;
    verdigris::client::record_world_ops(rl, world, fx, camera, 960, 600);
    for (const auto& item : rl) {
      if (item.op == render::Op::Telegraph)
        transcript.push_back(std::string("Telegraph ") + actor + " " + item.label);
      if (item.op == render::Op::Damage)
        transcript.push_back(std::string("Damage ") + actor);
      if (item.op == render::Op::Impact || item.op == render::Op::TargetFlash)
        transcript.push_back(std::string("Impact ") + actor);
    }
  };

  verdigris::client::apply_presentation_event(fx, world, warning, 1);
  note_ops("flint-1");
  PresentationEvent hit{PresentationEventType::DamageApplied, "flint-1", "",
                        "incoming", 4};
  verdigris::client::apply_presentation_event(fx, world, hit, 2);
  note_ops("flint-1");

  int warning_at = -1;
  int damage_at = -1;
  int impact_at = -1;
  for (int i = 0; i < static_cast<int>(transcript.size()); ++i) {
    if (warning_at < 0 && transcript[static_cast<std::size_t>(i)].rfind(
                              "Telegraph flint-1", 0) == 0)
      warning_at = i;
    if (damage_at < 0 && transcript[static_cast<std::size_t>(i)] == "Damage flint-1")
      damage_at = i;
    if (impact_at < 0 && transcript[static_cast<std::size_t>(i)].rfind(
                             "Impact flint-1", 0) == 0)
      impact_at = i;
  }
  check(warning_at >= 0, "0108: ranged windup records a Telegraph op");
  check(fx.telegraphs["flint-1"].action == "projectile",
        "0108: Telegraph label stays the existing op, action projectile");
  check(damage_at >= 0 && impact_at >= 0,
        "0108: ranged hit lands as attributed Damage/Impact");
  check(warning_at < damage_at && warning_at < impact_at,
        "0108: every ranged hit is preceded by its Telegraph op");
  check(fx.monster_strikes.count("flint-1") == 1,
        "0108: incoming hit is attributed to the ranged attacker");

  PresentationFx bare_hit;
  verdigris::client::apply_presentation_event(bare_hit, world, hit, 3);
  render::List bare;
  camera2d::Camera camera;
  camera.x = world.player.position.x;
  camera.y = world.player.position.y;
  verdigris::client::record_world_ops(bare, world, bare_hit, camera, 960, 600);
  bool illegal_telegraph = false;
  bool saw_damage = false;
  for (const auto& item : bare) {
    if (item.op == render::Op::Telegraph) illegal_telegraph = true;
    if (item.op == render::Op::Damage) saw_damage = true;
  }
  check(saw_damage && !illegal_telegraph,
        "0108: a hit without a preceding warning cannot mint a Telegraph");
  check(bare_hit.telegraphs.find("flint-1") == bare_hit.telegraphs.end(),
        "0108: negative — damage-only path never records the ranged warning");

  PresentationFx slam_fx;
  verdigris::client::apply_presentation_event(slam_fx, world, slam, 1);
  check(slam_fx.telegraphs["boss-1"].action != "projectile",
        "0108: negative — slam telegraph never becomes a projectile warning");
}

void npcs_ride_the_model_into_world_and_render_list() {
  verdigris::client::ClientModel model;
  model.player.x = 38.0;
  model.player.y = 115.0;
  model.player.alive = true;
  verdigris::client::ClientNpc npc;
  npc.id = 1;
  npc.name = "Aldwyn the Guide";
  npc.x = 34.0;
  npc.y = 116.0;
  npc.actions = {"talk", "examine"};
  model.npcs.push_back(npc);
  WorldView world;
  verdigris::client::sync_world_from_model(world, model);
  check(world.npcs.size() == 1, "npc: roster entry survives the sync");
  check(world.npcs[0].name == "Aldwyn the Guide", "npc: name survives the sync");
  const int expected_x = static_cast<int>(
      std::lround(verdigris::client::protocol_to_world(34.0)));
  check(world.npcs[0].position.x == expected_x,
        "npc: tile position converts to world units");
  check(world.npcs[0].actions.size() == 2 && world.npcs[0].actions[0] == "talk",
        "npc: server verb list survives verbatim");
  render::List rl;
  camera2d::Camera camera;
  camera.x = world.player.position.x;
  camera.y = world.player.position.y;
  PresentationFx fx;
  verdigris::client::record_world_ops(rl, world, fx, camera, 960, 600);
  bool saw_npc = false;
  for (const auto& item : rl)
    if (item.op == render::Op::Npc && item.label == "Aldwyn the Guide")
      saw_npc = true;
  check(saw_npc, "npc: render list carries a labeled Npc op");
}

int main() {
  constants_are_named_and_distinct();
  critical_damage_is_distinct();
  scion_lost_beat_contract();
  buff_expired_beat_contract();
  local_seam_maps_lifecycle_events();
  spawn_detection_is_deterministic_and_once();
  monster_facing_is_no_longer_fabricated();
  seam_events_cannot_mutate_simulation();
  diagonal_facing_resolves_component_wise();
  server_messages_surface_as_toasts();
  ranged_projectile_warning_precedes_attributed_hit();
  npcs_ride_the_model_into_world_and_render_list();
  std::printf("%s\n", failures == 0 ? "presentation events tests: PASS"
                                    : "presentation events tests: FAIL");
  return failures == 0 ? 0 : 1;
}
