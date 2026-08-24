// item_art_renderer_tests.cpp — TASK-0182 acceptance tests (r2).
//
// Covers the two REVIEW corrections:
//   1. resolve()/resolve_sim() keyed on REAL string ids: at least three
//      real sim item ids (kItemCatalogue, native/src/core.cpp:2636+)
//      resolve end-to-end to manifest art entries.
//   2. Drift guard: argv[1] is the actual items/manifest.json; a minimal
//      dependency-free parser reads it at test time and the guard fails on
//      any divergence in id/footprint/category/filename between the header
//      catalog and the manifest — with deliberate-mismatch negative
//      controls proving the guard can fail.

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "item_art_renderer.hpp"

using namespace item_art_renderer;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

// ── compile-time proof the adapter stays constexpr-usable ────────────────

constexpr CellRect kCell{0, 0, 64, 64};
static_assert(resolve_sim("bronze-dagger", kCell).status == Status::Ok,
              "sim id resolves at compile time");
static_assert(str_eq(resolve_sim("bronze-dagger", kCell).entry.id, "dagger_bronze"),
              "sim id maps to manifest id at compile time");
static_assert(resolve_sim("coins", kCell).status == Status::NoArt,
              "documented no-art sim id at compile time");
static_assert(resolve("boar_pike", kCell).status == Status::Ok,
              "manifest id resolves at compile time");
static_assert(resolve("no_such_art", kCell).status == Status::NotFound,
              "unknown manifest id at compile time");

// ── correction 1: real-id keying ─────────────────────────────────────────

void test_sim_ids_end_to_end() {
  // Acceptance: >= 3 real sim item ids resolve end-to-end to manifest art
  // entries. All four sim ids with a concept match in the pack are covered.
  struct Case {
    const char* sim_id;
    const char* manifest_id;
    const char* path;
    Category category;
  };
  const Case cases[] = {
      {"bronze-dagger", "dagger_bronze", "dagger_bronze.png", Category::Weapon},
      {"bronze-pike", "boar_pike", "boar_pike.png", Category::Weapon},
      {"knife", "cur_knife", "cur_knife.png", Category::Tool},
      {"bronze-boots", "boots_fur", "boots_fur.png", Category::Armor},
  };
  int resolved = 0;
  for (const Case& c : cases) {
    const ResolveResult result = resolve_sim(c.sim_id, {0, 0, 64, 64});
    const std::string tag = std::string("sim '") + c.sim_id + "' ";
    check(result.status == Status::Ok, tag + "resolves Ok");
    check(str_eq(result.entry.id, c.manifest_id), tag + "manifest id");
    check(str_eq(result.entry.path, c.path), tag + "art filename");
    check(result.entry.category == c.category, tag + "category");
    check(result.blit.valid(), tag + "drawable blit");
    check(str_eq(result.blit.id, c.manifest_id), tag + "blit carries manifest id");
    check(result.blit.dst_w <= 64 && result.blit.dst_h <= 64, tag + "fits cell");
    ++resolved;
  }
  check(resolved >= 3, "at least three real sim ids resolve end-to-end");
}

void test_sim_no_art_documented() {
  // Known sim items without pack art return NoArt with EMPTY entry/blit —
  // a status, never an invented fallback sprite.
  const char* no_art_ids[] = {"coins", "bronze-sword", "iron-sword",
                              "steel-battleaxe", "vessel-spear", "gold-ring",
                              "wooden-shield", "bronze-med-helm"};
  for (const char* sim_id : no_art_ids) {
    const ResolveResult result = resolve_sim(sim_id, {0, 0, 64, 64});
    const std::string tag = std::string("no-art sim '") + sim_id + "' ";
    check(result.status == Status::NoArt, tag + "returns NoArt");
    check(result.entry.id[0] == '\0', tag + "empty entry (no invention)");
    check(!result.blit.valid(), tag + "no blit emitted");
  }
}

void test_sim_unknown_and_invalid() {
  check(resolve_sim("dragon-lance", {0, 0, 64, 64}).status == Status::NotFound,
        "unknown sim id -> NotFound");
  check(resolve_sim(nullptr, {0, 0, 64, 64}).status == Status::Invalid,
        "null sim id -> Invalid");
  check(resolve_sim("", {0, 0, 64, 64}).status == Status::Invalid,
        "empty sim id -> Invalid");
  check(resolve_sim("bronze-dagger", {0, 0, 0, 0}).status == Status::Invalid,
        "degenerate cell -> Invalid");
}

