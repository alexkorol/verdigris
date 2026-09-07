// relic_provenance_tests.cpp — TASK-0202 model-slice acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "verdigris/relic_provenance.hpp"

using namespace verdigris;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void expect_status(RelicStatus got, RelicStatus expected,
                   const std::string& label) {
  check(got == expected,
        label + ": expected " + relic_status_name(expected) + " got " +
            relic_status_name(got));
}

void test_owner_demo_full_loop() {
  RelicCirculation flow = make_owner_demo_circulation();
  expect_status(mark_fallen_equipped(flow), RelicStatus::Ok, "fallen equipped");
  expect_status(entomb_to_crypt(flow), RelicStatus::Ok, "crypt queue");
  expect_status(release_circulating_relic(flow), RelicStatus::Ok, "circulate");
  check(ground_matches_seed(flow.ground, flow.seed), "ground provenance");
  expect_status(recover_from_ground(flow, kOwnerDemoHeirloomUuid), RelicStatus::Ok,
                "recover");
  check(demo_recovery_complete(flow), "recovery complete");
  check(std::string(flow.seed.item_catalog_id) == "steel-battleaxe",
        "battleaxe catalog");
}

void test_reject_wrong_uuid() {
  RelicCirculation flow = make_owner_demo_circulation();
  mark_fallen_equipped(flow);
  entomb_to_crypt(flow);
  release_circulating_relic(flow);
  expect_status(recover_from_ground(flow, "wrong-uuid"), RelicStatus::ProvenanceMismatch,
                "wrong uuid");
}

void test_no_second_player_seed() {
  const RelicSeed seed = make_owner_demo_heirloom_seed();
  check(seed.valid(), "seed valid without live peer");
  check(std::string(seed.source_scion_name) == "Orun the First", "authored name");
}

}  // namespace

int main() {
  test_owner_demo_full_loop();
  test_reject_wrong_uuid();
  test_no_second_player_seed();
  std::cout << "relic_provenance_tests: PASS (" << g_checks << " checks)\n";
  return 0;
}
