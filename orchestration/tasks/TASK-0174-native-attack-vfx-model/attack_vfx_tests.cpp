// attack_vfx_tests.cpp — TASK-0174 acceptance tests.
//
// Self-contained: compiles against native/client/attack_vfx.hpp only.
// Covers arc span coverage, thrust/slam geometry, trail cadence, projectile
// integration determinism (byte-identical runs), impact particle seeding,
// decay/expiry, the visibility invariant under load, deterministic clipping,
// and the negative controls (zero-span / zero-duration / zero-velocity /
// zero-facing emissions must be refused; the visibility checker must reject
// hand-built degenerate elements).

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "attack_vfx.hpp"

using namespace attack_vfx;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

bool approx(float a, float b, float eps) {
  return std::fabs(a - b) <= eps;
}

const Element* find_kind(const Emitter& em, ElementKind kind) {
  for (std::size_t i = 0; i < em.active_count(); ++i) {
    if (em.element(i).kind == kind) {
      return &em.element(i);
    }
  }
  return nullptr;
}

std::size_t count_kind(const Emitter& em, ElementKind kind) {
  std::size_t n = 0;
  for (std::size_t i = 0; i < em.active_count(); ++i) {
    if (em.element(i).kind == kind) {
      ++n;
    }
  }
  return n;
}

void put_bytes(std::vector<std::uint8_t>& out, const void* p, std::size_t n) {
  const auto* b = static_cast<const std::uint8_t*>(p);
  out.insert(out.end(), b, b + n);
}

// Field-by-field serialization: immune to struct padding, sensitive to any
// state a renderer would consume.
std::vector<std::uint8_t> serialize(const Emitter& em) {
  std::vector<std::uint8_t> out;
  const std::uint32_t n = static_cast<std::uint32_t>(em.active_count());
  put_bytes(out, &n, sizeof(n));
  for (std::size_t i = 0; i < em.active_count(); ++i) {
    const Element& e = em.element(i);
    put_bytes(out, &e.kind, sizeof(e.kind));
    put_bytes(out, &e.color, sizeof(e.color));
    put_bytes(out, &e.source_id, sizeof(e.source_id));
    put_bytes(out, &e.seed, sizeof(e.seed));
    put_bytes(out, &e.serial, sizeof(e.serial));
    put_bytes(out, &e.origin.x, sizeof(e.origin.x));
    put_bytes(out, &e.origin.y, sizeof(e.origin.y));
    put_bytes(out, &e.direction.x, sizeof(e.direction.x));
    put_bytes(out, &e.direction.y, sizeof(e.direction.y));
    put_bytes(out, &e.velocity.x, sizeof(e.velocity.x));
    put_bytes(out, &e.velocity.y, sizeof(e.velocity.y));
    put_bytes(out, &e.extent, sizeof(e.extent));
    put_bytes(out, &e.extent_max, sizeof(e.extent_max));
    put_bytes(out, &e.span_half_final, sizeof(e.span_half_final));
    put_bytes(out, &e.alpha, sizeof(e.alpha));
    put_bytes(out, &e.age, sizeof(e.age));
    put_bytes(out, &e.lifetime, sizeof(e.lifetime));
    put_bytes(out, &e.aux_clock, sizeof(e.aux_clock));
    put_bytes(out, &e.auto_impact, sizeof(e.auto_impact));
  }
  const std::uint32_t emitted = em.total_emitted();
  const std::uint32_t expired = em.total_expired();
  const std::uint64_t steps = em.fixed_steps_run();
  put_bytes(out, &emitted, sizeof(emitted));
  put_bytes(out, &expired, sizeof(expired));
  put_bytes(out, &steps, sizeof(steps));
  return out;
}

void drive(Emitter& em, int ticks) {
  for (int i = 0; i < ticks; ++i) {
    em.tick(kFixedDt);
  }
}

// ---------------------------------------------------------------------------
// Arc sweep
// ---------------------------------------------------------------------------

