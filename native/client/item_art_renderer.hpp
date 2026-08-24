// item_art_renderer.hpp — TASK-0182 item-art render adapter (r2).
//
// Maps REAL string item ids to TASK-0169 RPG Inventory art and plans
// deterministic sprite blits into grid cells with aspect-preserving fit.
//
// Two id vocabularies exist and neither is invented here:
//   - manifest ids: native/client/assets/wizard/items/manifest.json
//     ("dagger_bronze", "boar_pike", ...). The catalog below mirrors that
//     manifest row-for-row; the task-dir drift-guard test compares the two
//     and fails the harness on any divergence.
//   - sim ids: kItemCatalogue in native/src/core.cpp (~:2636)
//     ("bronze-dagger", "bronze-pike", ...; pick_up/equip take these
//     strings — native/include/verdigris/core.hpp Command::pick_up).
// kSimArtMap is the single explicit sim-id -> manifest-id table for the
// items present in both vocabularies. Sim items with no art in the pack
// are listed with an empty manifest id and resolve to Status::NoArt — a
// diagnostic for presentation to draw its own placeholder, never an
// invented fallback sprite.
//
// Header stays pure: constexpr-friendly, no I/O, no allocation.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace item_art_renderer {

inline constexpr std::size_t kCatalogSize = 12;
inline constexpr std::size_t kSimArtMapSize = 28;
inline constexpr std::size_t kMaxIdLen = 32;
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
  Ok,        // resolved to a manifest art entry with a drawable blit
  NoArt,     // known sim item, deliberately no art in the pack (documented)
  NotFound,  // id unknown to the adapter
  Invalid,   // bad input (null/empty id, degenerate cell or blit)
};

// Deterministic diagnostic names (SPEC: deterministic fallback diagnostics).
[[nodiscard]] constexpr const char* category_name(Category category) {
  switch (category) {
    case Category::Weapon: return "weapon";
    case Category::Armor: return "armor";
    case Category::Tool: return "tool";
    case Category::Reagent: return "reagent";
    case Category::Trophy: return "trophy";
    case Category::Recoverable: return "recoverable";
    case Category::Unknown: break;
  }
  return "unknown";
}

[[nodiscard]] constexpr const char* status_name(Status status) {
  switch (status) {
    case Status::Ok: return "ok";
    case Status::NoArt: return "no-art";
    case Status::NotFound: return "not-found";
    case Status::Invalid: return "invalid";
  }
  return "invalid";
}

// ── constexpr string helpers (no <cstring> in constexpr) ─────────────────

[[nodiscard]] constexpr bool str_eq(const char* a, const char* b) {
  if (a == nullptr || b == nullptr) return false;
  for (std::size_t i = 0;; ++i) {
    if (a[i] != b[i]) return false;
    if (a[i] == '\0') return true;
  }
}

[[nodiscard]] constexpr std::size_t str_len(const char* s) {
  std::size_t n = 0;
  while (s != nullptr && s[n] != '\0') ++n;
  return n;
}

// Bounded copy that ALWAYS null-terminates (REVIEW r1 minor note: the old
// copy_path left the buffer unterminated for >= cap-length names).
constexpr void copy_str(char* dest, std::size_t cap, const char* src) {
  if (cap == 0) return;
  std::size_t i = 0;
  for (; i + 1 < cap && src[i] != '\0'; ++i) dest[i] = src[i];
  dest[i] = '\0';
}

// ── data types ───────────────────────────────────────────────────────────

struct Footprint {
  std::uint16_t width = 0;
  std::uint16_t height = 0;

  [[nodiscard]] constexpr bool valid() const {
    return width > 0 && height > 0;
  }

  [[nodiscard]] constexpr bool operator==(const Footprint&) const = default;
};

struct CatalogEntry {
  char id[kMaxIdLen]{};  // manifest string id, e.g. "dagger_bronze"
  Category category = Category::Unknown;
  Footprint footprint{};
  char path[kMaxPathLen]{};  // manifest "path" (file name inside the pack)

  [[nodiscard]] constexpr bool valid() const {
    return id[0] != '\0' && footprint.valid() && path[0] != '\0';
  }

  [[nodiscard]] constexpr bool operator==(const CatalogEntry&) const = default;
};

struct SimArtMapping {
  char sim_id[kMaxIdLen]{};       // kItemCatalogue id, e.g. "bronze-dagger"
  char manifest_id[kMaxIdLen]{};  // empty => documented no-art sim item

  [[nodiscard]] constexpr bool has_art() const { return manifest_id[0] != '\0'; }
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
  char id[kMaxIdLen]{};  // manifest id of the resolved art
  Category category = Category::Unknown;

  // A blit is drawable only with both destination dimensions non-zero;
  // degenerate (zero-width/height) blits are never emitted as Ok.
  [[nodiscard]] constexpr bool valid() const { return dst_w > 0 && dst_h > 0; }

