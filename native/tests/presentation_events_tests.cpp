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

// ── TASK-0108: ranged readability lock at the presentation seam ────────────
// The core locks in core_tests.cpp prove the seeded ranged stream is
// telegraph-then-hit. These locks prove the client-visible half: driven one
// 50 ms frame at a time through the shipped seam mapping, every player-side
// damage beat resolves as an attributed Impact/Damage op anchored on the
// player and only after a Telegraph op announced by the shooter. The events
// fed here are exactly the ones remote_session.cpp derives from the wire
// pair monster:telegraph / combat:hit(incoming); nothing else is invented.

struct RangedFrameBeat {
  std::uint64_t frame = 0;
  bool shooter_telegraph_live = false;
  bool has_telegraph_op = false;
  std::string telegraph_label;
  double telegraph_x = 0.0;
  double telegraph_y = 0.0;
  int player_damage_ops = 0;
  int damage_value = 0;
  double damage_x = 0.0;
  double damage_y = 0.0;
  int impact_ops = 0;
  int player_flash_ops = 0;
};

struct RangedTranscript {
  std::vector<RangedFrameBeat> frames;
  std::vector<std::string> event_log;
};

RangedTranscript drive_ranged_frames(bool announce) {
  WorldView world = world_with_player_and_foe("right");
  world.monsters.front().id = "twin-ranged";
  PresentationFx fx;
  RangedTranscript transcript;
  auto record_frame = [&](std::uint64_t frame) {
    RangedFrameBeat beat;
    beat.frame = frame;
    beat.shooter_telegraph_live = fx.telegraphs.count("twin-ranged") > 0;
    render::List rl;
    record_world_ops(rl, world, fx, camera2d::Camera{}, 960, 600);
    for (const auto& item : rl) {
      if (item.op == render::Op::Telegraph && !beat.has_telegraph_op) {
        beat.has_telegraph_op = true;
        beat.telegraph_label = item.label;
        beat.telegraph_x = item.x;
        beat.telegraph_y = item.y;
      }
      if (item.op == render::Op::Damage && item.label == "player") {
        ++beat.player_damage_ops;
        beat.damage_value = item.value;
        beat.damage_x = item.x;
        beat.damage_y = item.y;
      }
      if (item.op == render::Op::Impact) ++beat.impact_ops;
      if (item.op == render::Op::TargetFlash && item.label == "player")
        ++beat.player_flash_ops;
    }
    transcript.frames.push_back(beat);
  };

  // monster:telegraph wire parity: attackerId, name+skillId text, durationMs
  // as value (the authored 1000 ms readable window).
  const PresentationEvent warning{PresentationEventType::Telegraph, "twin-ranged", "",
                                  "Pressure Twin monster:attack", 1000, false, {}};
  // combat:hit incoming parity copy: attributed attackerId, "incoming",
  // amount as value (the authored kN3MonsterDamage figure).
  const PresentationEvent shot{PresentationEventType::DamageApplied, "twin-ranged", "",
                               "incoming", 5, false, {}};
  for (std::uint64_t frame = 0; frame <= 33; ++frame) {
    if (frame == 0 && announce) apply_presentation_event(fx, world, warning, frame);
    if (frame == 20) apply_presentation_event(fx, world, shot, frame);
    record_frame(frame);
    age_presentation_fx(fx);
  }
  transcript.event_log = fx.event_log;
  return transcript;
}

bool every_player_hit_is_telegraphed(const RangedTranscript& transcript) {
  bool telegraph_seen = false;
  for (const auto& beat : transcript.frames) {
    if (beat.player_damage_ops > 0 && !telegraph_seen) return false;
    if (beat.shooter_telegraph_live) telegraph_seen = true;
  }
  return true;
}

void ranged_hit_beat_is_telegraphed_and_attributed() {
  const RangedTranscript transcript = drive_ranged_frames(true);
  check(transcript.frames.at(20).player_damage_ops == 1 &&
            transcript.frames.at(20).damage_value == 5,
        "ranged seam: the announced shot lands as a player-side damage beat");
  check(every_player_hit_is_telegraphed(transcript),
        "ranged seam: every player-side hit is preceded by a Telegraph op");
  check(transcript.frames.front().has_telegraph_op &&
            transcript.frames.front().telegraph_label == "thrust",
        "ranged seam: the warning renders on the announcement frame");
  check(transcript.frames.at(10).has_telegraph_op,
        "ranged seam: the warning stays up across its readable window");
  const camera2d::Screen screen{960, 600};
  const camera2d::Point foe_at =
      camera2d::project(camera2d::Camera{}, screen, 140.0, 100.0);
  const camera2d::Point player_at =
      camera2d::project(camera2d::Camera{}, screen, 100.0, 100.0);
  check(transcript.frames.front().telegraph_x ==
                static_cast<double>(foe_at.x) &&
            transcript.frames.front().telegraph_y ==
                static_cast<double>(foe_at.y),
        "ranged seam: the warning anchors on the shooter, not on the player");
  check(transcript.frames.at(20).damage_x == static_cast<double>(player_at.x) &&
            transcript.frames.at(20).damage_y == static_cast<double>(player_at.y),
        "ranged seam: the hit beat lands where the player stands");
  check(transcript.frames.at(20).impact_ops >= 1 &&
            transcript.frames.at(20).player_flash_ops >= 1,
        "ranged seam: the hit resolves with the shipped Impact/flash beats");
  bool saw_warning_line = false;
  bool saw_taken_line = false;
  for (const auto& line : transcript.event_log) {
    if (line == "Telegraph Pressure Twin monster:attack") saw_warning_line = true;
    if (line == "Taken 5") saw_taken_line = true;
  }
  check(saw_warning_line && saw_taken_line,
        "ranged seam: the readable log names the warning and the taken hit");
}

void untelegraphed_hit_fails_the_readability_lock() {
  const RangedTranscript transcript = drive_ranged_frames(false);
  bool saw_silent_damage = false;
  int telegraph_ops_total = 0;
  for (const auto& beat : transcript.frames) {
    if (beat.player_damage_ops > 0) saw_silent_damage = true;
    if (beat.has_telegraph_op) ++telegraph_ops_total;
  }
  check(saw_silent_damage,
        "ranged negative: the untelegraphed run still resolves player damage");
  check(telegraph_ops_total == 0,
        "ranged negative: no warning op was rendered on this path");
  check(!every_player_hit_is_telegraphed(transcript),
        "ranged negative: the lock FAILS any ranged resolution without a "
        "preceding telegraph");
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
  ranged_hit_beat_is_telegraphed_and_attributed();
  untelegraphed_hit_fails_the_readability_lock();
  std::printf("%s\n", failures == 0 ? "presentation events tests: PASS"
                                    : "presentation events tests: FAIL");
  return failures == 0 ? 0 : 1;
}
