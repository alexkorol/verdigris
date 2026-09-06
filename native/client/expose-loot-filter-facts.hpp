#pragma once

// VG-ITEM-006: presentation loot facts and category filter. Hiding a
// category cannot move ownership or change droprate. Core item tables stay
// with Kimi.

#include <string>

namespace verdigris::client::items {

enum class LootFact : unsigned char { Weapon = 0, Trophy, Misc };

struct LootFilter {
  bool show_weapon = true;
  bool show_trophy = true;
  bool show_misc = true;
};

inline LootFact classify_loot(const std::string& id, const std::string& name) {
  std::string lower = name;
  for (char& ch : lower)
    ch = static_cast<char>(ch >= 'A' && ch <= 'Z' ? ch - 'A' + 'a' : ch);
  if (id.rfind("trophy", 0) == 0 || lower.find("trophy") != std::string::npos ||
      lower.find("omen") != std::string::npos)
    return LootFact::Trophy;
  if (lower.find("sword") != std::string::npos ||
      lower.find("dagger") != std::string::npos ||
      lower.find("pike") != std::string::npos ||
      lower.find("spear") != std::string::npos ||
      lower.find("axe") != std::string::npos ||
      lower.find("weapon") != std::string::npos)
    return LootFact::Weapon;
  return LootFact::Misc;
}

inline bool category_visible(const LootFilter& filter, LootFact fact) {
  switch (fact) {
    case LootFact::Weapon:
      return filter.show_weapon;
    case LootFact::Trophy:
      return filter.show_trophy;
    case LootFact::Misc:
      return filter.show_misc;
  }
  return true;
}

inline const char* fact_hud_label(LootFact fact) {
  switch (fact) {
    case LootFact::Weapon:
      return "loot-fact:weapon";
    case LootFact::Trophy:
      return "loot-fact:trophy";
    case LootFact::Misc:
      return "loot-fact:misc";
  }
  return "loot-fact:misc";
}

inline const char* filter_hud_label(const LootFilter& filter) {
  if (filter.show_weapon && filter.show_trophy && filter.show_misc)
    return "loot-filter:all";
  if (!filter.show_trophy && filter.show_weapon && filter.show_misc)
    return "loot-filter:hide:trophy";
  if (!filter.show_weapon && filter.show_trophy && filter.show_misc)
    return "loot-filter:hide:weapon";
  if (!filter.show_misc && filter.show_weapon && filter.show_trophy)
    return "loot-filter:hide:misc";
  return "loot-filter:custom";
}

}  // namespace verdigris::client::items
