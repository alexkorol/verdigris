// actor_combat_bridge_tests.cpp — TASK-0186/0187 bridge tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "actor_combat_bridge.hpp"

using namespace actor_combat_bridge;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_swing_attack_emits_arc() {
  actor_animation::State actor;
  actor_animation::Timing timing;
  timing.windup_ticks = 2;
  attack_vfx::Planner planner;
  attack_vfx::Config cfg;
  const attack_vfx::Point origin{100, 200};

  check(actor_animation::apply_intent(actor, actor_animation::Intent::AttackSwing,
                                      actor_animation::Facing::East) ==
            actor_animation::Status::Ok,
        "begin swing");

  TickResult windup1 = tick_actor(actor, timing, planner, cfg, 7, origin);
  check(windup1.after.phase == actor_animation::Phase::Windup,
        "still winding up");

  TickResult release = tick_actor(actor, timing, planner, cfg, 7, origin);
  check(release.after.phase == actor_animation::Phase::Swing,
        "swing phase active");
  check(planner.count == 1, "one primitive emitted");
  check(planner.items[0].kind == attack_vfx::PrimitiveKind::SwingArc,
        "swing arc primitive");
  check(planner.items[0].attacker_id == 7u, "attacker id preserved");
}

void test_hit_emits_marker() {
  actor_animation::State actor;
  actor_animation::Timing timing;
  attack_vfx::Planner planner;
  attack_vfx::Config cfg;
  const attack_vfx::Point origin{50, 60};

  TickResult hit =
      apply_and_tick(actor, actor_animation::Intent::TakeHit,
                     actor_animation::Facing::South, timing, planner, cfg, 3,
                     origin, 99);
  check(hit.after.phase == actor_animation::Phase::Hit, "hit phase");
  check(planner.count == 1, "hit marker emitted");
  check(planner.items[0].kind == attack_vfx::PrimitiveKind::HitMarker,
        "hit marker kind");
  check(planner.items[0].target_id == 99u, "target id");
}

void test_death_suppresses_attack_vfx() {
  actor_animation::State actor;
  actor.phase = actor_animation::Phase::Death;
  attack_vfx::Planner planner;
  attack_vfx::Config cfg;
  const TickResult result =
      tick_actor(actor, actor_animation::Timing{}, planner, cfg, 1,
                 attack_vfx::Point{0, 0});
  check(result.anim_status == actor_animation::Status::Dead, "dead actor");
  check(planner.count == 0, "no vfx while dead");
}

void test_facing_map() {
  check(map_facing(actor_animation::Facing::West) == attack_vfx::Facing::West,
        "facing map");
}

}  // namespace

int main() {
  test_swing_attack_emits_arc();
  test_hit_emits_marker();
  test_death_suppresses_attack_vfx();
  test_facing_map();
  std::cout << "actor_combat_bridge_tests: " << g_checks << " checks passed\n";
  return 0;
}
