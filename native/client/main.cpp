#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
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

// TASK-0122 Phase A: the single named presentation constants table.
namespace phase_a = verdigris::client::phase_a;

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
#include "ui_skin.hpp"
#include "../audio/audio_mixer.hpp"
#include "audio_out.hpp"
#include "framekit_renderer.hpp"
#include "geometric_skill_tree.hpp"

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
// The zoom constants were tuned on a 600px-tall window. Fullscreen keeps the
// same on-screen world scale by growing zoom with window height; the shipped
// test resolutions (height 600) resolve to a factor of exactly 1.
inline double zoom_height_factor(int height) {
  return std::max(1.0, static_cast<double>(height) / 600.0);
}
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
using verdigris::client::ExpeditionPhaseView;
using verdigris::client::extraction_action_hint;

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
  // WIZARD Framekit chrome (TASK-0180 assets, finally consumed): nine-slice
  // panel/slot plates and the item-art sprites for inventory cells.
  SpriteBitmap fk_panel;
  SpriteBitmap fk_slot;
  std::unordered_map<std::string, SpriteBitmap> item_art;
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
    fk_panel.reset();
    fk_slot.reset();
    for (auto& entry : item_art) entry.second.reset();
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

// TASK-0145: the two owner-facing screens. Expedition is the historical
// TASK-0142 presentation, untouched. Chronicles is the pre-game front door
// (House/Scion/oath/admission) plus the post-fall succession view.
enum class Screen { Expedition, Chronicles };

// One actionable front-door control, rebuilt deterministically from the
// authoritative chronicle model every frame. `key` is the keyboard binding
// shown to the owner; `command`/`arg` feed IClientSession::submit.
struct ChronicleAction {
  std::string key;
  std::string command;
  std::string arg;
  std::string label;
};

// ── TASK-0159: deterministic readability geometry ────────────────────────
// One pure integer-geometry source of truth for every fixed screen region the
// readability contract names (gear pane, minimap, quickbar, vital orbs). The
// painter, the top-HUD planner, and the scenario harness all reason about
// these exact rectangles, so a collision is a provable fact rather than a
// pixel impression. No asset, font, or windowing dependency.
struct HudRect {
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
};

bool hud_rects_overlap(const HudRect& a, const HudRect& b) {
  return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h &&
         b.y < a.y + a.h;
}

// Integer HUD scale by window height: the shipped 960x600/1366x768 layouts
// keep their exact historical geometry (scale 1, so every readability
// contract number is unchanged), while fullscreen 1440p+ doubles the chrome
// instead of shrinking it to ant size.
int hud_scale(int height) { return std::max(1, height / 700); }

// The shipped gear pane. Identical numbers to the historical painter, now
// shared with the planner so global HUD text can never be placed onto it.
HudRect gear_pane_rect(int width, int height) {
  const int s = hud_scale(height);
  const int pane_w = 380 * s;
  const int pane_top = 64 * s;
  const int x = std::max(24, width - pane_w - 24);
  const int bottom = std::min(height - 28, pane_top + 430 * s);
  return {x, pane_top, std::min(pane_w, std::max(0, width - x)),
          std::max(0, bottom - pane_top)};
}

HudRect minimap_rect(int height) {
  const int s = hud_scale(height);
  const int size = 108 * s;
  const int margin = 12 * s;
  return {margin, margin, size, size};
}

constexpr int kVitalOrbRadius = 34;

HudRect vital_orb_rect(int width, int height, bool resource) {
  const int radius = kVitalOrbRadius * hud_scale(height);
  const int cx = resource ? width - 18 - radius : 18 + radius;
  const int cy = height - 18 - radius;
  // +3 clears the low-life pulse ring, the widest the orb ever paints.
  const int r = radius + 3;
  return {cx - r, cy - r, r * 2, r * 2};
}

constexpr int kQuickbarSlotCount = 4;

HudRect quickbar_strip_rect(int width, int height) {
  const int s = hud_scale(height);
  const int slot_w = 58 * s;
  const int slot_h = 52 * s;
  const int gap = 8 * s;
  const int strip_w =
      kQuickbarSlotCount * slot_w + (kQuickbarSlotCount - 1) * gap;
  const int bottom = height - 18;
  const int top = bottom - slot_h;
  const int left = (width - strip_w) / 2;
  // The painted plate extends 10 left/right of the slots and 8 above/4 below.
  return {left - 10, top - 8, strip_w + 20, slot_h + 12};
}

// Offscreen floor cache: the tiled ground re-renders only when the camera
// crosses a tile boundary, the zoom or scene changes, or the window is
// resized; every other frame is one BitBlt. Cuts the largest per-frame GDI
// cost (a StretchBlt per visible tile) to near zero when standing still and
// to a couple of refreshes per second while walking.
struct FloorCache {
  HDC dc = nullptr;
  HBITMAP bitmap = nullptr;
  HGDIOBJ old_bitmap = nullptr;
  int width = 0;
  int height = 0;
  int tx0 = 0, ty0 = 0, tx1 = 0, ty1 = 0;
  double zoom = 0.0;
  int view_w = 0, view_h = 0;
  std::string route;
  bool valid = false;

  void release() {
    if (dc) {
      SelectObject(dc, old_bitmap);
      DeleteDC(dc);
      dc = nullptr;
    }
    if (bitmap) {
      DeleteObject(bitmap);
      bitmap = nullptr;
    }
    valid = false;
  }

  ~FloorCache() { release(); }
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
  // TASK-0159: rectangles of every readability-contract HUD region, recorded
  // next to each draw (the same discipline as render_list ops) so the
  // deterministic scenario can hard-fail on intersections. Presentation
  // diagnostic only; normal play never reads it.
  std::vector<std::pair<std::string, HudRect>> hud_rect_trace;
  int loot_scatter = 0;
  // TASK-0122 Phase A: presentation-side memory of already-materialized foes
  // so the spawn beat fires exactly once per monster.
  std::unordered_set<std::string> known_monsters;
  bool loot_labels = false;
  bool gear_overlay = false;
  bool debug_overlay = false;
  // Last full paint_scene duration in milliseconds (F3 overlay); the honest
  // per-frame budget readout that catches presentation-cost regressions.
  double last_paint_ms = 0.0;
  // Section breakdown of the last frame (floor+walls, world pass, HUD).
  double paint_ms_floor = 0.0;
  double paint_ms_world = 0.0;
  double paint_ms_hud = 0.0;
  // Frame pacing: the timer fires ~66x/s for smooth rendering while the
  // simulation-facing logic keeps its exact 50 ms cadence via accumulator.
  long long last_frame_qpc = 0;
  double tick_accum_ms = 0.0;
  // Honest on-screen frame counter (painted frames per wall second).
  int fps_frames = 0;
  long long fps_window_qpc = 0;
  int fps = 0;
  // Tick stamp of the last client-predicted swing arc (rate limit).
  std::uint64_t last_predicted_swing_tick = ~0ULL;
  // Trade/countinghouse pane interaction: keyboard cursor plus the exact
  // painted row rectangles for mouse hit-testing (rebuilt every frame).
  struct TradeRowHit {
    RECT rect{};
    int kind = 0;  // 0 = shop buy, 1 = bank withdraw, 2 = bank deposit
    std::size_t index = 0;
    std::string ref;   // item id (shop) or uuid (bank/deposit)
    int value = 0;     // price or qty
  };
  std::size_t trade_selected = 0;
  std::vector<TradeRowHit> trade_row_hits;
  // Character sheet (C) and passive-tree (P) panes.
  bool character_pane = false;
  bool tree_pane = false;
  struct TreeSeatHit {
    int x = 0;
    int y = 0;
    int radius = 0;
    std::string node_id;
    bool frontier = false;
  };
  std::vector<TreeSeatHit> tree_seat_hits;
  // Scene the current scenery set was generated for (remote path).
  std::string scenery_scene;
  FloorCache floor_cache;
  // Persistent double buffer: allocating a full-screen DIB every WM_PAINT
  // was a hidden ~19 MB alloc/free per frame at 3440x1440.
  HDC back_dc = nullptr;
  HBITMAP back_bitmap = nullptr;
  HGDIOBJ back_old = nullptr;
  int back_w = 0;
  int back_h = 0;
  // TASK-0157 audio, finally voiced: the deterministic mixer drains into a
  // waveOut synth sink each fixed tick. M toggles mute.
  std::unique_ptr<verdigris::audio::WaveOutSink> audio_sink;
  std::unique_ptr<verdigris::audio::AudioMixer> audio_mixer;
  // Borderless windowed-fullscreen is the default presentation; F11 drops
  // back to a movable window for side-by-side development.
  bool fullscreen_window = true;
  std::size_t selected_item = 0;
  std::string hint;
  int hint_ticks = 0;
  // TASK-0153 owner Esc contract: Escape closes an open dismissible pane
  // first; only a bare Escape (no pane/modal open) requests application
  // exit. The Win32 path posts the quit from this flag so the deterministic
  // scenario harness can exercise the identical production seam.
  bool quit_requested = false;
  int screen_pulse_ticks = 0;
  render::List render_list;
  Screen screen = Screen::Expedition;
  bool chronicles_mode = false;  // remote owner path launched at the front door
  bool chronicles_oath = false;  // mortal-oath choice applied to the next admission
  std::vector<ChronicleAction> chronicles_menu;
  std::string relic_toast;
  int relic_toast_ticks = 0;
  std::unordered_map<std::string, std::string> known_crypt_status;
  // TASK-0122 Phase A: optional world-anchored beat legend for the capture
  // proof composite. Empty in every normal play path.
  std::vector<std::pair<std::string, verdigris::Vec2>> beat_legend;
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

// Resolve and load the vendored WIZARD framekit chrome and item art. The
// wizard pack lives under native/client/assets/wizard; candidates cover the
// repo-root working directory and the build-directory executable.
void load_framekit_assets(BillboardAssets& assets) {
  std::vector<std::string> candidates;
  candidates.push_back("native/client/assets/wizard");
  const std::string executable = executable_directory();
  if (!executable.empty()) {
    candidates.push_back(executable + "/../client/assets/wizard");
    candidates.push_back(executable + "/../../client/assets/wizard");
    candidates.push_back(executable + "/assets/wizard");
  }
  for (const auto& root : candidates) {
    if (!directory_exists(root)) continue;
    const bool chrome_loaded =
        load_sprite(assets, root + "/framekit/textures/panel.png",
                    assets.fk_panel) &&
        load_sprite(assets, root + "/framekit/textures/slot.png",
                    assets.fk_slot);
    if (!chrome_loaded) {
      assets.fk_panel.reset();
      assets.fk_slot.reset();
      continue;
    }
    // Item art: server item id -> WIZARD sprite. Unmapped ids fall back to
    // the drawn cell; never map art that misrepresents the item.
    static constexpr struct { const char* item_id; const char* file; } kItemArt[] = {
        {"bronze-dagger", "dagger_bronze.png"},
        {"knife", "cur_knife.png"},
        {"bronze-sword", "boar_pike.png"},
        {"trophy", "bird_omen.png"},
    };
    for (const auto& entry : kItemArt) {
      SpriteBitmap& sprite = assets.item_art[entry.item_id];
      if (!load_sprite(assets, root + "/items/" + entry.file, sprite))
        sprite.reset();
    }
    return;
  }
}

// Nine-slice framekit blit through the TASK-0180 planner. Returns false when
// the plate is not loaded so callers can fall back to the vector skin.
bool draw_framekit_nine(const BillboardAssets& assets, HDC dc,
                        const SpriteBitmap& plate, const RECT& rect) {
  if (!plate.ready() || !assets.alpha_blend) return false;
  framekit_renderer::NineSliceAsset asset =
      &plate == &assets.fk_slot ? framekit_renderer::default_slot_asset()
                                : framekit_renderer::default_panel_asset();
  asset.source = {static_cast<std::uint16_t>(plate.width),
                  static_cast<std::uint16_t>(plate.height)};
  const framekit_renderer::Rect dest{
      static_cast<std::int16_t>(rect.left), static_cast<std::int16_t>(rect.top),
      static_cast<std::uint16_t>(rect.right - rect.left),
      static_cast<std::uint16_t>(rect.bottom - rect.top)};
  const framekit_renderer::NineSlicePlan plan =
      framekit_renderer::plan_nine_slice(dest, asset);
  if (!plan.valid) return false;
  const BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  for (const auto& region : plan.regions) {
    if (region.dst_w == 0 || region.dst_h == 0) continue;
    assets.alpha_blend(dc, region.dst_x, region.dst_y, region.dst_w,
                       region.dst_h, plate.dc, region.src_x, region.src_y,
                       region.src_w, region.src_h, blend);
  }
  return true;
}

// Aspect-fit alpha blit of an item sprite into a cell; false when unmapped.
bool draw_item_art(const BillboardAssets& assets, HDC dc, const std::string& id,
                   const RECT& cell) {
  const auto found = assets.item_art.find(id);
  if (found == assets.item_art.end() || !found->second.ready() ||
      !assets.alpha_blend)
    return false;
  const SpriteBitmap& sprite = found->second;
  const int cell_w = cell.right - cell.left;
  const int cell_h = cell.bottom - cell.top;
  const double scale =
      std::min(static_cast<double>(cell_w - 6) / sprite.width,
               static_cast<double>(cell_h - 6) / sprite.height);
  const int dest_w = std::max(1, static_cast<int>(sprite.width * scale));
  const int dest_h = std::max(1, static_cast<int>(sprite.height * scale));
  const BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  assets.alpha_blend(dc, cell.left + (cell_w - dest_w) / 2,
                     cell.top + (cell_h - dest_h) / 2, dest_w, dest_h,
                     sprite.dc, 0, 0, sprite.width, sprite.height, blend);
  return true;
}

void load_billboards(BillboardAssets& assets) {
  assets.msimg32_module = LoadLibraryA("msimg32.dll");
  assets.alpha_blend = reinterpret_cast<AlphaBlendProc>(
      assets.msimg32_module ? GetProcAddress(assets.msimg32_module, "AlphaBlend") : nullptr);
  if (!assets.alpha_blend || !initialize_gdiplus(assets)) {
    refresh_art_status(assets);
    return;
  }
  load_framekit_assets(assets);
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
  else if (state.session) {
    // scene.id only updates on transition envelopes; player.scene_id is
    // stamped by login and every snapshot, so it also covers the initial
    // town where no transition ever fires.
    route_id = state.session->model().player.scene_id;
    if (route_id.empty()) route_id = state.session->model().scene.id;
  }
  if (route_id.empty()) return;

  SceneryRng rng(scenery_seed(route_id));
  if (route_id.rfind("town:", 0) == 0) {
    // The Crossroads: landmarks anchored on the server's own contract
    // positions (fountain 38,115; Mara 49,103; Ludovicus 19,113; Rhea
    // 31,121; the spawn wagon 47,119) so every interaction point is a
    // visible thing. Non-solid: town collision is authoritatively open.
    const double t = kTileUnits;
    const double landmark_radius =
        static_cast<double>(verdigris::world_scale::kSceneryColliderRadius);
    add_scenery(state.scenery, SceneryKind::Shrine, 38.0 * t, 115.0 * t,
                landmark_radius * 1.4, false, 1.1);  // the fountain
    add_scenery(state.scenery, SceneryKind::Dwelling, 49.0 * t, 102.0 * t,
                landmark_radius * 1.6, false, 0.95);  // Mara's general stall
    add_scenery(state.scenery, SceneryKind::Dwelling, 19.0 * t, 112.0 * t,
                landmark_radius * 1.6, false, 0.95);  // Ludovicus' boards
    add_scenery(state.scenery, SceneryKind::Dwelling, 31.0 * t, 122.0 * t,
                landmark_radius * 1.6, false, 0.95);  // Rhea's countinghouse
    add_scenery(state.scenery, SceneryKind::Ruin, 48.0 * t, 120.0 * t,
                landmark_radius * 1.4, false, 0.9);  // the House wagon
    // A loose ring of trees frames the market square without crowding it.
    add_scenery(state.scenery, SceneryKind::Tree, 26.0 * t, 108.0 * t,
                landmark_radius, false, 1.1);
    add_scenery(state.scenery, SceneryKind::Tree, 44.0 * t, 111.0 * t,
                landmark_radius, false, 0.9);
    add_scenery(state.scenery, SceneryKind::Tree, 35.0 * t, 124.0 * t,
                landmark_radius, false, 1.0);
    add_scenery(state.scenery, SceneryKind::Tree, 52.0 * t, 116.0 * t,
                landmark_radius, false, 1.05);
    add_scenery(state.scenery, SceneryKind::Tree, 22.0 * t, 119.0 * t,
                landmark_radius, false, 0.85);
    return;
  }
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
  if (state.simulation) {
    verdigris::client::sync_world_from_simulation(state.world, *state.simulation);
    return;
  }
  if (!state.session) return;
  const auto& model = state.session->model();
  verdigris::client::sync_world_from_model(state.world, model);
  // Authoritative loot placement: the server snapshot carries every ground
  // item's real position; the event-scatter heuristic anchored on the last
  // death and piled every drop onto one spot. A small deterministic per-uuid
  // fan keeps same-tile stacks readable without moving the real item.
  state.loot_positions.clear();
  for (const auto& item : model.ground) {
    std::uint32_t hash = 2166136261u;
    for (const char c : item.uuid)
      hash = (hash ^ static_cast<std::uint8_t>(c)) * 16777619u;
    const double jitter_x =
        (static_cast<double>(hash % 100u) / 100.0 - 0.5) * 0.6;
    const double jitter_y =
        (static_cast<double>((hash / 100u) % 100u) / 100.0 - 0.5) * 0.6;
    state.loot_positions[item.uuid] = {
        static_cast<int>(std::lround(
            verdigris::client::protocol_to_world(item.x + jitter_x))),
        static_cast<int>(std::lround(
            verdigris::client::protocol_to_world(item.y + jitter_y)))};
  }
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
  fx.known_monsters = std::move(state.known_monsters);
  ++state.world.tick;
  if (!state.audio_mixer) {
    state.audio_sink = std::make_unique<verdigris::audio::WaveOutSink>();
    state.audio_mixer =
        std::make_unique<verdigris::audio::AudioMixer>(*state.audio_sink);
  }
  const std::string route_before = state.world.route_id;
  for (const auto& event : state.session->drain_events()) {
    verdigris::client::apply_presentation_event(fx, state.world, event, state.world.tick);
    state.audio_mixer->ingest(event, state.world.tick);
    if (!fx.hint.empty()) {
      state.hint = fx.hint;
      state.hint_ticks = fx.hint_ticks;
    }
  }
  state.audio_mixer->drain_scheduled();
  sync_world(state);
  if (state.world.route_id != route_before) generate_scenery(state);
  // TASK-0122 Phase A: deterministic first-sighting spawn beats on the
  // authoritative remote snapshot.
  verdigris::client::detect_monster_spawns(fx, state.world, state.world.tick);
  state.effects = std::move(fx.effects);
  state.telegraphs = std::move(fx.telegraphs);
  state.loot_positions = std::move(fx.loot_positions);
  state.last_death_pos = fx.last_death_pos;
  state.loot_scatter = fx.loot_scatter;
  state.screen_pulse_ticks = fx.screen_pulse_ticks;
  state.event_log = std::move(fx.event_log);
  state.known_monsters = std::move(fx.known_monsters);
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
  // Instant swing feedback on the remote path: the authoritative damage
  // still round-trips, but the arc itself must not wait on the wire (a whiff
  // that draws nothing reads as a dead input). Local play already gets its
  // arc from the core's own events in the same frame.
  if (is_remote(state) &&
      (skill.action == verdigris::ActionType::Melee ||
       skill.action == verdigris::ActionType::Thrust ||
       skill.action == verdigris::ActionType::Sweep)) {
    sync_world(state);
    const auto& player = state.world.player;
    // One predicted arc per presentation tick: input can arrive far faster
    // than frames (auto-clickers, synthetic floods), and an unbounded
    // effects vector is a frame-time leak. The authoritative attack rate is
    // enforced server-side either way.
    if (player.alive && state.world.tick != state.last_predicted_swing_tick &&
        state.effects.size() < 128) {
      state.last_predicted_swing_tick = state.world.tick;
      EffectFx arc;
      arc.kind = skill.action == verdigris::ActionType::Sweep ? EffectFx::Kind::SweepArc
                                                              : EffectFx::Kind::Swing;
      arc.wx = static_cast<double>(player.position.x);
      arc.wy = static_cast<double>(player.position.y);
      arc.angle = std::atan2(static_cast<double>(player.facing.y),
                             static_cast<double>(player.facing.x));
      arc.ttl = 6;
      state.effects.push_back(arc);
    }
  }
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
  {
    sync_world(state);
    const auto& items = state.world.carried;
    if (!items.empty()) {
      const std::size_t pick = std::min(state.selected_item, items.size() - 1);
      std::string lowered = items[pick].name;
      for (auto& ch : lowered)
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
      if (lowered.find("coin") != std::string::npos) {
        show_hint(state, "Coins spend; they do not equip");
        return;
      }
    }
  }

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
                const RECT& bounds, const std::string& route_id, render::List& rl,
                FloorCache* cache = nullptr) {
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
  // Clip the tile loop to what the window can actually show (plus a one-tile
  // margin), bounded by the historical +-arena envelope. At fullscreen zoom
  // the arena box is far larger than the viewport, and unclipped StretchBlts
  // for off-screen tiles were most of the frame cost.
  const double half_w_units =
      (static_cast<double>(bounds.right) * 0.5) / std::max(0.05, camera.zoom) +
      tile;
  const double half_h_units =
      (static_cast<double>(bounds.bottom) * 0.5) / std::max(0.05, camera.zoom) +
      tile;
  const double span_x = std::min(range, half_w_units);
  const double span_y = std::min(range, half_h_units);
  const int start_tx = static_cast<int>(std::floor((camera.x - span_x) / tile));
  const int end_tx = static_cast<int>(std::ceil((camera.x + span_x) / tile));
  const int start_ty = static_cast<int>(std::floor((camera.y - span_y) / tile));
  const int end_ty = static_cast<int>(std::ceil((camera.y + span_y) / tile));
  const bool theme_alt = terrain_theme_prefers_alt(route_id);
  const double half = tile * 0.5;

  // Semantic ops are recorded per frame regardless of the pixel path so the
  // scenario harness sees the identical vocabulary either way.
  for (int ty = start_ty; ty <= end_ty; ++ty) {
    for (int tx = start_tx; tx <= end_tx; ++tx) {
      const double wx = static_cast<double>(tx) * tile;
      const double wy = static_cast<double>(ty) * tile;
      const ScreenPoint center = project(camera, bounds, wx + half, wy + half);
      const bool use_alt = terrain_tile_uses_alt(tx, ty, theme_alt);
      const std::string label =
          std::string(use_alt ? "terrain4" : "terrain1") + ":" + std::to_string(tx) + ":" +
          std::to_string(ty);
      rl.push_back({render::Op::Tile, static_cast<double>(center.x),
                    static_cast<double>(center.y), half * center.scale, 0, label});
    }
  }

  // The per-tile pixel painter, shared by the direct and cached paths. The
  // target camera/bounds pair defines the affine frame tiles project into;
  // adjacent tiles share projected corners exactly (0075 rev2), so seams
  // never open in either frame.
  const auto paint_tiles = [&](HDC target, const Camera& cam, const RECT& frame,
                               int a_tx, int a_ty, int b_tx, int b_ty) {
    for (int ty = a_ty; ty <= b_ty; ++ty) {
      for (int tx = a_tx; tx <= b_tx; ++tx) {
        const double wx = static_cast<double>(tx) * tile;
        const double wy = static_cast<double>(ty) * tile;
        const ScreenPoint corner0 = project(cam, frame, wx, wy);
        const ScreenPoint corner1 = project(cam, frame, wx + tile, wy + tile);
        const bool use_alt = terrain_tile_uses_alt(tx, ty, theme_alt);
        if (tiled) {
          const SpriteBitmap& sprite = use_alt ? assets.terrain4 : assets.terrain1;
          draw_terrain_tile(target, sprite, corner0.x, corner0.y,
                            corner1.x - corner0.x, corner1.y - corner0.y,
                            terrain_tile_hash(tx, ty) >> 8);
        } else if (vector_tiled) {
          const kit::Symbol& motif = use_alt ? *motif_alt : *motif_primary;
          const int dest_w = corner1.x - corner0.x;
          const int dest_h = corner1.y - corner0.y;
          if (dest_w > 0 && dest_h > 0) {
            const double scale = static_cast<double>(dest_w) /
                                 static_cast<double>(motif.width);
            KitPlacement placement{target, static_cast<double>(corner0.x),
                                   static_cast<double>(corner0.y), scale,
                                   static_cast<double>(motif.width), false};
            for (int i = motif.shape_begin; i < motif.shape_end; ++i)
              draw_kit_shape(placement, kit::kShapes[i]);
          }
        }
      }
    }
  };

  if (!cache) {
    paint_tiles(dc, camera, bounds, start_tx, start_ty, end_tx, end_ty);
    return;
  }

  const bool range_covered = cache->valid && start_tx >= cache->tx0 &&
                             end_tx <= cache->tx1 && start_ty >= cache->ty0 &&
                             end_ty <= cache->ty1;
  if (!range_covered || cache->zoom != camera.zoom ||
      cache->route != route_id ||
      cache->view_w != static_cast<int>(bounds.right) ||
      cache->view_h != static_cast<int>(bounds.bottom)) {
    // Rebuild around the current view with a one-tile skirt so small camera
    // moves stay inside the cached region.
    const int margin = 1;
    const int c_tx0 = start_tx - margin;
    const int c_ty0 = start_ty - margin;
    const int c_tx1 = end_tx + margin;
    const int c_ty1 = end_ty + margin;
    const int need_w = static_cast<int>(
        std::ceil((c_tx1 - c_tx0 + 1) * tile * camera.zoom)) + 4;
    const int need_h = static_cast<int>(
        std::ceil((c_ty1 - c_ty0 + 1) * tile * camera.zoom)) + 4;
    if (!cache->dc || cache->width < need_w || cache->height < need_h) {
      cache->release();
      HDC screen_dc = GetDC(nullptr);
      cache->dc = CreateCompatibleDC(screen_dc);
      cache->bitmap = CreateCompatibleBitmap(screen_dc, need_w, need_h);
      ReleaseDC(nullptr, screen_dc);
      if (!cache->dc || !cache->bitmap) {
        cache->release();
        paint_tiles(dc, camera, bounds, start_tx, start_ty, end_tx, end_ty);
        return;
      }
      cache->old_bitmap = SelectObject(cache->dc, cache->bitmap);
      cache->width = need_w;
      cache->height = need_h;
    }
    // Cache frame: same zoom, camera centred on the cached world region.
    Camera cache_cam = camera;
    cache_cam.x = (static_cast<double>(c_tx0) + (c_tx1 - c_tx0 + 1) * 0.5) * tile;
    cache_cam.y = (static_cast<double>(c_ty0) + (c_ty1 - c_ty0 + 1) * 0.5) * tile;
    RECT cache_frame{0, 0, cache->width, cache->height};
    HBRUSH backing = CreateSolidBrush(RGB(23, 29, 32));
    FillRect(cache->dc, &cache_frame, backing);
    DeleteObject(backing);
    paint_tiles(cache->dc, cache_cam, cache_frame, c_tx0, c_ty0, c_tx1, c_ty1);
    cache->tx0 = c_tx0;
    cache->ty0 = c_ty0;
    cache->tx1 = c_tx1;
    cache->ty1 = c_ty1;
    cache->zoom = camera.zoom;
    cache->route = route_id;
    cache->view_w = static_cast<int>(bounds.right);
    cache->view_h = static_cast<int>(bounds.bottom);
    cache->valid = true;
  }

  // Blit alignment: both frames share the zoom, so aligning any one world
  // point aligns every tile. Anchor on the cached region's north-west tile
  // corner, computed in doubles and rounded once.
  {
    const double anchor_wx = static_cast<double>(cache->tx0) * tile;
    const double anchor_wy = static_cast<double>(cache->ty0) * tile;
    // Integer half-extents to match camera2d::project exactly.
    const double sx = (anchor_wx - camera.x) * camera.zoom +
                      static_cast<double>(bounds.right / 2);
    const double sy = (anchor_wy - camera.y) * camera.zoom +
                      static_cast<double>(bounds.bottom / 2);
    Camera cache_cam = camera;
    cache_cam.x = (static_cast<double>(cache->tx0) +
                   (cache->tx1 - cache->tx0 + 1) * 0.5) * tile;
    cache_cam.y = (static_cast<double>(cache->ty0) +
                   (cache->ty1 - cache->ty0 + 1) * 0.5) * tile;
    const double cx = (anchor_wx - cache_cam.x) * camera.zoom +
                      static_cast<double>(cache->width / 2);
    const double cy = (anchor_wy - cache_cam.y) * camera.zoom +
                      static_cast<double>(cache->height / 2);
    const int dest_x = static_cast<int>(std::lround(sx - cx));
    const int dest_y = static_cast<int>(std::lround(sy - cy));
    BitBlt(dc, dest_x, dest_y, cache->width, cache->height, cache->dc, 0, 0,
           SRCCOPY);
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
      std::string damage_label = fx.damage_to_player ? "player" : "monster";
      const COLORREF base_color =
          fx.critical ? RGB(phase_a::kCriticalNumberColor.r, phase_a::kCriticalNumberColor.g,
                            phase_a::kCriticalNumberColor.b)
                      : (fx.damage_to_player ? RGB(255, 118, 104) : RGB(240, 218, 132));
      if (fx.critical)
        damage_label = std::string(phase_a::kCriticalDamageLabel) + ":" +
                       (fx.style.empty() ? "slash" : fx.style);
      rl.push_back({render::Op::Damage, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, fx.value, damage_label});
      // Critical hits rise higher and read larger for their longer lifetime;
      // ordinary hits keep the accepted TASK-0142 treatment.
      const int lift = static_cast<int>(kTileUnits *
                                        (0.35 + grow * (fx.critical ? 1.05 : 0.75)) *
                                        base.scale);
      const COLORREF color = base_color;
      SetBkMode(dc, TRANSPARENT);
      // TASK-0142: bold numerals so the resolved damage reads instantly.
      const int font_h = std::clamp(
          static_cast<int>(kTileUnits * (fx.critical ? 0.44 : 0.34) * base.scale),
          fx.critical ? 16 : 13, fx.critical ? 26 : 22);
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
      if (fx.critical) {
        // A short white-hot burst cross behind the numeral separates the
        // critical beat from every ordinary hit flash.
        const COLORREF burst = fade_to_background(
            RGB(phase_a::kCriticalFlashColor.r, phase_a::kCriticalFlashColor.g,
                phase_a::kCriticalFlashColor.b),
            life);
        const int arm = std::max(6, static_cast<int>(kTileUnits * 0.5 * base.scale));
        draw_line(dc, base.x - arm, base.y - arm, base.x + arm, base.y + arm, burst, 2);
        draw_line(dc, base.x - arm, base.y + arm, base.x + arm, base.y - arm, burst, 2);
      }
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
    case EffectFx::Kind::Materialize: {
      // TASK-0122 Phase A: a deterministic materialization beat — two teal
      // rings collapsing onto the spawn point. Radii stay inside the foe's
      // own footprint neighborhood so an adjacent spawn never blankets the
      // player, the objective strip, or the exit controls.
      rl.push_back({render::Op::TargetFlash, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, 0, phase_a::kSpawnRenderLabel});
      const COLORREF color = fade_to_background(
          RGB(phase_a::kMaterializeColor.r, phase_a::kMaterializeColor.g,
              phase_a::kMaterializeColor.b),
          life);
      const int outer = std::max(
          4, static_cast<int>(kTileUnits * (0.5 - grow * 0.36) * base.scale));
      const int inner = std::max(2, outer / 2);
      ring_ellipse(dc, base.x, base.y, outer, outer, color, 2);
      ring_ellipse(dc, base.x, base.y, inner, inner,
                   fade_to_background(
                       RGB(phase_a::kMaterializeColor.r, phase_a::kMaterializeColor.g,
                           phase_a::kMaterializeColor.b),
                       life * 0.6),
                   1);
      break;
    }
    case EffectFx::Kind::WarCryFade: {
      // TASK-0122 Phase A: the BuffExpired contract beat — an imploding
      // dimmed-gold ring, clearly unlike the bright expanding apply aura.
      rl.push_back({render::Op::WarCry, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, 0, phase_a::kWarcryFadeLabel});
      const COLORREF color = fade_to_background(
          RGB(phase_a::kWarcryFadeColor.r, phase_a::kWarcryFadeColor.g,
              phase_a::kWarcryFadeColor.b),
          life);
      const int radius =
          static_cast<int>(kTileUnits * (0.9 - grow * 0.62) * base.scale);
      ring_ellipse(dc, base.x, base.y, (std::max)(3, radius), (std::max)(3, radius),
                   color, 2);
      break;
    }
    case EffectFx::Kind::ScionLostBeat: {
      // TASK-0122 Phase A: the longest beat in the packet — slow double rust
      // rings collapsing inward with a falling shimmer. Distinct from every
      // death ring in color, motion, and lifetime.
      rl.push_back({render::Op::ScreenPulse, 0.0, 0.0, 0.0, 0,
                    phase_a::kScionLostLabel});
      const COLORREF color = fade_to_background(
          RGB(phase_a::kScionLostColor.r, phase_a::kScionLostColor.g,
              phase_a::kScionLostColor.b),
          life);
      const int outer = std::max(
          4, static_cast<int>(kTileUnits * (1.2 - grow * 0.95) * base.scale));
      ring_ellipse(dc, base.x, base.y, outer, outer, color, 3);
      ring_ellipse(dc, base.x, base.y, (std::max)(2, outer / 2),
                   (std::max)(2, outer / 2),
                   fade_to_background(
                       RGB(phase_a::kScionLostColor.r, phase_a::kScionLostColor.g,
                           phase_a::kScionLostColor.b),
                       life * 0.7),
                   1);
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
    // TASK-0122 Phase A: BuffExpired/ScionLost log their own readable beat
    // lines ("war cry faded", "scion lost"); no generic label here.
    // TASK-0153: the core's authoritative phase transition is no longer
    // dropped; the event log carries the core's own phase tokens.
    case EventType::ExpeditionPhaseChanged: return "phase";
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
      case verdigris::EventType::BuffExpired:
        // TASK-0122 Phase A: war-cry end contract beat. Imploding dimmed-gold
        // ring at the anchor, lifetime from the phase_a constants table.
        if (event.text.empty() || event.text == "war-cry") {
          state.effects.push_back({EffectFx::Kind::WarCryFade, ex, ey, 0.0, 0,
                                   phase_a::kWarcryFadeTtlTicks});
          state.event_log.push_back("war cry faded");
          if (state.event_log.size() > 6) state.event_log.erase(state.event_log.begin());
        }
        break;
      case verdigris::EventType::ScionLost:
        // TASK-0122 Phase A: long somber loss beat; clears stale warnings and
        // pulses the screen edge exactly like the seam path does.
        state.telegraphs.clear();
        state.effects.push_back({EffectFx::Kind::ScionLostBeat, ex, ey, 0.0, 0,
                                 phase_a::kScionLostRingTtlTicks});
        state.screen_pulse_ticks = phase_a::kScionLostPulseTicks;
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

  // TASK-0122 Phase A: deterministic first-sighting spawn beats over the
  // authoritative local snapshot. Presentation bookkeeping only.
  sync_world(state);
  verdigris::client::PresentationFx spawn_fx;
  spawn_fx.effects = std::move(state.effects);
  spawn_fx.known_monsters = std::move(state.known_monsters);
  detect_monster_spawns(spawn_fx, state.world, sim.tick());
  state.effects = std::move(spawn_fx.effects);
  state.known_monsters = std::move(spawn_fx.known_monsters);
}

struct DepthDraw {
  double depth = 0.0;
  int order = 0;
  enum class What { Scenery, Player, Monster, Npc, Loot, Effect } what = What::Player;
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
  const int width = extent.cx + 20;
  const int height = extent.cy + 10;
  RECT rect{x, y, x + width, y + height};
  skin::chip(dc, rect, accent);
  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, accent);
  TextOutA(dc, x + 12, y + 5, text.c_str(), static_cast<int>(text.size()));
  rl.push_back({render::Op::Hud, static_cast<double>(x), static_cast<double>(y),
                0.0, 0, text});
  return width;
}

std::string loot_label(const ClientState& state, const std::string& id) {
  auto found = state.world.loot_names.find(id);
  if (found != state.world.loot_names.end()) return found->second;
  return id;
}

void paint_gear_overlay(ClientState& state, HDC dc, const RECT& bounds,
                        render::List& rl) {
  if (!state.gear_overlay) return;
  // TASK-0159: the pane rectangle comes from the shared pure geometry so the
  // planner, painter, and scenario harness cannot drift apart.
  const HudRect pane = gear_pane_rect(static_cast<int>(bounds.right),
                                      static_cast<int>(bounds.bottom));
  const int left = pane.x;
  const int top = pane.y;
  const int right = left + pane.w;
  const int bottom = top + pane.h;
  state.hud_rect_trace.push_back({"pane-frame", pane});

  RECT panel_rect{left, top, right, bottom};
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_panel,
                          panel_rect))
    skin::panel(dc, panel_rect, skin::kVerdigris, 245, 8.0f);

