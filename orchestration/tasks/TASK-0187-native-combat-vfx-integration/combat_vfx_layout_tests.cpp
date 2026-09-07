// combat_vfx_layout_tests.cpp — TASK-0187 layout planner tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "actor_combat_bridge.hpp"
#include "combat_vfx_layout.hpp"

using namespace combat_vfx_layout;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_swing_stroke_alpha_fade() {
  actor_animation::State actor;
  actor_animation::Timing timing;
  timing.windup_ticks = 1;
  attack_vfx::Planner planner;
  attack_vfx::Config cfg;
  const attack_vfx::Point origin{120, 80};

  check(actor_animation::apply_intent(actor, actor_animation::Intent::AttackSwing,
                                      actor_animation::Facing::East) ==
            actor_animation::Status::Ok,
        "begin swing");

  check(actor_combat_bridge::tick_actor(actor, timing, planner, cfg, 4, origin)
            .vfx_status == attack_vfx::Status::Ok,
        "emit swing");

  const Viewport vp{1920, 1080};
  const LayoutPlan plan = plan_combat_vfx(planner, vp);
  check(plan.valid, "layout valid");
  check(plan.count == 1, "one stroke");
  check(plan.strokes[0].kind == attack_vfx::PrimitiveKind::SwingArc,
        "swing arc stroke");
  check(plan.strokes[0].alpha == 1000, "full alpha at spawn");
  check(plan.strokes[0].attacker_id == 4u, "attacker preserved");

  attack_vfx::tick(planner);
  const LayoutPlan faded = plan_combat_vfx(planner, vp);
  check(faded.strokes[0].alpha < plan.strokes[0].alpha, "alpha fades on tick");
}

void test_hit_marker_point_stroke() {
  attack_vfx::Planner planner;
  attack_vfx::Config cfg;
  const attack_vfx::Point at{300, 400};
  check(attack_vfx::plan_hit_marker(planner, cfg, 55, at) ==
            attack_vfx::Status::Ok,
        "plan hit marker");

  const LayoutPlan plan = plan_combat_vfx(planner, Viewport{1280, 720});
  check(plan.strokes[0].kind == attack_vfx::PrimitiveKind::HitMarker,
        "hit marker stroke");
  check(plan.strokes[0].target_id == 55u, "target id");
  check(plan.strokes[0].from.x == 300, "marker x");
  check(plan.strokes[0].to.x == 300, "marker end x");
}

void test_invalid_viewport_rejected() {
  attack_vfx::Planner planner;
  attack_vfx::Config cfg;
  check(attack_vfx::plan_attack(planner, cfg, attack_vfx::Style::Thrust, 1,
                                attack_vfx::Facing::North,
                                attack_vfx::Point{0, 0}) ==
            attack_vfx::Status::Ok,
        "plan thrust");
  check(!plan_combat_vfx(planner, Viewport{0, 720}).valid,
        "zero width rejected");
}

void test_checksum_stable() {
  attack_vfx::Planner planner;
  attack_vfx::Config cfg;
  check(attack_vfx::plan_attack(planner, cfg, attack_vfx::Style::Slam, 9,
                                attack_vfx::Facing::South,
                                attack_vfx::Point{64, 64}) ==
            attack_vfx::Status::Ok,
        "plan slam");
  const Viewport vp{1366, 768};
  const LayoutPlan plan = plan_combat_vfx(planner, vp);
  const std::uint32_t a = plan_checksum(plan);
  const std::uint32_t b = plan_checksum(plan);
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

}  // namespace

int main() {
  test_swing_stroke_alpha_fade();
  test_hit_marker_point_stroke();
  test_invalid_viewport_rejected();
  test_checksum_stable();
  std::cout << "combat_vfx_layout_tests: " << g_checks << " checks passed\n";
  return 0;
}
