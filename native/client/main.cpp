#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <cstdint>
#include <thread>
#include <unordered_map>
#include <vector>

#include "verdigris/core.hpp"
#include "verdigris/seasonal.hpp"
#include "verdigris/networking.hpp"
#include "camera2d.hpp"
#include "render_list.hpp"
#include "remote_play.hpp"
#include "remote_session.hpp"
#include "presentation_state.hpp"
#include "session.hpp"

#ifdef VERDIGRIS_NATIVE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

// TASK-0141 data-only generated vector kit. Read-only consumption: the
// client never mutates these tables; art changes flow through the generator.
// The generator emits standards-conforming literals ("22.f"), so no
// reserved-suffix compatibility operators are needed here.
#include "assets/generated/visual_kit.h"

namespace {

using GpStatus = int;
using GpBitmap = void;
struct GdiplusStartupInput {
  UINT GdiplusVersion;
  void* DebugEventCallback;
  BOOL SuppressBackgroundThread;
  BOOL SuppressExternalCodecs;
};
using GdiplusStartupProc = GpStatus(WINAPI*)(ULONG_PTR*, const GdiplusStartupInput*, void*);
using GdiplusShutdownProc = void(WINAPI*)(ULONG_PTR);
using GdipCreateBitmapFromFileProc = GpStatus(WINAPI*)(const WCHAR*, GpBitmap**);
using GdipGetImageWidthProc = GpStatus(WINAPI*)(GpBitmap*, UINT*);
using GdipGetImageHeightProc = GpStatus(WINAPI*)(GpBitmap*, UINT*);
using GdipCreateHBITMAPFromBitmapProc = GpStatus(WINAPI*)(GpBitmap*, HBITMAP*, std::uint32_t);
using GdipCreateBitmapFromHBITMAPProc = GpStatus(WINAPI*)(HBITMAP, HPALETTE, GpBitmap**);
using GdipSaveImageToFileProc = GpStatus(WINAPI*)(GpBitmap*, const WCHAR*, const CLSID*,
                                                  const void*);
using GdipDisposeImageProc = GpStatus(WINAPI*)(GpBitmap*);
using AlphaBlendProc = BOOL(WINAPI*)(HDC, int, int, int, int, HDC, int, int, int, int,
                                     BLENDFUNCTION);

constexpr double kPi = 3.14159265358979323846;
// D-114 derives the visible ground envelope from the core's shared scale;
// eight grid tiles fill the arena half-extent at the default camera.
constexpr double kTileUnits =
    static_cast<double>(verdigris::world_scale::kArenaHalfExtent) / 8.0;
// D-118: clean top-down presentation. One orthographic camera with a uniform
// zoom; scale is camera-independent so no element can slide against movement.
constexpr double kCameraDefaultZoom = 0.85;
// Uniform zoom envelope: 0.5x..2x the default (TASK-0054). The spec framed
// this as "24-96 px/unit around camera2d.zoom (48)"; the client's actual
// played-verified unit is 0.85 px/world-unit, so the same 0.5x..2x envelope
// is applied here rather than re-scaling the whole world.
constexpr double kCameraMinZoom = kCameraDefaultZoom * 0.5;
constexpr double kCameraMaxZoom = kCameraDefaultZoom * 2.0;
// Adjustable top-down camera: zoom scales world units to pixels uniformly.
struct Camera {
  double x = 0.0;
  double y = 0.0;
  double zoom = kCameraDefaultZoom;  // pixels per world unit (uniform, both axes)
};

struct ScreenPoint {
  int x = 0;
  int y = 0;
  double scale = 1.0;
};

using verdigris::client::ActiveTelegraph;
using verdigris::client::EffectFx;
using verdigris::client::WorldActor;
using verdigris::client::WorldCarriedItem;
using verdigris::client::WorldView;

struct SpriteBitmap {
  HDC dc = nullptr;
  HDC mirror_dc = nullptr;
  HBITMAP bitmap = nullptr;
  HBITMAP mirror_bitmap = nullptr;
  HGDIOBJ old_bitmap = nullptr;
  HGDIOBJ old_mirror_bitmap = nullptr;
  int width = 0;
  int height = 0;
  int base_y = 0;

  bool ready() const {
    return dc != nullptr && mirror_dc != nullptr && width > 0 && height > 0;
  }

  void reset() {
    if (dc) {
      SelectObject(dc, old_bitmap);
      DeleteDC(dc);
    }
    if (mirror_dc) {
      SelectObject(mirror_dc, old_mirror_bitmap);
      DeleteDC(mirror_dc);
    }
    if (bitmap) DeleteObject(bitmap);
    if (mirror_bitmap) DeleteObject(mirror_bitmap);
    dc = nullptr;
    mirror_dc = nullptr;
    bitmap = nullptr;
    mirror_bitmap = nullptr;
    old_bitmap = nullptr;
    old_mirror_bitmap = nullptr;
    width = 0;
    height = 0;
    base_y = 0;
  }

  ~SpriteBitmap() { reset(); }
  SpriteBitmap() = default;
  SpriteBitmap(const SpriteBitmap&) = delete;
  SpriteBitmap& operator=(const SpriteBitmap&) = delete;
};

struct BillboardAssets {
  HMODULE gdiplus_module = nullptr;
  ULONG_PTR gdiplus_token = 0;
  GdiplusShutdownProc gdiplus_shutdown = nullptr;
  GdipCreateBitmapFromFileProc create_bitmap = nullptr;
  GdipGetImageWidthProc image_width = nullptr;
  GdipGetImageHeightProc image_height = nullptr;
  GdipCreateHBITMAPFromBitmapProc create_hbitmap = nullptr;
  GdipCreateBitmapFromHBITMAPProc create_bitmap_from_hbitmap = nullptr;
  GdipSaveImageToFileProc save_image_to_file = nullptr;
  GdipDisposeImageProc dispose_image = nullptr;
  HMODULE msimg32_module = nullptr;
  AlphaBlendProc alpha_blend = nullptr;
  SpriteBitmap player;
  SpriteBitmap raider;
  SpriteBitmap boss;
  SpriteBitmap tree;
  SpriteBitmap ruin;
  SpriteBitmap dwelling;
  SpriteBitmap shrine;
  SpriteBitmap terrain1;
  SpriteBitmap terrain4;
  std::string root;
  std::string status = "art: loading";
  std::string scenery_status = "scenery: loading";
  std::string terrain_status = "terrain: loading";

