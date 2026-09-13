// spell_lattice.hpp — TASK-0195 bounded WIZARD arcane-lattice presentation model.
//
// Plane-tier (tier 0) weave validation through the Element stratum for Owner
// Demo's first elemental modification choice. Mirrors arcane_lattice adjT
// topology without Three.js or shift/schism mutations in this packet.
#pragma once

#include <array>
#include <cstdint>

namespace spell_lattice {

inline constexpr std::uint8_t kLatticeSlotCount = 16;
inline constexpr std::uint8_t kStratumCount = 7;
inline constexpr std::uint8_t kOwnerDemoElementDepth = 4;  // Source..Element

enum class Tier : std::uint8_t {
  Plane = 0,
  Vessel = 1,
  Tesseract = 2,
};

enum class Node : std::uint8_t {
  Source = 0,
  Adversarial,
  Natural,
  Destructive,
  Sustaining,
  Creative,
  Fire,
  Air,
  Water,
  Earth,
  Body,
  Spirit,
  Mind,
  Outer,
  Inner,
  Manifestation,
  Count,
};

enum class Status : std::uint8_t {
  Ok,
  WeaveComplete,
  NotAdjacent,
  InvalidNode,
  InvalidStratum,
  TierLocked,
};

struct SlotRef {
  std::uint8_t row = 0;
  std::uint8_t idx = 0;
};

struct AdjacentDown {
  std::uint8_t count = 0;
  std::uint8_t idx[3]{};
};

struct State {
  Tier max_tier = Tier::Plane;
  std::uint8_t weave_length = 0;
  std::array<Node, kStratumCount> path{};
  bool elemental_choice_made = false;