  // Interior layout scale: the pane rect scales with window height, so every
  // hand-authored 1x offset inside must scale with it or rows collide.
  const int s = hud_scale(static_cast<int>(bounds.bottom));

  SetBkMode(dc, TRANSPARENT);

  // Title.
  SetTextColor(dc, RGB(230, 235, 220));
  // house().name is already prefixed ("House Verdigris"); do not double it.
  const std::string title = "Gear / " + state.world.house_name;
  {
    SIZE extent{};
    GetTextExtentPoint32A(dc, title.c_str(), static_cast<int>(title.size()),
                          &extent);
    state.hud_rect_trace.push_back(
        {"pane-title", {left + 14 * s, top + 12 * s, extent.cx, extent.cy}});
  }
  TextOutA(dc, left + 14 * s, top + 12 * s, title.c_str(),
           static_cast<int>(title.size()));

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
  {
    SIZE extent{};
    GetTextExtentPoint32A(dc, stats_line.c_str(),
                          static_cast<int>(stats_line.size()), &extent);
    state.hud_rect_trace.push_back(
        {"pane-stats", {left + 14 * s, top + 38 * s, extent.cx, extent.cy}});
  }
  TextOutA(dc, left + 14 * s, top + 38 * s, stats_line.c_str(),
           static_cast<int>(stats_line.size()));

  // Weapon (paperdoll) seat.
  const int seat_top = top + 62 * s;
  const int seat_left = left + 14 * s;
  const int seat_w = right - left - 28 * s;
  RECT seat{seat_left, seat_top, seat_left + seat_w, seat_top + 24 * s};
  state.hud_rect_trace.push_back(
      {"pane-seat", {seat.left, seat.top, seat_w, 24 * s}});
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
  TextOutA(dc, seat_left + 6 * s, seat_top + 4 * s, seat_label,
           static_cast<int>(strlen(seat_label)));
  std::string equipped_name = "(empty)";
  for (const auto& item : items)
    if (item.equipped) {
      equipped_name = item.name;
      break;
    }
  SetTextColor(dc, RGB(230, 220, 180));
  rl.push_back({render::Op::PaneWeapon, 0.0, 0.0, 0.0, 0, equipped_name});
  TextOutA(dc, seat_left + 96 * s, seat_top + 4 * s, equipped_name.c_str(),
           static_cast<int>(equipped_name.size()));

  // Grid backpack (4 columns), framekit slot chrome with item art.
  constexpr int kGridColumns = 4;
  const int cell_w =
      (right - left - (28 + (kGridColumns - 1) * 6) * s) / kGridColumns;
  const int cell_h = 56 * s;
  const int grid_top = seat_top + 38 * s;
  // On the remote path carried ids are uuids; the model's inventory rows
  // carry the stable item id the art catalog is keyed by.
  const auto art_key = [&](std::size_t index) -> std::string {
    if (!state.session) return items[index].id;
    for (const auto& slot_item : state.session->model().inventory)
      if (slot_item.uuid == items[index].id) return slot_item.id;
    return items[index].id;
  };
  if (items.empty()) {
    SetTextColor(dc, RGB(150, 160, 150));
    const char* empty = "Backpack empty. X picks up the nearest drop.";
    TextOutA(dc, left + 14 * s, grid_top + 6 * s, empty,
             static_cast<int>(strlen(empty)));
  } else {
    for (std::size_t i = 0; i < items.size(); ++i) {
      const int col = static_cast<int>(i % kGridColumns);
      const int row = static_cast<int>(i / kGridColumns);
      const int cx = left + 14 * s + col * (cell_w + 6 * s);
      const int cy = grid_top + row * (cell_h + 6 * s);
      const bool selected = i == std::min(state.selected_item, items.size() - 1);
      const bool equipped = items[i].equipped;
      RECT cell{cx, cy, cx + cell_w, cy + cell_h};
      if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot,
                              cell))
        skin::slot(dc, cell, equipped ? skin::kGold : skin::kVerdigris,
                   selected);
      if (selected || equipped) {
        // Selection/equip read on top of the raster chrome.
        HPEN cell_pen = CreatePen(PS_SOLID, 2,
                                  equipped ? RGB(210, 180, 90) : RGB(120, 214, 168));
        HGDIOBJ cp = SelectObject(dc, cell_pen);
        HGDIOBJ cb = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
        Rectangle(dc, cell.left, cell.top, cell.right, cell.bottom);
        SelectObject(dc, cb);
        SelectObject(dc, cp);
        DeleteObject(cell_pen);
      }
      RECT art_cell{cell.left, cell.top, cell.right, cell.bottom - 18 * s};
      {
        // Parchment backing: bronze-age sprites are dark; without a light
        // ground they vanish into the slot texture.
        RECT backing{art_cell.left + 4 * s, art_cell.top + 4 * s,
                     art_cell.right - 4 * s, art_cell.bottom};
        HBRUSH backing_brush = CreateSolidBrush(RGB(74, 82, 78));
        FillRect(dc, &backing, backing_brush);
        DeleteObject(backing_brush);
      }
      draw_item_art(state.billboards, dc, art_key(i), art_cell);
      SetTextColor(dc, equipped ? RGB(240, 210, 120) : RGB(205, 215, 204));
      std::string name = items[i].name;
      if (name.size() > 12) name = name.substr(0, 11) + ".";
      rl.push_back({render::Op::PaneItem, static_cast<double>(cx),
                    static_cast<double>(cy), 0.0, items[i].attack_bonus,
                    equipped ? name + " [E]" : name});
      state.hud_rect_trace.push_back(
          {"pane-cell", {cx, cy, cell_w, cell_h}});
      HGDIOBJ cell_font = SelectObject(dc, skin::font_small());
      TextOutA(dc, cx + 4 * s, cell.bottom - 17 * s, name.c_str(),
               static_cast<int>(name.size()));
      SetTextColor(dc, RGB(170, 185, 172));
      std::string bonus = "+" + std::to_string(items[i].attack_bonus) +
                          (equipped ? " [E]" : "");
      SIZE bonus_extent{};
      GetTextExtentPoint32A(dc, bonus.c_str(), static_cast<int>(bonus.size()),
                            &bonus_extent);
      TextOutA(dc, cell.right - bonus_extent.cx - 4 * s, cell.top + 2 * s,
               bonus.c_str(), static_cast<int>(bonus.size()));
      SelectObject(dc, cell_font);
    }
  }

  // Banked / extraction summary.
  SetTextColor(dc, RGB(150, 170, 158));
  const std::string banked =
      "Banked  items " + std::to_string(state.world.stored_items) +
      "  trophies " + std::to_string(state.world.stored_trophies);
  rl.push_back({render::Op::PaneBanked, 0.0, 0.0, 0.0, 0, banked});
  {
    SIZE extent{};
    GetTextExtentPoint32A(dc, banked.c_str(), static_cast<int>(banked.size()),
                          &extent);
    state.hud_rect_trace.push_back(
        {"pane-banked", {left + 14 * s, bottom - 50 * s, extent.cx, extent.cy}});
  }
  TextOutA(dc, left + 14 * s, bottom - 50 * s, banked.c_str(),
           static_cast<int>(banked.size()));
  // TASK-0156: compact authoritative progression summary, mirrored from the
  // passiveTree payload. Absence is stated as absence — never rendered as
  // zero — and no node ids, allocation actions, or invented copy appear.
  std::string progression;
  if (state.world.progression.present) {
    progression = "TREE pts " +
                  std::to_string(state.world.progression.unspent_points) + "/" +
                  std::to_string(state.world.progression.earned_points) +
                  "  nodes " + std::to_string(state.world.progression.node_count) +
                  "  conduits " +
                  std::to_string(state.world.progression.conduit_count);
  } else {
    progression = "TREE no authoritative data";
  }
  rl.push_back({render::Op::PaneStat, 0.0, 0.0, 0.0, 0, progression});
  {
    SIZE extent{};
    GetTextExtentPoint32A(dc, progression.c_str(),
                          static_cast<int>(progression.size()), &extent);
    state.hud_rect_trace.push_back(
        {"pane-progression", {left + 14 * s, bottom - 74 * s, extent.cx, extent.cy}});
  }
  TextOutA(dc, left + 14 * s, bottom - 74 * s, progression.c_str(),
           static_cast<int>(progression.size()));
  const char* controls = "Arrows select | Enter equip | U unequip | I close";
  {
    SIZE extent{};
    GetTextExtentPoint32A(dc, controls, static_cast<int>(strlen(controls)),
                          &extent);
    state.hud_rect_trace.push_back(
        {"pane-footer", {left + 14 * s, bottom - 26 * s, extent.cx, extent.cy}});
  }
  TextOutA(dc, left + 14 * s, bottom - 26 * s, controls,
           static_cast<int>(strlen(controls)));
}

void draw_orb(HDC dc, int cx, int cy, int radius, double ratio, COLORREF fill,
              COLORREF rim, const std::string& caption, bool pulse, render::List& rl,
              const char* label) {
  const int bounded = static_cast<int>(std::clamp(ratio, 0.0, 1.0) * 100.0);
  rl.push_back({render::Op::Orb, static_cast<double>(cx), static_cast<double>(cy),
                static_cast<double>(radius), bounded, label});
  // Skinned glass sphere: `fill` is the deep liquid tone, `rim` doubles as
  // the bright gradient head so existing call sites keep their palette.
  skin::orb(dc, cx, cy, radius, ratio, fill, rim, rim, pulse);
  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, skin::kInk);
  HGDIOBJ old_font = SelectObject(dc, skin::font_body_bold());
  SIZE caption_extent{};
  GetTextExtentPoint32A(dc, caption.c_str(), static_cast<int>(caption.size()),
                        &caption_extent);
  TextOutA(dc, cx - caption_extent.cx / 2, cy - caption_extent.cy / 2,
           caption.c_str(), static_cast<int>(caption.size()));
  SelectObject(dc, old_font);
}

void paint_vital_orbs(const WorldActor& player, std::uint64_t tick, int screen_pulse_ticks,
                      HDC dc, const RECT& bounds, render::List& rl,
                      std::vector<std::pair<std::string, HudRect>>* trace) {
  if (!player.alive && player.life <= 0 && player.life_max <= 0) return;
  const int radius =
      kVitalOrbRadius * hud_scale(static_cast<int>(bounds.bottom));
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
  // TASK-0159: record the exact painted orb extents (+pulse ring) so the
  // readability scenario can prove the pane never reaches into them.
  if (trace) {
    trace->push_back({"orb-life", vital_orb_rect(static_cast<int>(bounds.right),
                                                 static_cast<int>(bounds.bottom),
                                                 false)});
    trace->push_back({"orb-resource",
                      vital_orb_rect(static_cast<int>(bounds.right),
                                     static_cast<int>(bounds.bottom), true)});
  }
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

static_assert(kQuickbarSlotCount ==
                  sizeof(kQuickbarSlots) / sizeof(kQuickbarSlots[0]),
              "quickbar geometry helper and painter must agree on slot count");

void paint_quickbar(ClientState& state, HDC dc, const RECT& bounds, render::List& rl) {
  const WorldActor& player = state.world.player;
  const verdigris::PresentationCatalog catalog =
      verdigris::Simulation::presentation_catalog();
  const int s = hud_scale(static_cast<int>(bounds.bottom));
  const int slot_w = 58 * s;
  const int slot_h = 52 * s;
  const int gap = 8 * s;
  const int count = static_cast<int>(sizeof(kQuickbarSlots) / sizeof(kQuickbarSlots[0]));
  const int strip_w = count * slot_w + (count - 1) * gap;
  const int bottom = static_cast<int>(bounds.bottom) - 18;
  const int top = bottom - slot_h;
  const int left = (static_cast<int>(bounds.right) - strip_w) / 2;

  RECT strip{left - 10, top - 8, left + strip_w + 10, bottom + 4};
  state.hud_rect_trace.push_back(
      {"quickbar-strip",
       {strip.left, strip.top, strip.right - strip.left,
        strip.bottom - strip.top}});
  skin::panel(dc, strip);

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
    skin::slot(dc, box, active ? skin::kGold : skin::kVerdigris,
               available || active);

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
    TextOutA(dc, box.left + 6 * s, box.top + 4 * s, slot.key_label,
             static_cast<int>(strlen(slot.key_label)));
    SetTextColor(dc, available ? RGB(205, 221, 207) : RGB(112, 119, 115));
    TextOutA(dc, box.left + 6 * s, box.top + 22 * s, slot.name,
             static_cast<int>(strlen(slot.name)));
  }
}

