#pragma once

// VG-GPU-007: backend readback of the software sample plus a provenance
// sidecar. A semantic packet log is never a substitute for the image file.

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>

#include "add-one-dynamic-material-light-interaction.hpp"
#include "packets.hpp"

namespace verdigris::gpu {

struct CaptureProvenance {
  const char* backend = Sample::kBackendName;
  const char* content = "cooked:bronze-stone-v1";
  const char* platform = "software";
  int width = 0;
  int height = 0;
};

inline bool semantic_log_counts_as_capture(const std::string& text) {
  if (text.size() < 2) return false;
  return text[0] == 'B' && text[1] == 'M';
}

// GdipCreateBitmapFromHBITMAP + PNG treats DIB B,G,R as R,G,B. A file that
// swapped those channels cannot certify the painted COLORREF.
inline void swap_bgra_rb(std::uint8_t* px, int pixels) {
  if (!px || pixels <= 0) return;
  for (int i = 0; i < pixels; ++i) {
    const std::uint8_t b = px[static_cast<std::size_t>(i) * 4];
    px[static_cast<std::size_t>(i) * 4] = px[static_cast<std::size_t>(i) * 4 + 2];
    px[static_cast<std::size_t>(i) * 4 + 2] = b;
  }
}

inline bool swapped_png_rejected(int file_r, int file_b, int dib_r, int dib_b) {
  return file_r == dib_b && file_b == dib_r && dib_r != dib_b;
}

inline bool file_is_bmp(const std::string& path) {
  if (path.empty()) return false;
  FILE* file = nullptr;
#if defined(_MSC_VER)
  if (fopen_s(&file, path.c_str(), "rb") != 0 || !file) return false;
#else
  file = std::fopen(path.c_str(), "rb");
  if (!file) return false;
#endif
  unsigned char mag[2] = {0, 0};
  const bool ok = std::fread(mag, 1, 2, file) == 2 && mag[0] == 'B' && mag[1] == 'M';
  std::fseek(file, 0, SEEK_END);
  const long size = std::ftell(file);
  std::fclose(file);
  return ok && size > 54;
}

inline bool write_provenance(const std::string& path, const CaptureProvenance& rec) {
  if (path.empty() || rec.backend == nullptr || rec.content == nullptr) return false;
  FILE* file = nullptr;
#if defined(_MSC_VER)
  if (fopen_s(&file, path.c_str(), "wb") != 0 || !file) return false;
#else
  file = std::fopen(path.c_str(), "wb");
  if (!file) return false;
#endif
  std::fprintf(file, "backend=%s\ncontent=%s\nplatform=%s\nwidth=%d\nheight=%d\n",
               rec.backend, rec.content, rec.platform, rec.width, rec.height);
  std::fclose(file);
  return true;
}

inline bool provenance_complete(const std::string& path) {
  if (path.empty()) return false;
  FILE* file = nullptr;
#if defined(_MSC_VER)
  if (fopen_s(&file, path.c_str(), "rb") != 0 || !file) return false;
#else
  file = std::fopen(path.c_str(), "rb");
  if (!file) return false;
#endif
  char buf[512];
  const std::size_t n = std::fread(buf, 1, sizeof(buf) - 1, file);
  std::fclose(file);
  buf[n] = '\0';
  const std::string text(buf);
  return text.find("backend=software") != std::string::npos &&
         text.find("content=cooked:bronze-stone-v1") != std::string::npos &&
         text.find("platform=software") != std::string::npos &&
         text.find("width=") != std::string::npos;
}

inline bool capture_sample(const Sample& sample, const std::string& bmp_path,
                           const std::string& prov_path) {
  if (!sample.alive) return false;
  CaptureProvenance rec;
  rec.width = sample.width;
  rec.height = sample.height;
  return sample.write_bmp(bmp_path) && write_provenance(prov_path, rec) &&
         file_is_bmp(bmp_path) && provenance_complete(prov_path);
}

}  // namespace verdigris::gpu
