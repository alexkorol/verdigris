// attack_vfx.hpp — TASK-0174 presentation attack VFX planner.
//
// Plans swing arcs, thrust streaks, slam rings, projectile trails, impact
// flashes, and hit markers from combat events. Geometry, lifetime, facing,
// attribution, and clip bounds are deterministic; no GPU/paint integration.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace attack_vfx {

enum class Facing : std::uint8_t {
  North = 0,
  East,
  South,
  West,
};

enum class Style : std::uint8_t {
  Swing = 0,
  Thrust,
  Slam,
  Projectile,
};

enum class PrimitiveKind : std::uint8_t {
  SwingArc = 0,
  ThrustStreak,
  SlamRing,
  ProjectileTrail,
  ImpactFlash,
  HitMarker,
};

enum class Status : std::uint8_t {
  Ok,
  Full,
  Invalid,
};

struct Point {
  std::int16_t x = 0;
  std::int16_t y = 0;

  [[nodiscard]] constexpr bool operator==(const Point&) const = default;
};

struct ClipRect {
  std::int16_t left = 0;
  std::int16_t top = 0;
  std::int16_t right = 960;
  std::int16_t bottom = 600;

  [[nodiscard]] constexpr bool contains(Point p) const {
    return p.x >= left && p.x <= right && p.y >= top && p.y <= bottom;
  }
};

struct Config {
  std::uint16_t swing_ticks = 3;
  std::uint16_t thrust_ticks = 2;
  std::uint16_t slam_ticks = 5;
  std::uint16_t trail_ticks = 6;
  std::uint16_t flash_ticks = 2;
  std::uint16_t hit_ticks = 4;
  std::uint16_t swing_radius = 40;
  std::uint16_t thrust_length = 56;
  std::uint16_t slam_radius = 72;
  ClipRect clip{};
};

struct Primitive {
  PrimitiveKind kind = PrimitiveKind::SwingArc;
  std::uint32_t attacker_id = 0;
  std::uint32_t target_id = 0;
  Facing facing = Facing::South;
  Point origin{};
  Point end{};
  std::uint16_t radius = 0;
  std::uint16_t ticks_total = 0;
  std::uint16_t ticks_left = 0;
  bool clipped = false;

  [[nodiscard]] constexpr bool operator==(const Primitive&) const = default;
};

inline constexpr std::size_t kMaxPrimitives = 24;

struct Planner {
  std::array<Primitive, kMaxPrimitives> items{};
  std::uint8_t count = 0;
};

[[nodiscard]] constexpr const char* name(Status status) {
  switch (status) {
    case Status::Ok:
      return "ok";
    case Status::Full:
      return "full";
    case Status::Invalid:
      return "invalid";
  }
  return "unknown-status";
}

[[nodiscard]] constexpr Point facing_delta(Facing facing,
                                         std::int16_t magnitude) {
  switch (facing) {
    case Facing::North:
      return {0, -magnitude};
    case Facing::East:
      return {magnitude, 0};
    case Facing::South:
      return {0, magnitude};
    case Facing::West:
      return {-magnitude, 0};
  }
  return {0, 0};
}

[[nodiscard]] constexpr Point add(Point a, Point b) {
  return {static_cast<std::int16_t>(a.x + b.x),
          static_cast<std::int16_t>(a.y + b.y)};
}

constexpr bool mark_clip(Primitive& p, const ClipRect& clip) {
  const bool origin_inside = clip.contains(p.origin);
  const bool end_inside = clip.contains(p.end);
  p.clipped = !origin_inside || !end_inside;
  return p.clipped;
}

[[nodiscard]] constexpr Status push(Planner& planner, Primitive prim) {
  if (planner.count >= kMaxPrimitives) return Status::Full;
  planner.items[static_cast<std::size_t>(planner.count)] = prim;
  ++planner.count;
  return Status::Ok;
}

[[nodiscard]] constexpr Status plan_attack(Planner& planner, const Config& cfg,
                                         Style style, std::uint32_t attacker_id,
                                         Facing facing, Point origin) {
  if (attacker_id == 0) return Status::Invalid;

  Primitive p;
  p.attacker_id = attacker_id;
  p.facing = facing;
  p.origin = origin;

  switch (style) {
    case Style::Swing:
      p.kind = PrimitiveKind::SwingArc;
      p.radius = cfg.swing_radius;
      p.ticks_total = cfg.swing_ticks;
      p.ticks_left = cfg.swing_ticks;
      p.end = add(origin, facing_delta(facing, cfg.swing_radius));
      break;
    case Style::Thrust:
      p.kind = PrimitiveKind::ThrustStreak;
      p.ticks_total = cfg.thrust_ticks;
      p.ticks_left = cfg.thrust_ticks;
      p.end = add(origin, facing_delta(facing, static_cast<std::int16_t>(cfg.thrust_length)));
      break;
    case Style::Slam:
      p.kind = PrimitiveKind::SlamRing;
      p.radius = cfg.slam_radius;
      p.ticks_total = cfg.slam_ticks;
      p.ticks_left = cfg.slam_ticks;
      p.end = origin;
      break;
    case Style::Projectile:
      p.kind = PrimitiveKind::ProjectileTrail;
      p.ticks_total = cfg.trail_ticks;
      p.ticks_left = cfg.trail_ticks;
      p.end = add(origin, facing_delta(facing, static_cast<std::int16_t>(cfg.thrust_length * 2)));
      break;
  }

  mark_clip(p, cfg.clip);
  return push(planner, p);
}

[[nodiscard]] constexpr Status plan_impact(Planner& planner, const Config& cfg,
                                       std::uint32_t attacker_id,
                                       std::uint32_t target_id, Point at) {
  if (attacker_id == 0 || target_id == 0) return Status::Invalid;
  Primitive p;
  p.kind = PrimitiveKind::ImpactFlash;
  p.attacker_id = attacker_id;
  p.target_id = target_id;
  p.origin = at;
  p.end = at;
  p.ticks_total = cfg.flash_ticks;
  p.ticks_left = cfg.flash_ticks;
  mark_clip(p, cfg.clip);
  return push(planner, p);
}

[[nodiscard]] constexpr Status plan_hit_marker(Planner& planner,
                                               const Config& cfg,
                                               std::uint32_t target_id,
                                               Point at) {
  if (target_id == 0) return Status::Invalid;
  Primitive p;
  p.kind = PrimitiveKind::HitMarker;
  p.target_id = target_id;
  p.origin = at;
  p.end = at;
  p.ticks_total = cfg.hit_ticks;
  p.ticks_left = cfg.hit_ticks;
  mark_clip(p, cfg.clip);
  return push(planner, p);
}

constexpr void tick(Planner& planner) {
  std::uint8_t write = 0;
  for (std::uint8_t i = 0; i < planner.count; ++i) {
    Primitive p = planner.items[static_cast<std::size_t>(i)];
    if (p.ticks_left > 0) {
      --p.ticks_left;
    }
    if (p.ticks_left > 0) {
      planner.items[static_cast<std::size_t>(write)] = p;
      ++write;
    }
  }
  planner.count = write;
}

[[nodiscard]] constexpr std::array<std::uint8_t, kMaxPrimitives>
stable_render_order(const Planner& planner) {
  std::array<std::uint8_t, kMaxPrimitives> order{};
  for (std::uint8_t i = 0; i < planner.count; ++i) {
    order[i] = i;
  }
  return order;
}

}  // namespace attack_vfx
