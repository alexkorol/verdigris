#pragma once

// VG-ART-002: cooked bronze/stone family. Albedo + rim maps, SPDX provenance.
// A magenta placeholder fill is never a finished material.

#include <cstdint>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

namespace verdigris::art::bronze_stone {

inline constexpr const char* kLicense = "CC0-1.0";
inline constexpr const char* kSource = "cooked:bronze-stone-v1";
inline constexpr int kMapSize = 8;

inline constexpr std::uint32_t kBronze = 0x00C48E40u;
inline constexpr std::uint32_t kStone = 0x00767068u;
inline constexpr std::uint32_t kBronzeRim = 0x00E8C878u;
inline constexpr std::uint32_t kStoneRim = 0x00B0AAA0u;
inline constexpr std::uint32_t kPlaceholder = 0x00FF00FFu;

inline constexpr std::uint32_t kAlbedo[kMapSize * kMapSize] = {
    kBronze, kBronze, kBronze, kBronze, kStone,  kStone,  kStone,  kStone,
    kBronze, kBronze, kBronze, kBronze, kStone,  kStone,  kStone,  kStone,
    kBronze, kBronze, kBronze, kBronze, kStone,  kStone,  kStone,  kStone,
    kBronze, kBronze, kBronze, kBronze, kStone,  kStone,  kStone,  kStone,
    kStone,  kStone,  kStone,  kStone,  kBronze, kBronze, kBronze, kBronze,
    kStone,  kStone,  kStone,  kStone,  kBronze, kBronze, kBronze, kBronze,
    kStone,  kStone,  kStone,  kStone,  kBronze, kBronze, kBronze, kBronze,
    kStone,  kStone,  kStone,  kStone,  kBronze, kBronze, kBronze, kBronze,
};

// Rim/light response: brighter toward upper-left, not a flat fill.
inline constexpr std::uint32_t kRim[kMapSize * kMapSize] = {
    kBronzeRim, kBronzeRim, kBronzeRim, kStoneRim, kStoneRim, kStoneRim, kStone, kStone,
    kBronzeRim, kBronzeRim, kBronze,    kStoneRim, kStoneRim, kStone,    kStone, kStone,
    kBronzeRim, kBronze,    kBronze,    kStone,    kStone,    kStone,    kStone, kStone,
    kBronze,    kBronze,    kBronze,    kStone,    kStone,    kStone,    kStone, kStone,
    kStoneRim,  kStone,     kStone,     kBronze,   kBronze,   kBronze,   kBronze, kBronze,
    kStone,     kStone,     kStone,     kBronze,   kBronze,   kBronze,   kBronze, kBronze,
    kStone,     kStone,     kStone,     kBronze,   kBronze,   kBronze,   kBronzeRim, kBronzeRim,
    kStone,     kStone,     kStone,     kBronze,   kBronze,   kBronzeRim, kBronzeRim, kBronzeRim,
};

inline bool shippable() {
  return kLicense[0] != '\0' && kSource[0] != '\0' && kAlbedo[0] != kPlaceholder &&
         kRim[0] != kPlaceholder;
}

inline std::uint32_t sample_albedo(int u, int v) {
  const int x = ((u % kMapSize) + kMapSize) % kMapSize;
  const int y = ((v % kMapSize) + kMapSize) % kMapSize;
  return kAlbedo[y * kMapSize + x];
}

inline std::uint32_t sample_rim(int u, int v) {
  const int x = ((u % kMapSize) + kMapSize) % kMapSize;
  const int y = ((v % kMapSize) + kMapSize) % kMapSize;
  return kRim[y * kMapSize + x];
}

inline bool is_placeholder(std::uint32_t rgba) { return rgba == kPlaceholder; }

inline const char* owner_bronze_stone_label() { return "Bronze stone"; }
inline const char* owner_cooked_cc0_label() { return "Cooked CC0"; }

#ifdef _WIN32
inline COLORREF gdi(std::uint32_t rrggbb) {
  return RGB(static_cast<int>((rrggbb >> 16) & 0xFF),
             static_cast<int>((rrggbb >> 8) & 0xFF),
             static_cast<int>(rrggbb & 0xFF));
}
inline COLORREF gdi_bronze() { return gdi(kBronze); }
inline COLORREF gdi_stone() { return gdi(kStone); }
inline COLORREF gdi_bronze_rim() { return gdi(kBronzeRim); }
#endif

}  // namespace verdigris::art::bronze_stone
