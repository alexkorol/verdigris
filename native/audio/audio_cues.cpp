#include "audio_cues.hpp"

#include <algorithm>

namespace verdigris::audio {

bool canonical_order(const ScheduledCue& lhs, const ScheduledCue& rhs) {
  if (lhs.frame != rhs.frame) return lhs.frame < rhs.frame;
  // Priority descending: UI (2) before Feedback (1) before World (0).
  if (lhs.priority != rhs.priority) {
    return static_cast<uint8_t>(lhs.priority) > static_cast<uint8_t>(rhs.priority);
  }
  return lhs.sequence < rhs.sequence;
}

namespace {

void append_u8(std::vector<uint8_t>& out, uint8_t value) { out.push_back(value); }

void append_u32(std::vector<uint8_t>& out, uint32_t value) {
  out.push_back(static_cast<uint8_t>(value & 0xFFu));
  out.push_back(static_cast<uint8_t>((value >> 8) & 0xFFu));
  out.push_back(static_cast<uint8_t>((value >> 16) & 0xFFu));
  out.push_back(static_cast<uint8_t>((value >> 24) & 0xFFu));
}

void append_u64(std::vector<uint8_t>& out, uint64_t value) {
  for (int shift = 0; shift < 64; shift += 8)
    out.push_back(static_cast<uint8_t>((value >> shift) & 0xFFu));
}

void append_i32(std::vector<uint8_t>& out, int32_t value) {
  append_u32(out, static_cast<uint32_t>(value));
}

}  // namespace

std::vector<uint8_t> serialize_schedule(const std::vector<ScheduledCue>& cues) {
  std::vector<ScheduledCue> ordered = cues;
  std::sort(ordered.begin(), ordered.end(), canonical_order);

  std::vector<uint8_t> out;
  out.reserve(4 + 4 + ordered.size() * 64);
  const uint8_t magic[4] = {'V', 'G', 'A', '1'};
  for (const uint8_t byte : magic) append_u8(out, byte);
  append_u32(out, static_cast<uint32_t>(ordered.size()));
  for (const ScheduledCue& cue : ordered) {
    append_u64(out, cue.sequence);
    append_u64(out, cue.frame);
    append_u8(out, static_cast<uint8_t>(cue.bus));
    append_u8(out, static_cast<uint8_t>(cue.priority));
    append_u32(out, cue.effective_gain_milli);
    for (size_t i = 0; i < kCueLabelBytes; ++i) {
      append_u8(out, static_cast<uint8_t>(cue.spec.label[i]));
    }
    append_u8(out, static_cast<uint8_t>(cue.spec.waveform));
    append_u32(out, cue.spec.duration_ms);
    append_u32(out, cue.spec.attack_ms);
    append_u32(out, cue.spec.release_ms);
    append_u32(out, cue.spec.gain_milli);
    append_i32(out, cue.spec.pitch_start_permil);
    append_i32(out, cue.spec.pitch_end_permil);
  }
  return out;
}

std::string schedule_digest_hex(const std::vector<ScheduledCue>& cues) {
  const std::vector<uint8_t> bytes = serialize_schedule(cues);
  uint64_t hash = 1469598103934665603ull;  // FNV-1a 64 offset basis.
  for (const uint8_t byte : bytes) {
    hash ^= byte;
    hash *= 1099511628211ull;  // FNV-1a 64 prime.
  }
  static const char* digits = "0123456789abcdef";
  std::string hex(16, '0');
  for (size_t i = 0; i < 16; ++i)
    hex[i] = digits[(hash >> (60 - 4 * static_cast<int>(i))) & 0xFu];
  return hex;
}

}  // namespace verdigris::audio
