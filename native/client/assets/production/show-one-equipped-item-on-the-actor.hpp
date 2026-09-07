#pragma once

#include <cstring>

// VG-ART-005: the world actor attachment is the equipped appearance.
// A paper-doll seat by itself cannot satisfy the review. ITEM identity
// stays with Kimi; this lease is presentation bind only.

namespace verdigris::client::art {

inline bool world_shows_attachment(const char* held_label, int held_value) {
  if (!held_label || held_value == 0) return false;
  return std::strcmp(held_label, "held:none") != 0;
}

inline bool paper_doll_only_fails_review(bool seat_filled, bool world_held) {
  return seat_filled && !world_held;
}

inline const char* owner_world_hold_label() { return "World hold"; }
inline const char* owner_ack_equip_label() { return "Ack equip"; }
inline const char* owner_unarmed_first_label() { return "Unarmed first"; }

inline bool hold_strip_covers_hud_fails_review(bool overlap) { return overlap; }

}  // namespace verdigris::client::art
