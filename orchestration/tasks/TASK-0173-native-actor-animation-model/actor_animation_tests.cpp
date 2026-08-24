// actor_animation_tests.cpp — TASK-0173 acceptance tests.
//
// Build normally for the acceptance run (all checks must pass). Build with
// /DNEGATIVE_CONTROL to compile the deliberate negative-control check, which
// asserts the inverted expectation and therefore fails with exit code 1 —
// proving this harness actually detects a zero-length-attack regression.

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "actor_animation.hpp"

using namespace actor_animation;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

const Config kConfig = default_config();

std::size_t vidx(AttackVariant v) { return static_cast<std::size_t>(v); }

State windup_state(AttackVariant variant, Facing facing, std::uint32_t elapsed) {
  State s;
  s.id = StateId::Windup;
  s.variant = variant;
  s.facing = facing;
  s.elapsed_ms = elapsed;
  return s;
}

State attack_state(AttackVariant variant, std::uint32_t elapsed) {
  State s;
  s.id = StateId::Attack;
  s.variant = variant;
  s.elapsed_ms = elapsed;
  return s;
}

State recovery_state(AttackVariant variant, std::uint32_t elapsed) {
  State s;
  s.id = StateId::Recovery;
  s.variant = variant;
  s.elapsed_ms = elapsed;
  return s;
}

void test_transition_legality() {
  const State idle;
  check(valid(idle, kConfig), "default idle state is valid");

  const Decision move = start_move(idle, 700);
  check(move.status == Status::Ok && move.next.id == StateId::Locomotion &&
            move.next.speed_permille == 700,
        "idle + start_move -> locomotion at requested speed");

  check(stop(move.next).status == Status::Ok &&
            stop(move.next).next.id == StateId::Idle,
        "locomotion + stop -> idle");
  check(stop(idle).status == Status::Ok && stop(idle).next == idle,
        "idle + stop is a no-op");

  const State busy = attack_state(AttackVariant::Swing, 10);
  check(start_move(busy, 500).status == Status::InvalidState,
        "start_move refused during active attack");
  check(stop(busy).status == Status::InvalidState,
        "stop refused during active attack");
  check(start_move(windup_state(AttackVariant::Swing, Facing::South, 10), 400)
              .status == Status::InvalidState,
        "start_move refused during windup");
  check(stop(windup_state(AttackVariant::Swing, Facing::South, 10)).status ==
            Status::InvalidState,
        "stop refused during windup");

  const Decision swing = attack(idle, AttackVariant::Swing, kConfig);
  check(swing.status == Status::Ok && swing.next.id == StateId::Windup,
        "idle + attack -> windup");
  check(attack(busy, AttackVariant::Thrust, kConfig).status ==
            Status::InvalidState,
        "no re-entry during an active attack");
  check(start_move(start_move(idle, 900).next, 300).status == Status::Ok,
        "locomotion speed re-blend allowed");

  const Decision bad_speed = start_move(idle, 1001);
  check(bad_speed.status == Status::InvalidArgument && bad_speed.next == idle,
        "speed above 1000 permille rejected, state untouched");
  check(start_move(idle, 0).status == Status::InvalidArgument,
        "zero speed is stop(), not a blend; rejected");

  check(attack(recovery_state(AttackVariant::Swing, 10), AttackVariant::Thrust,
               kConfig)
              .status == Status::InvalidState,
        "attack request early in recovery refused (outside chain window)");
}

