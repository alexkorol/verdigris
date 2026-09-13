#pragma once

#include <cstdint>
#include <string>
#include <utility>

// VG-UI-002: pack-grid drag is presentation occupancy. A rejected drop
// cannot lose, duplicate, or silently equip. Core inventory-move stays
// with Kimi; this lease does not invent Command::inventory_move.

namespace verdigris::client::ui {

enum class PackDrop { Idle, Ok, Reject, EquipRequest };

inline PackDrop classify_pack_drop(bool drag_live, bool preview_ok,
                                   bool onto_weapon, bool known_id) {
  if (!drag_live) return PackDrop::Idle;
  if (onto_weapon) return known_id ? PackDrop::EquipRequest : PackDrop::Reject;
  return preview_ok ? PackDrop::Ok : PackDrop::Reject;
}

inline bool reject_loses_or_duplicates(PackDrop drop, int count_before,
                                       int count_after) {
  return drop == PackDrop::Reject && count_before != count_after;
}

inline bool reject_silently_equips(PackDrop drop, bool equipped_after) {
  return drop == PackDrop::Reject && equipped_after;
}

inline const char* pack_drop_hud(PackDrop drop) {
  switch (drop) {
    case PackDrop::Ok:
      return "pack-drop:ok";
    case PackDrop::Reject:
      return "pack-drop:reject";
    case PackDrop::EquipRequest:
      return "pack-drop:equip";
    case PackDrop::Idle:
    default:
      return "pack-drop:idle";
  }
}

inline const char* owner_pack_place_label() { return "Pack place"; }
inline const char* owner_reject_keeps_label() { return "Reject keeps"; }

// A 12-char period clip ("Ember-edged.") cannot certify the carried item.
// Wrap at the last space so both owner words stay at the type floor.
inline bool period_clipped_pack_name_fails_review(bool period_clip) {
  return period_clip;
}

inline bool pack_caption_is_period_clip(const std::string& full,
                                        const std::string& painted) {
  if (painted.empty() || painted == full) return false;
  std::string stem = painted;
  if (stem.size() >= 4 && stem.compare(stem.size() - 4, 4, " [E]") == 0)
    stem.resize(stem.size() - 4);
  if (stem.empty() || stem.back() != '.') return false;
  stem.pop_back();
  return full.rfind(stem, 0) == 0 && stem.size() < full.size();
}

inline std::pair<std::string, std::string> wrap_pack_caption(
    const std::string& name) {
  const auto space = name.rfind(' ');
  if (space == std::string::npos || space == 0 || space + 1 >= name.size())
    return {name, {}};
  return {name.substr(0, space), name.substr(space + 1)};
}

}  // namespace verdigris::client::ui
