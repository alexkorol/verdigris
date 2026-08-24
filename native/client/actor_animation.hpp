// actor_animation.hpp — TASK-0173 presentation-only actor animation state.
//
// Deterministic tick-driven phases for vector/sprite actors: idle, locomotion,
// attack windup/swing/thrust/slam, hit react, death, and recovery. Facing is
// explicit; death is terminal; hit interrupts non-terminal attack/locomotion.
#pragma once

#include <cstdint>

namespace actor_animation {

enum class Facing : std::uint8_t {
  North = 0,
  East,
  South,
  West,
};

enum class Phase : std::uint8_t {
  Idle = 0,
  Locomotion,
  Windup,
  Swing,
  Thrust,
  Slam,
  Hit,
  Recovery,
  Death,
};

enum class AttackKind : std::uint8_t {
  None = 0,
  Swing,
  Thrust,
  Slam,
};

enum class Intent : std::uint8_t {
  Stop,
  Move,
  AttackSwing,
  AttackThrust,
  AttackSlam,
  TakeHit,
  Die,
};

enum class Status : std::uint8_t {
  Ok,
  InvalidTransition,
  Dead,
  InvalidIntent,
};

inline constexpr std::uint8_t kIntentCount =
    static_cast<std::uint8_t>(Intent::Die) + 1;

struct Timing {
  std::uint16_t windup_ticks = 4;
  std::uint16_t swing_ticks = 3;
  std::uint16_t thrust_ticks = 2;
  std::uint16_t slam_ticks = 5;
  std::uint16_t hit_ticks = 3;
  std::uint16_t recovery_ticks = 2;
};

struct State {
  Phase phase = Phase::Idle;
  Facing facing = Facing::South;
  AttackKind pending_attack = AttackKind::None;
  std::uint16_t phase_ticks = 0;

  [[nodiscard]] constexpr bool dead() const { return phase == Phase::Death; }

