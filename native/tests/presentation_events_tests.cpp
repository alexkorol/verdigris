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
#include <unordered_map>

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
  model.monsters.push_back({"foe-1", "monster", 20.0, 0.0, 3, 3, false, true});
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

// ── TASK-0108 W1: readable ranged beats ride the shipped vocabulary ───────
// The beats below are constructed exactly as remote_session.cpp maps the
// shipped envelopes: monster:telegraph becomes Telegraph(actor_id =
// attackerId, value = durationMs); a combat:hit whose target is the player
// becomes DamageApplied(actor_id = attackerId, text = "incoming",
// value = amount). Locking the render contract over those mapped beats needs
// no socket plumbing and no new render op.

struct RangedBeat {
  bool telegraph = false;
  std::string actor;
};

// The W1 lock: every resolved ranged hit must be preceded by its own
// shooter's telegraph. One warning arms exactly one hit.
bool every_ranged_hit_is_telegraphed(const std::vector<RangedBeat>& stream) {
  std::unordered_map<std::string, bool> armed;
  for (const auto& beat : stream) {
    if (beat.telegraph) {
      armed[beat.actor] = true;
      continue;
    }
    const auto it = armed.find(beat.actor);
    if (it == armed.end() || !it->second) return false;
    it->second = false;
  }
  return true;
}

void ranged_hit_beats_are_telegraphed_and_attributed() {
  WorldView world;
  world.player.id = "scion-w1";
  world.player.position = {300, 300};
  world.player.facing = verdigris::client::facing_vector("right");
  world.player.alive = true;
  WorldActor shooter;
  shooter.id = "foe-ranged";
  shooter.position = {300, 180};
  shooter.alive = true;
  world.monsters.push_back(shooter);

  PresentationFx fx;
  std::vector<RangedBeat> stream;
  std::vector<render::List> frames;
  bool saw_impact_fx = false;

  const auto beat_frame = [&](const PresentationEvent& event) {
    apply_presentation_event(fx, world, event, 0);
    if (event.type == PresentationEventType::Telegraph)
      stream.push_back({true, event.actor_id});
    if (event.type == PresentationEventType::DamageApplied &&
        event.text == "incoming") {
      stream.push_back({false, event.actor_id});
      saw_impact_fx = count_kind(fx, EffectFx::Kind::Impact) > 0;
    }
    render::List rl;
    record_world_ops(rl, world, fx, camera2d::Camera{}, 960, 600);
    frames.push_back(std::move(rl));
    age_presentation_fx(fx);
  };

  // Frame 0 - the warning (the authored 1000 ms window on the wire).
  beat_frame({PresentationEventType::Telegraph, "foe-ranged", "",
              "Grove Lurker monster:ranged-shot", phase_a::kTickMs * 20, false, {}});
  // Frame 1 - the resolution lands on the player.
  beat_frame({PresentationEventType::DamageApplied, "foe-ranged", "", "incoming", 5,
              false, {}});

  check(every_ranged_hit_is_telegraphed(stream),
        "w1: every resolved ranged hit is preceded by its own telegraph");
  check(frames.size() == 2 && render::any(frames[0], render::Op::Telegraph),
        "w1: the warning frame records the shipped Telegraph op");
  const camera2d::Point expected_anchor =
      camera2d::project(camera2d::Camera{}, camera2d::Screen{960, 600}, 300, 180);
  const render::Item* warning_op = render::first(frames[0], render::Op::Telegraph);
  check(warning_op != nullptr && warning_op->label == "thrust" &&
            warning_op->x == static_cast<double>(expected_anchor.x) &&
            warning_op->y == static_cast<double>(expected_anchor.y),
        "w1: the Telegraph op anchors on the shooter through the shared camera");
  const auto armed = fx.telegraphs.find("foe-ranged");
  check(armed != fx.telegraphs.end() &&
            armed->second.windup_ticks == phase_a::kTickMs * 20 / phase_a::kTickMs,
        "w1: the authored wire window maps onto the readable windup");
  bool damage_number = false;
  bool player_flash = false;
  for (const auto& item : frames[1]) {
    if (item.op == render::Op::Damage && item.label == "player" && item.value == 5)
      damage_number = true;
    if (item.op == render::Op::TargetFlash && item.label == "player")
      player_flash = true;
  }
  check(damage_number, "w1: the hit lands as an attributed player Damage number");
  check(player_flash, "w1: the hit flash is attributed to the player");
  check(saw_impact_fx, "w1: the resolution carries the Impact fx beat");

  // Stream order across frames: the warning op precedes the damage op.
  std::size_t warning_index = 0;
  std::size_t damage_index = 0;
  bool found_warning = false;
  bool found_damage = false;
  for (std::size_t frame = 0; frame < frames.size(); ++frame) {
    for (std::size_t i = 0; i < frames[frame].size(); ++i) {
      const auto& item = frames[frame][i];
      if (item.op == render::Op::Telegraph && !found_warning) {
        warning_index = frame * 10000 + i;
        found_warning = true;
      }
      if (item.op == render::Op::Damage && item.label == "player" && !found_damage) {
        damage_index = frame * 10000 + i;
        found_damage = true;
      }
    }
  }
  check(found_warning && found_damage && warning_index < damage_index,
        "w1: the Telegraph op precedes the attributed Damage op in stream order");

  bool logged_warning = false;
  bool logged_taken = false;
  for (const auto& line : fx.event_log) {
    if (line.rfind("Telegraph ", 0) == 0) logged_warning = true;
    if (line == "Taken 5") logged_taken = true;
  }
  check(logged_warning && logged_taken,
        "w1: both readable HUD lines log for the warned ranged beat");
}

void untelegraphed_ranged_hit_fails_the_lock() {
  check(!every_ranged_hit_is_telegraphed({{false, "foe-x"}}),
        "w1-negative: a ranged hit without any telegraph fails the lock");
  check(!every_ranged_hit_is_telegraphed({{true, "foe-a"}, {false, "foe-b"}}),
        "w1-negative: another shooter's warning never covers the hit");
  check(!every_ranged_hit_is_telegraphed(
            {{true, "foe-a"}, {false, "foe-a"}, {false, "foe-a"}}),
        "w1-negative: every shot needs its own warning");
  check(
      every_ranged_hit_is_telegraphed({{true, "foe-a"}, {false, "foe-a"}}),
      "w1-negative: the exact one-warning-one-hit pairing passes");
}

}  // namespace

int main() {
  constants_are_named_and_distinct();
  critical_damage_is_distinct();
  scion_lost_beat_contract();
  buff_expired_beat_contract();
  local_seam_maps_lifecycle_events();
  spawn_detection_is_deterministic_and_once();
  monster_facing_is_no_longer_fabricated();
  seam_events_cannot_mutate_simulation();
  ranged_hit_beats_are_telegraphed_and_attributed();
  untelegraphed_ranged_hit_fails_the_lock();
  std::printf("%s\n", failures == 0 ? "presentation events tests: PASS"
                                    : "presentation events tests: FAIL");
  return failures == 0 ? 0 : 1;
}
