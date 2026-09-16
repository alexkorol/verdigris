// GATEB-FLAKE validation runner for the PROPOSED driver patch.
//
// Identical to gateb_flake_repro.cpp but includes the patched COPY at
// orchestration/vg/GATEB-FLAKE/patched/session_tests_patched.cpp so the
// proposed fix can be measured without touching the frozen
// native/tests/session_tests.cpp (D-129). The quoted ../client includes
// inside the copied file resolve via the extra /I"native\tests" on the
// compile line (see build-repro-patched.bat).

#define main verdigris_session_tests_patched_main
#include "../../orchestration/vg/GATEB-FLAKE/patched/session_tests_patched.cpp"
#undef main

int main() {
  std::setvbuf(stdout, nullptr, _IONBF, 0);
  gateb_driver_state_machine_controls();
  gate_b_chronicles_reconnect_journey();
  if (failures == 0) {
    std::printf("gate-b patched repro passed\n");
    return 0;
  }
  std::printf("%d gate-b patched repro check(s) failed\n", failures);
  return 1;
}
