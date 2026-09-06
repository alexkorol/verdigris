#pragma once

// VG-ACT-007: map authoritative presentation events to one attack beat.
// Swing sprites alone cannot mint a beat. Core resolution stays in
// native/src (Kimi); this header is the client bridge.

#include "presentation_events.hpp"

namespace verdigris::client::combat {

enum class AttackBeat : unsigned char {
  None = 0,
  Anticipate,
  Impact,
  Aftermath,
  Cancel
};

inline const char* beat_hud_label(AttackBeat beat) {
  switch (beat) {
    case AttackBeat::Anticipate:
      return "attack-beat:anticipate";
    case AttackBeat::Impact:
      return "attack-beat:impact";
    case AttackBeat::Aftermath:
      return "attack-beat:aftermath";
    case AttackBeat::Cancel:
      return "attack-beat:cancel";
    case AttackBeat::None:
      break;
  }
  return "attack-beat:none";
}

inline AttackBeat advance_from_event(AttackBeat current, PresentationEventType type) {
  switch (type) {
    case PresentationEventType::AttackStarted:
      return AttackBeat::Anticipate;
    case PresentationEventType::DamageApplied:
      return AttackBeat::Impact;
    case PresentationEventType::ActorDied:
      return AttackBeat::Aftermath;
    default:
      return current;
  }
}

inline AttackBeat dash_cancel(AttackBeat current) {
  return current == AttackBeat::Anticipate ? AttackBeat::Cancel : current;
}

inline const char* owner_beat_label() { return "Attack beat"; }
inline const char* owner_anticipate_beat_label() { return "Anticipate"; }
inline bool fabricated_swing_fails_review(bool event_driven) { return !event_driven; }

}  // namespace verdigris::client::combat