  ~BillboardAssets() {
    player.reset();
    raider.reset();
    boss.reset();
    tree.reset();
    ruin.reset();
    dwelling.reset();
    shrine.reset();
    terrain1.reset();
    terrain4.reset();
    if (gdiplus_shutdown && gdiplus_token) gdiplus_shutdown(gdiplus_token);
    if (gdiplus_module) FreeLibrary(gdiplus_module);
    if (msimg32_module) FreeLibrary(msimg32_module);
  }
  BillboardAssets() = default;
  BillboardAssets(const BillboardAssets&) = delete;
  BillboardAssets& operator=(const BillboardAssets&) = delete;
};

enum class SceneryKind { Tree, Ruin, Dwelling, Shrine };

struct SceneryItem {
  SceneryKind kind = SceneryKind::Tree;
  verdigris::Vec2 position{};
  double radius = static_cast<double>(verdigris::world_scale::kSceneryColliderRadius);
  double scale = 1.0;
  bool solid = true;
};

struct ClientState {
  std::unique_ptr<verdigris::Simulation> simulation;
  std::unique_ptr<verdigris::client::IClientSession> session;
  WorldView world;
  BillboardAssets billboards;
  std::vector<SceneryItem> scenery;
  bool w = false;
  bool a = false;
  bool s = false;
  bool d = false;
  POINT mouse{0, 0};
  Camera camera;
  verdigris::Vec2 last_aim_direction{1, 0};
  bool aim_direction_initialized = false;
  bool was_moving = false;
  std::vector<EffectFx> effects;
  std::unordered_map<std::string, ActiveTelegraph> telegraphs;
  std::unordered_map<std::string, verdigris::Vec2> loot_positions;
  verdigris::Vec2 last_death_pos{0, 0};
  std::size_t processed_events = 0;
  std::vector<std::string> event_log;
  int loot_scatter = 0;
  bool loot_labels = false;
  bool gear_overlay = false;
  bool debug_overlay = false;
  std::size_t selected_item = 0;
  std::string hint;
  int hint_ticks = 0;
  int screen_pulse_ticks = 0;
  render::List render_list;
};

std::string executable_directory() {
  char path[MAX_PATH]{};
  const DWORD length = GetModuleFileNameA(nullptr, path, MAX_PATH);
  if (length == 0 || length >= MAX_PATH) return {};
  std::string value(path, length);
  const std::size_t slash = value.find_last_of("\\/");
  return slash == std::string::npos ? std::string{} : value.substr(0, slash);
}

bool directory_exists(const std::string& path) {
  const DWORD attributes = GetFileAttributesA(path.c_str());
  return attributes != INVALID_FILE_ATTRIBUTES &&
         (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

std::wstring wide_path(const std::string& path) {
  const int required = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
  if (required <= 0) return {};
  std::wstring result(static_cast<std::size_t>(required), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, result.data(), required);
  result.resize(result.size() - 1);
  return result;
}

HBITMAP make_dib(const std::vector<std::uint8_t>& pixels, int width, int height,
                 HDC* dc_out, HGDIOBJ* old_bitmap_out) {
  BITMAPINFO info{};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = width;
  info.bmiHeader.biHeight = -height;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  void* destination = nullptr;
  HBITMAP bitmap = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &destination, nullptr, 0);
  if (!bitmap || !destination) {
    if (bitmap) DeleteObject(bitmap);
    return nullptr;
  }
  std::memcpy(destination, pixels.data(), pixels.size());
  HDC dc = CreateCompatibleDC(nullptr);
  if (!dc) {
    DeleteObject(bitmap);
    return nullptr;
  }
  HGDIOBJ old_bitmap = SelectObject(dc, bitmap);
  if (!old_bitmap || old_bitmap == HGDI_ERROR) {
    DeleteDC(dc);
    DeleteObject(bitmap);
    return nullptr;
  }
  *dc_out = dc;
  *old_bitmap_out = old_bitmap;
  return bitmap;
}

bool load_sprite(BillboardAssets& assets, const std::string& path, SpriteBitmap& sprite) {
  if (!assets.create_bitmap || !assets.image_width || !assets.image_height ||
      !assets.create_hbitmap || !assets.dispose_image)
    return false;
  const std::wstring filename = wide_path(path);
  if (filename.empty()) return false;
  GpBitmap* image = nullptr;
  if (assets.create_bitmap(filename.c_str(), &image) != 0 || !image) return false;
  UINT width = 0;
  UINT height = 0;
  HBITMAP source = nullptr;
  const bool dimensions_ok = assets.image_width(image, &width) == 0 &&
                             assets.image_height(image, &height) == 0 && width > 0 &&
                             height > 0;
  const bool bitmap_ok = dimensions_ok && assets.create_hbitmap(image, &source, 0) == 0 &&
                         source != nullptr;
  assets.dispose_image(image);
  if (!bitmap_ok) {
    if (source) DeleteObject(source);
    return false;
  }

  BITMAPINFO info{};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = static_cast<LONG>(width);
  info.bmiHeader.biHeight = -static_cast<LONG>(height);
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  std::vector<std::uint8_t> pixels(static_cast<std::size_t>(width) * height * 4);
  HDC screen = GetDC(nullptr);
  const int copied = GetDIBits(screen, source, 0, height, pixels.data(), &info,
                               DIB_RGB_COLORS);
  ReleaseDC(nullptr, screen);
  DeleteObject(source);
  if (copied == 0) return false;

  // Match the slice's keying and despill thresholds. Pixels are BGRA in the
  // DIB; alpha is premultiplied for AlphaBlend after the key is applied.
  for (std::size_t index = 0; index < pixels.size(); index += 4) {
    const int blue = pixels[index];
    const int green = pixels[index + 1];
    const int red = pixels[index + 2];
    const int magenta = std::min(red, blue) - green;
    int alpha = pixels[index + 3];
    if (magenta > 90) {
      alpha = 0;
    } else if (magenta > 28) {
      const double t = static_cast<double>(magenta - 28) / 62.0;
      alpha = static_cast<int>(alpha * (1.0 - t));
      pixels[index] = static_cast<std::uint8_t>(blue + (green - blue) * 0.6);
      pixels[index + 2] = static_cast<std::uint8_t>(red + (green - red) * 0.6);
    }
    pixels[index + 3] = static_cast<std::uint8_t>(alpha);
    pixels[index] = static_cast<std::uint8_t>(pixels[index] * alpha / 255);
    pixels[index + 1] = static_cast<std::uint8_t>(pixels[index + 1] * alpha / 255);
    pixels[index + 2] = static_cast<std::uint8_t>(pixels[index + 2] * alpha / 255);
  }

  int base_y = static_cast<int>(height) - 1;
  for (int y = static_cast<int>(height) - 1; y >= 0; --y) {
    bool opaque = false;
    for (UINT x = 0; x < width; x += 3) {
      if (pixels[(static_cast<std::size_t>(y) * width + x) * 4 + 3] > 40) {
        opaque = true;
        break;
      }
    }
    if (opaque) {
      base_y = y + 1;
      break;
    }
  }

  std::vector<std::uint8_t> mirrored(pixels.size());
  for (UINT y = 0; y < height; ++y) {
    for (UINT x = 0; x < width; ++x) {
      const std::size_t source_index = (static_cast<std::size_t>(y) * width + x) * 4;
      const std::size_t target_index =
          (static_cast<std::size_t>(y) * width + (width - x - 1)) * 4;
      std::memcpy(mirrored.data() + target_index, pixels.data() + source_index, 4);
    }
  }

  sprite.reset();
  sprite.width = static_cast<int>(width);
  sprite.height = static_cast<int>(height);
  sprite.base_y = base_y;
  sprite.bitmap = make_dib(pixels, sprite.width, sprite.height, &sprite.dc,
                           &sprite.old_bitmap);
  sprite.mirror_bitmap = make_dib(mirrored, sprite.width, sprite.height,
                                  &sprite.mirror_dc, &sprite.old_mirror_bitmap);
  if (!sprite.ready()) {
    sprite.reset();
    return false;
  }
  return true;
}

bool load_terrain_plate(BillboardAssets& assets, const std::string& path,
                        SpriteBitmap& sprite) {
  if (!assets.create_bitmap || !assets.image_width || !assets.image_height ||
      !assets.create_hbitmap || !assets.dispose_image)
    return false;
  const std::wstring filename = wide_path(path);
  if (filename.empty()) return false;
  GpBitmap* image = nullptr;
  if (assets.create_bitmap(filename.c_str(), &image) != 0 || !image) return false;
  UINT width = 0;
  UINT height = 0;
  HBITMAP source = nullptr;
  const bool dimensions_ok = assets.image_width(image, &width) == 0 &&
                             assets.image_height(image, &height) == 0 && width > 0 &&
                             height > 0;
  const bool bitmap_ok = dimensions_ok && assets.create_hbitmap(image, &source, 0) == 0 &&
                         source != nullptr;
  assets.dispose_image(image);
  if (!bitmap_ok) {
    if (source) DeleteObject(source);
    return false;
  }

  BITMAPINFO info{};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = static_cast<LONG>(width);
  info.bmiHeader.biHeight = -static_cast<LONG>(height);
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  std::vector<std::uint8_t> pixels(static_cast<std::size_t>(width) * height * 4);
  HDC screen = GetDC(nullptr);
  const int copied = GetDIBits(screen, source, 0, height, pixels.data(), &info,
                               DIB_RGB_COLORS);
  ReleaseDC(nullptr, screen);
  DeleteObject(source);
  if (copied == 0) return false;

  for (std::size_t index = 0; index < pixels.size(); index += 4)
    pixels[index + 3] = 255;

  // 0075 rev2: the floor is context, not content. Desaturate toward
  // luminance and pull the plate down toward the scene's dark base so
  // actors, FX, and loot stay dominant.
  for (std::size_t index = 0; index < pixels.size(); index += 4) {
    const int b = pixels[index + 0];
    const int g = pixels[index + 1];
    const int r = pixels[index + 2];
    const int luma = (r * 54 + g * 183 + b * 19) >> 8;
    const auto tone = [luma](int channel) {
      const int desaturated = (channel + luma) / 2;   // 50% toward grey
      return static_cast<std::uint8_t>((desaturated * 140) >> 8);  // ~55% brightness
    };
    pixels[index + 0] = tone(b);
    pixels[index + 1] = tone(g);
    pixels[index + 2] = tone(r);
  }

  std::vector<std::uint8_t> mirrored(pixels.size());
  for (UINT y = 0; y < height; ++y) {
    for (UINT x = 0; x < width; ++x) {
      const std::size_t source_index = (static_cast<std::size_t>(y) * width + x) * 4;
      const std::size_t target_index =
          (static_cast<std::size_t>(y) * width + (width - x - 1)) * 4;
      std::memcpy(mirrored.data() + target_index, pixels.data() + source_index, 4);
    }
  }

  sprite.reset();
  sprite.width = static_cast<int>(width);
  sprite.height = static_cast<int>(height);
  sprite.base_y = static_cast<int>(height);
  sprite.bitmap = make_dib(pixels, sprite.width, sprite.height, &sprite.dc,
                           &sprite.old_bitmap);
  sprite.mirror_bitmap = make_dib(mirrored, sprite.width, sprite.height,
                                  &sprite.mirror_dc, &sprite.old_mirror_bitmap);
  if (!sprite.ready()) {
    sprite.reset();
    return false;
  }
  return true;
}

bool initialize_gdiplus(BillboardAssets& assets) {
  assets.gdiplus_module = LoadLibraryA("gdiplus.dll");
  if (!assets.gdiplus_module) return false;
  assets.gdiplus_shutdown = reinterpret_cast<GdiplusShutdownProc>(
      GetProcAddress(assets.gdiplus_module, "GdiplusShutdown"));
  auto startup = reinterpret_cast<GdiplusStartupProc>(
      GetProcAddress(assets.gdiplus_module, "GdiplusStartup"));
  assets.create_bitmap = reinterpret_cast<GdipCreateBitmapFromFileProc>(
      GetProcAddress(assets.gdiplus_module, "GdipCreateBitmapFromFile"));
  assets.image_width = reinterpret_cast<GdipGetImageWidthProc>(
      GetProcAddress(assets.gdiplus_module, "GdipGetImageWidth"));
  assets.image_height = reinterpret_cast<GdipGetImageHeightProc>(
      GetProcAddress(assets.gdiplus_module, "GdipGetImageHeight"));
  assets.create_hbitmap = reinterpret_cast<GdipCreateHBITMAPFromBitmapProc>(
      GetProcAddress(assets.gdiplus_module, "GdipCreateHBITMAPFromBitmap"));
  assets.create_bitmap_from_hbitmap = reinterpret_cast<GdipCreateBitmapFromHBITMAPProc>(
      GetProcAddress(assets.gdiplus_module, "GdipCreateBitmapFromHBITMAP"));
  assets.save_image_to_file = reinterpret_cast<GdipSaveImageToFileProc>(
      GetProcAddress(assets.gdiplus_module, "GdipSaveImageToFile"));
  assets.dispose_image = reinterpret_cast<GdipDisposeImageProc>(
      GetProcAddress(assets.gdiplus_module, "GdipDisposeImage"));
  if (!startup || !assets.gdiplus_shutdown || !assets.create_bitmap ||
      !assets.image_width || !assets.image_height || !assets.create_hbitmap ||
      !assets.dispose_image)
    return false;
  GdiplusStartupInput input{1, nullptr, FALSE, FALSE};
  if (startup(&assets.gdiplus_token, &input, nullptr) != 0) return false;
  return true;
}

std::vector<std::string> billboard_roots() {
  std::vector<std::string> roots;
  // TASK-0142: discovery must hold from the repository root, the build
  // directory, and an installed-style directory where plates ship beside the
  // executable. Every candidate is checked for existence before any load, so
  // a miss is cheap and silent.
  const std::string executable = executable_directory();
  if (!executable.empty()) {
    // Installed-style: an assets folder shipped next to the executable.
    roots.push_back(executable + "\\assets");
    for (int depth = 1; depth <= 6; ++depth) {
      std::string prefix = executable;
      for (int part = 0; part < depth; ++part) prefix += "\\..";
      // Installed-style: assets folder beside a nested install root.
      roots.push_back(prefix + "\\assets");
      // Repository checkout reached by walking up from the build directory.
      roots.push_back(prefix + "\\prototypes\\founding-slice\\assets");
    }
  }
  // Repository checkout relative to the current working directory (the
  // historical layout, kept last so an explicit install always wins).
  std::string prefix = ".";
  for (int depth = 0; depth <= 4; ++depth) {
    roots.push_back(prefix + "\\prototypes\\founding-slice\\assets");
    prefix += "\\..";
  }
  return roots;
}

// TASK-0142: one honest source for the art status copy. The text is always
// derived from what is actually ready, never from what a load attempt hoped
// for, so the owner-facing HUD cannot claim assets it does not have.
void refresh_art_status(BillboardAssets& assets) {
  if (assets.player.ready() && assets.raider.ready() && assets.boss.ready())
    assets.status = "art: PNG billboards loaded";
  else
    assets.status = std::string("art: embedded vector kit ") +
                    verdigris::visual_kit::kKitVersion + " (procedural placeholder)";
  if (assets.tree.ready() && assets.ruin.ready() && assets.dwelling.ready() &&
      assets.shrine.ready())
    assets.scenery_status = "scenery: PNG plates loaded";
  else
    assets.scenery_status = "scenery: embedded vector kit (procedural placeholder)";
  if (assets.terrain1.ready() && assets.terrain4.ready())
    assets.terrain_status = "terrain: PNG plates tiled";
  else
    assets.terrain_status = "terrain: embedded vector kit tiles (procedural placeholder)";
}

void load_billboards(BillboardAssets& assets) {
  assets.msimg32_module = LoadLibraryA("msimg32.dll");
  assets.alpha_blend = reinterpret_cast<AlphaBlendProc>(
      assets.msimg32_module ? GetProcAddress(assets.msimg32_module, "AlphaBlend") : nullptr);
  if (!assets.alpha_blend || !initialize_gdiplus(assets)) {
    refresh_art_status(assets);
    return;
  }
  for (const auto& root : billboard_roots()) {
    if (!directory_exists(root)) continue;
    const bool actors_loaded =
        load_sprite(assets, root + "\\scion_str.png", assets.player) &&
        load_sprite(assets, root + "\\raider.png", assets.raider) &&
        load_sprite(assets, root + "\\boss.png", assets.boss);
    if (actors_loaded) {
      assets.root = root;
    } else {
      assets.player.reset();
      assets.raider.reset();
      assets.boss.reset();
    }

    const bool scenery_loaded =
        load_sprite(assets, root + "\\tree.png", assets.tree) &&
        load_sprite(assets, root + "\\ruin.png", assets.ruin) &&
        load_sprite(assets, root + "\\dwelling.png", assets.dwelling) &&
        load_sprite(assets, root + "\\shrine.png", assets.shrine);
    if (scenery_loaded) {
      if (assets.root.empty()) assets.root = root;
    } else {
      assets.tree.reset();
      assets.ruin.reset();
      assets.dwelling.reset();
      assets.shrine.reset();
    }

    const bool terrain_loaded =
        load_terrain_plate(assets, root + "\\terrain1.png", assets.terrain1) &&
        load_terrain_plate(assets, root + "\\terrain4.png", assets.terrain4);
    if (terrain_loaded) {
      if (assets.root.empty()) assets.root = root;
    } else {
      assets.terrain1.reset();
      assets.terrain4.reset();
    }

    // One honest status refresh for whatever actually loaded — the early
    // return must not skip it.
    refresh_art_status(assets);
    if (actors_loaded || scenery_loaded || terrain_loaded) return;
  }
  refresh_art_status(assets);
}

std::uint64_t scenery_seed(const std::string& route_id) {
  // FNV-1a keeps the route-to-layout mapping stable across processes and
  // platforms without coupling presentation randomness to Simulation::Rng.
  std::uint64_t value = 1469598103934665603ULL;
  for (unsigned char character : route_id) {
    value ^= character;
    value *= 1099511628211ULL;
  }
  return value;
}

class SceneryRng {
 public:
  explicit SceneryRng(std::uint64_t seed) : state_(seed) {}

  std::uint64_t next() {
    std::uint64_t value = (state_ += 0x9E3779B97F4A7C15ULL);
    value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;
    return value ^ (value >> 31U);
  }

  double unit() {
    return static_cast<double>(next() >> 11U) /
           static_cast<double>(1ULL << 53U);
  }

  double range(double minimum, double maximum) {
    return minimum + (maximum - minimum) * unit();
  }

 private:
  std::uint64_t state_;
};

void add_scenery(std::vector<SceneryItem>& scenery, SceneryKind kind, double x,
                 double y, double radius, bool solid, double scale) {
  scenery.push_back({kind, {static_cast<int>(std::lround(x)),
                            static_cast<int>(std::lround(y))},
                     radius, scale, solid});
}

void generate_scenery(ClientState& state) {
  state.scenery.clear();
  std::string route_id = state.world.route_id;
  if (state.simulation) route_id = state.simulation->instance().route_id;
  else if (state.session) route_id = state.session->model().scene.id;
  if (route_id.empty()) return;

  SceneryRng rng(scenery_seed(route_id));
  const bool village = route_id.find(":1:") != std::string::npos;
  const bool fields = route_id.find(":2:") != std::string::npos;
  const double tree_radius =
      static_cast<double>(verdigris::world_scale::kSceneryColliderRadius);
  const double structure_radius = tree_radius * 1.6;
  const double monument_radius = tree_radius * 1.5;
  if (village) {
    add_scenery(state.scenery, SceneryKind::Dwelling, -320, -260, structure_radius, true, 1.0);
    add_scenery(state.scenery, SceneryKind::Dwelling, 340, -300, structure_radius, true, 1.0);
    add_scenery(state.scenery, SceneryKind::Dwelling, -420, 180, structure_radius, true, 1.0);
    add_scenery(state.scenery, SceneryKind::Shrine, 60, -460, monument_radius, true, 1.0);
    // A near-field tree makes the grounded depth boundary easy to read in the
    // client lab while the remaining placements keep the route spacious.
    add_scenery(state.scenery, SceneryKind::Tree, 260, -100, tree_radius, true, 1.0);
    add_scenery(state.scenery, SceneryKind::Tree, -700, -500, tree_radius, true, 1.0);
    add_scenery(state.scenery, SceneryKind::Tree, 720, -420, tree_radius, true, 1.15);
    add_scenery(state.scenery, SceneryKind::Tree, -780, 420, tree_radius, true, 1.0);
    for (int i = 0; i < 5; ++i)
      add_scenery(state.scenery, SceneryKind::Tree,
                  rng.range(-verdigris::world_scale::kArenaHalfExtent,
                             verdigris::world_scale::kArenaHalfExtent),
                  rng.range(-650, 650), tree_radius, true, rng.range(.78, 1.15));
  } else if (fields) {
    add_scenery(state.scenery, SceneryKind::Ruin, -200, -380, monument_radius, true, 1.0);
    add_scenery(state.scenery, SceneryKind::Tree, 420, -520, tree_radius, true, 1.2);
    add_scenery(state.scenery, SceneryKind::Tree, -640, 240, tree_radius, true, 1.0);
    add_scenery(state.scenery, SceneryKind::Tree, 680, 380, tree_radius, true, .9);
    for (int i = 0; i < 8; ++i)
      add_scenery(state.scenery, SceneryKind::Tree,
                  rng.range(-verdigris::world_scale::kArenaHalfExtent,
                             verdigris::world_scale::kArenaHalfExtent),
                  rng.range(-700, 700), tree_radius, true, rng.range(.75, 1.2));
  } else {
    const int variant = static_cast<int>(rng.next() % 3);
    if (variant == 0) {
      add_scenery(state.scenery, SceneryKind::Ruin, -360, -320, monument_radius, true, 1.0);
      add_scenery(state.scenery, SceneryKind::Ruin, 360, -360, monument_radius, true, 1.1);
      add_scenery(state.scenery, SceneryKind::Shrine, 0, -600, monument_radius, true, 1.1);
    } else if (variant == 1) {
      add_scenery(state.scenery, SceneryKind::Shrine, 0, -420, monument_radius, true, 1.1);
      add_scenery(state.scenery, SceneryKind::Ruin, 360, -420, monument_radius, true, 1.0);
    } else {
      add_scenery(state.scenery, SceneryKind::Dwelling, -320, -300, structure_radius, true, 1.0);
      add_scenery(state.scenery, SceneryKind::Shrine, 300, -420, monument_radius, true, 1.0);
    }
    for (int i = 0; i < 8; ++i)
      add_scenery(state.scenery, SceneryKind::Tree,
                  rng.range(-verdigris::world_scale::kArenaHalfExtent,
                             verdigris::world_scale::kArenaHalfExtent),
                  rng.range(-700, 700), tree_radius, true, rng.range(.78, 1.18));
  }
}

ClientState* state_from(HWND window) {
  return reinterpret_cast<ClientState*>(GetWindowLongPtr(window, GWLP_USERDATA));
}

bool is_remote(const ClientState& state) { return static_cast<bool>(state.session); }

void sync_world(ClientState& state) {
  if (state.simulation)
    verdigris::client::sync_world_from_simulation(state.world, *state.simulation);
  else if (state.session)
    verdigris::client::sync_world_from_model(state.world, state.session->model());
}

void ingest_session_events(ClientState& state) {
  if (!state.session) return;
  verdigris::client::PresentationFx fx;
  fx.effects = std::move(state.effects);
  fx.telegraphs = std::move(state.telegraphs);
  fx.loot_positions = std::move(state.loot_positions);
  fx.last_death_pos = state.last_death_pos;
  fx.loot_scatter = state.loot_scatter;
  fx.screen_pulse_ticks = state.screen_pulse_ticks;
  fx.event_log = std::move(state.event_log);
  fx.hint = state.hint;
  fx.hint_ticks = state.hint_ticks;
  ++state.world.tick;
  const std::string route_before = state.world.route_id;
  for (const auto& event : state.session->drain_events()) {
    verdigris::client::apply_presentation_event(fx, state.world, event, state.world.tick);
    if (!fx.hint.empty()) {
      state.hint = fx.hint;
      state.hint_ticks = fx.hint_ticks;
    }
  }
  sync_world(state);
  if (state.world.route_id != route_before) generate_scenery(state);
  state.effects = std::move(fx.effects);
  state.telegraphs = std::move(fx.telegraphs);
  state.loot_positions = std::move(fx.loot_positions);
  state.last_death_pos = fx.last_death_pos;
  state.loot_scatter = fx.loot_scatter;
  state.screen_pulse_ticks = fx.screen_pulse_ticks;
  state.event_log = std::move(fx.event_log);
}

void submit_move(ClientState& state, int dx, int dy) {
  if (state.session)
    state.session->submit(verdigris::client::ClientCommand::move(dx, dy));
  else if (state.simulation)
    state.simulation->dispatch(verdigris::Command::move(dx, dy));
}

void submit_aim(ClientState& state, int dx, int dy) {
  if (state.session)
    state.session->submit(verdigris::client::ClientCommand::aim(dx, dy));
  else if (state.simulation)
    state.simulation->dispatch(verdigris::Command::aim(dx, dy));
}

void submit_action(ClientState& state, verdigris::ActionType action, const char* remote_name) {
  if (state.session) {
    state.session->submit(verdigris::client::ClientCommand::use_action(remote_name));
    return;
  }
  if (state.simulation) state.simulation->dispatch(verdigris::Command::action_use(action));
}

void submit_pick_up(ClientState& state, const std::string& id) {
  if (state.session)
    state.session->submit(verdigris::client::ClientCommand::pick_up(id));
  else if (state.simulation)
    state.simulation->dispatch(verdigris::Command::pick_up(id));
}

void submit_equip(ClientState& state, const std::string& id) {
  if (state.session)
    state.session->submit(verdigris::client::ClientCommand::equip(id));
  else if (state.simulation)
    state.simulation->dispatch(verdigris::Command::equip(id));
}

void submit_extract(ClientState& state) {
  if (state.session)
    state.session->submit(verdigris::client::ClientCommand::extract());
  else if (state.simulation)
    state.simulation->dispatch(verdigris::Command::extract());
}

void show_hint(ClientState& state, const std::string& message) {
  state.hint = message;
  state.hint_ticks = 80;
}

struct SkillInfo {
  char key;
  const char* name;
  verdigris::ActionType action;
};

constexpr SkillInfo kSkills[] = {
    {'Q', "Thrust", verdigris::ActionType::Thrust},
    {'E', "Sweep", verdigris::ActionType::Sweep},
    {'R', "WarCry", verdigris::ActionType::WarCry},
};

int skill_resource_cost(const verdigris::PresentationCatalog& catalog,
                        verdigris::ActionType action) {
  switch (action) {
    case verdigris::ActionType::Thrust: return catalog.thrust_resource_cost;
    case verdigris::ActionType::Sweep: return catalog.sweep_resource_cost;
    case verdigris::ActionType::WarCry: return catalog.war_cry_resource_cost;
    default: return 0;
  }
}

const SkillInfo* skill_for_key(WPARAM key) {
  for (const auto& skill : kSkills)
    if (key == static_cast<WPARAM>(skill.key)) return &skill;
  return nullptr;
}

void dispatch_skill(ClientState& state, const SkillInfo& skill) {
  // Do not duplicate target/range/cooldown rules in the client.  A key press
  // is a presentation request; the core decides whether it resolves.
  const char* remote = "melee";
  if (skill.action == verdigris::ActionType::Thrust) remote = "thrust";
  else if (skill.action == verdigris::ActionType::Sweep) remote = "sweep";
  else if (skill.action == verdigris::ActionType::WarCry) remote = "war-cry";
  submit_action(state, skill.action, remote);
  show_hint(state, std::string(skill.name) + " requested");
}

std::string nearest_pickup_id(const ClientState& state) {
  if (is_remote(state)) return {};
  const auto* player = state.simulation->actor(state.simulation->scion().actor_id);
  if (!player) return {};

  std::string best_id;
  int best_distance = std::numeric_limits<int>::max();
  int best_kind = std::numeric_limits<int>::max();
  std::size_t best_order = std::numeric_limits<std::size_t>::max();
  auto consider = [&](const std::string& id, int kind, std::size_t order) {
    auto position = state.loot_positions.find(id);
    if (position == state.loot_positions.end()) return;
    const int distance = verdigris::manhattan_distance(player->position, position->second);
    if (distance < best_distance ||
        (distance == best_distance &&
         (kind < best_kind || (kind == best_kind && order < best_order)))) {
      best_id = id;
      best_distance = distance;
      best_kind = kind;
      best_order = order;
    }
  };

  // Items win equal-distance ties over trophies; vector order is stable for
  // equal-kind candidates and therefore deterministic across a run.
  for (std::size_t i = 0; i < state.simulation->ground_items().size(); ++i)
    consider(state.simulation->ground_items()[i].id, 0, i);
  for (std::size_t i = 0; i < state.simulation->ground_trophies().size(); ++i)
    consider(state.simulation->ground_trophies()[i].id, 1, i);
  return best_id;
}

void equip_selected(ClientState& state) {
  if (state.world.carried.empty()) {
    show_hint(state, "Gear empty: pick up an item first");
    return;
  }
  state.selected_item = std::min(state.selected_item, state.world.carried.size() - 1);
  submit_equip(state, state.world.carried[state.selected_item].id);
  show_hint(state, "Equipped " + state.world.carried[state.selected_item].name);
}

COLORREF fade_to_background(COLORREF color, double remaining) {
  const double t = std::clamp(remaining, 0.0, 1.0);
  const int bg_r = 23;
  const int bg_g = 29;
  const int bg_b = 32;
  return RGB(static_cast<int>(bg_r + (GetRValue(color) - bg_r) * t),
             static_cast<int>(bg_g + (GetGValue(color) - bg_g) * t),
             static_cast<int>(bg_b + (GetBValue(color) - bg_b) * t));
}

ScreenPoint project(const Camera& camera, const RECT& bounds, double wx, double wy) {
  const camera2d::Camera cam{camera.x, camera.y, camera.zoom};
  const camera2d::Screen screen{bounds.right, bounds.bottom};
  const camera2d::Point point = camera2d::project(cam, screen, wx, wy);
  return {point.x, point.y, point.scale};
}

// Inverse of project() on the ground plane, good enough for mouse aim.
void unproject(const Camera& camera, const RECT& bounds, int sx, int sy, double& wx,
               double& wy) {
  const camera2d::Camera cam{camera.x, camera.y, camera.zoom};
  const camera2d::Screen screen{bounds.right, bounds.bottom};
  camera2d::unproject(cam, screen, sx, sy, wx, wy);
}

void fill_ellipse(HDC dc, int cx, int cy, int rx, int ry, COLORREF color) {
  HBRUSH brush = CreateSolidBrush(color);
  HPEN pen = CreatePen(PS_SOLID, 1, color);
  HGDIOBJ old_brush = SelectObject(dc, brush);
  HGDIOBJ old_pen = SelectObject(dc, pen);
  Ellipse(dc, cx - rx, cy - ry, cx + rx, cy + ry);
  SelectObject(dc, old_brush);
  SelectObject(dc, old_pen);
  DeleteObject(brush);
  DeleteObject(pen);
}

void ring_ellipse(HDC dc, int cx, int cy, int rx, int ry, COLORREF color, int width) {
  HPEN pen = CreatePen(PS_SOLID, width, color);
  HGDIOBJ old_pen = SelectObject(dc, pen);
  HGDIOBJ old_brush = SelectObject(dc, GetStockObject(NULL_BRUSH));
  Ellipse(dc, cx - rx, cy - ry, cx + rx, cy + ry);
  SelectObject(dc, old_brush);
  SelectObject(dc, old_pen);
  DeleteObject(pen);
}

void draw_line(HDC dc, int x0, int y0, int x1, int y1, COLORREF color, int width) {
  HPEN pen = CreatePen(PS_SOLID, width, color);
  HGDIOBJ old_pen = SelectObject(dc, pen);
  MoveToEx(dc, x0, y0, nullptr);
  LineTo(dc, x1, y1);
  SelectObject(dc, old_pen);
  DeleteObject(pen);
}

// ── TASK-0142: embedded vector kit renderer ─────────────────────────────
// Draws the TASK-0141 generated symbols (verdigris::visual_kit) straight
// from the data-only header with plain GDI fills. This is the deterministic
// owner-facing fallback when PNG/GDI+ plates are unavailable — no files are
// consulted, so it renders identically on every machine. The kit is a
// placeholder art pass, never claimed as final owner-approved art.

namespace kit = verdigris::visual_kit;

COLORREF kit_color(int index) {
  const kit::Color& color = kit::kColors[index];
  // GDI has no per-shape alpha; blend translucent kit colors over the dark
  // scene background so authored shadows/flames keep their softness.
  const double alpha = std::clamp(static_cast<double>(color.a), 0.0, 1.0);
  const auto mix = [alpha](float channel, int background) {
    const double foreground = static_cast<double>(channel) * 255.0;
    return static_cast<int>(background + (foreground - background) * alpha + 0.5);
  };
  return RGB(mix(color.r, 23), mix(color.g, 29), mix(color.b, 32));
}

const kit::Symbol* kit_symbol(const char* role, const char* motif = nullptr) {
  for (int i = 0; i < kit::kSymbolCount; ++i) {
    const kit::Symbol& symbol = kit::kSymbols[i];
    if (std::strcmp(symbol.role, role) != 0) continue;
    if (motif && std::strcmp(symbol.motif, motif) != 0) continue;
    return &symbol;
  }
  return nullptr;
}

struct KitPlacement {
  HDC dc;
  double origin_x;   // screen x of the viewBox left edge
  double origin_y;   // screen y of the viewBox top edge
  double scale;      // pixels per viewBox unit
  double box_width;  // authored symbol width in viewBox units
  bool mirror;       // flip horizontally around the viewBox center
};

POINT kit_point(const KitPlacement& placement, float x, float y) {
  const double units_x =
      placement.mirror ? placement.box_width - static_cast<double>(x)
                       : static_cast<double>(x);
  return {static_cast<int>(std::lround(placement.origin_x +
                                       units_x * placement.scale)),
          static_cast<int>(std::lround(placement.origin_y +
                                       static_cast<double>(y) * placement.scale))};
}

void draw_kit_shape(const KitPlacement& placement, const kit::Shape& shape) {
  constexpr int kColorCount =
      static_cast<int>(sizeof(kit::kColors) / sizeof(kit::kColors[0]));
  const bool has_fill =
      shape.fill >= 0 && shape.fill < kColorCount;
  const bool has_stroke =
      shape.stroke >= 0 && shape.stroke < kColorCount;
  if (!has_fill && !has_stroke) return;
  COLORREF fill_color = has_fill ? kit_color(shape.fill) : 0;
  const int stroke_w =
      has_stroke ? std::max(1, static_cast<int>(std::lround(
                                     shape.stroke_width * placement.scale)))
                 : 1;

  HBRUSH brush = nullptr;
  HPEN pen = nullptr;
  HGDIOBJ old_brush = nullptr;
  HGDIOBJ old_pen = nullptr;
  HGDIOBJ old_hollow = nullptr;
  if (has_fill) {
    brush = CreateSolidBrush(fill_color);
    old_brush = SelectObject(placement.dc, brush);
  } else {
    // Stroke-only shapes must not inherit whatever brush the DC last used.
    old_hollow = SelectObject(placement.dc, GetStockObject(NULL_BRUSH));
  }
  if (has_stroke) {
    pen = CreatePen(PS_SOLID, stroke_w, kit_color(shape.stroke));
    old_pen = SelectObject(placement.dc, pen);
  }

  switch (shape.kind) {
    case kit::ShapeKind::Polygon:
    case kit::ShapeKind::Polyline: {
      const int count = shape.point_end - shape.point_begin;
      if (count > 1) {
        std::vector<POINT> points(static_cast<std::size_t>(count));
        for (int p = 0; p < count; ++p) {
          points[static_cast<std::size_t>(p)] = kit_point(
              placement, kit::kPoints[(shape.point_begin + p) * 2],
              kit::kPoints[(shape.point_begin + p) * 2 + 1]);
        }
        if (shape.kind == kit::ShapeKind::Polygon)
          Polygon(placement.dc, points.data(), count);
        else
          Polyline(placement.dc, points.data(), count);
      }
      break;
    }
    case kit::ShapeKind::Circle:
    case kit::ShapeKind::Ellipse: {
      const double units_x =
          placement.mirror ? placement.box_width - static_cast<double>(shape.cx)
                           : static_cast<double>(shape.cx);
      const int cx = static_cast<int>(
          std::lround(placement.origin_x + units_x * placement.scale));
      const int cy = static_cast<int>(std::lround(
          placement.origin_y + static_cast<double>(shape.cy) * placement.scale));
      const int rx =
          std::max(1, static_cast<int>(std::lround(
                           static_cast<double>(shape.rx) * placement.scale)));
      const int ry_raw = shape.kind == kit::ShapeKind::Ellipse
                             ? static_cast<int>(shape.ry)
                             : static_cast<int>(shape.rx);
      const int ry =
          std::max(1, static_cast<int>(std::lround(
                           static_cast<double>(ry_raw) * placement.scale)));
      Ellipse(placement.dc, cx - rx, cy - ry, cx + rx, cy + ry);
      break;
    }
  }

  if (old_pen) SelectObject(placement.dc, old_pen);
  if (old_hollow) SelectObject(placement.dc, old_hollow);
  if (old_brush) SelectObject(placement.dc, old_brush);
  if (pen) DeleteObject(pen);
  if (brush) DeleteObject(brush);
}

// Draws one symbol standing on (base_x, base_y). pixel_height scales the
// authored 64-unit box; the motif's ground baseline sits ~90% down the box,
// so feet land on the contact point and the lower margin overlaps the
// contact shadow like a keyed PNG plate would.
void draw_kit_symbol(HDC dc, const kit::Symbol& symbol, int base_x, int base_y,
                     int pixel_height, bool mirror) {
  if (pixel_height <= 0 || symbol.width <= 0 || symbol.height <= 0) return;
  constexpr double kGroundFraction = 58.0 / 64.0;
  const double scale = static_cast<double>(pixel_height) /
                       static_cast<double>(symbol.height);
  const int baseline_offset =
      static_cast<int>(std::lround(static_cast<double>(pixel_height) *
                                   kGroundFraction));
  KitPlacement placement{dc,
                         static_cast<double>(base_x) -
                             static_cast<double>(symbol.width) * scale * 0.5,
                         static_cast<double>(base_y - baseline_offset), scale,
                         static_cast<double>(symbol.width), mirror};
  for (int i = symbol.shape_begin; i < symbol.shape_end; ++i)
    draw_kit_shape(placement, kit::kShapes[i]);
}

void draw_contact_shadow(HDC dc, const ScreenPoint& base, double world_radius) {
  const int rx = std::max(3, static_cast<int>(world_radius * base.scale));
  const int ry = std::max(2, static_cast<int>(world_radius * base.scale * 0.8));
  fill_ellipse(dc, base.x, base.y, rx, ry, RGB(14, 18, 20));
}

// TASK-0142: a squashed ground ring in team colors so friend/foe reads at a
// glance even before the silhouette resolves.
void draw_team_ring(HDC dc, const ScreenPoint& base, double world_radius,
                    COLORREF color) {
  const int rx = std::max(5, static_cast<int>(world_radius * base.scale));
  const int ry = std::max(3, static_cast<int>(world_radius * base.scale * 0.62));
  ring_ellipse(dc, base.x, base.y, rx, ry, color, 2);
}

// A billboard stands vertically on its ground point regardless of camera pitch.
void draw_billboard(HDC dc, const ScreenPoint& base, double world_width,
                    double world_height, COLORREF body, COLORREF trim) {
  const int half_w = std::max(3, static_cast<int>(world_width * base.scale * 0.5));
  const int height = std::max(6, static_cast<int>(world_height * base.scale));
  RECT body_rect{base.x - half_w, base.y - height, base.x + half_w, base.y};
  HBRUSH brush = CreateSolidBrush(body);
  FillRect(dc, &body_rect, brush);
  DeleteObject(brush);
  const int head_r = std::max(2, half_w - 2);
  fill_ellipse(dc, base.x, base.y - height, head_r, head_r, trim);
}

bool draw_billboard_sprite(const BillboardAssets& assets, HDC dc, const SpriteBitmap& sprite,
                           const ScreenPoint& base, double world_height, int facing_x) {
  if (!sprite.ready() || !assets.alpha_blend) return false;
  const int destination_height =
      std::max(6, static_cast<int>(world_height * base.scale));
  const int destination_width = std::max(
      4, static_cast<int>(static_cast<double>(sprite.width) / sprite.height *
                          destination_height));
  const int destination_foot = std::clamp(
      static_cast<int>(static_cast<double>(sprite.base_y) / sprite.height *
                       destination_height),
      1, destination_height);
  const int destination_x = base.x - destination_width / 2;
  const int destination_y = base.y - destination_foot;
  const HDC source = facing_x < 0 ? sprite.mirror_dc : sprite.dc;
  BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  return assets.alpha_blend(dc, destination_x, destination_y, destination_width,
                            destination_height, source, 0, 0, sprite.width,
                            sprite.height, blend) != FALSE;
}

const SpriteBitmap& scenery_sprite(const BillboardAssets& assets, SceneryKind kind) {
  switch (kind) {
    case SceneryKind::Tree: return assets.tree;
    case SceneryKind::Ruin: return assets.ruin;
    case SceneryKind::Dwelling: return assets.dwelling;
    case SceneryKind::Shrine: return assets.shrine;
  }
  return assets.tree;
}

const char* scenery_kit_role(SceneryKind kind) {
  switch (kind) {
    case SceneryKind::Tree: return "tree";
    case SceneryKind::Ruin: return "ruin";
    case SceneryKind::Dwelling: return "dwelling";
    case SceneryKind::Shrine: return "shrine";
  }
  return "tree";
}

double scenery_height(SceneryKind kind) {
  switch (kind) {
    case SceneryKind::Tree: return kTileUnits * 3.0;
    case SceneryKind::Ruin: return kTileUnits * 2.6;
    case SceneryKind::Dwelling: return kTileUnits * 2.3;
    case SceneryKind::Shrine: return kTileUnits * 2.0;
  }
  return kTileUnits * 2.0;
}

void draw_scenery_fallback(HDC dc, const ScreenPoint& base, const SceneryItem& item,
                           const Camera& camera) {
  const int radius = std::max(4, static_cast<int>(item.radius * base.scale));
  const int height = std::max(8, static_cast<int>(scenery_height(item.kind) *
                                                   item.scale * base.scale));
  if (item.kind == SceneryKind::Tree) {
    fill_ellipse(dc, base.x, base.y - height + radius, radius * 2, radius,
                 RGB(48, 86, 61));
    draw_line(dc, base.x, base.y - height / 2, base.x, base.y,
              RGB(94, 66, 42), std::max(2, radius / 5));
  } else if (item.kind == SceneryKind::Dwelling) {
    RECT body{base.x - radius, base.y - height / 2, base.x + radius, base.y};
    HBRUSH wall = CreateSolidBrush(RGB(113, 86, 63));
    FillRect(dc, &body, wall);
    DeleteObject(wall);
    POINT roof[3] = {{base.x - radius - 4, body.top},
                     {base.x, body.top - radius},
                     {base.x + radius + 4, body.top}};
    HBRUSH roof_brush = CreateSolidBrush(RGB(127, 61, 49));
    HGDIOBJ old = SelectObject(dc, roof_brush);
    Polygon(dc, roof, 3);
    SelectObject(dc, old);
    DeleteObject(roof_brush);
  } else if (item.kind == SceneryKind::Ruin) {
    RECT stone{base.x - radius, base.y - height, base.x + radius, base.y};
    HBRUSH wall = CreateSolidBrush(RGB(84, 92, 91));
    FillRect(dc, &stone, wall);
    DeleteObject(wall);
    draw_line(dc, stone.left, stone.top + height / 3, stone.right,
              stone.top + height / 3, RGB(37, 44, 44), 2);
    draw_line(dc, stone.left + radius / 2, stone.top, stone.left + radius / 2,
              stone.bottom, RGB(37, 44, 44), 2);
  } else {
    fill_ellipse(dc, base.x, base.y - height + radius, radius, radius,
                 RGB(157, 130, 78));
    draw_line(dc, base.x, base.y - height + radius, base.x, base.y,
              RGB(112, 88, 54), std::max(2, radius / 4));
  }
  // Keep fallback proportions visibly grounded at the same camera squash as a
  // loaded plate; this also prevents a missing plate from floating above its
  // contact shadow.
  (void)camera;
}

void draw_scenery_item(const BillboardAssets& assets, HDC dc, const Camera& camera,
                       const RECT& bounds, const SceneryItem& item,
                       render::List& rl) {
  const ScreenPoint base =
      project(camera, bounds, item.position.x, item.position.y);
  rl.push_back({render::Op::Scenery, static_cast<double>(base.x),
                static_cast<double>(base.y), 0.0, 0,
                static_cast<int>(item.kind) == 0   ? "tree"
                : static_cast<int>(item.kind) == 1 ? "ruin"
                : static_cast<int>(item.kind) == 2 ? "dwelling"
                                                   : "shrine"});
  draw_contact_shadow(dc, base, item.radius * 0.9);
  const SpriteBitmap& sprite = scenery_sprite(assets, item.kind);
  if (!draw_billboard_sprite(assets, dc, sprite, base,
                             scenery_height(item.kind) * item.scale, 1)) {
    // TASK-0142: deterministic vector-kit silhouette before the geometric
    // last resort, so a machine without PNG plates still reads as a game.
    const int kit_height =
        std::max(8, static_cast<int>(scenery_height(item.kind) * item.scale *
                                     base.scale));
    if (const kit::Symbol* symbol = kit_symbol(scenery_kit_role(item.kind)))
      draw_kit_symbol(dc, *symbol, base.x, base.y, kit_height, false);
    else
      draw_scenery_fallback(dc, base, item, camera);
  }
}

void draw_ground_grid(HDC dc, const Camera& camera, const RECT& bounds) {
  const double range = static_cast<double>(verdigris::world_scale::kArenaHalfExtent);
  const double start_x = std::floor((camera.x - range) / kTileUnits) * kTileUnits;
  const double start_y = std::floor((camera.y - range) / kTileUnits) * kTileUnits;
  for (double gx = start_x; gx <= camera.x + range; gx += kTileUnits) {
    const ScreenPoint a = project(camera, bounds, gx, camera.y - range);
    const ScreenPoint b = project(camera, bounds, gx, camera.y + range);
    draw_line(dc, a.x, a.y, b.x, b.y, RGB(33, 41, 44), 1);
  }
  for (double gy = start_y; gy <= camera.y + range; gy += kTileUnits) {
    const ScreenPoint a = project(camera, bounds, camera.x - range, gy);
    const ScreenPoint b = project(camera, bounds, camera.x + range, gy);
    draw_line(dc, a.x, a.y, b.x, b.y, RGB(33, 41, 44), 1);
  }
}

bool terrain_theme_prefers_alt(const std::string& route_id) {
  return route_id.find("marsh") != std::string::npos ||
         route_id.find("barrow") != std::string::npos ||
         route_id.find("circle") != std::string::npos;
}

std::uint32_t terrain_tile_hash(int tx, int ty) {
  std::uint32_t hash = static_cast<std::uint32_t>(tx) * 374761393U +
                       static_cast<std::uint32_t>(ty) * 668265263U;
  hash ^= hash >> 13;
  hash *= 1274126177U;
  hash ^= hash >> 16;
  return hash;
}

bool terrain_tile_uses_alt(int tx, int ty, bool theme_alt) {
  // 0075 rev2: one dominant plate with an occasional variant — a 50/50-ish
  // mix read as a loud checkerboard.
  const int bucket = static_cast<int>(terrain_tile_hash(tx, ty) % 100);
  return theme_alt ? bucket >= 12 : bucket < 12;
}

bool draw_terrain_tile(HDC dc, const SpriteBitmap& sprite, int dest_x, int dest_y,
                       int dest_w, int dest_h, std::uint32_t hash) {
  if (!sprite.ready()) return false;
  // 0075 rev2: sample a hashed quadrant of the plate per tile so the grain
  // reads finer than one plate-per-tile, without new assets.
  const int src_w = std::max(1, sprite.width / 2);
  const int src_h = std::max(1, sprite.height / 2);
  const int src_x = (hash & 1u) ? src_w : 0;
  const int src_y = (hash & 2u) ? src_h : 0;
  return StretchBlt(dc, dest_x, dest_y, dest_w, dest_h, sprite.dc, src_x, src_y,
                    src_w, src_h, SRCCOPY) != FALSE;
}

void draw_floor(const BillboardAssets& assets, HDC dc, const Camera& camera,
                const RECT& bounds, const std::string& route_id, render::List& rl) {
  const bool tiled = assets.terrain1.ready() && assets.terrain4.ready();
  // TASK-0142: with the embedded vector kit the floor stays textured even
  // when PNG plates are missing — the "tiled" contract is honest in both
  // paths because real tiles are drawn.
  const kit::Symbol* motif_primary =
      tiled ? nullptr : kit_symbol("terrain", "grass-court");
  const kit::Symbol* motif_alt =
      tiled ? nullptr : kit_symbol("terrain", "mossy-stone");
  const bool vector_tiled = motif_primary && motif_alt;
  rl.push_back({render::Op::Floor, 0.0, 0.0, 0.0,
                (tiled || vector_tiled) ? 1 : 0,
                (tiled || vector_tiled) ? "tiled" : "flat"});

  HBRUSH background = CreateSolidBrush(RGB(23, 29, 32));
  FillRect(dc, &bounds, background);
  DeleteObject(background);

  if (!tiled && !vector_tiled) {
    draw_ground_grid(dc, camera, bounds);
    return;
  }

  const double range = static_cast<double>(verdigris::world_scale::kArenaHalfExtent);
  const double tile = kTileUnits;
  const int start_tx = static_cast<int>(std::floor((camera.x - range) / tile));
  const int end_tx = static_cast<int>(std::ceil((camera.x + range) / tile));
  const int start_ty = static_cast<int>(std::floor((camera.y - range) / tile));
  const int end_ty = static_cast<int>(std::ceil((camera.y + range) / tile));
  const bool theme_alt = terrain_theme_prefers_alt(route_id);
  const double half = tile * 0.5;

  for (int ty = start_ty; ty <= end_ty; ++ty) {
    for (int tx = start_tx; tx <= end_tx; ++tx) {
      const double wx = static_cast<double>(tx) * tile;
      const double wy = static_cast<double>(ty) * tile;
      // 0075 rev2: seam-free tiling — adjacent tiles share projected corner
      // coordinates exactly, so no background grid bleeds through.
      const ScreenPoint corner0 = project(camera, bounds, wx, wy);
      const ScreenPoint corner1 = project(camera, bounds, wx + tile, wy + tile);
      const ScreenPoint center = project(camera, bounds, wx + half, wy + half);
      const bool use_alt = terrain_tile_uses_alt(tx, ty, theme_alt);
      const std::string label =
          std::string(use_alt ? "terrain4" : "terrain1") + ":" + std::to_string(tx) + ":" +
          std::to_string(ty);
      rl.push_back({render::Op::Tile, static_cast<double>(center.x),
                    static_cast<double>(center.y), half * center.scale, 0, label});
      if (tiled) {
        const SpriteBitmap& sprite = use_alt ? assets.terrain4 : assets.terrain1;
        draw_terrain_tile(dc, sprite, corner0.x, corner0.y, corner1.x - corner0.x,
                          corner1.y - corner0.y, terrain_tile_hash(tx, ty) >> 8);
      } else if (vector_tiled) {
        // Deterministic vector motif tile: same hashed dominant/variant mix
        // as the PNG path (marsh/barrow/circle themes favor mossy stone).
        const kit::Symbol& motif = use_alt ? *motif_alt : *motif_primary;
        const int dest_w = corner1.x - corner0.x;
        const int dest_h = corner1.y - corner0.y;
        if (dest_w > 0 && dest_h > 0 && corner0.x < bounds.right &&
            corner0.y < bounds.bottom && corner1.x > 0 && corner1.y > 0) {
          const double scale = static_cast<double>(dest_w) /
                               static_cast<double>(motif.width);
          KitPlacement placement{dc, static_cast<double>(corner0.x),
                                 static_cast<double>(corner0.y), scale,
                                 static_cast<double>(motif.width), false};
          for (int i = motif.shape_begin; i < motif.shape_end; ++i)
            draw_kit_shape(placement, kit::kShapes[i]);
        }
      }
    }
  }
}

// HUD reserve (TASK-0068 / TASK-0076): minimap top-left; orbs + quickbar along
// the bottom edge. FX must clip or fade before entering these rects.
struct HudSafeZones {
  RECT minimap{};
  RECT bottom_hud{};
};

HudSafeZones hud_safe_zones(const RECT& bounds) {
  HudSafeZones zones;
  zones.minimap = {0, 0, 132, 132};
  const int bottom = static_cast<int>(bounds.bottom);
  const int right = static_cast<int>(bounds.right);
  zones.bottom_hud = {0, std::max(0, bottom - 96), right, bottom};
  return zones;
}

bool circle_hits_rect(double x, double y, double radius, const RECT& rc) {
  const double nearest_x =
      std::clamp(x, static_cast<double>(rc.left), static_cast<double>(rc.right));
  const double nearest_y =
      std::clamp(y, static_cast<double>(rc.top), static_cast<double>(rc.bottom));
  const double dx = x - nearest_x;
  const double dy = y - nearest_y;
  return dx * dx + dy * dy <= radius * radius;
}

double clamp_radius_from_hud(double x, double y, double radius, const RECT& bounds) {
  const HudSafeZones zones = hud_safe_zones(bounds);
  double r = radius;
  while (r > 4.0 && (circle_hits_rect(x, y, r, zones.minimap) ||
                     circle_hits_rect(x, y, r, zones.bottom_hud)))
    r -= 2.0;
  if (circle_hits_rect(x, y, std::max(r, 4.0), zones.minimap) ||
      circle_hits_rect(x, y, std::max(r, 4.0), zones.bottom_hud))
    return 0.0;
  return r;
}

bool telegraph_avoids_hud(const render::List& list, const RECT& bounds) {
  const HudSafeZones zones = hud_safe_zones(bounds);
  for (const auto& item : list) {
    if (item.op != render::Op::Telegraph) continue;
    const double radius = std::max(item.radius, 4.0);
    if (circle_hits_rect(item.x, item.y, radius, zones.minimap) ||
        circle_hits_rect(item.x, item.y, radius, zones.bottom_hud))
      return false;
  }
  return true;
}

COLORREF telegraph_color(double visibility, COLORREF source) {
  return fade_to_background(source, std::clamp(visibility, 0.0, 1.0));
}

void draw_thrust_telegraph(HDC dc, const Camera& camera, const RECT& bounds,
                           const ActiveTelegraph& telegraph, double visibility,
                           double length, render::List& rl) {
  const double facing_x = static_cast<double>(telegraph.facing.x);
  const double facing_y = static_cast<double>(telegraph.facing.y);
  const double angle = std::atan2(facing_y, facing_x);
  const ScreenPoint base = project(camera, bounds, telegraph.position.x,
                                   telegraph.position.y);
  rl.push_back({render::Op::Telegraph, static_cast<double>(base.x),
                static_cast<double>(base.y), 0.0, 0, "thrust"});
  // The shared resolver accepts the full forward half-plane (strict dot > 0),
  // so the warning uses a broad 180-degree fan instead of promising a
  // narrower client-only hit cone.
  constexpr int kSegments = 12;
  POINT points[kSegments + 2]{};
  points[0] = {base.x, base.y};
  for (int i = 0; i <= kSegments; ++i) {
    const double a = angle - kPi * 0.5 + kPi * static_cast<double>(i) / kSegments;
    const double wx = telegraph.position.x + std::cos(a) * length;
    const double wy = telegraph.position.y + std::sin(a) * length;
    const ScreenPoint point = project(camera, bounds, wx, wy);
    points[i + 1] = {point.x, point.y};
  }
  const COLORREF fill = telegraph_color(visibility * 0.38, RGB(214, 52, 52));
  const COLORREF edge = telegraph_color(visibility, RGB(238, 72, 64));
  HBRUSH brush = CreateSolidBrush(fill);
  HPEN pen = CreatePen(PS_SOLID, 2, edge);
  HGDIOBJ old_brush = SelectObject(dc, brush);
  HGDIOBJ old_pen = SelectObject(dc, pen);
  Polygon(dc, points, kSegments + 2);
  SelectObject(dc, old_brush);
  SelectObject(dc, old_pen);
  DeleteObject(brush);
  DeleteObject(pen);
  // A centerline and a short origin ring make the warning readable when the
  // wedge is projected nearly edge-on at the current camera pitch.
  const ScreenPoint tip = project(
      camera, bounds, telegraph.position.x + std::cos(angle) * length,
      telegraph.position.y + std::sin(angle) * length);
  draw_line(dc, base.x, base.y, tip.x, tip.y, edge, 2);
  const int origin_r = std::max(4, static_cast<int>(kTileUnits * 0.18 * base.scale));
  const double clamped =
      clamp_radius_from_hud(static_cast<double>(base.x), static_cast<double>(base.y),
                            static_cast<double>(origin_r), bounds);
  rl.back().radius = clamped;
  if (clamped <= 0.0) {
    rl.pop_back();
    return;
  }
  ring_ellipse(dc, base.x, base.y, static_cast<int>(clamped), static_cast<int>(clamped),
               edge, 2);
}

void draw_sweep_telegraph(HDC dc, const Camera& camera, const RECT& bounds,
                          const ActiveTelegraph& telegraph, double visibility,
                          double radius_world, render::List& rl) {
  const ScreenPoint base = project(camera, bounds, telegraph.position.x,
                                   telegraph.position.y);
  const int radius = std::max(4, static_cast<int>(radius_world * base.scale));
  const double clamped =
      clamp_radius_from_hud(static_cast<double>(base.x), static_cast<double>(base.y),
                            static_cast<double>(radius), bounds);
  if (clamped <= 0.0) return;
  rl.push_back({render::Op::Telegraph, static_cast<double>(base.x),
                static_cast<double>(base.y), clamped, 0, "sweep"});
  const COLORREF fill = telegraph_color(visibility * 0.28, RGB(214, 52, 52));
  const COLORREF edge = telegraph_color(visibility, RGB(238, 72, 64));
  const int draw_r = static_cast<int>(clamped);
  fill_ellipse(dc, base.x, base.y, draw_r, draw_r, fill);
  ring_ellipse(dc, base.x, base.y, draw_r, draw_r, edge, 3);
  if (draw_r > 12)
    ring_ellipse(dc, base.x, base.y, draw_r - 10, draw_r - 10,
                 telegraph_color(visibility * 0.82, RGB(255, 112, 82)), 1);
}

double telegraph_visibility(const ClientState& state,
                            const ActiveTelegraph& telegraph) {
  const std::uint64_t now = state.world.tick != 0 || !state.simulation
                                ? state.world.tick
                                : state.simulation->tick();
  const std::uint64_t elapsed_ticks =
      now >= telegraph.start_tick ? now - telegraph.start_tick : 0;
  const double progress = std::clamp(
      static_cast<double>(elapsed_ticks) /
          std::max(1, telegraph.windup_ticks),
      0.0, 1.0);
  const double pulse = 0.72 + 0.28 *
      std::sin((static_cast<double>(elapsed_ticks) + 0.25) * 2.35);
  // Always readable on the first frame, then intensify toward the strike.
  return std::clamp((0.38 + 0.62 * progress) * pulse, 0.18, 1.0);
}

void paint_telegraphs(const ClientState& state, HDC dc, const RECT& bounds,
                      render::List& rl) {
  const verdigris::PresentationCatalog catalog =
      verdigris::Simulation::presentation_catalog();
  const int saved = SaveDC(dc);
  const HudSafeZones zones = hud_safe_zones(bounds);
  ExcludeClipRect(dc, zones.minimap.left, zones.minimap.top, zones.minimap.right,
                  zones.minimap.bottom);
  ExcludeClipRect(dc, zones.bottom_hud.left, zones.bottom_hud.top, zones.bottom_hud.right,
                  zones.bottom_hud.bottom);
  for (const auto& entry : state.telegraphs) {
    const ActiveTelegraph& telegraph = entry.second;
    const double visibility = telegraph_visibility(state, telegraph);
    if (telegraph.action == "sweep")
      draw_sweep_telegraph(dc, state.camera, bounds, telegraph, visibility,
                           catalog.melee_range, rl);
    else
      draw_thrust_telegraph(dc, state.camera, bounds, telegraph, visibility,
                            catalog.thrust_range, rl);
  }
  RestoreDC(dc, saved);
}

void draw_effect(HDC dc, const Camera& camera, const RECT& bounds, const EffectFx& fx,
                 render::List& rl) {
  const ScreenPoint base = project(camera, bounds, fx.wx, fx.wy);
  const double life = 1.0 - static_cast<double>(fx.age) / fx.ttl;
  const double grow = static_cast<double>(fx.age) / fx.ttl;
  switch (fx.kind) {
    case EffectFx::Kind::Swing: {
      rl.push_back({render::Op::Swing, static_cast<double>(base.x),
                    static_cast<double>(base.y)});
      // A readable melee arc sweeping toward the aim angle, drawn flat on the
      // top-down ground plane.
      const int radius = static_cast<int>(kTileUnits * 1.1 * base.scale);
      const COLORREF color = fade_to_background(RGB(226, 220, 180), life);
      const double spread = kPi * 0.45;
      const double sweep = fx.angle - spread * 0.5 + spread * grow;
      for (int i = 0; i < 3; ++i) {
        const double a = sweep - i * 0.12;
        const int x1 = base.x + static_cast<int>(std::cos(a) * radius);
        const int y1 = base.y + static_cast<int>(std::sin(a) * radius);
        draw_line(dc, base.x, base.y, x1, y1, color, i == 0 ? 3 : 1);
      }
      break;
    }
    case EffectFx::Kind::SweepArc: {
      // Sweep is an area action in the core.  A complete circle keeps the
      // presentation honest about that area instead of implying a single
      // facing direction that the deterministic action does not own.
      const int radius = static_cast<int>(kTileUnits * (0.72 + grow * 0.62) *
                                          base.scale);
      rl.push_back({render::Op::Sweep, static_cast<double>(base.x),
                    static_cast<double>(base.y), static_cast<double>(radius)});
      const COLORREF color = fade_to_background(RGB(116, 204, 208), life);
      ring_ellipse(dc, base.x, base.y, radius, radius, color, 3);
      if (radius > 8)
        ring_ellipse(dc, base.x, base.y, radius - 7, radius - 7, color, 1);
      break;
    }
    case EffectFx::Kind::WarCryAura: {
      // A short-lived, expanding aura communicates the buff event without
      // turning the renderer into a second source of gameplay state.
      const int radius = static_cast<int>(kTileUnits * (0.38 + grow * 0.72) *
                                          base.scale);
      rl.push_back({render::Op::WarCry, static_cast<double>(base.x),
                    static_cast<double>(base.y), static_cast<double>(radius)});
      const COLORREF color = fade_to_background(RGB(239, 190, 78), life);
      ring_ellipse(dc, base.x, base.y, radius, radius, color, 3);
      ring_ellipse(dc, base.x, base.y, std::max(3, radius - 8),
                   std::max(3, radius - 8), fade_to_background(RGB(255, 224, 128), life), 1);
      break;
    }
    case EffectFx::Kind::Impact: {
      rl.push_back({render::Op::Impact, static_cast<double>(base.x),
                    static_cast<double>(base.y)});
      const int r = std::max(4, static_cast<int>(kTileUnits * 0.35 * base.scale));
      fill_ellipse(dc, base.x, base.y, r, r, fade_to_background(RGB(255, 214, 120), life));
      break;
    }
    case EffectFx::Kind::DeathRing: {
      rl.push_back({render::Op::Death, static_cast<double>(base.x),
                    static_cast<double>(base.y)});
      const int rx = static_cast<int>(kTileUnits * (0.3 + grow * 1.5) * base.scale);
      ring_ellipse(dc, base.x, base.y, rx, rx,
                   fade_to_background(RGB(214, 118, 86), life), 2);
      break;
    }
    case EffectFx::Kind::Dust: {
      const COLORREF color = fade_to_background(RGB(126, 118, 98), life * 0.8);
      for (int i = 0; i < 5; ++i) {
        const double a = fx.angle + i * (2.0 * kPi / 5.0);
        const double d = kTileUnits * (0.2 + grow * 0.9);
        const ScreenPoint p = project(camera, bounds, fx.wx + std::cos(a) * d,
                                      fx.wy + std::sin(a) * d);
        const int r = std::max(2, static_cast<int>(kTileUnits * 0.12 * p.scale));
        fill_ellipse(dc, p.x, p.y, r, r, color);
      }
      break;
    }
    case EffectFx::Kind::Sparkle: {
      const double pulse = 0.6 + 0.4 * std::sin(fx.age * 0.9);
      const int r = std::max(2, static_cast<int>(kTileUnits * 0.18 * base.scale * pulse));
      const COLORREF color = fade_to_background(RGB(240, 214, 120), life);
      draw_line(dc, base.x - r, base.y, base.x + r, base.y, color, 1);
      draw_line(dc, base.x, base.y - r, base.x, base.y + r, color, 1);
      break;
    }
    case EffectFx::Kind::DamageNumber: {
      rl.push_back({render::Op::Damage, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, fx.value,
                    fx.damage_to_player ? "player" : "monster"});
      const int lift = static_cast<int>(kTileUnits * (0.35 + grow * 0.75) * base.scale);
      const COLORREF color = fx.damage_to_player ? RGB(255, 118, 104) : RGB(240, 218, 132);
      SetBkMode(dc, TRANSPARENT);
      // TASK-0142: bold numerals so the resolved damage reads instantly.
      const int font_h = std::clamp(
          static_cast<int>(kTileUnits * 0.34 * base.scale), 13, 22);
      HFONT number_font = CreateFontA(font_h, 0, 0, 0, FW_BOLD, FALSE, FALSE,
                                      FALSE, DEFAULT_CHARSET,
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      DEFAULT_QUALITY, FF_SWISS,
                                      "Verdana");
      HGDIOBJ old_number_font = SelectObject(dc, number_font);
      // Rise AND fade toward the background over the effect lifetime.
      SetTextColor(dc, fade_to_background(color, life));
      const std::string text = std::to_string(fx.value);
      TextOutA(dc, base.x - 9, base.y - lift, text.c_str(),
               static_cast<int>(text.size()));
      SelectObject(dc, old_number_font);
      DeleteObject(number_font);
      break;
    }
    case EffectFx::Kind::TargetFlash: {
      rl.push_back({render::Op::TargetFlash, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, 0,
                    fx.damage_to_player ? "player" : "monster"});
      // A brief bright ring over the hit target reads as a tint on the sprite.
      const int r = std::max(6, static_cast<int>(kTileUnits * 0.5 * base.scale));
      ring_ellipse(dc, base.x, base.y, r, r,
                   fade_to_background(RGB(255, 244, 190), life), 3);
      fill_ellipse(dc, base.x, base.y, r / 2, r / 2,
                   fade_to_background(RGB(255, 238, 160), life));
      break;
    }
  }
}

const char* event_label(verdigris::EventType type) {
  using verdigris::EventType;
  switch (type) {
    case EventType::AttackStarted: return "attack";
    case EventType::AttackTelegraphed: return "telegraph";
    case EventType::DamageApplied: return "damage";
    case EventType::ActorDied: return "death";
    case EventType::ItemDropped: return "item drop";
    case EventType::TrophyDropped: return "trophy drop";
    case EventType::ItemPickedUp: return "pickup";
    case EventType::TrophyPickedUp: return "pickup";
    case EventType::ItemEquipped: return "equip";
    case EventType::ItemExtracted: return "extracted item";
    case EventType::TrophyExtracted: return "extracted trophy";
    case EventType::HouseStoreChanged: return "house store";
    case EventType::RouteUnlocked: return "route unlocked";
    case EventType::SeasonalRewardGranted: return "seasonal reward";
    case EventType::BuffApplied: return "buff";
    case EventType::BuffExpired: return "buff expired";
    default: return nullptr;
  }
}

double aim_angle(const ClientState& state, const RECT& bounds, double from_x,
                 double from_y) {
  double wx = from_x + kTileUnits;
  double wy = from_y;
  unproject(state.camera, bounds, state.mouse.x, state.mouse.y, wx, wy);
  return std::atan2(wy - from_y, wx - from_x);
}

verdigris::Vec2 quantized_mouse_aim(const ClientState& state, const RECT& bounds,
                                    const verdigris::Actor& player) {
  double wx = static_cast<double>(player.position.x) + kTileUnits;
  double wy = static_cast<double>(player.position.y);
  unproject(state.camera, bounds, state.mouse.x, state.mouse.y, wx, wy);
  const double dx = wx - static_cast<double>(player.position.x);
  const double dy = wy - static_cast<double>(player.position.y);
  const int qx = dx < 0.0 ? -1 : dx > 0.0 ? 1 : 0;
  const int qy = dy < 0.0 ? -1 : dy > 0.0 ? 1 : 0;
  return {qx, qy};
}

void dispatch_aim_if_changed(ClientState& state, const RECT& bounds, bool force = false) {
  sync_world(state);
  if (!state.world.player.alive) return;
  verdigris::Actor stand_in;
  stand_in.position = state.world.player.position;
  stand_in.facing = state.world.player.facing;
  const verdigris::Vec2 direction = quantized_mouse_aim(state, bounds, stand_in);
  if (direction.x == 0 && direction.y == 0) return;
  if (!force && state.aim_direction_initialized &&
      state.last_aim_direction.x == direction.x &&
      state.last_aim_direction.y == direction.y) {
    return;
  }
  // The quantized heading is the only client-side state used for throttling;
  // facing and all consequences remain authoritative in the core.
  submit_aim(state, direction.x, direction.y);
  state.last_aim_direction = direction;
  state.aim_direction_initialized = true;
}

// Turn new simulation events into procedural presentation effects.
void ingest_events(ClientState& state, const RECT& bounds) {
  if (!state.simulation) return;
  const auto& sim = *state.simulation;
  const auto& events = sim.events();
  for (; state.processed_events < events.size(); ++state.processed_events) {
    const auto& event = events[state.processed_events];
    const verdigris::Actor* subject =
        event.actor_id.empty() ? nullptr : sim.actor(event.actor_id);
    const double ex = subject ? subject->position.x : state.last_death_pos.x;
    const double ey = subject ? subject->position.y : state.last_death_pos.y;
    switch (event.type) {
      case verdigris::EventType::AttackTelegraphed:
        if (subject && subject->alive && subject->kind == verdigris::ActorKind::Monster &&
            subject->elite) {
          ActiveTelegraph telegraph;
          telegraph.actor_id = event.actor_id;
          telegraph.action = event.text;
          telegraph.position = subject->position;
          telegraph.facing = subject->facing;
          telegraph.start_tick = event.tick;
          telegraph.windup_ticks = std::max(1, event.value);
          state.telegraphs[event.actor_id] = std::move(telegraph);
        }
        break;
      case verdigris::EventType::AttackStarted:
        // A strike (including one which is ultimately absorbed by a gate in
        // the core) ends the presentation warning for this actor.
        state.telegraphs.erase(event.actor_id);
        state.effects.push_back({event.text == "sweep" ? EffectFx::Kind::SweepArc
                                                         : EffectFx::Kind::Swing,
                                 ex, ey, aim_angle(state, bounds, ex, ey), 0,
                                 event.text == "sweep" ? 8 : 6});
        break;
      case verdigris::EventType::BuffApplied:
        if (event.text == "war-cry")
          state.effects.push_back({EffectFx::Kind::WarCryAura, ex, ey, 0.0, 0, 14});
        break;
      case verdigris::EventType::DamageApplied: {
        const bool to_player =
            subject && subject->kind == verdigris::ActorKind::Player;
        state.effects.push_back({EffectFx::Kind::Impact, ex, ey, 0.0, 0, 4});
        // Brief tint on the hit target's sprite so "what I hit" reads at a
        // glance, separate from the position flash.
        EffectFx flash;
        flash.kind = EffectFx::Kind::TargetFlash;
        flash.wx = ex;
        flash.wy = ey;
        flash.age = 0;
        flash.ttl = 4;
        flash.damage_to_player = to_player;
        state.effects.push_back(flash);
        // Floating damage number: the value the core resolved, rising and
        // fading above the target over ~600ms. Red when the Scion took the hit.
        EffectFx number;
        number.kind = EffectFx::Kind::DamageNumber;
        number.wx = ex;
        number.wy = ey;
        number.age = 0;
        number.ttl = 12;
        number.value = event.value;
        number.damage_to_player = to_player;
        state.effects.push_back(number);
        // A 150ms screen-edge red pulse when the player takes damage.
        if (to_player) state.screen_pulse_ticks = 3;
        break;
      }
      case verdigris::EventType::ActorDied:
        state.telegraphs.erase(event.actor_id);
        // The core cancels all elite windups when the Scion dies; clear any
        // remaining client records at the same event boundary.
        if (event.text == "scion" ||
            (subject && subject->kind == verdigris::ActorKind::Player))
          state.telegraphs.clear();
        if (subject) state.last_death_pos = subject->position;
        state.effects.push_back({EffectFx::Kind::DeathRing, ex, ey, 0.0, 0, 12});
        state.effects.push_back({EffectFx::Kind::Dust, ex, ey, 0.7, 0, 10});
        break;
      case verdigris::EventType::InstanceEntered:
        // A route transition invalidates all event-time actor snapshots.
        state.telegraphs.clear();
        generate_scenery(state);
        break;
      case verdigris::EventType::ActorMoved:
        if (event.text == "dash")
          state.effects.push_back({EffectFx::Kind::Dust, ex, ey, 0.2, 0, 8});
        break;
      case verdigris::EventType::ItemDropped:
      case verdigris::EventType::TrophyDropped: {
        // The simulation keeps loot abstract; scatter it around the kill site.
        verdigris::Vec2 at = state.last_death_pos;
        at.x += (state.loot_scatter % 3 - 1) * 40;
        at.y += ((state.loot_scatter / 3) % 3 - 1) * 40 + 30;
        ++state.loot_scatter;
        const std::string& id = event.item_id.empty() ? event.trophy_id : event.item_id;
        state.loot_positions[id] = at;
        state.effects.push_back({EffectFx::Kind::Sparkle, static_cast<double>(at.x),
                                 static_cast<double>(at.y), 0.0, 0, 24});
        break;
      }
      case verdigris::EventType::ItemPickedUp:
      case verdigris::EventType::TrophyPickedUp: {
        const std::string& id = event.item_id.empty() ? event.trophy_id : event.item_id;
        state.loot_positions.erase(id);
        break;
      }
      default:
        break;
    }
    if (const char* label = event_label(event.type)) {
      std::string line = label;
      if (!event.text.empty()) line += " " + event.text;
      if (event.value != 0) line += " (" + std::to_string(event.value) + ")";
      state.event_log.push_back(line);
      if (state.event_log.size() > 6)
        state.event_log.erase(state.event_log.begin());
      }
  }

  // There is intentionally no client cancellation prediction.  If a pending
  // action fizzles or an event stream is truncated, the simulation-provided
  // windup duration is the upper bound for the warning's lifetime.
  const std::uint64_t now = sim.tick();
  for (auto it = state.telegraphs.begin(); it != state.telegraphs.end();) {
    const auto* actor = sim.actor(it->first);
    const ActiveTelegraph& telegraph = it->second;
    const bool elapsed = now >= telegraph.start_tick +
                                   static_cast<std::uint64_t>(telegraph.windup_ticks);
    if (!actor || !actor->alive || elapsed)
      it = state.telegraphs.erase(it);
    else
      ++it;
  }
}

struct DepthDraw {
  double depth = 0.0;
  int order = 0;
  enum class What { Scenery, Player, Monster, Loot, Effect } what = What::Player;
  std::size_t index = 0;
};

// TASK-0142: eight-way compass for the objective strip; world y grows
// southward exactly like the ground projection.
const char* compass_step(int dx, int dy) {
  if (dx == 0 && dy == 0) return "here";
  static const char* const kNames[8] = {"E",  "SE", "S", "SW",
                                        "W",  "NW", "N", "NE"};
  const double angle = std::atan2(static_cast<double>(dy),
                                  static_cast<double>(dx));
  const int octant =
      static_cast<int>(std::lround(angle / (kPi / 4.0)));
  return kNames[((octant % 8) + 8) % 8];
}

// Draws one owner-facing status chip and records it as a Hud op. Returns the
// chip width so callers can lay out stacked chips deterministically.
int paint_status_chip(HDC dc, int x, int y, const std::string& text,
                      COLORREF accent, render::List& rl) {
  SIZE extent{};
  GetTextExtentPoint32A(dc, text.c_str(), static_cast<int>(text.size()), &extent);
  const int width = extent.cx + 16;
  const int height = extent.cy + 8;
  RECT rect{x, y, x + width, y + height};
  HBRUSH bg = CreateSolidBrush(RGB(25, 33, 37));
  FillRect(dc, &rect, bg);
  DeleteObject(bg);
  HPEN pen = CreatePen(PS_SOLID, 1, accent);
  HGDIOBJ old_pen = SelectObject(dc, pen);
  HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
  Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);
  SelectObject(dc, old_brush);
  SelectObject(dc, old_pen);
  DeleteObject(pen);
  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, accent);
  TextOutA(dc, x + 8, y + 4, text.c_str(), static_cast<int>(text.size()));
  rl.push_back({render::Op::Hud, static_cast<double>(x), static_cast<double>(y),
                0.0, 0, text});
  return width;
}

