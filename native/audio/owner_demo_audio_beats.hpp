// owner_demo_audio_beats.hpp — TASK-0204 Owner Demo audio beat planner.
//
// Maps presentation events to the seven Owner Demo cue ids without touching
// event_cues.cpp in this packet. Integrators wire these ids into cue_for_event.
#pragma once

#include "presentation_events.hpp"

#include <cstdint>

namespace verdigris::audio::owner_demo {

enum class Beat : std::uint8_t {
  Attack = 0,
  Hit,
  BossDeath,
  LevelUp,
  Gate,
  Loot,
  Menu,
  Count,
};

struct BeatCuePlan {
  Beat beat = Beat::Attack;
  const char* cue_id = nullptr;
  bool maps = false;

  [[nodiscard]] constexpr bool operator==(const BeatCuePlan&) const = default;
};

[[nodiscard]] constexpr const char* beat_name(Beat beat) {
  switch (beat) {
    case Beat::Attack:
      return "attack";
    case Beat::Hit:
      return "hit";
    case Beat::BossDeath:
      return "boss-death";
    case Beat::LevelUp:
      return "level-up";
    case Beat::Gate:
      return "gate";
    case Beat::Loot:
      return "loot";
    case Beat::Menu:
      return "menu";
    case Beat::Count:
      return "invalid";
  }
  return "unknown";
}

[[nodiscard]] constexpr const char* cue_id_for_beat(Beat beat) {
  return beat_name(beat);
}

[[nodiscard]] constexpr bool string_eq(const char* a, const char* b) {
  if (a == nullptr || b == nullptr) {
    return a == b;
  }
  while (*a != '\0' && *b != '\0') {
    if (*a != *b) {
      return false;
    }
    ++a;
    ++b;
  }
  return *a == '\0' && *b == '\0';
}

[[nodiscard]] constexpr bool text_contains(const char* text,
                                         const char* needle) {
  if (text == nullptr || needle == nullptr) {
    return false;
  }
  if (*needle == '\0') {
    return true;
  }
  for (const char* p = text; *p != '\0'; ++p) {
    const char* t = p;
    const char* n = needle;
    while (*t != '\0' && *n != '\0' && *t == *n) {
      ++t;
      ++n;
    }
    if (*n == '\0') {
      return true;
    }
  }
  return false;
}

[[nodiscard]] constexpr BeatCuePlan plan_for_discriminator(
    verdigris::client::PresentationEventType type, const char* text) {
  using verdigris::client::PresentationEventType;
  BeatCuePlan plan{};

  switch (type) {
    case PresentationEventType::AttackStarted:
      plan = {Beat::Attack, cue_id_for_beat(Beat::Attack), true};
      break;
    case PresentationEventType::DamageApplied:
      plan = {Beat::Hit, cue_id_for_beat(Beat::Hit), true};
      break;
    case PresentationEventType::ActorDied:
      if (string_eq(text, "boss") || string_eq(text, "warden") ||
          text_contains(text, "Warden")) {
        plan = {Beat::BossDeath, cue_id_for_beat(Beat::BossDeath), true};
      }
      break;
    case PresentationEventType::ItemDropped:
    case PresentationEventType::ItemPickedUp:
      plan = {Beat::Loot, cue_id_for_beat(Beat::Loot), true};
      break;
    case PresentationEventType::Message:
      if (text_contains(text, "level")) {
        plan = {Beat::LevelUp, cue_id_for_beat(Beat::LevelUp), true};
      } else if (text_contains(text, "gate") || text_contains(text, "Gate")) {
        plan = {Beat::Gate, cue_id_for_beat(Beat::Gate), true};
      } else if (text_contains(text, "menu") || text_contains(text, "Menu")) {
        plan = {Beat::Menu, cue_id_for_beat(Beat::Menu), true};
      }
      break;
    case PresentationEventType::SessionReady:
      plan = {Beat::Menu, cue_id_for_beat(Beat::Menu), true};
      break;
    default:
      break;
  }
  return plan;
}

inline BeatCuePlan plan_for_event(
    const verdigris::client::PresentationEvent& event) {
  return plan_for_discriminator(event.type, event.text.c_str());
}

[[nodiscard]] constexpr bool all_owner_demo_beats_defined() {
  for (std::uint8_t i = 0; i < static_cast<std::uint8_t>(Beat::Count); ++i) {
    const Beat beat = static_cast<Beat>(i);
    if (cue_id_for_beat(beat) == nullptr) {
      return false;
    }
  }
  return true;
}

}  // namespace verdigris::audio::owner_demo
