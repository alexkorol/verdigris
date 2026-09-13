// gate_interaction.hpp — TASK-0175 readable zone-gate interaction model.
//
// Physical gates with destination labels, hover highlight when the player is
// in range, normal entry, and Ctrl-click fresh-instance intent. Pure planner:
// no networking, no main.cpp integration in this packet.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace gate_interaction {

inline constexpr std::size_t kMaxGates = 8;
inline constexpr std::size_t kLabelCapacity = 24;

struct Point {
  std::int16_t x = 0;
  std::int16_t y = 0;

  [[nodiscard]] constexpr bool operator==(const Point&) const = default;
};

struct GateDef {
  std::uint32_t id = 0;
  std::uint32_t destination_zone = 0;
  Point center{};
  std::uint16_t radius = 32;
  bool accessible = true;
  std::array<char, kLabelCapacity> label{};

  [[nodiscard]] constexpr bool valid() const {
    return id != 0 && destination_zone != 0 && radius > 0;
  }
};

struct World {
  std::array<GateDef, kMaxGates> gates{};
  std::uint8_t count = 0;
};

struct Pointer {
  Point player{};
  bool ctrl_held = false;
};

enum class Hover : std::uint8_t {
  None = 0,
  OutOfRange,
  Highlighted,
  Inaccessible,
};

enum class Command : std::uint8_t {
  None = 0,
  EnterZone,
  FreshInstance,
};

enum class Status : std::uint8_t {
  Ok,
  NotFound,
  OutOfRange,
  Inaccessible,
  Invalid,
};

struct State {
  std::int8_t hovered_index = -1;
  Hover hover = Hover::None;
};

struct Decision {
  Status status = Status::Ok;
  Command command = Command::None;
  std::uint32_t gate_id = 0;
  std::uint32_t destination_zone = 0;

  [[nodiscard]] constexpr bool operator==(const Decision&) const = default;
};

[[nodiscard]] constexpr const char* name(Status status) {
  switch (status) {
    case Status::Ok:
      return "ok";
    case Status::NotFound:
      return "not-found";
    case Status::OutOfRange:
      return "out-of-range";
    case Status::Inaccessible:
      return "inaccessible";
    case Status::Invalid:
      return "invalid";
  }
  return "unknown-status";
}

[[nodiscard]] constexpr std::int32_t distance_squared(Point a, Point b) {
  const std::int32_t dx = static_cast<std::int32_t>(a.x) -
                          static_cast<std::int32_t>(b.x);
  const std::int32_t dy = static_cast<std::int32_t>(a.y) -
                          static_cast<std::int32_t>(b.y);
  return dx * dx + dy * dy;
}

[[nodiscard]] constexpr bool in_range(const GateDef& gate, Point player) {
  const std::int32_t r = static_cast<std::int32_t>(gate.radius);
  return distance_squared(gate.center, player) <= r * r;
}

[[nodiscard]] constexpr State update_hover(const World& world, Point player) {
  State state;
  std::int32_t best_dist = -1;

  for (std::uint8_t i = 0; i < world.count; ++i) {
    const GateDef& gate = world.gates[static_cast<std::size_t>(i)];
    if (!gate.valid()) continue;

    if (!gate.accessible) {
      if (in_range(gate, player)) {
        state.hovered_index = static_cast<std::int8_t>(i);
        state.hover = Hover::Inaccessible;
        return state;
      }
      continue;
    }

    if (!in_range(gate, player)) continue;

    const std::int32_t dist = distance_squared(gate.center, player);
    if (best_dist < 0 || dist < best_dist) {
      best_dist = dist;
      state.hovered_index = static_cast<std::int8_t>(i);
      state.hover = Hover::Highlighted;
    }
  }

  if (state.hover == Hover::None) {
    std::int32_t nearest = -1;
    std::int32_t nearest_dist = -1;
    for (std::uint8_t i = 0; i < world.count; ++i) {
      const GateDef& gate = world.gates[static_cast<std::size_t>(i)];
      if (!gate.valid() || !gate.accessible) continue;
      const std::int32_t max_dist =
          static_cast<std::int32_t>(gate.radius * gate.radius * 4);
      const std::int32_t dist = distance_squared(gate.center, player);
      if (dist > max_dist) continue;
      if (nearest_dist < 0 || dist < nearest_dist) {
        nearest_dist = dist;
        nearest = static_cast<std::int32_t>(i);
      }
    }
    if (nearest >= 0 && !in_range(world.gates[static_cast<std::size_t>(nearest)],
                                  player)) {
      state.hovered_index = static_cast<std::int8_t>(nearest);
      state.hover = Hover::OutOfRange;
    }
  }

  return state;
}

[[nodiscard]] constexpr const char* hovered_label(const World& world,
                                                  const State& state) {
  if (state.hovered_index < 0 ||
      static_cast<std::uint8_t>(state.hovered_index) >= world.count) {
    return "";
  }
  return world.gates[static_cast<std::size_t>(state.hovered_index)].label.data();
}

[[nodiscard]] constexpr Decision activate(const World& world,
                                          const State& state,
                                          bool ctrl_held) {
  Decision out;
  if (state.hovered_index < 0 ||
      static_cast<std::uint8_t>(state.hovered_index) >= world.count) {
    out.status = Status::NotFound;
    return out;
  }

  const GateDef& gate =
      world.gates[static_cast<std::size_t>(state.hovered_index)];

  if (!gate.valid()) {
    out.status = Status::Invalid;
    return out;
  }

  if (!gate.accessible) {
    out.status = Status::Inaccessible;
    return out;
  }

  if (state.hover != Hover::Highlighted) {
    out.status = Status::OutOfRange;
    return out;
  }

  out.gate_id = gate.id;
  out.destination_zone = gate.destination_zone;
  out.command = ctrl_held ? Command::FreshInstance : Command::EnterZone;
  return out;
}

}  // namespace gate_interaction