void test_swing_arc_span_coverage_over_time() {
  Emitter em;
  constexpr float kSpanHalf = 1.15f;
  const Status s =
      em.attack_started_ex(AttackKind::Swing, Vec2{0.0f, 0.0f},
                           Vec2{1.0f, 0.0f}, 34.0f, kSpanHalf, 0.28f);
  check(s == Status::Ok, "swing accepted");
  check(em.active_count() == 1, "swing produced exactly one element");

  float prev_extent = -1.0f;
  float peak = 0.0f;
  bool saw_birth_nonzero = false;
  bool saw_midlife_wide = false;
  bool saw_alpha_window = true;
  int live_steps = 0;
  while (em.active_count() > 0) {
    const Element* arc = find_kind(em, ElementKind::ArcSweep);
    check(arc != nullptr, "arc present while alive");
    check(element_visible(*arc), "arc passes visibility contract");
    saw_birth_nonzero = saw_birth_nonzero || arc->extent > 0.0f;
    check(arc->extent >= prev_extent - 1e-6f, "arc span grows monotonically");
    prev_extent = arc->extent;
    peak = peak > arc->extent ? peak : arc->extent;
    if (arc->age >= 0.14f) {
      saw_midlife_wide = saw_midlife_wide || arc->extent >= 0.5f * kSpanHalf;
    }
    saw_alpha_window =
        saw_alpha_window && arc->alpha > kAlphaFloor && arc->alpha <= 1.0f;
    // The arc is centered on facing: facing is covered whenever span > 0.
    check(approx(arc->direction.x, 1.0f, 1e-5f) &&
              approx(arc->direction.y, 0.0f, 1e-5f),
          "arc stays centered on facing direction");
    em.tick(kFixedDt);
    ++live_steps;
  }

  check(saw_birth_nonzero, "arc geometry nonzero from birth");
  check(saw_midlife_wide, "arc covers wide cone around facing by mid-life");
  check(saw_alpha_window, "arc alpha stayed in visible range whole life");
  check(approx(peak, kSpanHalf, 1e-3f), "arc reached full half-span 1.15 rad");
  check(live_steps >= 32 && live_steps <= 36,
        "swing lifetime ~0.28s at 120Hz fixed steps");
  check(em.total_expired() == 1 && em.active_count() == 0,
        "arc expired exactly once");
}

// ---------------------------------------------------------------------------
// Thrust line / slam ring geometry
// ---------------------------------------------------------------------------

void test_thrust_and_slam_geometry() {
  Emitter em;
  check(em.attack_started(AttackKind::Thrust, Vec2{2.0f, 3.0f},
                          Vec2{0.0f, 1.0f}) == Status::Ok,
        "thrust accepted");
  check(em.attack_started(AttackKind::Slam, Vec2{2.0f, 3.0f},
                          Vec2{-1.0f, 0.0f}) == Status::Ok,
        "slam accepted");

  float thrust_peak = 0.0f;
  float slam_min = 1e9f;
  float slam_prev = -1.0f;
  bool thrust_dir_ok = true;
  while (em.active_count() > 0) {
    if (const Element* t = find_kind(em, ElementKind::ThrustLine)) {
      check(element_visible(*t), "thrust visible whole life");
      thrust_dir_ok = thrust_dir_ok && approx(t->direction.x, 0.0f, 1e-5f) &&
                      approx(t->direction.y, 1.0f, 1e-5f);
      thrust_peak = thrust_peak > t->extent ? thrust_peak : t->extent;
    }
    if (const Element* r = find_kind(em, ElementKind::SlamRing)) {
      check(element_visible(*r), "slam ring visible whole life");
      check(r->extent >= slam_prev - 1e-6f, "slam ring expands monotonically");
      slam_prev = r->extent;
      slam_min = slam_min < r->extent ? slam_min : r->extent;
    }
    check(em.visibility_invariant_holds(), "invariant holds during melee mix");
    em.tick(kFixedDt);
  }

  check(thrust_dir_ok, "thrust aligned with facing");
  check(thrust_peak >= 45.0f && thrust_peak <= 47.0f,
        "thrust reached ~46u line length");
  check(slam_min > 0.0f, "slam ring radius positive from birth");
  check(slam_prev >= 55.0f, "slam ring expanded to ~58u by expiry");
}

