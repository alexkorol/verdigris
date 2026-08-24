// attack_vfx_tests.cpp — TASK-0174 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "attack_vfx.hpp"

using namespace attack_vfx;

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
        label + ": expected " + name(expected) + " got " + name(got));
}

void test_swing_geometry_and_facing() {
  Planner p;
  Config cfg;
  Point origin{100, 100};
  expect_status(plan_attack(p, cfg, Style::Swing, 7, Facing::East, origin),
                Status::Ok, "swing spawn");
  const Primitive& prim = p.items[0];
  check(prim.kind == PrimitiveKind::SwingArc, "swing kind");
  check(prim.attacker_id == 7, "attacker attribution");
  check(prim.end.x == origin.x + static_cast<std::int16_t>(cfg.swing_radius),
        "swing end east");
  check(prim.ticks_left == cfg.swing_ticks, "swing lifetime");
}

void test_thrust_and_slam() {
  Planner p;
  Config cfg;
  Point o{50, 50};
  expect_status(plan_attack(p, cfg, Style::Thrust, 1, Facing::South, o),
                Status::Ok, "thrust");
  check(p.items[0].kind == PrimitiveKind::ThrustStreak, "thrust kind");
  check(p.items[0].end.y == o.y + static_cast<std::int16_t>(cfg.thrust_length),
        "thrust length south");

  expect_status(plan_attack(p, cfg, Style::Slam, 2, Facing::West, o), Status::Ok,
                "slam");
  check(p.items[1].kind == PrimitiveKind::SlamRing, "slam kind");
  check(p.items[1].radius == cfg.slam_radius, "slam radius");
}

void test_projectile_trail() {
  Planner p;
  Config cfg;
  expect_status(plan_attack(p, cfg, Style::Projectile, 3, Facing::North,
                            {200, 200}),
                Status::Ok, "projectile");
  check(p.items[0].kind == PrimitiveKind::ProjectileTrail, "trail kind");
  check(p.items[0].ticks_total == cfg.trail_ticks, "trail lifetime");
}

void test_impact_and_hit_marker() {
  Planner p;
  Config cfg;
  Point hit{300, 120};
  expect_status(plan_impact(p, cfg, 5, 9, hit), Status::Ok, "impact");
  check(p.items[0].kind == PrimitiveKind::ImpactFlash, "flash kind");
  check(p.items[0].target_id == 9, "impact target attribution");

  expect_status(plan_hit_marker(p, cfg, 9, hit), Status::Ok, "hit marker");
  check(p.items[1].kind == PrimitiveKind::HitMarker, "marker kind");
}

void test_clipping() {
  Planner p;
  Config cfg;
  cfg.clip = {0, 0, 100, 100};
  expect_status(plan_attack(p, cfg, Style::Thrust, 1, Facing::East, {90, 50}),
                Status::Ok, "off-screen thrust");
  check(p.items[0].clipped, "thrust marked clipped outside bounds");
}

void test_deterministic_expiry() {
  Planner p;
  Config cfg;
  expect_status(plan_attack(p, cfg, Style::Swing, 1, Facing::South, {10, 10}),
                Status::Ok, "spawn for expiry");
  for (std::uint16_t i = 0; i < cfg.swing_ticks; ++i) {
    tick(p);
  }
  check(p.count == 0, "swing expired deterministically");
}

void test_capacity_and_invalid() {
  Planner p;
  Config cfg;
  for (std::uint8_t i = 1; i <= kMaxPrimitives; ++i) {
    expect_status(plan_hit_marker(p, cfg, i, {0, 0}), Status::Ok, "fill");
  }
  expect_status(plan_hit_marker(p, cfg, 99, {0, 0}), Status::Full,
                "reject when full");
  expect_status(plan_attack(p, cfg, Style::Swing, 0, Facing::North, {0, 0}),
                Status::Invalid, "reject attacker 0");
}

void test_stable_render_order() {
  Planner p;
  Config cfg;
  expect_status(plan_attack(p, cfg, Style::Swing, 1, Facing::North, {1, 1}),
                Status::Ok, "order swing");
  expect_status(plan_attack(p, cfg, Style::Thrust, 2, Facing::East, {2, 2}),
                Status::Ok, "order thrust");
  const auto order = stable_render_order(p);
  check(order[0] == 0 && order[1] == 1, "insertion order preserved");
}

}  // namespace

int main() {
  test_swing_geometry_and_facing();
  test_thrust_and_slam();
  test_projectile_trail();
  test_impact_and_hit_marker();
  test_clipping();
  test_deterministic_expiry();
  test_capacity_and_invalid();
  test_stable_render_order();

  std::cout << "TASK-0174 attack VFX acceptance: " << g_checks << " checks passed\n";
  return 0;
}
