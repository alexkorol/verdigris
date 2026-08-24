// instance_refresh_tests.cpp — TASK-0176 acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "instance_refresh.hpp"

using namespace instance_refresh;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

ZonePolicy combat_zone(std::uint32_t id) {
  ZonePolicy z;
  z.zone_id = id;
  z.kind = ZoneKind::Combat;
  z.allow_fresh = true;
  z.lifetime_ticks = 100;
  return z;
}

ZonePolicy town_zone(std::uint32_t id) {
  ZonePolicy z;
  z.zone_id = id;
  z.kind = ZoneKind::Town;
  z.allow_fresh = false;
  z.lifetime_ticks = 0;
  return z;
}

void test_reuse_combat_instance() {
  Registry reg;
  const ZonePolicy zone = combat_zone(10);
  const Decision first = evaluate(reg, zone, Request::Reuse);
  check(first.outcome == Outcome::CreateFresh, "first entry creates");
  const std::uint32_t id1 = first.instance_id;
  const Decision second = evaluate(reg, zone, Request::Reuse);
  check(second.outcome == Outcome::ReuseExisting, "reuse existing");
  check(second.instance_id == id1, "same instance id");
  check(second.message == MessageCode::Reused, "reuse message");
}

void test_fresh_instance_combat() {
  Registry reg;
  const ZonePolicy zone = combat_zone(20);
  const Decision first = evaluate(reg, zone, Request::Reuse);
  check(first.outcome == Outcome::CreateFresh, "first entry creates");
  const Decision fresh = evaluate(reg, zone, Request::FreshInstance);
  check(fresh.outcome == Outcome::CreateFresh, "fresh created");
  check(fresh.instance_id != 0, "fresh id assigned");
  check(std::string(message_text(fresh.message)).find("Fresh") != std::string::npos,
        "fresh message");
}

void test_town_rejects_fresh() {
  Registry reg;
  const ZonePolicy zone = town_zone(1);
  const Decision d = evaluate(reg, zone, Request::FreshInstance);
  check(d.outcome == Outcome::RejectedTownFresh, "town rejects fresh");
  check(d.message == MessageCode::TownNoFresh, "town message");
}

void test_expiry_blocks_reuse() {
  Registry reg;
  ZonePolicy zone = combat_zone(30);
  zone.lifetime_ticks = 5;
  const Decision first = evaluate(reg, zone, Request::Reuse);
  reg.tick = 5;
  const Decision d = evaluate(reg, zone, Request::Reuse);
  check(d.outcome == Outcome::RejectedExpired, "expired blocks reuse");
  check(d.message == MessageCode::Expired, "expiry message");
}

void test_no_accidental_refresh() {
  Registry reg;
  const ZonePolicy zone = combat_zone(40);
  const Decision a = evaluate(reg, zone, Request::Reuse);
  const Decision b = evaluate(reg, zone, Request::Reuse);
  check(a.instance_id == b.instance_id, "reuse does not spawn duplicate");
  check(reg.count == 1, "single instance record");
}

void test_not_refreshable_zone() {
  Registry reg;
  ZonePolicy zone = combat_zone(50);
  zone.allow_fresh = false;
  const Decision first = evaluate(reg, zone, Request::Reuse);
  const Decision blocked = evaluate(reg, zone, Request::FreshInstance);
  check(blocked.outcome == Outcome::RejectedNotRefreshable, "fresh blocked");
}

void test_deterministic_replay() {
  Registry a;
  Registry b;
  const ZonePolicy zone = combat_zone(60);
  check(evaluate(a, zone, Request::Reuse) == evaluate(b, zone, Request::Reuse),
        "deterministic first");
  check(evaluate(a, zone, Request::Reuse) == evaluate(b, zone, Request::Reuse),
        "deterministic reuse");
}

}  // namespace

int main() {
  test_reuse_combat_instance();
  test_fresh_instance_combat();
  test_town_rejects_fresh();
  test_expiry_blocks_reuse();
  test_no_accidental_refresh();
  test_not_refreshable_zone();
  test_deterministic_replay();

  std::cout << "TASK-0176 instance refresh acceptance: " << g_checks
            << " checks passed\n";
  return 0;
}