// ---------------------------------------------------------------------------
// Projectile trail sampling cadence
// ---------------------------------------------------------------------------

void test_projectile_trail_sampling_cadence() {
  Emitter em;
  Emitter::Launch lp;
  lp.origin = Vec2{0.0f, 0.0f};
  lp.velocity = Vec2{120.0f, 0.0f};
  lp.flight_seconds = 0.6f;
  lp.seed = 7;
  lp.auto_impact = false;
  check(em.projectile_launched(lp) == Status::Ok, "projectile accepted");

  std::vector<std::uint32_t> sample_serials;
  std::vector<Vec2> sample_positions;
  for (int i = 0; i < 72; ++i) {  // 0.6s = 72 fixed steps
    em.tick(kFixedDt);
    for (std::size_t j = 0; j < em.active_count(); ++j) {
      const Element& e = em.element(j);
      if (e.kind != ElementKind::TrailSample) {
        continue;
      }
      bool seen = false;
      for (const std::uint32_t ser : sample_serials) {
        seen = seen || ser == e.serial;
      }
      if (!seen) {
        sample_serials.push_back(e.serial);
        sample_positions.push_back(e.origin);
      }
    }
  }

  check(sample_serials.size() >= 22 && sample_serials.size() <= 26,
        "trail sampled at ~40Hz cadence (got " +
            std::to_string(sample_serials.size()) + " samples)");
  check(sample_positions.size() >= 2, "enough samples for spacing check");
  float worst_err = 0.0f;
  for (std::size_t i = 1; i < sample_positions.size(); ++i) {
    const float dx = sample_positions[i].x - sample_positions[i - 1].x;
    const float dy = sample_positions[i].y - sample_positions[i - 1].y;
    const float gap = std::sqrt(dx * dx + dy * dy);
    worst_err = worst_err > std::fabs(gap - 3.0f) ? worst_err
                                                  : std::fabs(gap - 3.0f);
  }
  check(worst_err <= 0.3f, "consecutive trail gaps ~= speed * interval");

  drive(em, 16);  // fp accumulation can land age a hair under flight time
  const Element* body = find_kind(em, ElementKind::ProjectileBody);
  check(body == nullptr, "projectile expired after planned flight");
}

// ---------------------------------------------------------------------------
// Deterministic integration — byte-identical reruns
// ---------------------------------------------------------------------------

void run_full_combat_script(Emitter& em, std::uint32_t seed_override) {
  for (int step = 0; step < 240; ++step) {
    switch (step) {
      case 0:
        (void)em.attack_started(AttackKind::Swing, Vec2{10.0f, 10.0f},
                                Vec2{1.0f, 0.0f});
        break;
      case 10:
        (void)em.attack_started(AttackKind::Thrust, Vec2{10.0f, 10.0f},
                                Vec2{0.7f, 0.7f});
        break;
      case 20:
        (void)em.attack_started(AttackKind::Slam, Vec2{10.0f, 10.0f},
                                Vec2{-1.0f, 0.0f});
        break;
      case 30: {
        Emitter::Launch lp;
        lp.origin = Vec2{12.0f, 12.0f};
        lp.velocity = Vec2{140.0f, -40.0f};
        lp.flight_seconds = 0.8f;
        lp.seed = seed_override;
        lp.source_id = 77;
        (void)em.projectile_launched(lp);
        break;
      }
      case 45: {
        Emitter::Launch lp;
        lp.origin = Vec2{-30.0f, 0.0f};
        lp.velocity = Vec2{-90.0f, 120.0f};
        lp.flight_seconds = 1.1f;
        lp.seed = seed_override + 1;
        (void)em.projectile_launched(lp);
        break;
      }
      case 60:
        (void)em.impact(Vec2{50.0f, 10.0f}, seed_override + 2, 77);
        break;
      case 75:
        (void)em.impact(Vec2{-20.0f, 60.0f}, seed_override + 3);
        break;
      default:
        break;
    }
    em.tick(kFixedDt);
  }
}

