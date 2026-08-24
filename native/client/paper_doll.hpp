// paper_doll.hpp — TASK-0172 pure equipment slot model.
//
// Header-only deterministic paper doll aligned with WIZARD rpg_inventory
// DEFAULT_EQUIPMENT slots. Maps weapon/tool, armor, and accessories without
// medieval class restrictions. Two-handed weapons block off-hand occupancy.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace paper_doll {

enum class Slot : std::uint8_t {
  Head = 0,
  Amulet,
  MainHand,
  Body,
  Cloak,
  OffHand,
  Gloves,
  Belt,
  Boots,
  Ring1,
  Ring2,
  Warhorn,
  QuickRig,
  Attendant,
};

inline constexpr std::uint8_t kSlotCount =
    static_cast<std::uint8_t>(Slot::Attendant) + 1;

enum class Kind : std::uint8_t {
  Helmet = 0,
  Amulet,
  Weapon,
  BodyArmor,
  Cloak,
  Shield,
  Focus,
  Gloves,
  Belt,
  Boots,
  Ring,
  Warcall,
  QuickRig,
  Attendant,
};

inline constexpr std::uint8_t kKindCount =
    static_cast<std::uint8_t>(Kind::Attendant) + 1;

enum class Status : std::uint8_t {
  Ok,
  WrongSlot,
  TwoHandedConflict,
  InvalidItem,
  Empty,
  NotFound,
};

struct Item {
  std::uint32_t id = 0;
  Kind kind = Kind::Helmet;
  bool two_handed = false;

  [[nodiscard]] constexpr bool valid() const {
    return id != 0 && static_cast<std::uint8_t>(kind) < kKindCount;
  }
};

struct Equipped {
  std::uint32_t id = 0;
  Kind kind = Kind::Helmet;
  bool two_handed = false;

  [[nodiscard]] constexpr bool empty() const { return id == 0; }

  [[nodiscard]] constexpr bool operator==(const Equipped&) const = default;
};

struct State {
  std::array<Equipped, kSlotCount> slots{};

  [[nodiscard]] constexpr bool valid() const {
    for (const Equipped& e : slots) {
      if (e.id == 0) continue;
      if (static_cast<std::uint8_t>(e.kind) >= kKindCount) return false;
    }
    const Equipped& main = slots[static_cast<std::size_t>(Slot::MainHand)];
    const Equipped& off = slots[static_cast<std::size_t>(Slot::OffHand)];
    if (!main.empty() && main.kind == Kind::Weapon && main.two_handed &&
        !off.empty()) {
      return false;
    }
    return true;
  }

  [[nodiscard]] constexpr bool operator==(const State&) const = default;
};

[[nodiscard]] constexpr const char* name(Slot slot) {
  switch (slot) {
    case Slot::Head:
      return "head";
    case Slot::Amulet:
      return "amulet";
    case Slot::MainHand:
      return "main-hand";
    case Slot::Body:
      return "body";
    case Slot::Cloak:
      return "cloak";
    case Slot::OffHand:
      return "off-hand";
    case Slot::Gloves:
      return "gloves";
    case Slot::Belt:
      return "belt";
    case Slot::Boots:
      return "boots";
    case Slot::Ring1:
      return "ring1";
    case Slot::Ring2:
      return "ring2";
    case Slot::Warhorn:
      return "warhorn";
    case Slot::QuickRig:
      return "quick-rig";
    case Slot::Attendant:
      return "attendant";
  }
  return "unknown-slot";
}

[[nodiscard]] constexpr const char* name(Kind kind) {
  switch (kind) {
    case Kind::Helmet:
      return "helmet";
    case Kind::Amulet:
      return "amulet";
    case Kind::Weapon:
      return "weapon";
    case Kind::BodyArmor:
      return "body-armor";
    case Kind::Cloak:
      return "cloak";
    case Kind::Shield:
      return "shield";
    case Kind::Focus:
      return "focus";
    case Kind::Gloves:
      return "gloves";
    case Kind::Belt:
      return "belt";
    case Kind::Boots:
      return "boots";
    case Kind::Ring:
      return "ring";
    case Kind::Warcall:
      return "warcall";
    case Kind::QuickRig:
      return "quick-rig";
    case Kind::Attendant:
      return "attendant";
  }
  return "unknown-kind";
}

[[nodiscard]] constexpr const char* name(Status status) {
  switch (status) {
    case Status::Ok:
      return "ok";
    case Status::WrongSlot:
      return "wrong-slot";
    case Status::TwoHandedConflict:
      return "two-handed-conflict";
    case Status::InvalidItem:
      return "invalid-item";
    case Status::Empty:
      return "empty";
    case Status::NotFound:
      return "not-found";
  }
  return "unknown-status";
}

[[nodiscard]] constexpr std::size_t slot_index(Slot slot) {
  return static_cast<std::size_t>(slot);
}

