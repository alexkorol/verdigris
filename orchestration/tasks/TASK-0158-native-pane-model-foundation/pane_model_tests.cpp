// pane_model_tests.cpp — TASK-0158 acceptance tests for the pure pane model.
// Self-contained: no framework, no I/O beyond stdout/stderr, exit code is the
// verdict. Compiled by run-tests.ps1 against the production header only.

#include "pane_model.hpp"

#include <cstdio>
#include <string>

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool condition, const std::string& what) {
  ++g_checks;
  if (!condition) {
    ++g_failures;
    std::fprintf(stderr, "FAIL: %s\n", what.c_str());
  }
}

void check_eq(int actual, int expected, const std::string& what) {
  ++g_checks;
  if (actual != expected) {
    ++g_failures;
    std::fprintf(stderr, "FAIL: %s (actual %d, expected %d)\n",
                 what.c_str(), actual, expected);
  }
}

std::string where(const char* context, const char* detail) {
  return std::string(context) + ": " + detail;
}

bool rects_equal(const pane_model::Rect& a, const pane_model::Rect& b) {
  return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

bool plans_geometry_equal(const pane_model::Plan& a, const pane_model::Plan& b) {
  if (a.ok != b.ok || a.failure != b.failure || a.degraded != b.degraded) {
    return false;
  }
  if (!rects_equal(a.panel, b.panel) || !rects_equal(a.header, b.header) ||
      !rects_equal(a.tab_bar, b.tab_bar) || !rects_equal(a.content, b.content) ||
      !rects_equal(a.footer, b.footer) ||
      !rects_equal(a.reserved_top, b.reserved_top) ||
      !rects_equal(a.reserved_bottom, b.reserved_bottom)) {
    return false;
  }
  for (std::size_t i = 0; i < a.tabs.size(); ++i) {
    if (!rects_equal(a.tabs[i].rect, b.tabs[i].rect)) {
      return false;
    }
  }
  return true;
}

bool plans_fully_equal(const pane_model::Plan& a, const pane_model::Plan& b) {
  if (!plans_geometry_equal(a, b)) {
    return false;
  }
  const pane_model::ScrollInfo& sa = a.scroll;
  const pane_model::ScrollInfo& sb = b.scroll;
  return sa.clipped == sb.clipped &&
         sa.visible_rows == sb.visible_rows &&
         sa.first_visible_row == sb.first_visible_row &&
         sa.scroll_offset_px == sb.scroll_offset_px &&
         sa.max_scroll_offset_px == sb.max_scroll_offset_px &&
         sa.total_height_px == sb.total_height_px;
}

// Structural invariants every successful plan must hold.
void assert_plan_invariants(const pane_model::Plan& plan, int viewport_w,
                            int viewport_h, const char* context,
                            bool expect_degraded) {
  check(plan.ok, where(context, "plan.ok"));
  check_eq(static_cast<int>(plan.failure),
           static_cast<int>(pane_model::Failure::None),
           where(context, "failure == None"));
  check_eq(plan.degraded ? 1 : 0, expect_degraded ? 1 : 0,
           where(context, "degraded flag"));

  const pane_model::Rect top =
      pane_model::reserved_top_rect(viewport_w);
  const pane_model::Rect bottom =
      pane_model::reserved_bottom_rect(viewport_w, viewport_h);
  check(!pane_model::intersects(plan.panel, top),
        where(context, "panel never overlaps reserved top HUD"));
  check(!pane_model::intersects(plan.panel, bottom),
        where(context, "panel never overlaps reserved bottom orbs/quickbar"));
  check(!pane_model::intersects(plan.header, top),
        where(context, "header clears top HUD"));
  check(!pane_model::intersects(plan.footer, bottom),
        where(context, "footer clears bottom orbs/quickbar"));

  check(plan.panel.x >= 0 && plan.panel.y >= 0 &&
            plan.panel.x + plan.panel.w <= viewport_w &&
            plan.panel.y + plan.panel.h <= viewport_h,
        where(context, "panel fully on-screen"));
  check(pane_model::contains(plan.panel, plan.header) &&
            pane_model::contains(plan.panel, plan.tab_bar) &&
            pane_model::contains(plan.panel, plan.content) &&
            pane_model::contains(plan.panel, plan.footer),
        where(context, "header/tab/content/footer bounded by panel"));
  check(plan.content.w > 0 && plan.content.h > 0,
        where(context, "content rect positive"));

  bool any_active = false;
  for (std::size_t i = 0; i < plan.tabs.size(); ++i) {
    check(pane_model::contains(plan.tab_bar, plan.tabs[i].rect),
          where(context, "tab button bounded by tab bar"));
    any_active = any_active || plan.tabs[i].active;
  }
  check(any_active, where(context, "exactly one active tab"));
}

void test_invalid_viewports_fail() {
  const char* ctx = "invalid-viewport";
  const int widths[] = {0, -1, 639};
  const int heights[] = {0, -50, 399};
  for (int i = 0; i < 3; ++i) {
    pane_model::Request request;
    request.viewport_width = widths[i];
    request.viewport_height = heights[i];
    const pane_model::Plan plan = pane_model::plan(request);
    check(!plan.ok, where(ctx, "below-floor or non-positive viewport fails"));
    check_eq(static_cast<int>(plan.failure),
             static_cast<int>(pane_model::Failure::InvalidViewport),
             where(ctx, "failure is InvalidViewport"));
    check(rects_equal(plan.panel, pane_model::Rect{}) &&
              rects_equal(plan.content, pane_model::Rect{}),
          where(ctx, "no rectangles produced on failure"));
  }
  // A valid viewport with impossible minimums degrades explicitly, never
  // off-screen: empty rects plus InsufficientArea.
  pane_model::Request request;
  request.viewport_width = 960;
  request.viewport_height = 600;
  request.minimums.min_panel_width = 5000;
  const pane_model::Plan plan = pane_model::plan(request);
  check(!plan.ok, where(ctx, "impossible minimums fail"));
  check_eq(static_cast<int>(plan.failure),
           static_cast<int>(pane_model::Failure::InsufficientArea),
           where(ctx, "explicit InsufficientArea instead of off-screen draw"));
  check(rects_equal(plan.panel, pane_model::Rect{}),
        where(ctx, "failed plan draws nothing"));
}

void test_viewport_960x600_degrades_explicitly() {
  const char* ctx = "960x600";
  pane_model::Request request;
  request.viewport_width = 960;
  request.viewport_height = 600;
  request.row_counts = {12, 40, 7};
  const pane_model::Plan plan = pane_model::plan(request);
  assert_plan_invariants(plan, 960, 600, ctx, true);

  const pane_model::Rect top{0, 0, 960, pane_model::kReservedTopHudHeight};
  const pane_model::Rect bottom{
      0, 600 - pane_model::kReservedBottomHudHeight, 960,
      pane_model::kReservedBottomHudHeight};
  check(plan.panel.y >= top.y + top.h,
        where(ctx, "panel starts below top HUD band"));
  check(plan.panel.y + plan.panel.h <= bottom.y,
        where(ctx, "panel ends above orbs/quickbar band"));
  check_eq(static_cast<int>(plan.failure),
           static_cast<int>(pane_model::Failure::None),
           where(ctx, "degraded but usable at the minimum window size"));
}

void test_viewport_1366x768_full_metrics() {
  const char* ctx = "1366x768";
  pane_model::Request request;
  request.viewport_width = 1366;
  request.viewport_height = 768;
  request.row_counts = {30, 30, 30};
  const pane_model::Plan plan = pane_model::plan(request);
  assert_plan_invariants(plan, 1366, 768, ctx, false);
}

void test_viewport_1920x1080_full_metrics() {
  const char* ctx = "1920x1080";
  pane_model::Request request;
  request.viewport_width = 1920;
  request.viewport_height = 1080;
  request.row_counts = {5, 5, 5};
  const pane_model::Plan plan = pane_model::plan(request);
  assert_plan_invariants(plan, 1920, 1080, ctx, false);
  check_eq(plan.panel.w, pane_model::kPreferredPanelWidthFull,
           where(ctx, "full-size preferred panel width"));
  check_eq(plan.panel.x, (1920 - plan.panel.w) / 2,
           where(ctx, "panel centered horizontally"));
}

void test_zero_rows() {
  const char* ctx = "zero-rows";
  pane_model::Request request;
  request.viewport_width = 1280;
  request.viewport_height = 720;
  request.row_counts = {0, 0, 0};
  const pane_model::Plan plan = pane_model::plan(request);
  assert_plan_invariants(plan, 1280, 720, ctx, true);
  check(!plan.scroll.clipped, where(ctx, "empty list is not clipped"));
  check_eq(plan.scroll.visible_rows, 0, where(ctx, "no visible rows"));
  check_eq(plan.scroll.max_scroll_offset_px, 0, where(ctx, "no scroll range"));
  check_eq(plan.scroll.scroll_offset_px, 0, where(ctx, "scroll stays at zero"));
}

void test_large_row_set_clips_and_scrolls() {
  const char* ctx = "large-rows";
  pane_model::Request request;
  request.viewport_width = 1920;
  request.viewport_height = 1080;
  request.active_tab = pane_model::Tab::Passives;
  request.row_counts = {10, 10, 100000};
  request.row_height = 24;
  request.scroll_offset_px = 999999999;
  const pane_model::Plan plan = pane_model::plan(request);
  assert_plan_invariants(plan, 1920, 1080, ctx, false);
  check(plan.scroll.clipped, where(ctx, "huge list reports clipping"));
  check(plan.scroll.visible_rows * request.row_height <= plan.content.h,
        where(ctx, "visible rows fit inside content rect"));
  check_eq(plan.scroll.max_scroll_offset_px,
           plan.scroll.total_height_px - plan.content.h,
           where(ctx, "max scroll is total minus visible height"));
  check_eq(plan.scroll.scroll_offset_px, plan.scroll.max_scroll_offset_px,
           where(ctx, "requested offset clamped to scroll range"));
  check(plan.scroll.first_visible_row >= 0 &&
            plan.scroll.first_visible_row <= 100000 - plan.scroll.visible_rows,
        where(ctx, "first visible row within row range"));
}

void test_tab_stability() {
  const char* ctx = "tab-stability";
  pane_model::Request base;
  base.viewport_width = 1600;
  base.viewport_height = 900;
  base.row_counts = {4, 900, 41};

  pane_model::Plan plans[pane_model::kTabCount];
  for (int t = 0; t < pane_model::kTabCount; ++t) {
    pane_model::Request request = base;
    request.active_tab = static_cast<pane_model::Tab>(t);
    plans[t] = pane_model::plan(request);
    assert_plan_invariants(plans[t], 1600, 900,
                           (ctx + std::string(":tab") + std::to_string(t)).c_str(),
                           false);
    check(plans[t].tabs[static_cast<std::size_t>(t)].active,
          where((ctx + std::string(":active-flag") + std::to_string(t)).c_str(),
                "selected tab is flagged active"));
  }
  for (int t = 1; t < pane_model::kTabCount; ++t) {
    check(plans_geometry_equal(plans[0], plans[t]),
          where(ctx, "switching tabs keeps panel geometry stable"));
  }
  check(plans[1].scroll.total_height_px > plans[0].scroll.total_height_px,
        where(ctx, "scroll metadata follows the active tab's row count"));
}

void test_deterministic_plans() {
  const char* ctx = "determinism";
  pane_model::Request request;
  request.viewport_width = 1024;
  request.viewport_height = 640;
  request.active_tab = pane_model::Tab::Character;
  request.row_counts = {13, 250, 8};
  request.scroll_offset_px = 77;
  const pane_model::Plan first = pane_model::plan(request);
  const pane_model::Plan second = pane_model::plan(request);
  check(plans_fully_equal(first, second), where(ctx, "identical inputs -> identical plans"));
  check(first.ok, where(ctx, "sweep plan succeeded"));
}

void test_reserved_non_overlap_sweep() {
  const char* ctx = "reserved-sweep";
  const int widths[] = {640, 800, 960, 1024, 1280, 1366, 1600, 1920, 2560};
  const int heights[] = {400, 450, 540, 600, 720, 768, 900, 1080, 1440};
  for (int wi = 0; wi < 9; ++wi) {
    for (int hi = 0; hi < 9; ++hi) {
      pane_model::Request request;
      request.viewport_width = widths[wi];
      request.viewport_height = heights[hi];
      request.row_counts = {20, 20, 20};
      const pane_model::Plan plan = pane_model::plan(request);
      const std::string tag = " " + std::to_string(widths[wi]) + "x" +
                              std::to_string(heights[hi]);
      if (plan.ok) {
        const pane_model::Rect top = pane_model::reserved_top_rect(widths[wi]);
        const pane_model::Rect bottom =
            pane_model::reserved_bottom_rect(widths[wi], heights[hi]);
        check(!pane_model::intersects(plan.panel, top),
              where(ctx, ("panel clears top HUD" + tag).c_str()));
        check(!pane_model::intersects(plan.panel, bottom),
              where(ctx, ("panel clears orbs/quickbar" + tag).c_str()));
        check(plan.panel.x >= 0 && plan.panel.y >= 0 &&
                  plan.panel.x + plan.panel.w <= widths[wi] &&
                  plan.panel.y + plan.panel.h <= heights[hi],
              where(ctx, ("panel on-screen" + tag).c_str()));
      } else {
        check_eq(static_cast<int>(plan.failure),
                 static_cast<int>(pane_model::Failure::InsufficientArea),
                 where(ctx, ("only explicit degradation" + tag).c_str()));
        check(rects_equal(plan.panel, pane_model::Rect{}),
              where(ctx, ("degraded plan draws nothing" + tag).c_str()));
      }
    }
  }
}

void test_minimums_and_request_validation() {
  const char* ctx = "minimums";
  pane_model::Request request;
  request.viewport_width = 1920;
  request.viewport_height = 1080;
  request.row_counts = {1, 1, 1};
  request.minimums.min_panel_width = 800;
  request.minimums.min_content_width = 700;
  const pane_model::Plan plan = pane_model::plan(request);
  check(plan.ok, where(ctx, "wide panel fits a large viewport"));
  check_eq(plan.panel.w, 800, where(ctx, "caller minimum width honored"));

  pane_model::Request bad = request;
  bad.row_height = 0;
  check(!pane_model::plan(bad).ok, where(ctx, "non-positive row height fails"));
  bad = request;
  bad.row_counts = {1, -1, 1};
  check(!pane_model::plan(bad).ok, where(ctx, "negative row count fails"));

  pane_model::Request negative_scroll = request;
  negative_scroll.row_counts = {200, 200, 200};
  negative_scroll.scroll_offset_px = -500;
  const pane_model::Plan clamped = pane_model::plan(negative_scroll);
  check(clamped.ok, where(ctx, "negative scroll offset still plans"));
  check_eq(clamped.scroll.scroll_offset_px, 0,
           where(ctx, "negative scroll offset clamps to zero"));
}

void test_rect_helpers() {
  const char* ctx = "rect-helpers";
  const pane_model::Rect a{0, 0, 10, 10};
  const pane_model::Rect b{10, 0, 10, 10};
  const pane_model::Rect c{5, 5, 10, 10};
  check(!pane_model::intersects(a, b), where(ctx, "edge-adjacent rects do not intersect"));
  check(pane_model::intersects(a, c), where(ctx, "overlapping rects intersect"));
  check(pane_model::contains(a, pane_model::Rect{2, 2, 4, 4}),
        where(ctx, "contained rect reported"));
  check(!pane_model::contains(a, c), where(ctx, "spilling rect not contained"));
}

} // namespace

int main() {
  test_invalid_viewports_fail();
  test_viewport_960x600_degrades_explicitly();
  test_viewport_1366x768_full_metrics();
  test_viewport_1920x1080_full_metrics();
  test_zero_rows();
  test_large_row_set_clips_and_scrolls();
  test_tab_stability();
  test_deterministic_plans();
  test_reserved_non_overlap_sweep();
  test_minimums_and_request_validation();
  test_rect_helpers();

  std::printf("pane_model tests: %d checks, %d failures\n", g_checks, g_failures);
  if (g_failures == 0) {
    std::printf("pane_model tests: PASS\n");
    return 0;
  }
  std::printf("pane_model tests: FAIL\n");
  return 1;
}
