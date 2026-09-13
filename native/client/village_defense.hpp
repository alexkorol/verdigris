// village_defense.hpp — TASK-0203 village-defense prologue presentation model.
//
// Owner Demo opening crisis: occupation nudge, civilian tool, two pack waves,
// square/well boss, forgiving restart, and first level-up at boss defeat.
#pragma once

#include <cstdint>

namespace village_defense {

inline constexpr const char* kPrologueZoneId = "owner-demo-prologue";
inline constexpr const char* kPalisadeLandmark = "palisade_rampart";
inline constexpr const char* kBossLandmark = "square_well";
inline constexpr const char* kCivilianToolId = "wooden_club";
inline constexpr std::uint8_t kPackWavesBeforeBoss = 2;
inline constexpr std::uint8_t kVictoryPlayerLevel = 2;

enum class Occupation : std::uint8_t {
  Unchosen = 0,
  FieldHand,
  Scout,
  Scribe,
};

enum class Phase : std::uint8_t {
  HouseNamed = 0,
  OccupationChosen,
  CrisisActive,
  PackWave,
  BossSquareWell,
  VictoryLevelUp,
};

enum class Status : std::uint8_t {
  Ok,
  InvalidTransition,
  InvalidOccupation,
  BossNotReady,
  AlreadyVictorious,
};

struct AttributeNudge {
  std::int8_t strength = 0;
  std::int8_t dexterity = 0;
  std::int8_t intelligence = 0;

  [[nodiscard]] constexpr bool operator==(const AttributeNudge&) const =
      default;
};

struct State {
  Phase phase = Phase::HouseNamed;
  Occupation occupation = Occupation::Unchosen;
  AttributeNudge nudge{};
  std::uint8_t packs_cleared = 0;
  std::uint8_t player_level = 1;
  std::uint8_t restart_count = 0;
  bool tool_equipped = false;
  bool boss_defeated = false;

  [[nodiscard]] constexpr bool operator==(const State&) const = default;
};

[[nodiscard]] constexpr const char* status_name(Status status) {
  switch (status) {
    case Status::Ok:
      return "Ok";
    case Status::InvalidTransition:
      return "InvalidTransition";
    case Status::InvalidOccupation:
      return "InvalidOccupation";
    case Status::BossNotReady:
      return "BossNotReady";
    case Status::AlreadyVictorious:
      return "AlreadyVictorious";
  }
  return "Unknown";
}

[[nodiscard]] constexpr const char* occupation_name(Occupation occupation) {
  switch (occupation) {
    case Occupation::Unchosen:
      return "unchosen";
    case Occupation::FieldHand:
      return "field_hand";
    case Occupation::Scout:
      return "scout";
    case Occupation::Scribe:
      return "scribe";
  }
  return "unknown";
}

[[nodiscard]] constexpr AttributeNudge nudge_for(Occupation occupation) {
  switch (occupation) {
    case Occupation::FieldHand:
      return {1, 0, 0};
    case Occupation::Scout:
      return {0, 1, 0};
    case Occupation::Scribe:
      return {0, 0, 1};
    case Occupation::Unchosen:
      return {};
  }
  return {};
}

[[nodiscard]] constexpr State make_owner_demo_prologue() {
  State s{};
  s.phase = Phase::HouseNamed;
  return s;
}

[[nodiscard]] constexpr Status choose_occupation(State& state,
                                                 Occupation occupation) {
  if (occupation == Occupation::Unchosen) {
    return Status::InvalidOccupation;
  }
  if (state.phase != Phase::HouseNamed) {
    return Status::InvalidTransition;
  }
  state.occupation = occupation;
  state.nudge = nudge_for(occupation);
  state.phase = Phase::OccupationChosen;
  return Status::Ok;
}

[[nodiscard]] constexpr Status begin_crisis(State& state) {
  if (state.phase != Phase::OccupationChosen) {
    return Status::InvalidTransition;
  }
  state.phase = Phase::CrisisActive;
  return Status::Ok;
}

[[nodiscard]] constexpr Status equip_civilian_tool(State& state) {
  if (state.phase != Phase::CrisisActive && state.phase != Phase::PackWave) {
    return Status::InvalidTransition;
  }
  state.tool_equipped = true;
  if (state.phase == Phase::CrisisActive) {
    state.phase = Phase::PackWave;
  }
  return Status::Ok;
}

[[nodiscard]] constexpr Status clear_pack_wave(State& state) {
  if (state.phase != Phase::PackWave) {
    return Status::InvalidTransition;
  }
  if (!state.tool_equipped) {
    return Status::InvalidTransition;
  }
  state.packs_cleared =
      static_cast<std::uint8_t>(state.packs_cleared + 1);
  if (state.packs_cleared >= kPackWavesBeforeBoss) {
    state.phase = Phase::BossSquareWell;
  }
  return Status::Ok;
}

[[nodiscard]] constexpr Status defeat_boss(State& state) {
  if (state.boss_defeated) {
    return Status::AlreadyVictorious;
  }
  if (state.phase != Phase::BossSquareWell) {
    return Status::BossNotReady;
  }
  state.boss_defeated = true;
  state.player_level = kVictoryPlayerLevel;
  state.phase = Phase::VictoryLevelUp;
  return Status::Ok;
}

[[nodiscard]] constexpr Status forgiving_restart(State& state) {
  if (state.phase == Phase::VictoryLevelUp) {
    return Status::InvalidTransition;
  }
  state.packs_cleared = 0;
  state.boss_defeated = false;
  state.player_level = 1;
  state.tool_equipped = false;
  state.phase = Phase::CrisisActive;
  state.restart_count =
      static_cast<std::uint8_t>(state.restart_count + 1);
  return Status::Ok;
}

[[nodiscard]] constexpr bool ready_for_skill_tree(const State& state) {
  return state.phase == Phase::VictoryLevelUp &&
         state.player_level == kVictoryPlayerLevel && state.boss_defeated;
}

}  // namespace village_defense