void test_projectile_integration_determinism() {
  Emitter a;
  run_full_combat_script(a, 11);
  Emitter b;
  run_full_combat_script(b, 11);

  const std::vector<std::uint8_t> da = serialize(a);
  const std::vector<std::uint8_t> db = serialize(b);
  check(da.size() == db.size() && std::memcmp(da.data(), db.data(),
                                              da.size()) == 0,
        "identical scripts produce byte-identical emitter state");
  check(da.size() > 64, "state snapshot non-trivial");
  check(a.visibility_invariant_holds() && b.visibility_invariant_holds(),
        "both runs satisfy visibility invariant");
  check(a.fixed_steps_run() == 240, "fixed-step counter matches script length");

  Emitter c;
  run_full_combat_script(c, 999);
  const std::vector<std::uint8_t> dc = serialize(c);
  check(std::memcmp(da.data(), dc.data(),
                    da.size() < dc.size() ? da.size() : dc.size()) != 0,
        "control: different seeds actually change the stream (comparison "
        "cannot pass vacuously)");
}

// ---------------------------------------------------------------------------
// Impact particle seeding determinism
// ---------------------------------------------------------------------------

std::vector<Element> burst_sparks(const Emitter& em) {
  std::vector<Element> sparks;
  for (std::size_t i = 0; i < em.active_count(); ++i) {
    const Element& e = em.element(i);
    if (e.kind == ElementKind::ImpactSpark || e.kind == ElementKind::HitFlash) {
      sparks.push_back(e);
    }
  }
  return sparks;
}

bool sparks_equal(const std::vector<Element>& x,
                  const std::vector<Element>& y) {
  if (x.size() != y.size()) {
    return false;
  }
  for (std::size_t i = 0; i < x.size(); ++i) {
    if (x[i].serial != y[i].serial ||
        std::memcmp(&x[i].velocity.x, &y[i].velocity.x, sizeof(float)) != 0 ||
        std::memcmp(&x[i].velocity.y, &y[i].velocity.y, sizeof(float)) != 0 ||
        std::memcmp(&x[i].lifetime, &y[i].lifetime, sizeof(float)) != 0 ||
        std::memcmp(&x[i].extent, &y[i].extent, sizeof(float)) != 0) {
      return false;
    }
  }
  return true;
}

void test_impact_particle_seeding_determinism() {
  Emitter a;
  (void)a.impact(Vec2{5.0f, 5.0f}, 42, 9);
  const std::vector<Element> sa = burst_sparks(a);

  Emitter b;
  (void)b.impact(Vec2{5.0f, 5.0f}, 42, 9);
  const std::vector<Element> sb = burst_sparks(b);

  check(sa.size() == kMaxSparksPerBurst + 1,
        "burst emits 14 sparks + 1 hit flash");
  check(sparks_equal(sa, sb), "same seed reproduces identical burst");

  Emitter c;
  (void)c.impact(Vec2{5.0f, 5.0f}, 43, 9);
  const std::vector<Element> sc = burst_sparks(c);
  bool differs = false;
  for (std::size_t i = 0; i < sa.size() && i < sc.size(); ++i) {
    differs = differs || sa[i].velocity.x != sc[i].velocity.x ||
              sa[i].velocity.y != sc[i].velocity.y;
  }
  check(differs, "different seed yields different spark velocities");
  for (const Element& s : sa) {
    check(s.alpha > 0.0f && s.extent > 0.0f && s.lifetime > 0.0f,
          "every burst element born visible");
    if (s.kind != ElementKind::ImpactSpark) {
      continue;
    }
    const float sp = std::sqrt(s.velocity.x * s.velocity.x +
                               s.velocity.y * s.velocity.y);
    check(sp >= kSparkSpeedMin - 0.01f && sp <= kSparkSpeedMax + 0.01f,
          "spark speed within seeded band");
  }
}

