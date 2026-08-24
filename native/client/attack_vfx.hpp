// attack_vfx.hpp — TASK-0174 pure attack VFX presentation model.
//
// Header-only, dependency-free (std only) planner for combat visual effects:
// melee swing arcs, thrust streaks, slam rings, projectile flight with trail
// sampling, impact bursts, and hit flashes. The game layer feeds abstract
// events (attack_started / projectile_launched / impact) and advances time
// with tick(dt); the model integrates every moving piece in deterministic
// fixed steps and exposes a queryable flat list of live visual elements with
// alpha and color-role hints for a renderer.
//
// Hard guarantees:
//   * Determinism: identical event sequences produce byte-identical state.
//     Integration uses a fixed timestep; random-looking quantities (impact
//     spark directions/speeds) come from an embedded splitmix32 stream keyed
//     by caller-supplied seeds, never from wall clocks or RNG libraries.
//   * Visibility invariant: every successfully accepted emission yields
//     nonzero visible geometry (positive extent and alpha) for its entire
//     lifetime; degenerate inputs (zero span, zero duration, zero velocity,
//     zero facing) are rejected at emission time and never enter the element
//     list. The negative-control helpers let tests prove the checker bites.
//   * No rendering, no I/O, no allocation: fixed-capacity storage, no heap,
//     no clocks, no Win32, and zero integration with main.cpp in this packet.
//
// Successor note (TASK-0187 combat VFX integration): drive these calls from
// the client's existing combat event handlers and forward the queried element
// list straight to the sprite/batch layer; color roles map 1:1 to palette
// entries and extents are world units ready for camera projection.
#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace attack_vfx {

// ---------------------------------------------------------------------------
// Tunables (world units / seconds)
// ---------------------------------------------------------------------------

inline constexpr float kFixedDt = 1.0f / 120.0f;
inline constexpr std::size_t kMaxElements = 192;
inline constexpr std::size_t kMaxStepsPerTick = 360;
inline constexpr std::size_t kMaxSparksPerBurst = 14;
inline constexpr float kGoldenAngleRad = 2.39996323f;
inline constexpr float kSparkJitterRad = 0.6f;
inline constexpr float kSparkSpeedMin = 90.0f;
inline constexpr float kSparkSpeedMax = 220.0f;
inline constexpr float kSparkDragPerSecond = 3.0f;
inline constexpr float kSparkLifeBase = 0.42f;
inline constexpr float kSparkLifeJitter = 0.20f;
inline constexpr float kHitFlashLife = 0.16f;
inline constexpr float kHitFlashRadius = 18.0f;
inline constexpr float kTrailInterval = 1.0f / 40.0f;
inline constexpr float kTrailLife = 0.24f;
inline constexpr float kTrailRadius = 4.5f;
inline constexpr float kProjectileRadius = 6.0f;
// Geometry born at t=0 already carries this fraction of its final extent so
// the visibility invariant holds from birth to expiry with no zero frames.
inline constexpr float kBirthExtentRatio = 0.12f;
// Alpha ramps to this small-but-positive floor across the fade window; the
// element is removed exactly at end-of-life, never faded to invisible early.
inline constexpr float kAlphaFloor = 1.0f / 255.0f;
inline constexpr float kFadeWindowFraction = 0.35f;
inline constexpr float kDegenerateEpsilonSq = 1e-10f;

enum class AttackKind : std::uint8_t {
  Swing = 0,
  Thrust,
  Slam,
};

enum class ElementKind : std::uint8_t {
  ArcSweep = 0,
  ThrustLine,
  SlamRing,
  ProjectileBody,
  TrailSample,
  ImpactSpark,
  HitFlash,
};

inline constexpr std::size_t kElementKindCount =
    static_cast<std::size_t>(ElementKind::HitFlash) + 1;

// Palette-role hints only; the renderer owns actual RGBA values.
enum class ColorRole : std::uint8_t {
  SteelWhite = 0,
  ArcCyan,
  ThrustPale,
  SlamAmber,
  TrailBlue,
  SparkOrange,
  HitRed,
};

