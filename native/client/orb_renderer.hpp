// orb_renderer.hpp — TASK-0181 WIZARD orb render adapter.
//
// Deterministic layer/compositing planner for life/mana globes from TASK-0168
// art plates. Maps authoritative fill and reservation ratios to active layers
// and depletion feedback without primitive circle placeholders.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace orb_renderer {

inline constexpr std::size_t kMaxLayers = 6;
inline constexpr std::uint16_t kRatioScale = 1000;

enum class OrbKind : std::uint8_t {
  Life = 0,
  Mana,
};

enum class LayerId : std::uint8_t {
  ColorPlate = 0,
  Mask,
  NormalMap,
  DepthAoPack,
  ReservedStone,
  EmptyGlass,
};

enum class DepletionBand : std::uint8_t {
  Full = 0,
  Half,
  Low,
  Empty,
};

enum class Status : std::uint8_t {
  Ok,
  Invalid,
};

struct LayerDraw {
  LayerId layer = LayerId::ColorPlate;
  bool active = false;
  std::uint16_t weight = 0;  // 0..1000 blend weight

  [[nodiscard]] constexpr bool operator==(const LayerDraw&) const = default;
};

struct OrbState {
  OrbKind kind = OrbKind::Life;
  std::uint16_t current = 0;
  std::uint16_t max = 0;
  std::uint16_t reserved = 0;

  [[nodiscard]] constexpr bool valid() const {
    return max > 0 && current <= max && reserved <= max;
  }

  [[nodiscard]] constexpr std::uint16_t fill_ratio() const {
    if (max == 0) return 0;
    return static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(current) * kRatioScale) / max);
  }

  [[nodiscard]] constexpr std::uint16_t reserve_ratio() const {
    if (max == 0) return 0;
    return static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(reserved) * kRatioScale) / max);
  }
};

struct OrbDrawPlan {
  Status status = Status::Invalid;
  DepletionBand band = DepletionBand::Empty;
  std::uint16_t fill_ratio = 0;
  std::uint16_t reserve_ratio = 0;
  bool pulse_low = false;
  std::array<LayerDraw, kMaxLayers> layers{};

  [[nodiscard]] constexpr bool operator==(const OrbDrawPlan&) const = default;
};

[[nodiscard]] constexpr DepletionBand band_for_ratio(std::uint16_t ratio) {
  if (ratio > 750) return DepletionBand::Full;
  if (ratio > 400) return DepletionBand::Half;
  if (ratio > 80) return DepletionBand::Low;
  return DepletionBand::Empty;
}

[[nodiscard]] constexpr LayerDraw layer(LayerId id, bool active,
                                      std::uint16_t weight = kRatioScale) {
  LayerDraw draw;
  draw.layer = id;
  draw.active = active;
  draw.weight = weight;
  return draw;
}

[[nodiscard]] constexpr OrbDrawPlan plan_orb(const OrbState& state) {
  OrbDrawPlan plan;
  if (!state.valid()) return plan;

  plan.status = Status::Ok;
  plan.fill_ratio = state.fill_ratio();
  plan.reserve_ratio = state.reserve_ratio();
  plan.band = band_for_ratio(plan.fill_ratio);
  plan.pulse_low = plan.band == DepletionBand::Low || plan.band == DepletionBand::Empty;

  const bool show_liquid = plan.band != DepletionBand::Empty;
  const bool show_empty = plan.band == DepletionBand::Empty || plan.reserve_ratio > 0;
  const bool show_stone = plan.reserve_ratio > 0;

  plan.layers[0] = layer(LayerId::ColorPlate, show_liquid, plan.fill_ratio);
  plan.layers[1] = layer(LayerId::Mask, true, kRatioScale);
  plan.layers[2] = layer(LayerId::NormalMap, show_liquid, plan.fill_ratio);
  plan.layers[3] = layer(LayerId::DepthAoPack, show_liquid, plan.fill_ratio);
  plan.layers[4] =
      layer(LayerId::ReservedStone, show_stone, plan.reserve_ratio);
  plan.layers[5] = layer(LayerId::EmptyGlass, show_empty, kRatioScale);

  return plan;
}

[[nodiscard]] constexpr std::uint32_t plan_checksum(const OrbDrawPlan& plan) {
  std::uint32_t hash = static_cast<std::uint32_t>(plan.status);
  hash ^= static_cast<std::uint32_t>(plan.band) * 17u;
  hash ^= plan.fill_ratio * 131u;
  hash ^= plan.reserve_ratio * 257u;
  hash ^= plan.pulse_low ? 389u : 0u;
  for (const LayerDraw& draw : plan.layers) {
    hash ^= static_cast<std::uint32_t>(draw.layer) * 521u;
    hash ^= draw.active ? 613u : 0u;
    hash ^= draw.weight * 719u;
  }
  return hash;
}

}  // namespace orb_renderer
