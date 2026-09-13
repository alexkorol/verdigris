// geometric_skill_tree.hpp — TASK-0193 WIZARD geometric-tree presentation model.
//
// Deterministic axial-hex seat lattice for Owner Demo first level-up: adjacency
// allocation, one-sign cap, skill-point pool, and active-seat bookkeeping.
// Bounded slice (center + ring-1 neighbors) mirrors WIZARD tree-data geometry
// without importing the full 331-seat lattice in this packet.
#pragma once

#include <array>
#include <cstdint>

namespace geometric_skill_tree {

struct Axial {
  std::int8_t q = 0;
  std::int8_t r = 0;

  [[nodiscard]] constexpr bool operator==(const Axial&) const = default;
};

enum class SeatType : std::uint8_t {
  Passive = 0,
  Sign,
  Socket,
  Gateway,
  Keystone,
  Class,
};

enum class Status : std::uint8_t {
  Ok,
  TreeLocked,
  InvalidSeat,
  AlreadyActive,
  NotAdjacent,
  NoPoints,
  SignTaken,
};

inline constexpr std::uint8_t kMaxSeats = 32;
inline constexpr std::uint16_t kWizardPhase0SkillPoints = 140;
inline constexpr std::uint16_t kOwnerDemoFirstLevelPoints = 1;
inline constexpr std::uint8_t kOwnerDemoFirstLevel = 2;

struct Seat {
  Axial pos{};
  SeatType type = SeatType::Passive;
  std::uint8_t point_cost = 1;
  bool active = false;

  [[nodiscard]] constexpr bool operator==(const Seat&) const = default;
};

struct State {
  std::uint8_t seat_count = 0;
  std::array<Seat, kMaxSeats> seats{};
  std::uint16_t skill_points = 0;
  std::uint8_t player_level = 1;
  bool tree_unlocked = false;
  bool sign_allocated = false;
  std::uint8_t allocated_count = 0;