std::string loot_label(const ClientState& state, const std::string& id) {
  auto found = state.world.loot_names.find(id);
  if (found != state.world.loot_names.end()) return found->second;
  return id;
}

void paint_gear_overlay(const ClientState& state, HDC dc, const RECT& bounds,
                        render::List& rl) {
  if (!state.gear_overlay) return;
  const int panel_w = 380;
  const int left = std::max(24, static_cast<int>(bounds.right) - panel_w - 24);
  const int top = 64;
  const int right = left + panel_w;
  const int bottom = std::min(static_cast<int>(bounds.bottom) - 28, top + 430);

  HBRUSH panel = CreateSolidBrush(RGB(25, 33, 37));
  RECT panel_rect{left, top, right, bottom};
  FillRect(dc, &panel_rect, panel);
  DeleteObject(panel);
  HPEN border = CreatePen(PS_SOLID, 2, RGB(104, 160, 137));
  HGDIOBJ old_pen = SelectObject(dc, border);
  HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
  Rectangle(dc, left, top, right, bottom);
  SelectObject(dc, old_brush);
  SelectObject(dc, old_pen);
  DeleteObject(border);

  SetBkMode(dc, TRANSPARENT);

  // Title.
  SetTextColor(dc, RGB(230, 235, 220));
  // house().name is already prefixed ("House Verdigris"); do not double it.
  const std::string title = "Gear / " + state.world.house_name;
  TextOutA(dc, left + 14, top + 12, title.c_str(), static_cast<int>(title.size()));

  // Authoritative stats readout. The base attack is the actor's stat; the
  // equipped item's attack bonus (authoritative item data) is added on top,
  // matching how the core folds it into damage resolution.
  const WorldActor& player = state.world.player;
  const auto& items = state.world.carried;
  int equipped_bonus = 0;
  for (const auto& item : items)
    if (item.equipped) {
      equipped_bonus = item.attack_bonus;
      break;
    }
  const int base_attack = player.attack;
  std::string attack_text = std::to_string(base_attack + equipped_bonus);
  if (equipped_bonus != 0)
    attack_text += " (+" + std::to_string(equipped_bonus) + ")";
  SetTextColor(dc, RGB(150, 170, 158));
  std::string stats_line =
      "LIFE " + std::to_string(player.life) + "/" +
      std::to_string(player.life_max) + "  RES " +
      std::to_string(player.resource) + "/" +
      std::to_string(player.resource_max) + "  ATK " +
      attack_text + "  DEF " +
      std::to_string(player.defense) + "  LVL " +
      std::to_string(player.level);
  rl.push_back({render::Op::PaneStat, 0.0, 0.0, 0.0, 0, stats_line});
  TextOutA(dc, left + 14, top + 36, stats_line.c_str(),
           static_cast<int>(stats_line.size()));

  // Weapon (paperdoll) seat.
  const int seat_top = top + 58;
  const int seat_left = left + 14;
  const int seat_w = right - left - 28;
  RECT seat{seat_left, seat_top, seat_left + seat_w, seat_top + 24};
  HBRUSH seat_bg = CreateSolidBrush(RGB(32, 40, 42));
  FillRect(dc, &seat, seat_bg);
  DeleteObject(seat_bg);
  HPEN seat_pen = CreatePen(PS_SOLID, 1, RGB(104, 160, 137));
  HGDIOBJ sp = SelectObject(dc, seat_pen);
  Rectangle(dc, seat.left, seat.top, seat.right, seat.bottom);
  SelectObject(dc, sp);
  DeleteObject(seat_pen);
  SetTextColor(dc, RGB(170, 190, 178));
  const char* seat_label = "Weapon";
  TextOutA(dc, seat_left + 6, seat_top + 5, seat_label, static_cast<int>(strlen(seat_label)));
  std::string equipped_name = "(empty)";
  for (const auto& item : items)
    if (item.equipped) {
      equipped_name = item.name;
      break;
    }
  SetTextColor(dc, RGB(230, 220, 180));
  rl.push_back({render::Op::PaneWeapon, 0.0, 0.0, 0.0, 0, equipped_name});
  TextOutA(dc, seat_left + 96, seat_top + 5, equipped_name.c_str(),
           static_cast<int>(equipped_name.size()));

  // Grid backpack (4 columns).
  constexpr int kGridColumns = 4;
  const int cell_w = (right - left - 28 - (kGridColumns - 1) * 6) / kGridColumns;
  const int cell_h = 40;
  const int grid_top = seat_top + 34;
  if (items.empty()) {
    SetTextColor(dc, RGB(150, 160, 150));
    const char* empty = "Backpack empty. X picks up the nearest drop.";
    TextOutA(dc, left + 14, grid_top + 6, empty, static_cast<int>(strlen(empty)));
  } else {
    for (std::size_t i = 0; i < items.size(); ++i) {
      const int col = static_cast<int>(i % kGridColumns);
      const int row = static_cast<int>(i / kGridColumns);
      const int cx = left + 14 + col * (cell_w + 6);
      const int cy = grid_top + row * (cell_h + 6);
      const bool selected = i == std::min(state.selected_item, items.size() - 1);
      const bool equipped = items[i].equipped;
      RECT cell{cx, cy, cx + cell_w, cy + cell_h};
      HBRUSH cell_bg = CreateSolidBrush(selected ? RGB(52, 74, 66) : RGB(30, 38, 40));
      FillRect(dc, &cell, cell_bg);
      DeleteObject(cell_bg);
      HPEN cell_pen = CreatePen(
          PS_SOLID, 1,
          equipped ? RGB(210, 180, 90) : (selected ? RGB(104, 160, 137) : RGB(56, 66, 64)));
      HGDIOBJ cp = SelectObject(dc, cell_pen);
      Rectangle(dc, cell.left, cell.top, cell.right, cell.bottom);
      SelectObject(dc, cp);
      DeleteObject(cell_pen);
      SetTextColor(dc, equipped ? RGB(240, 210, 120) : RGB(205, 215, 204));
      std::string name = items[i].name;
      if (name.size() > 12) name = name.substr(0, 11) + ".";
      rl.push_back({render::Op::PaneItem, static_cast<double>(cx),
                    static_cast<double>(cy), 0.0, items[i].attack_bonus,
                    equipped ? name + " [E]" : name});
      TextOutA(dc, cx + 4, cy + 4, name.c_str(), static_cast<int>(name.size()));
      SetTextColor(dc, RGB(150, 165, 152));
      std::string bonus = "+" + std::to_string(items[i].attack_bonus) + " ATK" +
                          (equipped ? "  [E]" : "");
      TextOutA(dc, cx + 4, cy + 21, bonus.c_str(), static_cast<int>(bonus.size()));
    }
  }

  // Banked / extraction summary.
  SetTextColor(dc, RGB(150, 170, 158));
  const std::string banked =
      "Banked  items " + std::to_string(state.world.stored_items) +
      "  trophies " + std::to_string(state.world.stored_trophies);
  rl.push_back({render::Op::PaneBanked, 0.0, 0.0, 0.0, 0, banked});
  TextOutA(dc, left + 14, bottom - 46, banked.c_str(), static_cast<int>(banked.size()));
  const char* controls = "Arrows select | Enter equip | U unequip | I close";
  TextOutA(dc, left + 14, bottom - 24, controls, static_cast<int>(strlen(controls)));
}