void paint_minimap(ClientState& state, HDC dc, const RECT& bounds, render::List& rl) {
  const HudRect map = minimap_rect(static_cast<int>(bounds.bottom));
  const int kSize = map.w;
  const int s = std::max(1, map.w / 108);
  RECT panel{map.x, map.y, map.x + map.w, map.y + map.h};
  state.hud_rect_trace.push_back({"minimap", map});
  skin::panel(dc, panel);

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
    fill_ellipse(dc, mx, my, 2 * s, 2 * s, RGB(96, 112, 98));
    ++dots;
  }

  if (world.has_extraction) {
    const auto [mx, my] = to_map(world.extraction.x, world.extraction.y);
    fill_ellipse(dc, mx, my, 4 * s, 4 * s, RGB(239, 208, 116));
    ++dots;
  }

  for (const auto& monster : world.monsters) {
    if (!monster.alive) continue;
    const auto [mx, my] = to_map(monster.position.x, monster.position.y);
    fill_ellipse(dc, mx, my, 3 * s, 3 * s, RGB(196, 58, 48));
    ++dots;
  }

  for (const auto& npc : world.npcs) {
    // NPC blips clamp to the panel border so an off-map NPC still points
    // the way — the town is far larger than the minimap window.
    auto [mx, my] = to_map(npc.position.x, npc.position.y);
    mx = std::clamp(mx, static_cast<int>(panel.left) + 3,
                    static_cast<int>(panel.right) - 3);
    my = std::clamp(my, static_cast<int>(panel.top) + 3,
                    static_cast<int>(panel.bottom) - 3);
    fill_ellipse(dc, mx, my, 3 * s, 3 * s, RGB(122, 168, 230));
    ++dots;
  }

  const auto [px, py] = to_map(origin_x, origin_y);
  const double facing_angle =
      std::atan2(world.player.facing.y, world.player.facing.x);
  const int tip_x = px + static_cast<int>(std::cos(facing_angle) * 8.0 * s);
  const int tip_y = py + static_cast<int>(std::sin(facing_angle) * 8.0 * s);
  const int wing_x = px - static_cast<int>(std::cos(facing_angle) * 4.0 * s);
  const int wing_y = py - static_cast<int>(std::sin(facing_angle) * 4.0 * s);
  const double wing = facing_angle + kPi * 0.75;
  const int wing_a_x = wing_x + static_cast<int>(std::cos(wing) * 5.0 * s);
  const int wing_a_y = wing_y + static_cast<int>(std::sin(wing) * 5.0 * s);
  const int wing_b_x = wing_x + static_cast<int>(std::cos(wing + kPi * 0.5) * 5.0 * s);
  const int wing_b_y = wing_y + static_cast<int>(std::sin(wing + kPi * 0.5) * 5.0 * s);
  POINT arrow[3] = {{tip_x, tip_y}, {wing_a_x, wing_a_y}, {wing_b_x, wing_b_y}};
  HBRUSH player_brush = CreateSolidBrush(RGB(168, 214, 188));
  HGDIOBJ old_arrow_brush = SelectObject(dc, player_brush);
  Polygon(dc, arrow, 3);
  SelectObject(dc, old_arrow_brush);
  DeleteObject(player_brush);

  rl.push_back({render::Op::Minimap, static_cast<double>(panel.left),
                static_cast<double>(panel.top), static_cast<double>(kSize), dots, "panel"});
}

// ── TASK-0145: Chronicles owner journey ─────────────────────────────────
// The front door is the default remote owner path: a coherent pre-game
// screen over the accepted Gate-B envelopes. Everything here renders the
// authoritative ClientModel; no House, Scion, oath, or relic is ever
// invented presentation-side.

std::string chronicle_account_root(const ClientState& state) {
  if (!state.session) return {};
  return state.session->model().chronicle.account_name;
}

// Deterministic founder naming without any text-input console: derive the
// House name from the guest identity once the account payload arrives.
std::string house_display_name(const ClientState& state) {
  const std::string account = chronicle_account_root(state);
  if (account.empty()) return "New House";
  std::string root = account;
  for (auto& character : root)
    character = static_cast<char>(character == '-' ? ' ' : character);
  if (!root.empty())
    root[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(root[0])));
  return "House of " + root;
}

std::string next_scion_name(const ClientState& state) {
  static const char* kOrdinals[] = {"Firstborn", "Secondborn", "Thirdborn",
                                    "Fourthborn", "Fifthborn"};
  std::size_t total = 0;
  if (state.session) {
    for (const auto& house : state.session->model().chronicle.houses)
      total += house.scions.size() + house.crypt.size();
  }
  const std::string house_name = house_display_name(state);
  if (total < sizeof(kOrdinals) / sizeof(kOrdinals[0]))
    return house_name + " " + kOrdinals[total];
  return house_name + " Heir " + std::to_string(total + 1);
}

// The actionable front-door menu. Order is deterministic so scenarios and
// keyboard input agree: found → per-scion admissions → create → oath.
std::vector<ChronicleAction> chronicle_actions(const ClientState& state) {
  std::vector<ChronicleAction> menu;
  if (!state.session) return menu;
  const auto& model = state.session->model();
  // A fall awaits succession while the active scion is the fallen one. The
  // wire contract is explicit: heirs are admitted through
  // player:chronicles:select (it alone resets the permadead lifecycle);
  // chronicles:scion:set-out is the living scion's road-purse outing.
  const bool succession_pending =
      !model.chronicle.fallen.scion_id.empty() &&
      model.chronicle.fallen.scion_id == model.chronicle.active_scion_id;
  if (!model.chronicle.present || model.chronicle.houses.empty()) {
    menu.push_back({"F", "found-house", "", "Found your House"});
    return menu;
  }
  int slot = 1;
  for (const auto& house : model.chronicle.houses) {
    for (const auto& scion : house.scions) {
      ChronicleAction action;
      action.key = std::to_string(std::min(slot, 9));
      // The mortal oath rides only on player:chronicles:select on the wire,
      // so an armed oath admits through select; a plain journey claims the
      // road purse via chronicles:scion:set-out.
      if (state.chronicles_oath || succession_pending) {
        action.command = "select-scion";
        action.arg = scion.id;
        action.label = "Set out as " + scion.name;
        if (succession_pending)
          action.label += ", heir of " + model.chronicle.fallen.name;
        if (state.chronicles_oath) action.label += " under the mortal oath";
      } else {
        action.command = "set-out";
        action.arg = scion.id;
        action.label = "Set out as " + scion.name;
      }
      menu.push_back(std::move(action));
      if (++slot > 9) break;
    }
    if (slot > 9) break;
  }
  menu.push_back({"C", "create-scion", "",
                  "Name a new Scion (" + next_scion_name(state) + ")"});
  ChronicleAction oath;
  oath.key = "M";
  oath.command = "oath-toggle";
  oath.label = state.chronicles_oath ? "Mortal oath: ARMED" : "Mortal oath: not taken";
  menu.push_back(std::move(oath));
  return menu;
}

// Screen authority: expedition while admitted+alive; the front door before
// admission, during hard-death consequences, or whenever the connection is
// not usable (visible failure, never silent local fallback).
void update_screen_for_model(ClientState& state) {
  if (!state.chronicles_mode || !state.session) return;
  const auto& model = state.session->model();
  const bool soft_death = model.lifecycle == "awaiting-respawn";
  const bool admitted_and_alive =
      state.session->connection_state() ==
          verdigris::client::ConnectionState::Ready &&
      !model.chronicles_pending && model.player.alive;
  state.screen =
      (admitted_and_alive || soft_death) ? Screen::Expedition : Screen::Chronicles;
}

// Crypt transitions are authoritative relic-recovery beats: lost → recovered
// deserves an honest toast naming the fallen.
void watch_crypt_statuses(ClientState& state) {
  if (!state.session) return;
  const auto& model = state.session->model();
  std::unordered_map<std::string, std::string> current;
  for (const auto& house : model.chronicle.houses) {
    for (const auto& entry : house.crypt) {
      const std::string key = house.id + "/" + entry.id;
      current[key] = entry.relic_status;
      const auto it = state.known_crypt_status.find(key);
      if (it != state.known_crypt_status.end() &&
          it->second == "lost" && entry.relic_status == "recovered") {
        state.relic_toast = "Heirloom of " + entry.name + " recovered";
        state.relic_toast_ticks = 160;
      }
    }
  }
  state.known_crypt_status = std::move(current);
}

void submit_chronicle_action(ClientState& state, const ChronicleAction& action) {
  using verdigris::client::ClientCommand;
  if (action.command == "found-house") {
    state.session->submit(ClientCommand::found_house(house_display_name(state)));
    show_hint(state, "Your House enters the chronicles");
  } else if (action.command == "create-scion") {
    state.session->submit(ClientCommand::create_scion(next_scion_name(state)));
    show_hint(state, "A new Scion joins the lineage");
  } else if (action.command == "select-scion") {
    state.session->submit(
        ClientCommand::select_scion(action.arg, state.chronicles_oath));
    show_hint(state, state.chronicles_oath ? "The mortal oath is spoken"
                                           : "The heir walks on");
  } else if (action.command == "set-out") {
    state.session->submit(ClientCommand::set_out(action.arg));
    show_hint(state, "The wagon rolls out");
  } else if (action.command == "oath-toggle") {
    state.chronicles_oath = !state.chronicles_oath;
  }
}

void handle_chronicles_key(ClientState& state, WPARAM wparam) {
  state.chronicles_menu = chronicle_actions(state);
  for (const auto& action : state.chronicles_menu) {
    if (action.key.size() == 1 && wparam == static_cast<WPARAM>(action.key[0])) {
      submit_chronicle_action(state, action);
      return;
    }
  }
  if (wparam == VK_RETURN && !state.chronicles_menu.empty()) {
    for (const auto& action : state.chronicles_menu)
      if (action.command == "set-out" || action.command == "select-scion") {
        submit_chronicle_action(state, action);
        return;
      }
  }
}

// ── TASK-0161: contained capture-root isolation ─────────────────────────
// A full validation gate passes -CaptureRoot (threaded through the
// VERDIGRIS_CAPTURE_ROOT environment seam) so fresh evidence lands in a
// disposable directory instead of rewriting committed captures from earlier
// tasks. Without that variable every helper keeps its historical ladder, so
// default owner play and direct task-specific evidence runs are unchanged.

std::string absolute_path_normalized(const std::string& raw) {
  char full[MAX_PATH]{};
  const DWORD length = GetFullPathNameA(raw.c_str(), MAX_PATH, full, nullptr);
  if (length == 0 || length >= MAX_PATH) return {};
  return std::string(full, length);
}

std::string lowercase_ascii(std::string value) {
  for (char& character : value)
    if (character >= 'A' && character <= 'Z')
      character = static_cast<char>(character - 'A' + 'a');
  return value;
}

// Repository root: nearest ancestor of the cwd (then the executable
// directory) holding both native\ and orchestration\ markers.
std::string repository_root_for_capture_validation() {
  std::vector<std::string> bases;
  const std::string cwd = absolute_path_normalized(".");
  if (!cwd.empty()) bases.push_back(cwd);
  const std::string exe_dir = absolute_path_normalized(executable_directory());
  if (!exe_dir.empty()) bases.push_back(exe_dir);
  for (const auto& base : bases) {
    std::string prefix = base;
    for (int depth = 0; depth <= 6; ++depth) {
      if (directory_exists(prefix + "\\native") &&
          directory_exists(prefix + "\\orchestration"))
        return prefix;
      prefix += "\\..";
    }
  }
  return {};
}

bool create_directories_nested(const std::string& abs_path) {
  if (abs_path.size() < 3 || abs_path[1] != ':') return false;
  std::size_t index = 3;  // skip "X:\"
  while (true) {
    const std::size_t slash = abs_path.find('\\', index);
    const std::string part =
        abs_path.substr(0, slash == std::string::npos ? abs_path.size() : slash);
    if (!directory_exists(part) && !CreateDirectoryA(part.c_str(), nullptr) &&
        !directory_exists(part))
      return false;
    if (slash == std::string::npos) break;
    index = slash + 1;
  }
  return directory_exists(abs_path);
}

struct CaptureRootDecision {
  bool active = false;  // override requested via VERDIGRIS_CAPTURE_ROOT
  bool valid = false;   // contained inside the repository and created
  std::string dir;      // absolute contained root when valid
  std::string error;    // rejection reason when active but invalid
};

const CaptureRootDecision& capture_root_decision() {
  static const CaptureRootDecision decision = [] {
    CaptureRootDecision computed;
    const char* override_raw = std::getenv("VERDIGRIS_CAPTURE_ROOT");
    if (!override_raw) return computed;
    computed.active = true;
    const std::string requested = absolute_path_normalized(override_raw);
    if (requested.empty()) {
      computed.error = "capture root does not resolve to an absolute path";
      return computed;
    }
    const std::string repo_root = repository_root_for_capture_validation();
    if (repo_root.empty()) {
      computed.error = "repository root not found; capture containment cannot be proven";
      return computed;
    }
    const std::string request_lower = lowercase_ascii(requested);
    const std::string repo_prefix = lowercase_ascii(repo_root) + "\\";
    // Strictly inside only: an exact repository-root target would scatter
    // evidence into the worktree itself.
    if (request_lower.size() <= repo_prefix.size() ||
        request_lower.compare(0, repo_prefix.size(), repo_prefix) != 0) {
      computed.error = "capture root '" + requested +
                       "' is outside repository root '" + repo_root + "'";
      return computed;
    }
    // Containment is proven before any filesystem mutation, so a rejected
    // target can never be created or written.
    if (!create_directories_nested(requested)) {
      computed.error = "capture root '" + requested + "' could not be created";
      return computed;
    }
    computed.valid = true;
    computed.dir = requested;
    return computed;
  }();
  return decision;
}

// Consulted by every scenario capture helper. Returns 0 when no override is
// active (caller uses its historical ladder), 1 with *out set to the
// validated contained root, or -1 after reporting the rejection; on -1 the
// caller must fail the run without attempting any write.
int capture_root_override(std::string* out) {
  const CaptureRootDecision& decision = capture_root_decision();
  if (!decision.active) return 0;
  if (!decision.valid) {
    std::printf("FAIL capture-root: %s (nothing written)\n", decision.error.c_str());
    return -1;
  }
  *out = decision.dir;
  return 1;
}

std::string chronicles_capture_dir() {
  std::string forced;
  const int overridden = capture_root_override(&forced);
  if (overridden != 0) return overridden > 0 ? forced : std::string{};
  std::vector<std::string> bases{".", executable_directory()};
  const char* marker =
      "orchestration\\tasks\\TASK-0145-native-chronicles-owner-journey";
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

// TASK-0122 Phase A: fresh animation/VFX evidence lands in THIS task's
// captures/ folder for architect visual review.
std::string animation_vfx_capture_dir() {
  std::string forced;
  const int overridden = capture_root_override(&forced);
  if (overridden != 0) return overridden > 0 ? forced : std::string{};
  std::vector<std::string> bases{".", executable_directory()};
  const char* marker =
      "orchestration\\tasks\\TASK-0122-animation-vfx-system-wave";
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

// TASK-0156: fresh progression-surface evidence lands in THIS task's
// captures/ folder for architect visual review.
std::string progression_capture_dir() {
  std::string forced;
  const int overridden = capture_root_override(&forced);
  if (overridden != 0) return overridden > 0 ? forced : std::string{};
  std::vector<std::string> bases{".", executable_directory()};
  const char* marker =
      "orchestration\\tasks\\TASK-0156-native-progression-visibility";
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

void paint_chronicles_front_door(ClientState& state, HDC dc, const RECT& bounds,
                                 render::List& rl) {
  const auto& model = state.session ? state.session->model() : verdigris::client::ClientModel{};
  RECT panel{0, 0, bounds.right, bounds.bottom};
  HBRUSH backdrop = CreateSolidBrush(RGB(10, 14, 12));
  FillRect(dc, &panel, backdrop);
  DeleteObject(backdrop);

  struct Line {
    std::string label;
    std::string text;
    COLORREF color;
    bool accent;
  };
  std::vector<Line> lines;
  lines.push_back({"title", "V E R D I G R I S   C H R O N I C L E S",
                   RGB(120, 214, 168), true});
  {
    std::string status_line;
    if (!state.session ||
        state.session->connection_state() ==
            verdigris::client::ConnectionState::Disconnected) {
      status_line = "The chronicles lie closed - connection lost.";
    } else if (!model.chronicle.present) {
      status_line = "Opening the chronicles...";
    } else {
      status_line = "Account of " +
                    (model.chronicle.account_name.empty() ? std::string("the guest")
                                                          : model.chronicle.account_name);
    }
    lines.push_back({"account", status_line, RGB(185, 198, 188), false});
  }

  if (!model.chronicle.present || model.chronicle.houses.empty()) {
    lines.push_back({"prompt", "No House stands in these pages yet.",
                     RGB(230, 235, 220), false});
  } else {
    for (const auto& house : model.chronicle.houses) {
      lines.push_back({"house " + house.name,
                       "House " + house.name, RGB(239, 208, 116), false});
      for (const auto& scion : house.scions) {
        std::string row = "  Scion " + scion.name + " - level " +
                          std::to_string(scion.level) +
                          (scion.mortal ? " (mortal)" : "");
        lines.push_back({"scion " + scion.id, row, RGB(140, 208, 172), false});
      }
      for (const auto& entry : house.crypt) {
        std::string relic = "rests unrecorded";
        if (!entry.relic_status.empty()) {
          relic = "heirloom " + entry.relic_status;
          if (entry.relic_count > 0)
            relic += " (" + std::to_string(entry.relic_count) + " to circulation)";
        }
        lines.push_back({"crypt " + entry.id,
                         "  In the crypt: " + entry.name + " - " + relic,
                         RGB(150, 160, 170), false});
      }
    }
  }
  if (!model.chronicle.fallen.name.empty()) {
    lines.push_back(
        {"fallen:" + model.chronicle.fallen.scion_id,
         "The chronicle records the fall of " + model.chronicle.fallen.name +
             " (level " + std::to_string(model.chronicle.fallen.level) + ").",
         RGB(214, 92, 72), false});
  }

  state.chronicles_menu = chronicle_actions(state);
  for (const auto& action : state.chronicles_menu) {
    lines.push_back({"action:" + action.command +
                         (action.arg.empty() ? "" : ":" + action.arg),
                     "[" + action.key + "] " + action.label, RGB(239, 208, 116), false});
  }
  lines.push_back({state.chronicles_oath ? "oath:on" : "oath:off",
                   std::string("Oath field: ") +
                       (state.chronicles_oath ? "mortal - death is final"
                                              : "soft - wounds can be recovered"),
                   RGB(185, 198, 188), false});

  SetBkMode(dc, TRANSPARENT);
  const int door_scale = hud_scale(static_cast<int>(bounds.bottom));
  skin::set_ui_scale(door_scale);
  const int left =
      std::max(24, (static_cast<int>(bounds.right) - 620 * door_scale) / 2);
  int block_height = 24 * door_scale;
  for (const auto& line : lines)
    block_height += (line.accent ? 44 : 26) * door_scale;
  // Vertically centred chronicle page: a framed panel behind the text block
  // so the front door reads as a bound ledger, not text floating on black.
  int y = std::max(64 * door_scale,
                   (static_cast<int>(bounds.bottom) - block_height) / 2);
  {
    RECT page{left - 36 * door_scale, y - 28 * door_scale,
              left + 656 * door_scale,
              std::min(static_cast<int>(bounds.bottom) - 32, y + block_height)};
    skin::panel(dc, page, skin::kPanelBorder, 250, 10.0f);
  }
  for (const auto& line : lines) {
    rl.push_back({render::Op::Chronicles, static_cast<double>(left),
                  static_cast<double>(y), 0.0, 0, line.label});
    HGDIOBJ old_font = SelectObject(
        dc, line.accent ? skin::font_title() : skin::font_heading());
    SetTextColor(dc, line.color);
    TextOutA(dc, left, y, line.text.c_str(), static_cast<int>(line.text.size()));
    SelectObject(dc, old_font);
    y += (line.accent ? 44 : 26) * door_scale;
    if (y > bounds.bottom - 40) break;
  }
  if (state.relic_toast_ticks > 0 && !state.relic_toast.empty()) {
    rl.push_back({render::Op::Chronicles, 0.0, 0.0, 0.0, 0, "relic-toast"});
    SetTextColor(dc, RGB(239, 208, 116));
    TextOutA(dc, 18, bounds.bottom - 28, state.relic_toast.c_str(),
             static_cast<int>(state.relic_toast.size()));
  }
}

// Shared owner-facing chrome: the visible connection state lives on both
// screens — a failed connection is always explicit, never a silent fallback.
// TASK-0153 rev2: the chip draws where the measured top-HUD planner puts it.
int connection_chip_w(int height) { return 168 * hud_scale(height); }
int connection_chip_h(int height) { return 22 * hud_scale(height); }
void paint_connection_chip(ClientState& state, HDC dc, const RECT& bounds,
                           render::List& rl, int chip_x, int chip_y) {
  if (!state.session) return;
    const auto conn = state.session->connection_state();
    const char* label = verdigris::client::connection_state_label(conn);
    const std::string chip = std::string("connection ") + label;
    rl.push_back({render::Op::Hud, static_cast<double>(chip_x),
                  static_cast<double>(chip_y), 0.0, 0, chip});
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
    RECT chip_rect{chip_x, chip_y,
                   chip_x + connection_chip_w(static_cast<int>(bounds.bottom)),
                   chip_y + connection_chip_h(static_cast<int>(bounds.bottom))};
    state.hud_rect_trace.push_back(
        {"connection",
         {chip_rect.left, chip_rect.top,
          chip_rect.right - chip_rect.left,
          chip_rect.bottom - chip_rect.top}});
    HBRUSH chip_bg = CreateSolidBrush(RGB(25, 33, 37));
    FillRect(dc, &chip_rect, chip_bg);
    DeleteObject(chip_bg);
    HPEN chip_pen = CreatePen(PS_SOLID, 1, chip_color);
    HGDIOBJ old_chip_pen = SelectObject(dc, chip_pen);
    HGDIOBJ old_chip_brush =
        SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
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
      // TASK-0159: the banner keeps the left column but starts below the
      // minimap panel instead of painting across it.
      const HudRect map = minimap_rect(static_cast<int>(bounds.bottom));
      const int banner_y = std::max(76, map.y + map.h + 8);
      TextOutA(dc, 18, banner_y, banner, static_cast<int>(strlen(banner)));
    }
}

// ── TASK-0153 rev2 / TASK-0159: measured top-HUD row planner ────────────
// Every normal-HUD top region — identity, objective, connection, art-status,
// controls — is placed by one pure integer-geometry pass from actually
// measured extents. TASK-0159 extends the pass with the fixed screen regions
// those rows must never fight: the minimap panel always, and the shipped gear
// pane whenever it is open, plus the bottom quickbar/orbs for completeness.
// Candidates walk deterministic fallback ladders (centered/right pins, then a
// left lane beside the minimap, then the raw gutter), so no region is ever
// deleted and none lands inside another at any width, including 960x600 and
// 1366x768. The painter draws exactly what this returns; the scenario runs
// the same function over real frames as acceptance evidence.
constexpr int kTopHudGutter = 12;      // screen-edge breathing room
constexpr int kTopHudGap = 10;         // minimum clearance between regions
constexpr int kTopHudRow0Y = 12;
constexpr int kTopHudRowStep = 34;     // clears a chip's full height + margin
constexpr int kTopHudRowCount = 6;     // headroom for pane-open fallback rows

// TASK-0159: the planner's rectangle type is now the shared HudRect, so the
// blocked fixed regions and the placed text regions are one geometry.
using TopHudRect = HudRect;

struct TopHudLayout {
  TopHudRect identity;
  TopHudRect objective;
  TopHudRect connection;
  TopHudRect art;
  TopHudRect controls;
  bool objective_placed = false;
  bool controls_placed = false;
  // TASK-0159: when even the fallback ladders cannot fit the one-line
  // controls hint (a 643 px line cannot sit beside the open gear pane at
  // 960 width), the planner places the hint as two stacked lines instead of
  // letting the painter fall back onto other regions. Same words, same
  // authority, wrapped — never deleted.
  bool controls_wrapped = false;
  TopHudRect controls_second;
};

bool top_hud_clear(const TopHudRect& a, const TopHudRect& b, int gap) {
  return a.x + a.w + gap <= b.x || b.x + b.w + gap <= a.x ||
         a.y + a.h + gap <= b.y || b.y + b.h + gap <= a.y;
}

TopHudLayout plan_top_hud(int width, int height, bool gear_open,
                          const TopHudRect& identity_size,
                          const TopHudRect& objective_size,
                          const TopHudRect& art_size,
                          const TopHudRect& controls_size,
                          const TopHudRect& controls_size_a,
                          const TopHudRect& controls_size_b, bool session) {
  TopHudLayout layout;
  std::vector<TopHudRect> blocked;
  const auto keep_out = [&](const HudRect& r) {
    blocked.push_back(TopHudRect{r.x, r.y, r.w, r.h});
  };
  keep_out(minimap_rect(height));
  keep_out(quickbar_strip_rect(width, height));
  keep_out(vital_orb_rect(width, height, false));
  keep_out(vital_orb_rect(width, height, true));
  if (gear_open) keep_out(gear_pane_rect(width, height));

  std::vector<TopHudRect> occupied[kTopHudRowCount];
  const auto row_y = [&](int row) { return kTopHudRow0Y + row * kTopHudRowStep; };
  const auto fits = [&](int row, const TopHudRect& cand) {
    if (cand.x < kTopHudGutter) return false;
    if (cand.x + cand.w > width - kTopHudGutter) return false;
    for (const auto& taken : occupied[row])
      if (!top_hud_clear(cand, taken, kTopHudGap)) return false;
    for (const auto& keep_out_zone : blocked)
      if (!top_hud_clear(cand, keep_out_zone, kTopHudGap)) return false;
    return true;
  };
  const HudRect map = minimap_rect(height);
  // The left lane beside the minimap: the deterministic second anchor for
  // every region whose preferred pin is crowded or pane-blocked.
  const int lane_x = map.x + map.w + kTopHudGap;
  const auto try_rows_left = [&](const TopHudRect& size,
                                 int x) -> TopHudRect {
    for (int row = 0; row < kTopHudRowCount; ++row) {
      TopHudRect cand{x, row_y(row), size.w, size.h};
      if (fits(row, cand)) {
        occupied[row].push_back(cand);
        return cand;
      }
    }
    return TopHudRect{};
  };
  // Right-aligned chips keep their historical edge pin; if the right side is
  // crowded or pane-blocked, the left lane takes them instead.
  const auto place_right = [&](const TopHudRect& size) {
    for (int row = 0; row < kTopHudRowCount; ++row) {
      TopHudRect cand{std::max(kTopHudGutter, width - kTopHudGutter - size.w),
                      row_y(row), size.w, size.h};
      if (fits(row, cand)) {
        occupied[row].push_back(cand);
        return cand;
      }
    }
    return try_rows_left(size, lane_x);
  };
  const auto place_centered = [&](const TopHudRect& size, bool& placed) {
    for (int row = 0; row < kTopHudRowCount; ++row) {
      TopHudRect cand{std::max(kTopHudGutter, (width - size.w) / 2), row_y(row),
                      size.w, size.h};
      if (fits(row, cand)) {
        occupied[row].push_back(cand);
        placed = true;
        return cand;
      }
    }
    placed = false;
    // Centered fallback ladder: the left lane beside the minimap, then the
    // raw gutter once rows have cleared the map's height.
    for (int pass = 0; pass < 2 && !placed; ++pass) {
      const TopHudRect got =
          try_rows_left(size, pass == 0 ? lane_x : kTopHudGutter + 6);
      if (got.w > 0) {
        placed = true;
        return got;
      }
    }
    return TopHudRect{};
  };

  // Identity leads the hierarchy: top row, in the lane beside the minimap so
  // it can never paint across the map again; deeper rows only if contested.
  layout.identity = try_rows_left(identity_size, lane_x);
  if (layout.identity.w == 0)
    layout.identity = try_rows_left(identity_size, kTopHudGutter + 6);
  if (layout.identity.w == 0) {
    layout.identity =
        TopHudRect{kTopHudGutter + 6, row_y(0), identity_size.w, identity_size.h};
    occupied[0].push_back(layout.identity);
  }
  if (session)
    layout.connection =
        place_right(TopHudRect{0, 0, connection_chip_w(height),
                                connection_chip_h(height)});
  // Art keeps its historical rows: beside the identity locally, under the
  // connection chip on the remote owner path (row 0 is taken there); the left
  // lane catches it when an open gear pane owns the right side.
  layout.art = place_right(art_size);
  // The objective outranks the controls hint when rows are contested.
  layout.objective = place_centered(objective_size, layout.objective_placed);
  layout.controls = place_centered(controls_size, layout.controls_placed);
  // TASK-0159: if no single-line slot exists, wrap the hint into two stacked
  // lines placed as one unit on the left ladders.
  if (layout.controls.w == 0 && controls_size_b.w > 0) {
    const auto try_rows_left_pair = [&](const TopHudRect& first,
                                        const TopHudRect& second,
                                        int x) -> TopHudRect {
      for (int row = 0; row + 1 < kTopHudRowCount; ++row) {
        TopHudRect cand_a{x, row_y(row), first.w, first.h};
        TopHudRect cand_b{x, row_y(row + 1), second.w, second.h};
        if (fits(row, cand_a) && fits(row + 1, cand_b)) {
          occupied[row].push_back(cand_a);
          occupied[row + 1].push_back(cand_b);
          layout.controls_wrapped = true;
          layout.controls_second = cand_b;
          return cand_a;
        }
      }
      return TopHudRect{};
    };
    for (int pass = 0; pass < 2 && !layout.controls_wrapped; ++pass) {
      const TopHudRect got = try_rows_left_pair(
          controls_size_a, controls_size_b,
          pass == 0 ? lane_x : kTopHudGutter + 6);
      if (got.w > 0) {
        layout.controls = got;
        layout.controls_placed = true;
      }
    }
  }
  return layout;
}

// -- Wall tiles -----------------------------------------------------------
// Blocked cells of the authoritative walkable grid, drawn as chunky raised
// stone so collision is always visible. Vector-only: no assets required.
void draw_wall_tiles(const WorldView& world, HDC dc, const Camera& camera,
                     const RECT& bounds) {
  if (world.map_width <= 0 || world.map_height <= 0 ||
      world.map_walkable.size() !=
          static_cast<std::size_t>(world.map_width) * world.map_height)
    return;
  const double tile = kTileUnits;
  const double half_w_units =
      (static_cast<double>(bounds.right) * 0.5) / std::max(0.05, camera.zoom) +
      tile;
  const double half_h_units =
      (static_cast<double>(bounds.bottom) * 0.5) / std::max(0.05, camera.zoom) +
      tile;
  int start_tx = static_cast<int>(std::floor((camera.x - half_w_units) / tile));
  int end_tx = static_cast<int>(std::ceil((camera.x + half_w_units) / tile));
  int start_ty = static_cast<int>(std::floor((camera.y - half_h_units) / tile));
  int end_ty = static_cast<int>(std::ceil((camera.y + half_h_units) / tile));
  start_tx = std::max(start_tx, 0);
  start_ty = std::max(start_ty, 0);
  end_tx = std::min(end_tx, world.map_width - 1);
  end_ty = std::min(end_ty, world.map_height - 1);
  for (int ty = start_ty; ty <= end_ty; ++ty) {
    for (int tx = start_tx; tx <= end_tx; ++tx) {
      if (world.map_walkable[static_cast<std::size_t>(ty) * world.map_width +
                             tx])
        continue;
      // The tile occupies [tx-0.5, tx+0.5) in protocol space (positions round
      // to the nearest tile), so the block is centred on the tile coordinate.
      const double wx = (static_cast<double>(tx) - 0.5) * tile;
      const double wy = (static_cast<double>(ty) - 0.5) * tile;
      const ScreenPoint c0 = project(camera, bounds, wx, wy);
      const ScreenPoint c1 = project(camera, bounds, wx + tile, wy + tile);
      if (c1.x < 0 || c1.y < 0 || c0.x > bounds.right || c0.y > bounds.bottom)
        continue;
      const int lift = std::max(4, static_cast<int>((c1.y - c0.y) * 0.45));
      // Shadow face below the slab.
      RECT face{c0.x, c1.y - lift, c1.x, c1.y};
      HBRUSH face_brush = CreateSolidBrush(RGB(16, 14, 12));
      FillRect(dc, &face, face_brush);
      DeleteObject(face_brush);
      // Raised top slab, clearly lighter than any floor plate.
      RECT top{c0.x, c0.y - lift, c1.x, c1.y - lift};
      HBRUSH top_brush = CreateSolidBrush(RGB(88, 78, 66));
      FillRect(dc, &top, top_brush);
      DeleteObject(top_brush);
      // Lit rim + seams for the cut-stone read.
      draw_line(dc, c0.x, top.top, c1.x, top.top, RGB(146, 132, 108), 2);
      draw_line(dc, c0.x, top.top, c0.x, top.bottom, RGB(118, 106, 88), 1);
      draw_line(dc, c1.x - 1, top.top, c1.x - 1, top.bottom, RGB(30, 26, 22), 1);
      const int mid_y = (top.top + top.bottom) / 2;
      draw_line(dc, c0.x, mid_y, c1.x, mid_y, RGB(52, 46, 38), 1);
      draw_line(dc, c0.x + (c1.x - c0.x) / 2, top.top,
                c0.x + (c1.x - c0.x) / 2, mid_y, RGB(52, 46, 38), 1);
      draw_line(dc, c0.x + (c1.x - c0.x) / 4, mid_y,
                c0.x + (c1.x - c0.x) / 4, top.bottom, RGB(52, 46, 38), 1);
    }
  }
}

// -- Character sheet pane ------------------------------------------------
// Authoritative Scion sheet: identity, vitals, combat totals, attributes.
void paint_character_pane(ClientState& state, HDC dc, const RECT& bounds,
                          render::List& rl) {
  if (!state.character_pane) return;
  const int s = hud_scale(static_cast<int>(bounds.bottom));
  const int pane_w = 360 * s;
  // Content-derived height: header, portrait, nine stat rows, footer. A
  // fixed height under a scaled type ramp is exactly how rows clip out.
  const int row_h = 26 * s;
  const int pane_h = (56 + 150 + 14) * s + 9 * row_h + 40 * s;
  const int left = 24 * s;
  const int top =
      std::max(48 * s, (static_cast<int>(bounds.bottom) - pane_h) / 2 - 20 * s);
  RECT pane{left, top, left + pane_w, top + pane_h};
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_panel, pane))
    skin::panel(dc, pane, skin::kVerdigris, 245, 8.0f);
  rl.push_back({render::Op::Hud, static_cast<double>(left),
                static_cast<double>(top), 0.0, 0, "character-pane"});
  SetBkMode(dc, TRANSPARENT);
  HGDIOBJ old_font = SelectObject(dc, skin::font_heading());
  SetTextColor(dc, skin::kVerdigris);
  const std::string title = state.world.scion_name.empty()
                                ? std::string("The Scion")
                                : state.world.scion_name;
  TextOutA(dc, left + 16 * s, top + 10 * s, title.c_str(),
           static_cast<int>(title.size()));
  SelectObject(dc, skin::font_small());
  SetTextColor(dc, skin::kInkDim);
  TextOutA(dc, left + 16 * s, top + 34 * s, state.world.house_name.c_str(),
           static_cast<int>(state.world.house_name.size()));

  // Portrait: the player plate, drawn tall on the left of the sheet.
  const int portrait_h = 150 * s;
  if (state.billboards.player.ready() && state.billboards.alpha_blend) {
    const SpriteBitmap& sprite = state.billboards.player;
    const int dest_h = portrait_h;
    const int dest_w = dest_h * sprite.width / std::max(1, sprite.height);
    const BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    state.billboards.alpha_blend(dc, left + 20 * s, top + 56 * s, dest_w,
                                 dest_h, sprite.dc, 0, 0, sprite.width,
                                 sprite.height, blend);
  }

  const WorldActor& player = state.world.player;
  int equipped_bonus = 0;
  std::string weapon = "(unarmed)";
  for (const auto& item : state.world.carried)
    if (item.equipped) {
      equipped_bonus = item.attack_bonus;
      weapon = item.name;
      break;
    }
  int attr_str = 10, attr_dex = 10, attr_int = 10;
  if (state.session) {
    const auto& model = state.session->model();
    attr_str = model.attr_strength;
    attr_dex = model.attr_dexterity;
    attr_int = model.attr_intelligence;
  }
  struct StatRow {
    std::string label;
    std::string value;
  };
  const StatRow rows[] = {
      {"Level", std::to_string(player.level)},
      {"Life", std::to_string(player.life) + " / " + std::to_string(player.life_max)},
      {"Resource", std::to_string(player.resource) + " / " +
                       std::to_string(player.resource_max)},
      {"Attack", std::to_string(player.attack + equipped_bonus) +
                     (equipped_bonus ? " (+" + std::to_string(equipped_bonus) + ")"
                                     : "")},
      {"Defense", std::to_string(player.defense)},
      {"Weapon", weapon},
      {"Strength", std::to_string(attr_str)},
      {"Dexterity", std::to_string(attr_dex)},
      {"Intelligence", std::to_string(attr_int)},
  };
  int y = top + 56 * s + portrait_h + 14 * s;
  SelectObject(dc, skin::font_body());
  for (const auto& row : rows) {
    SetTextColor(dc, skin::kInkDim);
    TextOutA(dc, left + 20 * s, y, row.label.c_str(),
             static_cast<int>(row.label.size()));
    SIZE extent{};
    GetTextExtentPoint32A(dc, row.value.c_str(),
                          static_cast<int>(row.value.size()), &extent);
    SetTextColor(dc, skin::kInk);
    TextOutA(dc, left + pane_w - 20 * s - extent.cx, y, row.value.c_str(),
             static_cast<int>(row.value.size()));
    rl.push_back({render::Op::Hud, static_cast<double>(left),
                  static_cast<double>(y), 0.0, 0,
                  "char:" + row.label + ":" + row.value});
    y += row_h;
  }
  SelectObject(dc, skin::font_small());
  SetTextColor(dc, skin::kInkDim);
  const char* footer = "C or Esc closes";
  TextOutA(dc, left + 20 * s, y + 8 * s, footer,
           static_cast<int>(strlen(footer)));
  SelectObject(dc, old_font);
}

