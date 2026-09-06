#pragma once

// TASK-0108 client stage (Cursor lease): a JS-parity world:projectile windup
// becomes the existing Telegraph render op. No new Op, no projectile art,
// no remote_session.cpp parse (SPEC-frozen successor), no monster:telegraph.

#include <algorithm>
#include <cmath>
#include <string>

#include "presentation_events.hpp"
#include "presentation_state.hpp"
#include "verdigris/core.hpp"

namespace verdigris::client::projectile {

inline constexpr int kAuthoredVolleyTravelMs = 800;

inline bool is_projectile_warning(const PresentationEvent& event) {
  if (event.type != PresentationEventType::Telegraph) return false;
  if (event.text.find("boss:") != std::string::npos) return false;
  if (event.text.find("ground-slam") != std::string::npos) return false;
  return event.text == "projectile" || event.style == "ranged:volley" ||
         event.text.find("ranged:volley") != std::string::npos;
}

inline bool js_payload_shape(int travel_ms, const std::string& kind) {
  return travel_ms > 0 && kind == "monster";
}

inline int windup_ticks_from_travel_ms(int travel_ms) {
  if (travel_ms <= 0) return kTelegraphTicks;
  return (std::max)(1, travel_ms / kSimulationTickMs);
}

inline PresentationEvent from_js_payload(const std::string& actor_id, int from_x,
                                         int from_y, int to_x, int to_y,
                                         int travel_ms, const char* kind) {
  PresentationEvent event;
  event.type = PresentationEventType::Telegraph;
  event.actor_id = actor_id;
  event.text = "projectile";
  event.style = "ranged:volley";
  event.value = travel_ms;
  event.from_x = from_x;
  event.from_y = from_y;
  event.to_x = to_x;
  event.to_y = to_y;
  (void)kind;
  return event;
}

inline void apply_warning(PresentationFx& fx, const WorldView& world,
                          const PresentationEvent& event, std::uint64_t now_tick) {
  ActiveTelegraph telegraph;
  telegraph.actor_id = event.actor_id;
  telegraph.action = "projectile";
  telegraph.start_tick = now_tick;
  telegraph.windup_ticks = windup_ticks_from_travel_ms(event.value);
  const int dx = event.to_x - event.from_x;
  const int dy = event.to_y - event.from_y;
  telegraph.facing = {dx < 0 ? -1 : dx > 0 ? 1 : 0, dy < 0 ? -1 : dy > 0 ? 1 : 0};
  if (event.from_x != 0 || event.from_y != 0) {
    telegraph.position = {static_cast<int>(std::lround(protocol_to_world(event.from_x))),
                          static_cast<int>(std::lround(protocol_to_world(event.from_y)))};
  } else {
    telegraph.position = world.player.position;
    for (const auto& monster : world.monsters) {
      if (monster.id == event.actor_id) {
        telegraph.position = monster.position;
        break;
      }
    }
  }
  const int chebyshev = (std::max)(std::abs(dx), std::abs(dy));
  telegraph.reach = chebyshev > 0
                        ? static_cast<int>(std::lround(protocol_to_world(chebyshev)))
                        : verdigris::Simulation::presentation_catalog().thrust_range;
  const std::string key = event.actor_id.empty() ? "foe" : event.actor_id;
  fx.telegraphs[key] = std::move(telegraph);
}

inline std::string hud_chip(const ActiveTelegraph& telegraph) {
  return std::string("ranged-warning:") + telegraph.action + ":ticks:" +
         std::to_string(telegraph.windup_ticks);
}

}  // namespace verdigris::client::projectile
