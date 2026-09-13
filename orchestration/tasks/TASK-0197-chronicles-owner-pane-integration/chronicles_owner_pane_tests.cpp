// chronicles_owner_pane_tests.cpp — TASK-0197 model-slice acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "chronicles_owner_pane.hpp"

using namespace chronicles_owner_pane;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_owner_demo_journey_pane() {
  const PaneState pane = make_owner_demo_journey_pane();
  check(pane.count >= 5, "pane has journey entries");
  check(covers_owner_demo_beats(pane), "covers required beats");
  check(std::string(kind_name(pane.entries[4].kind)) == "recovered_item",
        "heirloom entry");
}

}  // namespace

int main() {
  test_owner_demo_journey_pane();
  std::cout << "chronicles_owner_pane_tests: PASS (" << g_checks << " checks)\n";
  return 0;
}