enum class Status : std::uint8_t {
  Ok = 0,
  RejectedDegenerate,  // zero span/duration/velocity/facing refused up front
  CapacityFull,        // element budget exhausted; nothing partially emitted
};

struct Vec2 {
  float x = 0.0f;
  float y = 0.0f;
};

constexpr Vec2 operator+(const Vec2& a, const Vec2& b) {
  return Vec2{a.x + b.x, a.y + b.y};
}

constexpr Vec2 operator*(const Vec2& a, float s) {
  return Vec2{a.x * s, a.y * s};
}

[[nodiscard]] inline float vec_length_sq(const Vec2& v) {
  return v.x * v.x + v.y * v.y;
}

[[nodiscard]] inline float vec_length(const Vec2& v) {
  return std::sqrt(vec_length_sq(v));
}

[[nodiscard]] inline Vec2 vec_normalized_or_x(const Vec2& v) {
  const float len = vec_length(v);
  if (len <= 0.0f) {
    return Vec2{1.0f, 0.0f};
  }
  return Vec2{v.x / len, v.y / len};
}

struct MeleeTuning {
  float duration = 0.0f;
  float reach = 0.0f;        // swing arc radius / thrust max length / slam max radius
  float span_half_rad = 0.0f;  // swing only: final half-angle of the sweep
};

[[nodiscard]] inline MeleeTuning tuning_for(AttackKind kind) {
  switch (kind) {
    case AttackKind::Thrust:
      return MeleeTuning{0.22f, 46.0f, 0.0f};
    case AttackKind::Slam:
      return MeleeTuning{0.45f, 58.0f, 0.0f};
    case AttackKind::Swing:
    default:
      return MeleeTuning{0.28f, 34.0f, 1.15f};
  }
}

// One renderable primitive. `extent` is the *current* visible size in world
// units: half-angle for arcs, length for thrust lines, radius for rings /
// bodies / particles / flashes. It is strictly positive while the element is
// alive.
struct Element {
  ElementKind kind = ElementKind::ArcSweep;
  ColorRole color = ColorRole::SteelWhite;
  std::uint32_t source_id = 0;
  std::uint32_t seed = 0;      // projectiles: impact seed; others: 0
  std::uint32_t serial = 0;    // emission-order identity (for diffing runs)
  Vec2 origin;                 // anchor; current position for movers
  Vec2 direction;              // unit heading (facing or velocity)
  Vec2 velocity;               // world units/second (movers only)
  float extent = 0.0f;
  float extent_max = 0.0f;
  float span_half_final = 0.0f;  // swing arcs only: final half-angle
  float alpha = 0.0f;
  float age = 0.0f;
  float lifetime = 0.0f;
  float aux_clock = 0.0f;      // projectile trail-sample accumulator
  bool auto_impact = false;    // projectile spawns burst on natural expiry
};

// Per-element visibility contract. Public so tests can use it as the failing
// negative control against hand-built degenerate elements.
[[nodiscard]] inline bool element_visible(const Element& e) {
  if (!(e.lifetime > 0.0f)) {
    return false;
  }
  if (!(e.age >= 0.0f && e.age < e.lifetime)) {
    return false;
  }
  if (!(e.alpha >= kAlphaFloor && e.alpha <= 1.0f)) {
    return false;
  }
  if (!(e.extent > 0.0f)) {
    return false;
  }
    switch (e.kind) {
    case ElementKind::ArcSweep:
      return e.span_half_final > 0.0f && e.extent <= e.span_half_final &&
             vec_length_sq(e.direction) > 0.5f;
    case ElementKind::ThrustLine:
    case ElementKind::SlamRing:
      return e.extent_max > 0.0f && e.extent <= e.extent_max;
    case ElementKind::ProjectileBody:
    case ElementKind::TrailSample:
    case ElementKind::ImpactSpark:
    case ElementKind::HitFlash:
      return true;
    default:
      return false;
  }
}

// ---------------------------------------------------------------------------
// Splitmix32 — tiny deterministic seed stream (no <random>, no libc rand).
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::uint32_t splitmix_next(std::uint32_t& state) {
  state += 0x9E3779B9u;
  std::uint32_t z = state;
  z ^= z >> 16;
  z *= 0x21F0AAADu;
  z ^= z >> 15;
  z *= 0x735A2D97u;
  z ^= z >> 15;
  return z;
}