void test_manifest_id_resolve() {
  const ResolveResult result = resolve("dagger_bronze", {0, 0, 64, 64});
  check(result.status == Status::Ok, "manifest id found");
  check(result.entry.category == Category::Weapon, "weapon category");
  check(result.blit.src_w == 344, "source footprint width");
  check(result.blit.dst_w <= 64 && result.blit.dst_h <= 64, "fits cell");

  check(resolve("no_such_art", {0, 0, 32, 32}).status == Status::NotFound,
        "unknown manifest id");
  check(resolve(nullptr, {0, 0, 32, 32}).status == Status::Invalid,
        "null manifest id");
}

void test_aspect_fit_tall_item() {
  const ResolveResult result = resolve("boar_pike", {10, 10, 48, 48});
  check(result.status == Status::Ok, "pike found");
  check(result.blit.dst_h == 48, "height clamped to cell");
  check(result.blit.dst_w < 48, "width scaled down");
}

void test_recoverable_item() {
  const ResolveResult result = resolve("bowl_bronze_offering", {0, 0, 80, 80});
  check(result.status == Status::Ok, "offering bowl");
  check(result.entry.category == Category::Recoverable, "recoverable");
}

void test_catalog_complete() {
  const auto catalog = default_catalog();
  for (const CatalogEntry& entry : catalog) {
    check(entry.valid(), "catalog entry valid");
    check(str_len(entry.id) < kMaxIdLen, "id fits buffer untruncated");
    check(str_len(entry.path) < kMaxPathLen, "path fits buffer untruncated");
  }
  for (std::size_t i = 0; i < catalog.size(); ++i) {
    for (std::size_t j = i + 1; j < catalog.size(); ++j) {
      check(!str_eq(catalog[i].id, catalog[j].id), "catalog ids unique");
    }
  }
}

void test_sim_map_integrity() {
  const auto map = sim_art_map();
  const auto catalog = default_catalog();
  std::size_t mapped = 0;
  for (const SimArtMapping& mapping : map) {
    check(mapping.sim_id[0] != '\0', "sim id nonempty");
    check(str_len(mapping.sim_id) < kMaxIdLen, "sim id fits buffer");
    if (mapping.has_art()) {
      ++mapped;
      check(find_entry(catalog, mapping.manifest_id) != nullptr,
            std::string("mapped manifest id exists in catalog: ") +
                mapping.manifest_id);
    }
  }
  for (std::size_t i = 0; i < map.size(); ++i) {
    for (std::size_t j = i + 1; j < map.size(); ++j) {
      check(!str_eq(map[i].sim_id, map[j].sim_id), "sim ids unique");
    }
  }
  check(mapped == 4, "exactly the four documented concept matches are mapped");
}

void test_zero_width_blit_skipped() {
  // 84x512 chisel into a 512x1 sliver: width would collapse to zero.
  const ResolveResult result = resolve("cur_chisel", {0, 0, 512, 1});
  check(result.status == Status::Invalid, "degenerate blit -> Invalid");
  check(!result.blit.valid(), "no zero-width blit emitted");
  check(result.blit.dst_w == 0 && result.blit.dst_h == 0, "blit stays empty");
}

void test_copy_str_null_terminates() {
  char small[8] = {'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x'};
  copy_str(small, sizeof(small), "abcdefghij");
  check(small[7] == '\0', "overlong copy null-terminated");
  check(std::string(small) == "abcdefg", "overlong copy truncated at cap");

  const CatalogEntry entry = make_entry(
      "this_id_is_way_longer_than_the_thirty_two_byte_buffer",
      Category::Weapon, 1, 1, "p.png");
  check(str_len(entry.id) == kMaxIdLen - 1, "make_entry truncates and terminates");
}

void test_deterministic_checksum() {
  const std::uint32_t a = catalog_checksum();
  const std::uint32_t b = catalog_checksum();
  check(a == b, "checksum stable");
  check(a != 0u, "checksum nonzero");
}

// ── correction 2: manifest drift guard ───────────────────────────────────

struct ManifestItem {
  std::string id;
  std::string category;
  std::string path;
  long w = -1;
  long h = -1;
};

