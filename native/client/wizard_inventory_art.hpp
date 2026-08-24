// wizard_inventory_art.hpp — Owner Demo integration of the WIZARD RPG
// Inventory item art (TASK-0169 pack + coordinator additions) into the gear
// pane: token-matched art registry, slot nine-slice cells, and paper-doll
// seat art. Presentation only; the authoritative item data and interaction
// contract stay in the simulation and existing pane code.
#pragma once

#include "wizard_orb_art.hpp"
#include "wizard_splash_art.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace wizard_inventory_art {

using wizard_orb_art::OrbBitmap;
using wizard_orb_art::GdiPlusProcs;
using wizard_orb_art::load_orb_plate;
using wizard_orb_art::release_orb_bitmap;
using wizard_orb_art::AlphaBlendProc;
using wizard_splash_art::stretch_alpha;

struct ArtEntry {
  const char* file;
  const char* tokens;  // comma-separated lowercase name tokens
};

// Registry order matters: first token match wins, so specific entries precede
// generic ones. All files are real WIZARD rpg_inventory raster art.
inline const ArtEntry kArtRegistry[] = {
    {"war_axe.png", "axe"},
    {"greataxe_bronze.png", "greataxe"},
    {"handaxe_flint.png", "hatchet"},
    {"macuahuitl_obsidian.png", "macuahuitl"},
    {"sword_flint.png", "sword,longsword,blade"},
    {"spear_bone.png", "spear,lance"},
    {"boar_pike.png", "pike"},
    {"warclub_bone.png", "club"},
    {"dagger_bronze.png", "dagger"},
    {"cur_knife.png", "knife"},
    {"astral_plate.png", "plate,cuirass,armor,armour,hauberk"},
    {"boots_fur.png", "boot,boots,greaves,sandals"},
    {"bowl_bronze_offering.png", "bowl,offering"},
    {"cur_chisel.png", "chisel"},
    {"cur_draught.png", "draught,potion,elixir,vial"},
    {"cur_orb.png", "orb,sphere,relic"},
    {"bird_omen.png", "bird,feather"},
    {"blood_omen.png", "blood"},
    {"ember_shell.png", "shell"},
};

struct InventoryArtSet {
  std::vector<OrbBitmap> art;
  std::vector<std::string> files;
  OrbBitmap slot;    // framekit 32x32 slot texture for grid cells
  OrbBitmap panel;   // framekit 48x48 panel texture for the pane frame
  bool slot_loaded = false;
  bool panel_loaded = false;

  [[nodiscard]] bool ok() const { return !art.empty(); }
};

inline std::string lower_copy(const std::string& text) {
  std::string out = text;
  std::transform(out.begin(), out.end(), out.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return out;
}

inline constexpr std::size_t kRegistryCount =
    sizeof(kArtRegistry) / sizeof(kArtRegistry[0]);

inline bool load_inventory_art_set(const GdiPlusProcs& procs,
                                   InventoryArtSet* set) {
  char exe_buffer[MAX_PATH];
  const DWORD exe_len = GetModuleFileNameA(nullptr, exe_buffer, MAX_PATH);
  std::string executable;
  if (exe_len > 0 && exe_len < MAX_PATH) {
    std::string path(exe_buffer, exe_len);
    const std::size_t slash = path.find_last_of("\\/");
    executable = slash == std::string::npos ? "." : path.substr(0, slash);
  }
  std::vector<std::string> bases;
  if (!executable.empty()) {
    bases.push_back(executable);
    std::string prefix = executable;
    for (int depth = 1; depth <= 6; ++depth) {
      prefix += "\\..";
      bases.push_back(prefix);
    }
  }
  bases.push_back(".");
  std::string walk = ".";
  for (int depth = 0; depth <= 4; ++depth) {
    bases.push_back(walk);
    walk += "\\..";
  }
  auto directory_exists = [](const std::string& path) {
    DWORD attributes = GetFileAttributesA(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES &&
           (attributes & FILE_ATTRIBUTE_DIRECTORY);
  };

  for (const auto& base : bases) {
    const std::string root = base + "\\native\\client\\assets\\wizard\\items";
    if (!directory_exists(root)) continue;
    for (std::size_t i = 0; i < kRegistryCount; ++i) {
      const ArtEntry& entry = kArtRegistry[i];
      OrbBitmap plate;
      if (load_orb_plate(procs, root + "\\" + entry.file, &plate)) {
        set->art.push_back(plate);
        set->files.push_back(entry.file);
      }
    }
    for (const auto& candidate : bases) {
      const std::string framekit =
          candidate + "\\native\\client\\assets\\wizard\\framekit\\textures";
      if (!set->slot_loaded &&
          load_orb_plate(procs, framekit + "\\slot.png", &set->slot)) {
        set->slot_loaded = true;
      }
      if (!set->panel_loaded &&
          load_orb_plate(procs, framekit + "\\panel.png", &set->panel)) {
        set->panel_loaded = true;
      }
      if (set->slot_loaded && set->panel_loaded) break;
    }
    return set->ok();
  }
  return false;
}

inline void release_inventory_art_set(InventoryArtSet* set) {
  for (OrbBitmap& plate : set->art) release_orb_bitmap(&plate);
  set->art.clear();
  set->files.clear();
  release_orb_bitmap(&set->slot);
  set->slot_loaded = false;
  release_orb_bitmap(&set->panel);
  set->panel_loaded = false;
}

// Picks the registry index whose token list matches the item name; falls
// back to the curio orb so every item presents real art.
inline int pick_art_for_name(const InventoryArtSet& set,
                             const std::string& item_name) {
  if (set.art.empty()) return -1;
  const std::string lower = lower_copy(item_name);
  for (std::size_t i = 0; i < kRegistryCount && i < set.art.size(); ++i) {
    std::string tokens = lower_copy(kArtRegistry[i].tokens);
    std::size_t start = 0;
    while (start <= tokens.size()) {
      const std::size_t comma = tokens.find(',', start);
      const std::string token = comma == std::string::npos
                                    ? tokens.substr(start)
                                    : tokens.substr(start, comma - start);
      if (!token.empty() && lower.find(token) != std::string::npos) {
        return static_cast<int>(i);
      }
      if (comma == std::string::npos) break;
      start = comma + 1;
    }
  }
  return static_cast<int>(set.art.size() - 1);
}

// Aspect-fit draw inside a cell with 2px inset.
inline void draw_item_art_fit(HDC dst, AlphaBlendProc alpha_blend,
                              const OrbBitmap& plate, int x, int y, int w,
                              int h) {
  if (!alpha_blend || !plate.ok() || w <= 4 || h <= 4) return;
  const int box_w = w - 4;
  const int box_h = h - 4;
  int dest_w = box_w;
  int dest_h = plate.height * box_w / plate.width;
  if (dest_h > box_h) {
    dest_h = box_h;
    dest_w = plate.width * box_h / plate.height;
  }
  const int dest_x = x + 2 + (box_w - dest_w) / 2;
  const int dest_y = y + 2 + (box_h - dest_h) / 2;
  stretch_alpha(dst, alpha_blend, plate, dest_x, dest_y, dest_w, dest_h, 255);
}

}  // namespace wizard_inventory_art
