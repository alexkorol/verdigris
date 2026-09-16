// village_defense_bridge.hpp — TASK-0203 prep: prologue defense bridge.
//
// Composes village_defense phase transitions with zone runtime and skill-tree
// unlock. No core.cpp in this packet.
#pragma once

#include <cstdint>

#include "geometric_skill_tree.hpp"
#include "village_defense.hpp"
#include "zone_runtime_bridge.hpp"

namespace village_defense_bridge {

inline constexpr std::uint8_t kPrologueZoneIndex = 0;

struct BossVictoryResult {
  village_defense::Status defense_status = village_defense::Status::Ok;
  geometric_skill_tree::Status tree_status = geometric_skill_tree::Status::Ok;
  bool skill_tree_unlocked = false;

  [[nodiscard]] constexpr bool operator==(const BossVictoryResult&) const =
      default;
};

[[nodiscard]] constexpr bool prologue_combat_active(
    const village_defense::State& state) {
  return state.phase == village_defense::Phase::PackWave ||
         state.phase == village_defense::Phase::BossSquareWell;
}

[[nodiscard]] constexpr const zone_runtime_bridge::ZoneRuntimeEntry&
prologue_zone(const zone_runtime_bridge::RuntimePlan& plan) {
  return plan.zones[static_cast<std::size_t>(kPrologueZoneIndex)];
}

[[nodiscard]] constexpr BossVictoryResult on_boss_defeated(
    village_defense::State& defense,
    geometric_skill_tree::State& skill_tree) {
  BossVictoryResult result;
  result.defense_status = village_defense::defeat_boss(defense);
  if (result.defense_status != village_defense::Status::Ok) {
    return result;
  }
  if (!village_defense::ready_for_skill_tree(defense)) {
    return result;
  }
  result.tree_status =
      geometric_skill_tree::on_player_level(skill_tree, defense.player_level);
  result.skill_tree_unlocked = skill_tree.tree_unlocked;
  return result;
}

[[nodiscard]] constexpr village_defense::Status run_prologue_beat(
    village_defense::State& defense, village_defense::Occupation occupation) {
  village_defense::Status status =
      village_defense::choose_occupation(defense, occupation);
  if (status != village_defense::Status::Ok) {
    return status;
  }
  status = village_defense::begin_crisis(defense);
  if (status != village_defense::Status::Ok) {
    return status;
  }
  status = village_defense::equip_civilian_tool(defense);
  if (status != village_defense::Status::Ok) {
    return status;
  }
  for (std::uint8_t wave = 0; wave < village_defense::kPackWavesBeforeBoss;
       ++wave) {
    status = village_defense::clear_pack_wave(defense);
    if (status != village_defense::Status::Ok) {
      return status;
    }
  }
  return village_defense::Status::Ok;
}

}  // namespace village_defense_bridge
