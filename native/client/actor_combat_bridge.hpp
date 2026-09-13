// actor_combat_bridge.hpp — TASK-0186/0187 prep: animation + VFX bridge.
//
// Synchronizes actor_animation phase transitions with attack_vfx primitive
// emission so integrators can drive both models from one presentation step.
// No main.cpp, GPU, or simulation coupling in this packet.
#pragma once

#include "actor_animation.hpp"
#include "attack_vfx.hpp"

namespace actor_combat_bridge {

struct TickResult {
  actor_animation::Status anim_status = actor_animation::Status::Ok;
  attack_vfx::Status vfx_status = attack_vfx::Status::Ok;
  actor_animation::State before{};
  actor_animation::State after{};

  [[nodiscard]] constexpr bool operator==(const TickResult&) const = default;
};

[[nodiscard]] constexpr attack_vfx::Facing map_facing(
    actor_animation::Facing facing) {
  switch (facing) {
    case actor_animation::Facing::North:
      return attack_vfx::Facing::North;
    case actor_animation::Facing::East:
      return attack_vfx::Facing::East;
    case actor_animation::Facing::South:
      return attack_vfx::Facing::South;
    case actor_animation::Facing::West:
      return attack_vfx::Facing::West;
  }
  return attack_vfx::Facing::South;
}

[[nodiscard]] constexpr attack_vfx::Style style_for(
    actor_animation::AttackKind kind) {
  switch (kind) {
    case actor_animation::AttackKind::Swing:
      return attack_vfx::Style::Swing;
    case actor_animation::AttackKind::Thrust:
      return attack_vfx::Style::Thrust;
    case actor_animation::AttackKind::Slam:
      return attack_vfx::Style::Slam;
    default:
      return attack_vfx::Style::Swing;
  }
}

[[nodiscard]] constexpr bool attack_phase_started(
    const actor_animation::State& before,
    const actor_animation::State& after) {
  return before.phase == actor_animation::Phase::Windup &&
         (after.phase == actor_animation::Phase::Swing ||
          after.phase == actor_animation::Phase::Thrust ||
          after.phase == actor_animation::Phase::Slam);
}

[[nodiscard]] constexpr bool hit_phase_started(
    const actor_animation::State& before,
    const actor_animation::State& after) {
  return before.phase != actor_animation::Phase::Hit &&
         after.phase == actor_animation::Phase::Hit;
}

[[nodiscard]] constexpr attack_vfx::Status sync_vfx(
    const actor_animation::State& before,
    const actor_animation::State& after, attack_vfx::Planner& planner,
    const attack_vfx::Config& cfg, std::uint32_t attacker_id,
    attack_vfx::Point origin, std::uint32_t target_id = 0) {
  if (attack_phase_started(before, after)) {
    return attack_vfx::plan_attack(planner, cfg, style_for(after.pending_attack),
                                   attacker_id, map_facing(after.facing),
                                   origin);
  }
  if (hit_phase_started(before, after) && target_id != 0) {
    return attack_vfx::plan_hit_marker(planner, cfg, target_id, origin);
  }
  return attack_vfx::Status::Ok;
}

[[nodiscard]] constexpr TickResult tick_actor(
    actor_animation::State& state, const actor_animation::Timing& timing,
    attack_vfx::Planner& planner, const attack_vfx::Config& cfg,
    std::uint32_t attacker_id, attack_vfx::Point origin,
    std::uint32_t hit_target_id = 0) {
  TickResult result;
  result.before = state;
  result.anim_status = actor_animation::tick(state, timing);
  result.after = state;
  result.vfx_status =
      sync_vfx(result.before, result.after, planner, cfg, attacker_id, origin,
               hit_target_id);
  return result;
}

[[nodiscard]] constexpr TickResult apply_and_tick(
    actor_animation::State& state, actor_animation::Intent intent,
    actor_animation::Facing facing, const actor_animation::Timing& timing,
    attack_vfx::Planner& planner, const attack_vfx::Config& cfg,
    std::uint32_t attacker_id, attack_vfx::Point origin,
    std::uint32_t hit_target_id = 0) {
  TickResult result;
  result.before = state;
  result.anim_status = actor_animation::apply_intent(state, intent, facing);
  result.after = state;
  // TakeHit enters Hit on apply; tick_actor only sees before/after both Hit.
  if (intent == actor_animation::Intent::TakeHit && hit_target_id != 0) {
    result.vfx_status =
        sync_vfx(result.before, result.after, planner, cfg, attacker_id, origin,
                 hit_target_id);
  }
  const TickResult ticked =
      tick_actor(state, timing, planner, cfg, attacker_id, origin, hit_target_id);
  if (ticked.vfx_status != attack_vfx::Status::Ok) {
    result.vfx_status = ticked.vfx_status;
  }
  result.anim_status = ticked.anim_status;
  result.after = ticked.after;
  return result;
}

}  // namespace actor_combat_bridge