void test_timing_and_phase_wrap() {
  const Config cfg = default_config();
  const std::uint32_t windup = cfg.windup_ms[vidx(AttackVariant::Swing)];
  const std::uint32_t atk = cfg.attack_ms[vidx(AttackVariant::Swing)];
  const std::uint32_t recovery = cfg.recovery_ms[vidx(AttackVariant::Swing)];

  State s = attack(State{}, AttackVariant::Swing, cfg).next;
  check(s.id == StateId::Windup && s.elapsed_ms == 0,
        "swing starts in windup at zero");

  s = tick(s, static_cast<std::int64_t>(windup) - 1, cfg);
  check(s.id == StateId::Windup && s.elapsed_ms == windup - 1,
        "windup holds one ms before expiry");
  s = tick(s, 1, cfg);
  check(s.id == StateId::Attack && s.elapsed_ms == 0,
        "windup expiry rolls into attack with exact carry");

  // One giant dt spanning every boundary must land in idle with remainder.
  const std::uint64_t full = static_cast<std::uint64_t>(windup) +
                             static_cast<std::uint64_t>(atk) +
                             static_cast<std::uint64_t>(recovery);
  State jumped = attack(State{}, AttackVariant::Swing, cfg).next;
  jumped = tick(jumped, static_cast<std::int64_t>(full + 4), cfg);
  check(jumped.id == StateId::Idle && jumped.elapsed_ms == 4,
        "overshoot across windup+attack+recovery lands in idle carrying remainder");

  State s2;
  s2 = attack(s2, AttackVariant::Slam, cfg).next;
  const Pose mid =
      pose(tick(s2, cfg.windup_ms[vidx(AttackVariant::Slam)] / 2, cfg), cfg);
  check(mid.phase > 0.0f && mid.phase < 1.0f,
        "mid-windup phase strictly inside (0,1)");
  const State expired_slam =
      tick(s2, static_cast<std::int64_t>(cfg.windup_ms[vidx(AttackVariant::Slam)] +
                                         cfg.attack_ms[vidx(AttackVariant::Slam)]),
           cfg);
  check(expired_slam.id == StateId::Recovery,
        "slam windup+attack expiry reaches recovery");

  const State untimed = tick(State{}, 123456, cfg);
  check(untimed.id == StateId::Idle && untimed.elapsed_ms == 123456,
        "untimed idle accumulates time without transitioning");
  check(tick(untimed, 0, cfg) == untimed && tick(untimed, -7, cfg) == untimed,
        "zero/negative dt are no-ops");
}

void test_facing_changes() {
  const Decision turn = face(State{}, Facing::NorthEast);
  check(turn.status == Status::Ok && turn.next.facing == Facing::NorthEast,
        "face() retargets while idle");

  check(rotated(Facing::NorthEast, 3) == Facing::South,
        "clockwise rotation wraps across the south seam");
  check(rotated(Facing::South, -3) == Facing::NorthEast,
        "counterclockwise rotation wraps the other way");
  check(rotated(Facing::West, 8) == Facing::West, "full rotation is identity");
  check(rotated(Facing::SouthEast, 1) == Facing::South,
        "single step advances one compass stop");

  const State moving = start_move(face(State{}, Facing::West).next, 600).next;
  check(moving.facing == Facing::West && moving.id == StateId::Locomotion,
        "facing survives the move transition");

  const Decision locked =
      face(windup_state(AttackVariant::Cast, Facing::North, 5), Facing::South);
  check(locked.status == Status::InvalidState &&
            locked.next.facing == Facing::North,
        "facing locked during committed windup");

  const State dead = die(turn.next).next;
  check(face(dead, Facing::East).status == Status::Terminal,
        "facing locked forever after death");
  const Pose dead_pose = pose(dead, kConfig);
  check(dead_pose.facing_degrees == 225.0f,
        "dead pose keeps the facing captured at death");
}

void test_interruption_matrix() {
  const Config cfg = default_config();
  const std::uint32_t windup_swing = cfg.windup_ms[vidx(AttackVariant::Swing)];

  const State early_windup =
      windup_state(AttackVariant::Swing, Facing::South, windup_swing / 4);
  const Decision early = take_hit(early_windup, cfg);
  check(early.status == Status::Ok && early.next.id == StateId::Hit,
        "hit interrupts windup below threshold");

  const State late_windup =
      windup_state(AttackVariant::Swing, Facing::South, (windup_swing * 3) / 4);
  const Decision late = take_hit(late_windup, cfg);
  check(late.status == Status::InvalidState && late.next == late_windup,
        "hit does not interrupt windup at/above threshold");

  check(take_hit(attack_state(AttackVariant::Thrust, 20), cfg).status ==
            Status::InvalidState,
        "active attacks absorb hits");
  check(take_hit(State{}, cfg).status == Status::Ok, "idle accepts hit");
  check(take_hit(start_move(State{}, 500).next, cfg).status == Status::Ok,
        "locomotion accepts hit");
  check(
      take_hit(recovery_state(AttackVariant::Slam, 30), cfg).status == Status::Ok,
      "recovery accepts hit");
  check(take_hit(take_hit(State{}, cfg).next, cfg).status ==
            Status::InvalidState,
        "hit does not stack on hit");

  const Decision chain_in_window = attack(
      recovery_state(AttackVariant::Swing,
                     cfg.recovery_ms[0] - cfg.recovery_chain_window_ms),
      AttackVariant::Swing, cfg);
  check(chain_in_window.status == Status::Ok &&
            chain_in_window.next.id == StateId::Windup,
        "attack chains from inside the recovery window");
  check(chain_in_window.next.variant == AttackVariant::Swing,
        "chained windup carries the new variant");
}