  [[nodiscard]] constexpr bool operator==(const SpriteBlit&) const = default;
};

struct ResolveResult {
  Status status = Status::Invalid;
  CatalogEntry entry{};
  SpriteBlit blit{};

  [[nodiscard]] constexpr bool operator==(const ResolveResult&) const = default;
};

// ── catalog: mirrors items/manifest.json row-for-row ─────────────────────

[[nodiscard]] constexpr CatalogEntry make_entry(const char* id, Category category,
                                                std::uint16_t w, std::uint16_t h,
                                                const char* path) {
  CatalogEntry entry;
  copy_str(entry.id, kMaxIdLen, id);
  entry.category = category;
  entry.footprint = {w, h};
  copy_str(entry.path, kMaxPathLen, path);
  return entry;
}

[[nodiscard]] constexpr std::array<CatalogEntry, kCatalogSize> default_catalog() {
  return {
      make_entry("dagger_bronze", Category::Weapon, 344, 512, "dagger_bronze.png"),
      make_entry("boar_pike", Category::Weapon, 204, 512, "boar_pike.png"),
      make_entry("astral_plate", Category::Armor, 421, 512, "astral_plate.png"),
      make_entry("boots_fur", Category::Armor, 471, 512, "boots_fur.png"),
      make_entry("cur_chisel", Category::Tool, 84, 512, "cur_chisel.png"),
      make_entry("cur_knife", Category::Tool, 470, 512, "cur_knife.png"),
      make_entry("cur_draught", Category::Reagent, 376, 512, "cur_draught.png"),
      make_entry("cur_orb", Category::Reagent, 416, 512, "cur_orb.png"),
      make_entry("bird_omen", Category::Trophy, 339, 512, "bird_omen.png"),
      make_entry("blood_omen", Category::Trophy, 512, 381, "blood_omen.png"),
      make_entry("ember_shell", Category::Recoverable, 399, 512, "ember_shell.png"),
      make_entry("bowl_bronze_offering", Category::Recoverable, 512, 323,
                 "bowl_bronze_offering.png"),
  };
}

// ── sim-id -> manifest-id mapping (the ONE explicit table) ───────────────
//
// Covers every id in kItemCatalogue (native/src/core.cpp:2636-2674).
// Mapped rows pair the same item concept in both vocabularies:
//   bronze-dagger -> dagger_bronze  (both "Bronze Dagger")
//   bronze-pike   -> boar_pike      (the only pike in each vocabulary)
//   bronze-boots  -> boots_fur      (the only feet-slot boots art; material
//                                    differs — fur art for a bronze item)
//   knife         -> cur_knife      (the only knife in each vocabulary)
// Empty manifest_id documents a sim item with NO art in the 12-item pack:
// currency/jewelry/belt/amulet have no counterpart; swords, axes and the
// remaining armor slots have no sword/axe/helm/glove/shield art (a dagger
// is not a sword — no substitution); the 13 vessel-* bases roll procedural
// identity via the forge and await a dedicated pack.

[[nodiscard]] constexpr SimArtMapping make_mapping(const char* sim_id,
                                                   const char* manifest_id) {
  SimArtMapping mapping;
  copy_str(mapping.sim_id, kMaxIdLen, sim_id);
  copy_str(mapping.manifest_id, kMaxIdLen, manifest_id);
  return mapping;
}

[[nodiscard]] constexpr std::array<SimArtMapping, kSimArtMapSize> sim_art_map() {
  return {
      // general.js / jewelry.js / belts.js — no counterpart art in the pack.
      make_mapping("coins", ""),
      make_mapping("ring", ""),
      make_mapping("gold-ring", ""),
      make_mapping("hide-girdle", ""),
      make_mapping("garnet-amulet", ""),
      // weapons.js / verdigris.js curated bases.
      make_mapping("bronze-sword", ""),  // pack has no sword art
      make_mapping("bronze-pike", "boar_pike"),
      // vessels.js — procedural forge bases, no dedicated art yet.
      make_mapping("vessel-handaxe", ""),
      make_mapping("vessel-spear", ""),
      make_mapping("vessel-macuahuitl", ""),
      make_mapping("vessel-atlatl", ""),
      make_mapping("vessel-khopesh", ""),
      make_mapping("vessel-sling", ""),
      make_mapping("vessel-shield", ""),
      make_mapping("vessel-wrap", ""),
      make_mapping("vessel-crest", ""),
      make_mapping("vessel-grips", ""),
      make_mapping("vessel-sandals", ""),
      make_mapping("vessel-gorget", ""),
      make_mapping("vessel-ring", ""),
      // weapons.js iron/steel tiers — no sword or battleaxe art.
      make_mapping("iron-sword", ""),
      make_mapping("steel-battleaxe", ""),
      // town-amenities starter kit.
      make_mapping("bronze-dagger", "dagger_bronze"),
      make_mapping("bronze-med-helm", ""),  // no helm art
      make_mapping("bronze-gloves", ""),    // no glove art
      make_mapping("bronze-boots", "boots_fur"),
      make_mapping("knife", "cur_knife"),
      make_mapping("wooden-shield", ""),  // no shield art
  };
}