// -- Passive tree pane ---------------------------------------------------
// The geometric first-level slice over the authoritative allocation. Click
// (or Enter on) a frontier seat to spend a point; the server owns budget.
void paint_tree_pane(ClientState& state, HDC dc, const RECT& bounds,
                     render::List& rl) {
  state.tree_seat_hits.clear();
  if (!state.tree_pane) return;
  const int s = hud_scale(static_cast<int>(bounds.bottom));
  const int pane_w = 460 * s;
  const int pane_h = 420 * s;
  const int left = (static_cast<int>(bounds.right) - pane_w) / 2;
  const int top =
      std::max(48 * s, (static_cast<int>(bounds.bottom) - pane_h) / 2 - 20 * s);
  RECT pane{left, top, left + pane_w, top + pane_h};
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_panel, pane))
    skin::panel(dc, pane, skin::kVerdigris, 245, 8.0f);
  rl.push_back({render::Op::Hud, static_cast<double>(left),
                static_cast<double>(top), 0.0, 0, "tree-pane"});
  SetBkMode(dc, TRANSPARENT);
  HGDIOBJ old_font = SelectObject(dc, skin::font_heading());
  SetTextColor(dc, skin::kVerdigris);
  const char* title = "Geometric Passives";
  TextOutA(dc, left + 16 * s, top + 10 * s, title,
           static_cast<int>(strlen(title)));

  const bool remote = state.session != nullptr;
  const auto* progression =
      remote ? &state.session->model().progression : nullptr;
  const bool present = progression && progression->present;
  SelectObject(dc, skin::font_body());
  std::string points_line = present
      ? std::to_string(progression->unspent_points) + " point(s) to spend of " +
            std::to_string(progression->earned_points) + " earned"
      : std::string("No authoritative tree data on this session.");
  SetTextColor(dc, present && progression->unspent_points > 0 ? skin::kGold
                                                              : skin::kInkDim);
  TextOutA(dc, left + 16 * s, top + 38 * s, points_line.c_str(),
           static_cast<int>(points_line.size()));

  const auto slice = geometric_skill_tree::make_owner_demo_first_level_slice();
  const auto node_id_of = [](geometric_skill_tree::Axial pos) {
    return std::to_string(pos.q) + "," + std::to_string(pos.r);
  };
  const auto allocated = [&](const std::string& id) {
    if (!present) return id == std::string("0,0");
    for (const auto& node : progression->nodes)
      if (node == id) return true;
    return false;
  };
  const int center_x = left + pane_w / 2;
  const int center_y = top + pane_h / 2 + 20 * s;
  const double hex = 62.0 * s;
  const int seat_r = 24 * s;
  for (std::uint8_t i = 0; i < slice.seat_count; ++i) {
    const auto& seat = slice.seats[i];
    const std::string id = node_id_of(seat.pos);
    const bool active = allocated(id);
    bool frontier = false;
    if (!active && present && progression->unspent_points > 0) {
      for (std::uint8_t j = 0; j < slice.seat_count; ++j) {
        const std::string other = node_id_of(slice.seats[j].pos);
        if (allocated(other) &&
            geometric_skill_tree::hex_distance(seat.pos, slice.seats[j].pos) == 1) {
          frontier = true;
          break;
        }
      }
    }
    const int sx = center_x + static_cast<int>(hex * 1.5 * seat.pos.q);
    const int sy = center_y +
                   static_cast<int>(hex * 0.8660254 *
                                    (2.0 * seat.pos.r + seat.pos.q));
    const COLORREF fill = active ? RGB(52, 112, 86)
                          : frontier ? RGB(64, 58, 30)
                                     : RGB(26, 32, 31);
    const COLORREF ring = active ? skin::kVerdigris
                          : frontier ? skin::kGold
                                     : RGB(64, 74, 70);
    fill_ellipse(dc, sx, sy, seat_r, seat_r, fill);
    ring_ellipse(dc, sx, sy, seat_r, seat_r, ring, active || frontier ? 3 : 1);
    const char* type_label = geometric_skill_tree::seat_type_name(seat.type);
    SIZE extent{};
    HGDIOBJ seat_font = SelectObject(dc, skin::font_small());
    GetTextExtentPoint32A(dc, type_label, static_cast<int>(strlen(type_label)),
                          &extent);
    SetTextColor(dc, active ? skin::kInk : skin::kInkDim);
    TextOutA(dc, sx - extent.cx / 2, sy - extent.cy / 2, type_label,
             static_cast<int>(strlen(type_label)));
    SelectObject(dc, seat_font);
    rl.push_back({render::Op::Hud, static_cast<double>(sx),
                  static_cast<double>(sy), 0.0, active ? 1 : 0,
                  "tree-seat:" + id + (active ? std::string(":active")
                                     : frontier ? std::string(":frontier")
                                                : std::string(":locked"))});
    state.tree_seat_hits.push_back({sx, sy, seat_r, id, frontier});
  }
  SelectObject(dc, skin::font_small());
  SetTextColor(dc, skin::kInkDim);
  const char* footer = "Click a gold seat to allocate | P or Esc closes";
  TextOutA(dc, left + 16 * s, top + pane_h - 24 * s, footer,
           static_cast<int>(strlen(footer)));
  SelectObject(dc, old_font);
}

// -- Trader / countinghouse panes ----------------------------------------
// Mouse-and-keyboard modal panes over the authoritative open:screen model.
// Rows rebuild their hit rectangles every frame; activation submits the
// exact context-menu action the server owns and the refreshed screen
// arrives on the same wire that opened the pane.

void activate_trade_row(ClientState& state, const ClientState::TradeRowHit& hit) {
  if (!state.session) return;
  const char* action = hit.kind == 0   ? "player:shop:buy"
                       : hit.kind == 1 ? "player:bank:withdraw"
                                       : "player:bank:deposit";
  state.session->submit(
      verdigris::client::ClientCommand::menu_action(action, hit.ref, hit.value));
}

void paint_trade_pane(ClientState& state, HDC dc, const RECT& bounds,
                      render::List& rl) {
  state.trade_row_hits.clear();
  if (!state.session) return;
  const auto& model = state.session->model();
  if (!model.shop.open && !model.bank.open) return;
  const int s = hud_scale(static_cast<int>(bounds.bottom));
  const bool shop = model.shop.open;
  const int pane_w = (shop ? 460 : 560) * s;
  const int row_h = 30 * s;

  // Build the row list first so the pane height fits the content.
  struct Row {
    std::string left;
    std::string right;
    ClientState::TradeRowHit hit;
    bool header = false;
  };
  std::vector<Row> rows;
  if (shop) {
    for (std::size_t i = 0; i < model.shop.rows.size(); ++i) {
      const auto& stock = model.shop.rows[i];
      Row row;
      row.left = stock.name;
      row.right = std::to_string(stock.price) + "g  x" + std::to_string(stock.qty);
      row.hit.kind = 0;
      row.hit.ref = stock.id;
      row.hit.value = stock.price;
      rows.push_back(std::move(row));
    }
  } else {
    if (!model.bank.items.empty()) {
      Row header;
      header.left = "Stored with the House";
      header.header = true;
      rows.push_back(std::move(header));
    }
    for (const auto& item : model.bank.items) {
      Row row;
      row.left = item.name;
      row.right = "x" + std::to_string(item.qty) + "  withdraw";
      row.hit.kind = 1;
      row.hit.ref = item.uuid;
      row.hit.value = 1;
      rows.push_back(std::move(row));
    }
    if (!model.inventory.empty()) {
      Row header;
      header.left = "Carried (click to deposit)";
      header.header = true;
      rows.push_back(std::move(header));
    }
    for (const auto& item : model.inventory) {
      Row row;
      row.left = item.name.empty() ? item.id : item.name;
      row.right = "deposit";
      row.hit.kind = 2;
      row.hit.ref = item.uuid;
      row.hit.value = 1;
      rows.push_back(std::move(row));
    }
    if (rows.empty()) {
      Row row;
      row.left = "Nothing stored and nothing carried.";
      row.header = true;
      rows.push_back(std::move(row));
    }
  }

  const int title_h = 34 * s;
  const int footer_h = 26 * s;
  const int pane_h = title_h + static_cast<int>(rows.size()) * row_h +
                     footer_h + 20 * s;
  const int left = (static_cast<int>(bounds.right) - pane_w) / 2;
  const int top = std::max(
      24 * s, (static_cast<int>(bounds.bottom) - pane_h) / 2 - 20 * s);
  RECT pane{left, top, left + pane_w, top + pane_h};
  skin::panel(dc, pane, shop ? skin::kGold : skin::kVerdigris, 250, 9.0f);
  rl.push_back({render::Op::Hud, static_cast<double>(left),
                static_cast<double>(top), 0.0, 0,
                shop ? "shop-pane" : "bank-pane"});

  SetBkMode(dc, TRANSPARENT);
  HGDIOBJ old_font = SelectObject(dc, skin::font_heading());
  SetTextColor(dc, shop ? skin::kGold : skin::kVerdigris);
  const std::string title = shop ? model.shop.name : "Rhea's Countinghouse";
  TextOutA(dc, left + 16 * s, top + 8 * s, title.c_str(),
           static_cast<int>(title.size()));
  SelectObject(dc, skin::font_body());

  // Clamp the keyboard cursor to the activatable rows.
  std::size_t activatable = 0;
  for (const auto& row : rows)
    if (!row.header) ++activatable;
  if (activatable == 0) state.trade_selected = 0;
  else if (state.trade_selected >= activatable)
    state.trade_selected = activatable - 1;

  int y = top + title_h;
  std::size_t active_index = 0;
  for (const auto& row : rows) {
    RECT line{left + 10 * s, y, left + pane_w - 10 * s, y + row_h};
    if (row.header) {
      SelectObject(dc, skin::font_body_bold());
      SetTextColor(dc, skin::kInkDim);
      TextOutA(dc, line.left + 6 * s, y + 6 * s, row.left.c_str(),
               static_cast<int>(row.left.size()));
      SelectObject(dc, skin::font_body());
    } else {
      const bool selected = active_index == state.trade_selected;
      skin::slot(dc, line, shop ? skin::kGold : skin::kVerdigris, selected);
      SetTextColor(dc, selected ? skin::kInk : skin::kInkDim);
      TextOutA(dc, line.left + 8 * s, y + 6 * s, row.left.c_str(),
               static_cast<int>(row.left.size()));
      SIZE extent{};
      GetTextExtentPoint32A(dc, row.right.c_str(),
                            static_cast<int>(row.right.size()), &extent);
      SetTextColor(dc, shop ? skin::kGold : skin::kVerdigris);
      TextOutA(dc, line.right - extent.cx - 8 * s, y + 6 * s, row.right.c_str(),
               static_cast<int>(row.right.size()));
      ClientState::TradeRowHit hit = row.hit;
      hit.rect = line;
      hit.index = active_index;
      state.trade_row_hits.push_back(std::move(hit));
      rl.push_back({render::Op::Hud, static_cast<double>(line.left),
                    static_cast<double>(y), 0.0, static_cast<int>(active_index),
                    (shop ? std::string("shop-row:") : std::string("bank-row:")) +
                        row.left});
      ++active_index;
    }
    y += row_h;
  }

  SelectObject(dc, skin::font_small());
  SetTextColor(dc, skin::kInkDim);
  const std::string footer =
      (shop ? "carrying " + std::to_string(model.shop.carried_coins) + "g"
            : "House treasury " + std::to_string(model.bank.treasury) +
                  "g - carrying " + std::to_string(model.bank.carried_coins) +
                  "g") +
      "  |  click or Enter - Esc closes";
  TextOutA(dc, left + 16 * s, top + pane_h - footer_h, footer.c_str(),
           static_cast<int>(footer.size()));
  SelectObject(dc, old_font);
}