void test_death_override_irreversibility() {
  const Config cfg = default_config();
  const State mid_attack = attack_state(AttackVariant::Slam, 200);

  const Decision d = die(mid_attack);
  check(d.status == Status::Ok && d.next.id == StateId::Death &&
            d.next.variant == AttackVariant::Slam,
        "death overrides an active attack");
  check(valid(d.next, cfg), "death state is structurally valid");

  State dead = d.next;
  check(start_move(dead, 500).status == Status::Terminal,
        "no movement after death");
  check(stop(dead).status == Status::Terminal, "no stop after death");
  check(attack(dead, AttackVariant::Cast, cfg).status == Status::Terminal,
        "no attack after death");
  check(take_hit(dead, cfg).status == Status::Terminal, "corpse ignores hits");
  check(die(dead).status == Status::Terminal, "second death is refused");
  check(face(dead, Facing::North).status == Status::Terminal,
        "no facing changes after death");

  const State advanced =
      tick(dead, static_cast<std::int64_t>(cfg.death_ms) * 3, cfg);
  check(advanced.id == StateId::Death, "tick never leaves death");
  const Pose settled = pose(advanced, cfg);
  check(settled.phase == 1.0f, "death phase clamps at 1 and never loops");
  check(pose(dead, cfg).phase < settled.phase,
        "death plays forward before clamping");

  dead = advanced;
  check(attack(dead, AttackVariant::Swing, cfg).status == Status::Terminal,
        "irreversibility holds after any amount of time");
}

void append_snapshot(const State& s, const char* label, std::string& out) {
  const Config cfg = default_config();
  const Pose pp = pose(s, cfg);
  char buf[224];
  std::snprintf(buf, sizeof(buf),
                "%s %s %s %s u=%u sp=%u ph=%.5f kf=%u bob=%.5f wa=%.5f "
                "oa=%.5f ln=%.5f\n",
                label, name(s.id), name(s.variant), name(s.facing),
                static_cast<unsigned>(s.elapsed_ms),
                static_cast<unsigned>(s.speed_permille), pp.phase,
                static_cast<unsigned>(pp.keyframe), pp.root_bob,
                pp.weapon_arm_deg, pp.off_arm_deg, pp.lean_deg);
  out += buf;
}

void run_scripted_trace(std::string& out) {
  const Config cfg = default_config();
  State s;
  append_snapshot(s, "boot", out);
  s = face(s, Facing::NorthEast).next;
  s = start_move(s, 850).next;
  for (int i = 0; i < 5; ++i) {
    s = tick(s, 137, cfg);
    append_snapshot(s, "move", out);
  }
  s = attack(s, AttackVariant::Thrust, cfg).next;
  for (int i = 0; i < 12; ++i) {
    s = tick(s, 53, cfg);
    append_snapshot(s, "thrust", out);
    if (s.id == StateId::Idle) break;
  }
  const Decision interrupt = take_hit(s, cfg);
  if (interrupt.status == Status::Ok) s = interrupt.next;
  s = tick(s, 90, cfg);
  append_snapshot(s, "after-hit", out);
  s = attack(s, AttackVariant::Slam, cfg).next;
  for (int i = 0; i < 60; ++i) {
    s = tick(s, 41, cfg);
    append_snapshot(s, "slam", out);
    if (s.id == StateId::Recovery &&
        attack(s, AttackVariant::Cast, cfg).status == Status::Ok) {
      break;
    }
  }
  s = attack(s, AttackVariant::Cast, cfg).next;
  for (int i = 0; i < 40; ++i) {
    s = tick(s, 29, cfg);
    append_snapshot(s, "cast-chain", out);
    if (s.id == StateId::Idle) break;
  }
  s = die(s).next;
  for (int i = 0; i < 10; ++i) {
    s = tick(s, 333, cfg);
    append_snapshot(s, "death", out);
  }
  check(valid(s, cfg), "scripted walk ends in a structurally valid state");
}

