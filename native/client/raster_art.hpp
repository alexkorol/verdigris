#pragma once

// Runtime PNG art for the Win32 presentation shell. Images are decoded once;
// each requested size/facing is rasterized once with GDI+ nearest-neighbor
// sampling into a premultiplied DIB. Steady-state draws are BitBlt/AlphaBlend,
// with no image decoding, GDI+ Graphics creation, or heap allocation.
//
// Sprite coordinates identify the bottom center of the normalized canvas.
// Heights are exact screen pixels: integer multiples of source height preserve
// uniform pixel blocks, while arbitrary camera zoom remains nearest-neighbor.
// This cache belongs to the presentation thread, never to simulation workers.

#ifdef _WIN32

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objidl.h>
namespace Gdiplus {
using std::max;
using std::min;
}  // namespace Gdiplus
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "msimg32.lib")

namespace raster_art {

struct Dimensions {
  int width = 0;
  int height = 0;
  [[nodiscard]] bool valid() const { return width > 0 && height > 0; }
};

struct CacheStats {
  std::size_t loaded_assets = 0;
  std::size_t missing_assets = 0;
  std::size_t scaled_bitmaps = 0;
  std::size_t source_bytes = 0;
  std::size_t scaled_bytes = 0;
  std::uint64_t image_loads = 0;
  std::uint64_t scale_builds = 0;
  std::uint64_t cache_hits = 0;
  std::uint64_t draws = 0;
};

namespace detail {

inline constexpr std::size_t kMaxAssets = 512;
inline constexpr std::size_t kMaxScaledBitmaps = 192;
inline constexpr std::size_t kMaxSourceBytes = 64u * 1024u * 1024u;
inline constexpr std::size_t kMaxScaledBytes = 64u * 1024u * 1024u;
inline constexpr int kMaxDimension = 2048;

struct Asset {
  std::unique_ptr<Gdiplus::Bitmap> image;
  Dimensions size;
  RECT content_bounds{};  // Nonzero-alpha bounds; right/bottom are exclusive.
  std::uint32_t id = 0;
  bool opaque = false;
};

struct Surface {
  HDC dc = nullptr;
  HBITMAP bitmap = nullptr;
  HGDIOBJ old_bitmap = nullptr;
  void* pixels = nullptr;
  int width = 0;
  int height = 0;
  std::uint64_t used = 0;

  Surface() = default;
  Surface(const Surface&) = delete;
  Surface& operator=(const Surface&) = delete;
  ~Surface() {
    if (dc && old_bitmap && old_bitmap != HGDI_ERROR)
      SelectObject(dc, old_bitmap);
    if (bitmap) DeleteObject(bitmap);
    if (dc) DeleteDC(dc);
  }

  [[nodiscard]] std::size_t bytes() const {
    return static_cast<std::size_t>(width) * height * 4u;
  }

  bool create(int w, int h) {
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = w;
    info.bmiHeader.biHeight = -h;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    dc = CreateCompatibleDC(nullptr);
    if (!dc) return false;
    bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);
    if (!bitmap || !pixels) return false;
    old_bitmap = SelectObject(dc, bitmap);
    if (!old_bitmap || old_bitmap == HGDI_ERROR) return false;
    width = w;
    height = h;
    std::memset(pixels, 0, bytes());
    return true;
  }
};

using ScaleKey = std::tuple<std::uint32_t, int, int, bool>;

struct Cache {
  ULONG_PTR token = 0;
  std::wstring root;
  bool discovered_root = false;
  std::wstring error;
  std::map<std::string, Asset, std::less<>> assets;
  std::map<ScaleKey, std::unique_ptr<Surface>> scales;
  CacheStats stats;
  std::uint64_t clock = 0;