void draw_orb(HDC dc, int cx, int cy, int radius, double ratio, COLORREF fill,
              COLORREF rim, const std::string& caption, bool pulse, render::List& rl,
              const char* label) {
  const int bounded = static_cast<int>(std::clamp(ratio, 0.0, 1.0) * 100.0);
  rl.push_back({render::Op::Orb, static_cast<double>(cx), static_cast<double>(cy),
                static_cast<double>(radius), bounded, label});
  fill_ellipse(dc, cx, cy, radius, radius, RGB(14, 18, 20));
  if (ratio > 0.0) {
    HRGN outer = CreateEllipticRgn(cx - radius, cy - radius, cx + radius, cy + radius);
    const int fill_top =
        cy + radius - static_cast<int>(static_cast<double>(radius) * 2.0 * ratio);
    HRGN fill_rgn = CreateRectRgn(cx - radius, fill_top, cx + radius, cy + radius);
    HRGN clip = CreateRectRgn(0, 0, 0, 0);
    CombineRgn(clip, outer, fill_rgn, RGN_AND);
    SelectClipRgn(dc, clip);
    fill_ellipse(dc, cx, cy, radius - 2, radius - 2, fill);
    SelectClipRgn(dc, nullptr);
    DeleteObject(clip);
    DeleteObject(fill_rgn);
    DeleteObject(outer);
  }
  const int pulse_r = pulse ? radius + 3 : radius;
  ring_ellipse(dc, cx, cy, pulse_r, pulse_r, pulse ? RGB(220, 72, 58) : rim, pulse ? 3 : 2);
  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, RGB(232, 223, 202));
  const int text_x = cx - static_cast<int>(caption.size()) * 3;
  TextOutA(dc, text_x, cy - 6, caption.c_str(), static_cast<int>(caption.size()));
}

