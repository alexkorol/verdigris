#pragma once

// VG-GPU-008: resize / recreate / minimize-restore for the software sample.
// One live pixel buffer. A failed recreate surfaces gpu-error:recreate
// instead of crashing or leaking the previous buffer.

#include <cstddef>
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

}  // namespace verdigris::gpu
