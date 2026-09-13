#pragma once

#include <cstring>

// VG-BUILD-001: three slice fixtures. Reach, pressure, and magic must
// differ in action, range, and resource — a tint swap is not a build.
// Core STAT/BUILD algebra stays with Kimi.

namespace verdigris::client::builds {

struct SliceBuild {
  const char* id;
  const char* role;
  const char* primary_action;
  int reach_tiles;
  const char* resource;
  const char* tactics;
  const char* weakness;
  const char* gear;
  const char* encounter;
  unsigned tint;
};

inline constexpr SliceBuild kReach{
    "reach",
    "reach",
    "thrust",
    2,
    "stamina",
    "keep pike range; sidestep windups",
    "boxed corridors and dash gaps",
    "bronze pike",
    "kite mixed wardens then extract",
    0x00C4B080};

inline constexpr SliceBuild kPressure{
    "pressure",
    "pressure",
    "melee",
    1,
    "none",
    "step in, sweep, interrupt the elite",
    "ranged leashes and long telegraphs",
    "close blade",
    "break the elite cast then finish",
    0x00B05048};

inline constexpr SliceBuild kMagic{
    "magic",
    "magic",
    "war-cry",
    0,
    "resource",
    "buff, weave, recover between packs",
    "empty resource and silence",
    "attuned vessel",
    "burst the pack then walk the pad",
    0x004070C0};

inline constexpr SliceBuild kSliceBuilds[3] = {kReach, kPressure, kMagic};

inline bool distinct_slice_loops(const SliceBuild* builds, int count) {
  if (count != 3) return false;
  for (int i = 0; i < count; ++i) {
    if (!builds[i].tactics || !builds[i].weakness || !builds[i].gear ||
        !builds[i].encounter)
      return false;
    for (int j = i + 1; j < count; ++j) {
      if (std::strcmp(builds[i].primary_action, builds[j].primary_action) ==
              0 &&
          builds[i].reach_tiles == builds[j].reach_tiles &&
          std::strcmp(builds[i].resource, builds[j].resource) == 0)
        return false;
      if (std::strcmp(builds[i].role, builds[j].role) == 0) return false;
    }
  }
  return true;
}

inline constexpr SliceBuild kTintOnlyClones[3] = {
    {"clone-a", "reach", "melee", 1, "none", "same loop", "same", "same",
     "same", 0x000000FF},
    {"clone-b", "pressure", "melee", 1, "none", "same loop", "same", "same",
     "same", 0x0000FF00},
    {"clone-c", "magic", "melee", 1, "none", "same loop", "same", "same",
     "same", 0x00FF0000}};

inline bool tint_only_clones_fail_review() {
  return !distinct_slice_loops(kTintOnlyClones, 3);
}

inline const char* fixture_hud_label(const SliceBuild& build) {
  if (std::strcmp(build.role, "reach") == 0) return "build-fixture:reach";
  if (std::strcmp(build.role, "pressure") == 0) return "build-fixture:pressure";
  return "build-fixture:magic";
}

inline const char* owner_three_slices_label() { return "Three slices"; }
inline const char* owner_reach_pike_label() { return "Reach pike"; }

}  // namespace verdigris::client::builds