  Cache() {
    Gdiplus::GdiplusStartupInput input;
    if (Gdiplus::GdiplusStartup(&token, &input, nullptr) != Gdiplus::Ok) {
      token = 0;
      error = L"GDI+ startup failed for raster art.";
    }
  }
  ~Cache() {
    scales.clear();
    assets.clear();
    if (token) Gdiplus::GdiplusShutdown(token);
  }
};

inline Cache& cache() {
  static Cache value;
  return value;
}

inline bool directory_exists(const std::wstring& path) {
  const DWORD attributes = GetFileAttributesW(path.c_str());
  return attributes != INVALID_FILE_ATTRIBUTES &&
         (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

inline std::wstring parent_path(const std::wstring& path) {
  const auto at = path.find_last_of(L"\\/");
  return at == std::wstring::npos ? std::wstring() : path.substr(0, at);
}

inline bool find_root_from(Cache& c, std::wstring directory) {
  for (int depth = 0; depth < 8 && !directory.empty(); ++depth) {
    // Installed assets take priority over source-tree fallbacks.
    for (const wchar_t* suffix : {L"\\assets\\raster\\runtime",
                                  L"\\native\\client\\assets\\raster\\runtime",
                                  L"\\client\\assets\\raster\\runtime"}) {
      const std::wstring candidate = directory + suffix;
      if (directory_exists(candidate)) {
        c.root = candidate;
        return true;
      }
    }
    directory = parent_path(directory);
  }
  return false;
}

inline void discover_root(Cache& c) {
  if (c.discovered_root) return;
  c.discovered_root = true;
  std::vector<wchar_t> path(32768);
  const DWORD executable_length =
      GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
  if (executable_length > 0 && executable_length < path.size() &&
      find_root_from(c, parent_path(std::wstring(path.data(), executable_length))))
    return;
  const DWORD cwd_length =
      GetCurrentDirectoryW(static_cast<DWORD>(path.size()), path.data());
  if (cwd_length > 0 && cwd_length < path.size())
    find_root_from(c, std::wstring(path.data(), cwd_length));
}

// Runtime names are canonical lowercase filenames, optionally ending in .png.
// Heterogeneous map lookup uses this view without allocating on a cache hit.
inline std::string_view asset_key(const char* name) {
  if (!name) return {};
  std::string_view key(name);
  if (key.ends_with(".png")) key.remove_suffix(4);
  if (key.empty() || key.size() > 96) return {};
  for (char ch : key) {
    if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
          ch == '_' || ch == '-'))
      return {};
  }
  return key;
}

inline void fail_asset(Cache& c, std::string_view key, const wchar_t* reason) {
  ++c.stats.missing_assets;
  c.error = L"Raster art ";
  c.error.append(key.begin(), key.end());
  c.error += L": ";
  c.error += reason;
  c.error += L" [" + c.root + L"]";
  OutputDebugStringW((c.error + L"\n").c_str());
}

inline Asset* asset(const char* name) {
  const std::string_view key = asset_key(name);
  if (key.empty()) return nullptr;
  Cache& c = cache();
  const auto found = c.assets.find(key);
  if (found != c.assets.end())
    return found->second.image ? &found->second : nullptr;
  if (!c.token || c.assets.size() >= kMaxAssets) return nullptr;
  discover_root(c);
  Asset& result = c.assets.try_emplace(std::string(key)).first->second;
  result.id = static_cast<std::uint32_t>(c.assets.size());
  if (c.root.empty()) {
    fail_asset(c, key, L"runtime directory was not found");
    return nullptr;
  }
  std::wstring filename = c.root + L"\\";
  filename.append(key.begin(), key.end());
  filename += L".png";
  const DWORD attributes = GetFileAttributesW(filename.c_str());
  if (attributes == INVALID_FILE_ATTRIBUTES ||
      (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
    fail_asset(c, key, L"PNG file is missing");
    return nullptr;
  }
  Gdiplus::Bitmap png(filename.c_str(), FALSE);
  if (png.GetLastStatus() != Gdiplus::Ok || png.GetWidth() == 0 ||
      png.GetHeight() == 0 || png.GetWidth() > kMaxDimension ||
      png.GetHeight() > kMaxDimension) {
    fail_asset(c, key, L"PNG decode failed or dimensions are unsupported");
    return nullptr;
  }
  const int width = static_cast<int>(png.GetWidth());
  const int height = static_cast<int>(png.GetHeight());
  const std::size_t bytes = static_cast<std::size_t>(width) * height * 4u;
  if (bytes > kMaxSourceBytes - c.stats.source_bytes) {
    fail_asset(c, key, L"source cache byte limit reached");
    return nullptr;
  }
  auto detached = std::make_unique<Gdiplus::Bitmap>(width, height,
                                                  PixelFormat32bppPARGB);
  if (detached->GetLastStatus() != Gdiplus::Ok) {
    fail_asset(c, key, L"source bitmap allocation failed");
    return nullptr;
  }
  {
    Gdiplus::Graphics graphics(detached.get());
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
    if (graphics.DrawImage(&png, 0, 0, 0, 0, width, height,
                           Gdiplus::UnitPixel) != Gdiplus::Ok) {
      fail_asset(c, key, L"source bitmap conversion failed");
      return nullptr;
    }
  }
  // Decode into an independent PARGB bitmap so no PNG file remains locked.
  // Inspect alpha once, enabling opaque terrain to take the cheaper BitBlt
  // and static props to size against visible content instead of atlas padding.
  Gdiplus::BitmapData pixels{};
  const Gdiplus::Rect rect(0, 0, width, height);
  if (detached->LockBits(&rect, Gdiplus::ImageLockModeRead,
                         PixelFormat32bppPARGB, &pixels) != Gdiplus::Ok) {
    fail_asset(c, key, L"source pixel access failed");
    return nullptr;
  }
  bool opaque = true;
  RECT content{width, height, 0, 0};
  for (int y = 0; y < height; ++y) {
    const BYTE* row = static_cast<const BYTE*>(pixels.Scan0) + y * pixels.Stride;
    for (int x = 0; x < width; ++x) {
      const BYTE alpha = row[x * 4 + 3];
      if (alpha != 255) opaque = false;
      if (alpha != 0) {
        content.left = std::min(content.left, static_cast<LONG>(x));
        content.top = std::min(content.top, static_cast<LONG>(y));
        content.right = std::max(content.right, static_cast<LONG>(x + 1));
        content.bottom = std::max(content.bottom, static_cast<LONG>(y + 1));
      }
    }
  }
  detached->UnlockBits(&pixels);
  result.image = std::move(detached);
  result.size = {width, height};
  if (content.right > content.left && content.bottom > content.top)
    result.content_bounds = content;
  result.opaque = opaque;
  c.stats.source_bytes += bytes;
  ++c.stats.loaded_assets;
  ++c.stats.image_loads;
  return &result;
}

inline Surface* scaled(Asset& source, int width, int height, bool flip) {
  if (width <= 0 || height <= 0 || width > kMaxDimension ||
      height > kMaxDimension)
    return nullptr;
  Cache& c = cache();
  const ScaleKey key{source.id, width, height, flip};
  const auto found = c.scales.find(key);
  if (found != c.scales.end()) {
    found->second->used = ++c.clock;
    ++c.stats.cache_hits;
    return found->second.get();
  }
  const std::size_t bytes = static_cast<std::size_t>(width) * height * 4u;
  // Bound both bitmap handles and pixel storage across repeated zoom/resize.
  while (!c.scales.empty() &&
         (c.scales.size() >= kMaxScaledBitmaps ||
          bytes > kMaxScaledBytes - c.stats.scaled_bytes)) {
    const auto oldest = std::min_element(
        c.scales.begin(), c.scales.end(), [](const auto& a, const auto& b) {
          return a.second->used < b.second->used;
        });
    c.stats.scaled_bytes -= oldest->second->bytes();
    c.scales.erase(oldest);
  }
  auto result = std::make_unique<Surface>();
  if (!result->create(width, height)) return nullptr;
  {
    Gdiplus::Bitmap canvas(width, height, width * 4, PixelFormat32bppPARGB,
                           static_cast<BYTE*>(result->pixels));
    Gdiplus::Graphics graphics(&canvas);
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
    graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
    Gdiplus::ImageAttributes attributes;
    attributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
    const Gdiplus::Point points[3] = {
        {flip ? width : 0, 0}, {flip ? 0 : width, 0},
        {flip ? width : 0, height}};
    if (graphics.DrawImage(source.image.get(), points, 3, 0, 0,
                            source.size.width, source.size.height,
                            Gdiplus::UnitPixel, &attributes) != Gdiplus::Ok)
      return nullptr;
    graphics.Flush(Gdiplus::FlushIntentionSync);
  }
  result->used = ++c.clock;
  Surface* surface = result.get();
  c.scales.emplace(key, std::move(result));
  c.stats.scaled_bytes += bytes;
  c.stats.scaled_bitmaps = c.scales.size();
  ++c.stats.scale_builds;
  return surface;
}

inline bool composite(HDC dc, Surface& surface, int x, int y, bool opaque,
                      BYTE opacity) {
  bool drawn;
  if (opaque && opacity == 255) {
    drawn = BitBlt(dc, x, y, surface.width, surface.height, surface.dc,
                    0, 0, SRCCOPY) != FALSE;
  } else {
    const BLENDFUNCTION blend{AC_SRC_OVER, 0, opacity, AC_SRC_ALPHA};
    drawn = AlphaBlend(dc, x, y, surface.width, surface.height, surface.dc,
                        0, 0, surface.width, surface.height, blend) != FALSE;
  }
  if (drawn) ++cache().stats.draws;
  return drawn;
}

}  // namespace detail

// Explicit reload is useful to the art import/preview flow. Negative lookups
// otherwise stay cached so missing assets never cause repeated disk probes.
inline void reset_cache() {
  auto& c = detail::cache();
  c.scales.clear();
  c.assets.clear();
  c.stats = {};
  c.clock = 0;
  c.error.clear();
  c.root.clear();
  c.discovered_root = false;
}

inline void set_asset_root(const std::wstring& runtime_directory) {
  reset_cache();
  auto& c = detail::cache();
  c.root = runtime_directory;
  c.discovered_root = !runtime_directory.empty();
}

inline const std::wstring& asset_root() {
  auto& c = detail::cache();
  detail::discover_root(c);
  return c.root;
}

inline const std::wstring& last_error() { return detail::cache().error; }

inline CacheStats cache_stats() {
  auto stats = detail::cache().stats;
  stats.scaled_bitmaps = detail::cache().scales.size();
  return stats;
}

inline std::vector<std::string> missing_assets() {
  std::vector<std::string> names;
  for (const auto& [name, asset] : detail::cache().assets)
    if (!asset.image) names.push_back(name);
  return names;
}

inline Dimensions dimensions(const char* name) {
  const auto* source = detail::asset(name);
  return source ? source->size : Dimensions{};
}

// Cached at decode. Empty/missing assets return an empty RECT; bounds include
// every nonzero-alpha pixel, with right and bottom exclusive as in Win32 RECT.
inline RECT content_bounds(const char* name) {
  const auto* source = detail::asset(name);
  return source ? source->content_bounds : RECT{};
}

inline Dimensions content_dimensions(const char* name) {
  const RECT bounds = content_bounds(name);
  return {static_cast<int>(bounds.right - bounds.left),
          static_cast<int>(bounds.bottom - bounds.top)};
}

inline bool available(const char* name) { return detail::asset(name) != nullptr; }

inline std::size_t preload(std::initializer_list<const char*> names) {
  std::size_t count = 0;
  for (const char* name : names)
    if (available(name)) ++count;
  return count;
}

// Optional startup warmup: loads every canonical PNG without selecting a pose.
inline std::size_t preload_runtime() {
  const std::wstring root = asset_root();
  if (root.empty()) return 0;
  WIN32_FIND_DATAW file{};
  const HANDLE search = FindFirstFileW((root + L"\\*.png").c_str(), &file);
  if (search == INVALID_HANDLE_VALUE) return 0;
  std::size_t count = 0;
  do {
    if ((file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) continue;
    std::string name;
    for (const wchar_t ch : std::wstring_view(file.cFileName)) {
      if (ch > 127) {
        name.clear();
        break;
      }
      name.push_back(static_cast<char>(ch));
    }
    if (!name.empty() && available(name.c_str())) ++count;
  } while (FindNextFileW(search, &file));
  FindClose(search);
  return count;
}

inline bool warm_sprite(const char* name, int height, bool flip = false) {
  auto* source = detail::asset(name);
  if (!source || height <= 0 || height > detail::kMaxDimension) return false;
  const int width = std::max(1, static_cast<int>(std::lround(
      static_cast<double>(height) * source->size.width / source->size.height)));
  return detail::scaled(*source, width, height, flip) != nullptr;
}

inline bool draw_sprite(HDC dc, const char* name, int center_x, int feet_y,
                        int height, bool flip = false, float opacity = 1.0f) {
  if (!dc || height <= 0 || height > detail::kMaxDimension ||
      !std::isfinite(opacity))
    return false;
  auto* source = detail::asset(name);
  if (!source) return false;
  const BYTE alpha = static_cast<BYTE>(
      std::lround(std::clamp(opacity, 0.0f, 1.0f) * 255.0f));
  if (alpha == 0) return true;
  const int width = std::max(1, static_cast<int>(std::lround(
      static_cast<double>(height) * source->size.width / source->size.height)));
  const RECT bounds{center_x - width / 2, feet_y - height,
                    center_x - width / 2 + width, feet_y};
  if (!RectVisible(dc, &bounds)) return true;
  auto* surface = detail::scaled(*source, width, height, flip);
  return surface && detail::composite(dc, *surface, bounds.left, bounds.top,
                                      source->opaque, alpha);
}

// Static-prop sizing: use the visible alpha height to derive one uniform scale,
// but draw the entire canvas around its original bottom-center pivot. This
// deliberately retains transparent side/bottom margins; it never recenters or
// crops the content. Integer destination rounding can change visible height by
// one pixel at fractional scales. Animation frames must keep using draw_sprite
// so different poses retain the same common-canvas scale and grounded pivot.
inline bool draw_sprite_by_visible_height(HDC dc, const char* name, int center_x,
                                          int feet_y, int visible_height,
                                          bool flip = false,
                                          float opacity = 1.0f) {
  if (!dc || visible_height <= 0 || visible_height > detail::kMaxDimension ||
      !std::isfinite(opacity))
    return false;
  const auto* source = detail::asset(name);
  if (!source) return false;
  const int content_height = static_cast<int>(source->content_bounds.bottom -
                                             source->content_bounds.top);
  if (content_height <= 0) return false;
  const long long canvas_height =
      (static_cast<long long>(visible_height) * source->size.height +
       content_height / 2) / content_height;
  if (canvas_height > detail::kMaxDimension) return false;
  return draw_sprite(dc, name, center_x, feet_y,
                      static_cast<int>(canvas_height), flip, opacity);
}

// One whole texture fills one already-projected floor cell. Caller owns world
// projection/tile adjacency and may draw these into its existing FloorCache.
inline bool draw_ground(HDC dc, const char* name, const RECT& cell) {
  if (!dc) return false;
  const long long width = static_cast<long long>(cell.right) - cell.left;
  const long long height = static_cast<long long>(cell.bottom) - cell.top;
  if (width <= 0 || height <= 0 || width > detail::kMaxDimension ||
      height > detail::kMaxDimension)
    return false;
  auto* source = detail::asset(name);
  if (!source) return false;
  if (!RectVisible(dc, &cell)) return true;
  auto* surface = detail::scaled(*source, static_cast<int>(width),
                                 static_cast<int>(height), false);
  return surface && detail::composite(dc, *surface, cell.left, cell.top,
                                      source->opaque, 255);
}

}  // namespace raster_art

#endif  // _WIN32
