// zone_runtime_bridge.hpp — TASK-0192 prep: multi-zone runtime bridge.
//
// Composes cartographer_adapter zone seeds with instance_refresh policies and
// gate_interaction worlds for Owner Demo multi-zone runtime. No core.cpp in
// this packet.
#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include "../content/cartographer_adapter.hpp"
#include "gate_interaction.hpp"
#include "instance_refresh.hpp"

namespace zone_runtime_bridge {

struct ZoneRuntimeEntry {
  std::uint32_t zone_id = 0;
  cartographer_adapter::MapGenPlan mapgen{};
  instance_refresh::ZonePolicy policy{};
  gate_interaction::World gates{};
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const ZoneRuntimeEntry&) const = default;
};

struct RuntimePlan {
  std::array<ZoneRuntimeEntry, cartographer_adapter::kOwnerDemoZoneCount> zones{};
  std::uint8_t count = 0;
  bool valid = false;

  [[nodiscard]] constexpr bool operator==(const RuntimePlan&) const = default;
};

inline constexpr std::uint16_t kTilePixels = 16;
inline constexpr std::uint16_t kGateRadius = 24;

inline constexpr std::array<std::uint32_t, 1> kPrologueGateIds = {1u};
inline constexpr std::array<std::uint32_t, 2> kCrossroadsGateIds = {10u, 11u};
inline constexpr std::array<std::uint32_t, 3> kThornwardGateIds = {20u, 21u, 22u};
inline constexpr std::array<std::uint32_t, 2> kRiftGateIds = {30u, 31u};
inline constexpr std::array<std::uint32_t, 1> kGlimmerGateIds = {40u};

[[nodiscard]] constexpr std::int16_t clamp_i16(std::int32_t value) {
  if (value < static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::min())) {
    return std::numeric_limits<std::int16_t>::min();
  }
  if (value > static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::max())) {
    return std::numeric_limits<std::int16_t>::max();
  }
  return static_cast<std::int16_t>(value);
}

[[nodiscard]] constexpr std::uint32_t zone_id_for(const char* id) {
  return cartographer_adapter::combine_seed(id, 0u);
}

[[nodiscard]] constexpr instance_refresh::ZonePolicy owner_demo_policy(
    std::uint8_t zone_index) {
  instance_refresh::ZonePolicy policy;
  policy.zone_id =
      zone_id_for(cartographer_adapter::kOwnerDemoZones[zone_index].id);
  switch (zone_index) {
    case 0:
      policy.kind = instance_refresh::ZoneKind::Combat;
      policy.allow_fresh = false;
      policy.lifetime_ticks = 0;
      break;
    case 1:
      policy.kind = instance_refresh::ZoneKind::Town;
      policy.allow_fresh = false;
      policy.lifetime_ticks = 0;
      break;
    case 2:
      policy.kind = instance_refresh::ZoneKind::Combat;
      policy.allow_fresh = true;
      policy.lifetime_ticks = 36000;
      break;
    case 3:
      policy.kind = instance_refresh::ZoneKind::Combat;
      policy.allow_fresh = true;
      policy.lifetime_ticks = 48000;
      break;
    case 4:
      policy.kind = instance_refresh::ZoneKind::Combat;
      policy.allow_fresh = true;
      policy.lifetime_ticks = 18000;
      break;
    default:
      policy.zone_id = 0;
      break;
  }
  return policy;
}

[[nodiscard]] constexpr const std::uint32_t* gate_ids_for(std::uint8_t zone_index,
                                                        std::uint8_t& count) {
  switch (zone_index) {
    case 0:
      count = static_cast<std::uint8_t>(kPrologueGateIds.size());
      return kPrologueGateIds.data();
    case 1:
      count = static_cast<std::uint8_t>(kCrossroadsGateIds.size());
      return kCrossroadsGateIds.data();
    case 2:
      count = static_cast<std::uint8_t>(kThornwardGateIds.size());
      return kThornwardGateIds.data();
    case 3:
      count = static_cast<std::uint8_t>(kRiftGateIds.size());
      return kRiftGateIds.data();
    case 4:
      count = static_cast<std::uint8_t>(kGlimmerGateIds.size());
      return kGlimmerGateIds.data();
    default:
      count = 0;
      return nullptr;
  }
}

[[nodiscard]] constexpr gate_interaction::World build_gate_world(
    std::uint8_t zone_index) {
  gate_interaction::World world;
  if (zone_index >= cartographer_adapter::kOwnerDemoZoneCount) {
    return world;
  }
  const cartographer_adapter::ZoneSeed& zone =
      cartographer_adapter::kOwnerDemoZones[zone_index];
  std::uint8_t id_count = 0;
  const std::uint32_t* ids = gate_ids_for(zone_index, id_count);
  if (ids == nullptr || id_count != zone.gate_count) {
    return world;
  }

  for (std::uint8_t i = 0; i < zone.gate_count; ++i) {
    const cartographer_adapter::GateAnchor& anchor = zone.gates[i];
    gate_interaction::GateDef gate;
    gate.id = ids[i];
    gate.destination_zone = zone_id_for(anchor.to_zone);
    gate.center = {
        clamp_i16(static_cast<std::int32_t>(anchor.x) *
                      static_cast<std::int32_t>(kTilePixels) +
                  static_cast<std::int32_t>(kTilePixels) / 2),
        clamp_i16(static_cast<std::int32_t>(anchor.y) *
                      static_cast<std::int32_t>(kTilePixels) +
                  static_cast<std::int32_t>(kTilePixels) / 2),
    };
    gate.radius = kGateRadius;
    gate.accessible = true;
    world.gates[static_cast<std::size_t>(world.count)] = gate;
    ++world.count;
  }
  return world;
}

[[nodiscard]] constexpr RuntimePlan build_owner_demo_runtime() {
  RuntimePlan plan;
  if (cartographer_adapter::validate_owner_demo_graph() !=
      cartographer_adapter::Status::Ok) {
    return plan;
  }

  for (std::uint8_t i = 0;
       i < cartographer_adapter::kOwnerDemoZoneCount; ++i) {
    ZoneRuntimeEntry entry;
    entry.zone_id = zone_id_for(cartographer_adapter::kOwnerDemoZones[i].id);
    entry.policy = owner_demo_policy(i);
    const cartographer_adapter::Status map_status =
        cartographer_adapter::build_mapgen_plan(
            cartographer_adapter::kOwnerDemoZones[i], entry.mapgen);
    entry.gates = build_gate_world(i);
    entry.valid =
        map_status == cartographer_adapter::Status::Ok &&
        entry.policy.valid() && entry.gates.count > 0;
    if (!entry.valid) {
      return plan;
    }
    plan.zones[static_cast<std::size_t>(plan.count)] = entry;
    ++plan.count;
  }

  plan.valid = plan.count == cartographer_adapter::kOwnerDemoZoneCount;
  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const RuntimePlan& plan) {
  std::uint32_t hash = plan.valid ? 1u : 0u;
  hash ^= static_cast<std::uint32_t>(plan.count) * 17u;
  for (std::uint8_t i = 0; i < plan.count; ++i) {
    const ZoneRuntimeEntry& entry = plan.zones[static_cast<std::size_t>(i)];
    hash ^= entry.zone_id * 31u;
    hash ^= entry.mapgen.seed * 7u;
    hash ^= static_cast<std::uint32_t>(entry.gates.count) * 13u;
    hash ^= entry.policy.allow_fresh ? 3u : 0u;
  }
  return hash;
}

}  // namespace zone_runtime_bridge