// ---------------------------------------------------------------------------
// Decay removes expired elements
// ---------------------------------------------------------------------------

void test_decay_removes_expired() {
  Emitter em;
  (void)em.attack_started(AttackKind::Swing, Vec2{0.0f, 0.0f},
                          Vec2{1.0f, 0.0f});
  (void)em.attack_started(AttackKind::Slam, Vec2{0.0f, 0.0f},
                          Vec2{1.0f, 0.0f});
  Emitter::Launch lp;
  lp.origin = Vec2{0.0f, 0.0f};
  lp.velocity = Vec2{60.0f, 0.0f};
  lp.flight_seconds = 0.3f;
  lp.seed = 5;
  (void)em.projectile_launched(lp);
  (void)em.impact(Vec2{1.0f, 1.0f}, 6);

  check(em.active_count() > 0, "scene populated before decay");
  drive(em, 480);  // 4s: exceeds every configured lifetime

  check(em.active_count() == 0, "all elements decayed");
  check(em.total_expired() == em.total_emitted(),
        "expired count reconciles with emitted count");
}

// ---------------------------------------------------------------------------
// Visibility invariant under load
// ---------------------------------------------------------------------------

void test_visibility_invariant_under_load() {
  Emitter em;
  run_full_combat_script(em, 31);
  // run_full_combat_script already ticked; now hammer a second window while
  // spawning more bursts to stress mixed lifetimes.
  for (int i = 0; i < 120; ++i) {
    if (i % 30 == 0) {
      (void)em.impact(Vec2{static_cast<float>(i), 4.0f},
                      static_cast<std::uint32_t>(100 + i));
    }
    em.tick(kFixedDt);
    check(em.visibility_invariant_holds(),
          "no zero-geometry element ever observable (step " +
              std::to_string(i) + ")");
    for (std::size_t j = 0; j < em.active_count(); ++j) {
      check(element_visible(em.element(j)),
            "renderer-visible element satisfies contract");
    }
  }
}

// ---------------------------------------------------------------------------
// Negative controls
// ---------------------------------------------------------------------------

void test_negative_control_rejects_degenerate_emissions() {
  Emitter em;
  const std::uint32_t baseline = em.total_emitted();

  // Zero-span swing must be refused outright.
  check(em.attack_started_ex(AttackKind::Swing, Vec2{0.0f, 0.0f},
                             Vec2{1.0f, 0.0f}, 34.0f, 0.0f, 0.28f) ==
            Status::RejectedDegenerate,
        "zero-span swing rejected");
  // Zero-duration melee must be refused.
  check(em.attack_started_ex(AttackKind::Thrust, Vec2{0.0f, 0.0f},
                             Vec2{1.0f, 0.0f}, 46.0f, 0.0f, 0.0f) ==
            Status::RejectedDegenerate,
        "zero-duration thrust rejected");
  // Zero-reach slam must be refused.
  check(em.attack_started_ex(AttackKind::Slam, Vec2{0.0f, 0.0f},
                             Vec2{1.0f, 0.0f}, 0.0f, 0.0f, 0.45f) ==
            Status::RejectedDegenerate,
        "zero-reach slam rejected");
  // Zero/near-zero facing must be refused.
  check(em.attack_started(AttackKind::Swing, Vec2{0.0f, 0.0f},
                          Vec2{0.0f, 0.0f}) == Status::RejectedDegenerate,
        "zero facing rejected");
  // Stationary or zero-duration projectiles must be refused.
  Emitter::Launch lp;
  lp.velocity = Vec2{0.0f, 0.0f};
  lp.flight_seconds = 0.5f;
  check(em.projectile_launched(lp) == Status::RejectedDegenerate,
        "zero-velocity projectile rejected");
  lp.velocity = Vec2{100.0f, 0.0f};
  lp.flight_seconds = 0.0f;
  check(em.projectile_launched(lp) == Status::RejectedDegenerate,
        "zero-duration projectile rejected");

  check(em.total_emitted() == baseline && em.active_count() == 0,
        "rejected emissions never enter the visible list");

  // The visibility checker must bite on hand-built degenerate elements —
  // this is the failing control that keeps the invariant meaningful.
  Element bad{};
  bad.kind = ElementKind::ArcSweep;
  bad.direction = Vec2{1.0f, 0.0f};
  bad.lifetime = 1.0f;
  bad.alpha = 1.0f;
  bad.span_half_final = 1.0f;
  bad.extent = 0.0f;  // zero-span geometry
  check(!element_visible(bad), "control: zero-extent element flagged");

  bad.extent = 0.5f;
  bad.alpha = 0.0f;
  check(!element_visible(bad), "control: zero-alpha element flagged");

  bad.alpha = 1.0f;
  bad.lifetime = 0.0f;
  check(!element_visible(bad), "control: zero-lifetime element flagged");

  bad.lifetime = 1.0f;
  bad.age = 1.0f;  // already at expiry boundary
  check(!element_visible(bad), "control: expired element flagged");

  // Healthy elements must still pass the same checker.
  Emitter good;
  (void)good.attack_started(AttackKind::Swing, Vec2{0.0f, 0.0f},
                            Vec2{1.0f, 0.0f});
  check(element_visible(good.element(0)), "healthy arc passes same checker");
}

