// town_runtime_layout.hpp — TASK-0190 prep: Owner Demo town layout planner.
//
// Maps owner_demo_town.json tile anchors to map-pixel interaction bounds and
// composes gate_interaction worlds for readable exits. No core.cpp or runtime
// load in this packet.
#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include "gate_interaction.hpp"
#include "instance_refresh.hpp"

namespace town_runtime_layout {

struct Viewport {
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

struct PixelRect {
  std::int16_t x = 0;
  std::int16_t y = 0;
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

struct TileCoord {
  std::int16_t x = 0;
  std::int16_t y = 0;

  [[nodiscard]] constexpr bool operator==(const TileCoord&) const = default;
};

enum class NpcRole : std::uint8_t {
  Elder = 0,
  WeaponsToolsTrainer,
  ArmorRitualMerchant,
  Steward,
};

struct NpcAnchor {
  NpcRole role = NpcRole::Elder;
  TileCoord tile{};
  PixelRect interact{};
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const NpcAnchor&) const = default;
};

struct ExitGate {
  std::uint32_t gate_id = 0;
  std::uint32_t destination_zone = 0;
  TileCoord tile{};
  PixelRect interact{};
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const ExitGate&) const = default;
};

struct TownLayoutPlan {
  std::array<NpcAnchor, 4> npcs{};
  std::uint8_t npc_count = 0;
  std::array<ExitGate, 2> exits{};
  std::uint8_t exit_count = 0;
  bool crisis_banner = true;
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const TownLayoutPlan&) const = default;
};

inline constexpr std::uint16_t kTilePixels = 16;
inline constexpr std::uint16_t kInteractPad = 8;
inline constexpr std::uint16_t kGateRadius = 24;

// Matches cartographer_adapter::combine_seed(id, 0).
inline constexpr std::uint32_t kZoneCrossroads = 1886746992u;
inline constexpr std::uint32_t kZoneThornward = 2454248526u;
inline constexpr std::uint32_t kZoneRiftHollow = 678091740u;

[[nodiscard]] constexpr std::int16_t clamp_i16(std::int32_t value) {
  if (value < static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::min())) {
    return std::numeric_limits<std::int16_t>::min();
  }
  if (value > static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::max())) {
    return std::numeric_limits<std::int16_t>::max();
  }
  return static_cast<std::int16_t>(value);
}

[[nodiscard]] constexpr std::uint16_t clamp_u16(std::int32_t value) {
  if (value <= 0) return 0;
  if (value > static_cast<std::int32_t>(std::numeric_limits<std::uint16_t>::max())) {
    return std::numeric_limits<std::uint16_t>::max();
  }
  return static_cast<std::uint16_t>(value);
}

[[nodiscard]] constexpr PixelRect tile_interact_rect(TileCoord tile) {
  const std::int32_t cx =
      static_cast<std::int32_t>(tile.x) * static_cast<std::int32_t>(kTilePixels) +
      static_cast<std::int32_t>(kTilePixels) / 2;
  const std::int32_t cy =
      static_cast<std::int32_t>(tile.y) * static_cast<std::int32_t>(kTilePixels) +
      static_cast<std::int32_t>(kTilePixels) / 2;
  const std::int32_t half =
      static_cast<std::int32_t>(kTilePixels) / 2 +
      static_cast<std::int32_t>(kInteractPad);
  PixelRect out;
  out.x = clamp_i16(cx - half);
  out.y = clamp_i16(cy - half);
  out.width = clamp_u16(half * 2);
  out.height = out.width;
  return out;
}

[[nodiscard]] constexpr gate_interaction::Point map_pixel_center(TileCoord tile) {
  return {
      clamp_i16(static_cast<std::int32_t>(tile.x) *
                    static_cast<std::int32_t>(kTilePixels) +
                static_cast<std::int32_t>(kTilePixels) / 2),
      clamp_i16(static_cast<std::int32_t>(tile.y) *
                    static_cast<std::int32_t>(kTilePixels) +
                static_cast<std::int32_t>(kTilePixels) / 2),
  };
}

