// actor_animation_tests.cpp — TASK-0173 acceptance tests.

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

void expect_status(Status got, Status expected, const std::string& label) {
  check(got == expected,
        label + ": expected " + name(got) + " vs " + name(expected));
}

void test_idle_locomotion_facing() {
  State s;
  Timing t;
  expect_status(apply_intent(s, Intent::Move, Facing::East), Status::Ok,
                "move from idle");
  check(s.phase == Phase::Locomotion && s.facing == Facing::East,
        "locomotion east");
  expect_status(apply_intent(s, Intent::Stop, Facing::East), Status::Ok,
                "stop");
  check(s.phase == Phase::Idle, "back to idle");
}

void test_swing_attack_pipeline() {
  State s;
  Timing t{4, 3, 2, 5, 3, 2};
  expect_status(apply_intent(s, Intent::AttackSwing, Facing::North), Status::Ok,
                "start swing");
  check(s.phase == Phase::Windup, "windup");
  check(is_attack_visible(s), "windup visible");

  expect_status(advance(s, t, t.windup_ticks), Status::Ok, "windup ticks");
  check(s.phase == Phase::Swing, "swing phase");
  check(is_attack_visible(s), "swing visible nonzero");

  expect_status(advance(s, t, t.swing_ticks), Status::Ok, "swing ticks");
  check(s.phase == Phase::Recovery, "recovery");

  expect_status(advance(s, t, t.recovery_ticks), Status::Ok, "recovery ticks");
  check(s.phase == Phase::Idle, "idle after recovery");
}

void test_hit_interrupts_windup() {
  State s;
  Timing t;
  expect_status(apply_intent(s, Intent::AttackSlam, Facing::West), Status::Ok,
                "start slam");
  expect_status(apply_intent(s, Intent::TakeHit, Facing::West), Status::Ok,
                "hit interrupts windup");
  check(s.phase == Phase::Hit, "in hit react");
  expect_status(advance(s, t, t.hit_ticks), Status::Ok, "hit duration");
  check(s.phase == Phase::Idle, "idle after hit");
}

void test_death_terminal() {
  State s;
  Timing t;
  expect_status(apply_intent(s, Intent::AttackThrust, Facing::South), Status::Ok,
                "start thrust");
  expect_status(apply_intent(s, Intent::Die, Facing::South), Status::Ok, "die");
  check(s.dead(), "dead flag");
  expect_status(apply_intent(s, Intent::Move, Facing::East), Status::Dead,
                "no move when dead");
  expect_status(tick(s, t), Status::Dead, "tick when dead");
}

void test_thrust_and_slam_timing() {
  State s;
  Timing t{2, 3, 4, 6, 2, 1};
  expect_status(apply_intent(s, Intent::AttackThrust, Facing::East), Status::Ok,
                "thrust start");
  expect_status(advance(s, t, t.windup_ticks), Status::Ok, "thrust windup");
  check(s.phase == Phase::Thrust, "thrust phase");
  expect_status(advance(s, t, t.thrust_ticks), Status::Ok, "thrust active");
  check(s.phase == Phase::Recovery, "thrust recovery");

  State s2;
  expect_status(apply_intent(s2, Intent::AttackSlam, Facing::South), Status::Ok,
                "slam start");
  expect_status(advance(s2, t, t.windup_ticks), Status::Ok, "slam windup");
  check(s2.phase == Phase::Slam, "slam phase");
  check(is_attack_visible(s2), "slam visible");
}

void test_deterministic_replay() {
  Timing t;
  State a;
  State b;
  expect_status(apply_intent(a, Intent::AttackSwing, Facing::West), Status::Ok,
                "replay a");
  expect_status(apply_intent(b, Intent::AttackSwing, Facing::West), Status::Ok,
                "replay b");
  for (std::uint16_t i = 0; i < 20; ++i) {
    expect_status(tick(a, t), Status::Ok, "tick a");
    expect_status(tick(b, t), Status::Ok, "tick b");
  }
  check(a == b, "replay identical");
}

void test_invalid_attack_during_recovery() {
  State s;
  Timing t;
  expect_status(apply_intent(s, Intent::AttackSwing, Facing::North), Status::Ok,
                "start swing");
  expect_status(advance(s, t, t.windup_ticks + t.swing_ticks), Status::Ok,
                "through swing");
  check(s.phase == Phase::Recovery, "in recovery");
  expect_status(apply_intent(s, Intent::AttackSwing, Facing::North),
                Status::InvalidTransition, "reject attack during recovery");
}

}  // namespace

int main() {
  test_idle_locomotion_facing();
  test_swing_attack_pipeline();
  test_hit_interrupts_windup();
  test_death_terminal();
  test_thrust_and_slam_timing();
  test_deterministic_replay();
  test_invalid_attack_during_recovery();

  std::cout << "TASK-0173 actor animation acceptance: " << g_checks
            << " checks passed\n";
  return 0;
}
