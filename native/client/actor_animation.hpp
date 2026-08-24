// actor_animation.hpp — TASK-0173 pure presentation animation-state model.
//
// Header-only, dependency-free presentation model for vector/sprite actors.
// States: idle, locomotion (speed-blended), windup, four attack variants
// (swing/thrust/slam/cast) with normalized-time phases, hit reaction, death,
// and recovery. All advancement is event-free: tick(dt) alone moves time, and
// explicit request_* calls propose transitions that follow fixed interruption
// rules (hit interrupts early windup only; death overrides everything and is
// irreversible; a fresh attack chains only from the late recovery window).
// Facing is tracked 8-way. pose() emits renderer-ready numbers (keyframe
// index, phase, limb angles) computed with basic IEEE arithmetic only — no
// libm, no clocks, no I/O, no rendering. Same input sequence always yields
// the same state sequence.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace actor_animation {

enum class StateId : std::uint8_t {
  Idle = 0,
  Locomotion,
  Windup,
  Attack,
  Recovery,
  Hit,
  Death,
};

enum class AttackVariant : std::uint8_t {
  Swing = 0,
  Thrust,
  Slam,
  Cast,
};

inline constexpr std::uint8_t kVariantCount =
    static_cast<std::uint8_t>(AttackVariant::Cast) + 1;

// 8-way facing, clockwise from South, 45 degrees per step.
enum class Facing : std::uint8_t {
  South = 0,
  SouthWest,
  West,
  NorthWest,
  North,
  NorthEast,
  East,
  SouthEast,
};

inline constexpr std::uint8_t kFacingCount = 8;

enum class Status : std::uint8_t {
  Ok,
  InvalidState,
  InvalidArgument,
  Terminal,  // death latched; nothing mutates the actor anymore
};

struct Config {
  std::array<std::uint32_t, kVariantCount> windup_ms{};
  std::array<std::uint32_t, kVariantCount> attack_ms{};
  std::array<std::uint32_t, kVariantCount> recovery_ms{};
  std::uint32_t hit_ms = 0;
  std::uint32_t death_ms = 0;
  std::uint32_t recovery_chain_window_ms = 0;
  std::uint32_t locomotion_stride_ms = 0;
};

[[nodiscard]] constexpr Config default_config() {
  Config c;
  c.windup_ms = {180, 140, 320, 260};
  c.attack_ms = {220, 160, 420, 300};
  c.recovery_ms = {200, 150, 360, 240};
  c.hit_ms = 260;
  c.death_ms = 900;
  c.recovery_chain_window_ms = 120;
  c.locomotion_stride_ms = 600;
  return c;
}

// Every animation duration must be strictly positive so an attack can never
// be invisible-zero-length; the chain window must fit inside every recovery.
[[nodiscard]] constexpr bool valid_config(const Config& c) {
  for (std::size_t i = 0; i < kVariantCount; ++i) {
    if (c.windup_ms[i] == 0 || c.attack_ms[i] == 0 || c.recovery_ms[i] == 0) {
      return false;
    }
    if (c.recovery_chain_window_ms > c.recovery_ms[i]) {
      return false;
    }
  }
  return c.hit_ms > 0 && c.death_ms > 0 && c.locomotion_stride_ms > 0;
}

struct State {
  StateId id = StateId::Idle;
  AttackVariant variant = AttackVariant::Swing;  // meaningful when winding up, attacking, recovering
  Facing facing = Facing::South;
  std::uint32_t elapsed_ms = 0;
  std::uint16_t speed_permille = 0;  // locomotion blend [0, 1000]

  [[nodiscard]] constexpr bool operator==(const State&) const = default;
};

struct Decision {
  Status status = Status::Ok;
  State next{};

  [[nodiscard]] constexpr bool operator==(const Decision&) const = default;
};

// Renderer-facing pose snapshot. Angles are degrees, bob is abstract units.
struct Pose {
  float phase = 0.0f;              // normalized progress in current state [0, 1]
  float facing_degrees = 0.0f;
  std::uint8_t keyframe = 0;       // frame index into the state's keyframe strip
  float root_bob = 0.0f;
  float weapon_arm_deg = 0.0f;
  float off_arm_deg = 0.0f;
  float lean_deg = 0.0f;
};