void test_determinism_byte_identical() {
  std::string first;
  std::string second;
  run_scripted_trace(first);
  run_scripted_trace(second);
  check(first.size() > 2000, "determinism trace is substantial");
  check(first == second,
        "two identical runs produce byte-identical state sequences");

  std::uint64_t hash = 1469598103934665603ull;
  for (const char ch : first) {
    hash ^= static_cast<std::uint8_t>(ch);
    hash *= 1099511628211ull;
  }
  check(hash != 0, "trace hashes to a nonzero fingerprint");
}

void test_nonzero_visible_attacks() {
  const Config cfg = default_config();

  for (std::uint8_t v = 0; v < kVariantCount; ++v) {
    const auto variant = static_cast<AttackVariant>(v);
    check(cfg.windup_ms[v] > 0,
          std::string(name(variant)) + " windup duration nonzero");
    check(cfg.attack_ms[v] > 0,
          std::string(name(variant)) + " attack duration nonzero");
    check(cfg.recovery_ms[v] > 0,
          std::string(name(variant)) + " recovery duration nonzero");

    State s = attack(State{}, variant, cfg).next;
    bool sampled_attack = false;
    const std::uint64_t full = static_cast<std::uint64_t>(cfg.windup_ms[v]) +
                               cfg.attack_ms[v] + cfg.recovery_ms[v];
    std::uint64_t t = 0;
    while (t <= full && s.id != StateId::Idle) {
      if (s.id == StateId::Attack) {
        sampled_attack = true;
      }
      s = tick(s, 13, cfg);
      t += 13;
    }
    check(sampled_attack,
          std::string(name(variant)) + " visibly enters its attack state");
    const Pose near_end =
        pose(attack_state(variant, cfg.attack_ms[v] - 1), kConfig);
    check(near_end.phase >= 0.99f,
          std::string(name(variant)) + " attack phase reaches full extension");
    check(s.id == StateId::Idle,
          std::string(name(variant)) +
              " completes into idle within its own nonzero budget");
  }

  const auto swing_q = pose(attack_state(AttackVariant::Swing, 110), kConfig);
  const auto slam_q = pose(attack_state(AttackVariant::Slam, 210), kConfig);
  const auto cast_q = pose(attack_state(AttackVariant::Cast, 150), kConfig);
  const auto thrust_q = pose(attack_state(AttackVariant::Thrust, 80), kConfig);
  check(swing_q.weapon_arm_deg != slam_q.weapon_arm_deg &&
            swing_q.weapon_arm_deg != cast_q.weapon_arm_deg &&
            slam_q.weapon_arm_deg != thrust_q.weapon_arm_deg &&
            cast_q.keyframe != swing_q.keyframe,
        "attack variants emit pairwise-distinct pose curves");
  check(detail::attack_curve(AttackVariant::Swing).keyframe_count !=
            detail::attack_curve(AttackVariant::Thrust).keyframe_count &&
            detail::attack_curve(AttackVariant::Slam).keyframe_count !=
                detail::attack_curve(AttackVariant::Cast).keyframe_count,
        "variant keyframe strips differ");

  Config zeroed = default_config();
  zeroed.attack_ms[vidx(AttackVariant::Slam)] = 0;
  check(!valid_config(zeroed),
        "zero-duration attack config rejected by validation");
  Config zero_windup = default_config();
  zero_windup.windup_ms[vidx(AttackVariant::Cast)] = 0;
  check(!valid_config(zero_windup), "zero-duration windup config rejected");
  Config window_too_big = default_config();
  window_too_big.recovery_chain_window_ms = window_too_big.recovery_ms[0] + 1;
  check(!valid_config(window_too_big),
        "chain window larger than recovery rejected");

#ifdef NEGATIVE_CONTROL
  // Deliberate negative control: assert the inverted expectation so this
  // binary MUST fail (exit code 1). The harness compiles it separately and
  // requires that failure, proving these checks detect a zero-duration
  // attack regression rather than passing vacuously.
  check(valid_config(zeroed),
        "NEGATIVE CONTROL (expected failure): zero-duration attack accepted");
#endif

  check(valid_config(default_config()), "default config validates");
}

}  // namespace

int main() {
  test_transition_legality();
  test_timing_and_phase_wrap();
  test_facing_changes();
  test_interruption_matrix();
  test_death_override_irreversibility();
  test_determinism_byte_identical();
  test_nonzero_visible_attacks();
  std::cout << g_checks << " checks passed\n";
  return 0;
}
