// GATEB-FLAKE repro runner (diagnostic tool; NOT part of the gate).
//
// Includes the FROZEN session_tests.cpp translation unit unchanged (the file
// on disk stays byte-identical - D-129) and drives only the gate-b journey,
// skipping the 17 unrelated remote_* journeys so stress iterations fit a
// tight time budget. Build command is documented in
// orchestration/vg/GATEB-FLAKE/repro.ps1 (uses the same object set as
// verdigris_session_tests.exe).

#define main verdigris_session_tests_frozen_main
#include "../tests/session_tests.cpp"
#undef main

int main() {
  std::setvbuf(stdout, nullptr, _IONBF, 0);
  gateb_driver_state_machine_controls();
  gate_b_chronicles_reconnect_journey();
  if (failures == 0) {
    std::printf("gate-b repro passed\n");
    return 0;
  }
  std::printf("%d gate-b repro check(s) failed\n", failures);
  return 1;
}