[[nodiscard]] constexpr const char* name(StateId id) {
  switch (id) {
    case StateId::Idle:
      return "idle";
    case StateId::Locomotion:
      return "locomotion";
    case StateId::Windup:
      return "windup";
    case StateId::Attack:
      return "attack";
    case StateId::Recovery:
      return "recovery";
    case StateId::Hit:
      return "hit";
    case StateId::Death:
      return "death";
  }
  return "unknown-state";
}

[[nodiscard]] constexpr const char* name(AttackVariant variant) {
  switch (variant) {
    case AttackVariant::Swing:
      return "swing";
    case AttackVariant::Thrust:
      return "thrust";
    case AttackVariant::Slam:
      return "slam";
    case AttackVariant::Cast:
      return "cast";
  }
  return "unknown-variant";
}

[[nodiscard]] constexpr const char* name(Facing facing) {
  switch (facing) {
    case Facing::South:
      return "south";
    case Facing::SouthWest:
      return "south-west";
    case Facing::West:
      return "west";
    case Facing::NorthWest:
      return "north-west";
    case Facing::North:
      return "north";
    case Facing::NorthEast:
      return "north-east";
    case Facing::East:
      return "east";
    case Facing::SouthEast:
      return "south-east";
  }
  return "unknown-facing";
}

[[nodiscard]] constexpr const char* name(Status status) {
  switch (status) {
    case Status::Ok:
      return "ok";
    case Status::InvalidState:
      return "invalid-state";
    case Status::InvalidArgument:
      return "invalid-argument";
    case Status::Terminal:
      return "terminal";
  }
  return "unknown-status";
}

[[nodiscard]] constexpr std::uint8_t facing_index(Facing facing) {
  return static_cast<std::uint8_t>(facing);
}

[[nodiscard]] constexpr Facing facing_from_index(std::uint8_t index) {
  return static_cast<Facing>(index % kFacingCount);
}

[[nodiscard]] constexpr Facing rotated(Facing facing, int steps) {
  const int index = static_cast<int>(facing_index(facing)) + steps;
  return facing_from_index(static_cast<std::uint8_t>(
      ((index % kFacingCount) + kFacingCount) % kFacingCount));
}

[[nodiscard]] constexpr bool is_timed(StateId id) {
  return id == StateId::Windup || id == StateId::Attack ||
         id == StateId::Recovery || id == StateId::Hit ||
         id == StateId::Death;
}

[[nodiscard]] constexpr std::uint32_t duration_of(const State& s,
                                                  const Config& c) {
  switch (s.id) {
    case StateId::Windup:
      return c.windup_ms[static_cast<std::size_t>(s.variant)];
    case StateId::Attack:
      return c.attack_ms[static_cast<std::size_t>(s.variant)];
    case StateId::Recovery:
      return c.recovery_ms[static_cast<std::size_t>(s.variant)];
    case StateId::Hit:
      return c.hit_ms;
    case StateId::Death:
      return c.death_ms;
    case StateId::Idle:
    case StateId::Locomotion:
      return 0;
  }
  return 0;
}

[[nodiscard]] constexpr bool valid(const State& s, const Config& c) {
  if (static_cast<std::uint8_t>(s.variant) >= kVariantCount) return false;
  if (facing_index(s.facing) >= kFacingCount) return false;
  if (s.speed_permille > 1000) return false;
  if (s.id == StateId::Death) return true;
  if (!is_timed(s.id)) return true;
  const std::uint32_t duration = duration_of(s, c);
  return duration > 0 && s.elapsed_ms < duration;
}