  [[nodiscard]] constexpr bool operator==(const State&) const = default;
};

[[nodiscard]] constexpr const char* name(Phase phase) {
  switch (phase) {
    case Phase::Idle:
      return "idle";
    case Phase::Locomotion:
      return "locomotion";
    case Phase::Windup:
      return "windup";
    case Phase::Swing:
      return "swing";
    case Phase::Thrust:
      return "thrust";
    case Phase::Slam:
      return "slam";
    case Phase::Hit:
      return "hit";
    case Phase::Recovery:
      return "recovery";
    case Phase::Death:
      return "death";
  }
  return "unknown-phase";
}

[[nodiscard]] constexpr const char* name(Status status) {
  switch (status) {
    case Status::Ok:
      return "ok";
    case Status::InvalidTransition:
      return "invalid-transition";
    case Status::Dead:
      return "dead";
    case Status::InvalidIntent:
      return "invalid-intent";
  }
  return "unknown-status";
}

[[nodiscard]] constexpr Phase attack_phase_for(AttackKind kind) {
  switch (kind) {
    case AttackKind::Swing:
      return Phase::Swing;
    case AttackKind::Thrust:
      return Phase::Thrust;
    case AttackKind::Slam:
      return Phase::Slam;
    default:
      return Phase::Idle;
  }
}

[[nodiscard]] constexpr std::uint16_t attack_ticks_for(const Timing& timing,
                                                     AttackKind kind) {
  switch (kind) {
    case AttackKind::Swing:
      return timing.swing_ticks;
    case AttackKind::Thrust:
      return timing.thrust_ticks;
    case AttackKind::Slam:
      return timing.slam_ticks;
    default:
      return 0;
  }
}

[[nodiscard]] constexpr bool is_attack_visible(const State& state) {
  return state.phase == Phase::Windup || state.phase == Phase::Swing ||
         state.phase == Phase::Thrust || state.phase == Phase::Slam;
}

[[nodiscard]] constexpr bool can_start_attack(const State& state) {
  return !state.dead() && state.phase != Phase::Hit &&
         state.phase != Phase::Recovery && state.phase != Phase::Windup &&
         state.phase != Phase::Swing && state.phase != Phase::Thrust &&
         state.phase != Phase::Slam;
}

[[nodiscard]] constexpr Status begin_attack(State& state, AttackKind kind,
                                          Facing facing) {
  if (state.dead()) return Status::Dead;
  if (!can_start_attack(state)) return Status::InvalidTransition;
  if (kind == AttackKind::None) return Status::InvalidIntent;
  state.phase = Phase::Windup;
  state.pending_attack = kind;
  state.facing = facing;
  state.phase_ticks = 0;
  return Status::Ok;
}

[[nodiscard]] constexpr Status apply_intent(State& state, Intent intent,
                                          Facing facing) {
  if (static_cast<std::uint8_t>(intent) >= kIntentCount) {
    return Status::InvalidIntent;
  }
  if (state.dead()) return Status::Dead;

  switch (intent) {
    case Intent::Die:
      state.phase = Phase::Death;
      state.pending_attack = AttackKind::None;
      state.phase_ticks = 0;
      return Status::Ok;
    case Intent::TakeHit:
      if (state.phase == Phase::Death) return Status::Dead;
      state.phase = Phase::Hit;
      state.pending_attack = AttackKind::None;
      state.phase_ticks = 0;
      return Status::Ok;
    case Intent::Stop:
      if (state.phase == Phase::Locomotion) {
        state.phase = Phase::Idle;
        state.phase_ticks = 0;
      }
      return Status::Ok;
    case Intent::Move:
      if (can_start_attack(state) && state.phase == Phase::Idle) {
        state.phase = Phase::Locomotion;
        state.facing = facing;
        state.phase_ticks = 0;
      } else if (state.phase == Phase::Locomotion) {
        state.facing = facing;
      }
      return Status::Ok;
    case Intent::AttackSwing:
      return begin_attack(state, AttackKind::Swing, facing);
    case Intent::AttackThrust:
      return begin_attack(state, AttackKind::Thrust, facing);
    case Intent::AttackSlam:
      return begin_attack(state, AttackKind::Slam, facing);
  }
  return Status::InvalidIntent;
}

[[nodiscard]] constexpr Status tick(State& state, const Timing& timing) {
  if (state.dead()) return Status::Dead;

  ++state.phase_ticks;

  switch (state.phase) {
    case Phase::Idle:
    case Phase::Locomotion:
    case Phase::Death:
      return Status::Ok;
    case Phase::Windup:
      if (state.phase_ticks >= timing.windup_ticks) {
        state.phase = attack_phase_for(state.pending_attack);
        state.phase_ticks = 0;
      }
      return Status::Ok;
    case Phase::Swing:
    case Phase::Thrust:
    case Phase::Slam:
      if (state.phase_ticks >=
          attack_ticks_for(timing, state.pending_attack)) {
        state.phase = Phase::Recovery;
        state.phase_ticks = 0;
      }
      return Status::Ok;
    case Phase::Hit:
      if (state.phase_ticks >= timing.hit_ticks) {
        state.phase = Phase::Idle;
        state.phase_ticks = 0;
      }
      return Status::Ok;
    case Phase::Recovery:
      if (state.phase_ticks >= timing.recovery_ticks) {
        state.phase = Phase::Idle;
        state.pending_attack = AttackKind::None;
        state.phase_ticks = 0;
      }
      return Status::Ok;
  }
  return Status::InvalidTransition;
}

// Advance `ticks` deterministically; returns final status of last tick.
[[nodiscard]] constexpr Status advance(State& state, const Timing& timing,
                                     std::uint16_t ticks) {
  Status last = Status::Ok;
  for (std::uint16_t i = 0; i < ticks; ++i) {
    last = tick(state, timing);
    if (last == Status::Dead) break;
  }
  return last;
}

}  // namespace actor_animation
