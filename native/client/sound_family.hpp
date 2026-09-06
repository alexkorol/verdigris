#pragma once

// VG-SOUND-002: one legal, source-traceable family for swing/impact/warning.
// Cues stay procedural (TASK-0157) so this is provenance, not a binary bank.
// A cue without license/source cannot ship even if the mixer plays it.

#include <cstring>

namespace verdigris::client::sound_family {

struct Entry {
  const char* cue_id;
  const char* role;      // swing | impact | warning
  const char* license;   // SPDX
  const char* source;    // origin of the cooked params
};

inline constexpr Entry kCombatFamily[] = {
    {"hit", "impact", "CC0-1.0", "procedural-synth:sine-220-110"},
    {"crit", "impact", "CC0-1.0", "procedural-synth:square-440-110"},
    {"kill", "impact", "CC0-1.0", "procedural-synth:saw-196-49"},
    {"scion-lost", "warning", "CC0-1.0", "procedural-synth:sine-165-41"},
    {"warcry-expire", "warning", "CC0-1.0", "procedural-synth:sine-392-262"},
    {"cosmetic", "swing", "CC0-1.0", "procedural-synth:placeholder-ui"},
};

inline const Entry* find(const char* cue_id) {
  if (!cue_id) return nullptr;
  for (const auto& row : kCombatFamily)
    if (std::strcmp(row.cue_id, cue_id) == 0) return &row;
  return nullptr;
}

inline bool shippable(const char* cue_id) {
  const Entry* row = find(cue_id);
  return row && row->license && row->license[0] != '\0' && row->source &&
         row->source[0] != '\0';
}

}  // namespace verdigris::client::sound_family