namespace detail {

constexpr float kInvPermille = 1.0f / 1000.0f;

[[nodiscard]] constexpr float clamp01(float v) {
  if (v < 0.0f) return 0.0f;
  if (v > 1.0f) return 1.0f;
  return v;
}

// Triangle wave on [0,1]: rises to 1 at p=0.5, falls back to 0 at p=1.
[[nodiscard]] constexpr float tri(float p) {
  p = p - static_cast<float>(static_cast<int>(p));
  return p < 0.5f ? p * 2.0f : (1.0f - p) * 2.0f;
}

[[nodiscard]] constexpr float ease_out(float p) {
  const float q = 1.0f - p;
  return 1.0f - q * q;
}

[[nodiscard]] constexpr float ease_in(float p) {
  return p * p;
}

struct VariantCurve {
  std::uint8_t keyframe_count;
  float start_angle;
  float sweep_angle;
  float end_lean;
};

// Distinct duration, keyframe count, easing, and arc per variant guarantee
// pairwise-distinct, strictly-progressing attack phase curves.
[[nodiscard]] constexpr VariantCurve attack_curve(AttackVariant variant) {
  switch (variant) {
    case AttackVariant::Swing:
      return {5, -70.0f, 190.0f, 6.0f};    // linear wide arc
    case AttackVariant::Thrust:
      return {4, -25.0f, 39.0f, 10.0f};    // fast ease-out extension
    case AttackVariant::Slam:
      return {7, -95.0f, 260.0f, 14.0f};   // heavy ease-in overhead
    case AttackVariant::Cast:
      return {6, -60.0f, 108.0f, -3.0f};   // hold then release
  }
  return {1, 0.0f, 0.0f, 0.0f};
}

[[nodiscard]] constexpr float attack_progress(AttackVariant variant, float p) {
  switch (variant) {
    case AttackVariant::Swing:
      return p;
    case AttackVariant::Thrust:
      return ease_out(p);
    case AttackVariant::Slam:
      return ease_in(p);
    case AttackVariant::Cast:
      return p < 0.55f ? 0.15f * (p / 0.55f)
                       : 0.15f + 0.85f * ((p - 0.55f) / 0.45f);
  }
  return p;
}

}  // namespace detail

// Event-free deterministic time advance. Non-positive dt is a no-op. Timed
// states hand their overflow remainder into the successor state so the same
// dt sequence always lands in the same state at the same elapsed time.
[[nodiscard]] constexpr State tick(const State& in, std::int64_t dt_ms,
                                   const Config& c) {
  State s = in;
  if (dt_ms <= 0) return s;
  if (s.id == StateId::Death) {
    // Terminal forever: the clock runs on but never wraps into a replay.
    std::uint64_t t = static_cast<std::uint64_t>(s.elapsed_ms) +
                      static_cast<std::uint64_t>(dt_ms);
    constexpr std::uint64_t kMaxMs = static_cast<std::uint64_t>(~std::uint32_t{0});
    if (t > kMaxMs) t = kMaxMs;
    s.elapsed_ms = static_cast<std::uint32_t>(t);
    return s;
  }
  std::uint64_t t = static_cast<std::uint64_t>(s.elapsed_ms) +
                    static_cast<std::uint64_t>(dt_ms);
  for (;;) {
    if (!is_timed(s.id)) {
      s.elapsed_ms = static_cast<std::uint32_t>(t);
      return s;
    }
    const std::uint64_t duration = duration_of(s, c);
    if (t < duration) {
      s.elapsed_ms = static_cast<std::uint32_t>(t);
      return s;
    }
    t -= duration;
    switch (s.id) {
      case StateId::Windup:
        s.id = StateId::Attack;
        break;
      case StateId::Attack:
        s.id = StateId::Recovery;
        break;
      case StateId::Recovery:
      case StateId::Hit:
        s.id = StateId::Idle;
        break;
      default:
        s.id = StateId::Idle;
        break;
    }
  }
}

[[nodiscard]] constexpr Decision face(const State& in, Facing facing) {
  Decision d;
  if (in.id == StateId::Death) {
    d.status = Status::Terminal;
    d.next = in;
    return d;
  }
  if (in.id != StateId::Idle && in.id != StateId::Locomotion &&
      in.id != StateId::Recovery) {
    // Committed actions lock facing until the action resolves.
    d.status = Status::InvalidState;
    d.next = in;
    return d;
  }
  d.next = in;
  d.next.facing = facing;
  return d;
}

[[nodiscard]] constexpr Decision start_move(const State& in,
                                            std::uint16_t speed_permille) {
  Decision d;
  d.next = in;
  if (in.id == StateId::Death) {
    d.status = Status::Terminal;
    return d;
  }
  if (speed_permille == 0 || speed_permille > 1000) {
    d.status = Status::InvalidArgument;
    return d;
  }
  if (in.id == StateId::Idle || in.id == StateId::Locomotion ||
      in.id == StateId::Recovery) {
    d.next.id = StateId::Locomotion;
    d.next.speed_permille = speed_permille;
    d.next.elapsed_ms = 0;
    return d;
  }
  d.status = Status::InvalidState;
  return d;
}

