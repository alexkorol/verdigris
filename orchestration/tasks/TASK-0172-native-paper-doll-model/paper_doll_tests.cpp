// paper_doll_tests.cpp — TASK-0172 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "paper_doll.hpp"

using namespace paper_doll;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

Item make_item(std::uint32_t id, Kind kind, bool two_handed = false) {
  Item item;
  item.id = id;
  item.kind = kind;
  item.two_handed = two_handed;
  return item;
}

void expect_status(Status got, Status expected, const std::string& label) {
  check(got == expected,
        label + ": expected " + name(expected) + " got " + name(got));
}

void test_equip_and_wrong_slot() {
  State s;
  expect_status(equip(s, make_item(1, Kind::Helmet), Slot::Head), Status::Ok,
                "helmet to head");
  expect_status(equip(s, make_item(2, Kind::Boots), Slot::Head),
                Status::WrongSlot, "reject boots on head");
  check(s.valid(), "state valid after equip");
}

void test_replace() {
  State before;
  expect_status(equip(before, make_item(10, Kind::Amulet), Slot::Amulet),
                Status::Ok, "first amulet");
  State after = before;
  expect_status(equip(after, make_item(11, Kind::Amulet), Slot::Amulet),
                Status::Ok, "replace amulet");
  check(replaced_id(before, after, Slot::Amulet) == 10, "replaced id tracked");
  check(after.slots[slot_index(Slot::Amulet)].id == 11, "new id in slot");
}

void test_two_handed_conflict() {
  State s;
  expect_status(equip(s, make_item(20, Kind::Focus), Slot::OffHand), Status::Ok,
                "focus off-hand");
  expect_status(equip(s, make_item(21, Kind::Weapon, true), Slot::MainHand),
                Status::TwoHandedConflict, "reject 2h with off-hand occupied");

  State s2;
  expect_status(equip(s2, make_item(22, Kind::Weapon, true), Slot::MainHand),
                Status::Ok, "2h main alone");
  expect_status(equip(s2, make_item(23, Kind::Shield), Slot::OffHand),
                Status::TwoHandedConflict, "reject off-hand under 2h");
}

void test_unequip_empty() {
  State s;
  expect_status(unequip(s, Slot::Belt), Status::Empty, "unequip empty belt");
  expect_status(equip(s, make_item(30, Kind::Belt), Slot::Belt), Status::Ok,
                "equip belt");
  expect_status(unequip(s, Slot::Belt), Status::Ok, "unequip belt");
  check(s.slots[slot_index(Slot::Belt)].empty(), "belt empty");
}

void test_ring_auto_and_replace() {
  State s;
  expect_status(equip_auto(s, make_item(40, Kind::Ring)), Status::Ok,
                "ring auto ring1");
  check(s.slots[slot_index(Slot::Ring1)].id == 40, "ring1 filled");
  expect_status(equip_auto(s, make_item(41, Kind::Ring)), Status::Ok,
                "second ring");
  check(s.slots[slot_index(Slot::Ring2)].id == 41, "ring2 filled");
  expect_status(equip_auto(s, make_item(42, Kind::Ring)), Status::Ok,
                "replace ring1");
  check(s.slots[slot_index(Slot::Ring1)].id == 42, "ring1 replaced");
}

void test_accessory_slots() {
  State s;
  expect_status(equip(s, make_item(50, Kind::Warcall), Slot::Warhorn), Status::Ok,
                "warcall");
  expect_status(equip(s, make_item(51, Kind::QuickRig), Slot::QuickRig),
                Status::Ok, "quickrig");
  expect_status(equip(s, make_item(52, Kind::Attendant), Slot::Attendant),
                Status::Ok, "attendant");
}

void test_serialization_order_stable() {
  const auto order = serialization_order();
  for (std::uint8_t i = 0; i < kSlotCount; ++i) {
    check(static_cast<std::uint8_t>(order[i]) == i,
          "serialization slot order stable");
  }
}

void test_invalid_item_negative() {
  State s;
  Item bad;
  expect_status(equip(s, bad, Slot::Head), Status::InvalidItem, "id 0 rejected");
}

}  // namespace

int main() {
  test_equip_and_wrong_slot();
  test_replace();
  test_two_handed_conflict();
  test_unequip_empty();
  test_ring_auto_and_replace();
  test_accessory_slots();
  test_serialization_order_stable();
  test_invalid_item_negative();

  std::cout << "TASK-0172 paper doll acceptance: " << g_checks << " checks passed\n";
  return 0;
}