void paint_scene(ClientState& state, HDC dc, const RECT& bounds) {
  sync_world(state);
  const WorldView& world = state.world;
  render::List rl;
  // TASK-0159: one fresh rectangle trace per presented frame.
  state.hud_rect_trace.clear();
  // Skin type ramp: every HUD measure and draw this frame uses the real UI
  // face instead of the stock bitmap font, scaled to the window height.
  // Selected once per frame so the planner's extents and the painted glyphs
  // can never disagree.
  skin::set_ui_scale(hud_scale(static_cast<int>(bounds.bottom)));
  SelectObject(dc, skin::font_body());
  LARGE_INTEGER section_freq{}, section_t0{}, section_t1{}, section_t2{};
  QueryPerformanceFrequency(&section_freq);
  QueryPerformanceCounter(&section_t0);
  const auto section_ms = [&](const LARGE_INTEGER& a, const LARGE_INTEGER& b) {
    return section_freq.QuadPart > 0
               ? 1000.0 * static_cast<double>(b.QuadPart - a.QuadPart) /
                     static_cast<double>(section_freq.QuadPart)
               : 0.0;
  };

  // TASK-0145: the Chronicles front door replaces the abrupt game-window
  // entry for the remote owner path. Expedition painting is skipped
  // entirely; the door renders from the authoritative chronicle model.
  if (state.screen == Screen::Chronicles) {
    paint_chronicles_front_door(state, dc, bounds, rl);
    // The front door owns its whole canvas, so the shared chip keeps its
    // historical edge-pin position here; the planner governs the expedition.
    paint_connection_chip(
        state, dc, bounds, rl,
        std::max(18, static_cast<int>(bounds.right) -
                         connection_chip_w(static_cast<int>(bounds.bottom)) - 18),
        12);
    state.render_list = std::move(rl);
    return;
  }

  draw_floor(state.billboards, dc, state.camera, bounds, world.route_id, rl,
             &state.floor_cache);
  draw_wall_tiles(world, dc, state.camera, bounds);
  QueryPerformanceCounter(&section_t1);
  state.paint_ms_floor = section_ms(section_t0, section_t1);

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
  for (std::size_t i = 0; i < world.npcs.size(); ++i) {
    order.push_back({camera2d::draw_order_key(
                         static_cast<double>(world.npcs[i].position.y),
                         static_cast<double>(world.npcs[i].position.x)),
                     2, DepthDraw::What::Npc, i});
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
        ScreenPoint base =
            project(state.camera, bounds, player.position.x, player.position.y);
        rl.push_back({render::Op::Player, static_cast<double>(base.x),
                      static_cast<double>(base.y)});
        draw_contact_shadow(dc, base, kTileUnits * 0.42);
        draw_team_ring(dc, base, kTileUnits * 0.55, RGB(120, 214, 168));
        // Strike lunge: while a swing effect is alive the body steps into
        // the blow along the facing and recovers - a half-sine over the
        // arc's lifetime, sub-tick smoothed so 60 fps rendering reads it
        // as motion rather than three poses.
        for (const auto& fx : state.effects) {
          if (fx.kind != EffectFx::Kind::Swing &&
              fx.kind != EffectFx::Kind::SweepArc)
            continue;
          const double phase = std::clamp(
              (static_cast<double>(fx.age) + state.tick_accum_ms / 50.0) /
                  std::max(1, fx.ttl),
              0.0, 1.0);
          const double push = std::sin(phase * kPi) * kTileUnits * 0.28;
          base.x += static_cast<int>(std::cos(fx.angle) * push * base.scale);
          base.y += static_cast<int>(std::sin(fx.angle) * push * base.scale);
          break;
        }
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
      case DepthDraw::What::Npc: {
        const auto& npc = world.npcs[entry.index];
        const ScreenPoint base =
            project(state.camera, bounds, npc.position.x, npc.position.y);
        rl.push_back({render::Op::Npc, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, npc.id, npc.name});
        draw_contact_shadow(dc, base, kTileUnits * 0.42);
        // Role-coloured ring, and always the vector silhouette so townsfolk
        // never read as copies of the raster player plate.
        COLORREF ring = RGB(122, 168, 230);  // guide/talk
        if (!npc.actions.empty()) {
          const std::string& lead = npc.actions.front();
          if (lead == "trade") ring = RGB(239, 208, 116);
          else if (lead == "bank") ring = RGB(120, 214, 168);
          else if (std::find(npc.actions.begin(), npc.actions.end(),
                             std::string("trade")) != npc.actions.end())
            ring = RGB(239, 208, 116);
          else if (std::find(npc.actions.begin(), npc.actions.end(),
                             std::string("bank")) != npc.actions.end())
            ring = RGB(120, 214, 168);
        }
        draw_team_ring(dc, base, kTileUnits * 0.55, ring);
        {
          const int kit_height =
              std::max(8, static_cast<int>(kTileUnits * 1.35 * base.scale));
          if (const kit::Symbol* symbol = kit_symbol("player"))
            draw_kit_symbol(dc, *symbol, base.x, base.y, kit_height, false);
          else
            draw_billboard(dc, base, kTileUnits * 0.62, kTileUnits * 1.35,
                           RGB(96, 132, 178), RGB(150, 186, 226));
        }
        // Name plate: NPCs are the town's story surface; they stay labeled.
        {
          SetBkMode(dc, TRANSPARENT);
          SetTextColor(dc, RGB(170, 202, 240));
          SIZE extent{};
          GetTextExtentPoint32A(dc, npc.name.c_str(),
                                static_cast<int>(npc.name.size()), &extent);
          const int name_y =
              base.y - static_cast<int>(kTileUnits * 1.6 * base.scale) - 6;
          TextOutA(dc, base.x - extent.cx / 2, name_y, npc.name.c_str(),
                   static_cast<int>(npc.name.size()));
          // Interaction prompt when the player is within hailing distance.
          const int ddx = npc.position.x - world.player.position.x;
          const int ddy = npc.position.y - world.player.position.y;
          const double hail = kTileUnits * 1.9;
          if (std::abs(ddx) <= hail && std::abs(ddy) <= hail) {
            std::string verb = "Examine";
            for (const auto& action : npc.actions) {
              if (action == "talk") { verb = "Talk"; break; }
              if (action == "trade") { verb = "Trade"; break; }
              if (action == "bank") { verb = "Bank"; break; }
            }
            const std::string prompt = "[T] " + verb;
            SIZE prompt_extent{};
            GetTextExtentPoint32A(dc, prompt.c_str(),
                                  static_cast<int>(prompt.size()), &prompt_extent);
            SetTextColor(dc, RGB(239, 208, 116));
            TextOutA(dc, base.x - prompt_extent.cx / 2, name_y - extent.cy - 2,
                     prompt.c_str(), static_cast<int>(prompt.size()));
          }
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
        const int r = std::max(4, static_cast<int>(kTileUnits * 0.20 * base.scale));
        const int lift = static_cast<int>(kTileUnits * 0.28 * base.scale);
        const int gx = base.x;
        const int gy = base.y - lift;
        // Category glyph from the item's name: every drop reads as a thing,
        // not an abstract marker. Vector-only placeholders by design.
        std::string kind_name = loot_label(state, entry_loot.first);
        for (auto& ch : kind_name)
          ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        const bool is_trophy = entry_loot.first.rfind("trophy", 0) == 0 ||
                               kind_name.find("omen") != std::string::npos ||
                               kind_name.find("trophy") != std::string::npos;
        const bool is_coins = kind_name.find("coin") != std::string::npos ||
                              kind_name.find("gold") != std::string::npos;
        const bool is_weapon = kind_name.find("sword") != std::string::npos ||
                               kind_name.find("dagger") != std::string::npos ||
                               kind_name.find("knife") != std::string::npos ||
                               kind_name.find("axe") != std::string::npos ||
                               kind_name.find("pike") != std::string::npos ||
                               kind_name.find("spear") != std::string::npos;
        const bool is_shield = kind_name.find("shield") != std::string::npos;
        const bool is_vessel = kind_name.find("vessel") != std::string::npos ||
                               kind_name.find("draught") != std::string::npos ||
                               kind_name.find("orb") != std::string::npos ||
                               kind_name.find("bowl") != std::string::npos;
        const COLORREF color = is_trophy ? RGB(196, 148, 220)
                               : is_coins ? RGB(240, 198, 80)
                               : is_weapon ? RGB(200, 206, 214)
                               : is_shield ? RGB(168, 128, 84)
                               : is_vessel ? RGB(120, 190, 214)
                                           : RGB(230, 181, 74);
        HBRUSH brush = CreateSolidBrush(color);
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(18, 16, 14));
        HGDIOBJ old_brush = SelectObject(dc, brush);
        HGDIOBJ old_pen = SelectObject(dc, pen);
        if (is_coins) {
          // A short stack of coins.
          for (int c = 0; c < 3; ++c) {
            Ellipse(dc, gx - r, gy - c * (r / 2) - r / 3,
                    gx + r, gy - c * (r / 2) + r / 3);
          }
        } else if (is_weapon) {
          // An angled blade with a crossguard.
          POINT blade[4] = {{gx - r, gy + r}, {gx - r + r / 3, gy + r},
                           {gx + r, gy - r + r / 3}, {gx + r - r / 3, gy - r}};
          Polygon(dc, blade, 4);
          POINT guard[4] = {{gx - r / 2 - r / 4, gy + r / 4},
                           {gx - r / 4, gy + r / 2 + r / 4},
                           {gx - r / 8, gy + r / 2},
                           {gx - r / 2, gy + r / 8}};
          Polygon(dc, guard, 4);
        } else if (is_shield) {
          POINT shield[5] = {{gx - r, gy - r / 2}, {gx + r, gy - r / 2},
                            {gx + r, gy + r / 4}, {gx, gy + r},
                            {gx - r, gy + r / 4}};
          Polygon(dc, shield, 5);
        } else if (is_trophy) {
          // A curved horn.
          POINT horn[6] = {{gx - r, gy + r / 2}, {gx - r / 3, gy + r / 4},
                          {gx + r / 4, gy - r / 4}, {gx + r, gy - r},
                          {gx + r / 2, gy + r / 6}, {gx - r / 2, gy + r}};
          Polygon(dc, horn, 6);
        } else if (is_vessel) {
          // An amphora: body plus neck.
          Ellipse(dc, gx - r + r / 4, gy - r / 3, gx + r - r / 4, gy + r);
          RECT neck{gx - r / 4, gy - r, gx + r / 4, gy - r / 4};
          FillRect(dc, &neck, brush);
        } else {
          // Default: a tied pouch.
          Ellipse(dc, gx - r, gy - r / 3, gx + r, gy + r);
          POINT tie[3] = {{gx - r / 3, gy - r / 3}, {gx + r / 3, gy - r / 3},
                         {gx, gy - r}};
          Polygon(dc, tie, 3);
        }
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

  // TASK-0122 Phase A: world-anchored beat legend. Only the capture proof
  // composite populates it; normal play renders nothing here.
  for (const auto& entry : state.beat_legend) {
    const ScreenPoint at =
        project(state.camera, bounds, static_cast<double>(entry.second.x),
                static_cast<double>(entry.second.y));
    HFONT legend_font = CreateFontA(13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                    CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS,
                                    "Verdana");
    HGDIOBJ old_legend_font = SelectObject(dc, legend_font);
    SIZE extent{};
    GetTextExtentPoint32A(dc, entry.first.c_str(),
                          static_cast<int>(entry.first.size()), &extent);
    RECT chip{at.x - extent.cx / 2 - 5, at.y - 24, at.x + extent.cx / 2 + 5,
              at.y - 8};
    HBRUSH chip_bg = CreateSolidBrush(RGB(12, 18, 16));
    FillRect(dc, &chip, chip_bg);
    DeleteObject(chip_bg);
    HPEN chip_edge = CreatePen(PS_SOLID, 1, RGB(104, 160, 137));
    HGDIOBJ old_pen = SelectObject(dc, chip_edge);
    HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(dc, chip.left, chip.top, chip.right, chip.bottom);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(chip_edge);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(226, 238, 230));
    TextOutA(dc, chip.left + 5, chip.top + 2, entry.first.c_str(),
             static_cast<int>(entry.first.size()));
    SelectObject(dc, old_legend_font);
    DeleteObject(legend_font);
    rl.push_back({render::Op::Hud, static_cast<double>(chip.left),
                  static_cast<double>(chip.top), 0.0, 0, "beat:" + entry.first});
  }

  QueryPerformanceCounter(&section_t2);
  state.paint_ms_world = section_ms(section_t1, section_t2);
  paint_minimap(state, dc, bounds, rl);
  paint_vital_orbs(player, world.tick, state.screen_pulse_ticks, dc, bounds, rl,
                   &state.hud_rect_trace);
  paint_quickbar(state, dc, bounds, rl);
  paint_gear_overlay(state, dc, bounds, rl);
  paint_character_pane(state, dc, bounds, rl);
  paint_tree_pane(state, dc, bounds, rl);
  paint_trade_pane(state, dc, bounds, rl);

  if (state.screen_pulse_ticks > 0) {
    // TASK-0122 Phase A: while a ScionLost beat is live the edge pulse is the
    // loss treatment (deeper rust, heavier edge), not the player-damage red.
    bool scion_lost_beat = false;
    for (const auto& fx : state.effects)
      if (fx.kind == EffectFx::Kind::ScionLostBeat) scion_lost_beat = true;
    const COLORREF pulse_color =
        scion_lost_beat
            ? RGB(phase_a::kScionLostColor.r, phase_a::kScionLostColor.g,
                  phase_a::kScionLostColor.b)
            : RGB(196, 46, 40);
    rl.push_back({render::Op::ScreenPulse, 0.0, 0.0, 0.0, 0,
                  scion_lost_beat ? phase_a::kScionLostLabel : "player-damage"});
    HPEN pulse = CreatePen(PS_SOLID, scion_lost_beat ? 14 : 10, pulse_color);
    HGDIOBJ old_pen = SelectObject(dc, pulse);
    HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    RECT inner{4, 4, bounds.right - 4, bounds.bottom - 4};
    Rectangle(dc, inner.left, inner.top, inner.right, inner.bottom);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(pulse);
  }

  // TASK-0153 rev2: every normal-HUD top region is measured, then placed by
  // the single pure planner pass, then drawn exactly where it was placed.
  {
    std::string objective;
    COLORREF accent = RGB(120, 214, 168);
    const bool carrying = !world.carried.empty() || world.carried_trophies > 0;
    if (!world.has_extraction) {
      // In town the NPC roster is the tell; guide toward the story loop
      // instead of the placeholder explore line.
      objective = !world.npcs.empty()
                      ? "objective: hail an NPC with T - press N to take the tin road"
                      : "objective: explore the route";
    } else if (world.expedition_phase == ExpeditionPhaseView::SlayWardens) {
      objective = "objective: slay the wardens (" +
                  std::to_string(world.monsters.size()) + " remain)";
      accent = RGB(214, 92, 72);
    } else {
      const int ddx = world.extraction.x - player.position.x;
      const int ddy = world.extraction.y - player.position.y;
      const int dist = static_cast<int>(
          std::lround(std::sqrt(static_cast<double>(ddx * ddx + ddy * ddy))));
      objective = std::string(carrying
                                  ? "objective: carry your loot to the EXIT ("
                                  : "objective: reach the EXIT (");
      objective += compass_step(ddx, ddy);
      objective += ", " + std::to_string(dist) + "u) - ";
      objective += extraction_action_hint(is_remote(state));
      if (carrying) accent = RGB(239, 208, 116);
    }

    // TASK-0159: house().name is already prefixed ("House Verdigris") — the
    // leading literal here painted "House House Verdigris" on the shipped HUD.
    const std::string identity =
        world.house_name + " - Scion " +
        (world.scion_name.empty() ? std::string("(unnamed)") : world.scion_name);
    static constexpr char kControls[] =
        "WASD move | mouse aim | LMB attack | RMB/Space dash | Q E R skills | "
        "X take | Z names | I gear | T hail | N road";
    const std::string& art_text = state.billboards.status;

    // TASK-0159: pre-measure the controls hint and its deterministic
    // mid-separator wrap (the " | " boundary nearest the middle) so the
    // planner can stack the hint when no single line fits.
    const std::string controls_full(kControls);
    std::string controls_line_a = controls_full;
    std::string controls_line_b;
    {
      std::size_t best = std::string::npos;
      std::size_t pos = controls_full.find(" | ");
      const long middle = static_cast<long>(controls_full.size() / 2);
      while (pos != std::string::npos) {
        if (best == std::string::npos ||
            std::labs(static_cast<long>(pos) - middle) <
                std::labs(static_cast<long>(best) - middle))
          best = pos;
        pos = controls_full.find(" | ", pos + 1);
      }
      if (best != std::string::npos) {
        controls_line_a = controls_full.substr(0, best);
        controls_line_b = controls_full.substr(best + 3);
      }
    }
    SIZE controls_a_extent{}, controls_b_extent{};
    GetTextExtentPoint32A(dc, controls_line_a.c_str(),
                          static_cast<int>(controls_line_a.size()),
                          &controls_a_extent);
    if (!controls_line_b.empty())
      GetTextExtentPoint32A(dc, controls_line_b.c_str(),
                            static_cast<int>(controls_line_b.size()),
                            &controls_b_extent);

    SIZE identity_extent{}, objective_extent{}, art_extent{}, controls_extent{};
    GetTextExtentPoint32A(dc, identity.c_str(),
                          static_cast<int>(identity.size()), &identity_extent);
    GetTextExtentPoint32A(dc, objective.c_str(),
                          static_cast<int>(objective.size()), &objective_extent);
    GetTextExtentPoint32A(dc, art_text.c_str(),
                          static_cast<int>(art_text.size()), &art_extent);
    GetTextExtentPoint32A(dc, kControls,
                          static_cast<int>(sizeof(kControls) - 1),
                          &controls_extent);

    const int width = static_cast<int>(bounds.right);
    const TopHudRect identity_size{0, 0, identity_extent.cx + 6,
                                   identity_extent.cy + 8};
    const TopHudRect objective_size{0, 0, objective_extent.cx + 16,
                                    objective_extent.cy + 8};
    const TopHudRect art_size{0, 0, art_extent.cx + 16, art_extent.cy + 8};
    const TopHudRect controls_size{0, 0, controls_extent.cx + 12,
                                   controls_extent.cy + 6};
    const TopHudRect controls_size_a{
        0, 0, controls_line_b.empty() ? controls_extent.cx + 12
                                      : controls_a_extent.cx + 12,
        controls_line_b.empty() ? controls_extent.cy + 6
                                : controls_a_extent.cy + 6};
    const TopHudRect controls_size_b{
        0, 0, controls_line_b.empty() ? 0 : controls_b_extent.cx + 12,
        controls_line_b.empty() ? 0 : controls_b_extent.cy + 6};
    const TopHudLayout layout = plan_top_hud(
        width, static_cast<int>(bounds.bottom), state.gear_overlay,
        identity_size, objective_size, art_size, controls_size,
        controls_size_a, controls_size_b, static_cast<bool>(state.session));

    // Historical placements double as fallbacks for degenerate widths where
    // the planner cannot fit a region in any row.
    const auto placed_or = [](const TopHudRect& r, int fb_x, int fb_y) {
      return r.w > 0 ? r : TopHudRect{fb_x, fb_y, 0, 0};
    };
    const TopHudRect objective_at = placed_or(
        layout.objective,
        std::max(12, (width - objective_size.w) / 2), kTopHudRow0Y);
    const TopHudRect connection_at = placed_or(
        layout.connection,
        std::max(18, width - connection_chip_w(static_cast<int>(bounds.bottom)) - 18),
        kTopHudRow0Y);
    const TopHudRect art_at = placed_or(
        layout.art, std::max(12, width - art_size.w - 18),
        state.session ? 38 : 12);
    const TopHudRect controls_at = placed_or(
        layout.controls, std::max(12, (width - controls_size.w) / 2),
        state.session ? 64 : 40);

    SetBkMode(dc, TRANSPARENT);

    if (state.session)
      paint_connection_chip(state, dc, bounds, rl, connection_at.x,
                            connection_at.y);

    paint_status_chip(dc, objective_at.x, objective_at.y, objective, accent, rl);
    state.hud_rect_trace.push_back(
        {"objective",
         {objective_at.x, objective_at.y, objective_size.w, objective_size.h}});

    rl.push_back({render::Op::HouseChip, 0.0, 0.0, 0.0, 0, identity});
    SetTextColor(dc, RGB(140, 208, 172));
    TextOutA(dc, layout.identity.x, layout.identity.y, identity.c_str(),
             static_cast<int>(identity.size()));
    state.hud_rect_trace.push_back(
        {"identity",
         {layout.identity.x, layout.identity.y, identity_extent.cx,
          identity_extent.cy}});

    SetTextColor(dc, RGB(148, 160, 150));
    if (layout.controls_wrapped) {
      TextOutA(dc, controls_at.x, controls_at.y, controls_line_a.c_str(),
               static_cast<int>(controls_line_a.size()));
      state.hud_rect_trace.push_back(
          {"controls",
           {controls_at.x, controls_at.y, controls_a_extent.cx,
            controls_a_extent.cy}});
      TextOutA(dc, layout.controls_second.x, layout.controls_second.y,
               controls_line_b.c_str(),
               static_cast<int>(controls_line_b.size()));
      state.hud_rect_trace.push_back(
          {"controls-second",
           {layout.controls_second.x, layout.controls_second.y,
            controls_b_extent.cx, controls_b_extent.cy}});
    } else {
      TextOutA(dc, controls_at.x, controls_at.y, kControls,
               static_cast<int>(sizeof(kControls) - 1));
      state.hud_rect_trace.push_back(
          {"controls",
           {controls_at.x, controls_at.y, controls_extent.cx,
            controls_extent.cy}});
    }
    rl.push_back({render::Op::Hud, static_cast<double>(controls_at.x),
                  static_cast<double>(controls_at.y), 0.0, 0,
                  std::string("controls: ") + controls_full});

    const bool plates =
        state.billboards.player.ready() && state.billboards.raider.ready() &&
        state.billboards.boss.ready();
    const COLORREF art_accent =
        plates ? RGB(120, 214, 168) : RGB(239, 190, 78);
    paint_status_chip(dc, art_at.x, art_at.y, state.billboards.status,
                      art_accent, rl);
    state.hud_rect_trace.push_back(
        {"art", {art_at.x, art_at.y, art_size.w, art_size.h}});
  }

  // TASK-0145: relic-recovery toast — an authoritative crypt transition is
  // announced by name while exploring.
  if (state.relic_toast_ticks > 0 && !state.relic_toast.empty()) {
    rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0, 0,
                  "relic: " + state.relic_toast});
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(239, 208, 116));
    TextOutA(dc, 18, bounds.bottom - 28, state.relic_toast.c_str(),
             static_cast<int>(state.relic_toast.size()));
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
                  "tick %llu | player %d,%d | zoom %.2f | fps %d | paint %.1fms"
                  " (floor %.1f world %.1f hud %.1f) | effects %zu"
                  " | telegraphs %zu | monsters %zu | npcs %zu",
                  static_cast<unsigned long long>(world.tick),
                  player.position.x, player.position.y,
                  state.camera.zoom, state.fps, state.last_paint_ms,
                  state.paint_ms_floor, state.paint_ms_world, state.paint_ms_hud,
                  state.effects.size(), state.telegraphs.size(),
                  world.monsters.size(), world.npcs.size());
    TextOutA(dc, 18, 144, debug_line, static_cast<int>(strlen(debug_line)));
    char asset_line[256];
    int art_loaded = 0;
    for (const auto& art : state.billboards.item_art)
      if (art.second.ready()) ++art_loaded;
    std::snprintf(asset_line, sizeof(asset_line),
                  "%s | %zu scenery | %s | framekit %s | item art %d",
                  state.billboards.status.c_str(), state.scenery.size(),
                  state.billboards.scenery_status.c_str(),
                  state.billboards.fk_panel.ready() ? "loaded" : "MISSING",
                  art_loaded);
    TextOutA(dc, 18, 168, asset_line, static_cast<int>(strlen(asset_line)));
    int log_y = bounds.bottom - 24;
    for (auto it = state.event_log.rbegin(); it != state.event_log.rend(); ++it) {
      TextOutA(dc, 18, log_y, it->c_str(), static_cast<int>(it->size()));
      log_y -= 20;
    }
  }

  // The hint/message toast is core play feedback — quest dialogue, trade
  // receipts, pickup results — not debug telemetry. It renders on the
  // normal HUD, centered above the quickbar, word-wrapped so long quest
  // lines never run off the window, and never hides behind F3.
  if (state.hint_ticks > 0 && !state.hint.empty()) {
    SetBkMode(dc, TRANSPARENT);
    const int max_width = std::max(160L, bounds.right - 48);
    std::vector<std::string> lines;
    std::string remaining = state.hint;
    while (!remaining.empty() && lines.size() < 4) {
      SIZE full{};
      GetTextExtentPoint32A(dc, remaining.c_str(),
                            static_cast<int>(remaining.size()), &full);
      if (full.cx <= max_width) {
        lines.push_back(remaining);
        break;
      }
      // Longest prefix that fits, broken at the last space when one exists.
      std::size_t fit = remaining.size();
      while (fit > 1) {
        SIZE part{};
        GetTextExtentPoint32A(dc, remaining.c_str(), static_cast<int>(fit), &part);
        if (part.cx <= max_width) break;
        --fit;
      }
      std::size_t cut = remaining.rfind(' ', fit);
      if (cut == std::string::npos || cut == 0) cut = fit;
      lines.push_back(remaining.substr(0, cut));
      remaining = remaining.substr(remaining[cut] == ' ' ? cut + 1 : cut);
    }
    int line_height = 18;
    int widest = 0;
    for (const auto& line : lines) {
      SIZE extent{};
      GetTextExtentPoint32A(dc, line.c_str(), static_cast<int>(line.size()),
                            &extent);
      widest = std::max(widest, static_cast<int>(extent.cx));
      line_height = std::max(line_height, static_cast<int>(extent.cy));
    }
    const int block_height = line_height * static_cast<int>(lines.size());
    const int toast_x =
        std::max(12, static_cast<int>(bounds.right - widest) / 2);
    const HudRect quickbar = quickbar_strip_rect(static_cast<int>(bounds.right),
                                                 static_cast<int>(bounds.bottom));
    const int toast_y = quickbar.y - 16 - block_height;
    RECT plate{toast_x - 14, toast_y - 8, toast_x + widest + 14,
               toast_y + block_height + 8};
    skin::panel(dc, plate, skin::kGold, 240, 7.0f);
    SetTextColor(dc, skin::kGold);
    for (std::size_t i = 0; i < lines.size(); ++i) {
      TextOutA(dc, toast_x, toast_y + static_cast<int>(i) * line_height,
               lines[i].c_str(), static_cast<int>(lines[i].size()));
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

  // Double buffer: draw into a persistent memory bitmap, then blit once.
  if (!state->back_dc || state->back_w != bounds.right ||
      state->back_h != bounds.bottom) {
    if (state->back_dc) {
      SelectObject(state->back_dc, state->back_old);
      DeleteDC(state->back_dc);
      state->back_dc = nullptr;
    }
    if (state->back_bitmap) {
      DeleteObject(state->back_bitmap);
      state->back_bitmap = nullptr;
    }
    state->back_dc = CreateCompatibleDC(dc);
    state->back_bitmap = CreateCompatibleBitmap(dc, bounds.right, bounds.bottom);
    if (!state->back_dc || !state->back_bitmap) return;
    state->back_old = SelectObject(state->back_dc, state->back_bitmap);
    state->back_w = bounds.right;
    state->back_h = bounds.bottom;
  }
  HDC memory_dc = state->back_dc;
  LARGE_INTEGER paint_freq{}, paint_begin{}, paint_end{};
  QueryPerformanceFrequency(&paint_freq);
  QueryPerformanceCounter(&paint_begin);
  paint_scene(*state, memory_dc, bounds);
  QueryPerformanceCounter(&paint_end);
  if (paint_freq.QuadPart > 0)
    state->last_paint_ms = 1000.0 *
                           static_cast<double>(paint_end.QuadPart - paint_begin.QuadPart) /
                           static_cast<double>(paint_freq.QuadPart);
  state->paint_ms_hud = state->last_paint_ms - state->paint_ms_floor -
                        state->paint_ms_world;
  ++state->fps_frames;
  if (state->fps_window_qpc == 0) {
    state->fps_window_qpc = paint_end.QuadPart;
  } else if (paint_end.QuadPart - state->fps_window_qpc >=
             paint_freq.QuadPart) {
    state->fps = state->fps_frames;
    state->fps_frames = 0;
    state->fps_window_qpc = paint_end.QuadPart;
  }
  BitBlt(dc, 0, 0, bounds.right, bounds.bottom, memory_dc, 0, 0, SRCCOPY);
}

// The fixed 50 ms game tick: movement/aim sampling at the server's exact
// cadence, event ingestion, and tick-denominated presentation aging. This
// must never run faster than 20 Hz — the wire's movement sampling contract
// and every ttl/tick constant depend on it.
void fixed_game_tick(ClientState& state, const RECT& bounds) {
  if (state.session) {
    sync_world(state);
    ingest_session_events(state);
    update_screen_for_model(state);
    watch_crypt_statuses(state);
    // Regenerate landmark scenery whenever the authoritative scene changes
    // (login included - transition envelopes never fire for the first town).
    const std::string& scene = state.session->model().player.scene_id;
    if (!scene.empty() && scene != state.scenery_scene) {
      state.scenery_scene = scene;
      generate_scenery(state);
    }
  }
  if (state.relic_toast_ticks > 0) --state.relic_toast_ticks;

  const bool at_front_door =
      state.screen == Screen::Chronicles && state.session != nullptr;
  int dx = (state.d ? 1 : 0) - (state.a ? 1 : 0);
  int dy = (state.s ? 1 : 0) - (state.w ? 1 : 0);
  const bool moving = dx != 0 || dy != 0;
  if (at_front_door) {
    // The front door consumes movement: no world input exists pre-admission.
  } else if (state.session) {
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

  if (!at_front_door) dispatch_aim_if_changed(state, bounds, !moving && state.was_moving);
  state.was_moving = moving;

  ingest_events(state, bounds);

  for (auto& fx : state.effects) ++fx.age;
  state.effects.erase(std::remove_if(state.effects.begin(), state.effects.end(),
                                     [](const EffectFx& fx) { return fx.age >= fx.ttl; }),
                      state.effects.end());
  if (state.hint_ticks > 0) --state.hint_ticks;
  if (state.screen_pulse_ticks > 0) --state.screen_pulse_ticks;
}

// Per-frame pump (~66 Hz timer): drain the socket, run as many fixed ticks
// as wall time owes, then smooth the camera with a dt-correct factor. The
// 20 FPS presentation came from one 50 ms timer driving both simulation
// cadence AND rendering; they are now decoupled.
void timer_step(HWND window, ClientState& state) {
  RECT bounds;
  GetClientRect(window, &bounds);

  LARGE_INTEGER freq{}, now{};
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&now);
  double dt_ms = 15.0;
  if (state.last_frame_qpc != 0 && freq.QuadPart > 0) {
    dt_ms = 1000.0 *
            static_cast<double>(now.QuadPart - state.last_frame_qpc) /
            static_cast<double>(freq.QuadPart);
    dt_ms = std::clamp(dt_ms, 0.0, 250.0);
  }
  state.last_frame_qpc = now.QuadPart;

  if (state.session) state.session->poll();
  state.tick_accum_ms += dt_ms;
  while (state.tick_accum_ms >= 50.0) {
    state.tick_accum_ms -= 50.0;
    fixed_game_tick(state, bounds);
  }

  sync_world(state);
  {
    // Follow smoothing (dt-correct exponential, equal to the historical 0.2
    // per 50 ms). Across a scene load the camera starts continents away —
    // snap instead of panning the whole map past the player.
    const double gap_x = state.world.player.position.x - state.camera.x;
    const double gap_y = state.world.player.position.y - state.camera.y;
    const double snap_gap =
        static_cast<double>(verdigris::world_scale::kArenaHalfExtent);
    if (std::abs(gap_x) > snap_gap || std::abs(gap_y) > snap_gap) {
      state.camera.x = static_cast<double>(state.world.player.position.x);
      state.camera.y = static_cast<double>(state.world.player.position.y);
    } else {
      const double keep = std::pow(0.8, dt_ms / 50.0);
      state.camera.x += gap_x * (1.0 - keep);
      state.camera.y += gap_y * (1.0 - keep);
    }
  }
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

// TASK-0153: production gear-pane toggle, shared verbatim by the Win32 key
// path and the scenario harness ('I' opens/closes; Esc closes when open).
void toggle_gear_overlay(ClientState& state) {
  sync_world(state);
  state.gear_overlay = !state.gear_overlay;
  state.selected_item = 0;
  if (state.gear_overlay) show_hint(state, "Gear opened");
}

// TASK-0153: the one Escape contract for every screen. A dismissible pane
// (gear/inventory) consumes the first press and stays in the session; only
// with nothing open does Escape request exit via ClientState::quit_requested,
// which the window procedure turns into PostQuitMessage.
bool trade_pane_open(const ClientState& state) {
  if (!state.session) return false;
  const auto& model = state.session->model();
  return model.shop.open || model.bank.open;
}

void handle_escape_key(ClientState& state) {
  if (trade_pane_open(state)) {
    state.session->submit(verdigris::client::ClientCommand::close_screen());
    state.trade_selected = 0;
    return;
  }
  if (state.tree_pane) {
    state.tree_pane = false;
    return;
  }
  if (state.character_pane) {
    state.character_pane = false;
    return;
  }
  if (state.gear_overlay) {
    toggle_gear_overlay(state);
    return;
  }
  state.quit_requested = true;
}

// Apply the current window mode: borderless fullscreen on the primary
// monitor, or a movable 1280x800 window. F11 toggles between them.
void apply_window_mode(HWND window, const ClientState& state) {
  if (state.fullscreen_window) {
    SetWindowLongPtr(window, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(window, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
  } else {
    SetWindowLongPtr(window, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    SetWindowPos(window, nullptr, 120, 120, 1280, 800,
                 SWP_FRAMECHANGED | SWP_NOZORDER);
  }
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  ClientState* state = state_from(window);
  switch (message) {
    case WM_NCCREATE: {
      auto* create = reinterpret_cast<CREATESTRUCT*>(lparam);
      SetWindowLongPtr(window, GWLP_USERDATA,
                       reinterpret_cast<LONG_PTR>(create->lpCreateParams));
      // DefWindowProc must still run WM_NCCREATE: it stores the window title
      // passed to CreateWindowExA. Returning TRUE directly left every client
      // window nameless in the taskbar and to other tools.
      return DefWindowProc(window, message, wparam, lparam);
    }
    case WM_KEYDOWN:
      if (!state) break;
      if (wparam == VK_F3) {
        state->debug_overlay = !state->debug_overlay;
        break;
      }
      if (wparam == VK_F11) {
        state->fullscreen_window = !state->fullscreen_window;
        apply_window_mode(window, *state);
        RECT mode_bounds;
        GetClientRect(window, &mode_bounds);
        state->camera.zoom =
            kCameraDefaultZoom *
            zoom_height_factor(static_cast<int>(mode_bounds.bottom));
        break;
      }
      if (wparam == VK_ESCAPE) {
        // TASK-0153: dismiss an open pane first; exit only on a bare Escape.
        handle_escape_key(*state);
        if (state->quit_requested) PostQuitMessage(0);
        break;
      }
      if (state->screen == Screen::Chronicles && state->session) {
        handle_chronicles_key(*state, wparam);
        break;
      }
      if (trade_pane_open(*state)) {
        if (wparam == VK_UP && state->trade_selected > 0) --state->trade_selected;
        if (wparam == VK_DOWN) ++state->trade_selected;  // painter clamps
        if (wparam == VK_RETURN) {
          for (const auto& hit : state->trade_row_hits) {
            if (hit.index == state->trade_selected) {
              activate_trade_row(*state, hit);
              break;
            }
          }
        }
        if (wparam == VK_UP || wparam == VK_DOWN || wparam == VK_RETURN) {
          break;
        }
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
          // The server's Take requires a real uuid and chebyshev<=1 reach;
          // an empty uuid was silently ignored, which read as 'cannot pick
          // up loot'. Choose the nearest authoritative ground item in reach.
          const auto& model = state->session->model();
          const double px = model.player.x;
          const double py = model.player.y;
          const verdigris::client::ClientGroundItem* nearest = nullptr;
          double best = 1.6;  // tiles; server allows chebyshev<=1 from tile
          for (const auto& item : model.ground) {
            const double reach = std::max(std::abs(item.x - px),
                                          std::abs(item.y - py));
            if (reach < best) { best = reach; nearest = &item; }
          }
          if (nearest) {
            submit_pick_up(*state, nearest->uuid);
          } else if (!model.ground.empty()) {
            show_hint(*state, "No drop within reach");
          } else {
            show_hint(*state, "Nothing on the ground here");
          }
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
      if (wparam == 'T' && state->session) {
        sync_world(*state);
        const verdigris::client::WorldNpc* nearest = nullptr;
        double best = std::numeric_limits<double>::max();
        for (const auto& npc : state->world.npcs) {
          const double ddx = std::abs(static_cast<double>(
              npc.position.x - state->world.player.position.x));
          const double ddy = std::abs(static_cast<double>(
              npc.position.y - state->world.player.position.y));
          const double reach = std::max(ddx, ddy);
          if (reach < best) { best = reach; nearest = &npc; }
        }
        if (nearest && best <= kTileUnits * 1.9) {
          // Prefer the story verb, then commerce; mirror the server's
          // action-id table so the request lands on the real handler.
          std::string verb = nearest->actions.empty() ? "examine" : nearest->actions.front();
          for (const char* preferred : {"talk", "trade", "bank"}) {
            if (std::find(nearest->actions.begin(), nearest->actions.end(), preferred) !=
                nearest->actions.end()) { verb = preferred; break; }
          }
          const char* action_id = verb == "talk" ? "player:npc:talk"
                                : verb == "trade" ? "player:npc:trade"
                                : verb == "bank" ? "player:screen:bank"
                                : "player:npc:examine";
          state->session->submit(
              verdigris::client::ClientCommand::npc_action(nearest->id, action_id));
          show_hint(*state, "You hail " + nearest->name);
        } else if (!state->world.npcs.empty()) {
          show_hint(*state, "No one is within hailing distance");
        }
      }
      if (wparam == 'F') {
        submit_extract(*state);
        show_hint(*state, "Contextual interaction requested");
      }
      if (wparam == 'I') {
        toggle_gear_overlay(*state);
      }
      if (wparam == 'C') {
        state->character_pane = !state->character_pane;
      }
      if (wparam == 'M' && state->audio_sink) {
        state->audio_sink->set_muted(!state->audio_sink->muted());
        show_hint(*state, state->audio_sink->muted() ? "Sound muted"
                                                     : "Sound on");
      }
      if (wparam == 'P') {
        state->tree_pane = !state->tree_pane;
      }
      // Only the open gear pane needs a fresh view here; a bare sync on
      // every auto-repeating WASD keydown is per-input work the frame loop
      // pays for.
      if (state->gear_overlay) sync_world(*state);
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
        RECT home_bounds;
        GetClientRect(window, &home_bounds);
        state->camera.zoom =
            kCameraDefaultZoom *
            zoom_height_factor(static_cast<int>(home_bounds.bottom));
      }
      break;
    case WM_KEYUP:
      if (!state) break;
      if (wparam == 'W') state->w = false;
      if (wparam == 'A') state->a = false;
      if (wparam == 'S') state->s = false;
      if (wparam == 'D') state->d = false;
      break;
    case WM_MOUSEMOVE:
      // Store only. A gaming mouse delivers WM_MOUSEMOVE at up to 1000 Hz;
      // any per-event simulation or invalidation work here floods the queue
      // and starves WM_PAINT/WM_TIMER (both lowest-priority), which is how
      // the client ground to a crawl under move+attack input. The 20 Hz tick
      // dispatches aim from the stored position and repaints.
      if (state) {
        state->mouse.x = GET_X_LPARAM(lparam);
        state->mouse.y = GET_Y_LPARAM(lparam);
      }
      break;
    case WM_MOUSEWHEEL:
      if (state) {
        const int delta = GET_WHEEL_DELTA_WPARAM(wparam);
        const double factor = delta > 0 ? 1.1 : 1.0 / 1.1;
        RECT zoom_bounds;
        GetClientRect(window, &zoom_bounds);
        const double zf = zoom_height_factor(static_cast<int>(zoom_bounds.bottom));
        state->camera.zoom = std::clamp(state->camera.zoom * factor,
                                        kCameraMinZoom * zf, kCameraMaxZoom * zf);
      }
      break;
    case WM_LBUTTONDOWN:
      if (state) {
        if (trade_pane_open(*state)) {
          const int mx = GET_X_LPARAM(lparam);
          const int my = GET_Y_LPARAM(lparam);
          for (const auto& hit : state->trade_row_hits) {
            if (mx >= hit.rect.left && mx < hit.rect.right &&
                my >= hit.rect.top && my < hit.rect.bottom) {
              state->trade_selected = hit.index;
              activate_trade_row(*state, hit);
              break;
            }
          }
        } else if (state->tree_pane) {
          const int mx = GET_X_LPARAM(lparam);
          const int my = GET_Y_LPARAM(lparam);
          for (const auto& hit : state->tree_seat_hits) {
            const long long dx = mx - hit.x;
            const long long dy = my - hit.y;
            if (dx * dx + dy * dy >
                static_cast<long long>(hit.radius) * hit.radius)
              continue;
            if (hit.frontier && state->session) {
              state->session->submit(
                  verdigris::client::ClientCommand::allocate_node(hit.node_id));
              show_hint(*state, "The lattice takes the point");
            }
            break;
          }
        } else if (state->gear_overlay) {
          equip_selected(*state);
        } else {
          // Route through dispatch_skill so LMB gets the same instant
          // swing-arc feedback as the Q/E/R keys — the primary attack was
          // the one input with no animation at all.
          static constexpr SkillInfo kPrimaryStrike{'\0', "Strike",
                                                    verdigris::ActionType::Melee};
          dispatch_skill(*state, kPrimaryStrike);
        }
      }
      break;
    case WM_RBUTTONDOWN:
      if (state) dispatch_dash(*state);
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

// Deterministic phase-transition driver for scenarios that must reach the
// authoritative carry-to-exit phase. Mirrors the accepted core-test
// discipline (core_tests.cpp drive_expedition): each living warden is brought
// into forward melee reach at one life so the Scion never leaves its approach
// line, then the kill runs through the REAL scenario_step pipeline (dispatch
// → events → present). An owed pack that has not yet materialized is advanced
// with Wait ticks. Returns true when the core itself reports
// ExtractCarriedValue; false on death or timeout.
bool drive_to_extraction_phase(ClientState& state) {
  auto* sim = state.simulation.get();
  const int reach = verdigris::world_scale::kMeleeRange - 1;
  for (int round = 0; round < 16; ++round) {
    sync_world(state);
    if (state.world.expedition_phase == ExpeditionPhaseView::ExtractCarriedValue)
      return true;
    verdigris::Actor* player = sim->actor(sim->scion().actor_id);
    if (!player || !player->alive) return false;
    std::string target_id;
    for (const auto& candidate : sim->actors())
      if (candidate.kind == verdigris::ActorKind::Monster && candidate.alive) {
        target_id = candidate.id;
        break;
      }
    if (target_id.empty()) {
      scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait));
      continue;
    }
    verdigris::Actor* target = sim->actor(target_id);
    player = sim->actor(sim->scion().actor_id);
    target->position = {player->position.x + reach, player->position.y};
    target->stats.life = 1;
    player->cooldown_ticks = 0;
    scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Melee));
  }
  sync_world(state);
  return state.world.expedition_phase ==
         ExpeditionPhaseView::ExtractCarriedValue;
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

  // TASK-0153: the strip is phase-truthful now, so this journey must actually
  // finish the slay leg (through the same paced real pipeline) before the
  // carry-to-exit guidance is the authoritative thing to say.
  drive_to_extraction_phase(state);

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

// ── TASK-0145: chronicles-gate-b scenario ───────────────────────────────
// Drives the FULL owner screen-state journey against a real remote session
// and a real in-process protocol server bound to this lane's loopback
// capsule (6780-6799): front door → found → create → mortal oath →
// admission → expedition → fatal fall → succession → relic recovery →
// reconnect roster restore. Asserts screen transitions and actionable
// controls, never just parsed fields.

bool reference_present(ClientState& state, int width, int height,
                       const std::string& png_path);

bool chronicles_pump(ClientState& state, int max_ticks,
                     const std::function<bool()>& done) {
  for (int i = 0; i < max_ticks; ++i) {
    state.session->poll();
    ingest_session_events(state);
    update_screen_for_model(state);
    watch_crypt_statuses(state);
    if (state.relic_toast_ticks > 0) --state.relic_toast_ticks;
    for (auto& fx : state.effects) ++fx.age;
    state.effects.erase(std::remove_if(state.effects.begin(), state.effects.end(),
                                       [](const EffectFx& fx) { return fx.age >= fx.ttl; }),
                        state.effects.end());
    if (state.screen_pulse_ticks > 0) --state.screen_pulse_ticks;
    if (done()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  return done();
}

bool render_list_has(const ClientState& state, render::Op op,
                     const std::string& prefix) {
  for (const auto& item : state.render_list) {
    if (item.op != op) continue;
    if (prefix.empty() || item.label.rfind(prefix, 0) == 0) return true;
  }
  return false;
}

void fire_chronicle_action(ClientState& state, const std::string& command,
                           const std::string& arg = "") {
  state.chronicles_menu = chronicle_actions(state);
  for (const auto& action : state.chronicles_menu) {
    if (action.command != command) continue;
    if (!arg.empty() && action.arg != arg) continue;
    submit_chronicle_action(state, action);
    return;
  }
}

// Test-harness escape hatch: dev:* control-surface envelopes through the real
// remote session (same seam the remote render-list scenario's transport
// exposes). Production presentation code never calls these.
void send_dev_envelope(ClientState& state, const char* event,
                       verdigris::networking::JsonValue::Object fields = {}) {
  auto* remote =
      static_cast<verdigris::client::RemoteProtocolSession*>(state.session.get());
  remote->send_raw(event, verdigris::networking::JsonValue(std::move(fields)));
}

int scenario_chronicles_gate_b() {
  verdigris::networking::WebSocketServer* server = nullptr;
  std::uint16_t port = 0;
  for (std::uint16_t candidate = 6780; candidate <= 6799; ++candidate) {
    auto* probe = new verdigris::networking::WebSocketServer(candidate);
    std::string error;
    if (probe->start(&error)) {
      server = probe;
      port = candidate;
      break;
    }
    delete probe;
  }
  scenario_check(server != nullptr, "chronicles-gate-b: bound ox-pc-i capsule server");
  if (!server) return 0;

  ClientState state;
  state.chronicles_mode = true;
  state.screen = Screen::Chronicles;
  load_billboards(state.billboards);
  const std::string capture_dir = chronicles_capture_dir();
  if (capture_dir.empty()) {
    // TASK-0161: a rejected capture root fails before any evidence write.
    scenario_check(false, "evidence: capture root rejected before any write");
    return 0;
  }

  using verdigris::client::ClientCommand;
  using verdigris::client::ConnectionState;
  state.session = std::make_unique<verdigris::client::RemoteProtocolSession>(
      "127.0.0.1", port, "ox-pc-i-gate-b", false);
  std::string error;
  scenario_check(state.session->start(&error), "chronicles-gate-b: session start");

  // 1) The coherent pre-game screen: account payload opens the front door.
  const bool door_ready = chronicles_pump(
      state, 250, [&] { return state.session->model().chronicle.present; });
  scenario_check(door_ready, "front door: the account chronicle opens the door");
  scenario_present(state);
  scenario_check(render_list_has(state, render::Op::Chronicles, "title"),
                 "front door: the Chronicles title is on screen");
  scenario_check(render_list_has(state, render::Op::Chronicles, "action:found-house"),
                 "front door: founding a House is an actionable control");

  // 2) Found the House.
  fire_chronicle_action(state, "found-house");
  const bool house_ok = chronicles_pump(state, 250, [&] {
    return state.session->model().chronicle.houses.size() == 1;
  });
  scenario_check(house_ok, "front door: founding renders the House roster");
  scenario_present(state);
  scenario_check(render_list_has(state, render::Op::Chronicles, "house "),
                 "front door: the new House is named on screen");
  scenario_check(render_list_has(state, render::Op::Chronicles, "action:create-scion"),
                 "front door: naming a Scion is offered");

  // 3) Create the first Scion; the oath field starts soft.
  fire_chronicle_action(state, "create-scion");
  const bool scion_ok = chronicles_pump(state, 250, [&] {
    const auto& houses = state.session->model().chronicle.houses;
    return !houses.empty() && houses.front().scions.size() == 1;
  });
  scenario_check(scion_ok, "front door: the first Scion joins the roster");
  scenario_present(state);
  scenario_check(render_list_has(state, render::Op::Chronicles, "action:set-out:"),
                 "front door: set-out is actionable for the new Scion");
  scenario_check(render_list_has(state, render::Op::Chronicles, "oath:off"),
                 "front door: the mortal-oath field renders its soft state");

  // 4) Arm the mortal oath and admit through the oath-bearing select path.
  fire_chronicle_action(state, "oath-toggle");
  scenario_present(state);
  scenario_check(render_list_has(state, render::Op::Chronicles, "oath:on"),
                 "front door: the mortal-oath field arms on demand");
  const std::string first_scion_id =
      state.session->model().chronicle.houses.front().scions.front().id;
  fire_chronicle_action(state, "select-scion", first_scion_id);
  const bool admitted = chronicles_pump(state, 250, [&] {
    return state.session->connection_state() == ConnectionState::Ready &&
           !state.session->model().chronicles_pending &&
           state.session->model().player.alive &&
           state.screen == Screen::Expedition;
  });
  scenario_check(admitted, "admission: the mortal-oath select lands in the world");
  scenario_follow_camera(state);
  scenario_present(state);
  scenario_check(!render_list_has(state, render::Op::Chronicles, "title"),
                 "admission: the front door is dismissed");
  scenario_check(render_list_has(state, render::Op::HouseChip, "House "),
                 "admission: the expedition names the House and Scion");

  // 5) Take the road: the expedition HUD keeps the TASK-0142 presentation.
  state.session->submit(ClientCommand::enter_zone("tin:1:0"));
  const bool in_instance = chronicles_pump(state, 250, [&] {
    return state.session->model().scene.type == "instance";
  });
  scenario_check(in_instance, "expedition: the road instance is entered");
  generate_scenery(state);
  scenario_follow_camera(state);
  scenario_present(state);
  scenario_check(render::any(state.render_list, render::Op::Floor),
                 "expedition: the route renders with the existing presentation");

  // 6) Earn gear so the fatal fall commits a real heirloom to circulation.
  // dev:give runs the real inventory pipeline; the seeded grant keeps the
  // journey deterministic. Only starter coins/daggers are exempt from
  // circulation, so this sword is exactly what the fall will commit.
  send_dev_envelope(state, "dev:give",
                    {{"itemId", verdigris::networking::JsonValue("bronze-sword")},
                     {"seed", verdigris::networking::JsonValue(20260822)}});
  const bool sword_ok = chronicles_pump(state, 250, [&] {
    for (const auto& item : state.session->model().inventory)
      if (item.id == "bronze-sword") return true;
    return false;
  });
  scenario_check(sword_ok, "expedition: earned gear enters the inventory");

  // 7) The fatal fall: server-authoritative final death for the mortal oath.
  send_dev_envelope(state, "dev:kill");
  const bool fallen = chronicles_pump(state, 250, [&] {
    return state.session->model().chronicle.fallen.scion_id == first_scion_id;
  });
  scenario_check(fallen, "consequence: scion-fallen names the fallen Scion");
  scenario_check(state.screen == Screen::Chronicles,
                 "consequence: the fall returns the owner to the chronicles");
  chronicles_pump(state, 40, [&] { return false; });  // let dev:state refresh the crypt
  scenario_present(state);
  scenario_check(render_list_has(state, render::Op::Chronicles, "fallen:"),
                 "consequence: the fall is recorded on the front door");
  scenario_check(render_list_has(state, render::Op::Chronicles, "crypt " + first_scion_id),
                 "consequence: the fallen Scion rests in the crypt");
  scenario_check(render_list_has(state, render::Op::Chronicles, "action:create-scion"),
                 "succession: naming a successor is actionable after the fall");
  const bool door_png = reference_present(state, 960, 600,
                                          capture_dir + "\\front-door-960x600.png");
  scenario_check(door_png, "evidence: front-door capture written");

  // 8) Succession: the heir is admitted through the succession select path
  // (the only wire admission that resets a permadead lifecycle), with the
  // oath disarmed so the soft-heir journey is what ships.
  fire_chronicle_action(state, "create-scion");
  const bool successor_ok = chronicles_pump(state, 250, [&] {
    const auto& houses = state.session->model().chronicle.houses;
    return !houses.empty() && !houses.front().scions.empty() &&
           houses.front().scions.back().id != first_scion_id;
  });
  scenario_check(successor_ok, "succession: the heir joins the living roster");
  const std::string successor_id =
      state.session->model().chronicle.houses.front().scions.back().id;
  const std::string successor_name =
      state.session->model().chronicle.houses.front().scions.back().name;
  scenario_present(state);
  scenario_check(
      render_list_has(state, render::Op::Chronicles, "action:select-scion:" + successor_id),
      "succession: heirship admission is actionable without the oath");
  fire_chronicle_action(state, "oath-toggle");  // disarm for the soft heir
  fire_chronicle_action(state, "select-scion", successor_id);
  const bool admitted_heir = chronicles_pump(state, 250, [&] {
    return state.session->connection_state() == ConnectionState::Ready &&
           !state.session->model().chronicles_pending &&
           state.session->model().player.alive &&
           state.screen == Screen::Expedition;
  });
  scenario_check(admitted_heir,
                 "succession: the heirship select admits the successor");
  // RECORDED RED (TASK-0081 discipline): on the current tip
  // player:chronicles:select resets the lifecycle but not the Simulation
  // actor's life, so a successor inherits a zero-life seat until a fresh
  // socket heal. Continue every independent UI state via the accepted
  // dev:heal surface rather than editing forbidden server authority.
  send_dev_envelope(state, "dev:heal");
  const bool heir_out = chronicles_pump(state, 250, [&] {
    const auto& model = state.session->model();
    return model.player.alive && model.player.life > 0 &&
           model.lifecycle == "alive" && state.screen == Screen::Expedition;
  });
  scenario_check(heir_out, "succession: the heir takes a healed field");
  scenario_follow_camera(state);
  scenario_present(state);
  {
    bool chip_names_heir = false;
    for (const auto& item : state.render_list)
      if (item.op == render::Op::HouseChip &&
          item.label.find(successor_name) != std::string::npos)
        chip_names_heir = true;
    scenario_check(chip_names_heir, "succession: the identity chip names the heir");
  }

  // 9) Relic recovery: surface the heirloom, take it, watch the crypt flip.
  send_dev_envelope(state, "dev:release-relic");
  const bool surfaced = chronicles_pump(state, 250, [&] {
    for (const auto& item : state.session->model().ground)
      if (item.relic) return true;
    return false;
  });
  scenario_check(surfaced, "recovery: the surfaced heirloom carries its provenance");
  state.session->submit(ClientCommand::pick_up(""));
  const bool recovered = chronicles_pump(state, 250, [&] {
    for (const auto& house : state.session->model().chronicle.houses)
      for (const auto& entry : house.crypt)
        if (entry.id == first_scion_id && entry.relic_status == "recovered") return true;
    return false;
  });
  scenario_check(recovered, "recovery: the crypt record flips lost to recovered");
  scenario_check(!state.relic_toast.empty(),
                 "recovery: the recovery toast names the fallen");
  scenario_follow_camera(state);
  scenario_present(state);
  scenario_check(render_list_has(state, render::Op::Hud, "relic: "),
                 "recovery: the expedition HUD announces the recovery");

  // 10) Reconnect: the existing House/Scion state renders without any login
  // side effects — the front door shows the persisted roster.
  state.session->shutdown();
  state.session.reset();
  state.screen = Screen::Chronicles;
  state.session = std::make_unique<verdigris::client::RemoteProtocolSession>(
      "127.0.0.1", port, "ox-pc-i-gate-b", false);
  scenario_check(state.session->start(&error), "reconnect: session restarts");
  const bool roster_ok = chronicles_pump(state, 250, [&] {
    const auto& chronicle = state.session->model().chronicle;
    if (!chronicle.present) return false;
    bool heir_listed = false;
    for (const auto& house : chronicle.houses) {
      for (const auto& scion : house.scions)
        if (scion.id == successor_id) heir_listed = true;
      for (const auto& entry : house.crypt)
        if (entry.id == first_scion_id && entry.relic_status == "recovered") heir_listed = true;
    }
    return heir_listed && state.screen == Screen::Chronicles;
  });
  scenario_check(roster_ok, "reconnect: House, heir, and crypt render on return");
  scenario_present(state);
  scenario_check(render_list_has(state, render::Op::Chronicles, "scion " + successor_id),
                 "reconnect: the living heir is listed");

  // 11) Re-admit the heir — a living scion's plain outing now takes the
  // chronicles:scion:set-out wire path (road purse) — and capture the
  // expedition HUD evidence.
  fire_chronicle_action(state, "set-out", successor_id);
  const bool heir_again = chronicles_pump(state, 250, [&] {
    const auto& model = state.session->model();
    return state.session->connection_state() == ConnectionState::Ready &&
           !model.chronicles_pending && model.player.alive &&
           model.player.life > 0 && state.screen == Screen::Expedition;
  });
  scenario_check(heir_again, "reconnect: re-admission returns to the expedition");
  scenario_follow_camera(state);
  scenario_present(state);
  const bool hud_png = reference_present(state, 960, 600,
                                         capture_dir + "\\expedition-hud-960x600.png");
  scenario_check(hud_png, "evidence: expedition HUD capture written");

  if (state.session) state.session->shutdown();
  server->stop();
  delete server;
  return 0;
}

// ── TASK-0153: first-session-clarity ────────────────────────────────────
// Proves the three first-session clarity contracts through the REAL
// dispatch/ingest/present pipeline (local simulation and, for the owner
// path, a real remote protocol session on the shared loopback capsule):
//   1. the objective strip names the authoritative expedition phase and
//      flips slay -> carry-to-exit only from authoritative state;
//   2. the exit instruction is mode-aware: local play says "press F there",
//      the remote owner path says "walk onto it" and NEVER shows "press F";
//   3. essential controls (incl. dash) are visible on the normal HUD with
//      F3 disabled.
// Plus the owner Esc contract: an open gear pane consumes Escape and keeps
// the client/session alive; only a bare Escape requests exit. The remote
// half uses dev:clear-floor — the same accepted scenario-shortcut seam as
// gate-b's dev:give/dev:kill — to reach the cleared floor deterministically;
// the strip logic itself is production behavior driven by snapshots.

int scenario_first_session_clarity() {
  auto hud_labels = [](const ClientState& s) {
    std::vector<std::string> labels;
    for (const auto& item : s.render_list)
      if (item.op == render::Op::Hud) labels.push_back(item.label);
    return labels;
  };
  auto hud_contains = [&](const ClientState& s, const char* needle) {
    for (const auto& label : hud_labels(s))
      if (label.find(needle) != std::string::npos) return true;
    return false;
  };
  auto hud_prefixed = [&](const ClientState& s, const char* prefix) {
    for (const auto& label : hud_labels(s))
      if (label.rfind(prefix, 0) == 0) return true;
    return false;
  };

  // ── Local owner path: authoritative core phase drives the strip.
  {
    ClientState state;
    scenario_begin(state);
    scenario_follow_camera(state);
    scenario_present(state);
    scenario_check(hud_prefixed(state, "objective: slay the wardens"),
                   "first-session-clarity: strip names the authoritative slay phase");
    scenario_check(hud_contains(state, "dash"),
                   "first-session-clarity: controls hint includes dash on the normal HUD");
    scenario_check(!state.debug_overlay &&
                       hud_prefixed(state, "controls:"),
                   "first-session-clarity: controls hint visible with F3 disabled");

    // Clear the warden pack; the core flips ExpeditionPhaseChanged when no
    // wardens remain and no roster entry is owed — the strip must follow
    // that authoritative transition.
    const bool flipped = drive_to_extraction_phase(state);
    scenario_check(flipped,
                   "first-session-clarity: phase follows the core to carry-to-exit");
    scenario_present(state);
    const bool exit_guidance =
        hud_prefixed(state, "objective: reach the EXIT") ||
        hud_prefixed(state, "objective: carry your loot");
    scenario_check(exit_guidance,
                   "first-session-clarity: extract strip points at the EXIT locally");
    scenario_check(hud_contains(state, extraction_action_hint(false)),
                   "first-session-clarity: local strip gives the press-F contract");
    scenario_check(!hud_contains(state, extraction_action_hint(true)),
                   "first-session-clarity: local strip never shows the walk-on phrase");

    // Owner Esc contract through the same production seams the Win32 path
    // calls: open pane consumes Escape; bare Escape requests exit.
    toggle_gear_overlay(state);
    scenario_present(state);
    scenario_check(render::any(state.render_list, render::Op::PaneWeapon),
                   "first-session-clarity: gear pane renders when opened");
    handle_escape_key(state);
    scenario_check(!state.gear_overlay && !state.quit_requested,
                   "first-session-clarity: first Esc closes the pane, client stays alive");
    scenario_present(state);
    scenario_check(!render::any(state.render_list, render::Op::PaneWeapon) &&
                       !render::any(state.render_list, render::Op::PaneItem) &&
                       !render::any(state.render_list, render::Op::PaneStat),
                   "first-session-clarity: dismissed pane leaves the render list");
    handle_escape_key(state);
    scenario_check(state.quit_requested,
                   "first-session-clarity: bare Escape requests application exit");
  }

  // ── Remote owner path on the shared 6580-6599 test capsule.
  {
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
    scenario_check(server != nullptr,
                   "first-session-clarity: bound shared capsule server "
                   "(if busy, another worker holds 6580-6599 — retry after it clears)");
    if (!server) return 0;

    ClientState state;
    state.session = std::make_unique<verdigris::client::RemoteProtocolSession>(
        "127.0.0.1", port, "ox-pc-v-first-session", true);
    std::string error;
    scenario_check(state.session->start(&error), "first-session-clarity: remote start");
    auto pump_remote = [&](int max_ticks, const std::function<bool()>& done) {
      for (int i = 0; i < max_ticks; ++i) {
        state.session->poll();
        ingest_session_events(state);
        sync_world(state);
        if (done()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }
      return done();
    };
    const bool ready = pump_remote(250, [&] {
      return state.session->connection_state() ==
             verdigris::client::ConnectionState::Ready;
    });
    scenario_check(ready, "first-session-clarity: remote handshake ready");
    state.session->submit(verdigris::client::ClientCommand::enter_zone("tin:1:0"));
    const bool in_instance = pump_remote(250, [&] {
      const auto& model = state.session->model();
      return model.scene.type == "instance" && model.scene.has_stairs_up &&
             !model.monsters.empty();
    });
    scenario_check(in_instance,
                   "first-session-clarity: instance with authoritative foes reached");
    generate_scenery(state);
    scenario_follow_camera(state);
    load_billboards(state.billboards);
    scenario_present(state);
    scenario_check(hud_prefixed(state, "objective: slay the wardens"),
                   "first-session-clarity: remote strip mirrors the foe snapshot phase");
    bool saw_press_f = false;
    for (const auto& item : state.render_list)
      if (item.op == render::Op::Hud &&
          item.label.find(extraction_action_hint(false)) != std::string::npos)
        saw_press_f = true;
    scenario_check(!saw_press_f,
                   "first-session-clarity: NEGATIVE CONTROL - remote HUD never says press F");

    send_dev_envelope(state, "dev:clear-floor");
    const bool cleared = pump_remote(250, [&] {
      for (const auto& monster : state.session->model().monsters)
        if (monster.alive) return false;
      return true;
    });
    scenario_check(cleared, "first-session-clarity: authoritative snapshot clears the floor");
    scenario_follow_camera(state);
    scenario_present(state);
    const bool walk_on =
        (hud_prefixed(state, "objective: reach the EXIT") ||
         hud_prefixed(state, "objective: carry your loot")) &&
        hud_contains(state, extraction_action_hint(true));
    scenario_check(walk_on,
                   "first-session-clarity: remote extract strip gives the walk-on contract");
    scenario_check(!hud_contains(state, extraction_action_hint(false)),
                   "first-session-clarity: remote strip still never says press F after clearing");

    if (state.session) state.session->shutdown();
    server->stop();
    delete server;
  }
  return 0;
}

int scenario_first_session_clarity();

// TASK-0122 Phase A contract scenario; defined after reference_present so it
// can emit fresh PNG evidence into this task's captures/ folder.
int scenario_animation_vfx_phase_a();

// TASK-0156 progression-surface scenario; defined below so it can emit fresh
// PNG evidence into this task's captures/ folder.
int scenario_progression_surface();

// ── TASK-0156: progression-surface ──────────────────────────────────────
// Proves the shipped gear overlay mirrors ONLY the authoritative passiveTree
// payload: absent before any payload arrives, nonzero from a real quick-guest
// admission, zero from a real server-committed tree snapshot. Every payload
// flows through the production parser/presentation seam against an in-process
// protocol server bound to this lane's routed loopback capsule (7120-7139).
// The player:skilltree:save envelope below is the existing browser-wire event
// driven through the documented test-harness escape hatch so the ZERO state
// is produced by the real authority, never fabricated by the test.
int scenario_progression_surface() {
  verdigris::networking::WebSocketServer* server = nullptr;
  std::uint16_t port = 0;
  for (std::uint16_t candidate = 7120; candidate <= 7139; ++candidate) {
    auto* probe = new verdigris::networking::WebSocketServer(candidate);
    std::string error;
    if (probe->start(&error)) {
      server = probe;
      port = candidate;
      break;
    }
    delete probe;
  }
  scenario_check(server != nullptr,
                 "progression-surface: bound ox-pc-aa capsule server");
  if (!server) return 0;

  using verdigris::client::ClientCommand;
  using verdigris::client::ConnectionState;
  using verdigris::networking::JsonValue;
  ClientState state;
  state.session = std::make_unique<verdigris::client::RemoteProtocolSession>(
      "127.0.0.1", port, "progression-surface-0156", true);
  load_billboards(state.billboards);

  // 1) ABSENT: no passiveTree payload has ever arrived on this connection.
  // The surface must say so instead of rendering zeros.
  scenario_check(!state.session->model().progression.present,
                 "absent: no passiveTree payload has arrived");
  generate_scenery(state);
  sync_world(state);
  state.camera.x = static_cast<double>(state.world.player.position.x);
  state.camera.y = static_cast<double>(state.world.player.position.y);
  state.gear_overlay = true;
  reference_present(state, 960, 600, "");
  scenario_check(
      render_list_has(state, render::Op::PaneStat, "TREE no authoritative data"),
      "absent: the gear pane states absence, not zeros");

  // 2) NONZERO: a real admission carries the authoritative tree payload
  // through the production parser seam.
  std::string error;
  scenario_check(state.session->start(&error),
                 "nonzero: session start");
  bool ready = false;
  for (int i = 0; i < 250 && !ready; ++i) {
    state.session->poll();
    ready = state.session->connection_state() == ConnectionState::Ready;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  scenario_check(ready, "nonzero: handshake ready");
  bool in_instance = false;
  state.session->submit(ClientCommand::enter_zone("tin:1:0"));
  for (int i = 0; i < 80 && !in_instance; ++i) {
    state.session->poll();
    ingest_session_events(state);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    in_instance = state.session->model().scene.type == "instance";
  }
  scenario_check(in_instance, "nonzero: expedition entered");
  generate_scenery(state);
  ingest_session_events(state);
  sync_world(state);
  state.camera.x = static_cast<double>(state.world.player.position.x);
  state.camera.y = static_cast<double>(state.world.player.position.y);
  const auto& earned_view = state.session->model().progression;
  scenario_check(earned_view.present,
                 "nonzero: authoritative payload mirrored into the model");
  scenario_check(earned_view.unspent_points > 0 && earned_view.earned_points > 0,
                 "nonzero: unspent and earned points are nonzero");
  scenario_check(earned_view.unspent_points <= earned_view.earned_points,
                 "nonzero: unspent never exceeds earned");
  scenario_check(earned_view.node_count >= 1,
                 "nonzero: the committed node count mirrors the payload");
  state.gear_overlay = true;
  const std::string dir = progression_capture_dir();
  if (dir.empty()) {
    scenario_check(false,
                   "progression-surface: capture root rejected before any write");
    return 0;
  }
  const std::string png_960 = dir + "\\progression-surface-nonzero-960x600.png";
  reference_present(state, 960, 600, png_960);
  scenario_check(
      render_list_has(state, render::Op::PaneStat,
                      "TREE pts " + std::to_string(earned_view.unspent_points) +
                          "/" + std::to_string(earned_view.earned_points)),
      "nonzero: the pane text shows the authoritative points");
  scenario_check(!render_list_has(state, render::Op::PaneStat,
                                  "no authoritative data"),
                 "nonzero: a present payload never renders as absence");

  // 3) ZERO: a real server-committed snapshot spends every point. The reply
  // (player:skilltree:update) re-mirrors through the same production seam.
  JsonValue snapshot(JsonValue::Object{
      {"nodes",
       JsonValue(JsonValue::Array{JsonValue("0,0"), JsonValue("1,0"),
                                  JsonValue("-1,1")})},
      {"conduits", JsonValue(JsonValue::Array{})},
      {"selectedNodeId", JsonValue("1,0")}});
  send_dev_envelope(state, "player:skilltree:save",
                    {{"snapshot", snapshot}});
  bool zero_landed = false;
  for (int i = 0; i < 250 && !zero_landed; ++i) {
    state.session->poll();
    ingest_session_events(state);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    const auto& progression = state.session->model().progression;
    zero_landed = progression.present && progression.unspent_points == 0 &&
                  progression.node_count == 3 && progression.conduit_count == 0;
  }
  scenario_check(zero_landed,
                 "zero: the committed mirror shows zero unspent points");
  sync_world(state);
  state.gear_overlay = true;
  const std::string png_1366 =
      dir + "\\progression-surface-zero-1366x768.png";
  reference_present(state, 1366, 768, png_1366);
  scenario_check(render_list_has(state, render::Op::PaneStat, "TREE pts 0/"),
                 "zero: genuine zeros are rendered as zeros");
  scenario_check(!render_list_has(state, render::Op::PaneStat,
                                  "no authoritative data"),
                 "zero: zero is not rendered as absence");

  // Capture integrity: both PNGs exist and are non-trivial.
  for (const std::string& path : {png_960, png_1366}) {
    std::ifstream probe(path, std::ios::binary);
    scenario_check(probe.good(), "progression-surface: capture readable");
    probe.seekg(0, std::ios::end);
    const std::streamoff bytes = probe.tellg();
    char line[512];
    std::snprintf(line, sizeof(line), "    capture: %s (%lld bytes)\n",
                  path.c_str(), static_cast<long long>(bytes));
    std::printf("%s", line);
    scenario_check(bytes > 1024, "progression-surface: capture is non-trivial");
  }

  if (state.session) state.session->shutdown();
  server->stop();
  delete server;
  return 0;
}

// TASK-0159: fresh readability evidence lands in THIS task's captures/
// folder for architect visual review.
std::string readability_capture_dir() {
  std::string forced;
  const int overridden = capture_root_override(&forced);
  if (overridden != 0) return overridden > 0 ? forced : std::string{};
  std::vector<std::string> bases{".", executable_directory()};
  const char* marker =
      "orchestration\\tasks\\TASK-0159-native-hud-pane-readability";
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

// ── TASK-0159: hud-pane-readability ─────────────────────────────────────
// Opens the REAL gear pane through the production presentation path at
// 960x600 and 1366x768 and hard-fails on any rectangle intersection between
// the global HUD regions (identity, controls, objective, art/connection
// chips), the pane chrome (title/stats/seat/cells/banked/progression/footer),
// and the fixed combat surfaces (minimap, quickbar, vital orbs). Rectangles
// come from state.hud_rect_trace, recorded beside every draw during the real
// GDI presents, so a suppressed or moved draw cannot fake the proof. The Esc
// contracts are re-proven through the same production seams after each pass.
int scenario_hud_pane_readability() {
  auto trace_find = [](const ClientState& s,
                       const char* label) -> const HudRect* {
    for (const auto& entry : s.hud_rect_trace)
      if (entry.first == label) return &entry.second;
    return nullptr;
  };

  const char* kGlobalRegions[] = {"identity", "controls",     "objective",
                                  "art",      "minimap",      "quickbar-strip",
                                  "orb-life", "orb-resource"};
  const char* kPaneLines[] = {"pane-title",       "pane-stats",
                              "pane-seat",        "pane-banked",
                              "pane-progression", "pane-footer"};

  const struct Size { int w; int h; } sizes[] = {{960, 600}, {1366, 768}};
  const std::string dir = readability_capture_dir();
  if (dir.empty()) {
    scenario_check(false,
                   "hud-pane-readability: capture root rejected before any write");
    return 0;
  }

  // ── Local owner path: both required resolutions, closed then open pane.
  for (const auto& size : sizes) {
    const std::string tag =
        std::to_string(size.w) + "x" + std::to_string(size.h);
    ClientState state;
    scenario_begin(state);
    load_billboards(state.billboards);
    scenario_follow_camera(state);

    auto assert_pairwise_disjoint = [&](const ClientState& s,
                                        const char* const* labels,
                                        int count, const char* scope) {
      for (int i = 0; i < count; ++i)
        for (int j = i + 1; j < count; ++j) {
          const HudRect* a = trace_find(s, labels[i]);
          const HudRect* b = trace_find(s, labels[j]);
          char line[192];
          std::snprintf(line, sizeof(line), "%s: %s vs %s stays clear (%s)",
                        scope, labels[i], labels[j], tag.c_str());
          if (!a || !b) {
            scenario_check(false, line);
            continue;
          }
          scenario_check(!hud_rects_overlap(*a, *b), line);
        }
    };

    // Closed pane: the normal HUD is already collision-free.
    reference_present(state, size.w, size.h, "");
    const std::string png_closed = dir + "\\hud-pane-readability-closed-" +
                                   tag + ".png";
    reference_present(state, size.w, size.h, png_closed);
    scenario_check(trace_find(state, "identity") && trace_find(state, "controls"),
                   ("hud-pane-readability: closed HUD regions recorded (" +
                    tag + ")").c_str());
    assert_pairwise_disjoint(state, kGlobalRegions, 8, "hud-pane-readability");

    // Open the shipped gear pane through the production toggle seam.
    toggle_gear_overlay(state);
    scenario_check(state.gear_overlay,
                   "hud-pane-readability: gear pane opened through the "
                   "production seam");
    const std::string png_open =
        dir + "\\hud-pane-readability-open-" + tag + ".png";
    reference_present(state, size.w, size.h, png_open);

    // The pane really rendered its authoritative content.
    scenario_check(render::any(state.render_list, render::Op::PaneStat) &&
                       render::any(state.render_list, render::Op::PaneWeapon),
                   ("hud-pane-readability: pane content rendered (" + tag + ")")
                       .c_str());

    // No global HUD region may intersect the open pane.
    const HudRect* pane = trace_find(state, "pane-frame");
    scenario_check(pane != nullptr && pane->w > 100,
                   "hud-pane-readability: pane frame recorded");
    for (const char* region : kGlobalRegions) {
      const HudRect* rect = trace_find(state, region);
      char line[192];
      std::snprintf(line, sizeof(line),
                    "hud-pane-readability: %s never enters the gear pane (%s)",
                    region, tag.c_str());
      scenario_check(pane && rect && !hud_rects_overlap(*pane, *rect), line);
    }
    // The wrapped second controls line (when the planner stacked the hint)
    // obeys the same pane exclusion.
    if (const HudRect* second = trace_find(state, "controls-second")) {
      char line[192];
      std::snprintf(line, sizeof(line),
                    "hud-pane-readability: controls-second never enters the "
                    "gear pane (%s)",
                    tag.c_str());
      scenario_check(pane && !hud_rects_overlap(*pane, *second), line);
    }

    // Full mutual clearance still holds among global regions WITH the pane
    // open — the exact gap a fallback pin once slipped through.
    assert_pairwise_disjoint(state, kGlobalRegions, 8, "hud-pane-open");
    bool wrap_clear = true;
    if (const HudRect* second = trace_find(state, "controls-second")) {
      for (const char* region : kGlobalRegions) {
        const HudRect* rect = trace_find(state, region);
        if (rect && hud_rects_overlap(*second, *rect)) wrap_clear = false;
      }
    }
    scenario_check(wrap_clear,
                   ("hud-pane-readability: controls-second clears every "
                    "global region (" + tag + ")").c_str());

    // Pane chrome lines stay mutually clear, including backpack cells.
    assert_pairwise_disjoint(state, kPaneLines, 6, "hud-pane-readability");
    bool cells_clear = true;
    for (const auto& entry : state.hud_rect_trace) {
      if (entry.first != "pane-cell") continue;
      for (const char* line_label : kPaneLines) {
        const HudRect* other = trace_find(state, line_label);
        if (other && hud_rects_overlap(entry.second, *other))
          cells_clear = false;
      }
    }
    scenario_check(cells_clear,
                   ("hud-pane-readability: backpack cells clear of pane "
                    "chrome (" + tag + ")").c_str());

    // Hierarchy without deletion: every authority line survives.
    scenario_check(render_list_has(state, render::Op::HouseChip, "House ") &&
                       !render_list_has(state, render::Op::HouseChip,
                                        "House House"),
                   "hud-pane-readability: identity keeps its single House "
                   "prefix");
    scenario_check(render_list_has(state, render::Op::Hud, "controls:") &&
                       render::any(state.render_list, render::Op::Hud) &&
                       render_list_has(state, render::Op::PaneBanked, "Banked") &&
                       render_list_has(state, render::Op::PaneStat, "TREE"),
                   "hud-pane-readability: controls, banked, and progression "
                   "truth all remain");

    // Owner Esc contracts through the identical production seams.
    handle_escape_key(state);
    scenario_check(!state.gear_overlay && !state.quit_requested,
                   "hud-pane-readability: first Escape closes the pane");
    scenario_present(state);
    scenario_check(!render::any(state.render_list, render::Op::PaneStat) &&
                       !render::any(state.render_list, render::Op::PaneItem),
                   "hud-pane-readability: dismissed pane leaves the render "
                   "list");
    handle_escape_key(state);
    scenario_check(state.quit_requested,
                   "hud-pane-readability: bare Escape requests exit");

    // Capture integrity for this resolution.
    for (const std::string& path : {png_closed, png_open}) {
      std::ifstream probe(path, std::ios::binary);
      scenario_check(probe.good(),
                     ("hud-pane-readability: capture readable (" + tag + ")")
                         .c_str());
      probe.seekg(0, std::ios::end);
      const std::streamoff bytes = probe.tellg();
      char line[512];
      std::snprintf(line, sizeof(line), "    capture: %s (%lld bytes)\n",
                    path.c_str(), static_cast<long long>(bytes));
      std::printf("%s", line);
      scenario_check(bytes > 1024,
                     ("hud-pane-readability: capture non-trivial (" + tag + ")")
                         .c_str());
    }
  }

  // ── Remote owner path on this lane's routed loopback capsule (7100-7119):
  // the connection chip and art chip must also clear the open pane.
  {
    verdigris::networking::WebSocketServer* server = nullptr;
    std::uint16_t port = 0;
    for (std::uint16_t candidate = 7100; candidate <= 7119; ++candidate) {
      auto* probe = new verdigris::networking::WebSocketServer(candidate);
      std::string error;
      if (probe->start(&error)) {
        server = probe;
        port = candidate;
        break;
      }
      delete probe;
    }
    scenario_check(server != nullptr,
                   "hud-pane-readability: bound ox-pc-z capsule server "
                   "(if busy, another process holds 7100-7119)");
    if (server) {
      ClientState state;
      state.session = std::make_unique<verdigris::client::RemoteProtocolSession>(
          "127.0.0.1", port, "ox-pc-z-hud-readability", true);
      load_billboards(state.billboards);
      std::string error;
      scenario_check(state.session->start(&error),
                     "hud-pane-readability: remote start");
      bool ready = false;
      for (int i = 0; i < 250 && !ready; ++i) {
        state.session->poll();
        ready = state.session->connection_state() ==
                verdigris::client::ConnectionState::Ready;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }
      scenario_check(ready, "hud-pane-readability: remote handshake ready");
      state.session->submit(
          verdigris::client::ClientCommand::enter_zone("tin:1:0"));
      bool in_instance = false;
      for (int i = 0; i < 80 && !in_instance; ++i) {
        state.session->poll();
        ingest_session_events(state);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        in_instance = state.session->model().scene.type == "instance";
      }
      scenario_check(in_instance,
                     "hud-pane-readability: remote expedition entered");
      generate_scenery(state);
      sync_world(state);
      state.camera.x = static_cast<double>(state.world.player.position.x);
      state.camera.y = static_cast<double>(state.world.player.position.y);
      toggle_gear_overlay(state);
      reference_present(state, 960, 600, "");
      auto trace_find = [&](const char* label) -> const HudRect* {
        for (const auto& entry : state.hud_rect_trace)
          if (entry.first == label) return &entry.second;
        return nullptr;
      };
      const HudRect* pane = trace_find("pane-frame");
      const HudRect* connection = trace_find("connection");
      const HudRect* art = trace_find("art");
      scenario_check(pane && connection,
                     "hud-pane-readability: remote pane and connection chip "
                     "recorded");
      scenario_check(pane && art,
                     "hud-pane-readability: remote art chip recorded");
      scenario_check(pane && connection &&
                         !hud_rects_overlap(*pane, *connection),
                     "hud-pane-readability: connection chip clears the open "
                     "pane (960x600)");
      scenario_check(pane && art && !hud_rects_overlap(*pane, *art),
                     "hud-pane-readability: art chip clears the open pane "
                     "(960x600)");
      const HudRect* map = trace_find("minimap");
      const HudRect* identity = trace_find("identity");
      scenario_check(map && identity && !hud_rects_overlap(*map, *identity),
                     "hud-pane-readability: identity clears the minimap on "
                     "the remote path");
      if (state.session) state.session->shutdown();
      server->stop();
      delete server;
    }
  }
  return 0;
}

// Machine-checkable presentation budget: paints real fullscreen-sized 32bpp
// frames through the production paint_scene path and fails when the average
// frame cost would visibly stutter the 20 Hz tick. The bound is deliberately
// generous (regressions of the kind this gate exists for cost hundreds of
// milliseconds); the measured value prints so drift is visible in every run.
int scenario_frame_budget() {
  ClientState state;
  scenario_begin(state);
  scenario_follow_camera(state);
  const int width = 3440;
  const int height = 1440;
  state.camera.zoom = kCameraDefaultZoom * zoom_height_factor(height);
  BITMAPINFO info{};
  info.bmiHeader.biSize = sizeof(info.bmiHeader);
  info.bmiHeader.biWidth = width;
  info.bmiHeader.biHeight = -height;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  void* bits = nullptr;
  HDC dc = CreateCompatibleDC(nullptr);
  HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
  scenario_check(bitmap != nullptr, "frame-budget: 32bpp surface allocated");
  if (!bitmap) { DeleteDC(dc); return scenario_failures; }
  HGDIOBJ old = SelectObject(dc, bitmap);
  RECT bounds{0, 0, width, height};
  paint_scene(state, dc, bounds);  // warm caches (fonts, GDI+ startup)
  LARGE_INTEGER freq{}, begin{}, end{};
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&begin);
  constexpr int kFrames = 20;
  for (int i = 0; i < kFrames; ++i) paint_scene(state, dc, bounds);
  QueryPerformanceCounter(&end);
  const double avg_ms = 1000.0 *
                        static_cast<double>(end.QuadPart - begin.QuadPart) /
                        static_cast<double>(freq.QuadPart) / kFrames;
  std::printf("    frame-budget: %.1f ms average over %d frames at %dx%d\n",
              avg_ms, kFrames, width, height);
  scenario_check(avg_ms < 40.0,
                 "frame-budget: fullscreen frame stays under 40 ms");
  SelectObject(dc, old);
  DeleteObject(bitmap);
  DeleteDC(dc);
  return scenario_failures;
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
      {"chronicles-gate-b", scenario_chronicles_gate_b},
      {"first-session-clarity", scenario_first_session_clarity},
      {"animation-vfx-phase-a", scenario_animation_vfx_phase_a},
      {"progression-surface", scenario_progression_surface},
      {"hud-pane-readability", scenario_hud_pane_readability},
      {"frame-budget", scenario_frame_budget},
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
    case render::Op::Npc: return "Npc";
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
    case render::Op::Chronicles: return "Chronicles";
    case render::Op::HouseChip: return "HouseChip";
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
  std::string forced;
  const int overridden = capture_root_override(&forced);
  if (overridden != 0) return overridden > 0 ? forced : std::string{};
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

int count_kind(const ClientState& state, EffectFx::Kind kind) {
  int total = 0;
  for (const auto& fx : state.effects)
    if (fx.kind == kind) ++total;
  return total;
}

bool render_list_has_label(const render::List& list, render::Op op,
                           const std::string& label) {
  for (const auto& item : list)
    if (item.op == op && item.label == label) return true;
  return false;
}

// TASK-0122 Phase A: the animation/VFX contract scenario. Proves deterministic
// event timing for the spawn/materialization, war-cry fade, ScionLost, and
// ordinary-hit beats through the REAL pipeline, includes a
// presentation-cannot-mutate-simulation negative control, and writes fresh
// 960x600 + 1366x768 PNG evidence into this task's captures/ folder.
int scenario_animation_vfx_phase_a() {
  ClientState state;
  scenario_begin(state);

  // ── Spawn/materialization beat: exactly once per first-sighting foe ──
  scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait));
  sync_world(state);
  const int foe_count = static_cast<int>(state.world.monsters.size());
  scenario_check(foe_count > 0, "animation-vfx-phase-a: route snapshot has living foes");
  scenario_check(count_kind(state, EffectFx::Kind::Materialize) == foe_count,
                 "animation-vfx-phase-a: one materialization beat per first sighting");
  scenario_check(
      render_list_has_label(state.render_list, render::Op::TargetFlash,
                            phase_a::kSpawnRenderLabel),
      "animation-vfx-phase-a: spawn beat recorded in the render list");
  for (int i = 0; i < phase_a::kMaterializeTtlTicks; ++i)
    scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait));
  scenario_check(count_kind(state, EffectFx::Kind::Materialize) == 0,
                 "animation-vfx-phase-a: materialization expires exactly at its ttl");
  scenario_check(state.known_monsters.size() >= static_cast<std::size_t>(foe_count),
                 "animation-vfx-phase-a: seen foes never re-materialize");

  // ── Negative control: presentation beats never mutate the simulation ──
  {
    const std::uint64_t sim_tick_before = state.simulation->tick();
    const auto* scion_actor =
        state.simulation->actor(state.simulation->scion().actor_id);
    const int life_before = scion_actor ? scion_actor->stats.life : -1;
    const std::size_t actors_before = state.simulation->actors().size();
    const std::size_t store_before =
        state.simulation->house().stored_items.size() +
        state.simulation->house().stored_trophies.size();
    verdigris::client::PresentationFx control;
    control.known_monsters = state.known_monsters;
    detect_monster_spawns(control, state.world, state.world.tick);
    const verdigris::client::PresentationEvent crit_hit{
        verdigris::client::PresentationEventType::DamageApplied, "", "",
        "outgoing", 9, true, "stab"};
    const verdigris::client::PresentationEvent plain_hit{
        verdigris::client::PresentationEventType::DamageApplied, "", "",
        "outgoing", 4, false, {}};
    const verdigris::client::PresentationEvent lost{
        verdigris::client::PresentationEventType::ScionLost, "", "", "", 0, false, {}};
    const verdigris::client::PresentationEvent expired{
        verdigris::client::PresentationEventType::BuffExpired, "", "", "war-cry", 0,
        false, {}};
    apply_presentation_event(control, state.world, crit_hit, state.world.tick);
    apply_presentation_event(control, state.world, plain_hit, state.world.tick);
    apply_presentation_event(control, state.world, lost, state.world.tick);
    apply_presentation_event(control, state.world, expired, state.world.tick);
    scenario_check(state.simulation->tick() == sim_tick_before &&
                       state.simulation->actors().size() == actors_before &&
                       store_before == state.simulation->house().stored_items.size() +
                                           state.simulation->house().stored_trophies.size(),
                   "animation-vfx-phase-a: beats leave tick/actors/store untouched");
    const auto* scion_after =
        state.simulation->actor(state.simulation->scion().actor_id);
    scenario_check((scion_after ? scion_after->stats.life : -1) == life_before,
                   "animation-vfx-phase-a: beats leave scion life untouched");
    int crit_ttl = -1;
    int plain_ttl = -1;
    for (const auto& fx : control.effects) {
      if (fx.kind != EffectFx::Kind::DamageNumber) continue;
      if (fx.critical && crit_ttl < 0) crit_ttl = fx.ttl;
      if (!fx.critical && plain_ttl < 0) plain_ttl = fx.ttl;
    }
    scenario_check(crit_ttl == phase_a::kCriticalNumberTtlTicks &&
                       plain_ttl == 12 && crit_ttl != plain_ttl,
                   "animation-vfx-phase-a: critical and ordinary numbers differ in timing");
    scenario_check(!control.event_log.empty() &&
                       control.event_log.back() == "war cry faded",
                   "animation-vfx-phase-a: buff expiry logs its readable beat");
  }

  // ── War-cry expiration beat: timed by the authoritative buff window ──
  scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::WarCry));
  int fade_seen_at = -1;
  int fade_ttl = -1;
  bool saw_fade_op = false;
  for (int step = 1; step <= verdigris::presentation_constants::kWarCryDurationTicks + 8; ++step) {
    scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait));
    if (fade_seen_at < 0 && count_kind(state, EffectFx::Kind::WarCryFade) > 0) {
      fade_seen_at = step;
      for (const auto& fx : state.effects)
        if (fx.kind == EffectFx::Kind::WarCryFade && fade_ttl < 0) fade_ttl = fx.ttl;
      saw_fade_op = render_list_has_label(state.render_list, render::Op::WarCry,
                                          phase_a::kWarcryFadeLabel);
    }
    if (fade_seen_at > 0) break;
  }
  // The core applies the buff on the cast tick and then decrements on that
  // same tick's advance pass, so expiry fires within [duration-1, duration+2]
  // pipeline steps of the authoritative kWarCryDurationTicks window.
  scenario_check(
      fade_seen_at >= verdigris::presentation_constants::kWarCryDurationTicks - 1 &&
          fade_seen_at <= verdigris::presentation_constants::kWarCryDurationTicks + 2,
      "animation-vfx-phase-a: fade arrives with the buff window");
  scenario_check(fade_ttl == phase_a::kWarcryFadeTtlTicks,
                 "animation-vfx-phase-a: fade lifetime comes from the constants table");
  scenario_check(saw_fade_op,
                 "animation-vfx-phase-a: fade recorded with its distinct label");
  int fade_steps_left = 0;
  while (count_kind(state, EffectFx::Kind::WarCryFade) > 0 &&
         fade_steps_left <= phase_a::kWarcryFadeTtlTicks + 4) {
    scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait));
    ++fade_steps_left;
  }
  scenario_check(count_kind(state, EffectFx::Kind::WarCryFade) == 0,
                 "animation-vfx-phase-a: fade clears deterministically");

  // ── ScionLost beat: staged lethal pack, real authoritative death ──
  sync_world(state);
  const int melee = verdigris::world_scale::kMeleeRange;
  const verdigris::Vec2 here = state.world.player.position;
  state.simulation->spawn_monster({here.x - melee, here.y}, 3, true);
  state.simulation->spawn_monster({here.x + melee, here.y}, 3, true);
  state.simulation->spawn_monster({here.x, here.y + melee}, 3, true);
  int lost_seen_at = -1;
  int lost_pulse = -1;
  for (int step = 0; step < 400 && lost_seen_at < 0; ++step) {
    scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait));
    if (count_kind(state, EffectFx::Kind::ScionLostBeat) > 0) {
      lost_seen_at = step;
      lost_pulse = state.screen_pulse_ticks;
    }
  }
  scenario_check(lost_seen_at >= 0, "animation-vfx-phase-a: scion loss produces the loss beat");
  scenario_check(!state.simulation->scion().alive,
                 "animation-vfx-phase-a: loss beat matches an authoritative death");
  scenario_check(lost_pulse > 0 && lost_pulse <= phase_a::kScionLostPulseTicks,
                 "animation-vfx-phase-a: loss pulse uses the constants table");
  scenario_check(render_list_has_label(state.render_list, render::Op::ScreenPulse,
                                       phase_a::kScionLostLabel),
                 "animation-vfx-phase-a: loss beat recorded with its distinct label");
  int lost_steps_left = 0;
  while (count_kind(state, EffectFx::Kind::ScionLostBeat) > 0 &&
         lost_steps_left <= phase_a::kScionLostRingTtlTicks + 4) {
    scenario_step(state, verdigris::Command::action_use(verdigris::ActionType::Wait));
    ++lost_steps_left;
  }
  scenario_check(count_kind(state, EffectFx::Kind::ScionLostBeat) == 0,
                 "animation-vfx-phase-a: loss beat clears exactly at its ttl");

  // ── Task captures: one controlled composite with every Phase A beat ──
  {
    ClientState capture_state;
    scenario_begin(capture_state);
    // Clear the route through the REAL pipeline so no stray warden combat
    // noise pollutes the composite frame.
    scenario_check(drive_to_extraction_phase(capture_state),
                   "animation-vfx-phase-a: capture staging cleared the route");
    // Burn off residual death/loot FX (longest ttl is the 24-tick sparkle).
    for (int i = 0; i < 26; ++i)
      scenario_step(capture_state, verdigris::Command::action_use(verdigris::ActionType::Wait));
    // Relocate several full tiles into open ground so the entrance pad,
    // warden drops, and every expired effect are behind the camera.
    sync_world(capture_state);
    const verdigris::Vec2 origin = capture_state.world.player.position;
    const int tile = static_cast<int>(kTileUnits);
    for (int i = 0; i < 90; ++i) {
      sync_world(capture_state);
      const verdigris::Vec2 here = capture_state.world.player.position;
      const bool far_enough =
          std::abs(here.x - origin.x) >= 4 * tile &&
          std::abs(here.y - origin.y) <= 1 * tile;
      if (far_enough) break;
      const verdigris::Vec2 before = here;
      scenario_step(capture_state, verdigris::Command::move(1, 0));
      sync_world(capture_state);
      if (capture_state.world.player.position.x == before.x &&
          capture_state.world.player.position.y == before.y) {
        // East blocked: nudge around the obstacle.
        scenario_step(capture_state, verdigris::Command::move(0, -1));
        sync_world(capture_state);
        if (capture_state.world.player.position.x == before.x &&
            capture_state.world.player.position.y == before.y)
          scenario_step(capture_state, verdigris::Command::move(0, 1));
      }
    }
    sync_world(capture_state);
    scenario_check(std::abs(capture_state.world.player.position.x - origin.x) >= 3 * tile,
                   "animation-vfx-phase-a: capture relocated to open field");
    // Let the relocation dust settle, then start the authoritative war-cry
    // clock so its REAL BuffExpired fade is live in the final frame.
    scenario_step(capture_state, verdigris::Command::action_use(verdigris::ActionType::WarCry));
    bool fade_live = false;
    for (int step = 0; step < verdigris::presentation_constants::kWarCryDurationTicks + 6 &&
                       !fade_live;
         ++step) {
      scenario_step(capture_state, verdigris::Command::action_use(verdigris::ActionType::Wait));
      fade_live = count_kind(capture_state, EffectFx::Kind::WarCryFade) > 0;
    }
    scenario_check(fade_live,
                   "animation-vfx-phase-a: capture holds the real war-cry fade");
    // Stage the separated beats. Positions are player-relative tile offsets,
    // mutually spaced and clear of the player sprite and the frame edges.
    sync_world(capture_state);
    const verdigris::Vec2 base = capture_state.world.player.position;
    const int reach = verdigris::world_scale::kMeleeRange;
    // materialize: fresh spawn at +2E/+1.5S (rings live on it, untouched).
    capture_state.simulation->spawn_monster({base.x + 2 * reach, base.y + reach}, 2,
                                            false);
    // ordinary hit: adjacent spawn one tile north, struck on-camera below.
    capture_state.simulation->spawn_monster({base.x, base.y - reach}, 2, false);
    RECT capture_bounds{0, 0, 960, 600};
    const ScreenPoint aim_at =
        project(capture_state.camera, capture_bounds,
                static_cast<double>(base.x), static_cast<double>(base.y - reach));
    capture_state.mouse.x = std::clamp(aim_at.x, 0, 959);
    capture_state.mouse.y = std::clamp(aim_at.y, 0, 599);
    scenario_step(capture_state, verdigris::Command::move(0, 0));
    dispatch_aim_if_changed(capture_state, capture_bounds, true);
    scenario_step(capture_state, verdigris::Command::action_use(verdigris::ActionType::Melee));
    // Staged presentation-seam injections for the two treatments whose real
    // triggers live outside local play (remote combat:hit critical data) or
    // would end the session (ScionLoss). They exercise the exact seam paths
    // and lifetimes asserted above; nothing here touches the simulation.
    // ScionLost is placed by unprojecting an explicit 960x600 screen point
    // in the verified-clear lower-left safe area, so the complete rings plus
    // label stay inside the compact frame and clear of the quickbar.
    double lost_wx = static_cast<double>(base.x);
    double lost_wy = static_cast<double>(base.y);
    unproject(capture_state.camera, capture_bounds, 228, 468, lost_wx, lost_wy);
    {
      EffectFx crit_number;
      crit_number.kind = EffectFx::Kind::DamageNumber;
      crit_number.wx = static_cast<double>(base.x - 2 * reach);
      crit_number.wy = static_cast<double>(base.y - reach);
      crit_number.ttl = phase_a::kCriticalNumberTtlTicks;
      crit_number.value = 27;
      crit_number.critical = true;
      crit_number.style = "stab";
      capture_state.effects.push_back(crit_number);
      EffectFx lost;
      lost.kind = EffectFx::Kind::ScionLostBeat;
      lost.wx = lost_wx;
      lost.wy = lost_wy;
      lost.ttl = phase_a::kScionLostRingTtlTicks;
      capture_state.effects.push_back(lost);
    }
    // In-world legend so each treatment is identifiable without guessing.
    capture_state.beat_legend = {
        {"spawn beat", {base.x + 2 * reach, base.y + reach}},
        {"ordinary hit", {base.x, base.y - reach}},
        {"CRITICAL 27", {base.x - 2 * reach, base.y - reach / 2}},
        {"buff end", {base.x, base.y}},
        {"scion lost",
         {static_cast<int>(std::lround(lost_wx)),
          static_cast<int>(std::lround(lost_wy))}},
    };
    // Re-present so the recorded render list includes the staged treatments.
    scenario_present(capture_state);
    scenario_check(count_kind(capture_state, EffectFx::Kind::Materialize) >= 1,
                   "animation-vfx-phase-a: capture frame holds a live materialization beat");
    scenario_check(render_list_has_label(capture_state.render_list, render::Op::Damage,
                                         "monster") ||
                       render_list_has_label(capture_state.render_list, render::Op::Damage,
                                             "player"),
                   "animation-vfx-phase-a: capture frame holds an ordinary hit number");
    scenario_check(
        render_list_has_label(capture_state.render_list, render::Op::Damage,
                              std::string(phase_a::kCriticalDamageLabel) + ":stab"),
        "animation-vfx-phase-a: capture frame holds the critical treatment");
    scenario_check(
        render_list_has_label(capture_state.render_list, render::Op::WarCry,
                              phase_a::kWarcryFadeLabel),
        "animation-vfx-phase-a: capture frame holds the buff-expiry treatment");
    scenario_check(count_kind(capture_state, EffectFx::Kind::ScionLostBeat) >= 1,
                   "animation-vfx-phase-a: capture frame holds the loss treatment");
    scenario_check(
        render_list_has_label(capture_state.render_list, render::Op::TargetFlash,
                              phase_a::kSpawnRenderLabel),
        "animation-vfx-phase-a: capture frame records the spawn beat");
    const std::string dir = animation_vfx_capture_dir();
    if (dir.empty()) {
      scenario_check(
          false, "animation-vfx-phase-a: capture root rejected before any write");
      return 0;
    }
    const std::string png_960 = dir + "\\animation-vfx-phase-a-960x600.png";
    const std::string png_1366 = dir + "\\animation-vfx-phase-a-1366x768.png";
    scenario_check(reference_present(capture_state, 960, 600, png_960),
                   "animation-vfx-phase-a: 960x600 PNG written");
    scenario_check(reference_present(capture_state, 1366, 768, png_1366),
                   "animation-vfx-phase-a: 1366x768 PNG written");
    for (const std::string& path : {png_960, png_1366}) {
      std::ifstream probe(path, std::ios::binary);
      scenario_check(probe.good(),
                     "animation-vfx-phase-a: capture readable");
      probe.seekg(0, std::ios::end);
      const std::streamoff bytes = probe.tellg();
      char line[512];
      std::snprintf(line, sizeof(line), "    capture: %s (%lld bytes)\n", path.c_str(),
                    static_cast<long long>(bytes));
      std::printf("%s", line);
      scenario_check(bytes > 1024, "animation-vfx-phase-a: capture is non-trivial");
    }
  }
  return 0;
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
  if (out_dir.empty()) {
    std::printf("FAIL reference-scene: capture root rejected before any write\n");
    return 1;
  }
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