constexpr void copy_label(std::array<char, gate_interaction::kLabelCapacity>& dest,
                          const char* src) {
  for (std::size_t i = 0; i < gate_interaction::kLabelCapacity; ++i) {
    dest[i] = (src != nullptr && src[i] != '\0') ? src[i] : '\0';
    if (src == nullptr || src[i] == '\0') break;
  }
}

[[nodiscard]] constexpr TownLayoutPlan plan_crossroads(const Viewport& vp) {
  TownLayoutPlan plan;
  if (!vp.valid()) return plan;

  const NpcAnchor elder{
      NpcRole::Elder, {34, 116}, tile_interact_rect({34, 116}), true};
  const NpcAnchor weapons{
      NpcRole::WeaponsToolsTrainer, {19, 113}, tile_interact_rect({19, 113}), true};
  const NpcAnchor armor{
      NpcRole::ArmorRitualMerchant, {45, 108}, tile_interact_rect({45, 108}), true};
  const NpcAnchor steward{
      NpcRole::Steward, {31, 121}, tile_interact_rect({31, 121}), true};

  plan.npcs[0] = elder;
  plan.npcs[1] = weapons;
  plan.npcs[2] = armor;
  plan.npcs[3] = steward;
  plan.npc_count = 4;

  ExitGate thornward;
  thornward.gate_id = 10u;
  thornward.destination_zone = kZoneThornward;
  thornward.tile = {38, 95};
  thornward.interact = tile_interact_rect(thornward.tile);
  thornward.valid = true;

  ExitGate rift;
  rift.gate_id = 11u;
  rift.destination_zone = kZoneRiftHollow;
  rift.tile = {52, 110};
  rift.interact = tile_interact_rect(rift.tile);
  rift.valid = true;

  plan.exits[0] = thornward;
  plan.exits[1] = rift;
  plan.exit_count = 2;
  plan.crisis_banner = true;
  plan.valid = true;
  return plan;
}

[[nodiscard]] constexpr gate_interaction::World build_exit_world(
    const TownLayoutPlan& plan) {
  gate_interaction::World world;
  for (std::uint8_t i = 0; i < plan.exit_count; ++i) {
    const ExitGate& exit = plan.exits[static_cast<std::size_t>(i)];
    if (!exit.valid) continue;
    gate_interaction::GateDef gate;
    gate.id = exit.gate_id;
    gate.destination_zone = exit.destination_zone;
    gate.center = map_pixel_center(exit.tile);
    gate.radius = kGateRadius;
    gate.accessible = true;
    if (exit.gate_id == 10u) {
      copy_label(gate.label, "Thornward Approach");
    } else if (exit.gate_id == 11u) {
      copy_label(gate.label, "Rift Hollow Trail");
    }
    world.gates[static_cast<std::size_t>(world.count)] = gate;
    ++world.count;
  }
  return world;
}

[[nodiscard]] constexpr instance_refresh::ZonePolicy crossroads_town_policy() {
  instance_refresh::ZonePolicy zone;
  zone.zone_id = kZoneCrossroads;
  zone.kind = instance_refresh::ZoneKind::Town;
  zone.allow_fresh = false;
  zone.lifetime_ticks = 0;
  return zone;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const TownLayoutPlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.npc_count) * 17u;
  hash ^= static_cast<std::uint32_t>(plan.exit_count) * 31u;
  hash ^= plan.crisis_banner ? 3u : 0u;
  for (std::uint8_t i = 0; i < plan.npc_count; ++i) {
    const NpcAnchor& npc = plan.npcs[static_cast<std::size_t>(i)];
    hash ^= static_cast<std::uint32_t>(npc.tile.x) * 7u;
    hash ^= static_cast<std::uint32_t>(npc.tile.y) * 13u;
  }
  for (std::uint8_t i = 0; i < plan.exit_count; ++i) {
    hash ^= plan.exits[static_cast<std::size_t>(i)].gate_id * 19u;
  }
  return hash;
}

}  // namespace town_runtime_layout
