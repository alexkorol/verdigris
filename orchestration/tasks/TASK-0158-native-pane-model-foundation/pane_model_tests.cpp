// pane_model_tests.cpp — TASK-0158 acceptance tests.
//
// Self-contained: includes only the production header under test
// (native/client/pane_model.hpp, resolved by the harness include path) and
// the standard library. Exits nonzero on the first failed check, mirroring
// the convention of native/tests/core_tests.cpp.

#include <cstdlib>
#include <iostream>
#include <string>

#include "pane_model.hpp"

using namespace pane;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

Rect make_rect(int x, int y, int w, int h) { return Rect{x, y, w, h}; }

bool rects_equal(const Rect& a, const Rect& b) {
  return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

bool scrolls_equal(const Scroll& a, const Scroll& b) {
  return a.row_height == b.row_height && a.rows_total == b.rows_total &&
         a.rows_visible == b.rows_visible &&
         a.content_height_px == b.content_height_px &&
         a.total_height_px == b.total_height_px &&
         a.max_offset_px == b.max_offset_px && a.clipped == b.clipped;
}

bool plans_equal(const Plan& a, const Plan& b) {
  return a.status == b.status && rects_equal(a.reserved_top, b.reserved_top) &&
         rects_equal(a.reserved_bottom, b.reserved_bottom) &&
         rects_equal(a.panel, b.panel) && rects_equal(a.header, b.header) &&
         rects_equal(a.tab_bar, b.tab_bar) && rects_equal(a.content, b.content) &&
         rects_equal(a.footer, b.footer) && scrolls_equal(a.scroll, b.scroll) &&
         a.tab == b.tab;
}

// A degraded plan must carry nothing paintable: every rectangle stays zeroed
// so callers can never emit off-screen geometry from a failed result.
void expect_failed_plan(const Plan& p, Status expected, const std::string& label) {
  check(!p.ok(), label + ": plan reports failure");
  check(p.status == expected, label + ": exact degradation status (" +
                                  p.status_text() + ")");
  const Rect zero{};
  check(rects_equal(p.reserved_top, zero), label + ": reserved_top zeroed");
  check(rects_equal(p.reserved_bottom, zero), label + ": reserved_bottom zeroed");
  check(rects_equal(p.panel, zero), label + ": panel zeroed");
  check(rects_equal(p.header, zero), label + ": header zeroed");
  check(rects_equal(p.tab_bar, zero), label + ": tab_bar zeroed");
  check(rects_equal(p.content, zero), label + ": content zeroed");
  check(rects_equal(p.footer, zero), label + ": footer zeroed");
  check(p.scroll.clipped == false, label + ": no phantom clipping");
}

// Structural contract for a successful plan: everything inside the safe area,
// chrome stacked in order inside the panel, nothing overlapping either
// reserved HUD band.
void expect_healthy_plan(const Plan& p, const Viewport& vp,
                         const std::string& label) {
  check(p.ok(), label + ": plan succeeds");
  check(p.status_text() != nullptr && std::string(p.status_text()) == "ok",
        label + ": status text is ok");
  check(rects_equal(p.reserved_top, make_rect(0, 0, vp.width, 148)),
        label + ": top reserve is the full-width 148px HUD band");
  check(rects_equal(p.reserved_bottom,
                    make_rect(0, vp.height - 96, vp.width, 96)),
        label + ": bottom reserve is the full-width 96px orbs/quickbar band");
  check(p.panel.w >= 340 && p.panel.h >= 280,
        label + ": panel honors minimum size");
  check(p.panel.x >= 16 && p.panel.right() <= vp.width - 16,
        label + ": panel clears side margins");
  check(p.panel.contains(p.header) && p.panel.contains(p.tab_bar) &&
            p.panel.contains(p.content) && p.panel.contains(p.footer),
        label + ": chrome stack stays inside the panel");
  check(rects_equal(p.header, make_rect(p.panel.x, p.panel.y, p.panel.w, 34)),
        label + ": header sits at panel top");
  check(rects_equal(p.tab_bar,
                    make_rect(p.panel.x, p.header.bottom(), p.panel.w, 38)),
        label + ": tab bar follows the header");
  check(rects_equal(
            p.footer,
            make_rect(p.panel.x, p.panel.bottom() - 30, p.panel.w, 30)),
        label + ": footer sits at panel bottom");
  check(p.content.y == p.tab_bar.bottom() && p.content.bottom() == p.footer.y,
        label + ": content fills between tab bar and footer");
  check(p.content.h >= 0, label + ": content height is non-negative");
  const Rect pane_rects[] = {p.panel, p.header, p.tab_bar, p.content, p.footer};
  for (const Rect& r : pane_rects) {
    check(!r.overlaps(p.reserved_top),
          label + ": no pane rectangle overlaps the top HUD band");
    check(!r.overlaps(p.reserved_bottom),
          label + ": no pane rectangle overlaps the bottom orbs/quickbar band");
  }
  check(!p.header.overlaps(p.tab_bar) && !p.tab_bar.overlaps(p.content) &&
            !p.content.overlaps(p.footer) && !p.header.overlaps(p.footer),
        label + ": chrome regions never overlap each other");
  check(p.scroll.row_height == 26 && p.scroll.rows_visible ==
                                         p.content.h / 26,
        label + ": scroll metadata derives from real content height");
}

Request base_request(const Viewport& vp) {
  Request r;
  r.viewport = vp;
  r.tab = Tab::Gear;
  r.minimums = ContentMinimums{};
  r.rows = 24;
  return r;
}

void test_invalid_viewports() {
  const Viewport bad[] = {{0, 600}, {960, 0}, {-1, 600}, {960, -1}, {0, 0}};
  int i = 0;
  for (const Viewport& vp : bad) {
    expect_failed_plan(plan(base_request(vp)), Status::InvalidViewport,
                       "invalid viewport case " + std::to_string(i++));
  }
}

void test_invalid_minimums() {
  Viewport vp{1366, 768};
  auto with_minimums = [&](ContentMinimums m, int rows) {
    Request r = base_request(vp);
    r.minimums = m;
    r.rows = rows;
    return plan(r);
  };
  ContentMinimums preferred_below_min = ContentMinimums{};
  preferred_below_min.preferred_width = 100;
  expect_failed_plan(with_minimums(preferred_below_min, 5),
                     Status::InvalidMinimums, "preferred below min width");
  ContentMinimums zero_min_width = ContentMinimums{};
  zero_min_width.min_width = 0;
  expect_failed_plan(with_minimums(zero_min_width, 5),
                     Status::InvalidMinimums, "zero min width");
  ContentMinimums negative_header = ContentMinimums{};
  negative_header.header_height = -1;
  expect_failed_plan(with_minimums(negative_header, 5),
                     Status::InvalidMinimums, "negative header height");
  ContentMinimums negative_footer = ContentMinimums{};
  negative_footer.footer_height = -4;
  expect_failed_plan(with_minimums(negative_footer, 5),
                     Status::InvalidMinimums, "negative footer height");
  ContentMinimums zero_row_height = ContentMinimums{};
  zero_row_height.row_height = 0;
  expect_failed_plan(with_minimums(zero_row_height, 5),
                     Status::InvalidMinimums, "zero row height");
  expect_failed_plan(with_minimums(ContentMinimums{}, -1),
                     Status::InvalidMinimums, "negative row count");
  ContentMinimums chrome_overflow = ContentMinimums{};
  chrome_overflow.header_height = 200;
  chrome_overflow.tab_bar_height = 200;
  chrome_overflow.footer_height = 200;
  expect_failed_plan(with_minimums(chrome_overflow, 5),
                     Status::InvalidMinimums,
                     "chrome taller than minimum pane");
}

void test_degrade_explicitly() {
  // Width below even the minimum pane: degrade instead of drawing off-screen.
  expect_failed_plan(plan(base_request(Viewport{300, 900})),
                     Status::ViewportTooSmall, "narrow viewport degrades");
  // Safe band shorter than the minimum pane height: degrade explicitly.
  expect_failed_plan(plan(base_request(Viewport{1600, 380})),
                     Status::ViewportTooSmall, "short viewport degrades");
  // The spec's floor resolution with inflated minimums also degrades cleanly.
  Request greedy = base_request(Viewport{960, 600});
  greedy.minimums.preferred_height = 700;
  greedy.minimums.min_height = 700;
  expect_failed_plan(plan(greedy), Status::ViewportTooSmall,
                    "960x600 with impossible minimums degrades");
}

void test_supported_viewports() {
  struct Case {
    Viewport vp;
    int expect_w;
    int expect_h;
    int expect_x;
    int expect_y;
  };
  const Case cases[] = {
      {Viewport{960, 600}, 420, 356, 270, 148},
      {Viewport{1366, 768}, 420, 460, 473, 180},
      {Viewport{1920, 1080}, 420, 460, 750, 336},
  };
  int i = 0;
  for (const Case& c : cases) {
    const std::string label = "viewport " + std::to_string(c.vp.width) + "x" +
                              std::to_string(c.vp.height);
    const Plan p = plan(base_request(c.vp));
    expect_healthy_plan(p, c.vp, label);
    check(rects_equal(p.panel, make_rect(c.expect_x, c.expect_y, c.expect_w,
                                         c.expect_h)),
          label + ": panel lands at the deterministic centered position");
    ++i;
  }
}

void test_shrink_to_fit() {
  // Between min and preferred heights the planner shrinks to the safe band
  // instead of painting past it: 540 - 148 - 96 leaves exactly 296px.
  const Plan p = plan(base_request(Viewport{900, 540}));
  expect_healthy_plan(p, Viewport{900, 540}, "shrink-to-fit viewport");
  check(p.panel.h == 296, "shrink-to-fit: panel height clamps to safe band");
  check(p.panel.bottom() == 540 - 96,
        "shrink-to-fit: panel bottom stops at the orbs/quickbar band");
}

void test_zero_rows() {
  for (const Viewport vp : {Viewport{960, 600}, Viewport{1366, 768},
                            Viewport{1920, 1080}}) {
    Request r = base_request(vp);
    r.rows = 0;
    const Plan p = plan(r);
    expect_healthy_plan(p, vp, "zero rows");
    check(p.scroll.rows_total == 0, "zero rows: total reported as zero");
    check(p.scroll.clipped == false, "zero rows: never clipped");
    check(p.scroll.total_height_px == 0, "zero rows: zero content height");
    check(p.scroll.max_offset_px == 0, "zero rows: no scroll range");
    check(clamped_scroll_offset(p.scroll, 999999) == 0,
          "zero rows: any requested offset clamps to zero");
    check(first_visible_row(p.scroll, 5000) == 0,
          "zero rows: first visible row stays zero");
  }
}

void test_large_row_sets() {
  const Viewport vp{960, 600};
  Request r = base_request(vp);
  r.rows = 1000000;
  const Plan p = plan(r);
  expect_healthy_plan(p, vp, "large row set");
  check(p.scroll.clipped == true, "large row set: marked clipped");
  check(p.scroll.total_height_px == 26000000,
        "large row set: total height is rows * row height");
  check(p.scroll.max_offset_px == 26000000 - p.content.h,
        "large row set: max offset is total minus visible");
  const int last_offset = clamped_scroll_offset(p.scroll, 99999999);
  check(last_offset == p.scroll.max_offset_px,
        "large row set: offsets clamp to the valid range");
  check(clamped_scroll_offset(p.scroll, -50) == 0,
        "large row set: negative offsets clamp to zero");
  const int first = first_visible_row(p.scroll, last_offset);
  check(first + p.scroll.rows_visible <= p.scroll.rows_total,
        "large row set: last page never shows past the final row");
  check((last_offset + p.content.h - 1) / p.scroll.row_height ==
            p.scroll.rows_total - 1,
        "large row set: max scroll reaches the final row");

  // Extreme counts must saturate through 64-bit math without signed overflow.
  Request huge = base_request(vp);
  huge.rows = 2000000000;
  const Plan ph = plan(huge);
  expect_healthy_plan(ph, vp, "overflow-scale row set");
  check(ph.scroll.total_height_px == 2147483647,
        "overflow scale: total height saturates at INT_MAX");
  check(ph.scroll.max_offset_px == 2147483647,
        "overflow scale: max offset saturates safely");
  check(ph.scroll.clipped == true, "overflow scale: clipped");
  check(clamped_scroll_offset(ph.scroll, 2147483647) == 2147483647,
        "overflow scale: clamping stays stable at saturation");
}

void test_tab_stability() {
  for (const Viewport vp : {Viewport{960, 600}, Viewport{1366, 768},
                            Viewport{1920, 1080}}) {
    const Plan gear = plan(base_request(vp));
    Request character_req = base_request(vp);
    character_req.tab = Tab::Character;
    const Plan character = plan(character_req);
    Request passive_req = base_request(vp);
    passive_req.tab = Tab::Passive;
    const Plan passive = plan(passive_req);
    check(rects_equal(gear.panel, character.panel) &&
              rects_equal(gear.panel, passive.panel) &&
              rects_equal(gear.header, character.header) &&
              rects_equal(gear.header, passive.header) &&
              rects_equal(gear.tab_bar, character.tab_bar) &&
              rects_equal(gear.tab_bar, passive.tab_bar) &&
              rects_equal(gear.content, character.content) &&
              rects_equal(gear.content, passive.content) &&
              rects_equal(gear.footer, character.footer) &&
              rects_equal(gear.footer, passive.footer) &&
              scrolls_equal(gear.scroll, character.scroll) &&
              scrolls_equal(gear.scroll, passive.scroll),
          "tab choice never perturbs geometry or scroll metadata");
    check(gear.tab == Tab::Gear && character.tab == Tab::Character &&
              passive.tab == Tab::Passive,
          "each plan echoes its requested active tab");
  }
}

void test_determinism() {
  for (const Viewport vp : {Viewport{960, 600}, Viewport{1366, 768},
                            Viewport{1920, 1080}}) {
    const Request r = base_request(vp);
    const Plan a = plan(r);
    const Plan b = plan(r);
    check(plans_equal(a, b), "identical requests yield identical plans");
    check(std::string(a.status_text()) == std::string(b.status_text()),
          "status text is stable across identical requests");
  }
  // Degraded plans are equally reproducible.
  const Request bad = base_request(Viewport{0, 0});
  check(plans_equal(plan(bad), plan(bad)),
        "failed plans reproduce identically");
}

void run_all() {
  test_invalid_viewports();
  test_invalid_minimums();
  test_degrade_explicitly();
  test_supported_viewports();
  test_shrink_to_fit();
  test_zero_rows();
  test_large_row_sets();
  test_tab_stability();
  test_determinism();
}

}  // namespace

int main() {
  run_all();
  std::cout << "pane model tests: PASS (" << g_checks << " checks)\n";
  return 0;
}
