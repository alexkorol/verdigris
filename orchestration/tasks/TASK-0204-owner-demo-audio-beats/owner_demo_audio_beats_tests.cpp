// owner_demo_audio_beats_tests.cpp — TASK-0204 model-slice acceptance tests.

#include <cstdlib>
#include <iostream>
#include <string>

#include "owner_demo_audio_beats.hpp"

using namespace verdigris::audio::owner_demo;
using verdigris::client::PresentationEventType;

namespace {

int g_checks = 0;

void check(bool condition, const std::string& message) {
  ++g_checks;
  if (!condition) {
    std::cerr << "FAIL: " << message << "\n";
    std::exit(1);
  }
}

void test_all_beats_defined() {
  check(all_owner_demo_beats_defined(), "seven beats defined");
}

void test_owner_demo_mappings() {
  const BeatCuePlan attack =
      plan_for_discriminator(PresentationEventType::AttackStarted, "");
  check(attack.maps && attack.beat == Beat::Attack, "attack");

  const BeatCuePlan hit =
      plan_for_discriminator(PresentationEventType::DamageApplied, "");
  check(hit.maps && hit.beat == Beat::Hit, "hit");

  const BeatCuePlan boss =
      plan_for_discriminator(PresentationEventType::ActorDied, "warden");
  check(boss.maps && boss.beat == Beat::BossDeath, "boss death");

  const BeatCuePlan level =
      plan_for_discriminator(PresentationEventType::Message,
                             "real combat advanced the scion to level 2");
  check(level.maps && level.beat == Beat::LevelUp, "level up");

  const BeatCuePlan gate =
      plan_for_discriminator(PresentationEventType::Message, "Thornward gate");
  check(gate.maps && gate.beat == Beat::Gate, "gate");

  const BeatCuePlan loot =
      plan_for_discriminator(PresentationEventType::ItemPickedUp, "");
  check(loot.maps && loot.beat == Beat::Loot, "loot");

  const BeatCuePlan menu =
      plan_for_discriminator(PresentationEventType::SessionReady, "");
  check(menu.maps && menu.beat == Beat::Menu, "menu");
}

}  // namespace

int main() {
  test_all_beats_defined();
  test_owner_demo_mappings();
  std::cout << "owner_demo_audio_beats_tests: PASS (" << g_checks << " checks)\n";
  return 0;
}
