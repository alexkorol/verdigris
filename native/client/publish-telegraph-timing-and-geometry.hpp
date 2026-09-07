#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "presentation_state.hpp"
#include "verdigris/core.hpp"

// VG-ACT-005: warning window and footprint come from the presentation
// catalog (ticks + reach). A millisecond payload cannot invent a longer
// damaging cone via value/50. Core action scheduling stays with Kimi.

namespace verdigris::client::actions {

inline constexpr int kMaxTickWindow = 16;

struct TelegraphSpec {
  const char* action = "thrust";
  int duration_ticks = verdigris::kTelegraphTicks;
  int reach = 0;
};

inline TelegraphSpec spec_from_payload(const std::string& action, int payload,
                                       const PresentationCatalog& catalog) {
  const bool sweep = action.find("sweep") != std::string::npos;
  const int typed =
      catalog.telegraph_ticks > 0 ? catalog.telegraph_ticks : kTelegraphTicks;
  int duration = typed;
  if (payload > 0 && payload <= kMaxTickWindow) duration = payload;
  TelegraphSpec spec;
  spec.action = sweep ? "sweep" : "thrust";
  spec.duration_ticks = duration;
  spec.reach = sweep ? catalog.melee_range : catalog.thrust_range;
  return spec;
}

inline int guessed_ms_ticks(int payload) {
  return payload > 20 ? payload / 50 : payload;
}

inline bool millisecond_guess_diverges(int payload, const TelegraphSpec& spec) {
  return payload > kMaxTickWindow && guessed_ms_ticks(payload) != spec.duration_ticks;
}

inline void apply_spec(ActiveTelegraph& telegraph, const TelegraphSpec& spec) {
  telegraph.action = spec.action;
  telegraph.windup_ticks = spec.duration_ticks;
  telegraph.reach = spec.reach;
}

inline bool telegraph_expired(std::uint64_t now, const ActiveTelegraph& telegraph) {
  return now >= telegraph.start_tick +
                    static_cast<std::uint64_t>(std::max(1, telegraph.windup_ticks));
}

template <typename Map>
inline int prune_expired_telegraphs(Map& telegraphs, std::uint64_t now) {
  int removed = 0;
  for (auto it = telegraphs.begin(); it != telegraphs.end();) {
    if (telegraph_expired(now, it->second)) {
      it = telegraphs.erase(it);
      ++removed;
    } else {
      ++it;
    }
  }
  return removed;
}

inline bool same_warning(const TelegraphSpec& local, const TelegraphSpec& remote) {
  return local.duration_ticks == remote.duration_ticks && local.reach == remote.reach &&
         std::string(local.action) == remote.action;
}

inline std::string spec_hud(const TelegraphSpec& spec) {
  return std::string("telegraph-spec:") + spec.action + ":ticks:" +
         std::to_string(spec.duration_ticks) + ":reach:" +
         std::to_string(spec.reach);
}

inline const char* owner_action_name(const TelegraphSpec& spec) {
  return std::string(spec.action) == "sweep" ? "Sweep" : "Thrust";
}

inline std::string owner_window_line(const TelegraphSpec& spec) {
  const bool sweep = std::string(spec.action) == "sweep";
  return std::to_string(spec.duration_ticks) + (sweep ? " ticks  melee" : " ticks  reach");
}

inline bool protocol_hud_alone_fails_window_review(bool owner_window_painted) {
  return !owner_window_painted;
}

inline bool millisecond_window_fails_review(int guessed_ticks, int catalog_ticks) {
  return guessed_ticks != catalog_ticks && guessed_ticks > catalog_ticks;
}

inline const char* owner_dodge_clear_label() { return "Dodge clear"; }
inline const char* owner_life_holds_label() { return "Life holds"; }
inline bool dodge_strip_covers_hud_fails_review(bool overlap) {
  return overlap;
}

}  // namespace verdigris::client::actions
