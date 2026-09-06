#pragma once

// VG-UI-003: paper-doll + comparison are driven by the acknowledged equip
// result. A pending request cannot paint as success.

#include <string>

#include "../paper_doll.hpp"

namespace verdigris::client::ui {

struct EquipView {
  std::string acknowledged_id;
  int acknowledged_atk = 0;
  bool pending = false;
  std::string pending_id;
};

inline void request_equip(EquipView& view, const std::string& id) {
  view.pending = true;
  view.pending_id = id;
}

inline void ack_equip(EquipView& view, const std::string& id, int atk) {
  view.acknowledged_id = id;
  view.acknowledged_atk = atk;
  view.pending = false;
  view.pending_id.clear();
}

inline void reject_equip(EquipView& view) {
  view.pending = false;
  view.pending_id.clear();
}

inline int compare_delta(const EquipView& view, int candidate_atk) {
  return candidate_atk - view.acknowledged_atk;
}

inline int compare_baseline(const EquipView& view, int world_equipped_atk) {
  if (!view.acknowledged_id.empty()) return view.acknowledged_atk;
  if (view.pending) return view.acknowledged_atk;
  return world_equipped_atk;
}

inline bool leaky_pending_as_equipped(const EquipView& view,
                                      const std::string& focus_id) {
  return view.pending && !view.pending_id.empty() && view.pending_id == focus_id;
}

inline bool paint_focus_as_equipped(const EquipView& view, bool world_equipped) {
  if (view.pending) return false;
  return world_equipped;
}

inline bool paints_optimistic_success(const EquipView& view) {
  return view.pending && view.acknowledged_id == view.pending_id &&
         !view.pending_id.empty();
}

inline paper_doll::Status try_slot(paper_doll::State& doll,
                                   const paper_doll::Item& item,
                                   paper_doll::Slot slot) {
  return paper_doll::equip(doll, item, slot);
}

inline const char* owner_ack_only_label() { return "Ack only"; }
inline const char* owner_no_pending_label() { return "No pending"; }

}  // namespace verdigris::client::ui