[[nodiscard]] constexpr Decision stop(const State& in) {
  Decision d;
  d.next = in;
  if (in.id == StateId::Death) {
    d.status = Status::Terminal;
    return d;
  }
  if (in.id == StateId::Locomotion || in.id == StateId::Recovery) {
    d.next.id = StateId::Idle;
    d.next.speed_permille = 0;
    d.next.elapsed_ms = 0;
    return d;
  }
  if (in.id == StateId::Idle) return d;
  d.status = Status::InvalidState;
  return d;
}

// Attacks launch from idle/locomotion, or chain from the late recovery
// window. Active attacks and windups commit: no re-entry mid-swing.
[[nodiscard]] constexpr Decision attack(const State& in, AttackVariant variant,
                                        const Config& c) {
  Decision d;
  d.next = in;
  if (in.id == StateId::Death) {
    d.status = Status::Terminal;
    return d;
  }
  if (static_cast<std::uint8_t>(variant) >= kVariantCount) {
    d.status = Status::InvalidArgument;
    return d;
  }
  if (in.id == StateId::Idle || in.id == StateId::Locomotion) {
    d.next.id = StateId::Windup;
    d.next.variant = variant;
    d.next.speed_permille = 0;
    d.next.elapsed_ms = 0;
    return d;
  }
  if (in.id == StateId::Recovery) {
    const std::uint32_t duration = duration_of(in, c);
    const std::uint32_t remaining = duration - in.elapsed_ms;
    if (remaining <= c.recovery_chain_window_ms) {
      d.next.id = StateId::Windup;
      d.next.variant = variant;
      d.next.elapsed_ms = 0;
      return d;
    }
  }
  d.status = Status::InvalidState;
  return d;
}

// Hit interrupts windup only below the halfway threshold (i-frames aside);
// active attacks absorb hits; death is untouched by anything.
[[nodiscard]] constexpr Decision take_hit(const State& in, const Config& c) {
  Decision d;
  d.next = in;
  switch (in.id) {
    case StateId::Idle:
    case StateId::Locomotion:
    case StateId::Recovery:
      d.next.id = StateId::Hit;
      d.next.speed_permille = 0;
      d.next.elapsed_ms = 0;
      return d;
    case StateId::Windup:
      if (in.elapsed_ms * 2 < duration_of(in, c)) {
        d.next.id = StateId::Hit;
        d.next.elapsed_ms = 0;
        return d;
      }
      d.status = Status::InvalidState;
      return d;
    case StateId::Attack:
    case StateId::Hit:
    case StateId::Death:
      d.status = in.id == StateId::Death ? Status::Terminal : Status::InvalidState;
      return d;
  }
  d.status = Status::InvalidState;
  return d;
}

// Death overrides every state, including mid-attack, and is irreversible.
[[nodiscard]] constexpr Decision die(const State& in) {
  Decision d;
  if (in.id == StateId::Death) {
    d.status = Status::Terminal;
    d.next = in;
    return d;
  }
  d.next = in;
  d.next.id = StateId::Death;
  d.next.speed_permille = 0;
  d.next.elapsed_ms = 0;
  return d;
}

