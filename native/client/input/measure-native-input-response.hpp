#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// VG-MOVE-008: input QPC → present QPC. This is input-to-present through
// the production paint path, not input-to-photon (no vsync/readback).
// Headless Command::dispatch time must never use the photon label.
// VG-MOVE-007 action buffering stays with Kimi.

namespace verdigris::client::input {

inline constexpr std::size_t kMaxLatencySamples = 128;

struct LatencySample {
  long long input_qpc = 0;
  long long present_qpc = 0;
};

struct LatencyLog {
  std::vector<LatencySample> samples;
  long long pending_input_qpc = 0;
  long long freq = 0;
};

inline long long qpc_now() {
  LARGE_INTEGER now{};
  QueryPerformanceCounter(&now);
  return now.QuadPart;
}

inline long long qpc_freq() {
  LARGE_INTEGER freq{};
  QueryPerformanceFrequency(&freq);
  return freq.QuadPart;
}

inline void note_input(LatencyLog& log) {
  if (log.freq <= 0) log.freq = qpc_freq();
  log.pending_input_qpc = qpc_now();
}

inline void note_present(LatencyLog& log) {
  if (log.pending_input_qpc == 0) return;
  if (log.freq <= 0) log.freq = qpc_freq();
  LatencySample sample;
  sample.input_qpc = log.pending_input_qpc;
  sample.present_qpc = qpc_now();
  log.pending_input_qpc = 0;
  if (log.samples.size() >= kMaxLatencySamples)
    log.samples.erase(log.samples.begin());
  log.samples.push_back(sample);
}

inline double sample_ms(const LatencyLog& log, const LatencySample& sample) {
  if (log.freq <= 0) return 0.0;
  return 1000.0 * static_cast<double>(sample.present_qpc - sample.input_qpc) /
         static_cast<double>(log.freq);
}

inline double percentile_ms(const LatencyLog& log, double p) {
  if (log.samples.empty()) return 0.0;
  std::vector<double> xs;
  xs.reserve(log.samples.size());
  for (const auto& sample : log.samples) xs.push_back(sample_ms(log, sample));
  std::sort(xs.begin(), xs.end());
  const double idx = p * static_cast<double>(xs.size() - 1);
  const std::size_t lo = static_cast<std::size_t>(idx);
  const std::size_t hi = std::min(lo + 1, xs.size() - 1);
  const double frac = idx - static_cast<double>(lo);
  return xs[lo] * (1.0 - frac) + xs[hi] * frac;
}

inline bool command_time_is_not_photon(const char* label) {
  if (!label) return true;
  const std::string s = label;
  return s.find("photon") == std::string::npos;
}

inline const char* present_kind_hud() { return "input-latency:present"; }
inline const char* photon_kind_hud() { return "input-latency:photon"; }

inline std::string p50_hud(const LatencyLog& log) {
  char buf[48];
  std::snprintf(buf, sizeof(buf), "input-latency:p50:%.1f", percentile_ms(log, 0.50));
  return buf;
}

inline std::string p95_hud(const LatencyLog& log) {
  char buf[48];
  std::snprintf(buf, sizeof(buf), "input-latency:p95:%.1f", percentile_ms(log, 0.95));
  return buf;
}

}  // namespace verdigris::client::input
