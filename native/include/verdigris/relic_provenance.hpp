// relic_provenance.hpp — TASK-0202 bounded recoverable item provenance model.
//
// Seeds Owner Demo heirloom circulation without a second live player: one
// fallen Scion, crypt queue, ground circulation, and successor recovery.
#pragma once

#include <cstdint>

namespace verdigris {

enum class RelicQueueState : std::uint8_t {
  None = 0,
  FallenEquipped,
  QueuedInCrypt,
  Circulating,
  Recovered,
};

enum class RelicStatus : std::uint8_t {
  Ok,
  InvalidTransition,
  ProvenanceMismatch,
  AlreadyRecovered,
  NotCirculating,
};

struct RelicSeed {
  const char* record_id = nullptr;
  const char* source_scion_id = nullptr;
  const char* source_scion_name = nullptr;
  const char* item_catalog_id = nullptr;
  const char* item_uuid = nullptr;
  std::uint16_t vessel_item_level = 0;

  [[nodiscard]] constexpr bool valid() const {
    return record_id != nullptr && source_scion_id != nullptr &&
           source_scion_name != nullptr && item_catalog_id != nullptr &&
           item_uuid != nullptr && vessel_item_level > 0;
  }

  [[nodiscard]] constexpr bool operator==(const RelicSeed&) const = default;
};

struct GroundProvenance {
  const char* relic_record_id = nullptr;
  const char* relic_source_scion_id = nullptr;
  const char* relic_source_scion_name = nullptr;

  [[nodiscard]] constexpr bool operator==(const GroundProvenance&) const =
      default;
};

struct RelicCirculation {
  RelicQueueState state = RelicQueueState::None;
  RelicSeed seed{};
  GroundProvenance ground{};

  [[nodiscard]] constexpr bool operator==(const RelicCirculation&) const =
      default;
};

inline constexpr const char* kOwnerDemoFallenRecordId = "chron-demo-fallen-001";
inline constexpr const char* kOwnerDemoFallenScionId = "scion-chron-demo";
inline constexpr const char* kOwnerDemoFallenScionName = "Orun the First";
inline constexpr const char* kOwnerDemoHeirloomCatalogId = "steel-battleaxe";
inline constexpr const char* kOwnerDemoHeirloomUuid =
    "ce30de43-7dad-4f84-a2e6-4760677ed882";
inline constexpr std::uint16_t kOwnerDemoHeirloomItemLevel = 50;

[[nodiscard]] constexpr const char* relic_status_name(RelicStatus status) {
  switch (status) {
    case RelicStatus::Ok:
      return "Ok";
    case RelicStatus::InvalidTransition:
      return "InvalidTransition";
    case RelicStatus::ProvenanceMismatch:
      return "ProvenanceMismatch";
    case RelicStatus::AlreadyRecovered:
      return "AlreadyRecovered";
    case RelicStatus::NotCirculating:
      return "NotCirculating";
  }
  return "Unknown";
}

[[nodiscard]] constexpr RelicSeed make_owner_demo_heirloom_seed() {
  return RelicSeed{
      kOwnerDemoFallenRecordId,
      kOwnerDemoFallenScionId,
      kOwnerDemoFallenScionName,
      kOwnerDemoHeirloomCatalogId,
      kOwnerDemoHeirloomUuid,
      kOwnerDemoHeirloomItemLevel,
  };
}

[[nodiscard]] constexpr RelicCirculation make_owner_demo_circulation() {
  RelicCirculation flow{};
  flow.seed = make_owner_demo_heirloom_seed();
  return flow;
}

[[nodiscard]] constexpr bool string_eq(const char* a, const char* b) {
  if (a == nullptr || b == nullptr) {
    return a == b;
  }
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {
      return false;
    }
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

[[nodiscard]] constexpr RelicStatus mark_fallen_equipped(RelicCirculation& flow) {
  if (!flow.seed.valid()) {
    return RelicStatus::InvalidTransition;
  }
  if (flow.state != RelicQueueState::None) {
    return RelicStatus::InvalidTransition;
  }
  flow.state = RelicQueueState::FallenEquipped;
  return RelicStatus::Ok;
}

[[nodiscard]] constexpr RelicStatus entomb_to_crypt(RelicCirculation& flow) {
  if (flow.state != RelicQueueState::FallenEquipped) {
    return RelicStatus::InvalidTransition;
  }
  flow.state = RelicQueueState::QueuedInCrypt;
  return RelicStatus::Ok;
}

[[nodiscard]] constexpr RelicStatus release_circulating_relic(
    RelicCirculation& flow) {
  if (flow.state != RelicQueueState::QueuedInCrypt) {
    return RelicStatus::InvalidTransition;
  }
  flow.ground = GroundProvenance{
      flow.seed.record_id,
      flow.seed.source_scion_id,
      flow.seed.source_scion_name,
  };
  flow.state = RelicQueueState::Circulating;
  return RelicStatus::Ok;
}

[[nodiscard]] constexpr bool ground_matches_seed(
    const GroundProvenance& ground, const RelicSeed& seed) {
  return string_eq(ground.relic_record_id, seed.record_id) &&
         string_eq(ground.relic_source_scion_id, seed.source_scion_id) &&
         string_eq(ground.relic_source_scion_name, seed.source_scion_name);
}

[[nodiscard]] constexpr RelicStatus recover_from_ground(
    RelicCirculation& flow, const char* taken_uuid) {
  if (flow.state == RelicQueueState::Recovered) {
    return RelicStatus::AlreadyRecovered;
  }
  if (flow.state != RelicQueueState::Circulating) {
    return RelicStatus::NotCirculating;
  }
  if (!string_eq(taken_uuid, flow.seed.item_uuid)) {
    return RelicStatus::ProvenanceMismatch;
  }
  if (!ground_matches_seed(flow.ground, flow.seed)) {
    return RelicStatus::ProvenanceMismatch;
  }
  flow.state = RelicQueueState::Recovered;
  return RelicStatus::Ok;
}

[[nodiscard]] constexpr bool demo_recovery_complete(
    const RelicCirculation& flow) {
  return flow.state == RelicQueueState::Recovered && flow.seed.valid();
}

}  // namespace verdigris