void paint_vital_orbs(const WorldActor& player, std::uint64_t tick, int screen_pulse_ticks,
                      HDC dc, const RECT& bounds, render::List& rl) {
  if (!player.alive && player.life <= 0 && player.life_max <= 0) return;
  constexpr int radius = 34;
  const int bottom = static_cast<int>(bounds.bottom) - 18;
  const int left_cx = 18 + radius;
  const int right_cx = static_cast<int>(bounds.right) - 18 - radius;
  const int cy = bottom - radius;

  const int life_max = std::max(1, player.life_max);
  const int resource_max = std::max(1, player.resource_max);
  const double life_ratio =
      std::clamp(static_cast<double>(player.life) / life_max, 0.0, 1.0);
  const double resource_ratio =
      std::clamp(static_cast<double>(player.resource) / resource_max, 0.0, 1.0);
  const bool low_life = player.life * 4 < life_max;
  const bool pulse = low_life && (screen_pulse_ticks > 0 || (tick % 24) < 12);

  const std::string life_caption =
      std::to_string(player.life) + "/" + std::to_string(player.life_max);
  const std::string resource_caption =
      std::to_string(player.resource) + "/" + std::to_string(player.resource_max);
  draw_orb(dc, left_cx, cy, radius, life_ratio, RGB(177, 72, 62), RGB(214, 128, 96),
           life_caption, pulse, rl, "life");
  draw_orb(dc, right_cx, cy, radius, resource_ratio, RGB(58, 138, 168), RGB(120, 188, 214),
           resource_caption, false, rl, "resource");
}

struct QuickbarSlotDef {
  const char* key_label;
  const char* name;
  verdigris::ActionType action;
};

constexpr QuickbarSlotDef kQuickbarSlots[] = {
    {"LMB", "Strike", verdigris::ActionType::Melee},
    {"Q", "Thrust", verdigris::ActionType::Thrust},
    {"E", "Sweep", verdigris::ActionType::Sweep},
    {"R", "WarCry", verdigris::ActionType::WarCry},
};

void paint_quickbar(const ClientState& state, HDC dc, const RECT& bounds, render::List& rl) {
  const WorldActor& player = state.world.player;
  const verdigris::PresentationCatalog catalog =
      verdigris::Simulation::presentation_catalog();
  constexpr int slot_w = 58;
  constexpr int slot_h = 52;
  constexpr int gap = 8;
  const int count = static_cast<int>(sizeof(kQuickbarSlots) / sizeof(kQuickbarSlots[0]));
  const int strip_w = count * slot_w + (count - 1) * gap;
  const int bottom = static_cast<int>(bounds.bottom) - 18;
  const int top = bottom - slot_h;
  const int left = (static_cast<int>(bounds.right) - strip_w) / 2;

  HBRUSH strip_bg = CreateSolidBrush(RGB(18, 24, 26));
  RECT strip{left - 10, top - 8, left + strip_w + 10, bottom + 4};
  FillRect(dc, &strip, strip_bg);
  DeleteObject(strip_bg);
  HPEN strip_pen = CreatePen(PS_SOLID, 1, RGB(86, 116, 104));
  HGDIOBJ old_strip_pen = SelectObject(dc, strip_pen);
  HGDIOBJ old_strip_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
  Rectangle(dc, strip.left, strip.top, strip.right, strip.bottom);
  SelectObject(dc, old_strip_brush);
  SelectObject(dc, old_strip_pen);
  DeleteObject(strip_pen);

  for (int i = 0; i < count; ++i) {
    const QuickbarSlotDef& slot = kQuickbarSlots[i];
    const int slot_left = left + i * (slot_w + gap);
    const int cx = slot_left + slot_w / 2;
    const int cy = top + slot_h / 2;
    const int resource_cost = skill_resource_cost(catalog, slot.action);
    const bool cooldown =
        slot.action != verdigris::ActionType::WarCry && player.cooldown_ticks > 0;
    const bool affordable = player.resource >= resource_cost;
    const bool available = player.alive && affordable && !cooldown;
    const bool active =
        slot.action == verdigris::ActionType::WarCry && player.war_cry_ticks_remaining > 0;

    RECT box{slot_left, top, slot_left + slot_w, bottom};
    HBRUSH fill = CreateSolidBrush(available ? RGB(35, 42, 44) : RGB(29, 33, 34));
    FillRect(dc, &box, fill);
    DeleteObject(fill);
    HPEN border = CreatePen(PS_SOLID, 1,
                            available ? RGB(120, 164, 142) : RGB(63, 70, 68));
    HGDIOBJ old_pen = SelectObject(dc, border);
    HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(dc, box.left, box.top, box.right, box.bottom);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(border);

    if (cooldown && player.cooldown_ticks > 0) {
      const int max_ticks = 30;
      const double sweep =
          std::clamp(static_cast<double>(player.cooldown_ticks) / max_ticks, 0.0, 1.0);
      const int overlay_h = static_cast<int>(slot_h * sweep);
      RECT overlay{box.left, box.top, box.right, box.top + overlay_h};
      HBRUSH overlay_brush = CreateSolidBrush(RGB(10, 12, 14));
      FillRect(dc, &overlay, overlay_brush);
      DeleteObject(overlay_brush);
    }

    rl.push_back({render::Op::Quickbar, static_cast<double>(cx), static_cast<double>(cy),
                  static_cast<double>(slot_w), active ? 1 : 0,
                  std::string(slot.key_label) + ":" + slot.name});

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(239, 208, 116));
    TextOutA(dc, box.left + 6, box.top + 4, slot.key_label,
             static_cast<int>(strlen(slot.key_label)));
    SetTextColor(dc, available ? RGB(205, 221, 207) : RGB(112, 119, 115));
    TextOutA(dc, box.left + 6, box.top + 22, slot.name, static_cast<int>(strlen(slot.name)));
  }
}

void paint_minimap(const ClientState& state, HDC dc, const RECT& bounds, render::List& rl) {
  constexpr int kSize = 108;
  constexpr int margin = 12;
  RECT panel{margin, margin, margin + kSize, margin + kSize};
  HBRUSH panel_bg = CreateSolidBrush(RGB(16, 22, 24));
  FillRect(dc, &panel, panel_bg);
  DeleteObject(panel_bg);
  HPEN panel_pen = CreatePen(PS_SOLID, 1, RGB(86, 116, 104));
  HGDIOBJ old_pen = SelectObject(dc, panel_pen);
  HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
  Rectangle(dc, panel.left, panel.top, panel.right, panel.bottom);
  SelectObject(dc, old_brush);
  SelectObject(dc, old_pen);
  DeleteObject(panel_pen);

  const WorldView& world = state.world;
  const double arena = static_cast<double>(verdigris::world_scale::kArenaHalfExtent);
  const double map_scale = static_cast<double>(kSize) / (arena * 2.2);
  const int center_x = (panel.left + panel.right) / 2;
  const int center_y = (panel.top + panel.bottom) / 2;
  const double origin_x = static_cast<double>(world.player.position.x);
  const double origin_y = static_cast<double>(world.player.position.y);

  auto to_map = [&](double wx, double wy) {
    const int mx = center_x + static_cast<int>((wx - origin_x) * map_scale);
    const int my = center_y + static_cast<int>((wy - origin_y) * map_scale);
    return std::pair<int, int>{mx, my};
  };

  int dots = 0;
  for (const auto& item : state.scenery) {
    const auto [mx, my] = to_map(item.position.x, item.position.y);
    if (mx < panel.left + 2 || mx >= panel.right - 2 || my < panel.top + 2 ||
        my >= panel.bottom - 2)
      continue;
    fill_ellipse(dc, mx, my, 2, 2, RGB(96, 112, 98));
    ++dots;
  }

  if (world.has_extraction) {
    const auto [mx, my] = to_map(world.extraction.x, world.extraction.y);
    fill_ellipse(dc, mx, my, 4, 4, RGB(239, 208, 116));
    ++dots;
  }

  for (const auto& monster : world.monsters) {
    if (!monster.alive) continue;
    const auto [mx, my] = to_map(monster.position.x, monster.position.y);
    fill_ellipse(dc, mx, my, 3, 3, RGB(196, 58, 48));
    ++dots;
  }

  const auto [px, py] = to_map(origin_x, origin_y);
  const double facing_angle =
      std::atan2(world.player.facing.y, world.player.facing.x);
  const int tip_x = px + static_cast<int>(std::cos(facing_angle) * 8.0);
  const int tip_y = py + static_cast<int>(std::sin(facing_angle) * 8.0);
  const int wing_x = px - static_cast<int>(std::cos(facing_angle) * 4.0);
  const int wing_y = py - static_cast<int>(std::sin(facing_angle) * 4.0);
  const double wing = facing_angle + kPi * 0.75;
  const int wing_a_x = wing_x + static_cast<int>(std::cos(wing) * 5.0);
  const int wing_a_y = wing_y + static_cast<int>(std::sin(wing) * 5.0);
  const int wing_b_x = wing_x + static_cast<int>(std::cos(wing + kPi * 0.5) * 5.0);
  const int wing_b_y = wing_y + static_cast<int>(std::sin(wing + kPi * 0.5) * 5.0);
  POINT arrow[3] = {{tip_x, tip_y}, {wing_a_x, wing_a_y}, {wing_b_x, wing_b_y}};
  HBRUSH player_brush = CreateSolidBrush(RGB(168, 214, 188));
  HGDIOBJ old_arrow_brush = SelectObject(dc, player_brush);
  Polygon(dc, arrow, 3);
  SelectObject(dc, old_arrow_brush);
  DeleteObject(player_brush);

  rl.push_back({render::Op::Minimap, static_cast<double>(panel.left),
                static_cast<double>(panel.top), static_cast<double>(kSize), dots, "panel"});
}