[[nodiscard]] constexpr bool kind_fits_slot(Kind kind, Slot slot) {
  switch (kind) {
    case Kind::Helmet:
      return slot == Slot::Head;
    case Kind::Amulet:
      return slot == Slot::Amulet;
    case Kind::Weapon:
      return slot == Slot::MainHand;
    case Kind::BodyArmor:
      return slot == Slot::Body;
    case Kind::Cloak:
      return slot == Slot::Cloak;
    case Kind::Shield:
    case Kind::Focus:
      return slot == Slot::OffHand;
    case Kind::Gloves:
      return slot == Slot::Gloves;
    case Kind::Belt:
      return slot == Slot::Belt;
    case Kind::Boots:
      return slot == Slot::Boots;
    case Kind::Ring:
      return slot == Slot::Ring1 || slot == Slot::Ring2;
    case Kind::Warcall:
      return slot == Slot::Warhorn;
    case Kind::QuickRig:
      return slot == Slot::QuickRig;
    case Kind::Attendant:
      return slot == Slot::Attendant;
  }
  return false;
}

[[nodiscard]] constexpr Slot primary_slot_for(Kind kind) {
  switch (kind) {
    case Kind::Helmet:
      return Slot::Head;
    case Kind::Amulet:
      return Slot::Amulet;
    case Kind::Weapon:
      return Slot::MainHand;
    case Kind::BodyArmor:
      return Slot::Body;
    case Kind::Cloak:
      return Slot::Cloak;
    case Kind::Shield:
    case Kind::Focus:
      return Slot::OffHand;
    case Kind::Gloves:
      return Slot::Gloves;
    case Kind::Belt:
      return Slot::Belt;
    case Kind::Boots:
      return Slot::Boots;
    case Kind::Ring:
      return Slot::Ring1;
    case Kind::Warcall:
      return Slot::Warhorn;
    case Kind::QuickRig:
      return Slot::QuickRig;
    case Kind::Attendant:
      return Slot::Attendant;
  }
  return Slot::Head;
}

[[nodiscard]] constexpr bool main_hand_two_handed(const State& state) {
  const Equipped& main = state.slots[slot_index(Slot::MainHand)];
  return !main.empty() && main.kind == Kind::Weapon && main.two_handed;
}

[[nodiscard]] constexpr bool would_two_hand_conflict(const State& state,
                                                     const Item& item,
                                                     Slot slot) {
  if (item.kind == Kind::Weapon && item.two_handed && slot == Slot::MainHand) {
    return !state.slots[slot_index(Slot::OffHand)].empty();
  }
  if (slot == Slot::OffHand && main_hand_two_handed(state)) {
    return true;
  }
  return false;
}

[[nodiscard]] constexpr Status equip(State& state, const Item& item, Slot slot) {
  if (!item.valid()) return Status::InvalidItem;
  if (!kind_fits_slot(item.kind, slot)) return Status::WrongSlot;
  if (would_two_hand_conflict(state, item, slot)) {
    return Status::TwoHandedConflict;
  }
  Equipped& target = state.slots[slot_index(slot)];
  target.id = item.id;
  target.kind = item.kind;
  target.two_handed = item.kind == Kind::Weapon && item.two_handed;
  return Status::Ok;
}

[[nodiscard]] constexpr Status equip_auto(State& state, const Item& item) {
  if (!item.valid()) return Status::InvalidItem;
  if (item.kind == Kind::Ring) {
    if (state.slots[slot_index(Slot::Ring1)].empty()) {
      return equip(state, item, Slot::Ring1);
    }
    if (state.slots[slot_index(Slot::Ring2)].empty()) {
      return equip(state, item, Slot::Ring2);
    }
    return equip(state, item, Slot::Ring1);
  }
  return equip(state, item, primary_slot_for(item.kind));
}

[[nodiscard]] constexpr Status unequip(State& state, Slot slot) {
  Equipped& target = state.slots[slot_index(slot)];
  if (target.empty()) return Status::Empty;
  target = Equipped{};
  return Status::Ok;
}

[[nodiscard]] constexpr std::uint32_t replaced_id(const State& before,
                                                    const State& after,
                                                    Slot slot) {
  const Equipped& was = before.slots[slot_index(slot)];
  const Equipped& now = after.slots[slot_index(slot)];
  if (was.empty()) return 0;
  if (now.id == was.id) return 0;
  return was.id;
}

// Stable serialization order: ascending slot enum (head → attendant).
[[nodiscard]] constexpr std::array<Slot, kSlotCount> serialization_order() {
  std::array<Slot, kSlotCount> order{};
  for (std::uint8_t i = 0; i < kSlotCount; ++i) {
    order[i] = static_cast<Slot>(i);
  }
  return order;
}

}  // namespace paper_doll
