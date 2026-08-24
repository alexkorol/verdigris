// cartographer_adapter.hpp — TASK-0191 WIZARD Cartographer seed → native layout plan.
//
// Translates Owner Demo zone seeds (template, layout, theme, numeric seed) into
// deterministic MapGen parameters with connectivity and spawn guarantees.
// Gate anchors from content JSON are world-space exit markers; MapGen entrance,
// exit, and spawn slots are generator-owned.
#pragma once

#include <cstdint>

namespace cartographer_adapter {

inline constexpr std::uint16_t kDefaultWidth = 72;
inline constexpr std::uint16_t kDefaultHeight = 54;
inline constexpr std::uint8_t kMaxGatesPerZone = 8;
inline constexpr std::uint8_t kOwnerDemoZoneCount = 5;

enum class Status : std::uint8_t {
  Ok,
  InvalidTemplate,
  InvalidLayout,
  InvalidTheme,
  InvalidZoneRef,
  OutOfBounds,
};

struct GateAnchor {
  std::int16_t x = 0;
  std::int16_t y = 0;
  const char* to_zone = nullptr;
  bool optional_branch = false;
};

struct ZoneSeed {
  const char* id = nullptr;
  const char* template_id = nullptr;
  const char* layout = nullptr;
  const char* theme = nullptr;
  std::uint32_t seed = 0;
  std::uint8_t gate_count = 0;
  const GateAnchor* gates = nullptr;
};

struct MapGenPlan {
  std::uint32_t seed = 0;
  const char* mapgen_zone = nullptr;
  const char* resolved_theme = nullptr;
  const char* native_layout = nullptr;
  std::uint16_t width = kDefaultWidth;
  std::uint16_t height = kDefaultHeight;
  bool ensures_connected = true;
  std::uint8_t min_spawn_slots = 0;

