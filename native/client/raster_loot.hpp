#pragma once

// Physical drops share the authored pixel item art with their inventory and
// equipment families. Display names come from the presentation snapshot; IDs
// are a fallback for snapshots that do not supply a name. This never changes
// ownership, pickup rules, rarity, or filtering.

#include <string>
#include <string_view>

namespace raster_loot {

inline bool word(std::string_view text, std::string_view term) {
  const auto letter = [](char c) {
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
  };
  std::size_t at = 0;
  while ((at = text.find(term, at)) != std::string_view::npos) {
    const std::size_t end = at + term.size();
    if ((at == 0 || !letter(text[at - 1])) &&
        (end == text.size() || !letter(text[end]))) return true;
    at = end;
  }
  return false;
}

inline const char* sprite(const std::string& id, const std::string& name) {
  std::string lower = name.empty() ? id : name;
  for (char& c : lower)
    if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
  const auto has = [&](const char* term) { return word(lower, term); };
  const bool trophy = id.rfind("trophy", 0) == 0 || has("trophy") || has("omen");
  if (trophy) {
    if (has("blood")) return "item_blood";
    if (has("shell") || has("ember")) return "item_shell";
    return "item_bird";
  }
  if (has("shield")) return "item_shield";
  if (has("pike") || has("spear")) return "item_pike";
  if (has("dagger")) return "item_dagger";
  if (has("knife")) return "item_knife";
  if (has("axe") || has("hatchet")) return "weapon_axe";
  if (has("bow")) return "weapon_bow";
  if (has("staff")) return "weapon_staff";
  if (has("club") || has("mace")) return "weapon_club";
  if (has("sword")) return "weapon_sword";
  if (has("boots") || has("sandals")) return "item_boots";
  if (has("armor") || has("armour") || has("plate") || has("cuirass"))
    return "item_armor";
  if (has("chisel")) return "item_chisel";
  if (has("draught")) return "item_draught";
  if (has("orb")) return "item_orb";
  if (has("bowl")) return "item_bowl";
  if (has("vessel")) return "item_vessel";
  if (has("shell")) return "item_shell";
  return "item_bundle";
}

}  // namespace raster_loot
