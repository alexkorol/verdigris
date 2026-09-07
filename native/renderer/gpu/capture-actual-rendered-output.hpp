#pragma once

// VG-GPU-007: backend readback of the software sample plus a provenance
// sidecar. A semantic packet log is never a substitute for the image file.

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

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

// A packet/snapshot log is never a substitute for a BM-magic image file.
inline bool packet_log_fails_as_pixels(const std::string& text) {
  return text.size() < 2 || text[0] != 'B' || text[1] != 'M';
}

inline bool provenance_names_scene(const std::string& path) {
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
         text.find("content=gdi-scene:tin-village") != std::string::npos &&
         text.find("platform=win32-gdi") != std::string::npos &&
         text.find("width=960") != std::string::npos &&
         text.find("height=600") != std::string::npos;
}

inline bool write_dib32_bmp(const std::string& path, const std::uint8_t* bits,
                            int width, int height, int stride, bool top_down) {
  if (path.empty() || bits == nullptr || width <= 0 || height <= 0 ||
      stride < width * 4)
    return false;
  const int row_stride = ((width * 3 + 3) / 4) * 4;
  const std::uint32_t pixel_bytes =
      static_cast<std::uint32_t>(row_stride * height);
  FILE* file = nullptr;
#if defined(_MSC_VER)
  if (fopen_s(&file, path.c_str(), "wb") != 0 || !file) return false;
#else
  file = std::fopen(path.c_str(), "wb");
  if (!file) return false;
#endif
  const std::uint32_t off = 54;
  const std::uint32_t size = off + pixel_bytes;
  const std::uint8_t hdr[54] = {
      'B',
      'M',
      static_cast<std::uint8_t>(size),
      static_cast<std::uint8_t>(size >> 8),
      static_cast<std::uint8_t>(size >> 16),
      static_cast<std::uint8_t>(size >> 24),
      0,
      0,
      0,
      0,
      54,
      0,
      0,
      0,
      40,
      0,
      0,
      0,
      static_cast<std::uint8_t>(width),
      static_cast<std::uint8_t>(width >> 8),
      static_cast<std::uint8_t>(width >> 16),
      static_cast<std::uint8_t>(width >> 24),
      static_cast<std::uint8_t>(height),
      static_cast<std::uint8_t>(height >> 8),
      static_cast<std::uint8_t>(height >> 16),
      static_cast<std::uint8_t>(height >> 24),
      1,
      0,
      24,
      0};
  if (std::fwrite(hdr, 1, 54, file) != 54) {
    std::fclose(file);
    return false;
  }
  std::vector<std::uint8_t> row(static_cast<std::size_t>(row_stride), 0);
  for (int out_y = 0; out_y < height; ++out_y) {
    const int src_y = top_down ? (height - 1 - out_y) : out_y;
    const std::uint8_t* src =
        bits + static_cast<std::size_t>(src_y) * static_cast<std::size_t>(stride);
    for (int x = 0; x < width; ++x) {
      row[static_cast<std::size_t>(x * 3 + 0)] = src[static_cast<std::size_t>(x * 4 + 0)];
      row[static_cast<std::size_t>(x * 3 + 1)] = src[static_cast<std::size_t>(x * 4 + 1)];
      row[static_cast<std::size_t>(x * 3 + 2)] = src[static_cast<std::size_t>(x * 4 + 2)];
    }
    if (std::fwrite(row.data(), 1, static_cast<std::size_t>(row_stride), file) !=
        static_cast<std::size_t>(row_stride)) {
      std::fclose(file);
      return false;
    }
  }
  std::fclose(file);
  return true;
}

inline bool capture_painted_scene(const std::string& bmp_path,
                                  const std::string& prov_path,
                                  const std::uint8_t* bits, int width, int height,
                                  int stride) {
  CaptureProvenance rec;
  rec.content = "gdi-scene:tin-village";
  rec.platform = "win32-gdi";
  rec.width = width;
  rec.height = height;
  return write_dib32_bmp(bmp_path, bits, width, height, stride, true) &&
         write_provenance(prov_path, rec) && file_is_bmp(bmp_path) &&
         provenance_names_scene(prov_path);
}

inline const char* owner_pixel_capture_label() { return "Pixel capture"; }
inline const char* owner_bmp_provenance_label() { return "BMP + provenance"; }
inline bool capture_strip_covers_hud_fails_review(bool overlap) {
  return overlap;
}

}  // namespace verdigris::gpu
