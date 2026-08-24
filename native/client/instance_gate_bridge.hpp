// instance_gate_bridge.hpp — TASK-0189 prep: gate click + instance refresh bridge.
//
// Composes gate_interaction activation with instance_refresh policy so
// integrators can route EnterZone vs Ctrl-click fresh-instance in one step.
// No networking.cpp or main.cpp in this packet.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "gate_interaction.hpp"
#include "instance_refresh.hpp"

namespace instance_gate_bridge {

inline constexpr std::size_t kMaxZones = 8;

struct ZoneTable {
  std::array<instance_refresh::ZonePolicy, kMaxZones> zones{};
  std::uint8_t count = 0;
};

struct ResolveResult {
  gate_interaction::Decision gate{};
  instance_refresh::Decision instance{};
  bool issued_travel = false;

  [[nodiscard]] constexpr bool operator==(const ResolveResult&) const = default;
};

[[nodiscard]] constexpr const instance_refresh::ZonePolicy* find_zone(
    const ZoneTable& table, std::uint32_t zone_id) {
  for (std::uint8_t i = 0; i < table.count; ++i) {
    const instance_refresh::ZonePolicy& zone =
        table.zones[static_cast<std::size_t>(i)];
    if (zone.zone_id == zone_id) {
      return &zone;
    }
  }
  return nullptr;
}

[[nodiscard]] constexpr instance_refresh::Request request_for_command(
    gate_interaction::Command command) {
  return command == gate_interaction::Command::FreshInstance
             ? instance_refresh::Request::FreshInstance
             : instance_refresh::Request::Reuse;
}

[[nodiscard]] constexpr bool travel_allowed(
    const instance_refresh::Decision& instance) {
  return instance.status == instance_refresh::Status::Ok &&
         (instance.outcome == instance_refresh::Outcome::ReuseExisting ||
          instance.outcome == instance_refresh::Outcome::CreateFresh);
}

[[nodiscard]] constexpr ResolveResult resolve_activation(
    const gate_interaction::Decision& gate_decision,
    instance_refresh::Registry& registry, const ZoneTable& zones) {
  ResolveResult result;
  result.gate = gate_decision;
  if (gate_decision.status != gate_interaction::Status::Ok ||
      gate_decision.command == gate_interaction::Command::None) {
    return result;
  }

  const instance_refresh::ZonePolicy* zone =
      find_zone(zones, gate_decision.destination_zone);
  if (zone == nullptr) {
    result.instance.status = instance_refresh::Status::Invalid;
    return result;
  }

  result.instance = instance_refresh::evaluate(
      registry, *zone,
      request_for_command(gate_decision.command));
  result.issued_travel = travel_allowed(result.instance);
  return result;
}

[[nodiscard]] constexpr ResolveResult activate_gate(
    const gate_interaction::World& world, const gate_interaction::State& state,
    bool ctrl_held, instance_refresh::Registry& registry,
    const ZoneTable& zones) {
  const gate_interaction::Decision gate_decision =
      gate_interaction::activate(world, state, ctrl_held);
  return resolve_activation(gate_decision, registry, zones);
}

}  // namespace instance_gate_bridge
