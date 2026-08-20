// render_list.hpp — D-119 presentation recording layer (TASK-0051).
//
// The native client paints straight to GDI today, which makes "was a swing
// drawn / a telegraph circle shown / a damage number spawned" untestable
// without pixel comparison. The harness fixes that by recording SEMANTIC draw
// operations alongside the GDI calls: one RenderList per presented frame,
// which scenarios assert on. It is a presentation diagnostic, never gameplay
// state — the Simulation stays the only source of truth.
#pragma once

#include <string>
#include <vector>

namespace render {

enum class Op {
  Floor,      // label = "tiled" | "flat"; value = 1 when textured tiles drawn
  Tile,       // label = "terrain1" | "terrain4"; x/y = projected tile center
  Scenery,    // label = kind name; x/y = projected screen base point
  Player,     // x/y = projected screen base point
  Monster,    // x/y = projected base point; value = current life
  Telegraph,  // label = "thrust" | "sweep"; x/y = base; radius = pixels
  Swing,      // x/y = projected base point of the swing arc
  Sweep,      // x/y = base; radius = pixels
  WarCry,     // x/y = base; radius = pixels
  Impact,     // x/y = base (hit flash)
  Death,      // x/y = base (death ring)
  Damage,     // x/y = base; value = damage amount; label = "player"|"monster"
  TargetFlash,// x/y = base of the hit target; label = "player"|"monster"
  ScreenPulse,// label = "player-damage" (screen-edge red pulse)
  Drop,       // label = ground item/trophy id; x/y = base
  Extraction, // x/y = projected extraction pad base; radius = pixels
  Hud,        // label = hud text (life/resource/skill strip)
  PaneStat,   // label = stats readout line
  PaneWeapon, // label = equipped weapon name (or "(empty)")
  PaneItem,   // label = backpack cell contents
  PaneBanked, // label = "items N trophies M"
};

struct Item {
  Op op = Op::Scenery;
  double x = 0.0;
  double y = 0.0;
  double radius = 0.0;
  int value = 0;
  std::string label;
};

using List = std::vector<Item>;

inline int count(const List& list, Op op) {
  int total = 0;
  for (const auto& item : list)
    if (item.op == op) ++total;
  return total;
}

inline bool any(const List& list, Op op) { return count(list, op) > 0; }

inline const Item* first(const List& list, Op op) {
  for (const auto& item : list)
    if (item.op == op) return &item;
  return nullptr;
}

}  // namespace render