  [[nodiscard]] constexpr bool operator==(const State&) const = default;
};

[[nodiscard]] constexpr const char* name(Status status) {
  switch (status) {
    case Status::Ok:
      return "Ok";
    case Status::TreeLocked:
      return "TreeLocked";
    case Status::InvalidSeat:
      return "InvalidSeat";
    case Status::AlreadyActive:
      return "AlreadyActive";
    case Status::NotAdjacent:
      return "NotAdjacent";
    case Status::NoPoints:
      return "NoPoints";
    case Status::SignTaken:
      return "SignTaken";
  }
  return "Unknown";
}

[[nodiscard]] constexpr const char* seat_type_name(SeatType type) {
  switch (type) {
    case SeatType::Passive:
      return "passive";
    case SeatType::Sign:
      return "sign";
    case SeatType::Socket:
      return "socket";
    case SeatType::Gateway:
      return "gateway";
    case SeatType::Keystone:
      return "keystone";
    case SeatType::Class:
      return "class";
  }
  return "unknown";
}

[[nodiscard]] constexpr std::int8_t axial_s(std::int8_t q, std::int8_t r) {
  return static_cast<std::int8_t>(-q - r);
}

[[nodiscard]] constexpr std::uint8_t hex_distance(Axial a, Axial b) {
  const std::int8_t dq = static_cast<std::int8_t>(a.q - b.q);
  const std::int8_t dr = static_cast<std::int8_t>(a.r - b.r);
  const std::int8_t ds = static_cast<std::int8_t>(
      axial_s(a.q, a.r) - axial_s(b.q, b.r));
  const auto abs_i = [](std::int8_t v) {
    return v < 0 ? static_cast<std::uint8_t>(-v) : static_cast<std::uint8_t>(v);
  };
  const std::uint8_t adq = abs_i(dq);
  const std::uint8_t adr = abs_i(dr);
  const std::uint8_t ads = abs_i(ds);
  return adq > adr ? (adq > ads ? adq : ads) : (adr > ads ? adr : ads);
}

[[nodiscard]] constexpr std::uint8_t seat_ring(const Seat& seat) {
  return hex_distance(seat.pos, Axial{0, 0});
}

[[nodiscard]] constexpr bool seats_adjacent(const Seat& a, const Seat& b) {
  return hex_distance(a.pos, b.pos) == 1;
}

[[nodiscard]] constexpr State make_owner_demo_first_level_slice() {
  State s{};
  constexpr std::array<Axial, 7> kCoords = {
      Axial{0, 0},
      Axial{1, 0},
      Axial{0, 1},
      Axial{-1, 1},
      Axial{-1, 0},
      Axial{0, -1},
      Axial{1, -1},
  };
  constexpr std::array<SeatType, 7> kTypes = {
      SeatType::Passive,
      SeatType::Sign,
      SeatType::Passive,
      SeatType::Passive,
      SeatType::Socket,
      SeatType::Passive,
      SeatType::Passive,
  };
  s.seat_count = static_cast<std::uint8_t>(kCoords.size());
  for (std::uint8_t i = 0; i < s.seat_count; ++i) {
    s.seats[i].pos = kCoords[i];
    s.seats[i].type = kTypes[i];
    s.seats[i].point_cost = 1;
    s.seats[i].active = i == 0;
  }
  s.allocated_count = 1;
  return s;
}

[[nodiscard]] constexpr std::int8_t find_seat_index(const State& state, Axial pos) {
  for (std::uint8_t i = 0; i < state.seat_count; ++i) {
    if (state.seats[i].pos == pos) {
      return static_cast<std::int8_t>(i);
    }
  }
  return -1;
}

[[nodiscard]] constexpr bool is_adjacent_to_active(const State& state,
                                                   std::uint8_t seat_index) {
  if (seat_index >= state.seat_count) {
    return false;
  }
  const Seat& target = state.seats[seat_index];
  for (std::uint8_t i = 0; i < state.seat_count; ++i) {
    if (!state.seats[i].active) {
      continue;
    }
    if (seats_adjacent(target, state.seats[i])) {
      return true;
    }
  }
  return false;
}

[[nodiscard]] constexpr Status on_player_level(State& state, std::uint8_t level) {
  if (level < state.player_level) {
    return Status::Ok;
  }
  state.player_level = level;
  if (level >= kOwnerDemoFirstLevel && !state.tree_unlocked) {
    state.tree_unlocked = true;
    state.skill_points = kOwnerDemoFirstLevelPoints;
  }
  return Status::Ok;
}

[[nodiscard]] constexpr Status allocate_seat(State& state, std::uint8_t seat_index) {
  if (!state.tree_unlocked) {
    return Status::TreeLocked;
  }
  if (seat_index >= state.seat_count) {
    return Status::InvalidSeat;
  }
  Seat& seat = state.seats[seat_index];
  if (seat.active) {
    return Status::AlreadyActive;
  }
  if (!is_adjacent_to_active(state, seat_index)) {
    return Status::NotAdjacent;
  }
  if (seat.point_cost > state.skill_points) {
    return Status::NoPoints;
  }
  if (seat.type == SeatType::Sign && state.sign_allocated) {
    return Status::SignTaken;
  }

  seat.active = true;
  state.skill_points =
      static_cast<std::uint16_t>(state.skill_points - seat.point_cost);
  state.allocated_count =
      static_cast<std::uint8_t>(state.allocated_count + 1);
  if (seat.type == SeatType::Sign) {
    state.sign_allocated = true;
  }
  return Status::Ok;
}

[[nodiscard]] constexpr Status allocate_at(State& state, Axial pos) {
  const std::int8_t index = find_seat_index(state, pos);
  if (index < 0) {
    return Status::InvalidSeat;
  }
  return allocate_seat(state, static_cast<std::uint8_t>(index));
}

}  // namespace geometric_skill_tree