[[nodiscard]] inline float unit01(std::uint32_t raw) {
  return static_cast<float>(raw >> 8) * (1.0f / 16777216.0f);
}

// ---------------------------------------------------------------------------
// Emitter
// ---------------------------------------------------------------------------

class Emitter {
 public:
  // -- caller-driven events -------------------------------------------------
  [[nodiscard]] Status attack_started(AttackKind kind, const Vec2& origin,
                                      const Vec2& facing,
                                      std::uint32_t source_id = 0) {
    const MeleeTuning tuning = tuning_for(kind);
    return attack_started_ex(kind, origin, facing, tuning.reach,
                             tuning.span_half_rad, tuning.duration, source_id);
  }

  [[nodiscard]] Status attack_started_ex(AttackKind kind, const Vec2& origin,
                                         const Vec2& facing, float reach,
                                         float span_half_rad, float duration,
                                         std::uint32_t source_id = 0) {
    if (!(duration > 0.0f) || !(reach > 0.0f)) {
      return Status::RejectedDegenerate;
    }
    if (vec_length_sq(facing) < kDegenerateEpsilonSq) {
      return Status::RejectedDegenerate;
    }
    Element e;
    e.origin = origin;
    e.direction = vec_normalized_or_x(facing);
    e.lifetime = duration;
    e.extent_max = reach;
    e.source_id = source_id;
    switch (kind) {
      case AttackKind::Swing: {
        if (!(span_half_rad > 0.0f)) {
          return Status::RejectedDegenerate;  // negative control target
        }
        e.kind = ElementKind::ArcSweep;
        e.color = ColorRole::ArcCyan;
        e.extent_max = reach;
        e.span_half_final = span_half_rad;
        break;
      }
      case AttackKind::Thrust: {
        e.kind = ElementKind::ThrustLine;
        e.color = ColorRole::ThrustPale;
        break;
      }
      case AttackKind::Slam:
      default: {
        e.kind = ElementKind::SlamRing;
        e.color = ColorRole::SlamAmber;
        break;
      }
    }
    e.extent = birth_extent(e);
    e.alpha = 1.0f;
    return push(e);
  }

  struct Launch {
    Vec2 origin{};
    Vec2 velocity{};
    float flight_seconds = 0.0f;
    std::uint32_t seed = 1;
    std::uint32_t source_id = 0;
    bool auto_impact = true;  // spawn seeded burst on natural expiry
  };

  [[nodiscard]] Status projectile_launched(const Launch& launch) {
    if (!(launch.flight_seconds >= kFixedDt)) {
      return Status::RejectedDegenerate;  // zero/negligible-duration control
    }
    if (vec_length_sq(launch.velocity) < kDegenerateEpsilonSq) {
      return Status::RejectedDegenerate;  // stationary-projectile control
    }
    Element e;
    e.kind = ElementKind::ProjectileBody;
    e.color = ColorRole::SteelWhite;
    e.source_id = launch.source_id;
    e.seed = launch.seed;
    e.origin = launch.origin;
    e.direction = vec_normalized_or_x(launch.velocity);
    e.velocity = launch.velocity;
    e.extent = kProjectileRadius;
    e.extent_max = kProjectileRadius;
    e.alpha = 1.0f;
    e.lifetime = launch.flight_seconds;
    e.auto_impact = launch.auto_impact;
    return push(e);
  }

  [[nodiscard]] Status impact(const Vec2& origin, std::uint32_t seed,
                              std::uint32_t source_id = 0) {
    return spawn_impact(origin, seed, source_id);
  }

  // Optional renderer-frustum stand-in: elements whose body lies fully outside
  // the circle expire (deterministic clipping). radius <= 0 disables clipping.
  void set_clip_circle(const Vec2& center, float radius) {
    clip_center_ = center;
    clip_radius_ = radius;
  }

