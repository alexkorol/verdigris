#pragma once

// VG-GPU-008: resize / recreate / minimize-restore for the software sample.
// One live pixel buffer. A failed recreate surfaces gpu-error:recreate
// instead of crashing or leaking the previous buffer.

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "build-an-isolated-cross-platform-gpu-sample.hpp"

namespace verdigris::gpu {

inline constexpr const char* kRecreateError = "gpu-error:recreate";

struct RecoverablePresenter {
  Sample sample;
  int generation = 0;
  int live_buffers = 0;
  bool error_visible = false;
  char error[32]{};

  bool recreate(Backend backend, int w, int h) {
    sample.shutdown();
    live_buffers = 0;
    ++generation;
    error_visible = false;
    error[0] = '\0';
    if (!sample.init(backend, w, h) || !sample.alive) {
      error_visible = true;
      const char* msg = kRecreateError;
      std::size_t i = 0;
      for (; msg[i] != '\0' && i + 1 < sizeof(error); ++i) error[i] = msg[i];
      error[i] = '\0';
      return false;
    }
    live_buffers = 1;
    return true;
  }

  bool minimize_restore() {
    const int w = sample.width;
    const int h = sample.height;
    const Backend backend = Backend::Software;
    return recreate(backend, w, h);
  }
};

inline bool leaked_buffers_fail_review(int live_buffers) {
  return live_buffers != 1;
}

inline const char* owner_live_buffers_label() { return "Live buffers 1"; }

// A restored buffer is not the isolated sample still. The L-bracket marks
// that this pixels object survived recreate; gpu-sample has no mark.
inline bool stamp_restored_buffer(Sample& sample) {
  if (!sample.alive || sample.width < 10 || sample.height < 10) return false;
  const std::uint32_t mark = 0x005FA893u;
  for (int i = 0; i < 8; ++i) {
    sample.pixels[static_cast<std::size_t>(i)] = mark;
    sample.pixels[static_cast<std::size_t>(i * sample.width)] = mark;
  }
  return sample.pixel(0, 0) == mark && sample.pixel(0, 7) == mark;
}

}  // namespace verdigris::gpu
