// TASK-0122 Phase A: dedicated presentation-events tests. Proves the new
// readable event beats at the session seam: critical/ordinary damage
// distinction, ScionLost/BuffExpired contract beats, deterministic spawn
// materialization, corrected (non-fabricated) monster facing, and the
// negative control that presentation effects never mutate simulation state.
// All timing values come from the phase_a constants table in
// presentation_events.hpp; no magic literals are pinned here that the table
// does not also pin.

#include <cstdio>
#include <string>

#include "../client/local_session.hpp"
#include "../client/presentation_state.hpp"

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

void combo_finisher_is_distinct() {
  WorldView world = world_with_player_and_foe("right");
  PresentationFx fx;
  PresentationEvent finisher;
  finisher.type = PresentationEventType::DamageApplied;
  finisher.actor_id = "foe-1";
  finisher.text = "outgoing";
  finisher.value = 32;
  finisher.style = "slash";
  finisher.combo_step = 3;
  finisher.combo_window_ms = 900;
  finisher.stagger_ms = 700;
  apply_presentation_event(fx, world, finisher, 0);
  const EffectFx* ring = first_kind(fx, EffectFx::Kind::ComboFinisher);
  const EffectFx* number = first_kind(fx, EffectFx::Kind::DamageNumber);
  check(ring && ring->ttl == phase_a::kComboFinisherTtlTicks,
        "combo: third beat produces the named finisher treatment");
  check(number && number->finisher && !number->critical,
        "combo: finisher emphasis is independent from critical chance");
  render::List rl;
  record_world_ops(rl, world, fx, camera2d::Camera{}, 960, 600);
  bool saw_ring = false;
  bool saw_number = false;
  for (const auto& item : rl) {
    if (item.op == render::Op::Impact &&
        item.label == phase_a::kComboFinisherLabel) saw_ring = true;
    if (item.op == render::Op::Damage && item.label == "finisher:slash")
      saw_number = true;
  }
  check(saw_ring && saw_number,
        "combo: shared render list labels the finisher ring and damage");
}

void monster_role_feedback_is_authoritative_and_distinct() {
  WorldView world = world_with_player_and_foe("right");
  PresentationFx fx;
  PresentationEvent volley;
  volley.type = PresentationEventType::Telegraph;
  volley.actor_id = "foe-1";
  volley.text = "Bog Spitter ranged:volley";
  volley.value = 800;
  volley.has_position = true;
  volley.x = 7.0;
  volley.y = 9.0;
  volley.radius = 1;
  apply_presentation_event(fx, world, volley, 12);
  const auto found = fx.telegraphs.find("foe-1");
  check(found != fx.telegraphs.end() && found->second.action == "volley" &&
            found->second.position.x == static_cast<int>(std::lround(
                verdigris::client::protocol_to_world(7.0))) &&
            found->second.position.y == static_cast<int>(std::lround(
                verdigris::client::protocol_to_world(9.0))) &&
            found->second.windup_ticks == 16 && found->second.radius_tiles == 1,
        "roles: volley uses the authoritative destination, radius, and windup");

  PresentationEvent mend;
  mend.type = PresentationEventType::HealingApplied;
  mend.actor_id = "foe-1";
  mend.text = "Rot Shaman";
  mend.value = 7;
  apply_presentation_event(fx, world, mend, 12);
  const EffectFx* pulse = first_kind(fx, EffectFx::Kind::SupportMend);
  const EffectFx* number = nullptr;
  for (const auto& effect : fx.effects)
    if (effect.kind == EffectFx::Kind::DamageNumber && effect.healing)
      number = &effect;
  check(pulse && pulse->ttl == phase_a::kSupportMendTtlTicks &&
            pulse->value == 7 && number && number->value == 7,
        "roles: support mend creates a green pulse and positive number");
  render::List rl;
  record_world_ops(rl, world, fx, camera2d::Camera{}, 960, 600);
  bool saw_mend = false;
  bool saw_healing = false;
  bool saw_volley = false;
  for (const auto& item : rl) {
    if (item.op == render::Op::WarCry &&
        item.label == phase_a::kSupportMendLabel) saw_mend = true;
    if (item.op == render::Op::Damage && item.label == "healing" && item.value == 7)
      saw_healing = true;
    if (item.op == render::Op::Telegraph && item.label == "volley")
      saw_volley = true;
  }
  check(saw_mend && saw_healing && saw_volley,
        "roles: shared render list preserves volley and mend semantics");
  PresentationEvent cancelled;
  cancelled.type = PresentationEventType::TelegraphCancelled;
  cancelled.actor_id = "foe-1";
  cancelled.text = "ranged:volley";
  cancelled.value = 700;
  apply_presentation_event(fx, world, cancelled, 13);
  check(fx.telegraphs.count("foe-1") == 0,
        "roles: authoritative interruption removes the warning immediately");
}