// Minimal dependency-free extraction for the generated manifest. Scans the
// "items" array, splits it into top-level {...} spans (string-aware, so
// nested objects like "footprint" are handled), then pulls exact-quoted
// keys from each span.
bool find_string_field(const std::string& text, std::size_t begin,
                       std::size_t end, const std::string& key,
                       std::string* out) {
  const std::string needle = "\"" + key + "\"";
  const std::size_t key_pos = text.find(needle, begin);
  if (key_pos == std::string::npos || key_pos >= end) return false;
  std::size_t pos = key_pos + needle.size();
  while (pos < end && (text[pos] == ' ' || text[pos] == ':' ||
                       text[pos] == '\t' || text[pos] == '\n' ||
                       text[pos] == '\r')) {
    ++pos;
  }
  if (pos >= end || text[pos] != '"') return false;
  ++pos;
  const std::size_t close = text.find('"', pos);
  if (close == std::string::npos || close > end) return false;
  *out = text.substr(pos, close - pos);
  return true;
}

bool find_number_field(const std::string& text, std::size_t begin,
                       std::size_t end, const std::string& key, long* out) {
  const std::string needle = "\"" + key + "\"";
  const std::size_t key_pos = text.find(needle, begin);
  if (key_pos == std::string::npos || key_pos >= end) return false;
  std::size_t pos = key_pos + needle.size();
  while (pos < end && (text[pos] == ' ' || text[pos] == ':' ||
                       text[pos] == '\t' || text[pos] == '\n' ||
                       text[pos] == '\r')) {
    ++pos;
  }
  long value = 0;
  bool any = false;
  while (pos < end && text[pos] >= '0' && text[pos] <= '9') {
    value = value * 10 + (text[pos] - '0');
    any = true;
    ++pos;
  }
  if (!any) return false;
  *out = value;
  return true;
}

std::vector<ManifestItem> parse_manifest_items(const std::string& text,
                                               std::string* error) {
  std::vector<ManifestItem> items;
  const std::size_t items_key = text.find("\"items\"");
  if (items_key == std::string::npos) {
    *error = "no \"items\" key";
    return items;
  }
  const std::size_t array_start = text.find('[', items_key);
  if (array_start == std::string::npos) {
    *error = "no items array";
    return items;
  }

  int depth = 0;
  bool in_string = false;
  std::size_t obj_start = 0;
  std::vector<std::pair<std::size_t, std::size_t>> spans;
  for (std::size_t i = array_start; i < text.size(); ++i) {
    const char c = text[i];
    if (in_string) {
      if (c == '\\') {
        ++i;
      } else if (c == '"') {
        in_string = false;
      }
      continue;
    }
    if (c == '"') {
      in_string = true;
    } else if (c == '{') {
      if (depth == 0) obj_start = i;
      ++depth;
    } else if (c == '}') {
      --depth;
      if (depth == 0) spans.emplace_back(obj_start, i + 1);
    } else if (c == ']' && depth == 0) {
      break;
    }
  }

  for (const auto& span : spans) {
    ManifestItem item;
    if (!find_string_field(text, span.first, span.second, "id", &item.id)) {
      *error = "item without id";
      return {};
    }
    if (!find_string_field(text, span.first, span.second, "category",
                           &item.category)) {
      *error = "item '" + item.id + "' without category";
      return {};
    }
    if (!find_string_field(text, span.first, span.second, "path", &item.path)) {
      *error = "item '" + item.id + "' without path";
      return {};
    }
    const std::size_t fp = text.find("\"footprint\"", span.first);
    if (fp == std::string::npos || fp >= span.second ||
        !find_number_field(text, fp, span.second, "w", &item.w) ||
        !find_number_field(text, fp, span.second, "h", &item.h)) {
      *error = "item '" + item.id + "' without footprint w/h";
      return {};
    }
    items.push_back(std::move(item));
  }
  if (items.empty()) *error = "no items parsed";
  return items;
}