  [[nodiscard]] constexpr bool operator==(const State&) const = default;
};

[[nodiscard]] constexpr const char* name(Status status) {
  switch (status) {
    case Status::Ok:
      return "Ok";
    case Status::WeaveComplete:
      return "WeaveComplete";
    case Status::NotAdjacent:
      return "NotAdjacent";
    case Status::InvalidNode:
      return "InvalidNode";
    case Status::InvalidStratum:
      return "InvalidStratum";
    case Status::TierLocked:
      return "TierLocked";
  }
  return "Unknown";
}

[[nodiscard]] constexpr const char* node_name(Node node) {
  switch (node) {
    case Node::Source:
      return "Source";
    case Node::Adversarial:
      return "Adversarial";
    case Node::Natural:
      return "Natural";
    case Node::Destructive:
      return "Destructive";
    case Node::Sustaining:
      return "Sustaining";
    case Node::Creative:
      return "Creative";
    case Node::Fire:
      return "Fire";
    case Node::Air:
      return "Air";
    case Node::Water:
      return "Water";
    case Node::Earth:
      return "Earth";
    case Node::Body:
      return "Body";
    case Node::Spirit:
      return "Spirit";
    case Node::Mind:
      return "Mind";
    case Node::Outer:
      return "Outer";
    case Node::Inner:
      return "Inner";
    case Node::Manifestation:
      return "Manifestation";
    case Node::Count:
      return "Invalid";
  }
  return "Unknown";
}

[[nodiscard]] constexpr SlotRef slot_for(Node node) {
  switch (node) {
    case Node::Source:
      return {0, 0};
    case Node::Adversarial:
      return {1, 0};
    case Node::Natural:
      return {1, 1};
    case Node::Destructive:
      return {2, 0};
    case Node::Sustaining:
      return {2, 1};
    case Node::Creative:
      return {2, 2};
    case Node::Fire:
      return {3, 0};
    case Node::Air:
      return {3, 1};
    case Node::Water:
      return {3, 2};
    case Node::Earth:
      return {3, 3};
    case Node::Body:
      return {4, 0};
    case Node::Spirit:
      return {4, 1};
    case Node::Mind:
      return {4, 2};
    case Node::Outer:
      return {5, 0};
    case Node::Inner:
      return {5, 1};
    case Node::Manifestation:
      return {6, 0};
    case Node::Count:
      return {255, 255};
  }
  return {255, 255};
}

[[nodiscard]] constexpr std::uint8_t native_row(Node node) {
  return slot_for(node).row;
}

[[nodiscard]] constexpr AdjacentDown plane_adjacent_down(std::uint8_t row,
                                                         std::uint8_t idx) {
  switch (row) {
    case 0:
      return {2, {0, 1, 0}};
    case 1:
      return idx == 0 ? AdjacentDown{2, {0, 1, 0}}
                      : AdjacentDown{2, {1, 2, 0}};
    case 2:
      if (idx == 0) return {2, {0, 1, 0}};
      if (idx == 1) return {2, {1, 2, 0}};
      return {2, {2, 3, 0}};
    case 3:
      if (idx == 0) return {1, {0, 0, 0}};
      if (idx == 1) return {2, {0, 1, 0}};
      if (idx == 2) return {2, {1, 2, 0}};
      return {1, {2, 0, 0}};
    case 4:
      if (idx == 0) return {2, {0, 1, 0}};
      if (idx == 1) return {2, {1, 2, 0}};
      return {1, {2, 0, 0}};
    case 5:
      return {1, {0, 0, 0}};
    default:
      return {};
  }
}

[[nodiscard]] constexpr bool plane_can_descend(std::uint8_t up_row,
                                               std::uint8_t up_idx,
                                               std::uint8_t down_idx) {
  const AdjacentDown adj = plane_adjacent_down(up_row, up_idx);
  for (std::uint8_t i = 0; i < adj.count; ++i) {
    if (adj.idx[i] == down_idx) {
      return true;
    }
  }
  return false;
}

[[nodiscard]] constexpr bool is_element(Node node) {
  return node == Node::Fire || node == Node::Air || node == Node::Water ||
         node == Node::Earth;
}

[[nodiscard]] constexpr bool has_tag_fire(Node node) {
  return node == Node::Fire;
}

[[nodiscard]] constexpr bool has_tag_burn(Node node) {
  return node == Node::Fire;
}

[[nodiscard]] constexpr State make_owner_demo_plane_state() {
  State s{};
  s.path[0] = Node::Source;
  s.weave_length = 1;
  return s;
}

[[nodiscard]] constexpr Status append_weave(State& state, Node node) {
  if (node >= Node::Count) {
    return Status::InvalidNode;
  }
  if (state.weave_length == 0) {
    return Status::InvalidNode;
  }
  if (state.elemental_choice_made && native_row(node) <= 3) {
    return Status::WeaveComplete;
  }

  const SlotRef prev = slot_for(state.path[state.weave_length - 1]);
  const SlotRef next = slot_for(node);
  if (next.row != prev.row + 1) {
    return Status::InvalidStratum;
  }

  if (state.max_tier == Tier::Plane &&
      !plane_can_descend(prev.row, prev.idx, next.idx)) {
    return Status::NotAdjacent;
  }

  if (state.weave_length >= kStratumCount) {
    return Status::WeaveComplete;
  }

  state.path[state.weave_length] = node;
  state.weave_length =
      static_cast<std::uint8_t>(state.weave_length + 1);
  if (next.row == 3 && is_element(node)) {
    state.elemental_choice_made = true;
  }
  return Status::Ok;
}

[[nodiscard]] constexpr bool owner_demo_elemental_choice_complete(
    const State& state) {
  return state.elemental_choice_made &&
         state.weave_length >= kOwnerDemoElementDepth;
}

[[nodiscard]] constexpr Node chosen_element(const State& state) {
  for (std::uint8_t i = 0; i < state.weave_length; ++i) {
    if (is_element(state.path[i])) {
      return state.path[i];
    }
  }
  return Node::Count;
}

[[nodiscard]] constexpr const char* school_for_path(const State& state) {
  if (state.weave_length < 3) {
    return "";
  }
  const Node align = state.path[1];
  const Node focus = state.path[2];
  if (align == Node::Adversarial && focus == Node::Destructive) {
    return "Chaos";
  }
  if (align == Node::Adversarial && focus == Node::Sustaining) {
    return "Death-drain";
  }
  if (align == Node::Natural && focus == Node::Sustaining) {
    return "Life";
  }
  if (align == Node::Natural && focus == Node::Creative) {
    return "Nature";
  }
  return "";
}

}  // namespace spell_lattice
