#pragma once

// VG-WORLD-008: versioned visual dressing on a frozen topology. Dressing
// trees cannot become solids. Core layout seeds stay with Kimi.

#include <cstdint>
#include <string>

namespace verdigris::client::world {

inline constexpr int kDressingPassVersion = 1;

struct LayoutSample {
  int kind = 0;
  int x = 0;
  int y = 0;
  int radius = 0;
  bool solid = false;
  bool dressing = false;
};

inline std::uint64_t fnv1a(std::uint64_t value, std::uint64_t piece) {
  value ^= piece;
  value *= 1099511628211ULL;
  return value;
}

inline std::uint64_t topology_hash(const LayoutSample* items, std::size_t count,
                                   int spawn_x, int spawn_y,
                                   std::uint64_t reward_seed) {
  std::uint64_t value = 1469598103934665603ULL;
  value = fnv1a(value, static_cast<std::uint64_t>(spawn_x) + 0x9e3779b97f4a7c15ULL);
  value = fnv1a(value, static_cast<std::uint64_t>(spawn_y));
  value = fnv1a(value, reward_seed);
  for (std::size_t i = 0; i < count; ++i) {
    const LayoutSample& item = items[i];
    if (item.dressing) continue;
    value = fnv1a(value, static_cast<std::uint64_t>(item.kind + 1));
    value = fnv1a(value, static_cast<std::uint64_t>(item.x));
    value = fnv1a(value, static_cast<std::uint64_t>(item.y));
    value = fnv1a(value, static_cast<std::uint64_t>(item.radius));
    value = fnv1a(value, item.solid ? 1ULL : 0ULL);
  }
  return value;
}

inline std::uint64_t dressing_hash(const LayoutSample* items, std::size_t count,
                                   int version) {
  std::uint64_t value = 1469598103934665603ULL;
  value = fnv1a(value, static_cast<std::uint64_t>(version));
  for (std::size_t i = 0; i < count; ++i) {
    const LayoutSample& item = items[i];
    if (!item.dressing) continue;
    value = fnv1a(value, static_cast<std::uint64_t>(item.x));
    value = fnv1a(value, static_cast<std::uint64_t>(item.y));
    value = fnv1a(value, static_cast<std::uint64_t>(item.radius));
  }
  return value;
}

inline bool dressing_is_unreported_obstacle(const LayoutSample& item) {
  return item.dressing && item.solid;
}

inline int dressing_count(int version) { return version <= 1 ? 2 : 5; }

struct DressingSpec {
  int x = 0;
  int y = 0;
  int radius = 24;
  double scale = 0.7;
};

inline int append_dressing(DressingSpec* out, int capacity, int version,
                           int spawn_x, int spawn_y) {
  const int n = dressing_count(version);
  const int ox[5] = {180, -210, 240, -160, 90};
  const int oy[5] = {140, 190, -120, -220, 80};
  const double scale = version <= 1 ? 0.72 : 1.08;
  int written = 0;
  for (int i = 0; i < n && written < capacity; ++i) {
    out[written].x = spawn_x + ox[i];
    out[written].y = spawn_y + oy[i];
    out[written].radius = 24;
    out[written].scale = scale;
    ++written;
  }
  return written;
}

inline std::string pass_hud_label(int version) {
  return std::string("dressing-pass:v") + std::to_string(version);
}

inline const char* owner_dressing_label() { return "Dressing"; }
inline const char* owner_not_solid_label() { return "Not solid"; }

}  // namespace verdigris::client::world