void paint_scene(ClientState& state, HDC dc, const RECT& bounds) {
  sync_world(state);
  const WorldView& world = state.world;
  render::List rl;

  draw_floor(state.billboards, dc, state.camera, bounds, world.route_id, rl);

  // Ground decals render before anything that stands on the plane.
  if (world.has_extraction) {
    const ScreenPoint pad =
        project(state.camera, bounds, world.extraction.x, world.extraction.y);
    const int pad_r = static_cast<int>(kTileUnits * 0.9 * pad.scale);
    rl.push_back({render::Op::Extraction, static_cast<double>(pad.x),
                  static_cast<double>(pad.y), static_cast<double>(pad_r), 0,
                  "stairs-up"});
    // TASK-0142: the pad must own its corner of the screen — a bright plate,
    // a slow tick-driven pulse, and gold chevrons pointing at the way out.
    const bool pulse_on = (world.tick / 9) % 2 == 0;
    fill_ellipse(dc, pad.x, pad.y, pad_r, pad_r, RGB(30, 92, 64));
    ring_ellipse(dc, pad.x, pad.y, pad_r, pad_r, RGB(120, 214, 168), 3);
    if (pulse_on && pad_r > 6)
      ring_ellipse(dc, pad.x, pad.y, pad_r + 5, pad_r + 5, RGB(160, 236, 190), 2);
    const int inner = std::max(6, pad_r * 2 / 3);
    ring_ellipse(dc, pad.x, pad.y, inner, inner, RGB(239, 208, 116), 2);
    const int step = std::max(5, pad_r / 3);
    for (int i = 0; i < 3; ++i) {
      const int y = pad.y + pad_r / 4 - i * step;
      const int spread = std::max(3, pad_r / 2 - i * 3);
      const int tip_y = y - step;
      draw_line(dc, pad.x - spread, y, pad.x, tip_y, RGB(239, 208, 116), 3);
      draw_line(dc, pad.x + spread, y, pad.x, tip_y, RGB(239, 208, 116), 3);
      // Arrowheads make the chevron read as direction, not decoration.
      draw_line(dc, pad.x - spread / 2, tip_y + std::max(2, step / 4), pad.x,
                tip_y, RGB(255, 232, 150), 2);
      draw_line(dc, pad.x + spread / 2, tip_y + std::max(2, step / 4), pad.x,
                tip_y, RGB(255, 232, 150), 2);
    }
    {
      RECT label_backing{pad.x - 22, pad.y + pad_r + 2, pad.x + 22,
                         pad.y + pad_r + 18};
      HBRUSH label_bg = CreateSolidBrush(RGB(16, 22, 20));
      FillRect(dc, &label_backing, label_bg);
      DeleteObject(label_bg);
      HPEN label_pen = CreatePen(PS_SOLID, 1, RGB(120, 214, 168));
      HGDIOBJ old_label_pen = SelectObject(dc, label_pen);
      HGDIOBJ old_label_brush =
          SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
      Rectangle(dc, label_backing.left, label_backing.top, label_backing.right,
                label_backing.bottom);
      SelectObject(dc, old_label_brush);
      SelectObject(dc, old_label_pen);
      DeleteObject(label_pen);
      SetBkMode(dc, TRANSPARENT);
      SetTextColor(dc, RGB(239, 208, 116));
      TextOutA(dc, pad.x - 14, pad.y + pad_r + 4, "EXIT", 4);
    }
  }

  // Warnings live on the ground plane beneath billboards and loot so their
  // footprint remains readable without obscuring the actor that owns them.
  paint_telegraphs(state, dc, bounds, rl);

  const WorldActor& player = world.player;

  // Collect every standing element, then draw back-to-front by the top-down
  // painter's key (world y first, then world x) so lower entities render in
  // front. camera2d::draw_order_key is translation-invariant like the rest of
  // the camera math.
  std::vector<DepthDraw> order;
  for (std::size_t i = 0; i < state.scenery.size(); ++i)
    order.push_back({camera2d::draw_order_key(
                         static_cast<double>(state.scenery[i].position.y),
                         static_cast<double>(state.scenery[i].position.x)),
                     0, DepthDraw::What::Scenery, i});
  if (player.alive)
    order.push_back({camera2d::draw_order_key(static_cast<double>(player.position.y),
                                              static_cast<double>(player.position.x)),
                     1, DepthDraw::What::Player, 0});
  for (std::size_t i = 0; i < world.monsters.size(); ++i) {
    if (world.monsters[i].alive)
      order.push_back({camera2d::draw_order_key(
                           static_cast<double>(world.monsters[i].position.y),
                           static_cast<double>(world.monsters[i].position.x)),
                       2, DepthDraw::What::Monster, i});
  }
  std::vector<std::pair<std::string, verdigris::Vec2>> loot(
      state.loot_positions.begin(), state.loot_positions.end());
  std::sort(loot.begin(), loot.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
  for (std::size_t i = 0; i < loot.size(); ++i)
    order.push_back({camera2d::draw_order_key(static_cast<double>(loot[i].second.y),
                                              static_cast<double>(loot[i].second.x)),
                     3, DepthDraw::What::Loot, i});
  for (std::size_t i = 0; i < state.effects.size(); ++i)
    order.push_back({camera2d::draw_order_key(state.effects[i].wy + 1.0,
                                              state.effects[i].wx),
                     4, DepthDraw::What::Effect, i});
  std::sort(order.begin(), order.end(), [](const DepthDraw& lhs, const DepthDraw& rhs) {
    if (lhs.depth != rhs.depth) return lhs.depth < rhs.depth;
    return lhs.order < rhs.order;
  });

  for (const auto& entry : order) {
    switch (entry.what) {
      case DepthDraw::What::Scenery:
        draw_scenery_item(state.billboards, dc, state.camera, bounds,
                          state.scenery[entry.index], rl);
        break;
      case DepthDraw::What::Player: {
        const ScreenPoint base =
            project(state.camera, bounds, player.position.x, player.position.y);
        rl.push_back({render::Op::Player, static_cast<double>(base.x),
                      static_cast<double>(base.y)});
        draw_contact_shadow(dc, base, kTileUnits * 0.42);
        draw_team_ring(dc, base, kTileUnits * 0.55, RGB(120, 214, 168));
        if (!draw_billboard_sprite(state.billboards, dc, state.billboards.player, base,
                                   kTileUnits * 1.35, player.facing.x)) {
          // TASK-0142: generated vector silhouette before the capsule.
          const int kit_height =
              std::max(8, static_cast<int>(kTileUnits * 1.35 * base.scale));
          const kit::Symbol* symbol = kit_symbol("player");
          if (symbol)
            draw_kit_symbol(dc, *symbol, base.x, base.y, kit_height,
                            player.facing.x < 0);
          else
            draw_billboard(dc, base, kTileUnits * 0.62, kTileUnits * 1.35,
                           RGB(84, 158, 128), RGB(140, 208, 172));
        }
        // Draw the authoritative facing, rather than a client-only mouse hint.
        const double angle =
            std::atan2(static_cast<double>(player.facing.y),
                       static_cast<double>(player.facing.x));
        const int fx = base.x + static_cast<int>(std::cos(angle) * kTileUnits * 0.6 *
                                                 base.scale);
        const int fy = base.y + static_cast<int>(std::sin(angle) * kTileUnits * 0.6 *
                                                 base.scale);
        draw_line(dc, base.x, base.y, fx, fy, RGB(140, 208, 172), 2);
        break;
      }
      case DepthDraw::What::Monster: {
        const auto& monster = world.monsters[entry.index];
        const ScreenPoint base =
            project(state.camera, bounds, monster.position.x, monster.position.y);
        rl.push_back({render::Op::Monster, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, monster.life,
                      monster.elite ? "elite" : "monster"});
        draw_contact_shadow(dc, base, kTileUnits * 0.42);
        draw_team_ring(dc, base, kTileUnits * 0.58,
                       monster.elite ? RGB(239, 208, 116) : RGB(214, 92, 72));
        const SpriteBitmap& monster_sprite = monster.elite ? state.billboards.boss
                                                            : state.billboards.raider;
        const double foe_height =
            monster.elite ? kTileUnits * 1.85 : kTileUnits * 1.58;
        {
          const int halo_h = std::max(6, static_cast<int>(foe_height * base.scale));
          const int halo_w = std::max(5, static_cast<int>(halo_h * 0.42));
          fill_ellipse(dc, base.x, base.y - halo_h / 3, halo_w, halo_h / 2,
                       RGB(72, 22, 20));
        }
        if (!draw_billboard_sprite(state.billboards, dc, monster_sprite, base, foe_height,
                                   monster.facing.x)) {
          // TASK-0142: generated vector silhouettes (horned raider / caped
          // elite) before the capsule.
          const int kit_height =
              std::max(8, static_cast<int>(foe_height * base.scale));
          const kit::Symbol* symbol =
              kit_symbol(monster.elite ? "elite" : "raider");
          if (symbol)
            draw_kit_symbol(dc, *symbol, base.x, base.y, kit_height,
                            monster.facing.x < 0);
          else
            draw_billboard(dc, base, kTileUnits * 0.88, kTileUnits * 1.12,
                           RGB(186, 58, 44), RGB(42, 18, 16));
        }
        // TASK-0142: bordered life bar with a dark backing so the remaining
        // fraction stays readable against any floor.
        const int bar_w = static_cast<int>(kTileUnits * 0.7 * base.scale) + 4;
        const int bar_y =
            base.y - static_cast<int>(kTileUnits * 1.5 * base.scale) - 4;
        const double ratio =
            std::clamp(static_cast<double>(monster.life) /
                           std::max(1, monster.life_max),
                       0.0, 1.0);
        {
          RECT backing{base.x - bar_w / 2 - 1, bar_y - 3, base.x + bar_w / 2 + 1,
                       bar_y + 3};
          HBRUSH backing_brush = CreateSolidBrush(RGB(12, 14, 15));
          FillRect(dc, &backing, backing_brush);
          DeleteObject(backing_brush);
          if (ratio > 0.0) {
            const int fill_w = static_cast<int>(bar_w * ratio);
            RECT fill_rect{base.x - bar_w / 2, bar_y - 2,
                           base.x - bar_w / 2 + fill_w, bar_y + 2};
            const COLORREF bar_color = ratio > 0.55   ? RGB(120, 200, 130)
                                       : ratio > 0.25 ? RGB(239, 208, 116)
                                                      : RGB(214, 72, 58);
            HBRUSH fill_brush = CreateSolidBrush(bar_color);
            FillRect(dc, &fill_rect, fill_brush);
            DeleteObject(fill_brush);
          }
          HPEN bar_pen = CreatePen(PS_SOLID, 1, RGB(86, 116, 104));
          HGDIOBJ old_bar_pen = SelectObject(dc, bar_pen);
          HGDIOBJ old_bar_brush =
              SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
          Rectangle(dc, backing.left, backing.top, backing.right, backing.bottom);
          SelectObject(dc, old_bar_brush);
          SelectObject(dc, old_bar_pen);
          DeleteObject(bar_pen);
        }
        const auto telegraph = state.telegraphs.find(monster.id);
        if (telegraph != state.telegraphs.end()) {
          const ActiveTelegraph& warning = telegraph->second;
          SetBkMode(dc, TRANSPARENT);
          SetTextColor(dc, telegraph_color(telegraph_visibility(state, warning),
                                           RGB(255, 104, 86)));
          std::string pending = "! " + warning.action;
          std::transform(pending.begin(), pending.end(), pending.begin(),
                         [](unsigned char character) {
                           return static_cast<char>(std::toupper(character));
                         });
          TextOutA(dc, base.x - bar_w / 2, bar_y - 15, pending.c_str(),
                   static_cast<int>(pending.size()));
        }
        break;
      }
      case DepthDraw::What::Loot: {
        const auto& entry_loot = loot[entry.index];
        const ScreenPoint base = project(state.camera, bounds, entry_loot.second.x,
                                         entry_loot.second.y);
        rl.push_back({render::Op::Drop, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, 0, entry_loot.first});
        draw_contact_shadow(dc, base, kTileUnits * 0.2);
        const int r = std::max(3, static_cast<int>(kTileUnits * 0.16 * base.scale));
        const int lift = static_cast<int>(kTileUnits * 0.28 * base.scale);
        const bool is_trophy = entry_loot.first.rfind("trophy", 0) == 0;
        const COLORREF color =
            is_trophy ? RGB(196, 148, 220) : RGB(230, 181, 74);
        POINT diamond[4] = {{base.x, base.y - lift - r},
                            {base.x + r, base.y - lift},
                            {base.x, base.y - lift + r},
                            {base.x - r, base.y - lift}};
        HBRUSH brush = CreateSolidBrush(color);
        HPEN pen = CreatePen(PS_SOLID, 1, color);
        HGDIOBJ old_brush = SelectObject(dc, brush);
        HGDIOBJ old_pen = SelectObject(dc, pen);
        Polygon(dc, diamond, 4);
        SelectObject(dc, old_brush);
        SelectObject(dc, old_pen);
        DeleteObject(brush);
        DeleteObject(pen);
        if (state.loot_labels) {
          const std::string label = loot_label(state, entry_loot.first);
          SetBkMode(dc, TRANSPARENT);
          SetTextColor(dc, color);
          TextOutA(dc, base.x + r + 4, base.y - lift - r - 5, label.c_str(),
                   static_cast<int>(label.size()));
        }
        break;
      }
      case DepthDraw::What::Effect:
        draw_effect(dc, state.camera, bounds, state.effects[entry.index], rl);
        break;
    }
  }

  paint_minimap(state, dc, bounds, rl);
  paint_vital_orbs(player, world.tick, state.screen_pulse_ticks, dc, bounds, rl);
  paint_quickbar(state, dc, bounds, rl);
  paint_gear_overlay(state, dc, bounds, rl);

  if (state.screen_pulse_ticks > 0) {
    rl.push_back({render::Op::ScreenPulse, 0.0, 0.0, 0.0, 0, "player-damage"});
    HPEN pulse = CreatePen(PS_SOLID, 10, RGB(196, 46, 40));
    HGDIOBJ old_pen = SelectObject(dc, pulse);
    HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    RECT inner{4, 4, bounds.right - 4, bounds.bottom - 4};
    Rectangle(dc, inner.left, inner.top, inner.right, inner.bottom);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(pulse);
  }

  if (state.session) {
    const auto conn = state.session->connection_state();
    const char* label = verdigris::client::connection_state_label(conn);
    const std::string chip = std::string("connection ") + label;
    rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0, 0, chip});
    COLORREF chip_color = RGB(185, 198, 188);
    if (conn == verdigris::client::ConnectionState::Ready)
      chip_color = RGB(120, 214, 168);
    else if (conn == verdigris::client::ConnectionState::Connecting ||
             conn == verdigris::client::ConnectionState::Connected ||
             conn == verdigris::client::ConnectionState::Retrying)
      chip_color = RGB(239, 208, 116);
    else if (conn == verdigris::client::ConnectionState::Disconnected ||
             conn == verdigris::client::ConnectionState::Rejected ||
             conn == verdigris::client::ConnectionState::ProtocolMismatch)
      chip_color = RGB(255, 80, 70);
    const int chip_w = 168;
    const int chip_h = 22;
    const int chip_x = std::max(18, static_cast<int>(bounds.right) - chip_w - 18);
    const int chip_y = 12;
    RECT chip_rect{chip_x, chip_y, chip_x + chip_w, chip_y + chip_h};
    HBRUSH chip_bg = CreateSolidBrush(RGB(25, 33, 37));
    FillRect(dc, &chip_rect, chip_bg);
    DeleteObject(chip_bg);
    HPEN chip_pen = CreatePen(PS_SOLID, 1, chip_color);
    HGDIOBJ old_chip_pen = SelectObject(dc, chip_pen);
    HGDIOBJ old_chip_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(dc, chip_rect.left, chip_rect.top, chip_rect.right, chip_rect.bottom);
    SelectObject(dc, old_chip_brush);
    SelectObject(dc, old_chip_pen);
    DeleteObject(chip_pen);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, chip_color);
    TextOutA(dc, chip_x + 8, chip_y + 3, chip.c_str(), static_cast<int>(chip.size()));
    if (conn == verdigris::client::ConnectionState::Disconnected ||
        conn == verdigris::client::ConnectionState::Rejected ||
        conn == verdigris::client::ConnectionState::ProtocolMismatch) {
      SetTextColor(dc, RGB(255, 80, 70));
      const char* banner = "CONNECTION LOST — not playing offline";
      TextOutA(dc, 18, 76, banner, static_cast<int>(strlen(banner)));
    }
  }

  // TASK-0142: owner-facing objective strip, centered at the top. The copy
  // always reflects the authoritative world (extraction presence, carried
  // loot) and names the concrete action that banks progress.
  {
    std::string objective;
    COLORREF accent = RGB(120, 214, 168);
    if (!world.has_extraction) {
      objective = "objective: explore the route";
    } else {
      const int ddx = world.extraction.x - player.position.x;
      const int ddy = world.extraction.y - player.position.y;
      const int dist = static_cast<int>(
          std::lround(std::sqrt(static_cast<double>(ddx * ddx + ddy * ddy))));
      const bool carrying =
          !world.carried.empty() || world.carried_trophies > 0;
      objective = std::string(carrying
                                  ? "objective: carry your loot to the EXIT ("
                                  : "objective: reach the EXIT (");
      objective += compass_step(ddx, ddy);
      objective += ", " + std::to_string(dist) + "u) - press F there";
      if (carrying) accent = RGB(239, 208, 116);
    }
    SIZE extent{};
    GetTextExtentPoint32A(dc, objective.c_str(),
                          static_cast<int>(objective.size()), &extent);
    const int chip_x =
        std::max(12, (static_cast<int>(bounds.right) -
                      static_cast<int>(extent.cx) - 16) / 2);
    paint_status_chip(dc, chip_x, 12, objective, accent, rl);
  }

  // TASK-0142: honest art-status chip. It reports what is really on screen —
  // PNG plates when loaded, the embedded procedural kit otherwise — so a
  // missing asset can never masquerade as loaded art.
  {
    const bool plates =
        state.billboards.player.ready() && state.billboards.raider.ready() &&
        state.billboards.boss.ready();
    const COLORREF art_accent =
        plates ? RGB(120, 214, 168) : RGB(239, 190, 78);
    SIZE art_extent{};
    GetTextExtentPoint32A(dc, state.billboards.status.c_str(),
                          static_cast<int>(state.billboards.status.size()),
                          &art_extent);
    const int art_x =
        std::max(12, static_cast<int>(bounds.right) -
                         static_cast<int>(art_extent.cx) - 16 - 18);
    paint_status_chip(dc, art_x, state.session ? 38 : 12,
                      state.billboards.status, art_accent, rl);
  }

  state.render_list = std::move(rl);
  if (state.debug_overlay) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(230, 235, 220));
    const int life = player.life;
    const std::string status =
        "House " + world.house_name + " | Scion " + world.scion_name + " | Life " +
        std::to_string(life) + " | Resource " +
        std::to_string(player.resource) + " | Stored trophies " +
        std::to_string(world.stored_trophies) + " | Stored items " +
        std::to_string(world.stored_items) + " | Carried " +
        std::to_string(world.carried.size() + world.carried_trophies);
    TextOutA(dc, 18, 16, status.c_str(), static_cast<int>(status.size()));
    const char* help =
        "WASD move | Mouse aim | LMB melee | RMB/Space dash | Q Thrust | E Sweep | R WarCry";
    TextOutA(dc, 18, 72, help, static_cast<int>(strlen(help)));
    const char* help2 =
        "X nearest pickup | Z loot labels | F contextual extract | I gear/House overlay";
    TextOutA(dc, 18, 96, help2, static_cast<int>(strlen(help2)));
    const char* camera_help =
        "Wheel zoom | Home reset zoom";
    TextOutA(dc, 18, 120, camera_help, static_cast<int>(strlen(camera_help)));

    SetTextColor(dc, RGB(150, 160, 150));
    char debug_line[256];
    std::snprintf(debug_line, sizeof(debug_line),
                  "tick %llu | player %d,%d | zoom %.2f | effects %zu | telegraphs %zu",
                  static_cast<unsigned long long>(world.tick),
                  player.position.x, player.position.y,
                  state.camera.zoom,
                  state.effects.size(), state.telegraphs.size());
    TextOutA(dc, 18, 144, debug_line, static_cast<int>(strlen(debug_line)));
    char asset_line[256];
    std::snprintf(asset_line, sizeof(asset_line), "%s | %zu scenery | %s",
                  state.billboards.status.c_str(), state.scenery.size(),
                  state.billboards.scenery_status.c_str());
    TextOutA(dc, 18, 168, asset_line, static_cast<int>(strlen(asset_line)));
    if (state.hint_ticks > 0 && !state.hint.empty()) {
      SetTextColor(dc, RGB(239, 208, 116));
      TextOutA(dc, 18, 192, state.hint.c_str(), static_cast<int>(state.hint.size()));
    }
    int log_y = bounds.bottom - 24;
    for (auto it = state.event_log.rbegin(); it != state.event_log.rend(); ++it) {
      TextOutA(dc, 18, log_y, it->c_str(), static_cast<int>(it->size()));
      log_y -= 20;
    }
  }
}
  render::List rl;
constexpr double kActorColliderRadius =
    static_cast<double>(verdigris::world_scale::kActorColliderRadius);

bool scenery_blocks_segment(const ClientState& state, verdigris::Vec2 from,
                            verdigris::Vec2 to) {
  for (const SceneryItem& item : state.scenery) {
    if (!item.solid) continue;
    const double segment_x = static_cast<double>(to.x - from.x);
    const double segment_y = static_cast<double>(to.y - from.y);
    const double length_squared = segment_x * segment_x + segment_y * segment_y;
    const double to_center_x = static_cast<double>(item.position.x - from.x);
    const double to_center_y = static_cast<double>(item.position.y - from.y);
    const double projection = length_squared > 0.0
                                  ? std::clamp((to_center_x * segment_x +
                                                to_center_y * segment_y) /
                                                   length_squared,
                                               0.0, 1.0)
                                  : 0.0;
    const double closest_x = static_cast<double>(from.x) + segment_x * projection;
    const double closest_y = static_cast<double>(from.y) + segment_y * projection;
    const double dx = closest_x - static_cast<double>(item.position.x);
    const double dy = closest_y - static_cast<double>(item.position.y);
    const double minimum = item.radius + kActorColliderRadius;
    if (dx * dx + dy * dy < minimum * minimum) return true;
  }
  return false;
}

bool movement_hits_scenery(const ClientState& state, int dx, int dy,
                           int tick_multiplier = 1) {
  if (is_remote(state) || !state.simulation) return false;
  const auto* player = state.simulation->actor(state.simulation->scion().actor_id);
  if (!player || !player->alive) return false;
  const int step = verdigris::movement_step_per_tick(player->stats.move_speed) *
                   tick_multiplier;
  const int length = std::max(1, std::abs(dx) + std::abs(dy));
  const verdigris::Vec2 destination{
      player->position.x + (dx * step) / length,
      player->position.y + (dy * step) / length};
  return scenery_blocks_segment(state, player->position, destination);
}

void paint(HWND window, HDC dc) {
  ClientState* state = state_from(window);
  if (!state) return;
  RECT bounds;
  GetClientRect(window, &bounds);
  if (bounds.right <= 0 || bounds.bottom <= 0) return;

  // Double buffer: draw into a memory bitmap, then blit once.
  HDC memory_dc = CreateCompatibleDC(dc);
  HBITMAP bitmap = CreateCompatibleBitmap(dc, bounds.right, bounds.bottom);
  HGDIOBJ old_bitmap = SelectObject(memory_dc, bitmap);
  paint_scene(*state, memory_dc, bounds);
  BitBlt(dc, 0, 0, bounds.right, bounds.bottom, memory_dc, 0, 0, SRCCOPY);
  SelectObject(memory_dc, old_bitmap);
  DeleteObject(bitmap);
  DeleteDC(memory_dc);
}

void timer_step(HWND window, ClientState& state) {
  RECT bounds;
  GetClientRect(window, &bounds);

  if (state.session) {
    state.session->poll();
    sync_world(state);
    ingest_session_events(state);
  }

  int dx = (state.d ? 1 : 0) - (state.a ? 1 : 0);
  int dy = (state.s ? 1 : 0) - (state.w ? 1 : 0);
  const bool moving = dx != 0 || dy != 0;
  if (state.session) {
    if (state.session->connection_state() == verdigris::client::ConnectionState::Ready) {
      if (moving) submit_move(state, dx, dy);
    }
  } else if (state.simulation) {
    if (moving && !movement_hits_scenery(state, dx, dy)) {
      state.simulation->dispatch(verdigris::Command::move(dx, dy));
    } else {
      state.simulation->dispatch(verdigris::Command::action_use(verdigris::ActionType::Wait));
      if (moving && state.hint_ticks == 0) show_hint(state, "Blocked by scenery");
    }
  }

  dispatch_aim_if_changed(state, bounds, !moving && state.was_moving);
  state.was_moving = moving;

  ingest_events(state, bounds);

  for (auto& fx : state.effects) ++fx.age;
  state.effects.erase(std::remove_if(state.effects.begin(), state.effects.end(),
                                     [](const EffectFx& fx) { return fx.age >= fx.ttl; }),
                      state.effects.end());
  if (state.hint_ticks > 0) --state.hint_ticks;
  if (state.screen_pulse_ticks > 0) --state.screen_pulse_ticks;

  sync_world(state);
  state.camera.x += (state.world.player.position.x - state.camera.x) * 0.2;
  state.camera.y += (state.world.player.position.y - state.camera.y) * 0.2;
}

