// item_art_renderer.hpp — TASK-0182 item-art render adapter.
//
// Maps stable item ids to TASK-0169 RPG Inventory footprints and plans
// deterministic sprite blits into grid cells with aspect-preserving fit.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace item_art_renderer {

inline constexpr std::size_t kCatalogSize = 12;
inline constexpr std::size_t kMaxPathLen = 32;

enum class Category : std::uint8_t {
  Unknown = 0,
  Weapon,
  Armor,
  Tool,
  Reagent,
  Trophy,
  Recoverable,
};

enum class Status : std::uint8_t {
  Ok,
  NotFound,
  Invalid,
};

struct Footprint {
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const {
    return width > 0 && height > 0;
  }
};

struct CatalogEntry {
  std::uint32_t id = 0;
  Category category = Category::Unknown;
  Footprint footprint{};
  char path[kMaxPathLen]{};

  [[nodiscard]] constexpr bool valid() const {
    return id != 0 && footprint.valid() && path[0] != '\0';
  }
};

struct CellRect {
  std::int16_t x = 0;
  std::int16_t y = 0;
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

struct SpriteBlit {
  std::int16_t dst_x = 0;
  std::int16_t dst_y = 0;
  std::uint16_t dst_w = 0;
  std::uint16_t dst_h = 0;
  std::uint16_t src_w = 0;
  std::uint16_t src_h = 0;
  std::uint32_t item_id = 0;
  Category category = Category::Unknown;

  [[nodiscard]] constexpr bool operator==(const SpriteBlit&) const = default;
};

struct ResolveResult {
  Status status = Status::Invalid;
  CatalogEntry entry{};
  SpriteBlit blit{};

  [[nodiscard]] constexpr bool operator==(const ResolveResult&) const = default;
};

constexpr void copy_path(char* dest, const char* src) {
  for (std::size_t i = 0; i < kMaxPathLen; ++i) {
    dest[i] = src[i];
    if (src[i] == '\0') break;
  }
}

[[nodiscard]] constexpr CatalogEntry make_entry(std::uint32_t id, Category category,
                                                std::uint16_t w, std::uint16_t h,
                                                const char* path) {
  CatalogEntry entry;
  entry.id = id;
  entry.category = category;
  entry.footprint = {w, h};
  copy_path(entry.path, path);
  return entry;
}

[[nodiscard]] constexpr std::array<CatalogEntry, kCatalogSize> default_catalog() {
  return {
      make_entry(101, Category::Weapon, 344, 512, "dagger_bronze.png"),
      make_entry(102, Category::Weapon, 204, 512, "boar_pike.png"),
      make_entry(201, Category::Armor, 421, 512, "astral_plate.png"),
      make_entry(202, Category::Armor, 471, 512, "boots_fur.png"),
      make_entry(301, Category::Tool, 84, 512, "cur_chisel.png"),
      make_entry(302, Category::Tool, 470, 512, "cur_knife.png"),
      make_entry(401, Category::Reagent, 376, 512, "cur_draught.png"),
      make_entry(402, Category::Reagent, 416, 512, "cur_orb.png"),
      make_entry(501, Category::Trophy, 339, 512, "bird_omen.png"),
      make_entry(502, Category::Trophy, 512, 381, "blood_omen.png"),
      make_entry(601, Category::Recoverable, 399, 512, "ember_shell.png"),
      make_entry(602, Category::Recoverable, 512, 323, "bowl_bronze_offering.png"),
  };
}

[[nodiscard]] constexpr const CatalogEntry* find_entry(
    const std::array<CatalogEntry, kCatalogSize>& catalog, std::uint32_t id) {
  for (const CatalogEntry& entry : catalog) {
    if (entry.id == id) return &entry;
  }
  return nullptr;
}

[[nodiscard]] constexpr SpriteBlit fit_sprite(const CatalogEntry& entry,
                                              const CellRect& cell) {
  SpriteBlit blit;
  if (!entry.valid() || !cell.valid()) return blit;

  const std::uint32_t src_w = entry.footprint.width;
  const std::uint32_t src_h = entry.footprint.height;
  const std::uint32_t cell_w = cell.width;
  const std::uint32_t cell_h = cell.height;

  std::uint32_t dst_w = cell_w;
  std::uint32_t dst_h = static_cast<std::uint32_t>(
      (static_cast<std::uint64_t>(src_h) * cell_w) / src_w);
  if (dst_h > cell_h) {
    dst_h = cell_h;
    dst_w = static_cast<std::uint32_t>(
        (static_cast<std::uint64_t>(src_w) * cell_h) / src_h);
  }

  blit.dst_x =
      cell.x + static_cast<std::int16_t>((cell_w - dst_w) / 2);
  blit.dst_y =
      cell.y + static_cast<std::int16_t>((cell_h - dst_h) / 2);
  blit.dst_w = static_cast<std::uint16_t>(dst_w);
  blit.dst_h = static_cast<std::uint16_t>(dst_h);
  blit.src_w = static_cast<std::uint16_t>(src_w);
  blit.src_h = static_cast<std::uint16_t>(src_h);
  blit.item_id = entry.id;
  blit.category = entry.category;
  return blit;
}

[[nodiscard]] constexpr ResolveResult resolve(std::uint32_t item_id,
                                              const CellRect& cell) {
  ResolveResult result;
  if (!cell.valid()) return result;

  const auto catalog = default_catalog();
  const CatalogEntry* entry = find_entry(catalog, item_id);
  if (!entry) {
    result.status = Status::NotFound;
    return result;
  }

  result.status = Status::Ok;
  result.entry = *entry;
  result.blit = fit_sprite(*entry, cell);
  return result;
}

[[nodiscard]] constexpr std::uint32_t catalog_checksum() {
  const auto catalog = default_catalog();
  std::uint32_t hash = 0;
  for (const CatalogEntry& entry : catalog) {
    hash ^= entry.id * 131u;
    hash ^= static_cast<std::uint32_t>(entry.category) * 257u;
    hash ^= entry.footprint.width * 389u;
    hash ^= entry.footprint.height * 521u;
  }
  return hash;
}

}  // namespace item_art_renderer
