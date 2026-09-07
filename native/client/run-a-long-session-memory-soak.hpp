#pragma once

// VG-PERF-007: repeated present / effect / resize cycles. A two-frame
// scene cannot claim a long-session envelope.

#include <algorithm>

namespace verdigris::perf {

inline constexpr int kSoakMinCycles = 32;

struct SoakEnvelope {
  int cycles = 0;
  int max_floor_bitmaps = 0;
  int max_effects = 0;
  int max_pens = 0;
};

inline bool soak_is_long_enough(const SoakEnvelope& env) {
  return env.cycles >= kSoakMinCycles;
}

inline void note_cycle(SoakEnvelope& env, int floor_bitmaps, int effects, int pens) {
  ++env.cycles;
  env.max_floor_bitmaps = std::max(env.max_floor_bitmaps, floor_bitmaps);
  env.max_effects = std::max(env.max_effects, effects);
  env.max_pens = std::max(env.max_pens, pens);
}

inline bool envelope_bounded(const SoakEnvelope& env, int floor_cap, int effect_cap,
                             int pen_cap) {
  return env.max_floor_bitmaps <= floor_cap && env.max_effects <= effect_cap &&
         env.max_pens <= pen_cap;
}

inline const char* owner_thirty_two_cycles_label() { return "32 cycles"; }
inline const char* owner_cap_holds_label() { return "Cap holds"; }

}  // namespace verdigris::perf
