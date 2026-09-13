#pragma once

// VG-SOUND-007: score a dense combat mix from the mixer tape.
// An isolated tone preview cannot prove the encounter mix.

#include <cstdio>
#include <string>
#include <vector>

#include "../audio/cue_spec.hpp"

namespace verdigris::audio {

struct MixScore {
  int cues = 0;
  int unique_ids = 0;
  int peak_gain = 0;
  int floor_gain = 1000;
  bool has_hit = false;
  bool has_kill = false;
  bool has_warning = false;
  bool has_ambience = false;
  bool has_music = false;
  bool isolated_preview = false;
  std::string attribution;
};

inline MixScore score_mix(const std::vector<CueSpec>& cues, bool isolated_preview) {
  MixScore score;
  score.isolated_preview = isolated_preview;
  std::vector<std::string> seen;
  for (const auto& cue : cues) {
    if (cue.cue_id.empty()) continue;
    ++score.cues;
    bool fresh = true;
    for (const auto& id : seen)
      if (id == cue.cue_id) fresh = false;
    if (fresh) {
      seen.push_back(cue.cue_id);
      if (!score.attribution.empty()) score.attribution += ",";
      score.attribution += cue.cue_id;
    }
    const int gain = cue.effective_gain_permille > 0 ? cue.effective_gain_permille
                                                     : cue.params.gain_permille;
    if (gain > score.peak_gain) score.peak_gain = gain;
    if (gain < score.floor_gain) score.floor_gain = gain;
    if (cue.cue_id == "hit" || cue.cue_id == "crit") score.has_hit = true;
    if (cue.cue_id == "kill") score.has_kill = true;
    if (cue.cue_id == "scion-lost" || cue.cue_id.find("warn") != std::string::npos)
      score.has_warning = true;
    if (cue.cue_id.rfind("ambience:", 0) == 0) score.has_ambience = true;
    if (cue.cue_id.rfind("music:", 0) == 0) score.has_music = true;
  }
  score.unique_ids = static_cast<int>(seen.size());
  if (score.cues == 0) score.floor_gain = 0;
  return score;
}

inline bool isolated_preview_cannot_pass(const MixScore& score) {
  return score.isolated_preview || score.cues < 3 || score.unique_ids < 2;
}

inline bool mix_has_range(const MixScore& score) {
  return score.peak_gain > 0 && score.peak_gain > score.floor_gain;
}

inline bool mix_is_encounter(const MixScore& score) {
  return !isolated_preview_cannot_pass(score) && score.has_hit &&
         (score.has_ambience || score.has_music) && mix_has_range(score) &&
         !score.attribution.empty();
}

inline const char* owner_mix_label() { return "Encounter mix"; }
inline const char* owner_mix_range_label() { return "Hit + warning"; }

inline bool write_mix_score(const std::string& path, const MixScore& score) {
  if (path.empty()) return false;
  FILE* file = nullptr;
#if defined(_MSC_VER)
  if (fopen_s(&file, path.c_str(), "wb") != 0 || !file) return false;
#else
  file = std::fopen(path.c_str(), "wb");
  if (!file) return false;
#endif
  std::fprintf(file,
               "cues=%d\nunique=%d\npeak=%d\nfloor=%d\nhit=%d\nkill=%d\n"
               "warning=%d\nambience=%d\nmusic=%d\nisolated=%d\nattribution=%s\n",
               score.cues, score.unique_ids, score.peak_gain, score.floor_gain,
               score.has_hit ? 1 : 0, score.has_kill ? 1 : 0,
               score.has_warning ? 1 : 0, score.has_ambience ? 1 : 0,
               score.has_music ? 1 : 0, score.isolated_preview ? 1 : 0,
               score.attribution.c_str());
  std::fclose(file);
  return true;
}

}  // namespace verdigris::audio