void dispatch_dash(ClientState& state) {
  sync_world(state);
  if (!state.world.player.alive) return;
  if (movement_hits_scenery(state, state.world.player.facing.x,
                            state.world.player.facing.y, 10)) {
    show_hint(state, "Dash blocked by scenery");
    return;
  }
  submit_action(state, verdigris::ActionType::Dash, "dash");
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  ClientState* state = state_from(window);
  switch (message) {
    case WM_NCCREATE: {
      auto* create = reinterpret_cast<CREATESTRUCT*>(lparam);
      SetWindowLongPtr(window, GWLP_USERDATA,
                       reinterpret_cast<LONG_PTR>(create->lpCreateParams));
      return TRUE;
    }
    case WM_KEYDOWN:
      if (!state) break;
      if (wparam == VK_F3) {
        state->debug_overlay = !state->debug_overlay;
        InvalidateRect(window, nullptr, FALSE);
        break;
      }
      if (wparam == VK_ESCAPE) {
        PostQuitMessage(0);
        break;
      }
      if (wparam == 'W') state->w = true;
      if (wparam == 'A') state->a = true;
      if (wparam == 'S') state->s = true;
      if (wparam == 'D') state->d = true;
      if (wparam == VK_SPACE) dispatch_dash(*state);
      if (const SkillInfo* skill = skill_for_key(wparam)) dispatch_skill(*state, *skill);
      if (wparam == 'N' && state->session)
        state->session->submit(verdigris::client::ClientCommand::enter_zone("tin:1:0"));
      if (wparam == 'X') {
        if (is_remote(*state)) {
          submit_pick_up(*state, "");
          show_hint(*state, "Take underfoot requested");
        } else {
          const std::string target = nearest_pickup_id(*state);
          if (target.empty()) {
            show_hint(*state, "No ground item or trophy nearby");
          } else {
            submit_pick_up(*state, target);
            show_hint(*state, "Picked up nearest drop");
          }
        }
      }
      if (wparam == 'Z') {
        state->loot_labels = !state->loot_labels;
        show_hint(*state, state->loot_labels ? "Loot names shown" : "Loot names hidden");
      }
      if (wparam == 'F') {
        submit_extract(*state);
        show_hint(*state, "Contextual interaction requested");
      }
      if (wparam == 'I') {
        sync_world(*state);
        state->gear_overlay = !state->gear_overlay;
        state->selected_item = 0;
        if (state->gear_overlay) show_hint(*state, "Gear opened");
      }
      sync_world(*state);
      if (state->gear_overlay && !state->world.carried.empty()) {
        const std::size_t count = state->world.carried.size();
        constexpr int kGridColumns = 4;
        if (wparam == VK_UP && state->selected_item >= kGridColumns)
          state->selected_item -= kGridColumns;
        if (wparam == VK_DOWN)
          state->selected_item =
              std::min(count - 1, state->selected_item + kGridColumns);
        if (wparam == VK_LEFT && state->selected_item > 0) --state->selected_item;
        if (wparam == VK_RIGHT)
          state->selected_item = std::min(count - 1, state->selected_item + 1);
        if (wparam == VK_RETURN) equip_selected(*state);
        if (wparam == 'U' && state->simulation) {
          state->simulation->dispatch(verdigris::Command::unequip());
          show_hint(*state, "Weapon unequipped");
        }
      }
      if (wparam >= '1' && wparam <= '9' && state->session) {
        const std::size_t index = static_cast<std::size_t>(wparam - '1');
        sync_world(*state);
        if (index < state->world.carried.size())
          submit_equip(*state, state->world.carried[index].id);
      }
      if (wparam == VK_HOME) {
        state->camera.zoom = kCameraDefaultZoom;
      }
      InvalidateRect(window, nullptr, FALSE);
      break;
    case WM_KEYUP:
      if (!state) break;
      if (wparam == 'W') state->w = false;
      if (wparam == 'A') state->a = false;
      if (wparam == 'S') state->s = false;
      if (wparam == 'D') state->d = false;
      break;
    case WM_MOUSEMOVE:
      if (state) {
        state->mouse.x = GET_X_LPARAM(lparam);
        state->mouse.y = GET_Y_LPARAM(lparam);
        RECT bounds;
        GetClientRect(window, &bounds);
        dispatch_aim_if_changed(*state, bounds);
        InvalidateRect(window, nullptr, FALSE);
      }
      break;
    case WM_MOUSEWHEEL:
      if (state) {
        const int delta = GET_WHEEL_DELTA_WPARAM(wparam);
        const double factor = delta > 0 ? 1.1 : 1.0 / 1.1;
        state->camera.zoom = std::clamp(state->camera.zoom * factor,
                                        kCameraMinZoom, kCameraMaxZoom);
        InvalidateRect(window, nullptr, FALSE);
      }
      break;
    case WM_LBUTTONDOWN:
      if (state) {
        if (state->gear_overlay)
          equip_selected(*state);
        else
          submit_action(*state, verdigris::ActionType::Melee, "melee");
      }
      InvalidateRect(window, nullptr, FALSE);
      break;
    case WM_RBUTTONDOWN:
      if (state) dispatch_dash(*state);
      InvalidateRect(window, nullptr, FALSE);
      break;
    case WM_TIMER:
      if (state) {
        timer_step(window, *state);
        InvalidateRect(window, nullptr, FALSE);
      }
      break;
    case WM_ERASEBKGND:
      return 1;
    case WM_PAINT: {
      PAINTSTRUCT paint_struct;
      HDC dc = BeginPaint(window, &paint_struct);
      paint(window, dc);
      EndPaint(window, &paint_struct);
      return 0;
    }
    case WM_DESTROY:
      KillTimer(window, 1);
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(window, message, wparam, lparam);
}

// ── D-119 scenario harness ──────────────────────────────────────────────
// Drives the REAL input→simulation→presentation pipeline with deterministic
// input scripts and asserts on three layers: authoritative core state, the
// recorded render list, and the pane/HUD. The interactive window path is
// untouched; scenarios present into an offscreen memory DC.

int scenario_failures = 0;

void scenario_check(bool ok, const char* label) {
  if (ok) {
    std::printf("    ok: %s\n", label);
  } else {
    std::printf("    FAIL: %s\n", label);
    ++scenario_failures;
  }
}

void scenario_present(ClientState& state) {
  constexpr int width = 960;
  constexpr int height = 600;
  HDC dc = CreateCompatibleDC(nullptr);
  HBITMAP bitmap = CreateCompatibleBitmap(dc, width, height);
  HGDIOBJ old = SelectObject(dc, bitmap);
  RECT bounds{0, 0, width, height};
  paint_scene(state, dc, bounds);
  SelectObject(dc, old);
  DeleteObject(bitmap);
  DeleteDC(dc);
}

void scenario_follow_camera(ClientState& state) {
  sync_world(state);
  state.camera.x = static_cast<double>(state.world.player.position.x);
  state.camera.y = static_cast<double>(state.world.player.position.y);
}

// One full pipeline step: dispatch, ingest events, age effects, follow the
// camera, and present — the headless equivalent of timer_step + paint.
void scenario_step(ClientState& state, const verdigris::Command& command) {
  RECT bounds{0, 0, 960, 600};
  state.simulation->dispatch(command);
  ingest_events(state, bounds);
  for (auto& fx : state.effects) ++fx.age;
  state.effects.erase(std::remove_if(state.effects.begin(), state.effects.end(),
                                     [](const EffectFx& fx) { return fx.age >= fx.ttl; }),
                      state.effects.end());
  if (state.screen_pulse_ticks > 0) --state.screen_pulse_ticks;
  scenario_follow_camera(state);
  scenario_present(state);
}

void scenario_begin(ClientState& state) {
  load_billboards(state.billboards);
  state.simulation =
      std::make_unique<verdigris::Simulation>(0xC011AB1EULL, "House Verdigris");
  state.simulation->dispatch(verdigris::Command::enter("route:tin:1:0"));
  generate_scenery(state);
}

std::vector<std::pair<double, double>> scenery_screen_positions(const ClientState& state) {
  std::vector<std::pair<double, double>> positions;
  for (const auto& op : state.render_list)
    if (op.op == render::Op::Scenery) positions.push_back({op.x, op.y});
  return positions;
}

std::unordered_map<std::string, std::pair<double, double>> tile_screen_by_label(
    const ClientState& state) {
  std::unordered_map<std::string, std::pair<double, double>> positions;
  for (const auto& op : state.render_list) {
    if (op.op == render::Op::Tile) positions[op.label] = {op.x, op.y};
  }
  return positions;
}

int scenario_move_and_camera() {
  ClientState state;
  scenario_begin(state);
  state.camera.x = 0.0;
  state.camera.y = 0.0;
  scenario_present(state);
  const auto baseline = scenery_screen_positions(state);
  const auto tile_baseline = tile_screen_by_label(state);
  scenario_check(baseline.size() > 3, "move-and-camera: scenery present in render list");
  scenario_check(render::any(state.render_list, render::Op::Floor),
                 "move-and-camera: Floor op recorded");
  {
    const render::Item* floor =
        render::first(state.render_list, render::Op::Floor);
    scenario_check(floor && floor->value == 1,
                   "move-and-camera: floor is textured (plates or vector kit)");
  }
  scenario_check(tile_baseline.size() > 8, "move-and-camera: terrain tiles present in render list");
  scenario_check(render::count(state.render_list, render::Op::Orb) >= 2,
                 "move-and-camera: life and resource orbs recorded");
  scenario_check(render::count(state.render_list, render::Op::Quickbar) >= 4,
                 "move-and-camera: quickbar slots recorded");
  scenario_check(render::any(state.render_list, render::Op::Minimap),
                 "move-and-camera: minimap panel recorded");

  const int steps[4][2] = {{40, 0}, {0, 40}, {-40, 0}, {0, -40}};
  for (const auto& step : steps) {
    state.camera.x = static_cast<double>(step[0]);
    state.camera.y = static_cast<double>(step[1]);
    scenario_present(state);
    const auto moved = scenery_screen_positions(state);
    const auto tiles_moved = tile_screen_by_label(state);
    if (moved.size() != baseline.size()) {
      scenario_check(false, "move-and-camera: scenery count stable across camera moves");
      continue;
    }
    const double dx0 = moved[0].first - baseline[0].first;
    const double dy0 = moved[0].second - baseline[0].second;
    bool uniform = true;
    for (std::size_t i = 0; i < moved.size(); ++i) {
      if (std::abs((moved[i].first - baseline[i].first) - dx0) > 1.0 ||
          std::abs((moved[i].second - baseline[i].second) - dy0) > 1.0) {
        uniform = false;
      }
    }
    scenario_check(uniform, "move-and-camera: every scenery entity shifts by one uniform delta");
    const double expect_x = -static_cast<double>(step[0]) * state.camera.zoom;
    const double expect_y = -static_cast<double>(step[1]) * state.camera.zoom;
    scenario_check(std::abs(dx0 - expect_x) <= 1.0 && std::abs(dy0 - expect_y) <= 1.0,
                   "move-and-camera: uniform delta equals camera shift * zoom");

    if (tiles_moved.size() >= tile_baseline.size() / 2 && !tile_baseline.empty()) {
      const double expect_x = -static_cast<double>(step[0]) * state.camera.zoom;
      const double expect_y = -static_cast<double>(step[1]) * state.camera.zoom;
      int matched = 0;
      double tdx0 = 0.0;
      double tdy0 = 0.0;
      bool tiles_uniform = true;
      for (const auto& entry : tile_baseline) {
        const auto it = tiles_moved.find(entry.first);
        if (it == tiles_moved.end()) continue;
        const double dx = it->second.first - entry.second.first;
        const double dy = it->second.second - entry.second.second;
        if (matched == 0) {
          tdx0 = dx;
          tdy0 = dy;
        } else if (std::abs(dx - tdx0) > 1.0 || std::abs(dy - tdy0) > 1.0) {
          tiles_uniform = false;
        }
        ++matched;
      }
      scenario_check(matched > 8, "move-and-camera: overlapping terrain tiles across camera moves");
      scenario_check(tiles_uniform,
                     "move-and-camera: every overlapping terrain tile shifts by one uniform delta");
      scenario_check(std::abs(tdx0 - expect_x) <= 1.0 && std::abs(tdy0 - expect_y) <= 1.0,
                     "move-and-camera: terrain delta equals camera shift * zoom");
    } else {
      scenario_check(false, "move-and-camera: terrain tiles overlap across camera moves");
    }
  }
  return 0;
}

int scenario_first_fight() {
  ClientState state;
  scenario_begin(state);
  state.camera.x = static_cast<double>(state.world.player.position.x);
  state.camera.y = static_cast<double>(state.world.player.position.y);

  // TASK-0142 owner-facing checks: the HUD names the objective, the art chip
  // honestly reports what loaded, and the extraction pad is marked.
  scenario_present(state);
  bool art_op = false;
  for (const auto& item : state.render_list)
    if (item.op == render::Op::Hud &&
        item.label.rfind("art: ", 0) == 0)
      art_op = true;
  scenario_check(art_op, "first-fight: an honest art-status line is on the HUD");
  const bool claims_plates =
      state.billboards.status.find("PNG billboards") != std::string::npos;
  const bool really_plates = state.billboards.player.ready() &&
                             state.billboards.raider.ready() &&
                             state.billboards.boss.ready();
  scenario_check(claims_plates == really_plates,
                 "first-fight: art status matches what actually loaded");
  bool objective_op = false;
  for (const auto& item : state.render_list)
    if (item.op == render::Op::Hud &&
        item.label.rfind("objective:", 0) == 0)
      objective_op = true;
  scenario_check(objective_op,
                 "first-fight: an objective strip is on the HUD");
  if (state.world.has_extraction) {
    const render::Item* pad =
        render::first(state.render_list, render::Op::Extraction);
    scenario_check(pad && pad->label == "stairs-up" && pad->radius > 0.0,
                   "first-fight: the extraction pad is marked stairs-up");
  }
  scenario_check(render::any(state.render_list, render::Op::Player),
                 "first-fight: the Scion silhouette is recorded");

  // TASK-0142: force the deterministic vector-kit path by releasing the PNG
  // plates, then re-present. This proves the no-assets fallback still draws
  // the full owner-facing scene on any machine.
  state.billboards.player.reset();
  state.billboards.raider.reset();
  state.billboards.boss.reset();
  state.billboards.tree.reset();
  state.billboards.ruin.reset();
  state.billboards.dwelling.reset();
  state.billboards.shrine.reset();
  state.billboards.terrain1.reset();
  state.billboards.terrain4.reset();
  refresh_art_status(state.billboards);
  scenario_check(state.billboards.status.find("vector kit") != std::string::npos,
                 "first-fight: vector fallback reports itself honestly");
  scenario_present(state);
  {
    const render::Item* floor =
        render::first(state.render_list, render::Op::Floor);
    scenario_check(floor && floor->value == 1 && floor->label == "tiled",
                   "first-fight: vector terrain keeps the floor tiled");
    scenario_check(render::count(state.render_list, render::Op::Tile) > 8,
                   "first-fight: vector motif tiles are drawn");
    scenario_check(render::any(state.render_list, render::Op::Scenery),
                   "first-fight: vector scenery silhouettes are drawn");
    scenario_check(render::any(state.render_list, render::Op::Player),
                   "first-fight: vector Scion silhouette is drawn");
  }

  for (int i = 0; i < 52; ++i)
    scenario_step(state, verdigris::Command::move(1, 0));

  bool saw_swing = false, saw_damage = false, saw_death = false, saw_drop = false;
  for (int i = 0; i < 10 && !saw_death; ++i) {
    scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Melee));
    saw_swing = saw_swing || render::any(state.render_list, render::Op::Swing);
    saw_damage = saw_damage || render::any(state.render_list, render::Op::Damage);
    saw_death = saw_death || render::any(state.render_list, render::Op::Death);
    saw_drop = saw_drop || render::any(state.render_list, render::Op::Drop);
  }
  scenario_check(saw_swing, "first-fight: a swing is drawn");
  scenario_check(saw_damage, "first-fight: a floating damage number is spawned");
  scenario_check(saw_death, "first-fight: a death ring is drawn");
  scenario_check(saw_drop, "first-fight: the drop becomes visible");
  scenario_check(!render::any(state.render_list, render::Op::Monster),
                 "first-fight: the dead monster is removed from the render list");
  return 0;
}

int scenario_loot_to_bank() {
  ClientState state;
  scenario_begin(state);
  for (int i = 0; i < 52; ++i)
    scenario_step(state, verdigris::Command::move(1, 0));
  for (int i = 0; i < 8; ++i)
    scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Melee));
  scenario_check(render::any(state.render_list, render::Op::Drop),
                 "loot-to-bank: drop visible before pickup");

  if (!state.simulation->ground_items().empty())
    scenario_step(state, verdigris::Command::pick_up(state.simulation->ground_items().front().id));
  if (!state.simulation->ground_trophies().empty())
    scenario_step(state, verdigris::Command::pick_up(state.simulation->ground_trophies().front().id));
  const bool has_item = !state.simulation->scion().carried_items.empty();
  scenario_check(has_item, "loot-to-bank: pickup fills the carried grid");

  state.gear_overlay = true;
  scenario_present(state);
  scenario_check(render::any(state.render_list, render::Op::PaneItem),
                 "loot-to-bank: grid cell rendered in the pane");
  const render::Item* weapon = render::first(state.render_list, render::Op::PaneWeapon);
  scenario_check(weapon && weapon->label == "(empty)",
                 "loot-to-bank: weapon seat empty before equip");

  if (has_item) {
    scenario_step(state, verdigris::Command::equip(
                              state.simulation->scion().carried_items.front().id));
    state.gear_overlay = true;
    scenario_present(state);
    weapon = render::first(state.render_list, render::Op::PaneWeapon);
    scenario_check(weapon && weapon->label != "(empty)",
                   "loot-to-bank: equip fills the weapon seat");
    const render::Item* stat = render::first(state.render_list, render::Op::PaneStat);
    scenario_check(stat && stat->label.find("(+") != std::string::npos,
                   "loot-to-bank: equipped bonus appears in the stat readout");
  }

  bool objective_carries = false;
  {
    state.gear_overlay = false;
    scenario_present(state);
    for (const auto& item : state.render_list)
      if (item.op == render::Op::Hud &&
          item.label.rfind("objective: carry your loot", 0) == 0)
        objective_carries = true;
  }
  scenario_check(objective_carries,
                 "loot-to-bank: objective strip points at the EXIT while carrying");

  state.gear_overlay = false;
  for (int i = 0; i < 52; ++i)
    scenario_step(state, verdigris::Command::move(-1, 0));
  scenario_step(state, verdigris::Command::extract());
  scenario_check(state.simulation->house().stored_items.size() == 1,
                 "loot-to-bank: extraction banks the item");
  scenario_check(state.simulation->house().stored_trophies.size() == 1,
                 "loot-to-bank: extraction banks the trophy");

  state.gear_overlay = true;
  scenario_present(state);
  const render::Item* banked = render::first(state.render_list, render::Op::PaneBanked);
  scenario_check(banked && banked->label.find("items 1") != std::string::npos &&
                     banked->label.find("trophies 1") != std::string::npos,
                 "loot-to-bank: banked footer reflects the extraction");

  // TASK-0142: the owner-facing objective strip walks the loop — it points
  // at the EXIT while loot is carried and keeps guiding after banking.
  bool objective_points_exit = false;
  for (const auto& item : state.render_list)
    if (item.op == render::Op::Hud &&
        item.label.rfind("objective: reach the EXIT", 0) == 0)
      objective_points_exit = true;
  scenario_check(objective_points_exit,
                 "loot-to-bank: objective strip guides back to the extraction");
  return 0;
}

int scenario_telegraph_dodge() {
  ClientState state;
  scenario_begin(state);
  const auto* before = state.simulation->actor(state.simulation->scion().actor_id);
  if (!before) return 0;
  const int start_life = before->stats.life;

  // Spawn an elite at melee range behind the player's initial +x facing so a
  // single dash carries the player out of the sweep radius before the windup.
  const int melee = verdigris::world_scale::kMeleeRange;
  state.simulation->spawn_monster(
      {before->position.x - melee, before->position.y}, 1, true);

  // The elite's turn schedules a melee-range sweep telegraph.
  scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait));
  scenario_check(render::any(state.render_list, render::Op::Telegraph),
                 "telegraph-dodge: elite telegraph is drawn");
  scenario_check(telegraph_avoids_hud(state.render_list, RECT{0, 0, 960, 600}),
                 "telegraph-dodge: telegraph stays outside HUD reserve");

  // Dodge: dash along the facing (away from the elite) and let the windup
  // resolve across the remaining ticks.
  scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Dash));
  for (int i = 0; i < 3; ++i)
    scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait));

  const auto* after = state.simulation->actor(state.simulation->scion().actor_id);
  scenario_check(after && after->stats.life == start_life,
                 "telegraph-dodge: moving out avoids the telegraphed damage");
  return 0;
}

int scenario_combat_juice() {
  ClientState state;
  scenario_begin(state);
  for (int i = 0; i < 52; ++i)
    scenario_step(state, verdigris::Command::move(1, 0));

  // Melee until the monster dies, watching for the target flash + number.
  bool saw_target_flash = false, saw_damage = false, saw_death = false;
  for (int i = 0; i < 10 && !saw_death; ++i) {
    scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Melee));
    saw_target_flash = saw_target_flash || render::any(state.render_list, render::Op::TargetFlash);
    saw_damage = saw_damage || render::any(state.render_list, render::Op::Damage);
    saw_death = saw_death || render::any(state.render_list, render::Op::Death);
  }
  scenario_check(saw_target_flash, "combat-juice: target sprite flashes on the hit");
  scenario_check(saw_damage, "combat-juice: a floating damage number is spawned");

  // Number lifetime (~600ms = 12 ticks): present right after the killing blow,
  // still visible ~300ms in, gone after ~650ms. No further damage once dead.
  bool present_early = render::any(state.render_list, render::Op::Damage);
  bool present_mid = false, gone_late = false;
  for (int i = 0; i < 14; ++i) {
    scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait));
    if (i == 5) present_mid = render::any(state.render_list, render::Op::Damage);
    if (i == 13) gone_late = !render::any(state.render_list, render::Op::Damage);
  }
  scenario_check(present_early, "combat-juice: number present right after the hit");
  scenario_check(present_mid, "combat-juice: number still visible ~300ms in");
  scenario_check(gone_late, "combat-juice: number faded out after ~650ms");

  // Player damage: spawn an elite at melee range and let its sweep resolve
  // (no dodge) so the player takes a hit -> screen-edge pulse + player-tagged
  // number.
  const auto* player0 = state.simulation->actor(state.simulation->scion().actor_id);
  if (player0) {
    const int melee = verdigris::world_scale::kMeleeRange;
    state.simulation->spawn_monster(
        {player0->position.x - melee, player0->position.y}, 1, true);
    bool saw_pulse = false, saw_player_dmg = false;
    for (int i = 0; i < 6; ++i) {
      scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait));
      saw_pulse = saw_pulse || render::any(state.render_list, render::Op::ScreenPulse);
      const render::Item* dmg = render::first(state.render_list, render::Op::Damage);
      saw_player_dmg = saw_player_dmg || (dmg && dmg->label == "player");
    }
    scenario_check(saw_pulse, "combat-juice: screen-edge red pulse on player damage");
    scenario_check(saw_player_dmg, "combat-juice: player damage number is player-tagged");
  }
  return 0;
}

int scenario_remote_render_list() {
  verdigris::networking::WebSocketServer* server = nullptr;
  std::uint16_t port = 0;
  for (std::uint16_t candidate = 6580; candidate <= 6599; ++candidate) {
    auto* probe = new verdigris::networking::WebSocketServer(candidate);
    std::string error;
    if (probe->start(&error)) {
      server = probe;
      port = candidate;
      break;
    }
    delete probe;
  }
  scenario_check(server != nullptr, "remote-render-list: bound cursor-capsule server");
  if (!server) return 0;

  ClientState state;
  state.session = std::make_unique<verdigris::client::RemoteProtocolSession>(
      "127.0.0.1", port, "cursor-render-0064", true);
  std::string error;
  scenario_check(state.session->start(&error), "remote-render-list: session start");
  bool ready = false;
  for (int i = 0; i < 250 && !ready; ++i) {
    state.session->poll();
    ready = state.session->connection_state() == verdigris::client::ConnectionState::Ready;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  scenario_check(ready, "remote-render-list: handshake ready");
  state.session->submit(verdigris::client::ClientCommand::enter_zone("tin:1:0"));
  for (int i = 0; i < 80; ++i) {
    state.session->poll();
    ingest_session_events(state);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    if (state.session->model().scene.type == "instance") break;
  }
  generate_scenery(state);
  sync_world(state);
  state.camera.x = static_cast<double>(state.world.player.position.x);
  state.camera.y = static_cast<double>(state.world.player.position.y);
  load_billboards(state.billboards);
  scenario_present(state);
  scenario_check(render::any(state.render_list, render::Op::Floor),
                 "remote-render-list: Floor op in paint_scene render list");
  scenario_check(render::any(state.render_list, render::Op::Tile),
                 "remote-render-list: Tile ops in paint_scene render list");
  scenario_check(render::count(state.render_list, render::Op::Orb) >= 2,
                 "remote-render-list: vital orbs in paint_scene render list");
  scenario_check(render::count(state.render_list, render::Op::Quickbar) >= 4,
                 "remote-render-list: quickbar slots in paint_scene render list");
  scenario_check(render::any(state.render_list, render::Op::Minimap),
                 "remote-render-list: minimap in paint_scene render list");
  const render::Item* extract = render::first(state.render_list, render::Op::Extraction);
  scenario_check(extract && extract->label == "stairs-up",
                 "remote-render-list: Extraction pad marked stairs-up");
  bool saw_conn = false;
  const char* conn_label =
      verdigris::client::connection_state_label(state.session->connection_state());
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Hud &&
        item.label == std::string("connection ") + conn_label)
      saw_conn = true;
  }
  scenario_check(saw_conn, "remote-render-list: connection chip uses connection_state_label");

  bool saw_monster = false, saw_swing = false, saw_drop = false;
  for (int step = 0; step < 240; ++step) {
    state.session->submit(verdigris::client::ClientCommand::use_action("melee"));
    if (step % 4 == 0) state.session->submit(verdigris::client::ClientCommand::move(1, 0));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    state.session->poll();
    ingest_session_events(state);
    sync_world(state);
    state.camera.x = static_cast<double>(state.world.player.position.x);
    state.camera.y = static_cast<double>(state.world.player.position.y);
    for (auto& fx : state.effects) ++fx.age;
    state.effects.erase(std::remove_if(state.effects.begin(), state.effects.end(),
                                       [](const EffectFx& fx) { return fx.age >= fx.ttl; }),
                        state.effects.end());
    scenario_present(state);
    saw_monster = saw_monster || render::any(state.render_list, render::Op::Monster);
    saw_swing = saw_swing || render::any(state.render_list, render::Op::Swing);
    saw_drop = saw_drop || render::any(state.render_list, render::Op::Drop);
    if (saw_monster && saw_swing && saw_drop) break;
  }
  scenario_check(saw_monster, "remote-render-list: Monster op in paint_scene render list");
  scenario_check(saw_swing, "remote-render-list: Swing op in paint_scene render list");
  scenario_check(saw_drop, "remote-render-list: Drop op in paint_scene render list");
  scenario_check(!state.simulation, "remote-render-list: remote present uses no Simulation");

  if (state.session) state.session->shutdown();
  server->stop();
  delete server;
  return 0;
}