int run_remote_native_client(const char* host, unsigned short port, const char* guest_id,
                             bool chronicles_mode) {
  auto state = std::make_unique<ClientState>();
  state->chronicles_mode = chronicles_mode;
  state->screen = chronicles_mode ? Screen::Chronicles : Screen::Expedition;
  state->session = std::make_unique<verdigris::client::RemoteProtocolSession>(
      host ? host : "127.0.0.1", port, guest_id ? guest_id : "cursor-guest",
      !chronicles_mode);
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

  state->camera.zoom =
      kCameraDefaultZoom * zoom_height_factor(GetSystemMetrics(SM_CYSCREEN));
  HWND window = CreateWindowExA(
      0, window_class.lpszClassName,
      chronicles_mode ? "Verdigris Chronicles" : "Verdigris Remote Guest",
      WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
      nullptr, nullptr, instance, state.get());
  ShowWindow(window, SW_SHOW);
  SetTimer(window, 1, 15, nullptr);

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
      const char* guest = "cursor-guest";
      if (i + 1 < argc && argv[i + 1][0] != '-') {
        const bool dotted = std::strchr(argv[i + 1], '.') != nullptr;
        if (dotted) {
          host = argv[++i];
          if (i + 1 < argc && argv[i + 1][0] != '-')
            port = static_cast<unsigned short>(std::atoi(argv[++i]));
        } else {
          port = static_cast<unsigned short>(std::atoi(argv[++i]));
        }
        // Optional guest identity token after host/port.
        if (i + 1 < argc && argv[i + 1][0] != '-') guest = argv[++i];
      }
      // TASK-0145: the remote owner path opens at the Chronicles front door
      // by default; --quick preserves the legacy straight-into-world guest.
      bool chronicles_mode = true;
      for (int k = 1; k < argc; ++k)
        if (std::strcmp(argv[k], "--quick") == 0) chronicles_mode = false;
      return run_remote_native_client(host, port, guest, chronicles_mode);
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

  state->camera.zoom =
      kCameraDefaultZoom * zoom_height_factor(GetSystemMetrics(SM_CYSCREEN));
  HWND window = CreateWindowExA(0, window_class.lpszClassName, "Verdigris Core Testbed",
                                WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN),
                                GetSystemMetrics(SM_CYSCREEN),
                                nullptr, nullptr, instance, state.get());
  ShowWindow(window, SW_SHOW);
  SetTimer(window, 1, 15, nullptr);

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