  // -- time -----------------------------------------------------------------
  void tick(float dt) {
    if (!(dt > 0.0f)) {
      return;
    }
    accumulator_ += dt;
    std::size_t steps = 0;
    while (accumulator_ >= kFixedDt && steps < kMaxStepsPerTick) {
      accumulator_ -= kFixedDt;
      ++steps;
      step_fixed();
    }
    if (steps == kMaxStepsPerTick && accumulator_ >= kFixedDt) {
      accumulator_ = 0.0f;  // shed backlog; keeps worst-case bounded
    }
  }

  // -- queries --------------------------------------------------------------
  [[nodiscard]] std::size_t active_count() const { return count_; }
  [[nodiscard]] const Element* elements_data() const { return elems_.data(); }
  [[nodiscard]] const Element& element(std::size_t index) const {
    return elems_[index];
  }

  [[nodiscard]] bool visibility_invariant_holds() const {
    for (std::size_t i = 0; i < count_; ++i) {
      if (!element_visible(elems_[i])) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] std::uint32_t total_emitted() const { return emitted_total_; }
  [[nodiscard]] std::uint32_t total_expired() const { return expired_total_; }
  [[nodiscard]] std::uint64_t fixed_steps_run() const { return steps_; }

 private:
  std::array<Element, kMaxElements> elems_{};
  std::size_t count_ = 0;
  Vec2 clip_center_{};
  float clip_radius_ = 0.0f;
  float accumulator_ = 0.0f;
  std::uint32_t next_serial_ = 1;
  std::uint32_t emitted_total_ = 0;
  std::uint32_t expired_total_ = 0;
  std::uint64_t steps_ = 0;

  [[nodiscard]] float birth_extent(const Element& e) const {
    switch (e.kind) {
      case ElementKind::ArcSweep:
        return e.span_half_final * kBirthExtentRatio;
      case ElementKind::ThrustLine:
      case ElementKind::SlamRing:
        return e.extent_max * kBirthExtentRatio;
      default:
        return e.extent_max;
    }
  }

  [[nodiscard]] Status push(const Element& e) {
    if (count_ >= kMaxElements) {
      return Status::CapacityFull;
    }
    elems_[count_] = e;
    elems_[count_].serial = next_serial_;
    ++next_serial_;
    ++count_;
    ++emitted_total_;
    return Status::Ok;
  }

  void remove_at(std::size_t index) {
    if (index + 1 < count_) {
      elems_[index] = elems_[count_ - 1];
    }
    --count_;
    ++expired_total_;
  }

  [[nodiscard]] bool clipped(const Element& e) const {
    if (clip_radius_ <= 0.0f) {
      return false;
    }
    const float dx = e.origin.x - clip_center_.x;
    const float dy = e.origin.y - clip_center_.y;
    const float dist = std::sqrt(dx * dx + dy * dy);
    return dist - e.extent > clip_radius_;
  }

  [[nodiscard]] Status spawn_impact(const Vec2& origin, std::uint32_t seed,
                                    std::uint32_t source_id) {
    std::uint32_t rng = seed ^ 0xA5A5A5A5u;
    if (rng == 0u) {
      rng = 0xBEEFBEEFu;
    }
    for (std::size_t i = 0; i < kMaxSparksPerBurst; ++i) {
      Element spark;
      spark.kind = ElementKind::ImpactSpark;
      spark.color = ColorRole::SparkOrange;
      spark.source_id = source_id;
      const float angle = static_cast<float>(i) * kGoldenAngleRad +
                          (unit01(splitmix_next(rng)) - 0.5f) * kSparkJitterRad;
      const float speed =
          kSparkSpeedMin +
          (kSparkSpeedMax - kSparkSpeedMin) * unit01(splitmix_next(rng));
      spark.direction = Vec2{std::cos(angle), std::sin(angle)};
      spark.velocity = spark.direction * speed;
      spark.origin = origin;
      spark.extent = 3.0f + 2.5f * unit01(splitmix_next(rng));
      spark.extent_max = spark.extent;
      spark.alpha = 1.0f;
      spark.lifetime =
          kSparkLifeBase + kSparkLifeJitter * unit01(splitmix_next(rng));
      const Status s = push(spark);
      if (s != Status::Ok) {
        return s;
      }
    }
    Element flash;
    flash.kind = ElementKind::HitFlash;
    flash.color = ColorRole::HitRed;
    flash.source_id = source_id;
    flash.origin = origin;
    flash.direction = Vec2{1.0f, 0.0f};
    flash.extent = kHitFlashRadius;
    flash.extent_max = kHitFlashRadius;
    flash.alpha = 1.0f;
    flash.lifetime = kHitFlashLife;
    return push(flash);
  }

  void advance(Element& e, float dt) {
    e.age += dt;
    const float t = e.age / e.lifetime;

    switch (e.kind) {
      case ElementKind::ArcSweep: {
        const float grow = t < 0.55f ? t / 0.55f : 1.0f;
        e.extent = e.span_half_final *
                   (kBirthExtentRatio + (1.0f - kBirthExtentRatio) * grow);
        e.alpha = fade_alpha(e.age, e.lifetime);
        break;
      }
      case ElementKind::ThrustLine: {
        float grow = t < 0.30f ? t / 0.30f : 1.0f;
        if (t > 0.70f) {
          grow *= 1.0f - 0.4f * ((t - 0.70f) / 0.30f);
        }
        e.extent = e.extent_max * grow;
        e.alpha = fade_alpha(e.age, e.lifetime);
        break;
      }
      case ElementKind::SlamRing: {
        const float ease = 1.0f - (1.0f - t) * (1.0f - t);
        e.extent = e.extent_max *
                   (kBirthExtentRatio + (1.0f - kBirthExtentRatio) * ease);
        e.alpha = fade_alpha(e.age, e.lifetime);
        break;
      }
      case ElementKind::ProjectileBody: {
        e.origin = e.origin + e.velocity * dt;
        e.alpha = fade_alpha(e.age, e.lifetime);
        break;
      }
      case ElementKind::ImpactSpark: {
        e.origin = e.origin + e.velocity * dt;
        const float keep = 1.0f - kSparkDragPerSecond * dt;
        e.velocity = e.velocity * (keep > 0.0f ? keep : 0.0f);
        e.alpha = fade_alpha(e.age, e.lifetime);
        break;
      }
      case ElementKind::TrailSample:
      case ElementKind::HitFlash:
      default: {
        e.alpha = fade_alpha(e.age, e.lifetime);
        break;
      }
    }
    e.alpha = e.alpha < kAlphaFloor ? kAlphaFloor : e.alpha;
  }

  [[nodiscard]] float fade_alpha(float age, float lifetime) const {
    const float fade_start = lifetime * (1.0f - kFadeWindowFraction);
    if (age <= fade_start || fade_start >= lifetime) {
      return 1.0f;
    }
    const float fade_t = (age - fade_start) / (lifetime - fade_start);
    const float a = 1.0f - fade_t * (1.0f - kAlphaFloor);
    return a < kAlphaFloor ? kAlphaFloor : a;
  }

  void step_fixed() {
    ++steps_;
    std::size_t i = 0;
    while (i < count_) {
      Element& e = elems_[i];

      if (e.kind == ElementKind::ProjectileBody) {
        e.aux_clock += kFixedDt;
        if (e.aux_clock >= kTrailInterval) {
          e.aux_clock -= kTrailInterval;
          Element dot;
          dot.kind = ElementKind::TrailSample;
          dot.color = ColorRole::TrailBlue;
          dot.source_id = e.source_id;
          dot.origin = e.origin;
          dot.direction = e.direction;
          dot.extent = kTrailRadius;
          dot.extent_max = kTrailRadius;
          dot.alpha = 1.0f;
          dot.lifetime = kTrailLife;
          (void)push(dot);  // budget-full drops the dot; body remains correct
        }
      }

      advance(e, kFixedDt);

      bool dead = !(e.age < e.lifetime);
      if (!dead && clipped(e)) {
        dead = true;
      }
      if (dead) {
        const bool auto_burst =
            e.kind == ElementKind::ProjectileBody && e.auto_impact;
        const Vec2 at = e.origin;
        const std::uint32_t seed = e.seed;
        const std::uint32_t src = e.source_id;
        remove_at(i);
        if (auto_burst) {
          (void)spawn_impact(at, seed, src);
        }
        continue;  // re-examine swapped-in slot
      }
      ++i;
    }
  }
};

}  // namespace attack_vfx