  [[nodiscard]] constexpr bool operator==(const MapGenPlan&) const = default;
};

[[nodiscard]] constexpr const char* name(Status status) {
  switch (status) {
    case Status::Ok:
      return "Ok";
    case Status::InvalidTemplate:
      return "InvalidTemplate";
    case Status::InvalidLayout:
      return "InvalidLayout";
    case Status::InvalidTheme:
      return "InvalidTheme";
    case Status::InvalidZoneRef:
      return "InvalidZoneRef";
    case Status::OutOfBounds:
      return "OutOfBounds";
  }
  return "Unknown";
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

[[nodiscard]] constexpr bool is_zone_template(const char* template_id) {
  return string_eq(template_id, "wilds") || string_eq(template_id, "dungeon") ||
         string_eq(template_id, "crypt") || string_eq(template_id, "grove") ||
         string_eq(template_id, "marsh");
}

[[nodiscard]] constexpr bool is_zone_layout(const char* layout) {
  return string_eq(layout, "clearings") || string_eq(layout, "gauntlet") ||
         string_eq(layout, "warren");
}

[[nodiscard]] constexpr const char* mapgen_zone_for_template(
    const char* template_id) {
  if (string_eq(template_id, "wilds")) {
    return "wilds";
  }
  if (string_eq(template_id, "marsh")) {
    return "wilds";
  }
  if (string_eq(template_id, "grove")) {
    return "wilds";
  }
  if (string_eq(template_id, "dungeon")) {
    return "dungeon";
  }
  if (string_eq(template_id, "crypt")) {
    return "dungeon";
  }
  return nullptr;
}

[[nodiscard]] constexpr bool theme_allowed(const char* template_id,
                                           const char* theme) {
  if (string_eq(template_id, "wilds")) {
    return string_eq(theme, "forest") || string_eq(theme, "moor") ||
           string_eq(theme, "swamp") || string_eq(theme, "ash") ||
           string_eq(theme, "tundra");
  }
  if (string_eq(template_id, "marsh")) {
    return string_eq(theme, "swamp");
  }
  if (string_eq(template_id, "grove")) {
    return string_eq(theme, "forest");
  }
  if (string_eq(template_id, "dungeon") || string_eq(template_id, "crypt")) {
    return string_eq(theme, "crypt") || string_eq(theme, "fortress") ||
           string_eq(theme, "sewer") || string_eq(theme, "prison");
  }
  return false;
}

[[nodiscard]] constexpr std::uint8_t min_spawn_slots_for_layout(
    const char* layout) {
  if (string_eq(layout, "clearings")) {
    return 4;
  }
  if (string_eq(layout, "gauntlet")) {
    return 6;
  }
  if (string_eq(layout, "warren")) {
    return 8;
  }
  return 0;
}

[[nodiscard]] constexpr std::uint32_t combine_seed(const char* zone_id,
                                                 std::uint32_t seed) {
  std::uint32_t hash = 2166136261u;
  if (zone_id != nullptr) {
    for (const char* p = zone_id; *p != '\0'; ++p) {
      hash ^= static_cast<std::uint8_t>(*p);
      hash *= 16777619u;
    }
  }
  return (hash ^ seed) * 2654435761u;
}

[[nodiscard]] constexpr Status build_mapgen_plan(const ZoneSeed& zone,
                                                 MapGenPlan& plan) {
  if (!is_zone_template(zone.template_id)) {
    return Status::InvalidTemplate;
  }
  if (!is_zone_layout(zone.layout)) {
    return Status::InvalidLayout;
  }
  if (!theme_allowed(zone.template_id, zone.theme)) {
    return Status::InvalidTheme;
  }

  const char* mapgen_zone = mapgen_zone_for_template(zone.template_id);
  if (mapgen_zone == nullptr) {
    return Status::InvalidTemplate;
  }

  plan.seed = combine_seed(zone.id, zone.seed);
  plan.mapgen_zone = mapgen_zone;
  plan.resolved_theme = zone.theme;
  plan.native_layout = zone.layout;
  plan.width = kDefaultWidth;
  plan.height = kDefaultHeight;
  plan.ensures_connected = true;
  plan.min_spawn_slots = min_spawn_slots_for_layout(zone.layout);
  return Status::Ok;
}

[[nodiscard]] constexpr bool zone_id_known(const char* id,
                                         const ZoneSeed* zones,
                                         std::uint8_t count) {
  for (std::uint8_t i = 0; i < count; ++i) {
    if (string_eq(zones[i].id, id)) {
      return true;
    }
  }
  return false;
}

[[nodiscard]] constexpr Status validate_gate_refs(const ZoneSeed& zone,
                                                  const ZoneSeed* all_zones,
                                                  std::uint8_t zone_count) {
  for (std::uint8_t i = 0; i < zone.gate_count; ++i) {
    const GateAnchor& gate = zone.gates[i];
    if (gate.to_zone == nullptr ||
        !zone_id_known(gate.to_zone, all_zones, zone_count)) {
      return Status::InvalidZoneRef;
    }
  }
  return Status::Ok;
}

// Owner Demo zone graph excerpt (mirrors owner_demo_zones.json).
inline constexpr GateAnchor kPrologueGates[] = {
    {18, 42, "verdigris-crossroads", false},
};
inline constexpr GateAnchor kCrossroadsGates[] = {
    {38, 95, "owner-demo-thornward", false},
    {52, 110, "owner-demo-rift-hollow", false},
};
inline constexpr GateAnchor kThornwardGates[] = {
    {4, 8, "verdigris-crossroads", false},
    {68, 12, "owner-demo-glimmer-cave", true},
    {70, 48, "owner-demo-rift-hollow", false},
};
inline constexpr GateAnchor kRiftGates[] = {
    {6, 10, "owner-demo-thornward", false},
    {2, 6, "verdigris-crossroads", false},
};
inline constexpr GateAnchor kGlimmerGates[] = {
    {8, 14, "owner-demo-thornward", false},
};

inline constexpr ZoneSeed kOwnerDemoZones[kOwnerDemoZoneCount] = {
    {"owner-demo-prologue", "wilds", "clearings", "forest", 42001u, 1,
     kPrologueGates},
    {"verdigris-crossroads", "wilds", "clearings", "moor", 0u, 2,
     kCrossroadsGates},
    {"owner-demo-thornward", "wilds", "gauntlet", "forest", 77821u, 3,
     kThornwardGates},
    {"owner-demo-rift-hollow", "dungeon", "warren", "crypt", 90217u, 2,
     kRiftGates},
    {"owner-demo-glimmer-cave", "crypt", "warren", "crypt", 55103u, 1,
     kGlimmerGates},
};

[[nodiscard]] constexpr Status validate_owner_demo_graph() {
  for (std::uint8_t i = 0; i < kOwnerDemoZoneCount; ++i) {
    const Status gate_status =
        validate_gate_refs(kOwnerDemoZones[i], kOwnerDemoZones,
                           kOwnerDemoZoneCount);
    if (gate_status != Status::Ok) {
      return gate_status;
    }
    MapGenPlan plan{};
    const Status plan_status = build_mapgen_plan(kOwnerDemoZones[i], plan);
    if (plan_status != Status::Ok) {
      return plan_status;
    }
    if (!plan.ensures_connected || plan.min_spawn_slots == 0) {
      return Status::OutOfBounds;
    }
  }
  return Status::Ok;
}

}  // namespace cartographer_adapter