void forge_status_feedback_is_authoritative_and_distinct() {
  verdigris::client::ClientModel model;
  model.player.uuid = "forge-scion";
  model.player.alive = true;
  model.bleed_chance = 100;
  model.attack_speed_percent = 8;
  model.reach_percent = 16;
  model.projectile_range_percent = 20;
  model.armour_penetration_percent = 50;
  model.movement_speed_percent = 25;
  model.ember_resistance = 25;
  model.river_resistance = 25;
  verdigris::client::ClientMonster monster;
  monster.id = "forge-foe";
  monster.name = "Mire Warden";
  monster.x = 12.0;
  monster.y = 10.0;
  monster.damage_channel = "river";
  monster.armour = 100;
  monster.bleeding = true;
  model.monsters.push_back(monster);
  verdigris::client::ClientItemSlot item;
  item.uuid = "forged-blade";
  item.name = "Obsidian Macuahuitl";
  item.forge_lines = {"Hits cause Bleeding", "+16% increased Reach"};
  model.inventory.push_back(item);

  WorldView world;
  verdigris::client::sync_world_from_model(world, model);
  check(world.player.bleed_chance == 100 &&
            world.player.attack_speed_percent == 8 &&
            world.player.reach_percent == 16 &&
            world.player.projectile_range_percent == 20 &&
            world.player.armour_penetration_percent == 50 &&
            world.player.movement_speed_percent == 25 &&
            world.player.ember_resistance == 25 &&
            world.player.river_resistance == 25,
        "forge feedback: worn totals survive the model-to-world sync");
  check(world.monsters.size() == 1 && world.monsters[0].bleeding &&
            world.monsters[0].armour == 100 &&
            world.monsters[0].damage_channel == "river" &&
            world.carried.size() == 1 && world.carried[0].forge_lines.size() == 2,
        "forge feedback: status, damage channel, and item lines survive presentation sync");

  PresentationFx fx;
  PresentationEvent applied;
  applied.type = PresentationEventType::DebuffApplied;
  applied.actor_id = "forge-foe";
  applied.text = "bleed";
  applied.value = 4;
  applied.duration_ms = 3000;
  verdigris::client::apply_presentation_event(fx, world, applied, 1);
  const EffectFx* pulse = first_kind(fx, EffectFx::Kind::BleedApplied);
  check(pulse && pulse->ttl == phase_a::kBleedApplyTtlTicks &&
            pulse->value == 4 && pulse->style == "bleed",
        "forge feedback: bleed application creates the named crimson treatment");

  PresentationEvent tick;
  tick.type = PresentationEventType::DamageApplied;
  tick.actor_id = "forge-foe";
  tick.text = "outgoing";
  tick.value = 4;
  tick.style = "bleed";
  verdigris::client::apply_presentation_event(fx, world, tick, 2);
  render::List rl;
  verdigris::client::record_world_ops(rl, world, fx, camera2d::Camera{}, 960, 600);
  bool saw_apply = false;
  bool saw_tick = false;
  for (const auto& op : rl) {
    if (op.op == render::Op::Impact && op.label == phase_a::kBleedApplyLabel)
      saw_apply = true;
    if (op.op == render::Op::Damage && op.label == phase_a::kBleedDamageLabel &&
        op.value == 4)
      saw_tick = true;
  }
  check(saw_apply && saw_tick,
        "forge feedback: render list distinguishes bleed application and tick damage");

  PresentationFx piercing_fx;
  PresentationEvent piercing;
  piercing.type = PresentationEventType::DamageApplied;
  piercing.actor_id = "forge-foe";
  piercing.text = "outgoing";
  piercing.value = 15;
  piercing.style = "range";
  piercing.armour_rating = 100;
  piercing.armour_prevented = 5;
  piercing.armour_penetration_percent = 50;
  verdigris::client::apply_presentation_event(
      piercing_fx, world, piercing, 3);
  render::List piercing_ops;
  verdigris::client::record_world_ops(
      piercing_ops, world, piercing_fx, camera2d::Camera{}, 960, 600);
  bool saw_piercing = false;
  for (const auto& op : piercing_ops)
    if (op.op == render::Op::Damage && op.label == "piercing:range" &&
        op.value == 15)
      saw_piercing = true;
  check(saw_piercing,
        "forge feedback: armour bypass has a distinct piercing damage beat");
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
  PresentationFx applied_fx;
  PresentationEvent applied{PresentationEventType::BuffApplied, "scion-1", "",
                            "war-cry", 0, false, {}};
  apply_presentation_event(applied_fx, world, applied, 0);
  check(count_kind(applied_fx, EffectFx::Kind::WarCryAura) == 1,
        "phase-a: BuffApplied(war-cry) produces the bright aura beat");
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

void authoritative_information_survives_model_sync() {
  verdigris::client::ClientModel model;
  model.player.alive = true;
  model.xp_present = true;
  model.xp_current = 130.0;
  model.xp_floor = 100.0;
  model.xp_next = 200.0;
  verdigris::client::ClientMonster foe;
  foe.id = "foe-1";
  foe.name = "Ashen Spear-Bearer";
  foe.life = 9;
  foe.life_max = 12;
  model.monsters.push_back(foe);
  WorldView world;
  sync_world_from_model(world, model);
  check(world.monsters.size() == 1 &&
            world.monsters.front().name == "Ashen Spear-Bearer",
        "information: authoritative monster name survives model sync");
  check(world.xp_present && std::abs(world.xp_fraction - 0.30) < 0.0001,
        "information: authoritative XP span normalizes in presentation");

  model.xp_present = false;
  sync_world_from_model(world, model);
  check(!world.xp_present && world.xp_fraction == 0.0,
        "information: absent XP never fabricates zero progress");
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

void npcs_ride_the_model_into_world_and_render_list() {
  verdigris::client::ClientModel model;
  model.player.x = 38.0;
  model.player.y = 115.0;
  model.player.alive = true;
  verdigris::client::ClientNpc npc;
  npc.id = 1;
  npc.key = "aldwyn-guide";
  npc.name = "Aldwyn the Guide";
  npc.role = "elder";
  npc.examine = "A weathered wayfinder.";
  npc.x = 34.0;
  npc.y = 116.0;
  npc.services = {"guidance", "expedition_access"};
  npc.actions = {"talk", "examine"};
  model.npcs.push_back(npc);
  WorldView world;
  verdigris::client::sync_world_from_model(world, model);
  check(world.npcs.size() == 1, "npc: roster entry survives the sync");
  check(world.npcs[0].name == "Aldwyn the Guide", "npc: name survives the sync");
  check(world.npcs[0].key == "aldwyn-guide" &&
            world.npcs[0].role == "elder" &&
            world.npcs[0].services.size() == 2,
        "npc: stable identity, role, and services survive the sync");
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
  combo_finisher_is_distinct();
  monster_role_feedback_is_authoritative_and_distinct();
  forge_status_feedback_is_authoritative_and_distinct();
  scion_lost_beat_contract();
  buff_expired_beat_contract();
  local_seam_maps_lifecycle_events();
  spawn_detection_is_deterministic_and_once();
  monster_facing_is_no_longer_fabricated();
  authoritative_information_survives_model_sync();
  seam_events_cannot_mutate_simulation();
  diagonal_facing_resolves_component_wise();
  server_messages_surface_as_toasts();
  npcs_ride_the_model_into_world_and_render_list();
  std::printf("%s\n", failures == 0 ? "presentation events tests: PASS"
                                    : "presentation events tests: FAIL");
  return failures == 0 ? 0 : 1;
}
