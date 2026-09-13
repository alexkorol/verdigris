// chronicles_owner_pane.hpp — TASK-0197 Chronicles Owner journey pane model.
//
// Presentation model for House/Scion/crisis/expedition/recovered-item beats
// without main.cpp integration in this packet.
#pragma once

#include <cstdint>

namespace chronicles_owner_pane {

inline constexpr std::uint8_t kMaxEntries = 16;

enum class EntryKind : std::uint8_t {
  HouseFounded = 0,
  ScionNamed,
  CrisisBrief,
  ExpeditionStarted,
  RecoveredItem,
  InvestmentChoice,
};

struct Entry {
  EntryKind kind = EntryKind::HouseFounded;
  const char* headline = nullptr;
  const char* detail = nullptr;

  [[nodiscard]] constexpr bool operator==(const Entry&) const = default;
};

struct PaneState {
  std::uint8_t count = 0;
  Entry entries[kMaxEntries]{};

  [[nodiscard]] constexpr bool operator==(const PaneState&) const = default;
};

[[nodiscard]] constexpr const char* kind_name(EntryKind kind) {
  switch (kind) {
    case EntryKind::HouseFounded:
      return "house_founded";
    case EntryKind::ScionNamed:
      return "scion_named";
    case EntryKind::CrisisBrief:
      return "crisis_brief";
    case EntryKind::ExpeditionStarted:
      return "expedition_started";
    case EntryKind::RecoveredItem:
      return "recovered_item";
    case EntryKind::InvestmentChoice:
      return "investment_choice";
  }
  return "unknown";
}

[[nodiscard]] constexpr bool push_entry(PaneState& pane, Entry entry) {
  if (pane.count >= kMaxEntries || entry.headline == nullptr) {
    return false;
  }
  pane.entries[pane.count] = entry;
  pane.count = static_cast<std::uint8_t>(pane.count + 1);
  return true;
}

[[nodiscard]] constexpr PaneState make_owner_demo_journey_pane() {
  PaneState pane{};
  push_entry(pane,
             {EntryKind::HouseFounded, "House Ledger founded",
              "A new lineage begins at the Crossroads."});
  push_entry(pane,
             {EntryKind::ScionNamed, "Scion admitted",
              "First mortal oath sworn in the Chronicles."});
  push_entry(pane,
             {EntryKind::CrisisBrief, "Village Palisade threatened",
              "Ash banners crowd the Thornward ridge."});
  push_entry(pane,
             {EntryKind::ExpeditionStarted, "Thornward Ridge entered",
              "Warden of Thornward waits past the ash camp."});
  push_entry(pane,
             {EntryKind::RecoveredItem, "Heirloom recovered",
              "Orun the First's steel battleaxe returned to the House."});
  push_entry(pane,
             {EntryKind::InvestmentChoice, "Countinghouse choice recorded",
              "Scion gear or House production — one path taken."});
  return pane;
}

[[nodiscard]] constexpr bool covers_owner_demo_beats(const PaneState& pane) {
  bool house = false;
  bool scion = false;
  bool crisis = false;
  bool expedition = false;
  bool recovered = false;
  for (std::uint8_t i = 0; i < pane.count; ++i) {
    switch (pane.entries[i].kind) {
      case EntryKind::HouseFounded:
        house = true;
        break;
      case EntryKind::ScionNamed:
        scion = true;
        break;
      case EntryKind::CrisisBrief:
        crisis = true;
        break;
      case EntryKind::ExpeditionStarted:
        expedition = true;
        break;
      case EntryKind::RecoveredItem:
        recovered = true;
        break;
      case EntryKind::InvestmentChoice:
        break;
    }
  }
  return house && scion && crisis && expedition && recovered;
}

}  // namespace chronicles_owner_pane