// Renderer-ready pose derived purely from state + config.
[[nodiscard]] constexpr Pose pose(const State& s, const Config& c) {
  Pose p;
  p.facing_degrees = static_cast<float>(facing_index(s.facing)) * 45.0f;
  const float speed = static_cast<float>(s.speed_permille) * detail::kInvPermille;

  switch (s.id) {
    case StateId::Idle: {
      const float breathe = static_cast<float>(s.elapsed_ms % 1600u) / 1600.0f;
      p.phase = breathe;
      p.keyframe = 2;  // two-frame breathing strip
      p.root_bob = 0.4f * detail::tri(breathe) - 0.2f;
      p.weapon_arm_deg = 3.0f;
      p.off_arm_deg = -3.0f;
      break;
    }
    case StateId::Locomotion: {
      const float blend = speed < 0.05f ? 0.05f : speed;
      const float period = static_cast<float>(c.locomotion_stride_ms) / blend;
      const float raw = static_cast<float>(s.elapsed_ms) / period;
      const float gait = raw - static_cast<float>(static_cast<int>(raw));
      p.phase = gait;
      p.keyframe = static_cast<std::uint8_t>(gait * 6.0f * 0.9999f);
      p.root_bob = 1.5f * detail::tri(2.0f * gait) - 0.75f;
      p.weapon_arm_deg = 20.0f * detail::tri(gait) - 10.0f;
      p.off_arm_deg = -(20.0f * detail::tri(gait) - 10.0f);
      p.lean_deg = 5.0f * blend;
      break;
    }
    case StateId::Windup: {
      const float dur = static_cast<float>(duration_of(s, c));
      const float t = detail::clamp01(static_cast<float>(s.elapsed_ms) / dur);
      p.phase = t;
      p.keyframe = static_cast<std::uint8_t>(t * 2.9999f);
      float pullback = 60.0f;
      switch (s.variant) {
        case AttackVariant::Swing: pullback = 70.0f; break;
        case AttackVariant::Thrust: pullback = 25.0f; break;
        case AttackVariant::Slam: pullback = 95.0f; break;
        case AttackVariant::Cast: pullback = 60.0f; break;
      }
      p.weapon_arm_deg = -pullback * t;
      p.off_arm_deg = 12.0f * t;
      p.lean_deg = -4.0f * t;
      break;
    }
    case StateId::Attack: {
      const float dur = static_cast<float>(duration_of(s, c));
      const float t = detail::clamp01(static_cast<float>(s.elapsed_ms) / dur);
      const auto curve = detail::attack_curve(s.variant);
      const float q = detail::attack_progress(s.variant, t);
      p.phase = t;
      p.keyframe = static_cast<std::uint8_t>(
          static_cast<float>(curve.keyframe_count) * q * 0.9999f);
      p.weapon_arm_deg = curve.start_angle + curve.sweep_angle * q;
      p.off_arm_deg = 10.0f * q;
      p.lean_deg = curve.end_lean * q;
      p.root_bob = s.variant == AttackVariant::Slam ? -2.0f * q : 0.0f;
      break;
    }
    case StateId::Recovery: {
      const float dur = static_cast<float>(duration_of(s, c));
      const float t = detail::clamp01(static_cast<float>(s.elapsed_ms) / dur);
      const auto curve = detail::attack_curve(s.variant);
      p.phase = t;
      p.keyframe = static_cast<std::uint8_t>(t * 2.9999f);
      p.weapon_arm_deg = (curve.start_angle + curve.sweep_angle) * (1.0f - t);
      p.off_arm_deg = 10.0f * (1.0f - t);
      p.lean_deg = curve.end_lean * (1.0f - t);
      break;
    }
    case StateId::Hit: {
      const float dur = static_cast<float>(duration_of(s, c));
      const float t = detail::clamp01(static_cast<float>(s.elapsed_ms) / dur);
      p.phase = t;
      p.keyframe = static_cast<std::uint8_t>(t * 2.9999f);
      const float recoil = (1.0f - t) * detail::tri(t < 0.5f ? t * 2.0f : 1.0f);
      p.lean_deg = -18.0f * recoil;
      p.weapon_arm_deg = -14.0f * recoil;
      p.off_arm_deg = -14.0f * recoil;
      p.root_bob = -1.0f * recoil;
      break;
    }
    case StateId::Death: {
      const float dur = static_cast<float>(duration_of(s, c));
      const float t = detail::clamp01(static_cast<float>(s.elapsed_ms) / dur);
      const float q = detail::ease_in(t);
      p.phase = t;  // clamps at 1 and stays; death never loops
      p.keyframe = static_cast<std::uint8_t>(q * 3.9999f);
      p.lean_deg = 88.0f * q;
      p.weapon_arm_deg = 20.0f + 15.0f * q;
      p.off_arm_deg = -(20.0f + 15.0f * q);
      p.root_bob = -6.0f * q;
      break;
    }
  }
  return p;
}

}  // namespace actor_animation