// ---------------------------------------------------------------------------
// Deterministic clipping
// ---------------------------------------------------------------------------

void test_clip_circle_culls_deterministically() {
  Emitter em;
  em.set_clip_circle(Vec2{0.0f, 0.0f}, 200.0f);
  Emitter::Launch lp;
  lp.origin = Vec2{100.0f, 0.0f};
  lp.velocity = Vec2{300.0f, 0.0f};
  lp.flight_seconds = 10.0f;  // would fly for ages unclipped
  lp.seed = 3;
  lp.auto_impact = false;
  check(em.projectile_launched(lp) == Status::Ok, "clipped-scene launch ok");

  drive(em, 10);
  check(find_kind(em, ElementKind::ProjectileBody) != nullptr,
        "projectile alive while inside clip circle");
  drive(em, 200);
  check(em.active_count() == 0 && em.total_expired() >= 1,
        "projectile culled after leaving clip circle (well before 10s "
        "natural expiry)");

  // Melee at the protected center must be untouched by clipping.
  Emitter melee;
  melee.set_clip_circle(Vec2{0.0f, 0.0f}, 200.0f);
  (void)melee.attack_started(AttackKind::Swing, Vec2{0.0f, 0.0f},
                             Vec2{1.0f, 0.0f});
  drive(melee, 5);
  check(melee.active_count() == 1, "clip does not disturb centered melee");
}

// ---------------------------------------------------------------------------
// Hygiene
// ---------------------------------------------------------------------------

void test_tick_hygiene() {
  Emitter em;
  (void)em.attack_started(AttackKind::Swing, Vec2{0.0f, 0.0f},
                          Vec2{1.0f, 0.0f});
  em.tick(0.0f);
  em.tick(-1.0f);
  check(em.element(0).age == 0.0f, "non-positive dt is ignored");
  check(em.active_count() == 1, "non-positive dt changes nothing");
}

}  // namespace

int main() {
  test_swing_arc_span_coverage_over_time();
  test_thrust_and_slam_geometry();
  test_projectile_trail_sampling_cadence();
  test_projectile_integration_determinism();
  test_impact_particle_seeding_determinism();
  test_decay_removes_expired();
  test_visibility_invariant_under_load();
  test_negative_control_rejects_degenerate_emissions();
  test_clip_circle_culls_deterministically();
  test_tick_hygiene();

  std::cout << "TASK-0174 attack vfx acceptance: " << g_checks
            << " checks passed\n";
  return 0;
}