// The guard itself: every divergence between the header catalog and the
// parsed manifest, in either direction, as human-readable lines.
std::vector<std::string> diff_catalog_vs_manifest(
    const std::vector<ManifestItem>& items) {
  std::vector<std::string> diffs;
  const auto catalog = default_catalog();

  if (items.size() != kCatalogSize) {
    std::ostringstream line;
    line << "item count: manifest has " << items.size() << ", header has "
         << kCatalogSize;
    diffs.push_back(line.str());
  }

  for (const CatalogEntry& entry : catalog) {
    const ManifestItem* found = nullptr;
    for (const ManifestItem& item : items) {
      if (item.id == entry.id) {
        found = &item;
        break;
      }
    }
    if (found == nullptr) {
      diffs.push_back(std::string("header id '") + entry.id +
                      "' missing from manifest");
      continue;
    }
    if (found->category != category_name(entry.category)) {
      diffs.push_back(std::string("category mismatch for '") + entry.id +
                      "': header=" + category_name(entry.category) +
                      " manifest=" + found->category);
    }
    if (found->path != entry.path) {
      diffs.push_back(std::string("filename mismatch for '") + entry.id +
                      "': header=" + entry.path + " manifest=" + found->path);
    }
    if (found->w != static_cast<long>(entry.footprint.width) ||
        found->h != static_cast<long>(entry.footprint.height)) {
      std::ostringstream line;
      line << "footprint mismatch for '" << entry.id << "': header="
           << entry.footprint.width << "x" << entry.footprint.height
           << " manifest=" << found->w << "x" << found->h;
      diffs.push_back(line.str());
    }
  }

  for (const ManifestItem& item : items) {
    bool in_header = false;
    for (const CatalogEntry& entry : catalog) {
      if (item.id == entry.id) {
        in_header = true;
        break;
      }
    }
    if (!in_header) {
      diffs.push_back("manifest id '" + item.id + "' missing from header");
    }
  }
  return diffs;
}

void test_manifest_drift_guard(const std::string& manifest_path) {
  std::ifstream stream(manifest_path, std::ios::binary);
  check(stream.good(), "manifest readable: " + manifest_path);
  std::ostringstream buffer;
  buffer << stream.rdbuf();
  const std::string text = buffer.str();
  check(!text.empty(), "manifest non-empty");

  std::string error;
  const std::vector<ManifestItem> items = parse_manifest_items(text, &error);
  check(!items.empty(), "manifest parsed: " + error);
  check(items.size() == kCatalogSize, "manifest item count matches header");

  // Positive: the header table must match the shipped manifest exactly.
  const std::vector<std::string> diffs = diff_catalog_vs_manifest(items);
  for (const std::string& diff : diffs) {
    std::cerr << "DRIFT: " << diff << "\n";
  }
  check(diffs.empty(), "no drift between header catalog and items/manifest.json");

  // Negative controls: the SAME guard must flag each deliberate mismatch.
  {
    std::vector<ManifestItem> mutated = items;
    mutated[0].w += 1;
    check(!diff_catalog_vs_manifest(mutated).empty(),
          "guard catches footprint drift");
  }
  {
    std::vector<ManifestItem> mutated = items;
    mutated[0].category = (items[0].category == "trophy") ? "weapon" : "trophy";
    check(!diff_catalog_vs_manifest(mutated).empty(),
          "guard catches category drift");
  }
  {
    std::vector<ManifestItem> mutated = items;
    mutated[0].path = "deliberately_wrong.png";
    check(!diff_catalog_vs_manifest(mutated).empty(),
          "guard catches filename drift");
  }
  {
    std::vector<ManifestItem> mutated = items;
    mutated[0].id = "deliberately_renamed";
    check(!diff_catalog_vs_manifest(mutated).empty(),
          "guard catches id drift (both directions)");
  }
  {
    std::vector<ManifestItem> mutated = items;
    mutated.pop_back();
    check(!diff_catalog_vs_manifest(mutated).empty(),
          "guard catches removed item");
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "FAIL: usage: item_art_renderer_tests <path to "
                 "native/client/assets/wizard/items/manifest.json>\n"
                 "The drift guard is mandatory; it cannot be skipped.\n";
    return 1;
  }

  test_sim_ids_end_to_end();
  test_sim_no_art_documented();
  test_sim_unknown_and_invalid();
  test_manifest_id_resolve();
  test_aspect_fit_tall_item();
  test_recoverable_item();
  test_catalog_complete();
  test_sim_map_integrity();
  test_zero_width_blit_skipped();
  test_copy_str_null_terminates();
  test_deterministic_checksum();
  test_manifest_drift_guard(argv[1]);

  std::cout << "TASK-0182 item art renderer acceptance: " << g_checks
            << " checks passed\n";
  return 0;
}