// ── lookup + fit ─────────────────────────────────────────────────────────

[[nodiscard]] constexpr const CatalogEntry* find_entry(
    const std::array<CatalogEntry, kCatalogSize>& catalog, const char* manifest_id) {
  if (manifest_id == nullptr || manifest_id[0] == '\0') return nullptr;
  for (const CatalogEntry& entry : catalog) {
    if (str_eq(entry.id, manifest_id)) return &entry;
  }
  return nullptr;
}

[[nodiscard]] constexpr const SimArtMapping* find_sim_mapping(
    const std::array<SimArtMapping, kSimArtMapSize>& map, const char* sim_id) {
  if (sim_id == nullptr || sim_id[0] == '\0') return nullptr;
  for (const SimArtMapping& mapping : map) {
    if (str_eq(mapping.sim_id, sim_id)) return &mapping;
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
  // Extreme cell aspect ratios can collapse a dimension to zero; skip the
  // blit entirely instead of emitting a zero-width/height draw (REVIEW r1
  // minor note).
  if (dst_w == 0 || dst_h == 0) return blit;

  blit.dst_x = cell.x + static_cast<std::int16_t>((cell_w - dst_w) / 2);
  blit.dst_y = cell.y + static_cast<std::int16_t>((cell_h - dst_h) / 2);
  blit.dst_w = static_cast<std::uint16_t>(dst_w);
  blit.dst_h = static_cast<std::uint16_t>(dst_h);
  blit.src_w = static_cast<std::uint16_t>(src_w);
  blit.src_h = static_cast<std::uint16_t>(src_h);
  copy_str(blit.id, kMaxIdLen, entry.id);
  blit.category = entry.category;
  return blit;
}

// ── resolve: manifest-id keyed ───────────────────────────────────────────

[[nodiscard]] constexpr ResolveResult resolve(const char* manifest_id,
                                              const CellRect& cell) {
  ResolveResult result;
  if (manifest_id == nullptr || manifest_id[0] == '\0' || !cell.valid()) {
    return result;  // Status::Invalid
  }

  const auto catalog = default_catalog();
  const CatalogEntry* entry = find_entry(catalog, manifest_id);
  if (entry == nullptr) {
    result.status = Status::NotFound;
    return result;
  }

  result.entry = *entry;
  result.blit = fit_sprite(*entry, cell);
  result.status = result.blit.valid() ? Status::Ok : Status::Invalid;
  return result;
}

// ── resolve_sim: end-to-end sim-id -> manifest art ───────────────────────
//
// Ok       -> entry/blit carry the manifest art for this sim item.
// NoArt    -> sim id is known and documented art-less; entry/blit stay
//             empty so presentation renders its own placeholder.
// NotFound -> sim id absent from kSimArtMap (update the table when the sim
//             catalogue grows).
// Invalid  -> null/empty id or degenerate cell/blit.

[[nodiscard]] constexpr ResolveResult resolve_sim(const char* sim_id,
                                                  const CellRect& cell) {
  ResolveResult result;
  if (sim_id == nullptr || sim_id[0] == '\0' || !cell.valid()) {
    return result;  // Status::Invalid
  }

  const auto map = sim_art_map();
  const SimArtMapping* mapping = find_sim_mapping(map, sim_id);
  if (mapping == nullptr) {
    result.status = Status::NotFound;
    return result;
  }
  if (!mapping->has_art()) {
    result.status = Status::NoArt;
    return result;
  }
  return resolve(mapping->manifest_id, cell);
}

// ── deterministic catalog checksum (FNV-1a over id/category/footprint/path)

[[nodiscard]] constexpr std::uint32_t catalog_checksum() {
  constexpr std::uint32_t kFnvOffset = 2166136261u;
  constexpr std::uint32_t kFnvPrime = 16777619u;
  std::uint32_t hash = kFnvOffset;
  const auto mix = [&](std::uint32_t value) {
    hash ^= value;
    hash *= kFnvPrime;
  };
  const auto mix_str = [&](const char* s) {
    for (std::size_t i = 0; s[i] != '\0'; ++i) {
      mix(static_cast<std::uint32_t>(static_cast<unsigned char>(s[i])));
    }
    mix(0xffu);  // terminator sentinel
  };
  const auto catalog = default_catalog();
  for (const CatalogEntry& entry : catalog) {
    mix_str(entry.id);
    mix(static_cast<std::uint32_t>(entry.category));
    mix(entry.footprint.width);
    mix(entry.footprint.height);
    mix_str(entry.path);
  }
  return hash;
}

}  // namespace item_art_renderer