int scenario_zoom_invariance() {
  ClientState state;
  scenario_begin(state);
  const double zooms[] = {kCameraMinZoom, kCameraDefaultZoom, kCameraMaxZoom};
  const char* names[] = {"min", "default", "max"};
  for (int z = 0; z < 3; ++z) {
    state.camera.x = 0.0;
    state.camera.y = 0.0;
    state.camera.zoom = zooms[z];
    scenario_present(state);
    const auto base = scenery_screen_positions(state);
    const auto tile_base = tile_screen_by_label(state);
    scenario_check(base.size() > 3, "zoom-invariance: scenery present in render list");
    scenario_check(tile_base.size() > 8, "zoom-invariance: terrain tiles present in render list");

    state.camera.x = 40.0;
    state.camera.y = 0.0;
    scenario_present(state);
    const auto moved = scenery_screen_positions(state);
    const auto tiles_moved = tile_screen_by_label(state);
    bool uniform = moved.size() == base.size();
    double dx0 = 0.0, dy0 = 0.0;
    if (uniform) {
      dx0 = moved[0].first - base[0].first;
      dy0 = moved[0].second - base[0].second;
      for (std::size_t i = 0; i < moved.size(); ++i) {
        if (std::abs((moved[i].first - base[i].first) - dx0) > 1.0 ||
            std::abs((moved[i].second - base[i].second) - dy0) > 1.0) {
          uniform = false;
        }
      }
    }
    std::string label = std::string("zoom-invariance: uniform delta at ") + names[z] + " zoom";
    scenario_check(uniform, label.c_str());
    const double expect_x = -40.0 * state.camera.zoom;
    scenario_check(std::abs(dx0 - expect_x) <= 1.0,
                   "zoom-invariance: delta scales with the zoom factor");

    if (!tile_base.empty()) {
      int matched = 0;
      double tdx0 = 0.0;
      double tdy0 = 0.0;
      bool tiles_uniform = true;
      for (const auto& entry : tile_base) {
        const auto it = tiles_moved.find(entry.first);
        if (it == tiles_moved.end()) continue;
        const double dx = it->second.first - entry.second.first;
        const double dy = it->second.second - entry.second.second;
        if (matched == 0) {
          tdx0 = dx;
          tdy0 = dy;
        } else if (std::abs(dx - tdx0) > 1.0 || std::abs(dy - tdy0) > 1.0) {
          tiles_uniform = false;
        }
        ++matched;
      }
      scenario_check(matched > 8, "zoom-invariance: overlapping terrain tiles across camera shift");
      scenario_check(tiles_uniform,
                     "zoom-invariance: overlapping terrain tiles shift uniformly at zoom level");
    } else {
      scenario_check(false, "zoom-invariance: terrain tiles overlap across camera shift");
    }
  }
  return 0;
}

int run_scenarios(const std::string& which) {
  struct Entry {
    const char* name;
    int (*fn)();
  };
  const Entry entries[] = {
      {"move-and-camera", scenario_move_and_camera},
      {"first-fight", scenario_first_fight},
      {"loot-to-bank", scenario_loot_to_bank},
      {"telegraph-dodge", scenario_telegraph_dodge},
      {"combat-juice", scenario_combat_juice},
      {"remote-render-list", scenario_remote_render_list},
      {"zoom-invariance", scenario_zoom_invariance},
  };
  int total_failures = 0;
  for (const auto& entry : entries) {
    if (which != "all" && which != entry.name) continue;
    scenario_failures = 0;
    std::printf("== scenario %s ==\n", entry.name);
    entry.fn();
    total_failures += scenario_failures;
    std::printf("   %s (%d failures)\n", scenario_failures == 0 ? "PASS" : "FAIL",
                scenario_failures);
  }
  return total_failures;
}

const char* render_op_name(render::Op op) {
  switch (op) {
    case render::Op::Floor: return "Floor";
    case render::Op::Tile: return "Tile";
    case render::Op::Scenery: return "Scenery";
    case render::Op::Player: return "Player";
    case render::Op::Monster: return "Monster";
    case render::Op::Telegraph: return "Telegraph";
    case render::Op::Swing: return "Swing";
    case render::Op::Sweep: return "Sweep";
    case render::Op::WarCry: return "WarCry";
    case render::Op::Impact: return "Impact";
    case render::Op::Death: return "Death";
    case render::Op::Damage: return "Damage";
    case render::Op::TargetFlash: return "TargetFlash";
    case render::Op::ScreenPulse: return "ScreenPulse";
    case render::Op::Drop: return "Drop";
    case render::Op::Extraction: return "Extraction";
    case render::Op::Hud: return "Hud";
    case render::Op::Orb: return "Orb";
    case render::Op::Quickbar: return "Quickbar";
    case render::Op::Minimap: return "Minimap";
    case render::Op::PaneStat: return "PaneStat";
    case render::Op::PaneWeapon: return "PaneWeapon";
    case render::Op::PaneItem: return "PaneItem";
    case render::Op::PaneBanked: return "PaneBanked";
  }
  return "Unknown";
}

std::string json_escape(const std::string& text) {
  std::string out;
  out.reserve(text.size());
  for (char character : text) {
    if (character == '"' || character == '\\') out.push_back('\\');
    if (character == '\n') {
      out += "\\n";
      continue;
    }
    out.push_back(character);
  }
  return out;
}

std::string dump_render_list_json(const std::string& scene, int width, int height,
                                  const render::List& list) {
  std::string json = "{\n  \"scene\": \"" + json_escape(scene) + "\",\n  \"width\": " +
                     std::to_string(width) + ",\n  \"height\": " + std::to_string(height) +
                     ",\n  \"ops\": [\n";
  for (std::size_t i = 0; i < list.size(); ++i) {
    const auto& item = list[i];
    char line[512];
    std::snprintf(line, sizeof(line),
                  "    {\"op\":\"%s\",\"x\":%.4f,\"y\":%.4f,\"radius\":%.4f,\"value\":%d,"
                  "\"label\":\"%s\"}%s\n",
                  render_op_name(item.op), item.x, item.y, item.radius, item.value,
                  json_escape(item.label).c_str(), i + 1 < list.size() ? "," : "");
    json += line;
  }
  json += "  ]\n}\n";
  return json;
}

std::string reference_capture_dir() {
  std::vector<std::string> bases{".", executable_directory()};
  const char* marker = "orchestration\\tasks\\TASK-0070-reference-scenes";
  for (const auto& base : bases) {
    std::string prefix = base;
    for (int depth = 0; depth <= 6; ++depth) {
      const std::string folder = prefix + (prefix.empty() ? "" : "\\") + marker;
      if (directory_exists(folder)) {
        const std::string captures = folder + "\\captures";
        CreateDirectoryA(captures.c_str(), nullptr);
        return captures;
      }
      prefix += prefix.empty() ? ".." : "\\..";
    }
  }
  CreateDirectoryA("captures", nullptr);
  return "captures";
}

bool save_hbitmap_png(BillboardAssets& assets, HBITMAP bitmap, const std::string& path) {
  if (!assets.create_bitmap_from_hbitmap || !assets.save_image_to_file ||
      !assets.dispose_image)
    return false;
  GpBitmap* image = nullptr;
  if (assets.create_bitmap_from_hbitmap(bitmap, nullptr, &image) != 0 || !image) return false;
  const CLSID png_clsid = {0x557cf406, 0x1a04, 0x11d3, {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}};
  const std::wstring wide = wide_path(path);
  const bool ok = !wide.empty() && assets.save_image_to_file(image, wide.c_str(), &png_clsid, nullptr) == 0;
  assets.dispose_image(image);
  return ok;
}

bool reference_present(ClientState& state, int width, int height, const std::string& png_path) {
  BITMAPINFO info{};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = width;
  info.bmiHeader.biHeight = -height;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  void* bits = nullptr;
  HBITMAP bitmap = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
  if (!bitmap) return false;
  HDC dc = CreateCompatibleDC(nullptr);
  HGDIOBJ old = SelectObject(dc, bitmap);
  RECT bounds{0, 0, width, height};
  paint_scene(state, dc, bounds);
  bool saved = png_path.empty() || save_hbitmap_png(state.billboards, bitmap, png_path);
  SelectObject(dc, old);
  DeleteDC(dc);
  DeleteObject(bitmap);
  return saved;
}

void reference_step(ClientState& state, const verdigris::Command& command, int width, int height) {
  RECT bounds{0, 0, width, height};
  state.simulation->dispatch(command);
  ingest_events(state, bounds);
  for (auto& fx : state.effects) ++fx.age;
  state.effects.erase(std::remove_if(state.effects.begin(), state.effects.end(),
                                     [](const EffectFx& fx) { return fx.age >= fx.ttl; }),
                      state.effects.end());
  if (state.screen_pulse_ticks > 0) --state.screen_pulse_ticks;
  scenario_follow_camera(state);
  reference_present(state, width, height, "");
}

bool setup_reference_scene(int scene, ClientState& state, int width, int height, std::string& why) {
  scenario_begin(state);
  load_billboards(state.billboards);
  scenario_follow_camera(state);
  const int melee = verdigris::world_scale::kMeleeRange;
  const auto* scion = state.simulation->actor(state.simulation->scion().actor_id);
  if (!scion) {
    why = "no scion";
    return false;
  }
  const verdigris::Vec2 spawn = scion->position;

  if (scene == 1) {
    reference_present(state, width, height, "");
    return true;
  }

  if (scene == 2) {
    state.simulation->spawn_monster({spawn.x + melee, spawn.y}, 1, false);
    state.simulation->spawn_monster({spawn.x + melee, spawn.y + melee}, 1, false);
    for (int i = 0; i < 8; ++i)
      reference_step(state, verdigris::Command::move(1, 0), width, height);
    bool saw_swing = false;
    int monsters = 0;
    for (int i = 0; i < 12; ++i) {
      reference_step(state, verdigris::Command::action_use(verdigris::ActionType::Melee), width,
                     height);
      saw_swing = saw_swing || render::any(state.render_list, render::Op::Swing);
      monsters = render::count(state.render_list, render::Op::Monster);
      if (saw_swing && monsters >= 2) break;
    }
    if (!saw_swing || monsters < 2) {
      why = "pack combat missing swing or second monster";
      return false;
    }
    return true;
  }

  if (scene == 3) {
    state.simulation->spawn_monster({spawn.x - melee, spawn.y}, 1, true);
    reference_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait), width,
                   height);
    if (!render::any(state.render_list, render::Op::Telegraph)) {
      why = "elite telegraph not drawn";
      return false;
    }
    return true;
  }

  if (scene == 4) {
    for (int i = 0; i < 52; ++i)
      reference_step(state, verdigris::Command::move(1, 0), width, height);
    for (int i = 0; i < 12; ++i)
      reference_step(state, verdigris::Command::action_use(verdigris::ActionType::Melee), width,
                     height);
    if (!state.simulation->ground_items().empty())
      reference_step(state, verdigris::Command::pick_up(state.simulation->ground_items().front().id),
                     width, height);
    if (!state.simulation->ground_trophies().empty())
      reference_step(
          state, verdigris::Command::pick_up(state.simulation->ground_trophies().front().id), width,
          height);
    if (state.world.carried.empty()) {
      why = "named drop did not enter inventory";
      return false;
    }
    state.gear_overlay = true;
    reference_present(state, width, height, "");
    if (!render::any(state.render_list, render::Op::PaneItem) &&
        !render::any(state.render_list, render::Op::PaneWeapon)) {
      why = "gear pane not in render list";
      return false;
    }
    if (!render::any(state.render_list, render::Op::Drop) && state.world.carried.empty()) {
      why = "no drop or carried item";
      return false;
    }
    return true;
  }

  if (scene == 5) {
    for (int i = 0; i < 52; ++i)
      reference_step(state, verdigris::Command::move(1, 0), width, height);
    sync_world(state);
    const verdigris::Vec2 here = state.world.player.position;
    state.simulation->spawn_monster({here.x - melee, here.y}, 1, true);
    state.simulation->spawn_monster({here.x - melee, here.y + melee}, 1, true);
    state.simulation->spawn_monster({here.x + melee, here.y}, 1, false);
    bool critical = false;
    for (int i = 0; i < 80; ++i) {
      reference_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait), width,
                     height);
      sync_world(state);
      const int maximum = std::max(1, state.world.player.life_max);
      const bool pulse = state.screen_pulse_ticks > 0 ||
                         render::any(state.render_list, render::Op::ScreenPulse);
      if (state.world.player.life * 4 < maximum && pulse) {
        critical = true;
        break;
      }
    }
    if (!critical) {
      why = "life " + std::to_string(state.world.player.life) + "/" +
            std::to_string(state.world.player.life_max) + " pulse=" +
            std::to_string(state.screen_pulse_ticks);
      return false;
    }
    return true;
  }

  why = "unknown scene";
  return false;
}

int run_reference_scenes(const std::string& which) {
  struct Scene {
    int id;
    const char* name;
  };
  const Scene scenes[] = {
      {1, "01-route-entrance"}, {2, "02-pack-combat"}, {3, "03-elite-telegraph"},
      {4, "04-named-drop-gear"}, {5, "05-critical-health"},
  };
  const std::string out_dir = reference_capture_dir();
  std::printf("reference-scene: writing to %s\n", out_dir.c_str());
  int failures = 0;
  for (const auto& scene : scenes) {
    if (which != "all" && which != std::to_string(scene.id) && which != scene.name) continue;
    std::string why;
    ClientState first;
    if (!setup_reference_scene(scene.id, first, 1920, 1080, why)) {
      std::printf("FAIL %s: %s\n", scene.name, why.c_str());
      ++failures;
      continue;
    }
    const std::string json = dump_render_list_json(scene.name, 1920, 1080, first.render_list);
    ClientState second;
    if (!setup_reference_scene(scene.id, second, 1920, 1080, why)) {
      std::printf("FAIL %s second run: %s\n", scene.name, why.c_str());
      ++failures;
      continue;
    }
    const std::string json2 = dump_render_list_json(scene.name, 1920, 1080, second.render_list);
    if (json != json2) {
      std::printf("FAIL %s: render-list JSON differed across two runs\n", scene.name);
      ++failures;
      continue;
    }
    const std::string json_path = out_dir + "\\" + scene.name + ".json";
    std::ofstream file(json_path, std::ios::binary);
    file << json;
    file.close();
    if (!file) {
      std::printf("FAIL %s: could not write %s\n", scene.name, json_path.c_str());
      ++failures;
      continue;
    }
    const std::string png_1080 = out_dir + "\\" + scene.name + "-1920x1080.png";
    const std::string png_768 = out_dir + "\\" + scene.name + "-1366x768.png";
    if (!reference_present(first, 1920, 1080, png_1080)) {
      std::printf("FAIL %s: PNG 1920x1080\n", scene.name);
      ++failures;
      continue;
    }
    ClientState wide;
    if (!setup_reference_scene(scene.id, wide, 1366, 768, why) ||
        !reference_present(wide, 1366, 768, png_768)) {
      std::printf("FAIL %s: PNG 1366x768 (%s)\n", scene.name, why.c_str());
      ++failures;
      continue;
    }
    std::printf("ok %s (%zu ops)\n", scene.name, first.render_list.size());
  }
  return failures;
}

}  // namespace

int run_remote_native_client(const char* host, unsigned short port, const char* guest_id) {
  auto state = std::make_unique<ClientState>();
  state->session = std::make_unique<verdigris::client::RemoteProtocolSession>(
      host ? host : "127.0.0.1", port, guest_id ? guest_id : "cursor-guest", true);
  std::string error;
  if (!state->session->start(&error)) {
    std::fprintf(stderr, "verdigris_client --remote: %s\n", error.c_str());
    state->hint = error;
    state->hint_ticks = 200;
  }
  load_billboards(state->billboards);

  HINSTANCE instance = GetModuleHandle(nullptr);
  WNDCLASSA window_class{};
  window_class.hInstance = instance;
  window_class.lpfnWndProc = window_proc;
  window_class.lpszClassName = "VerdigrisNativeClient";
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassA(&window_class);

  HWND window =
      CreateWindowExA(0, window_class.lpszClassName, "Verdigris Remote Guest", WS_OVERLAPPEDWINDOW,
                      CW_USEDEFAULT, CW_USEDEFAULT, 960, 600, nullptr, nullptr, instance,
                      state.get());
  ShowWindow(window, SW_SHOW);
  SetTimer(window, 1, 50, nullptr);

  MSG message{};
  while (GetMessage(&message, nullptr, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  if (state->session) state->session->shutdown();
  return static_cast<int>(message.wParam);
}

int run_headless_demo() {
  verdigris::Simulation simulation(0xC011AB1EULL, "House Verdigris");
  verdigris::EmberHunt seasonal;
  simulation.set_seasonal_mechanic(&seasonal);
  simulation.dispatch(verdigris::Command::enter("route:tin:1:0"));
  for (int i = 0; i < 52; ++i) simulation.dispatch(verdigris::Command::move(1, 0));
  for (int i = 0; i < 8; ++i)
    simulation.dispatch(verdigris::Command::action_use(verdigris::ActionType::Melee));
  if (!simulation.ground_items().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_items().front().id));
  if (!simulation.ground_trophies().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_trophies().front().id));
  for (int i = 0; i < 52; ++i) simulation.dispatch(verdigris::Command::move(-1, 0));
  simulation.dispatch(verdigris::Command::extract());
  const std::size_t trophies_stored = simulation.house().stored_trophies.size();
  const std::size_t items_stored = simulation.house().stored_items.size();
  std::cout << "Verdigris native client shell\n"
            << "House: " << simulation.house().name
            << " | trophies stored: " << trophies_stored
            << " | items stored: " << items_stored << "\n";
  return trophies_stored == 1 && items_stored == 1 ? 0 : 1;
}

// A standard main() keeps the console subsystem so --headless output reaches
// stdout; the interactive window is created explicitly from the module handle.
int main(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--headless") == 0) return run_headless_demo();
    if (std::strcmp(argv[i], "--scenario") == 0 && i + 1 < argc)
      return run_scenarios(argv[i + 1]);
    if (std::strcmp(argv[i], "--reference-scene") == 0 && i + 1 < argc)
      return run_reference_scenes(argv[i + 1]);
    if (std::strcmp(argv[i], "--remote") == 0) {
      const char* host = "127.0.0.1";
      unsigned short port = 6580;
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        const bool dotted = std::strchr(argv[i + 1], '.') != nullptr;
        if (dotted) {
          host = argv[++i];
          if (i + 1 < argc && argv[i + 1][0] != '-')
            port = static_cast<unsigned short>(std::atoi(argv[++i]));
        } else {
          port = static_cast<unsigned short>(std::atoi(argv[++i]));
        }
      }
      return run_remote_native_client(host, port, "cursor-guest");
    }
  }
  HINSTANCE instance = GetModuleHandle(nullptr);
  auto state = std::make_unique<ClientState>();
  state->simulation = std::make_unique<verdigris::Simulation>(0xC011AB1EULL, "House Verdigris");
  verdigris::EmberHunt seasonal;
  state->simulation->set_seasonal_mechanic(&seasonal);
  state->simulation->dispatch(verdigris::Command::enter("route:tin:1:0"));
  generate_scenery(*state);
  load_billboards(state->billboards);

  WNDCLASSA window_class{};
  window_class.hInstance = instance;
  window_class.lpfnWndProc = window_proc;
  window_class.lpszClassName = "VerdigrisNativeClient";
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassA(&window_class);

  HWND window = CreateWindowExA(0, window_class.lpszClassName, "Verdigris Core Testbed",
                                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 960, 600,
                                nullptr, nullptr, instance, state.get());
  ShowWindow(window, SW_SHOW);
  SetTimer(window, 1, 50, nullptr);

  MSG message{};
  while (GetMessage(&message, nullptr, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  return static_cast<int>(message.wParam);
}
#else
int run_headless_demo() {
  verdigris::Simulation simulation(0xC011AB1EULL, "House Verdigris");
  verdigris::EmberHunt seasonal;
  simulation.set_seasonal_mechanic(&seasonal);
  simulation.dispatch(verdigris::Command::enter("route:tin:1:0"));
  for (int i = 0; i < 52; ++i) simulation.dispatch(verdigris::Command::move(1, 0));
  for (int i = 0; i < 8; ++i)
    simulation.dispatch(verdigris::Command::action_use(verdigris::ActionType::Melee));
  if (!simulation.ground_items().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_items().front().id));
  if (!simulation.ground_trophies().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_trophies().front().id));
  for (int i = 0; i < 52; ++i) simulation.dispatch(verdigris::Command::move(-1, 0));
  simulation.dispatch(verdigris::Command::extract());
  const std::size_t trophies_stored = simulation.house().stored_trophies.size();
  const std::size_t items_stored = simulation.house().stored_items.size();
  std::cout << "Verdigris native client shell\n"
            << "House: " << simulation.house().name
            << " | trophies stored: " << trophies_stored
            << " | items stored: " << items_stored << "\n";
  return trophies_stored == 1 && items_stored == 1 ? 0 : 1;
}

int main() {
  return run_headless_demo();
}
#endif
