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
#include "vector_art.hpp"
#include "framekit_renderer.hpp"
#include "geometric_skill_tree.hpp"
#include "title_scene.hpp"

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
  // Generated seamless interior used beneath WIZARD's authored panel frame.
  // Keeping this separate means the frozen source pack remains untouched and
  // the center can repeat at native resolution instead of being stretched.
  SpriteBitmap fk_panel_fill;
  // Authored WIZARD Framekit 2.0 game pack. These are the actual cropped
  // marble/brass sheets used by the flagship module, not procedural stand-ins.
  SpriteBitmap fk_panel_ornate;
  SpriteBitmap fk_banner;
  SpriteBitmap fk_tooltip;
  SpriteBitmap fk_xp_rail;
  SpriteBitmap fk_orb_life;
  SpriteBitmap fk_orb_resource;
  SpriteBitmap fk_skill_strike;
  SpriteBitmap fk_skill_thrust;
  SpriteBitmap fk_skill_sweep;
  SpriteBitmap fk_skill_warcry;
  SpriteBitmap fk_button;
  SpriteBitmap fk_chronicle_keyhole;
  SpriteBitmap fk_chronicle_knocker;
  SpriteBitmap fk_chronicle_laurel;
  SpriteBitmap fk_chronicle_input;
  std::unordered_map<std::string, SpriteBitmap> item_art;
  std::string root;
  std::string status = "art: loading";
  std::string framekit_status = "art: WIZARD Framekit loading";
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
    fk_panel_fill.reset();
    fk_panel_ornate.reset();
    fk_banner.reset();
    fk_tooltip.reset();
    fk_xp_rail.reset();
    fk_orb_life.reset();
    fk_orb_resource.reset();
    fk_skill_strike.reset();
    fk_skill_thrust.reset();
    fk_skill_sweep.reset();
    fk_skill_warcry.reset();
    fk_button.reset();
    fk_chronicle_keyhole.reset();
    fk_chronicle_knocker.reset();
    fk_chronicle_laurel.reset();
    fk_chronicle_input.reset();
    for (auto& entry : item_art) entry.second.reset();
    if (gdiplus_shutdown && gdiplus_token) gdiplus_shutdown(gdiplus_token);
    if (gdiplus_module) FreeLibrary(gdiplus_module);
    if (msimg32_module) FreeLibrary(msimg32_module);
  }
  BillboardAssets() = default;
  BillboardAssets(const BillboardAssets&) = delete;
  BillboardAssets& operator=(const BillboardAssets&) = delete;
};

enum class SceneryKind { Tree, Ruin, Dwelling, Shrine, Gate };

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

// The Chronicles front door owns a small text-entry modal instead of silently
// naming a player's lineage from their account id. The existing derived names
// remain as placeholders/fallbacks, so controller-less keyboard play can still
// accept a sensible default with Enter.
enum class ChronicleNamingMode { None, House, Scion };

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

// The tactical map has two deliberately different reads. Corner mode is the
// quiet moment-to-moment compass; Overlay mode is the PoE-style translucent
// route plan that leaves the live world visible underneath. Preferences are
// presentation-only: topology and markers still come exclusively from the
// authoritative WorldView.
enum class MinimapMode { Corner, Overlay };
enum class MinimapSide { Left, Right };

constexpr int kMinimapSettingSteps = 5;
constexpr int kDefaultMinimapZoomStep = 2;
constexpr int kDefaultMinimapOpacityStep = 2;

double minimap_zoom_factor(int step) {
  static constexpr double kSteps[kMinimapSettingSteps] = {0.65, 0.82, 1.0,
                                                           1.22, 1.48};
  return kSteps[std::clamp(step, 0, kMinimapSettingSteps - 1)];
}

int minimap_zoom_percent(int step) {
  return static_cast<int>(
      std::lround(minimap_zoom_factor(step) * 100.0));
}

BYTE minimap_body_alpha(int step) {
  static constexpr BYTE kSteps[kMinimapSettingSteps] = {105, 135, 170, 205,
                                                         235};
  return kSteps[std::clamp(step, 0, kMinimapSettingSteps - 1)];
}

int minimap_opacity_percent(int step) {
  return static_cast<int>(std::lround(
      static_cast<double>(minimap_body_alpha(step)) * 100.0 / 255.0));
}

bool hud_rects_overlap(const HudRect& a, const HudRect& b) {
  return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h &&
         b.y < a.y + a.h;
}

// Integer HUD scale by window height: the shipped 960x600/1366x768 layouts
// keep their exact historical geometry (scale 1, so every readability
// contract number is unchanged), while fullscreen 1440p+ doubles the chrome
// instead of shrinking it to ant size.
int hud_scale(int height) { return std::max(1, height / 700); }

// The shipped gear pane. It grows into otherwise unused vertical space at
// 768p+ so a selected living Vessel can explain every property without
// covering the backpack grid. The historical 600p footprint is preserved.
HudRect gear_pane_rect(int width, int height) {
  const int s = hud_scale(height);
  const int pane_w = 380 * s;
  const int pane_top = 64 * s;
  const int x = std::max(24, width - pane_w - 24);
  const int historical_bottom = pane_top + 430 * s;
  const int expanded_bottom = height - 120 * s;
  const int bottom = std::min(height - 28,
                              std::max(historical_bottom, expanded_bottom));
  return {x, pane_top, std::min(pane_w, std::max(0, width - x)),
          std::max(0, bottom - pane_top)};
}

HudRect character_pane_rect(int width, int height) {
  const int s = hud_scale(height);
  const int pane_w = std::min(500 * s, std::max(0, width - 48));
  const int pane_top = 64 * s;
  const int historical_bottom = pane_top + 430 * s;
  const int expanded_bottom = height - 120 * s;
  const int bottom = std::min(height - 28,
                              std::max(historical_bottom, expanded_bottom));
  return {24 * s, pane_top, pane_w, std::max(0, bottom - pane_top)};
}

HudRect quest_journal_rect(int width, int height) {
  const int w = (std::min)(760, width - 48);
  const int h = (std::min)(520, height - 128);
  return {(width - w) / 2, (height - h) / 2 - 12, w, h};
}

HudRect minimap_rect(int width, int height, MinimapSide side) {
  const int s = hud_scale(height);
  const int size = 108 * s;
  const int margin = 12 * s;
  const int x = side == MinimapSide::Right
                    ? std::max(margin, width - margin - size)
                    : margin;
  return {x, margin, size, size};
}

HudRect minimap_overlay_rect(int width, int height) {
  // Keep the tactical plate clear of the top information rows and bottom
  // combat HUD at every supported aspect ratio. It is intentionally broad,
  // but never a fully opaque pause screen.
  const int min_w = std::min(440, std::max(240, width - 32));
  const int max_w = std::max(min_w, width - 48);
  const int min_h = std::min(360, std::max(240, height - 180));
  const int max_h = std::max(min_h, height - 200);
  const int w =
      std::clamp(static_cast<int>(width * 0.68), min_w, max_w);
  const int h =
      std::clamp(static_cast<int>(height * 0.72), min_h, max_h);
  return {(width - w) / 2, std::max(70, (height - h) / 2), w, h};
}

constexpr int kVitalOrbRadius = 34;

HudRect vital_orb_rect(int width, int height, bool resource) {
  const int s = hud_scale(height);
  const int radius = kVitalOrbRadius * hud_scale(height);
  const int cx = resource ? width - 36 * s - radius : 36 * s + radius;
  const int cy = height - 18 - radius;
  // Match the flagship Framekit's 247x250 display geometry around a 155px
  // circular well. The asymmetrical statue chrome is part of the HUD reserve.
  const int cluster_w = (radius * 2 * 247 + 154) / 155;
  const int cluster_h = (cluster_w * 811 + 799) / 800;
  const int well_x = resource ? cluster_w * 10 / 247
                              : cluster_w * 81 / 247;
  const int well_y = cluster_h * 23 / 250;
  const int cluster_x = cx - radius - well_x;
  const int cluster_y = cy - radius - well_y;
  const int left = std::max(0, cluster_x);
  const int top = std::max(0, cluster_y);
  const int right = std::min(width, cluster_x + cluster_w);
  const int bottom = std::min(height, cluster_y + cluster_h);
  return {left, top, std::max(0, right - left), std::max(0, bottom - top)};
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
  struct InventoryHit {
    RECT rect{};
    std::string id;
  };
  std::vector<InventoryHit> inventory_hits;
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
  std::unordered_map<std::string, std::uint64_t> monster_strikes;
  bool loot_labels = false;
  bool gear_overlay = false;
  bool quest_journal = false;
  bool debug_overlay = false;
  MinimapMode minimap_mode = MinimapMode::Corner;
  MinimapSide minimap_side = MinimapSide::Left;
  int minimap_zoom_step = kDefaultMinimapZoomStep;
  int minimap_opacity_step = kDefaultMinimapOpacityStep;
  bool persist_minimap_preferences = false;
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
    int kind = 0;  // 0 buy, 1 withdraw, 2 deposit, 3 chart, 4 dialogue,
                   // 5 brand, 6 trophy
    std::size_t index = 0;
    std::string ref;   // item id (shop) or uuid (bank/deposit)
    std::string extra; // dialogue option id
    std::string choice;  // optional forge sub-choice (WIZARD trophy id)
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
  // Vector-actor animation state: walk cycles accumulate from authoritative
  // position deltas per rendered frame; breathe is a shared idle clock.
  struct ActorMotion {
    verdigris::Vec2 last_pos{};
    bool has_last = false;
    double walk_phase = 0.0;
    double moving = 0.0;
    // Remote actors render between accepted server tiles. The target remains
    // authoritative; these doubles are presentation-only and are discarded
    // when an actor leaves the scene.
    bool has_display = false;
    double display_x = 0.0;
    double display_y = 0.0;
    double from_x = 0.0;
    double from_y = 0.0;
    verdigris::Vec2 target_pos{};
    double move_elapsed_ms = 0.0;
    double move_duration_ms = 400.0;
  };
  std::unordered_map<std::string, ActorMotion> motions;
  double breathe_phase = 0.0;
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
  RECT rechart_tablet_hit{};
  bool rechart_tablet_hit_valid = false;
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
  bool title_open = false;
  bool title_enabled = false; // real startup + opt-in scenarios, never core play
  verdigris::client::TitleScene title_scene;
  verdigris::client::TitleOrbit title_orbit;
  std::chrono::steady_clock::time_point title_started = std::chrono::steady_clock::now();
  std::vector<RECT> title_action_hits;
  int title_selected = 0;
  enum class CreationWait { None, House, Scion, Admission };
  bool startup_creation = false;
  CreationWait creation_wait = CreationWait::None;
  int creation_focus = 0; // name, Hardcore, create, back
  RECT creation_input{}, creation_oath{}, creation_confirm{}, creation_back{};
  std::string creation_requested_name, creation_previous_receipt, creation_house;
  std::chrono::steady_clock::time_point creation_submitted{};
  bool chronicles_oath = false;  // mortal-oath choice applied to the next admission
  ChronicleNamingMode chronicles_naming = ChronicleNamingMode::None;
  std::string chronicles_name_input;
  std::string chronicles_name_error;
  bool chronicles_ignore_next_char = false;
  std::vector<ChronicleAction> chronicles_menu;
  // The front door is a real selectable surface, not a painted shortcut
  // legend.  The cursor and exact card bounds are rebuilt from the
  // authoritative Chronicle action list every frame, giving mouse and
  // keyboard the same admission path.
  struct ChronicleActionHit {
    RECT rect{};
    std::size_t index = 0;
  };
  std::size_t chronicles_selected = 0;
  std::vector<ChronicleActionHit> chronicles_action_hits;
  std::string relic_toast;
  int relic_toast_ticks = 0;
  std::unordered_map<std::string, std::string> known_crypt_status;
  // TASK-0122 Phase A: optional world-anchored beat legend for the capture
  // proof composite. Empty in every normal play path.
  std::vector<std::pair<std::string, verdigris::Vec2>> beat_legend;
};

MinimapSide active_minimap_side(const ClientState& state) {
  // Preserve the player's side preference, but temporarily yield the right
  // rail to the inventory diptych. Closing gear restores the chosen side.
  if (state.minimap_mode == MinimapMode::Corner && state.gear_overlay &&
      state.minimap_side == MinimapSide::Right)
    return MinimapSide::Left;
  return state.minimap_side;
}

std::string minimap_preferences_path() {
  char local_data[MAX_PATH]{};
  const DWORD length =
      GetEnvironmentVariableA("LOCALAPPDATA", local_data, MAX_PATH);
  if (length == 0 || length >= MAX_PATH) return {};
  const std::string directory = std::string(local_data) + "\\Verdigris";
  if (!CreateDirectoryA(directory.c_str(), nullptr) &&
      GetLastError() != ERROR_ALREADY_EXISTS)
    return {};
  return directory + "\\client-map.ini";
}

bool write_minimap_preferences(const ClientState& state,
                               const std::string& path) {
  const std::string temporary = path + ".tmp";
  std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
  if (!out) return false;
  out << "side="
      << (state.minimap_side == MinimapSide::Right ? "right" : "left")
      << "\nzoom=" << state.minimap_zoom_step
      << "\nopacity=" << state.minimap_opacity_step << "\n";
  out.flush();
  const bool complete = static_cast<bool>(out);
  out.close();
  if (!complete ||
      !MoveFileExA(temporary.c_str(), path.c_str(),
                   MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    (void)DeleteFileA(temporary.c_str());
    return false;
  }
  return true;
}

bool read_minimap_preferences(ClientState& state, const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return false;
  std::string line;
  const auto read_step = [](const std::string& value, int current) {
    char* end = nullptr;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (end == value.c_str() || !end || *end != '\0') return current;
    if (parsed < 0) return 0;
    if (parsed >= kMinimapSettingSteps) return kMinimapSettingSteps - 1;
    return static_cast<int>(parsed);
  };
  while (std::getline(in, line)) {
    const std::size_t split = line.find('=');
    if (split == std::string::npos) continue;
    const std::string key = line.substr(0, split);
    const std::string value = line.substr(split + 1);
    if (key == "side") {
      if (value == "right") state.minimap_side = MinimapSide::Right;
      if (value == "left") state.minimap_side = MinimapSide::Left;
    } else if (key == "zoom") {
      state.minimap_zoom_step = read_step(value, state.minimap_zoom_step);
    } else if (key == "opacity") {
      state.minimap_opacity_step =
          read_step(value, state.minimap_opacity_step);
    }
  }
  return true;
}

void save_minimap_preferences(const ClientState& state) {
  if (!state.persist_minimap_preferences) return;
  const std::string path = minimap_preferences_path();
  if (!path.empty()) (void)write_minimap_preferences(state, path);
}

void load_minimap_preferences(ClientState& state) {
  const std::string path = minimap_preferences_path();
  if (!path.empty()) (void)read_minimap_preferences(state, path);
  // Only real interactive clients persist subsequent changes. Scenario
  // states never call this loader and therefore cannot touch user settings.
  state.persist_minimap_preferences = true;
}

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
    const std::string game = root + "/framekit/game/";
    load_sprite(assets,
                root + "/../generated/framekit/panel_fill_dark_stone_v2.png",
                assets.fk_panel_fill);
    load_sprite(assets, game + "panel_plain.png", assets.fk_panel_ornate);
    load_sprite(assets, game + "banner_winged.png", assets.fk_banner);
    load_sprite(assets, game + "tooltip_frame.png", assets.fk_tooltip);
    load_sprite(assets, game + "rack_thin.png", assets.fk_xp_rail);
    load_sprite(assets, game + "orb_chrome_l.png", assets.fk_orb_life);
    load_sprite(assets, game + "orb_chrome_r.png", assets.fk_orb_resource);
    load_sprite(assets, game + "med_sun.png", assets.fk_skill_strike);
    load_sprite(assets, game + "chev_single.png", assets.fk_skill_thrust);
    load_sprite(assets, game + "med_star.png", assets.fk_skill_sweep);
    load_sprite(assets, game + "med_bull.png", assets.fk_skill_warcry);
    load_sprite(assets, game + "btn_primary.png", assets.fk_button);
    load_sprite(assets, game + "keyhole.png", assets.fk_chronicle_keyhole);
    load_sprite(assets, game + "knocker.png", assets.fk_chronicle_knocker);
    load_sprite(assets, game + "laurel_sun.png", assets.fk_chronicle_laurel);
    load_sprite(assets, game + "input_field.png", assets.fk_chronicle_input);
    const SpriteBitmap* runtime[] = {
        &assets.fk_panel_ornate, &assets.fk_banner,
        &assets.fk_tooltip,      &assets.fk_xp_rail,
        &assets.fk_orb_life,     &assets.fk_orb_resource,
        &assets.fk_skill_strike, &assets.fk_skill_thrust,
        &assets.fk_skill_sweep,  &assets.fk_skill_warcry,
        &assets.fk_button,       &assets.fk_chronicle_keyhole,
        &assets.fk_chronicle_knocker, &assets.fk_chronicle_laurel,
        &assets.fk_chronicle_input,
    };
    int ready = 0;
    for (const SpriteBitmap* sprite : runtime)
      if (sprite->ready()) ++ready;
    assets.framekit_status = "art: WIZARD Framekit " +
                             std::to_string(ready) + "/15";
    if (ready != 15) assets.framekit_status += " (vector fallback active)";
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
  assets.framekit_status = "art: WIZARD Framekit MISSING (vector fallback active)";
}

// Nine-slice framekit blit through the TASK-0180 planner. Returns false when
// the plate is not loaded so callers can fall back to the vector skin.
bool draw_framekit_nine(const BillboardAssets& assets, HDC dc,
                        const SpriteBitmap& plate, const RECT& rect,
                        BYTE constant_alpha = 255) {
  const bool panel = &plate == &assets.fk_panel;
  const SpriteBitmap& source =
      panel && assets.fk_panel_ornate.ready() ? assets.fk_panel_ornate : plate;
  if (!source.ready() || !assets.alpha_blend) return false;
  framekit_renderer::NineSliceAsset asset =
      &plate == &assets.fk_slot ? framekit_renderer::default_slot_asset()
                                : framekit_renderer::default_panel_asset();
  framekit_renderer::SliceInsets destination_insets = asset.insets;
  if (panel && &source == &assets.fk_panel_ornate) {
    // WIZARD's panel_plain.png is authored as a 60px border-image. Native
    // panes keep that source crop while presenting an 18px readable border.
    asset.insets = {60, 60, 60, 60};
    destination_insets = {18, 18, 18, 18};
  }
  asset.source = {static_cast<std::uint16_t>(source.width),
                  static_cast<std::uint16_t>(source.height)};
  const framekit_renderer::Rect dest{
      static_cast<std::int16_t>(rect.left), static_cast<std::int16_t>(rect.top),
      static_cast<std::uint16_t>(rect.right - rect.left),
      static_cast<std::uint16_t>(rect.bottom - rect.top)};
  const framekit_renderer::NineSlicePlan plan =
      framekit_renderer::plan_nine_slice(dest, asset, destination_insets);
  if (!plan.valid) return false;
  const BLENDFUNCTION blend{AC_SRC_OVER, 0, constant_alpha, AC_SRC_ALPHA};
  for (const auto& region : plan.regions) {
    if (region.dst_w == 0 || region.dst_h == 0) continue;
    if (panel && region.piece == framekit_renderer::Piece::Center &&
        assets.fk_panel_fill.ready()) {
      // The old center sampled one small portion of panel_plain.png and
      // enlarged it across the entire pane. Repeat the dedicated surface at
      // 1:1 pixels instead, clipping only the final row/column of tiles.
      const SpriteBitmap& fill = assets.fk_panel_fill;
      for (int y = region.dst_y; y < region.dst_y + region.dst_h;
           y += fill.height) {
        const int height =
            std::min(fill.height, region.dst_y + region.dst_h - y);
        for (int x = region.dst_x; x < region.dst_x + region.dst_w;
             x += fill.width) {
          const int width =
              std::min(fill.width, region.dst_x + region.dst_w - x);
          assets.alpha_blend(dc, x, y, width, height, fill.dc, 0, 0, width,
                             height, blend);
        }
      }
      continue;
    }
    assets.alpha_blend(dc, region.dst_x, region.dst_y, region.dst_w,
                       region.dst_h, source.dc, region.src_x, region.src_y,
                       region.src_w, region.src_h, blend);
  }
  return true;
}

// Whole-sheet Framekit blit for authored chrome whose geometry is already a
// complete component (banners, orb statues, ability medallions, and rails).
bool draw_framekit_sprite(const BillboardAssets& assets, HDC dc,
                          const SpriteBitmap& sprite, const RECT& rect,
                          BYTE constant_alpha = 255) {
  if (!sprite.ready() || !assets.alpha_blend || rect.right <= rect.left ||
      rect.bottom <= rect.top)
    return false;
  const BLENDFUNCTION blend{AC_SRC_OVER, 0, constant_alpha, AC_SRC_ALPHA};
  return assets.alpha_blend(dc, rect.left, rect.top, rect.right - rect.left,
                            rect.bottom - rect.top, sprite.dc, 0, 0,
                            sprite.width, sprite.height, blend) != FALSE;
}

bool draw_framekit_tooltip(const BillboardAssets& assets, HDC dc,
                           const RECT& rect) {
  if (!assets.fk_tooltip.ready() || !assets.alpha_blend) return false;
  framekit_renderer::NineSliceAsset asset;
  asset.id = framekit_renderer::TextureId::Panel;
  asset.insets = {34, 34, 34, 34};
  asset.source = {static_cast<std::uint16_t>(assets.fk_tooltip.width),
                  static_cast<std::uint16_t>(assets.fk_tooltip.height)};
  const framekit_renderer::Rect dest{
      static_cast<std::int16_t>(rect.left), static_cast<std::int16_t>(rect.top),
      static_cast<std::uint16_t>(rect.right - rect.left),
      static_cast<std::uint16_t>(rect.bottom - rect.top)};
  const framekit_renderer::NineSlicePlan plan =
      framekit_renderer::plan_nine_slice(dest, asset, {12, 12, 12, 12});
  if (!plan.valid) return false;
  const BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  for (const auto& region : plan.regions) {
    if (region.dst_w == 0 || region.dst_h == 0) continue;
    assets.alpha_blend(dc, region.dst_x, region.dst_y, region.dst_w,
                       region.dst_h, assets.fk_tooltip.dc, region.src_x,
                       region.src_y, region.src_w, region.src_h, blend);
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
    assets.framekit_status =
        "art: WIZARD Framekit MISSING (vector fallback active)";
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
    // The four road gates (server kRoadGates): tin N, salt E, chalk S,
    // copper W. Standing on the gate tile opens that road's chart.
    add_scenery(state.scenery, SceneryKind::Gate, 37.0 * t, 94.0 * t,
                landmark_radius * 1.5, false, 1.0);
    add_scenery(state.scenery, SceneryKind::Gate, 64.0 * t, 114.0 * t,
                landmark_radius * 1.5, false, 1.0);
    add_scenery(state.scenery, SceneryKind::Gate, 37.0 * t, 138.0 * t,
                landmark_radius * 1.5, false, 1.0);
    add_scenery(state.scenery, SceneryKind::Gate, 12.0 * t, 115.0 * t,
                landmark_radius * 1.5, false, 1.0);
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
  fx.monster_strikes = std::move(state.monster_strikes);
  ++state.world.tick;
  if (!state.audio_mixer) {
    state.audio_sink = std::make_unique<verdigris::audio::WaveOutSink>();
    state.audio_mixer =
        std::make_unique<verdigris::audio::AudioMixer>(*state.audio_sink);
  }
  const std::string route_before = state.world.route_id;
  const std::uint64_t scene_epoch_before = state.world.scene_epoch;
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
  if (state.world.route_id != route_before ||
      state.world.scene_epoch != scene_epoch_before)
    generate_scenery(state);
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
  state.monster_strikes = std::move(fx.monster_strikes);
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

ScreenPoint project(const Camera& camera, const RECT& bounds, double wx,
                    double wy);

bool try_world_click(ClientState& state, const RECT& bounds, int mouse_x,
                     int mouse_y) {
  const auto within = [&](const ScreenPoint& point, int radius) {
    const long long dx = mouse_x - point.x;
    const long long dy = mouse_y - point.y;
    return dx * dx + dy * dy <= static_cast<long long>(radius) * radius;
  };

  // Entry waymarks are deliberately larger than their exact movement tile:
  // clicking anywhere on the visible portal requests the server-authoritative
  // proximity check and return, with no coordinate hunting.
  if (state.world.has_extraction) {
    const ScreenPoint portal = project(state.camera, bounds,
                                       state.world.extraction.x,
                                       state.world.extraction.y);
    if (within(portal, 52)) {
      submit_extract(state);
      show_hint(state, "Returning through the entry waymark");
      return true;
    }
  }

  for (const auto& [uuid, position] : state.loot_positions) {
    const ScreenPoint loot = project(state.camera, bounds, position.x, position.y);
    if (!within(loot, 34)) continue;
    submit_pick_up(state, uuid);
    show_hint(state, "Taking the selected drop");
    return true;
  }

  if (state.session) {
    for (const auto& npc : state.world.npcs) {
      const ScreenPoint person = project(state.camera, bounds, npc.position.x,
                                         npc.position.y);
      if (!within(person, 42)) continue;
      std::string verb = npc.actions.empty() ? "examine" : npc.actions.front();
      for (const char* preferred : {"talk", "trade", "bank"}) {
        if (std::find(npc.actions.begin(), npc.actions.end(), preferred) !=
            npc.actions.end()) {
          verb = preferred;
          break;
        }
      }
      const char* action_id = verb == "talk" ? "player:npc:talk"
                            : verb == "trade" ? "player:npc:trade"
                            : verb == "bank" ? "player:screen:bank"
                            : "player:npc:examine";
      state.session->submit(
          verdigris::client::ClientCommand::npc_action(npc.id, action_id));
      show_hint(state, "Speaking with " + npc.name);
      return true;
    }
  }
  return false;
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
    const bool may_predict =
        state.session->connection_state() == verdigris::client::ConnectionState::Ready &&
        state.session->model().scene.type == "instance" && player.alive &&
        player.cooldown_ticks <= 0 &&
        player.resource >= skill_resource_cost(
            verdigris::Simulation::presentation_catalog(), skill.action);
    // This only suppresses misleading speculative FX. The command still goes
    // to the server so a stale client snapshot cannot reject a valid action.
    if (may_predict && state.world.tick != state.last_predicted_swing_tick &&
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

void equip_selected(ClientState& state, const std::string& expected_id = {}) {
  {
    sync_world(state);
    const auto& items = state.world.carried;
    if (!expected_id.empty()) {
      const auto found = std::find_if(items.begin(), items.end(),
          [&](const WorldCarriedItem& item) { return item.id == expected_id; });
      // An update may remove/reorder items between painting and clicking.
      // Never let a stale rectangle equip or consume its replacement.
      if (found == items.end()) return;
      state.selected_item = static_cast<std::size_t>(found - items.begin());
    }
    if (!items.empty()) {
      const std::size_t pick = std::min(state.selected_item, items.size() - 1);
      if (items[pick].expedition_map) {
        if (state.world.has_extraction) {
          show_hint(state, "Charted tablets can only be broken at the Crossroads");
          return;
        }
        if (!state.session) {
          show_hint(state, "Charted tablets require an authoritative session");
          return;
        }
        state.session->submit(verdigris::client::ClientCommand::menu_action(
            "player:endgame:open-map", items[pick].id, 0));
        show_hint(state, "Breaking " + items[pick].name);
        return;
      }
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

bool activate_inventory_at(ClientState& state, int x, int y) {
  if (!state.gear_overlay) return false;
  for (const auto& hit : state.inventory_hits) {
    if (x < hit.rect.left || x >= hit.rect.right ||
        y < hit.rect.top || y >= hit.rect.bottom) continue;
    const std::string id = hit.id;
    equip_selected(state, id);
    return true;
  }
  return false;
}

void rechart_selected_tablet(ClientState& state) {
  sync_world(state);
  if (!state.gear_overlay || state.world.carried.empty()) return;
  const std::size_t pick =
      std::min(state.selected_item, state.world.carried.size() - 1);
  const WorldCarriedItem& item = state.world.carried[pick];
  if (!item.expedition_map) {
    show_hint(state, "Only a charted tablet can be re-charted");
    return;
  }
  if (state.world.has_extraction) {
    show_hint(state, "Return to the Crossroads to re-chart this tablet");
    return;
  }
  if (!state.session) {
    show_hint(state, "Re-charting requires an authoritative session");
    return;
  }
  state.session->submit(verdigris::client::ClientCommand::menu_action(
      "player:endgame:rechart-map", item.id, 0));
  show_hint(state, "Re-charting " + item.name + " for " +
                       std::to_string(verdigris::kExpeditionRechartCost) +
                       " gold");
}

void move_inventory_selection(ClientState& state, int dx, int dy) {
  const auto& items = state.world.carried;
  if (items.empty() || (dx == 0 && dy == 0)) return;
  state.selected_item = std::min(state.selected_item, items.size() - 1);
  const auto center = [&](std::size_t index) {
    const WorldCarriedItem& item = items[index];
    const int slot = item.inventory_slot >= 0 && item.inventory_slot < 84
                         ? item.inventory_slot
                         : static_cast<int>(index % 84);
    return POINT{2 * (slot % 12) + std::max(1, item.width),
                 2 * (slot / 12) + std::max(1, item.height)};
  };
  const POINT from = center(state.selected_item);
  std::size_t best = state.selected_item;
  int best_score = std::numeric_limits<int>::max();
  for (std::size_t i = 0; i < items.size(); ++i) {
    if (i == state.selected_item || items[i].equipped) continue;
    const POINT to = center(i);
    const int along = (to.x - from.x) * dx + (to.y - from.y) * dy;
    if (along <= 0) continue;
    const int across = std::abs((to.x - from.x) * dy -
                                (to.y - from.y) * dx);
    const int score = along * 100 + across * 10 + static_cast<int>(i);
    if (score < best_score) {
      best_score = score;
      best = i;
    }
  }
  state.selected_item = best;
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
  // Flat warm shadow pool; vector figures no longer hide a tall ellipse the
  // way full sprite plates did.
  const int rx = std::max(3, static_cast<int>(world_radius * base.scale));
  const int ry = std::max(2, static_cast<int>(world_radius * base.scale * 0.45));
  fill_ellipse(dc, base.x, base.y, rx, ry, RGB(34, 28, 22));
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
    case SceneryKind::Gate: return kTileUnits * 2.6;
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
                       render::List& rl, bool town, double sway_clock) {
  (void)assets;
  const ScreenPoint base =
      project(camera, bounds, item.position.x, item.position.y);
  rl.push_back({render::Op::Scenery, static_cast<double>(base.x),
                static_cast<double>(base.y), 0.0, 0,
                static_cast<int>(item.kind) == 0   ? "tree"
                : static_cast<int>(item.kind) == 1 ? "ruin"
                : static_cast<int>(item.kind) == 2 ? "dwelling"
                : static_cast<int>(item.kind) == 3 ? "shrine"
                                                   : "gate"});
  draw_contact_shadow(dc, base, item.radius * 0.5);
  const int h = std::max(
      10, static_cast<int>(scenery_height(item.kind) * item.scale * base.scale));
  // Deterministic per-item phase offset so a stand of trees never sways in
  // lockstep.
  const double phase_seed =
      static_cast<double>((item.position.x * 31 + item.position.y * 17) % 628) /
      100.0;
  switch (item.kind) {
    case SceneryKind::Tree:
      vector_art::tree(dc, base.x, base.y, h, sway_clock + phase_seed,
                       town ? RGB(96, 138, 84) : RGB(78, 112, 74),
                       RGB(92, 70, 48));
      break;
    case SceneryKind::Ruin:
      if (town)
        vector_art::wagon(dc, base.x, base.y, h, RGB(118, 92, 60),
                          RGB(196, 182, 150));
      else
        vector_art::standing_stones(dc, base.x, base.y, h, RGB(118, 112, 104));
      break;
    case SceneryKind::Dwelling:
      vector_art::market_stall(dc, base.x, base.y, h, RGB(158, 96, 70),
                               RGB(112, 86, 56));
      break;
    case SceneryKind::Shrine:
      if (town)
        vector_art::fountain(dc, base.x, base.y, h,
                             std::fmod(sway_clock * 0.35, 1.0),
                             RGB(132, 126, 116), RGB(88, 148, 168));
      else
        vector_art::standing_stones(dc, base.x, base.y, h, RGB(126, 120, 110));
      break;
    case SceneryKind::Gate:
      vector_art::road_gate(dc, base.x, base.y, h, RGB(128, 120, 108),
                            RGB(120, 214, 168));
      break;
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
                FloorCache* cache = nullptr,
                const std::string& theme = std::string("town")) {
  (void)assets;
  // Fully procedural themed ground (vector_art::terrain_tile): the tiled
  // contract is always honest because real tiles are always drawn.
  rl.push_back({render::Op::Floor, 0.0, 0.0, 0.0, 1, "tiled"});

  HBRUSH background = CreateSolidBrush(RGB(23, 29, 32));
  FillRect(dc, &bounds, background);
  DeleteObject(background);

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
  const double half = tile * 0.5;

  // Semantic ops are recorded per frame regardless of the pixel path so the
  // scenario harness sees the identical vocabulary either way.
  for (int ty = start_ty; ty <= end_ty; ++ty) {
    for (int tx = start_tx; tx <= end_tx; ++tx) {
      const double wx = static_cast<double>(tx) * tile;
      const double wy = static_cast<double>(ty) * tile;
      const ScreenPoint center = project(camera, bounds, wx + half, wy + half);
      const bool use_alt =
          terrain_tile_uses_alt(tx, ty, terrain_theme_prefers_alt(route_id));
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
        const RECT cell{corner0.x, corner0.y, corner1.x, corner1.y};
        vector_art::terrain_tile(target, cell, theme, terrain_tile_hash(tx, ty));
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
      cache->route != route_id + "|" + theme ||
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
    cache->route = route_id + "|" + theme;
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

HudSafeZones hud_safe_zones(const RECT& bounds,
                            MinimapSide side = MinimapSide::Left) {
  HudSafeZones zones;
  const int bottom = static_cast<int>(bounds.bottom);
  const int right = static_cast<int>(bounds.right);
  const HudRect map = minimap_rect(right, bottom, side);
  zones.minimap = {map.x, map.y, map.x + map.w, map.y + map.h};
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

double clamp_radius_from_hud(double x, double y, double radius,
                             const RECT& bounds, MinimapSide side) {
  const HudSafeZones zones = hud_safe_zones(bounds, side);
  double r = radius;
  while (r > 4.0 && (circle_hits_rect(x, y, r, zones.minimap) ||
                     circle_hits_rect(x, y, r, zones.bottom_hud)))
    r -= 2.0;
  if (circle_hits_rect(x, y, std::max(r, 4.0), zones.minimap) ||
      circle_hits_rect(x, y, std::max(r, 4.0), zones.bottom_hud))
    return 0.0;
  return r;
}

bool telegraph_avoids_hud(const render::List& list, const RECT& bounds,
                          MinimapSide side = MinimapSide::Left) {
  const HudSafeZones zones = hud_safe_zones(bounds, side);
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
                           double length, MinimapSide minimap_side,
                           render::List& rl) {
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
  const COLORREF edge = telegraph_color(visibility, RGB(238, 72, 64));
  skin::ensure_started();
  Gdiplus::Point vertices[kSegments + 2];
  for (int i = 0; i < kSegments + 2; ++i)
    vertices[i] = Gdiplus::Point(points[i].x, points[i].y);
  Gdiplus::GraphicsPath warning;
  warning.AddPolygon(vertices, kSegments + 2);
  skin::warning_fill(dc, warning, RGB(214, 52, 52), visibility);
  HPEN pen = CreatePen(PS_SOLID, 2, edge);
  HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
  HGDIOBJ old_pen = SelectObject(dc, pen);
  Polygon(dc, points, kSegments + 2);
  SelectObject(dc, old_brush);
  SelectObject(dc, old_pen);
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
                            static_cast<double>(origin_r), bounds, minimap_side);
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
                          double radius_world, MinimapSide minimap_side,
                          render::List& rl) {
  const ScreenPoint base = project(camera, bounds, telegraph.position.x,
                                   telegraph.position.y);
  const int radius = std::max(4, static_cast<int>(radius_world * base.scale));
  const double clamped =
      clamp_radius_from_hud(static_cast<double>(base.x), static_cast<double>(base.y),
                            static_cast<double>(radius), bounds, minimap_side);
  if (clamped <= 0.0) return;
  rl.push_back({render::Op::Telegraph, static_cast<double>(base.x),
                static_cast<double>(base.y), clamped, 0, telegraph.action});
  const bool volley = telegraph.action == "volley";
  COLORREF fill_source = volley ? RGB(112, 48, 178) : RGB(214, 52, 52);
  COLORREF edge_source = volley ? RGB(210, 104, 246) : RGB(238, 72, 64);
  if (telegraph.damage_channel == "river") {
    fill_source = RGB(42, 112, 176);
    edge_source = RGB(104, 218, 255);
  } else if (telegraph.damage_channel == "ember") {
    fill_source = RGB(205, 74, 28);
    edge_source = RGB(255, 158, 62);
  } else if (telegraph.action == "stonefall") {
    fill_source = RGB(142, 116, 58);
    edge_source = RGB(240, 210, 120);
  } else if (telegraph.action == "grave-ring") {
    fill_source = RGB(112, 94, 126);
    edge_source = RGB(228, 208, 244);
  }
  const COLORREF edge = telegraph_color(visibility, edge_source);
  const int draw_r = static_cast<int>(clamped);
  const bool ring = telegraph.shape == "ring" &&
                    telegraph.inner_radius_tiles > 0;
  int inner_r = 0;
  skin::ensure_started();
  Gdiplus::GraphicsPath warning(Gdiplus::FillModeAlternate);
  warning.AddEllipse(base.x - draw_r, base.y - draw_r, draw_r * 2, draw_r * 2);
  if (ring) {
    inner_r = std::max(
        4, draw_r * telegraph.inner_radius_tiles /
               std::max(1, telegraph.radius_tiles));
    warning.AddEllipse(base.x - inner_r, base.y - inner_r,
                       inner_r * 2, inner_r * 2);
  }
  skin::warning_fill(dc, warning, fill_source, visibility);
  ring_ellipse(dc, base.x, base.y, draw_r, draw_r, edge, 3);
  if (ring) {
    ring_ellipse(dc, base.x, base.y, inner_r, inner_r, edge, 3);
  } else if (draw_r > 12) {
    ring_ellipse(dc, base.x, base.y, draw_r - 10, draw_r - 10,
                 telegraph_color(visibility * 0.82,
                                 volley ? RGB(236, 156, 255) : edge_source), 1);
  }
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
  const MinimapSide minimap_side = active_minimap_side(state);
  const HudSafeZones zones = hud_safe_zones(bounds, minimap_side);
  ExcludeClipRect(dc, zones.minimap.left, zones.minimap.top, zones.minimap.right,
                  zones.minimap.bottom);
  ExcludeClipRect(dc, zones.bottom_hud.left, zones.bottom_hud.top, zones.bottom_hud.right,
                  zones.bottom_hud.bottom);
  for (const auto& entry : state.telegraphs) {
    const ActiveTelegraph& telegraph = entry.second;
    const double visibility = telegraph_visibility(state, telegraph);
    if (telegraph.shape != "line" || telegraph.action == "sweep" ||
        telegraph.action == "volley")
      draw_sweep_telegraph(dc, state.camera, bounds, telegraph, visibility,
                           telegraph.action == "sweep"
                               ? catalog.melee_range
                               : kTileUnits * telegraph.radius_tiles,
                           minimap_side, rl);
    else
      draw_thrust_telegraph(dc, state.camera, bounds, telegraph, visibility,
                            catalog.thrust_range, minimap_side, rl);
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
    case EffectFx::Kind::ComboFinisher: {
      rl.push_back({render::Op::Impact, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, 0,
                    phase_a::kComboFinisherLabel});
      const COLORREF color = fade_to_background(
          RGB(phase_a::kComboFinisherColor.r, phase_a::kComboFinisherColor.g,
              phase_a::kComboFinisherColor.b), life);
      const int outer = std::max(
          10, static_cast<int>(kTileUnits * (0.48 + grow * 0.92) * base.scale));
      const int inner = std::max(5, outer / 2);
      ring_ellipse(dc, base.x, base.y, outer, outer, color, 3);
      ring_ellipse(dc, base.x, base.y, inner, inner, color, 1);
      const int arm = std::max(8, static_cast<int>(kTileUnits * 0.7 * base.scale));
      draw_line(dc, base.x - arm, base.y, base.x + arm, base.y, color, 3);
      draw_line(dc, base.x, base.y - arm, base.x, base.y + arm, color, 3);
      break;
    }
    case EffectFx::Kind::SupportMend: {
      rl.push_back({render::Op::WarCry, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, fx.value,
                    phase_a::kSupportMendLabel});
      const COLORREF color = fade_to_background(
          RGB(phase_a::kSupportMendColor.r, phase_a::kSupportMendColor.g,
              phase_a::kSupportMendColor.b), life);
      const int outer = std::max(
          7, static_cast<int>(kTileUnits * (0.35 + grow * 0.95) * base.scale));
      ring_ellipse(dc, base.x, base.y, outer, outer, color, 3);
      if (outer > 10)
        ring_ellipse(dc, base.x, base.y, outer - 8, outer - 8, color, 1);
      const int arm = std::max(5, outer / 3);
      draw_line(dc, base.x - arm, base.y, base.x + arm, base.y, color, 3);
      draw_line(dc, base.x, base.y - arm, base.x, base.y + arm, color, 3);
      break;
    }
    case EffectFx::Kind::BleedApplied: {
      rl.push_back({render::Op::Impact, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, fx.value,
                    phase_a::kBleedApplyLabel});
      const COLORREF color = fade_to_background(
          RGB(phase_a::kBleedColor.r, phase_a::kBleedColor.g,
              phase_a::kBleedColor.b), life);
      const int radius = std::max(
          8, static_cast<int>(kTileUnits * (0.38 + grow * 0.72) * base.scale));
      ring_ellipse(dc, base.x, base.y, radius, radius, color, 3);
      for (int i = -1; i <= 1; ++i) {
        const int x = base.x + i * std::max(4, radius / 3);
        draw_line(dc, x - 3, base.y - radius / 2, x + 3,
                  base.y + radius / 3, color, 2);
      }
      SetBkMode(dc, TRANSPARENT);
      SetTextColor(dc, color);
      HGDIOBJ old_font = SelectObject(dc, skin::font_small());
      TextOutA(dc, base.x - 20, base.y - radius - 15, "BLEED", 5);
      SelectObject(dc, old_font);
      break;
    }
    case EffectFx::Kind::BondPulse: {
      rl.push_back({render::Op::WarCry, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, fx.value,
                    std::string(phase_a::kBondPulseLabel) + ":" + fx.style});
      const bool slaughter = fx.style == "blood-price" ||
                             fx.style == "battle-rhythm" ||
                             fx.style == "echoing-kill";
      const bool warding = fx.style == "shieldwall" ||
                           fx.style == "old-grudge" ||
                           fx.style == "last-stand" ||
                           fx.style == "stand-ground" ||
                           fx.style == "block";
      const COLORREF raw = slaughter ? RGB(232, 116, 82)
                           : warding ? RGB(116, 174, 224)
                                     : RGB(phase_a::kBondPulseColor.r,
                                           phase_a::kBondPulseColor.g,
                                           phase_a::kBondPulseColor.b);
      const COLORREF color = fade_to_background(raw, life);
      const int outer = std::max(
          9, static_cast<int>(kTileUnits * (0.42 + grow * 1.18) * base.scale));
      const int inner = std::max(4, static_cast<int>(outer * 0.58));
      ring_ellipse(dc, base.x, base.y, outer, outer, color, 3);
      ring_ellipse(dc, base.x, base.y, inner, inner, color, 1);
      const char* label = "BOND";
      if (fx.style == "last-stand") label = "LAST STAND";
      else if (fx.style == "untraceable") label = "UNTRACEABLE";
      else if (fx.style == "echoing-kill") label = "ECHOING KILL";
      else if (fx.style == "shieldwall") label = "BLOCK";
      else if (fx.style == "sidestep") label = "SIDESTEP";
      SetBkMode(dc, TRANSPARENT);
      SetTextColor(dc, color);
      HGDIOBJ old_font = SelectObject(dc, skin::font_small());
      const int label_width = static_cast<int>(std::strlen(label)) * 4;
      TextOutA(dc, base.x - label_width, base.y - outer - 16, label,
               static_cast<int>(std::strlen(label)));
      SelectObject(dc, old_font);
      break;
    }
    case EffectFx::Kind::DamageNumber: {
      std::string damage_label = fx.healing ? "healing" :
          (fx.damage_to_player ? "player" : "monster");
      const COLORREF base_color =
          fx.healing ? RGB(phase_a::kSupportMendColor.r,
                           phase_a::kSupportMendColor.g,
                           phase_a::kSupportMendColor.b)
          : fx.style == "bleed" ? RGB(phase_a::kBleedColor.r,
                                      phase_a::kBleedColor.g,
                                      phase_a::kBleedColor.b)
          : fx.piercing && !fx.critical && !fx.finisher
              ? RGB(phase_a::kPiercingColor.r, phase_a::kPiercingColor.g,
                    phase_a::kPiercingColor.b)
          : fx.finisher ? RGB(phase_a::kComboFinisherColor.r,
                            phase_a::kComboFinisherColor.g,
                            phase_a::kComboFinisherColor.b)
                      : fx.critical ? RGB(phase_a::kCriticalNumberColor.r, phase_a::kCriticalNumberColor.g,
                                         phase_a::kCriticalNumberColor.b)
                      : (fx.damage_to_player ? RGB(255, 118, 104) : RGB(240, 218, 132));
      if (fx.style == "bleed")
        damage_label = phase_a::kBleedDamageLabel;
      else if (fx.piercing && !fx.critical && !fx.finisher)
        damage_label = std::string(phase_a::kPiercingDamageLabel) + ":" +
                       (fx.style.empty() ? "range" : fx.style);
      else if (fx.damage_to_player &&
               (fx.style == "river" || fx.style == "ember"))
        damage_label = "player:" + fx.style;
      else if (fx.finisher)
        damage_label = std::string(fx.critical ? "critical-finisher:" : "finisher:") +
                       (fx.style.empty() ? "slash" : fx.style);
      else if (fx.critical)
        damage_label = std::string(phase_a::kCriticalDamageLabel) + ":" +
                       (fx.style.empty() ? "slash" : fx.style);
      rl.push_back({render::Op::Damage, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, fx.value, damage_label});
      // Critical hits rise higher and read larger for their longer lifetime;
      // ordinary hits keep the accepted TASK-0142 treatment.
      const int lift = static_cast<int>(kTileUnits *
                                        (0.35 + grow * ((fx.critical || fx.finisher) ? 1.05 : 0.75)) *
                                        base.scale);
      const COLORREF color = base_color;
      SetBkMode(dc, TRANSPARENT);
      // TASK-0142: bold numerals so the resolved damage reads instantly.
      const int font_h = std::clamp(
          static_cast<int>(kTileUnits * ((fx.critical || fx.finisher) ? 0.44 : 0.34) * base.scale),
          (fx.critical || fx.finisher) ? 16 : 13,
          (fx.critical || fx.finisher) ? 26 : 22);
      HFONT number_font = CreateFontA(font_h, 0, 0, 0, FW_BOLD, FALSE, FALSE,
                                      FALSE, DEFAULT_CHARSET,
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                      DEFAULT_QUALITY, FF_SWISS,
                                      "Verdana");
      HGDIOBJ old_number_font = SelectObject(dc, number_font);
      // Rise AND fade toward the background over the effect lifetime.
      SetTextColor(dc, fade_to_background(color, life));
      const std::string text = fx.healing ? "+" + std::to_string(fx.value)
                                          : std::to_string(fx.value);
      TextOutA(dc, base.x - 9, base.y - lift, text.c_str(),
               static_cast<int>(text.size()));
      if (fx.piercing && !fx.critical && !fx.finisher) {
        HGDIOBJ pierce_font = SelectObject(dc, skin::font_small());
        TextOutA(dc, base.x - 18, base.y - lift + font_h, "PIERCE", 6);
        SelectObject(dc, pierce_font);
      }
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
                    fx.style == "bleed" ? phase_a::kBleedDamageLabel :
                        fx.finisher ? "finisher" :
                        (fx.damage_to_player ? "player" : "monster")});
      // A brief bright ring over the hit target reads as a tint on the sprite.
      const int r = std::max(6, static_cast<int>(kTileUnits * 0.5 * base.scale));
      const COLORREF flash_color = fx.style == "bleed"
          ? RGB(phase_a::kBleedColor.r, phase_a::kBleedColor.g,
                phase_a::kBleedColor.b)
          : fx.finisher
          ? RGB(phase_a::kComboFinisherColor.r, phase_a::kComboFinisherColor.g,
                phase_a::kComboFinisherColor.b)
          : RGB(255, 244, 190);
      ring_ellipse(dc, base.x, base.y, r, r,
                   fade_to_background(flash_color, life), 3);
      fill_ellipse(dc, base.x, base.y, r / 2, r / 2,
                   fade_to_background(
                       fx.style == "bleed"
                           ? RGB(phase_a::kBleedColor.r, phase_a::kBleedColor.g,
                                 phase_a::kBleedColor.b)
                           : RGB(255, 238, 160),
                       life));
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
int paint_status_chip(const BillboardAssets* assets,
                      const SpriteBitmap* raster_frame, HDC dc, int x, int y,
                      const std::string& text, COLORREF accent,
                      render::List& rl) {
  SIZE extent{};
  GetTextExtentPoint32A(dc, text.c_str(), static_cast<int>(text.size()), &extent);
  const int width = extent.cx + 20;
  const int height = extent.cy + 10;
  RECT rect{x, y, x + width, y + height};
  const BYTE raster_alpha =
      assets && raster_frame == &assets->fk_banner ? 96 : 255;
  const bool raster_drawn =
      assets && raster_frame &&
      draw_framekit_sprite(*assets, dc, *raster_frame, rect, raster_alpha);
  if (!raster_drawn)
    skin::chip(dc, rect, accent);
  else
    rl.push_back({render::Op::Hud, static_cast<double>(x),
                  static_cast<double>(y), 0.0, 0,
                  raster_frame == &assets->fk_banner
                      ? "framekit-raster:banner-winged"
                      : "framekit-raster:button-primary"});
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
  state.inventory_hits.clear();
  state.rechart_tablet_hit_valid = false;
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
  const bool raster_panel = draw_framekit_nine(
      state.billboards, dc, state.billboards.fk_panel, panel_rect);
  if (!raster_panel)
    skin::panel(dc, panel_rect, skin::kVerdigris, 245, 8.0f);
  else
    rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0, 0,
                  state.billboards.fk_panel_fill.ready()
                      ? "framekit-raster:panel-tiled-fill"
                      : "framekit-raster:panel-plain"});

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

  // Authoritative stats readout. Worn items live outside the backpack on the
  // production wire; the carried-item fallback keeps old/local fixtures
  // honest without making disappearance-from-inventory our source of truth.
  const WorldActor& player = state.world.player;
  const auto& items = state.world.carried;
  const WorldCarriedItem* main_hand = nullptr;
  for (const auto& item : state.world.worn)
    if (item.equip_seat == "right_hand") {
      main_hand = &item;
      break;
    }
  if (!main_hand)
    for (const auto& item : items)
      if (item.equipped) {
        main_hand = &item;
        break;
      }
  int equipped_bonus = 0;
  if (main_hand) equipped_bonus = main_hand->attack_bonus;
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

  // Compact loadout ribbon. The full eleven-seat paper doll lives on C; this
  // ribbon makes the I-only view useful without pretending one weapon is the
  // whole equipment model.
  const int seat_top = top + 62 * s;
  RECT seat{left + 14 * s, seat_top, right - 14 * s, seat_top + 24 * s};
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot, seat))
    skin::slot(dc, seat, skin::kVerdigris, false);
  state.hud_rect_trace.push_back(
      {"pane-seat", {seat.left, seat.top, seat.right - seat.left,
                      seat.bottom - seat.top}});
  SetTextColor(dc, RGB(170, 190, 178));
  const char* seat_label = "WORN";
  TextOutA(dc, seat.left + 6 * s, seat_top + 4 * s, seat_label,
           static_cast<int>(strlen(seat_label)));
  std::string equipped_name = main_hand ? main_hand->name : "(unarmed)";
  std::string loadout_value = equipped_name + "  |  " +
      std::to_string(state.world.worn.size()) + "/11 seats";
  SetTextColor(dc, RGB(230, 220, 180));
  rl.push_back({render::Op::PaneWeapon, 0.0, 0.0, 0.0, 0, equipped_name});
  TextOutA(dc, seat.left + 55 * s, seat_top + 4 * s, loadout_value.c_str(),
           static_cast<int>(loadout_value.size()));

  // WIZARD-derived spatial backpack, now driven by the server's real 12x7
  // cell index and item footprint. Every empty cell uses Framekit slot chrome;
  // multi-cell items span the exact occupied rectangle and mouse hover selects
  // the same item that keyboard traversal targets.
  constexpr int kGridColumns = 12;
  constexpr int kGridRows = 7;
  const int grid_left = left + 14 * s;
  const int grid_top = top + 102 * s;
  const int grid_gap = s;
  const int grid_available_w = right - left - 28 * s;
  const int cell_w =
      (grid_available_w - (kGridColumns - 1) * grid_gap) / kGridColumns;
  const int grid_vertical_budget =
      std::max(7 * 18 * s, bottom - grid_top - 150 * s);
  const int cell_h = std::min(
      cell_w, (grid_vertical_budget - (kGridRows - 1) * grid_gap) / kGridRows);
  const int grid_w = kGridColumns * cell_w + (kGridColumns - 1) * grid_gap;
  const int grid_h = kGridRows * cell_h + (kGridRows - 1) * grid_gap;
  SetTextColor(dc, RGB(150, 170, 158));
  HGDIOBJ grid_font = SelectObject(dc, skin::font_small());
  const char* backpack_label = "BACKPACK  12 x 7";
  TextOutA(dc, grid_left, grid_top - 15 * s, backpack_label,
           static_cast<int>(std::strlen(backpack_label)));
  SelectObject(dc, grid_font);
  for (int row = 0; row < kGridRows; ++row) {
    for (int col = 0; col < kGridColumns; ++col) {
      RECT grid_cell{grid_left + col * (cell_w + grid_gap),
                     grid_top + row * (cell_h + grid_gap),
                     grid_left + col * (cell_w + grid_gap) + cell_w,
                     grid_top + row * (cell_h + grid_gap) + cell_h};
      if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot,
                              grid_cell))
        skin::slot(dc, grid_cell, RGB(58, 78, 70), false);
    }
  }
  state.hud_rect_trace.push_back(
      {"pane-grid", {grid_left, grid_top, grid_w, grid_h}});

  struct BackpackDraw {
    std::size_t item_index = 0;
    RECT rect{};
  };
  std::vector<BackpackDraw> backpack_draws;
  backpack_draws.reserve(items.size());
  for (std::size_t i = 0; i < items.size(); ++i) {
    if (items[i].equipped) continue;
    int slot = items[i].inventory_slot;
    if (slot < 0 || slot >= kGridColumns * kGridRows)
      slot = static_cast<int>(i % (kGridColumns * kGridRows));
    const int col = slot % kGridColumns;
    const int row = slot / kGridColumns;
    const int span_w = std::clamp(items[i].width, 1, kGridColumns - col);
    const int span_h = std::clamp(items[i].height, 1, kGridRows - row);
    RECT footprint{
        grid_left + col * (cell_w + grid_gap),
        grid_top + row * (cell_h + grid_gap),
        grid_left + col * (cell_w + grid_gap) + span_w * cell_w +
            (span_w - 1) * grid_gap,
        grid_top + row * (cell_h + grid_gap) + span_h * cell_h +
            (span_h - 1) * grid_gap};
    backpack_draws.push_back({i, footprint});
    state.inventory_hits.push_back({footprint, items[i].id});
    if (state.mouse.x >= footprint.left && state.mouse.x < footprint.right &&
        state.mouse.y >= footprint.top && state.mouse.y < footprint.bottom)
      state.selected_item = i;
  }
  if (items.empty()) {
    SetTextColor(dc, RGB(150, 160, 150));
    const char* empty = "Backpack empty. X picks up the nearest drop.";
    TextOutA(dc, grid_left, grid_top + grid_h + 6 * s, empty,
             static_cast<int>(strlen(empty)));
  } else {
    for (const auto& draw : backpack_draws) {
      const std::size_t i = draw.item_index;
      const RECT cell = draw.rect;
      const bool selected = i == std::min(state.selected_item, items.size() - 1);
      if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot,
                              cell))
        skin::slot(dc, cell, skin::kVerdigris, selected);
      if (selected) {
        HPEN cell_pen = CreatePen(PS_SOLID, 2, RGB(120, 214, 168));
        HGDIOBJ cp = SelectObject(dc, cell_pen);
        HGDIOBJ cb = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
        Rectangle(dc, cell.left, cell.top, cell.right, cell.bottom);
        SelectObject(dc, cb);
        SelectObject(dc, cp);
        DeleteObject(cell_pen);
      }
      RECT art_cell{cell.left + 2 * s, cell.top + 2 * s,
                    cell.right - 2 * s, cell.bottom - 2 * s};
      const std::string art_key = items[i].art_id.empty() ? items[i].id
                                                          : items[i].art_id;
      const bool drew_art = draw_item_art(state.billboards, dc, art_key, art_cell);
      if (!drew_art && items[i].expedition_map) {
        const std::string seal = "T" + std::to_string(items[i].map_tier);
        HGDIOBJ seal_font = SelectObject(dc, skin::font_title());
        SIZE seal_extent{};
        GetTextExtentPoint32A(dc, seal.c_str(), static_cast<int>(seal.size()),
                              &seal_extent);
        SetTextColor(dc, skin::kGold);
        TextOutA(dc, (art_cell.left + art_cell.right - seal_extent.cx) / 2,
                 (art_cell.top + art_cell.bottom - seal_extent.cy) / 2,
                 seal.c_str(), static_cast<int>(seal.size()));
        SelectObject(dc, seal_font);
      }
      rl.push_back({render::Op::PaneItem, static_cast<double>(cell.left),
                    static_cast<double>(cell.top), 0.0, items[i].attack_bonus,
                    items[i].name});
      state.hud_rect_trace.push_back(
          {"pane-item-footprint",
           {cell.left, cell.top, cell.right - cell.left,
            cell.bottom - cell.top}});
      if (items[i].quantity > 1) {
        const std::string count = std::to_string(items[i].quantity);
        HGDIOBJ count_font = SelectObject(dc, skin::font_small());
        SIZE count_extent{};
        GetTextExtentPoint32A(dc, count.c_str(), static_cast<int>(count.size()),
                              &count_extent);
        SetTextColor(dc, skin::kGold);
        TextOutA(dc, cell.right - count_extent.cx - 3 * s,
                 cell.bottom - count_extent.cy - 2 * s, count.c_str(),
                 static_cast<int>(count.size()));
        SelectObject(dc, count_font);
      }
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
  const WorldCarriedItem* selected = items.empty()
                                         ? nullptr
                                         : &items[std::min(state.selected_item,
                                                           items.size() - 1)];
  const int detail_top = grid_top + grid_h + 8 * s;
  if (selected) {
    const std::string selected_line = "SELECTED  " + selected->name +
        "   " + std::to_string(selected->width) + "x" +
        std::to_string(selected->height) +
        (selected->quantity > 1
             ? "   x" + std::to_string(selected->quantity)
             : std::string());
    HGDIOBJ selected_font = SelectObject(dc, skin::font_small());
    SetTextColor(dc, RGB(220, 230, 218));
    TextOutA(dc, left + 14 * s, detail_top, selected_line.c_str(),
             static_cast<int>(selected_line.size()));
    SelectObject(dc, selected_font);
    rl.push_back({render::Op::PaneStat, 0.0, 0.0, 0.0, 0,
                  "selected:" + selected->name});
  }
  std::string progression;
  std::string progression_detail;
  if (selected && selected->expedition_map) {
    const bool mastered = !selected->map_objective_key.empty() &&
        std::find(state.world.endgame.mastery_keys.begin(),
                  state.world.endgame.mastery_keys.end(),
                  selected->map_objective_key) !=
            state.world.endgame.mastery_keys.end();
    progression = "MAP T" + std::to_string(selected->map_tier) + "  +" +
                  std::to_string(selected->map_goods_found_percent) +
                  "% goods  | " + (mastered ? "MASTERED" : "NEW MASTERY") +
                  "  | ascent " +
                  std::to_string(state.world.endgame.ascent_chance_percent) +
                  "%";
    RECT map_card{left + 10 * s, detail_top + 16 * s, right - 10 * s,
                  bottom - 78 * s};
    if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot,
                            map_card))
      skin::slot(dc, map_card, skin::kGold, false);
    HGDIOBJ map_font = SelectObject(dc, skin::font_small());
    const std::string map_heading = "CHARTED TABLET  " + selected->map_family;
    SetTextColor(dc, skin::kGold);
    TextOutA(dc, map_card.left + 8 * s, map_card.top + 5 * s,
             map_heading.c_str(), static_cast<int>(map_heading.size()));
    int mod_y = map_card.top + 22 * s;
    HGDIOBJ mod_font = SelectObject(dc, skin::font_small());
    SetTextColor(dc, skin::kGold);
    for (std::size_t i = 0;
         i < selected->map_modifiers.size() && i < 2; ++i) {
      const std::string& modifier = selected->map_modifiers[i];
      TextOutA(dc, left + 14 * s, mod_y, modifier.c_str(),
               static_cast<int>(modifier.size()));
      rl.push_back({render::Op::PaneStat, 0.0, 0.0, 0.0, 0,
                    "map-modifier:" + modifier});
      mod_y += 16 * s;
    }
    SelectObject(dc, mod_font);
    if (map_card.bottom - map_card.top >= 76 * s) {
      const std::string tier_seal = "T" + std::to_string(selected->map_tier);
      HGDIOBJ tier_font = SelectObject(dc, skin::font_heading());
      SIZE tier_extent{};
      GetTextExtentPoint32A(dc, tier_seal.c_str(),
                            static_cast<int>(tier_seal.size()), &tier_extent);
      const int tier_x = map_card.right - 56 * s - tier_extent.cx / 2;
      SetTextColor(dc, RGB(238, 205, 108));
      TextOutA(dc, tier_x, map_card.top + 30 * s, tier_seal.c_str(),
               static_cast<int>(tier_seal.size()));
      SelectObject(dc, tier_font);
      const std::string goods = "+" +
          std::to_string(selected->map_goods_found_percent) + "% GOODS";
      SIZE goods_extent{};
      GetTextExtentPoint32A(dc, goods.c_str(), static_cast<int>(goods.size()),
                            &goods_extent);
      TextOutA(dc, map_card.right - 56 * s - goods_extent.cx / 2,
               map_card.top + 55 * s, goods.c_str(),
               static_cast<int>(goods.size()));
    }
    SelectObject(dc, map_font);
  } else if (selected && !selected->forge_lines.empty()) {
    progression = "FORGE  SPD +" +
                  std::to_string(player.attack_speed_percent) +
                  "%  REACH +" + std::to_string(player.reach_percent) +
                  "%  SHOT +" +
                  std::to_string(player.projectile_range_percent) + "%";
    progression_detail = "PEN " +
                  std::to_string(player.armour_penetration_percent) +
                  "%  BLEED " + std::to_string(player.bleed_chance) +
                  "%  MOVE +" +
                  std::to_string(player.movement_speed_percent) +
                  "%  WARDS E" +
                  std::to_string(player.ember_resistance) + " R" +
                  std::to_string(player.river_resistance);
    // A living Vessel can carry Brands, Bonds, and an awakening together.
    // Give the selected item a real detail card rather than silently clipping
    // its identity to the historical two-line footer.
    RECT vessel_card{left + 10 * s, detail_top + 16 * s, right - 10 * s,
                     bottom - 92 * s};
    if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot,
                            vessel_card))
      skin::slot(dc, vessel_card, skin::kVerdigris, false);

    HGDIOBJ line_font = SelectObject(dc, skin::font_small());
    SetTextColor(dc, skin::kGold);
    const std::string vessel_heading = "LIVING VESSEL  " + selected->name;
    TextOutA(dc, vessel_card.left + 8 * s, vessel_card.top + 5 * s,
             vessel_heading.c_str(), static_cast<int>(vessel_heading.size()));
    int line_y = vessel_card.top + 23 * s;
    const int lines_bottom = vessel_card.bottom - 5 * s;
    for (std::size_t i = 0; i < selected->forge_lines.size(); ++i) {
      const std::string& line = selected->forge_lines[i];
      const bool bond = line.rfind("BOND:", 0) == 0;
      const bool awakened = line.rfind("AWAKENED:", 0) == 0;
      SetTextColor(dc, awakened ? RGB(245, 198, 104)
                                : bond ? RGB(112, 224, 185)
                                       : RGB(150, 205, 178));
      RECT measure{vessel_card.left + 8 * s, line_y,
                   vessel_card.right - 8 * s, lines_bottom};
      DrawTextA(dc, line.c_str(), static_cast<int>(line.size()), &measure,
                DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_CALCRECT);
      const int line_height =
          std::max(14 * s, static_cast<int>(measure.bottom - measure.top));
      RECT text_rect{vessel_card.left + 8 * s, line_y,
                     vessel_card.right - 8 * s,
                     std::min(lines_bottom, line_y + line_height)};
      if (text_rect.top < lines_bottom) {
        DrawTextA(dc, line.c_str(), static_cast<int>(line.size()), &text_rect,
                  DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX |
                      DT_END_ELLIPSIS);
        state.hud_rect_trace.push_back(
            {"pane-vessel-property",
             {text_rect.left, text_rect.top, text_rect.right - text_rect.left,
              text_rect.bottom - text_rect.top}});
      }
      rl.push_back({render::Op::PaneStat, 0.0, 0.0, 0.0, 0,
                    "forge-line:" + line});
      line_y += line_height + 2 * s;
    }
    SelectObject(dc, line_font);
  } else if (state.world.progression.present) {
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
  const int progression_y =
      bottom - (progression_detail.empty() ? 74 : 82) * s;
  {
    SIZE extent{};
    GetTextExtentPoint32A(dc, progression.c_str(),
                          static_cast<int>(progression.size()), &extent);
    state.hud_rect_trace.push_back(
        {"pane-progression", {left + 14 * s, progression_y, extent.cx, extent.cy}});
  }
  TextOutA(dc, left + 14 * s, progression_y, progression.c_str(),
           static_cast<int>(progression.size()));
  if (!progression_detail.empty()) {
    rl.push_back(
        {render::Op::PaneStat, 0.0, 0.0, 0.0, 0, progression_detail});
    SIZE extent{};
    GetTextExtentPoint32A(dc, progression_detail.c_str(),
                          static_cast<int>(progression_detail.size()), &extent);
    const int detail_y = bottom - 66 * s;
    state.hud_rect_trace.push_back(
        {"pane-progression-detail",
         {left + 14 * s, detail_y, extent.cx, extent.cy}});
    TextOutA(dc, left + 14 * s, detail_y, progression_detail.c_str(),
             static_cast<int>(progression_detail.size()));
  }
  const std::string controls =
      selected && selected->expedition_map
          ? state.world.has_extraction
                ? "Arrows select | Tablet sealed until Crossroads | I close"
                : "Arrows select | V rechart 50g | Enter break | I close"
          : "Arrows select | Enter equip | U unequip | I close";
  {
    SIZE extent{};
    GetTextExtentPoint32A(dc, controls.c_str(), static_cast<int>(controls.size()),
                          &extent);
    state.hud_rect_trace.push_back(
        {"pane-footer", {left + 14 * s, bottom - 26 * s, extent.cx, extent.cy}});
  }
  TextOutA(dc, left + 14 * s, bottom - 26 * s, controls.c_str(),
           static_cast<int>(controls.size()));
  rl.push_back({render::Op::PaneStat, 0.0, 0.0, 0.0, 0,
                "gear-controls:" + controls});
  if (selected && selected->expedition_map && !state.world.has_extraction) {
    const std::string prefix = "Arrows select | ";
    const std::string action = "V rechart 50g";
    SIZE prefix_extent{};
    SIZE action_extent{};
    GetTextExtentPoint32A(dc, prefix.c_str(), static_cast<int>(prefix.size()),
                          &prefix_extent);
    GetTextExtentPoint32A(dc, action.c_str(), static_cast<int>(action.size()),
                          &action_extent);
    const int action_x = left + 14 * s + prefix_extent.cx;
    const int action_y = bottom - 26 * s;
    state.rechart_tablet_hit =
        RECT{action_x - 2 * s, action_y - 2 * s,
             action_x + action_extent.cx + 2 * s,
             action_y + action_extent.cy + 2 * s};
    state.rechart_tablet_hit_valid = true;
    state.hud_rect_trace.push_back(
        {"pane-rechart",
         {state.rechart_tablet_hit.left, state.rechart_tablet_hit.top,
          state.rechart_tablet_hit.right - state.rechart_tablet_hit.left,
          state.rechart_tablet_hit.bottom - state.rechart_tablet_hit.top}});
    SetTextColor(dc, skin::kGold);
    TextOutA(dc, action_x, action_y, action.c_str(),
             static_cast<int>(action.size()));
    rl.push_back({render::Op::PaneStat, 0.0, 0.0, 0.0, 0,
                  "gear-action:rechart"});
  }
}

void paint_quest_journal(ClientState& state, HDC dc, const RECT& bounds,
                         render::List& rl) {
  if (!state.quest_journal) return;
  const HudRect pane = quest_journal_rect(static_cast<int>(bounds.right),
                                          static_cast<int>(bounds.bottom));
  const int s = hud_scale(static_cast<int>(bounds.bottom));
  RECT frame{pane.x, pane.y, pane.x + pane.w, pane.y + pane.h};
  state.hud_rect_trace.push_back({"quest-pane-frame", pane});
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_panel,
                          frame))
    skin::panel(dc, frame, skin::kVerdigris, 248, 10.0f);

  const verdigris::client::ClientQuestState empty;
  const auto& quests = state.session ? state.session->model().quests : empty;
  SetBkMode(dc, TRANSPARENT);
  HGDIOBJ old_font = SelectObject(dc, skin::font_heading());
  SetTextColor(dc, RGB(230, 235, 220));
  const char* heading = "CHRONICLE COMMISSIONS";
  TextOutA(dc, frame.left + 22 * s, frame.top + 17 * s, heading,
           static_cast<int>(std::strlen(heading)));
  rl.push_back({render::Op::Hud, static_cast<double>(frame.left),
                static_cast<double>(frame.top), 0.0, 0, "quest-journal"});

  SelectObject(dc, skin::font_small());
  SetTextColor(dc, RGB(145, 170, 156));
  const std::string ledger = quests.present
      ? (quests.act_number > 0
             ? "ACT " + std::to_string(quests.act_number) + "  " +
                   quests.act_title + "   |   " +
                   std::to_string(quests.quest_points) + "/" +
                   std::to_string(quests.campaign_quest_total) +
                   " POINTS   |   " + std::to_string(quests.house_renown) +
                   " RENOWN"
             : std::to_string(quests.quest_points) + " QUEST POINTS   /   " +
                   std::to_string(quests.house_renown) + " HOUSE RENOWN")
      : state.world.endgame.unlocked
          ? "WAYFINDER MASTERY  " +
                std::to_string(state.world.endgame.mastered) + " / " +
                std::to_string(state.world.endgame.mastery_total)
          : "AWAITING AN AUTHORITATIVE CAMPAIGN RECORD";
  TextOutA(dc, frame.left + 22 * s, frame.top + 46 * s, ledger.c_str(),
           static_cast<int>(ledger.size()));
  if (quests.present && quests.act_number > 0) {
    rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0, quests.act_number,
                  "campaign-act:" + std::to_string(quests.act_number) + ":" +
                      quests.act_title});
  }

  const int rail_w = 214 * s;
  RECT rail{frame.left + 18 * s, frame.top + 72 * s,
            frame.left + 18 * s + rail_w, frame.bottom - 46 * s};
  skin::panel(dc, rail, RGB(58, 78, 70), 225, 5.0f);
  SelectObject(dc, skin::font_body_bold());
  SetTextColor(dc, RGB(196, 208, 196));
  const char* record = "HOUSE RECORD";
  TextOutA(dc, rail.left + 12 * s, rail.top + 11 * s, record,
           static_cast<int>(std::strlen(record)));
  SelectObject(dc, skin::font_small());
  int row_y = rail.top + 39 * s;
  int seal = 1;
  constexpr std::size_t kVisibleDeeds = 9;
  const std::size_t hidden_deeds = quests.completed.size() > kVisibleDeeds
      ? quests.completed.size() - kVisibleDeeds
      : 0;
  if (hidden_deeds > 0) {
    SetTextColor(dc, RGB(142, 161, 149));
    const std::string earlier = "+ " + std::to_string(hidden_deeds) +
                                " EARLIER DEEDS";
    TextOutA(dc, rail.left + 12 * s, row_y, earlier.c_str(),
             static_cast<int>(earlier.size()));
    rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0,
                  static_cast<int>(hidden_deeds), "quest-earlier-deeds"});
    row_y += 29 * s;
  }
  for (std::size_t i = hidden_deeds; i < quests.completed.size(); ++i) {
    const auto& done = quests.completed[i];
    SetTextColor(dc, RGB(202, 176, 104));
    const std::string row = "[x] " + (done.title.empty() ? done.id : done.title);
    TextOutA(dc, rail.left + 12 * s, row_y, row.c_str(),
             static_cast<int>(row.size()));
    rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0, seal++,
                  "quest-complete:" + done.id});
    row_y += 29 * s;
  }
  if (!quests.active_id.empty()) {
    SetTextColor(dc, RGB(124, 220, 172));
    const std::string row = "[>] " + quests.title;
    RECT active_row{rail.left + 12 * s, row_y, rail.right - 10 * s,
                    row_y + 38 * s};
    DrawTextA(dc, row.c_str(), static_cast<int>(row.size()), &active_row,
              DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
    rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0, seal,
                  "quest-active:" + quests.active_id});
  } else if (quests.campaign_complete || state.world.endgame.unlocked) {
    SetTextColor(dc, RGB(239, 208, 116));
    const char* complete = "[x] CAMPAIGN SEALED";
    TextOutA(dc, rail.left + 12 * s, row_y, complete,
             static_cast<int>(std::strlen(complete)));
  }

  const int detail_left = rail.right + 22 * s;
  const int detail_right = frame.right - 22 * s;
  if (state.world.endgame.unlocked &&
      (!quests.present ||
       (quests.campaign_complete && quests.active_id.empty()))) {
    SelectObject(dc, skin::font_title());
    SetTextColor(dc, RGB(239, 208, 116));
    const char* title = "WAYFINDER'S LEDGER";
    TextOutA(dc, detail_left, rail.top + 10 * s, title,
             static_cast<int>(std::strlen(title)));
    SelectObject(dc, skin::font_small());
    SetTextColor(dc, RGB(164, 183, 169));
    const std::string summary =
        std::to_string(state.world.endgame.mastered) + " / " +
        std::to_string(state.world.endgame.mastery_total) +
        " OBJECTIVES   |   HIGHEST T" +
        std::to_string(state.world.endgame.highest_tier) +
        "   |   ASCENT " +
        std::to_string(state.world.endgame.ascent_chance_percent) + "%";
    TextOutA(dc, detail_left, rail.top + 50 * s, summary.c_str(),
             static_cast<int>(summary.size()));
    rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0,
                  state.world.endgame.mastered,
                  "endgame-mastery:" +
                      std::to_string(state.world.endgame.mastered) + "/" +
                      std::to_string(state.world.endgame.mastery_total)});

    const struct { const char* key; const char* label; } families[] = {
        {"barrow", "BARROW"}, {"reeds", "REEDS"},
        {"crown", "CROWN"}, {"thorns", "THORNS"}};
    int family_y = rail.top + 92 * s;
    for (const auto& family : families) {
      SetTextColor(dc, RGB(196, 208, 196));
      TextOutA(dc, detail_left, family_y + 2 * s, family.label,
               static_cast<int>(std::strlen(family.label)));
      int family_mastered = 0;
      for (int tier = 1; tier <= 16; ++tier) {
        const std::string key = std::string(family.key) + ":" +
                                std::to_string(tier);
        const bool mastered =
            std::find(state.world.endgame.mastery_keys.begin(),
                      state.world.endgame.mastery_keys.end(), key) !=
            state.world.endgame.mastery_keys.end();
        if (mastered) ++family_mastered;
        const int x = detail_left + (82 + (tier - 1) * 18) * s;
        RECT pip{x, family_y, x + 13 * s, family_y + 13 * s};
        HBRUSH brush = CreateSolidBrush(mastered ? RGB(202, 176, 104)
                                                  : RGB(39, 52, 47));
        FillRect(dc, &pip, brush);
        DeleteObject(brush);
      }
      rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0, family_mastered,
                    std::string("endgame-family:") + family.key + ":" +
                        std::to_string(family_mastered)});
      family_y += 34 * s;
    }
    SetTextColor(dc, RGB(156, 174, 161));
    RECT copy{detail_left, family_y + 10 * s, detail_right,
              frame.bottom - 58 * s};
    const char* text = "Each first family-and-tier Warden clear seals one objective, grants House renown, and improves the chance that the next tablet ascends a tier.";
    DrawTextA(dc, text, static_cast<int>(std::strlen(text)), &copy,
              DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
  } else if (!quests.present) {
    SelectObject(dc, skin::font_body());
    SetTextColor(dc, RGB(170, 184, 174));
    RECT absent{detail_left, rail.top + 8 * s, detail_right,
                frame.bottom - 70 * s};
    const char* copy = "No campaign state has arrived from the realm server. The journal will never substitute local objectives or fabricated rewards.";
    DrawTextA(dc, copy, static_cast<int>(std::strlen(copy)), &absent,
              DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
  } else {
    SelectObject(dc, skin::font_title());
    SetTextColor(dc, RGB(235, 224, 188));
    TextOutA(dc, detail_left, rail.top + 4 * s, quests.title.c_str(),
             static_cast<int>(quests.title.size()));
    SelectObject(dc, skin::font_small());
    SetTextColor(dc, RGB(130, 176, 151));
    const std::string giver = "COMMISSIONED BY " + quests.giver;
    TextOutA(dc, detail_left, rail.top + 37 * s, giver.c_str(),
             static_cast<int>(giver.size()));
    SelectObject(dc, skin::font_body());
    SetTextColor(dc, RGB(178, 192, 181));
    RECT summary{detail_left, rail.top + 67 * s, detail_right,
                 rail.top + 122 * s};
    DrawTextA(dc, quests.summary.c_str(), static_cast<int>(quests.summary.size()),
              &summary, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

    const int progress_top = rail.top + 136 * s;
    const int progress_w = detail_right - detail_left;
    RECT track{detail_left, progress_top, detail_right, progress_top + 7 * s};
    HBRUSH track_brush = CreateSolidBrush(RGB(30, 39, 37));
    FillRect(dc, &track, track_brush);
    DeleteObject(track_brush);
    const double ratio = quests.objective_count > 0
        ? static_cast<double>(quests.objective_index + 1) / quests.objective_count
        : 0.0;
    RECT fill = track;
    fill.right = fill.left + static_cast<int>(progress_w * std::clamp(ratio, 0.0, 1.0));
    HBRUSH fill_brush = CreateSolidBrush(RGB(104, 180, 139));
    FillRect(dc, &fill, fill_brush);
    DeleteObject(fill_brush);

    SelectObject(dc, skin::font_small());
    SetTextColor(dc, RGB(142, 161, 149));
    const std::string count = "CURRENT RITE  " +
        std::to_string(quests.objective_index + 1) + " / " +
        std::to_string(quests.objective_count);
    TextOutA(dc, detail_left, progress_top + 17 * s, count.c_str(),
             static_cast<int>(count.size()));
    RECT objective_box{detail_left, progress_top + 44 * s, detail_right,
                       progress_top + 117 * s};
    skin::panel(dc, objective_box, RGB(72, 104, 88), 235, 5.0f);
    SelectObject(dc, skin::font_body_bold());
    SetTextColor(dc, RGB(222, 232, 218));
    RECT objective_text{objective_box.left + 13 * s,
                        objective_box.top + 13 * s,
                        objective_box.right - 13 * s,
                        objective_box.bottom - 10 * s};
    DrawTextA(dc, quests.objective.c_str(),
              static_cast<int>(quests.objective.size()), &objective_text,
              DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
    rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0,
                  quests.objective_index, "quest-objective:" + quests.objective});

    SelectObject(dc, skin::font_small());
    SetTextColor(dc, RGB(202, 176, 104));
    const std::string reward = "REWARD  " + quests.reward;
    TextOutA(dc, detail_left, progress_top + 138 * s, reward.c_str(),
             static_cast<int>(reward.size()));
  }

  SelectObject(dc, skin::font_small());
  SetTextColor(dc, RGB(135, 151, 141));
  const char* controls = "J / ESC close journal";
  TextOutA(dc, frame.right - 154 * s, frame.bottom - 27 * s, controls,
           static_cast<int>(std::strlen(controls)));
  state.hud_rect_trace.push_back(
      {"quest-pane-footer", {frame.right - 154 * s, frame.bottom - 27 * s,
                              142 * s, 16 * s}});
  SelectObject(dc, old_font);
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

void paint_vital_orbs(const BillboardAssets& assets, const WorldActor& player,
                      std::uint64_t tick, int screen_pulse_ticks, HDC dc,
                      const RECT& bounds, render::List& rl,
                      std::vector<std::pair<std::string, HudRect>>* trace) {
  if (!player.alive && player.life <= 0 && player.life_max <= 0) return;
  const int radius =
      kVitalOrbRadius * hud_scale(static_cast<int>(bounds.bottom));
  const int bottom = static_cast<int>(bounds.bottom) - 18;
  const int s = hud_scale(static_cast<int>(bounds.bottom));
  const int left_cx = 36 * s + radius;
  const int right_cx = static_cast<int>(bounds.right) - 36 * s - radius;
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
  const HudRect life_chrome = vital_orb_rect(static_cast<int>(bounds.right),
                                             static_cast<int>(bounds.bottom), false);
  const HudRect resource_chrome = vital_orb_rect(
      static_cast<int>(bounds.right), static_cast<int>(bounds.bottom), true);
  const bool life_raster = draw_framekit_sprite(
      assets, dc, assets.fk_orb_life,
      RECT{life_chrome.x, life_chrome.y, life_chrome.x + life_chrome.w,
           life_chrome.y + life_chrome.h});
  const bool resource_raster = draw_framekit_sprite(
      assets, dc, assets.fk_orb_resource,
      RECT{resource_chrome.x, resource_chrome.y,
           resource_chrome.x + resource_chrome.w,
           resource_chrome.y + resource_chrome.h});
  if (life_raster)
    rl.push_back({render::Op::Hud, static_cast<double>(left_cx),
                  static_cast<double>(cy), 0.0, 0,
                  "framekit-raster:orb-life"});
  if (resource_raster)
    rl.push_back({render::Op::Hud, static_cast<double>(right_cx),
                  static_cast<double>(cy), 0.0, 0,
                  "framekit-raster:orb-resource"});
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
    std::string slot_name = slot.name;
    if (i == 0 && player.combo_window_ticks > 0) {
      if (player.combo_step == 1) slot_name = "Strike II";
      else if (player.combo_step == 2) slot_name = "FINISH";
      else if (player.combo_step == 3) slot_name = "Strike I";
    }
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
    const SpriteBitmap* skill_art = nullptr;
    switch (i) {
      case 0: skill_art = &state.billboards.fk_skill_strike; break;
      case 1: skill_art = &state.billboards.fk_skill_thrust; break;
      case 2: skill_art = &state.billboards.fk_skill_sweep; break;
      case 3: skill_art = &state.billboards.fk_skill_warcry; break;
    }
    if (skill_art) {
      RECT icon{box.left + 4 * s, box.top + 3 * s,
                box.right - 4 * s, box.bottom - 3 * s};
      if (draw_framekit_sprite(state.billboards, dc, *skill_art, icon, 178))
        rl.push_back({render::Op::Hud, static_cast<double>(cx),
                      static_cast<double>(cy), static_cast<double>(slot_w), i,
                      std::string("framekit-skill:") + slot.name});
    }

    if (cooldown && player.cooldown_ticks > 0) {
      // Older servers without a duration show a fully blocked slot, not a
      // fictitious 30-tick clock. Current servers report the accepted cadence,
      // including Sweep, combo finishers, equipment and temporary haste.
      const int max_ticks = (std::max)(player.cooldown_ticks, player.cooldown_total_ticks);
      const double sweep =
          std::clamp(static_cast<double>(player.cooldown_ticks) / max_ticks, 0.0, 1.0);
      skin::cooldown_wedge(dc, box, sweep);
      rl.push_back({render::Op::Quickbar, static_cast<double>(cx),
                    static_cast<double>(cy), sweep, player.cooldown_ticks,
                    "cooldown-radial:" + std::string(slot.name)});
    }

    rl.push_back({render::Op::Quickbar, static_cast<double>(cx), static_cast<double>(cy),
                  static_cast<double>(slot_w), active ? 1 : 0,
                  std::string(slot.key_label) + ":" + slot_name});

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(239, 208, 116));
    TextOutA(dc, box.left + 6 * s, box.top + 4 * s, slot.key_label,
             static_cast<int>(strlen(slot.key_label)));
    SetTextColor(dc, available ? RGB(205, 221, 207) : RGB(112, 119, 115));
    TextOutA(dc, box.left + 6 * s, box.top + 20 * s, slot_name.c_str(),
             static_cast<int>(slot_name.size()));
    if (i == 0) {
      const int lit = player.combo_window_ticks > 0
                          ? std::clamp(player.combo_step, 0, 3)
                          : 0;
      const int pip_w = 10 * s;
      const int pip_gap = 3 * s;
      const int pips_w = 3 * pip_w + 2 * pip_gap;
      const int pip_left = cx - pips_w / 2;
      for (int pip = 0; pip < 3; ++pip) {
        RECT pip_rect{pip_left + pip * (pip_w + pip_gap), box.bottom - 9 * s,
                      pip_left + pip * (pip_w + pip_gap) + pip_w,
                      box.bottom - 5 * s};
        HBRUSH pip_brush = CreateSolidBrush(
            pip < lit ? RGB(104, 232, 204) : RGB(54, 66, 64));
        FillRect(dc, &pip_rect, pip_brush);
        DeleteObject(pip_brush);
      }
      rl.push_back({render::Op::Quickbar, static_cast<double>(cx),
                    static_cast<double>(box.bottom - 7 * s),
                    static_cast<double>(pips_w), lit,
                    "combo-cadence:" + std::to_string(lit) + ":" +
                        std::to_string(player.combo_window_ticks)});
    }
  }

  // Living-item powers sit immediately above the actions they modify. Only
  // authoritative combat-state timers/readiness flags are shown here.
  std::vector<std::string> bond_states;
  const auto seconds = [](int ticks) {
    return std::to_string((ticks + 19) / 20) + "s";
  };
  if (player.bond_attack_speed_ticks > 0)
    bond_states.push_back("RHYTHM " + seconds(player.bond_attack_speed_ticks));
  if (player.bond_movement_speed_ticks > 0)
    bond_states.push_back("SPRINT " + seconds(player.bond_movement_speed_ticks));
  if (player.bond_old_grudge_ticks > 0)
    bond_states.push_back("GRUDGE " + seconds(player.bond_old_grudge_ticks));
  if (player.bond_last_stand_ready) bond_states.push_back("LAST STAND READY");
  if (player.bond_untraceable_ready) bond_states.push_back("UNTRACEABLE READY");
  if (!bond_states.empty()) {
    const int chip_gap = 6 * s;
    int total_width = 0;
    std::vector<int> widths;
    for (const auto& label : bond_states) {
      const int width = (18 + static_cast<int>(label.size()) * 7) * s;
      widths.push_back(width);
      total_width += width;
    }
    total_width += chip_gap * (static_cast<int>(bond_states.size()) - 1);
    int chip_left = (static_cast<int>(bounds.right) - total_width) / 2;
    const int chip_bottom = strip.top - 5 * s;
    const int chip_top = chip_bottom - 22 * s;
    for (std::size_t i = 0; i < bond_states.size(); ++i) {
      RECT chip{chip_left, chip_top, chip_left + widths[i], chip_bottom};
      skin::panel(dc, chip);
      SetBkMode(dc, TRANSPARENT);
      SetTextColor(dc, RGB(104, 232, 204));
      HGDIOBJ old_font = SelectObject(dc, skin::font_small());
      TextOutA(dc, chip.left + 9 * s, chip.top + 5 * s,
               bond_states[i].c_str(), static_cast<int>(bond_states[i].size()));
      SelectObject(dc, old_font);
      rl.push_back({render::Op::Quickbar,
                    static_cast<double>((chip.left + chip.right) / 2),
                    static_cast<double>((chip.top + chip.bottom) / 2),
                    static_cast<double>(widths[i]), 1,
                    "bond-state:" + bond_states[i]});
      chip_left += widths[i] + chip_gap;
    }
  }
}

bool trade_pane_open(const ClientState& state);

// World hover information follows the ARPG convention: names and a few
// decision-relevant facts appear only under the pointer, while the combat
// field stays clean at rest. Every fact comes from the authoritative model.
void paint_hover_tooltip(ClientState& state, HDC dc, const RECT& bounds,
                         render::List& rl) {
  if (trade_pane_open(state) || state.gear_overlay || state.quest_journal || state.tree_pane ||
      state.character_pane || state.minimap_mode == MinimapMode::Overlay)
    return;

  const WorldView& world = state.world;
  const int mx = state.mouse.x;
  const int my = state.mouse.y;
  const int s = hud_scale(static_cast<int>(bounds.bottom));
  std::string title;
  COLORREF title_color = skin::kInk;
  std::vector<std::string> facts;
  double best_distance = std::numeric_limits<double>::max();

  const auto consider = [&](int sx, int sy, double radius_px,
                            const std::string& candidate_title,
                            COLORREF candidate_color,
                            std::vector<std::string> candidate_facts) {
    if (candidate_title.empty()) return;
    const double dx = static_cast<double>(mx - sx);
    const double dy = static_cast<double>(my - sy);
    const double distance = dx * dx + dy * dy;
    if (distance > radius_px * radius_px || distance >= best_distance) return;
    best_distance = distance;
    title = candidate_title;
    title_color = candidate_color;
    facts = std::move(candidate_facts);
  };

  for (const auto& monster : world.monsters) {
    if (!monster.alive) continue;
    const ScreenPoint base =
        project(state.camera, bounds, monster.position.x, monster.position.y);
    const int body_y = base.y - static_cast<int>(kTileUnits * 0.7 * base.scale);
    std::vector<std::string> monster_facts{
        "Life " + std::to_string(monster.life) + " / " +
        std::to_string(monster.life_max)};
    if (monster.armour > 0)
      monster_facts.push_back("Armour " + std::to_string(monster.armour));
    if (!monster.behaviour.empty())
      monster_facts.push_back("Role " + monster.behaviour);
    if (monster.damage_channel == "river" || monster.damage_channel == "ember")
      monster_facts.push_back("Damage " + monster.damage_channel);
    if (monster.bleeding) monster_facts.push_back("Bleeding");
    consider(base.x, body_y, kTileUnits * 0.9 * base.scale,
             monster.name.empty() ? std::string("Unknown foe") : monster.name,
             monster.elite ? skin::kGold : skin::kEmber,
             std::move(monster_facts));
  }
  for (const auto& loot : state.loot_positions) {
    const ScreenPoint base =
        project(state.camera, bounds, loot.second.x, loot.second.y);
    consider(base.x, base.y - static_cast<int>(kTileUnits * 0.28 * base.scale),
             kTileUnits * 0.5 * base.scale, loot_label(state, loot.first),
             skin::kGold, {"X picks up"});
  }
  for (const auto& npc : world.npcs) {
    const ScreenPoint base =
        project(state.camera, bounds, npc.position.x, npc.position.y);
    const int body_y = base.y - static_cast<int>(kTileUnits * 0.7 * base.scale);
    const bool house_steward =
        std::find(npc.services.begin(), npc.services.end(),
                  std::string("house_investment")) != npc.services.end();
    const std::string verb = house_steward
                                 ? "talk"
                                 : npc.actions.empty() ? std::string("examine")
                                                       : npc.actions.front();
    consider(base.x, body_y, kTileUnits * 0.9 * base.scale, npc.name,
             RGB(150, 190, 240), {"T to " + verb});
  }
  if (title.empty()) return;

  HGDIOBJ old_font = SelectObject(dc, skin::font_body_bold());
  SIZE title_extent{};
  GetTextExtentPoint32A(dc, title.c_str(), static_cast<int>(title.size()),
                        &title_extent);
  int widest = title_extent.cx;
  SelectObject(dc, skin::font_small());
  for (const auto& fact : facts) {
    SIZE extent{};
    GetTextExtentPoint32A(dc, fact.c_str(), static_cast<int>(fact.size()),
                          &extent);
    widest = std::max(widest, static_cast<int>(extent.cx));
  }
  const int line_h = 16 * s;
  const int pad = 8 * s;
  const int box_w = widest + pad * 2;
  const int box_h = title_extent.cy + static_cast<int>(facts.size()) * line_h +
                    pad * 2;
  int box_x = std::clamp(mx + 18, 8,
                         std::max(8, static_cast<int>(bounds.right) - box_w - 8));
  const int box_y = std::max(8, my - box_h - 10);
  RECT plate{box_x, box_y, box_x + box_w, box_y + box_h};
  const bool raster_tooltip =
      draw_framekit_tooltip(state.billboards, dc, plate);
  if (!raster_tooltip)
    skin::panel(dc, plate, title_color, 245, 3.0f);
  else
    rl.push_back({render::Op::Hud, static_cast<double>(box_x),
                  static_cast<double>(box_y), 0.0, 0,
                  "framekit-raster:tooltip-frame"});
  SetBkMode(dc, TRANSPARENT);
  SelectObject(dc, skin::font_body_bold());
  SetTextColor(dc, title_color);
  TextOutA(dc, box_x + pad, box_y + pad - 2, title.c_str(),
           static_cast<int>(title.size()));
  SelectObject(dc, skin::font_small());
  SetTextColor(dc, skin::kInkDim);
  int fact_y = box_y + pad + title_extent.cy;
  for (const auto& fact : facts) {
    TextOutA(dc, box_x + pad, fact_y, fact.c_str(),
             static_cast<int>(fact.size()));
    fact_y += line_h;
  }
  SelectObject(dc, old_font);
  rl.push_back({render::Op::Hud, static_cast<double>(box_x),
                static_cast<double>(box_y), 0.0, 0, "tooltip:" + title});
}

// A thin bottom-edge strip makes long-term combat progress readable without
// competing with the quickbar. The server supplies the exact current-level
// span; this function only draws its normalized fraction.
void paint_xp_bar(ClientState& state, HDC dc, const RECT& bounds,
                  render::List& rl) {
  if (!state.world.xp_present) return;
  const int s = hud_scale(static_cast<int>(bounds.bottom));
  const int orb_reach = (36 + kVitalOrbRadius * 2 + 24) * s;
  const int left = orb_reach;
  const int right = static_cast<int>(bounds.right) - orb_reach;
  if (right - left < 60) return;
  const int height = 6 * s;
  const int top = static_cast<int>(bounds.bottom) - height - 4;
  const double fraction = std::clamp(state.world.xp_fraction, 0.0, 1.0);
  const RECT rail{left - 12 * s, top - 6 * s, right + 12 * s,
                  top + height + 6 * s};
  if (draw_framekit_sprite(state.billboards, dc,
                           state.billboards.fk_xp_rail, rail))
    rl.push_back({render::Op::Hud, static_cast<double>(left),
                  static_cast<double>(top), 0.0, 0,
                  "framekit-raster:xp-rail"});
  skin::progress_bar(dc, RECT{left, top, right, top + height}, fraction,
                     skin::kGold, 10);
  rl.push_back({render::Op::Hud, static_cast<double>(left),
                static_cast<double>(top), 0.0,
                static_cast<int>(std::lround(fraction * 100.0)), "xp-bar"});
}

void paint_minimap(ClientState& state, HDC dc, const RECT& bounds,
                   render::List& rl) {
  const int width = static_cast<int>(bounds.right);
  const int height = static_cast<int>(bounds.bottom);
  const bool overlay = state.minimap_mode == MinimapMode::Overlay;
  const MinimapSide minimap_side = active_minimap_side(state);
  const HudRect map = overlay
                          ? minimap_overlay_rect(width, height)
                          : minimap_rect(width, height, minimap_side);
  const int s = hud_scale(height);
  RECT panel{map.x, map.y, map.x + map.w, map.y + map.h};
  state.hud_rect_trace.push_back(
      {overlay ? "minimap-overlay" : "minimap", map});

  const BYTE body_alpha = minimap_body_alpha(state.minimap_opacity_step);
  if (overlay) {
    // The WIZARD nine-slice is opacity-controlled as one composited plate so
    // the world remains readable beneath it. The shared vector skin remains
    // the fail-closed presentation when assets are unavailable.
    if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_panel,
                            panel, body_alpha))
      skin::panel(dc, panel, skin::kVerdigris, body_alpha, 8.0f);
  } else {
    skin::panel(dc, panel, skin::kPanelBorder, 228, 6.0f);
  }

  const int header_h = overlay ? 50 * s : 5 * s;
  const int footer_h = overlay ? 42 * s : 5 * s;
  RECT plot{panel.left + 7 * s, panel.top + header_h,
            panel.right - 7 * s, panel.bottom - footer_h};
  if (plot.right <= plot.left || plot.bottom <= plot.top) return;
  state.hud_rect_trace.push_back(
      {overlay ? "minimap-overlay-plot" : "minimap-plot",
       {plot.left, plot.top, plot.right - plot.left, plot.bottom - plot.top}});

  const WorldView& world = state.world;
  const double arena =
      static_cast<double>(verdigris::world_scale::kArenaHalfExtent);
  const int plot_span = std::min(plot.right - plot.left, plot.bottom - plot.top);
  const double map_scale =
      static_cast<double>(plot_span) / (arena * 2.2) *
      minimap_zoom_factor(state.minimap_zoom_step);
  const int center_x = (plot.left + plot.right) / 2;
  const int center_y = (plot.top + plot.bottom) / 2;
  const double origin_x = static_cast<double>(world.player.position.x);
  const double origin_y = static_cast<double>(world.player.position.y);
  auto to_map = [&](double wx, double wy) {
    const int mx = center_x +
                   static_cast<int>(std::lround((wx - origin_x) * map_scale));
    const int my = center_y +
                   static_cast<int>(std::lround((wy - origin_y) * map_scale));
    return std::pair<int, int>{mx, my};
  };
  const auto inside_plot = [&](int x, int y, int pad = 0) {
    return x >= plot.left + pad && x < plot.right - pad &&
           y >= plot.top + pad && y < plot.bottom - pad;
  };

  const int saved = SaveDC(dc);
  IntersectClipRect(dc, plot.left, plot.top, plot.right, plot.bottom);

  // Draw the authoritative walkability topology first. Only the visible tile
  // envelope is visited, keeping both the small map and a live overlay cheap
  // even when a future act supplies a much larger grid.
  int topology_cells = 0;
  const bool valid_grid =
      world.map_width > 0 && world.map_height > 0 &&
      world.map_walkable.size() ==
          static_cast<std::size_t>(world.map_width) * world.map_height;
  if (valid_grid) {
    const double half_world_x =
        static_cast<double>(plot.right - plot.left) * 0.5 / map_scale;
    const double half_world_y =
        static_cast<double>(plot.bottom - plot.top) * 0.5 / map_scale;
    const int tx0 = std::clamp(
        static_cast<int>(std::floor((origin_x - half_world_x) / kTileUnits)) - 1,
        0, world.map_width - 1);
    const int tx1 = std::clamp(
        static_cast<int>(std::ceil((origin_x + half_world_x) / kTileUnits)) + 1,
        0, world.map_width - 1);
    const int ty0 = std::clamp(
        static_cast<int>(std::floor((origin_y - half_world_y) / kTileUnits)) - 1,
        0, world.map_height - 1);
    const int ty1 = std::clamp(
        static_cast<int>(std::ceil((origin_y + half_world_y) / kTileUnits)) + 1,
        0, world.map_height - 1);
    HBRUSH floor_brush = CreateSolidBrush(
        overlay ? RGB(48, 76, 66) : RGB(43, 61, 54));
    HPEN grid_pen = CreatePen(PS_SOLID, 1,
                              overlay ? RGB(80, 116, 100)
                                      : RGB(57, 78, 69));
    HGDIOBJ old_pen = SelectObject(dc, grid_pen);
    for (int ty = ty0; ty <= ty1; ++ty) {
      for (int tx = tx0; tx <= tx1; ++tx) {
        const std::size_t index =
            static_cast<std::size_t>(ty) * world.map_width + tx;
        if (!world.map_walkable[index]) continue;
        const auto [x0, y0] = to_map((tx - 0.46) * kTileUnits,
                                     (ty - 0.46) * kTileUnits);
        const auto [x1, y1] = to_map((tx + 0.46) * kTileUnits,
                                     (ty + 0.46) * kTileUnits);
        RECT cell{std::min(x0, x1), std::min(y0, y1),
                  std::max(x0, x1) + 1, std::max(y0, y1) + 1};
        FillRect(dc, &cell, floor_brush);
        if (overlay && cell.right - cell.left >= 5 &&
            cell.bottom - cell.top >= 5) {
          HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
          Rectangle(dc, cell.left, cell.top, cell.right, cell.bottom);
          SelectObject(dc, old_brush);
        }
        ++topology_cells;
      }
    }
    SelectObject(dc, old_pen);
    DeleteObject(grid_pen);
    DeleteObject(floor_brush);
  }

  const int marker_scale = overlay ? std::max(2, plot_span / 240)
                                   : std::max(1, map.w / 108);
  int dots = 0;
  for (const auto& item : state.scenery) {
    const auto [mx, my] = to_map(item.position.x, item.position.y);
    if (!inside_plot(mx, my, 2)) continue;
    fill_ellipse(dc, mx, my, 2 * marker_scale, 2 * marker_scale,
                 RGB(96, 112, 98));
    ++dots;
  }

  if (world.has_extraction) {
    const auto [mx, my] = to_map(world.extraction.x, world.extraction.y);
    if (inside_plot(mx, my)) {
      fill_ellipse(dc, mx, my, 4 * marker_scale, 4 * marker_scale,
                   RGB(239, 208, 116));
      ring_ellipse(dc, mx, my, 6 * marker_scale, 6 * marker_scale,
                   RGB(128, 216, 184), std::max(1, marker_scale));
      ++dots;
    }
  }

  for (const auto& monster : world.monsters) {
    if (!monster.alive) continue;
    const auto [mx, my] = to_map(monster.position.x, monster.position.y);
    if (!inside_plot(mx, my)) continue;
    fill_ellipse(dc, mx, my,
                 (monster.elite ? 4 : 3) * marker_scale,
                 (monster.elite ? 4 : 3) * marker_scale,
                 monster.elite ? RGB(229, 164, 73) : RGB(196, 58, 48));
    ++dots;
  }

  for (const auto& npc : world.npcs) {
    auto [mx, my] = to_map(npc.position.x, npc.position.y);
    if (!overlay) {
      // Corner-map NPCs clamp to the border as a directional cue. The large
      // overlay is spacious enough to hide truly off-map figures instead.
      mx = std::clamp(mx, static_cast<int>(plot.left) + 3,
                      static_cast<int>(plot.right) - 3);
      my = std::clamp(my, static_cast<int>(plot.top) + 3,
                      static_cast<int>(plot.bottom) - 3);
    }
    if (!inside_plot(mx, my)) continue;
    fill_ellipse(dc, mx, my, 3 * marker_scale, 3 * marker_scale,
                 RGB(122, 168, 230));
    ++dots;
  }

  const auto [px, py] = to_map(origin_x, origin_y);
  const double facing_angle =
      std::atan2(world.player.facing.y, world.player.facing.x);
  const int arrow_scale = overlay ? std::max(2, marker_scale) : marker_scale;
  const int tip_x = px +
                    static_cast<int>(std::cos(facing_angle) * 8.0 * arrow_scale);
  const int tip_y = py +
                    static_cast<int>(std::sin(facing_angle) * 8.0 * arrow_scale);
  const int wing_x = px -
                     static_cast<int>(std::cos(facing_angle) * 4.0 * arrow_scale);
  const int wing_y = py -
                     static_cast<int>(std::sin(facing_angle) * 4.0 * arrow_scale);
  const double wing = facing_angle + kPi * 0.75;
  const int wing_a_x = wing_x +
                       static_cast<int>(std::cos(wing) * 5.0 * arrow_scale);
  const int wing_a_y = wing_y +
                       static_cast<int>(std::sin(wing) * 5.0 * arrow_scale);
  const int wing_b_x = wing_x + static_cast<int>(
                                    std::cos(wing + kPi * 0.5) * 5.0 * arrow_scale);
  const int wing_b_y = wing_y + static_cast<int>(
                                    std::sin(wing + kPi * 0.5) * 5.0 * arrow_scale);
  POINT arrow[3] = {{tip_x, tip_y}, {wing_a_x, wing_a_y},
                    {wing_b_x, wing_b_y}};
  HBRUSH player_brush = CreateSolidBrush(RGB(168, 214, 188));
  HGDIOBJ old_arrow_brush = SelectObject(dc, player_brush);
  Polygon(dc, arrow, 3);
  SelectObject(dc, old_arrow_brush);
  DeleteObject(player_brush);
  RestoreDC(dc, saved);

  std::string label = overlay ? "overlay" : "corner";
  label += minimap_side == MinimapSide::Right ? ":right" : ":left";
  label += ":zoom=" + std::to_string(minimap_zoom_percent(
                            state.minimap_zoom_step));
  label += ":opacity=" + std::to_string(minimap_opacity_percent(
                               state.minimap_opacity_step));
  label += ":cells=" + std::to_string(topology_cells);
  rl.push_back({render::Op::Minimap, static_cast<double>(panel.left),
                static_cast<double>(panel.top), static_cast<double>(map.w),
                dots, label});

  if (overlay) {
    SetBkMode(dc, TRANSPARENT);
    HGDIOBJ old_font = SelectObject(dc, skin::font_heading());
    SetTextColor(dc, skin::kGold);
    std::string title = world.has_extraction ? "ROAD TACTICAL CHART"
                                             : "THE CROSSROADS";
    if (world.has_extraction && world.route_id.rfind("route:", 0) == 0) {
      std::string route = world.route_id.substr(6);
      const std::size_t split = route.find(':');
      std::string road = split == std::string::npos ? route
                                                     : route.substr(0, split);
      std::transform(road.begin(), road.end(), road.begin(),
                     [](unsigned char ch) {
                       return static_cast<char>(std::toupper(ch));
                     });
      std::string depth = split == std::string::npos
                              ? std::string{}
                              : route.substr(split + 1);
      std::replace(depth.begin(), depth.end(), ':', '.');
      title = road + " ROAD" +
              (depth.empty() ? std::string{} : " - CHART " + depth);
    }
    if (world.endgame.active && !world.endgame.name.empty())
      title = "CHARTED EXPEDITION - " + world.endgame.name;
    TextOutA(dc, panel.left + 22 * s, panel.top + 16 * s, title.c_str(),
             static_cast<int>(title.size()));
    SelectObject(dc, skin::font_small());
    SetTextColor(dc, skin::kInkDim);
    const std::string settings =
        "TAB close | wheel / [ ] zoom | - / = opacity | SHIFT+M corner side";
    TextOutA(dc, panel.left + 22 * s, panel.bottom - 25 * s,
             settings.c_str(), static_cast<int>(settings.size()));

    const int legend_y = panel.top + 20 * s;
    int legend_x = std::max(panel.left + 260 * s, panel.right - 310 * s);
    const struct {
      COLORREF color;
      const char* text;
    } legend[] = {{RGB(168, 214, 188), "Scion"},
                  {RGB(196, 58, 48), "foe"},
                  {RGB(229, 164, 73), "elite"},
                  {RGB(122, 168, 230), "townsfolk"},
                  {RGB(239, 208, 116), "exit"}};
    for (const auto& entry : legend) {
      fill_ellipse(dc, legend_x, legend_y + 4 * s, 3 * s, 3 * s,
                   entry.color);
      SetTextColor(dc, skin::kInkDim);
      TextOutA(dc, legend_x + 7 * s, legend_y - 3 * s, entry.text,
               static_cast<int>(std::strlen(entry.text)));
      SIZE extent{};
      GetTextExtentPoint32A(dc, entry.text,
                            static_cast<int>(std::strlen(entry.text)), &extent);
      legend_x += extent.cx + 19 * s;
    }
    SelectObject(dc, old_font);
    rl.push_back({render::Op::Hud, static_cast<double>(panel.left),
                  static_cast<double>(panel.bottom - 25 * s), 0.0, 0,
                  "map-control:tab-wheel-brackets-opacity-side"});
  }
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

// Deterministic fallback for the naming ceremony: Enter on an empty field
// derives a usable House name from the admitted account identity.
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

std::string house_prefixed_name(const std::string& name) {
  if (name.empty()) return "House Verdigris";
  if (name == "House" || name.rfind("House ", 0) == 0) return name;
  return "House " + name;
}

std::string active_house_display_name(const ClientState& state) {
  if (state.session) {
    const auto& chronicle = state.session->model().chronicle;
    if (const auto* house = verdigris::client::find_chronicle_house(
            chronicle, chronicle.active_house_id))
      return house->name;
    if (!chronicle.houses.empty()) return chronicle.houses.front().name;
  }
  return house_display_name(state);
}

std::string next_scion_name(const ClientState& state) {
  static const char* kOrdinals[] = {"Firstborn", "Secondborn", "Thirdborn",
                                    "Fourthborn", "Fifthborn"};
  std::size_t total = 0;
  if (state.session) {
    for (const auto& house : state.session->model().chronicle.houses)
      total += house.scions.size() + house.crypt.size();
  }
  std::string house_name = active_house_display_name(state);
  if (house_name.rfind("House of ", 0) == 0)
    house_name = house_name.substr(9);
  else if (house_name.rfind("House ", 0) == 0)
    house_name = house_name.substr(6);
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

std::string normalize_chronicle_name(const std::string& raw) {
  std::string out;
  out.reserve(raw.size());
  bool pending_space = false;
  for (unsigned char ch : raw) {
    if (std::isspace(ch)) {
      pending_space = !out.empty();
      continue;
    }
    if (!(std::isalnum(ch) || ch == '-' || ch == '\'')) continue;
    if (pending_space && !out.empty()) out.push_back(' ');
    pending_space = false;
    out.push_back(static_cast<char>(ch));
  }
  return out;
}

void begin_chronicle_naming(ClientState& state, ChronicleNamingMode mode) {
  state.chronicles_naming = mode;
  state.chronicles_name_input.clear();
  state.chronicles_name_error.clear();
  state.chronicles_ignore_next_char = false;
}

void cancel_chronicle_naming(ClientState& state) {
  state.chronicles_naming = ChronicleNamingMode::None;
  state.chronicles_name_input.clear();
  state.chronicles_name_error.clear();
  state.chronicles_ignore_next_char = false;
}

void confirm_chronicle_naming(ClientState& state) {
  if (!state.session || state.chronicles_naming == ChronicleNamingMode::None)
    return;
  std::string name = normalize_chronicle_name(state.chronicles_name_input);
  if (state.startup_creation && name.empty()) {
    state.chronicles_name_error = "Enter a name to continue.";
    return;
  }
  if (name.empty()) {
    name = state.chronicles_naming == ChronicleNamingMode::House
               ? house_display_name(state)
               : next_scion_name(state);
  }
  if (name.size() < 2) {
    state.chronicles_name_error = "Use at least two letters.";
    return;
  }
  if (name.size() > 28) {
    state.chronicles_name_error = "Keep the name to 28 characters.";
    return;
  }

  using verdigris::client::ClientCommand;
  const ChronicleNamingMode mode = state.chronicles_naming;
  if (state.startup_creation) {
    state.creation_requested_name = name;
    state.creation_previous_receipt = state.session->model().chronicle.created_scion_id;
    state.creation_house = state.session->model().chronicle.active_house_id;
    const auto& roster = state.session->model().chronicle;
    if (!verdigris::client::find_chronicle_house(roster,state.creation_house) && !roster.houses.empty())
      state.creation_house = roster.houses.front().id; // same fallback as the session command
    state.creation_submitted = std::chrono::steady_clock::now();
    state.creation_wait = mode == ChronicleNamingMode::House
        ? ClientState::CreationWait::House : ClientState::CreationWait::Scion;
  }
  cancel_chronicle_naming(state);
  if (mode == ChronicleNamingMode::House) {
    state.session->submit(ClientCommand::found_house(name));
    show_hint(state, "Your House enters the chronicles");
  } else {
    state.session->submit(ClientCommand::create_scion(name, state.chronicles_oath));
    show_hint(state, "A new Scion joins the lineage");
  }
}

void handle_chronicle_character(ClientState& state, unsigned int codepoint) {
  if (state.chronicles_naming == ChronicleNamingMode::None) return;
  if (state.chronicles_ignore_next_char) {
    state.chronicles_ignore_next_char = false;
    return;
  }
  if (codepoint == '\r') {
    confirm_chronicle_naming(state);
    return;
  }
  if (codepoint == '\b') {
    if (!state.chronicles_name_input.empty())
      state.chronicles_name_input.pop_back();
    state.chronicles_name_error.clear();
    return;
  }
  if (codepoint < 32 || codepoint > 126) return;
  const unsigned char ch = static_cast<unsigned char>(codepoint);
  if (!(std::isalnum(ch) || ch == ' ' || ch == '-' || ch == '\'')) return;
  if (state.chronicles_name_input.size() >= 28) {
    state.chronicles_name_error = "Keep the name to 28 characters.";
    return;
  }
  state.chronicles_name_input.push_back(static_cast<char>(ch));
  state.chronicles_name_error.clear();
}

// Shared by physical typing and Unicode clipboard insertion. Native custom
// fields do not receive automatic EDIT-control paste/selection behavior.
void append_chronicle_text(ClientState& state, const wchar_t* text, size_t length) {
  state.chronicles_ignore_next_char = false;
  for (size_t i = 0; i < std::min(length, size_t{4096}); ++i) {
    const unsigned int ch = static_cast<unsigned int>(text[i]);
    // Pasting control characters must never submit/cancel the dialog.
    if (ch >= 32 && ch <= 126) handle_chronicle_character(state, ch);
  }
}

void paste_chronicle_name(HWND window, ClientState& state) {
  if (!OpenClipboard(window)) return;
  const HANDLE data = GetClipboardData(CF_UNICODETEXT);
  if (data) {
    const size_t limit = std::min(GlobalSize(data) / sizeof(wchar_t), size_t{4096});
    const auto* text = static_cast<const wchar_t*>(GlobalLock(data));
    if (text) {
      size_t length = 0;
      while (length < limit && text[length]) ++length;
      append_chronicle_text(state, text, length);
      GlobalUnlock(data);
    }
  }
  CloseClipboard();
}

void submit_chronicle_action(ClientState& state, const ChronicleAction& action) {
  using verdigris::client::ClientCommand;
  if (action.command == "found-house") {
    begin_chronicle_naming(state, ChronicleNamingMode::House);
  } else if (action.command == "create-scion") {
    begin_chronicle_naming(state, ChronicleNamingMode::Scion);
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
  if (state.chronicles_naming != ChronicleNamingMode::None) return;
  state.chronicles_menu = chronicle_actions(state);
  if (state.chronicles_menu.empty()) return;
  state.chronicles_selected = std::min(
      state.chronicles_selected, state.chronicles_menu.size() - 1);
  if (wparam == VK_UP) {
    if (state.chronicles_selected > 0) --state.chronicles_selected;
    return;
  }
  if (wparam == VK_DOWN) {
    if (state.chronicles_selected + 1 < state.chronicles_menu.size())
      ++state.chronicles_selected;
    return;
  }
  for (std::size_t index = 0; index < state.chronicles_menu.size(); ++index) {
    const auto& action = state.chronicles_menu[index];
    if (action.key.size() == 1 && wparam == static_cast<WPARAM>(action.key[0])) {
      state.chronicles_selected = index;
      submit_chronicle_action(state, action);
      if (state.chronicles_naming != ChronicleNamingMode::None)
        state.chronicles_ignore_next_char = true;
      return;
    }
  }
  if (wparam == VK_RETURN) {
    submit_chronicle_action(
        state, state.chronicles_menu[state.chronicles_selected]);
  }
}

bool activate_chronicle_at(ClientState& state, int x, int y) {
  if (state.screen != Screen::Chronicles ||
      state.chronicles_naming != ChronicleNamingMode::None)
    return false;
  state.chronicles_menu = chronicle_actions(state);
  for (const auto& hit : state.chronicles_action_hits) {
    if (x < hit.rect.left || x >= hit.rect.right || y < hit.rect.top ||
        y >= hit.rect.bottom || hit.index >= state.chronicles_menu.size())
      continue;
    state.chronicles_selected = hit.index;
    submit_chronicle_action(state, state.chronicles_menu[hit.index]);
    return true;
  }
  return false;
}

const verdigris::client::ClientScionEntry* title_continue_scion(const ClientState& state) {
  if (!state.session) return nullptr;
  const auto& chronicle = state.session->model().chronicle;
  const auto* house = verdigris::client::find_chronicle_house(chronicle, chronicle.active_house_id);
  if (!house && !chronicle.houses.empty()) house = &chronicle.houses.front();
  if (!house || house->scions.empty()) return nullptr;
  for (const auto& scion : house->scions)
    if (scion.id == chronicle.active_scion_id) return &scion;
  return &house->scions.front();
}

bool title_account_ready(const ClientState& state) {
  if (!state.session || !state.session->model().chronicle.present) return false;
  const auto connection = state.session->connection_state();
  return connection == verdigris::client::ConnectionState::Connected ||
         connection == verdigris::client::ConnectionState::Ready;
}

bool activate_title_action(ClientState& state, int index) {
  if (!title_account_ready(state) || index < 0 || index > 2) return false;
  state.startup_creation = false;
  state.title_open = false;
  state.title_orbit.dragging = false;
  state.screen = Screen::Chronicles;
  if (index == 2) return true; // full lineage is a deliberate secondary screen
  const auto* scion = title_continue_scion(state);
  if (index == 0 && scion) {
    // Preserve the selected Scion's oath and identity; a title-screen resume
    // is not a new admission and must not silently change Hardcore mode.
    state.session->submit(verdigris::client::ClientCommand::select_scion(scion->id, scion->mortal));
  } else {
    state.startup_creation = true;
    state.creation_wait = ClientState::CreationWait::None;
    state.creation_focus = 0;
    state.chronicles_oath = false;
    begin_chronicle_naming(state, state.session->model().chronicle.houses.empty()
        ? ChronicleNamingMode::House : ChronicleNamingMode::Scion);
  }
  return true;
}

void paint_title_scene(ClientState& state, HDC dc, const RECT& bounds, render::List& rl,
                       bool menu = true) {
  std::filesystem::path root = "native/client/assets/wizard/splash";
  for (const auto& candidate : {root, std::filesystem::path(executable_directory()) / "../../client/assets/wizard/splash",
                                std::filesystem::path(executable_directory()) / "../client/assets/wizard/splash",
                                std::filesystem::path(executable_directory()) / "assets/wizard/splash"}) {
    if (std::filesystem::exists(candidate / "world/celestial_world_runtime_tapered.glb")) { root = candidate; break; }
  }
  const float seconds = std::chrono::duration<float>(std::chrono::steady_clock::now() - state.title_started).count();
  const bool rendered = state.title_scene.draw(dc, bounds, root, state.title_orbit, seconds);
  if (!rendered) skin::panel(dc, bounds, skin::kVerdigris, 255, 0);
  if (!menu) {
    rl.push_back({render::Op::Chronicles,0,0,0,static_cast<int>(state.title_scene.triangles()),
                  rendered ? "title:3d-island" : "title:render-error"});
    return;
  }
  const int s = std::max(1, std::min(static_cast<int>(bounds.bottom) / 650, static_cast<int>(bounds.right) / 900));
  skin::set_ui_scale(s);
  SetBkMode(dc, TRANSPARENT);
  const int left = std::max(28*s, static_cast<int>(bounds.right)*7/100);
  const int menu_width = std::min(340*s, static_cast<int>(bounds.right)-left*2);
  const int top = std::max(28*s, static_cast<int>(bounds.bottom)*29/100);
  auto text = [&](HFONT font, COLORREF color, RECT rect, const std::string& value, UINT flags=DT_SINGLELINE) {
    const auto old = SelectObject(dc,font); SetTextColor(dc,color);
    DrawTextA(dc,value.c_str(),static_cast<int>(value.size()),&rect,flags|DT_NOPREFIX|DT_END_ELLIPSIS);
    SelectObject(dc,old);
  };
  text(skin::font_title(),skin::kGold,{left,top,left+menu_width,top+44*s},"V E R D I G R I S");
  text(skin::font_small(),skin::kInkDim,{left,top+54*s,left+menu_width,top+80*s},"A WORLD OUTLIVES ITS HEROES");
  const bool ready=title_account_ready(state);
  const auto* scion=title_continue_scion(state);
  const std::string primary=scion ? "Continue as " + scion->name : "Begin your chronicle";
  const std::string labels[]={primary,"Create a Scion","House & chronicle"};
  state.title_action_hits.assign(3,RECT{});
  if (!scion && state.title_selected==1) state.title_selected=0;
  int row=0;
  for (int i=0;i<3;++i) {
    if (i==1 && !scion) continue; // Begin already opens the required creation step.
    RECT card{left,top+(112+row*62)*s,left+menu_width,top+(162+row*62)*s};
    ++row;
    if (i==state.title_selected) {
      if (!draw_framekit_nine(state.billboards,dc,state.billboards.fk_button,card))
        skin::panel(dc,card,skin::kGold,185,6);
    }
    RECT label{card.left+18*s,card.top,card.right-12*s,card.bottom};
    text(skin::font_heading(),ready ? (i==state.title_selected?skin::kGold:skin::kInk) : skin::kInkDim,label,labels[i],DT_SINGLELINE|DT_VCENTER);
    state.title_action_hits[i]=card;
    rl.push_back({render::Op::Chronicles,static_cast<double>(card.left),static_cast<double>(card.top),0,i,"title:action"});
  }
  std::string status=ready ? "Drag to orbit  /  Scroll to zoom  /  Home to reset" : "Connecting to your saved chronicle...";
  if (state.session && state.session->connection_state()==verdigris::client::ConnectionState::Disconnected)
    status="Connection lost. Restart the launcher to reconnect.";
  text(skin::font_small(),skin::kInkDim,{left,bounds.bottom-48*s,bounds.right-24*s,bounds.bottom-20*s},status);
  if (!rendered) {
    text(skin::font_small(),skin::kEmber,{left,28*s,bounds.right-28*s,100*s},"Title scene unavailable: " + state.title_scene.error(),DT_WORDBREAK);
  }
  rl.push_back({render::Op::Chronicles,0,0,0,static_cast<int>(state.title_scene.triangles()), rendered ? "title:3d-island" : "title:render-error"});
}

void cancel_startup_creation(ClientState& state) {
  cancel_chronicle_naming(state);
  state.startup_creation = false;
  state.creation_wait = ClientState::CreationWait::None;
  state.title_open = true;
}

void advance_startup_creation(ClientState& state) {
  if (!state.startup_creation || !title_account_ready(state)) return;
  using Wait = ClientState::CreationWait;
  const auto& model = state.session->model();
  if (state.creation_wait == Wait::House) {
    const auto* house = verdigris::client::find_chronicle_house(model.chronicle,model.chronicle.active_house_id);
    if (!house || house->id == state.creation_house || house->name != state.creation_requested_name) return;
    state.creation_wait = Wait::None;
    state.creation_focus = 0;
    begin_chronicle_naming(state,ChronicleNamingMode::Scion);
  } else if (state.creation_wait == Wait::Scion) {
    const auto& id = model.chronicle.created_scion_id;
    if (id.empty() || id == state.creation_previous_receipt) return;
    const auto* house = verdigris::client::find_chronicle_house(model.chronicle,state.creation_house);
    if (!house) return;
    for (const auto& scion : house->scions) {
      if (scion.id != id || scion.name != state.creation_requested_name || scion.mortal != state.chronicles_oath) continue;
      state.creation_wait = Wait::Admission; // set before submitting; no repeat admissions
      state.creation_submitted = std::chrono::steady_clock::now();
      state.session->submit(verdigris::client::ClientCommand::select_scion(id,scion.mortal));
      break;
    }
  } else if (state.creation_wait == Wait::Admission && !model.chronicles_pending && model.player.alive) {
    state.startup_creation = false;
    state.creation_wait = Wait::None;
  }
}

void activate_creation_control(ClientState& state, int control) {
  if (!state.startup_creation) return;
  if (control == 3) { cancel_startup_creation(state); return; }
  if (!title_account_ready(state) || state.creation_wait != ClientState::CreationWait::None) return;
  if (control == 1 && state.chronicles_naming == ChronicleNamingMode::Scion)
    state.chronicles_oath = !state.chronicles_oath;
  if (control == 2 || control == 0) confirm_chronicle_naming(state);
}

bool handle_creation_key(ClientState& state, WPARAM key) {
  if (!state.startup_creation) return false;
  if (key == VK_ESCAPE) { cancel_startup_creation(state); return true; }
  if (key == VK_TAB) {
    const int direction = (GetKeyState(VK_SHIFT) & 0x8000) ? -1 : 1;
    state.creation_focus = (state.creation_focus + direction + 4) % 4;
    if (state.creation_focus == 1 && state.chronicles_naming != ChronicleNamingMode::Scion)
      state.creation_focus = (state.creation_focus + direction + 4) % 4;
    return true;
  }
  if (key == VK_RETURN || (key == VK_SPACE && state.creation_focus != 0)) {
    activate_creation_control(state,state.creation_focus);
    state.chronicles_ignore_next_char = true;
    return true;
  }
  // Normal editing shortcuts go through the shared naming handler only.
  return state.creation_focus != 0 || state.creation_wait != ClientState::CreationWait::None;
}

void paint_startup_creation(ClientState& state, HDC dc, const RECT& bounds, render::List& rl) {
  paint_title_scene(state,dc,bounds,rl,false);
  const int s = std::max(1,std::min(static_cast<int>(bounds.right)/900,static_cast<int>(bounds.bottom)/650));
  skin::set_ui_scale(s); SetBkMode(dc,TRANSPARENT);
  const int width = std::min(520*s,static_cast<int>(bounds.right)-32);
  const int height = std::min(350*s,static_cast<int>(bounds.bottom)-32);
  RECT card{(bounds.right-width)/2,(bounds.bottom-height)/2,(bounds.right+width)/2,(bounds.bottom+height)/2};
  if (!draw_framekit_nine(state.billboards,dc,state.billboards.fk_panel,card)) skin::panel(dc,card,skin::kGold,250,10);
  auto text = [&](HFONT font,COLORREF color,RECT rect,const std::string& value,UINT flags=DT_SINGLELINE) {
    const auto old=SelectObject(dc,font); SetTextColor(dc,color);
    DrawTextA(dc,value.c_str(),static_cast<int>(value.size()),&rect,flags|DT_NOPREFIX|DT_END_ELLIPSIS); SelectObject(dc,old);
  };
  const bool house = state.chronicles_naming == ChronicleNamingMode::House || state.creation_wait == ClientState::CreationWait::House;
  const bool waiting = state.creation_wait != ClientState::CreationWait::None;
  const int inset=24*s;
  text(skin::font_heading(),skin::kGold,{card.left+inset,card.top+22*s,card.right-inset,card.top+50*s},house?"Found your House":"Create your Scion");
  text(skin::font_body(),skin::kInkDim,{card.left+inset,card.top+56*s,card.right-inset,card.top+88*s},house?"Your lineage keeps its history between characters.":"A new character in "+active_house_display_name(state));
  state.creation_input={card.left+inset,card.top+96*s,card.right-inset,card.top+145*s};
  if (!draw_framekit_sprite(state.billboards,dc,state.billboards.fk_chronicle_input,state.creation_input)) skin::slot(dc,state.creation_input,skin::kGold,true);
  const int input_inset=(state.creation_input.right-state.creation_input.left)*14/162+4*s;
  RECT content{state.creation_input.left+input_inset,state.creation_input.top+8*s,state.creation_input.right-input_inset,state.creation_input.bottom-8*s};
  std::string value=waiting?state.creation_requested_name:state.chronicles_name_input;
  const bool placeholder=value.empty();
  if (placeholder) value=house?"House name":"Character name";
  else if (!waiting && state.creation_focus==0) value+="|";
  const auto old=SelectObject(dc,skin::font_heading());
  SIZE extent{};
  while (value.size()>1) { GetTextExtentPoint32A(dc,value.c_str(),static_cast<int>(value.size()),&extent); if(extent.cx<=content.right-content.left) break; value.erase(0,1); }
  SelectObject(dc,old);
  text(skin::font_heading(),placeholder?skin::kInkDim:skin::kInk,content,value,DT_SINGLELINE|DT_VCENTER);
  state.creation_oath={card.left+inset,card.top+165*s,card.right-inset,card.top+201*s};
  if (!house) {
    RECT checkbox{state.creation_oath.left,state.creation_oath.top+3*s,state.creation_oath.left+28*s,state.creation_oath.top+31*s};
    skin::slot(dc,checkbox,state.creation_focus==1?skin::kGold:skin::kVerdigris,state.chronicles_oath);
    text(skin::font_heading(),skin::kGold,checkbox,state.chronicles_oath?"X":"",DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    text(skin::font_body_bold(),skin::kInk,{checkbox.right+12*s,checkbox.top,card.right-inset,checkbox.bottom},"Hardcore (mortal oath)",DT_SINGLELINE|DT_VCENTER);
    text(skin::font_small(),state.chronicles_oath?skin::kEmber:skin::kInkDim,
         {card.left+inset,card.top+207*s,card.right-inset,card.top+247*s},
         state.chronicles_oath?"Death is permanent. Your House and its history remain.":"Off: this Scion can return after defeat.",DT_WORDBREAK);
  }
  std::string status=state.chronicles_name_error;
  if (waiting) {
    status=state.creation_wait==ClientState::CreationWait::Admission?"Entering the world...":"Saving your name...";
    if (std::chrono::steady_clock::now()-state.creation_submitted>std::chrono::seconds(10))
      status="Still waiting. Back returns to your saved roster; do not resubmit.";
  } else if (!title_account_ready(state)) status="Connection lost. Your typed name is kept here.";
  text(skin::font_small(),waiting?skin::kInkDim:skin::kEmber,{card.left+inset,card.bottom-100*s,card.right-inset,card.bottom-65*s},status,DT_WORDBREAK);
  state.creation_back={card.left+inset,card.bottom-61*s,card.left+142*s,card.bottom-21*s};
  state.creation_confirm={card.right-235*s,card.bottom-61*s,card.right-inset,card.bottom-21*s};
  auto button=[&](RECT rect,int focus,const char* label) {
    if (!draw_framekit_nine(state.billboards,dc,state.billboards.fk_button,rect)) skin::panel(dc,rect,skin::kGold,230,6);
    text(skin::font_body_bold(),state.creation_focus==focus?skin::kGold:skin::kInk,rect,label,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
  };
  button(state.creation_back,3,"Back");
  button(state.creation_confirm,2,waiting?"Please wait":house?"Continue":"Create & play");
  rl.push_back({render::Op::Chronicles,static_cast<double>(card.left),static_cast<double>(card.top),0,0,house?"creation:house":"creation:scion"});
  rl.push_back({render::Op::Chronicles,0,0,0,0,state.chronicles_oath?"creation:hardcore:on":"creation:hardcore:off"});
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
  state.chronicles_action_hits.clear();
  RECT panel{0, 0, bounds.right, bounds.bottom};
  HBRUSH backdrop = CreateSolidBrush(RGB(10, 14, 12));
  FillRect(dc, &panel, backdrop);
  DeleteObject(backdrop);
  SetBkMode(dc, TRANSPARENT);
  const int door_scale = hud_scale(static_cast<int>(bounds.bottom));
  skin::set_ui_scale(door_scale);

  // A faint ruled vellum field gives the full-screen owner surface depth
  // while keeping every actionable element inside WIZARD Framekit chrome.
  for (int y = 12 * door_scale; y < bounds.bottom; y += 34 * door_scale)
    draw_line(dc, 0, y, bounds.right, y, RGB(15, 24, 20), 1);
  for (int x = -bounds.bottom; x < bounds.right; x += 180 * door_scale)
    draw_line(dc, x, bounds.bottom, x + bounds.bottom, 0, RGB(13, 21, 18), 1);

  const int margin = 18 * door_scale;
  const int available_w =
      (std::max)(1, static_cast<int>(bounds.right) - margin * 2);
  const int canvas_w = std::min(available_w, 1360 * door_scale);
  const int canvas_left = (bounds.right - canvas_w) / 2;
  const int header_top = 18 * door_scale;
  const int content_top = 82 * door_scale;
  const int content_bottom = bounds.bottom - 42 * door_scale;
  const int gap = 14 * door_scale;
  const int left_w = std::clamp(canvas_w * 42 / 100,
                                310 * door_scale, 520 * door_scale);
  RECT house_pane{canvas_left, content_top, canvas_left + left_w,
                  content_bottom};
  RECT action_pane{house_pane.right + gap, content_top,
                   canvas_left + canvas_w, content_bottom};

  auto put_text = [&](HFONT font, COLORREF color, int x, int y,
                      const std::string& text) {
    HGDIOBJ old_font = SelectObject(dc, font);
    SetTextColor(dc, color);
    TextOutA(dc, x, y, text.c_str(), static_cast<int>(text.size()));
    SelectObject(dc, old_font);
  };
  auto put_wrapped = [&](HFONT font, COLORREF color, RECT rect,
                         const std::string& text, UINT format) {
    HGDIOBJ old_font = SelectObject(dc, font);
    SetTextColor(dc, color);
    DrawTextA(dc, text.c_str(), static_cast<int>(text.size()), &rect,
              format | DT_NOPREFIX);
    SelectObject(dc, old_font);
  };
  auto frame = [&](const RECT& rect, COLORREF accent) {
    if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_panel,
                            rect))
      skin::panel(dc, rect, accent, 248, 10.0f);
  };
  auto outlined = [&](const RECT& rect, COLORREF color, int width) {
    HPEN pen = CreatePen(PS_SOLID, width, color);
    HGDIOBJ old_pen = SelectObject(dc, pen);
    HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(pen);
  };
  auto chronicle_raster = [&](const SpriteBitmap& sprite, const RECT& rect,
                              const char* label, BYTE alpha = 255) {
    if (!draw_framekit_sprite(state.billboards, dc, sprite, rect, alpha))
      return false;
    rl.push_back({render::Op::Chronicles, static_cast<double>(rect.left),
                  static_cast<double>(rect.top), 0.0, 0,
                  std::string("framekit-raster:") + label});
    return true;
  };

  put_text(skin::font_title(), skin::kVerdigris, canvas_left, header_top,
           "V E R D I G R I S");
  put_text(skin::font_small(), skin::kGold, canvas_left,
           header_top + 41 * door_scale, "CHRONICLES  /  HOUSE & SCION");
  state.hud_rect_trace.push_back(
      {"chronicles-title", {canvas_left, header_top, 240 * door_scale,
                             34 * door_scale}});
  state.hud_rect_trace.push_back(
      {"chronicles-subtitle",
       {canvas_left, header_top + 41 * door_scale, 240 * door_scale,
        14 * door_scale}});
  const int laurel_w = 62 * door_scale;
  const int laurel_h = 49 * door_scale;
  RECT laurel{canvas_left + canvas_w / 2 - laurel_w - 8 * door_scale,
              header_top - 3 * door_scale,
              canvas_left + canvas_w / 2 - 8 * door_scale,
              header_top - 3 * door_scale + laurel_h};
  (void)chronicle_raster(state.billboards.fk_chronicle_laurel, laurel,
                         "chronicle-laurel", 230);
  std::string account = "Opening the chronicles...";
  if (!state.session || state.session->connection_state() ==
                            verdigris::client::ConnectionState::Disconnected)
    account = "The chronicles lie closed - connection lost.";
  else if (model.chronicle.present)
    account = "ACCOUNT  " +
              (model.chronicle.account_name.empty() ? std::string("GUEST")
                                                     : model.chronicle.account_name);
  RECT account_rect{canvas_left + canvas_w / 2, header_top + 10 * door_scale,
                    canvas_left + canvas_w - 184 * door_scale,
                    header_top + 36 * door_scale};
  put_wrapped(skin::font_small(), skin::kInkDim, account_rect, account,
              DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
  rl.push_back({render::Op::Chronicles, static_cast<double>(canvas_left),
                static_cast<double>(header_top), 0.0, 0, "title"});
  rl.push_back({render::Op::Chronicles, static_cast<double>(account_rect.left),
                static_cast<double>(account_rect.top), 0.0, 0, "account"});

  frame(house_pane, skin::kVerdigris);
  frame(action_pane, skin::kGold);
  state.hud_rect_trace.push_back(
      {"chronicles-house-pane", {house_pane.left, house_pane.top,
                                  house_pane.right - house_pane.left,
                                  house_pane.bottom - house_pane.top}});
  state.hud_rect_trace.push_back(
      {"chronicles-action-pane", {action_pane.left, action_pane.top,
                                   action_pane.right - action_pane.left,
                                   action_pane.bottom - action_pane.top}});

  const int inset = 20 * door_scale;
  put_text(skin::font_small(), skin::kInkDim, house_pane.left + inset,
           house_pane.top + 16 * door_scale, "HOUSE LEDGER");
  const verdigris::client::ClientHouseEntry* ledger_house = nullptr;
  if (!model.chronicle.houses.empty()) {
    ledger_house = verdigris::client::find_chronicle_house(
        model.chronicle, model.chronicle.active_house_id);
    if (!ledger_house) ledger_house = &model.chronicle.houses.front();
  }

  if (!ledger_house) {
    const int seal_x = (house_pane.left + house_pane.right) / 2;
    const int seal_y = house_pane.top + 150 * door_scale;
    RECT lock{seal_x - 54 * door_scale, seal_y - 56 * door_scale,
              seal_x + 54 * door_scale, seal_y + 56 * door_scale};
    if (!chronicle_raster(state.billboards.fk_chronicle_keyhole, lock,
                          "chronicle-keyhole")) {
      fill_ellipse(dc, seal_x, seal_y, 54 * door_scale, 54 * door_scale,
                   RGB(18, 35, 29));
      ring_ellipse(dc, seal_x, seal_y, 54 * door_scale, 54 * door_scale,
                   skin::kVerdigris, 2 * door_scale);
      ring_ellipse(dc, seal_x, seal_y, 41 * door_scale, 41 * door_scale,
                   skin::kGold, 1 * door_scale);
      put_text(skin::font_title(), skin::kGold, seal_x - 14 * door_scale,
               seal_y - 17 * door_scale, "V");
    }
    RECT empty_copy{house_pane.left + 34 * door_scale,
                    seal_y + 76 * door_scale,
                    house_pane.right - 34 * door_scale,
                    seal_y + 150 * door_scale};
    put_wrapped(skin::font_heading(), skin::kInk, empty_copy,
                "No House stands in these pages. Name the lineage that will own every road, relic, and remembrance.",
                DT_CENTER | DT_WORDBREAK);
    rl.push_back({render::Op::Chronicles, static_cast<double>(seal_x),
                  static_cast<double>(seal_y), 0.0, 0, "prompt"});
  } else {
    const int seal_x = house_pane.left + 54 * door_scale;
    const int seal_y = house_pane.top + 82 * door_scale;
    RECT knocker{seal_x - 31 * door_scale, seal_y - 33 * door_scale,
                 seal_x + 31 * door_scale, seal_y + 33 * door_scale};
    if (!chronicle_raster(state.billboards.fk_chronicle_knocker, knocker,
                          "chronicle-knocker")) {
      fill_ellipse(dc, seal_x, seal_y, 30 * door_scale, 30 * door_scale,
                   RGB(20, 42, 33));
      ring_ellipse(dc, seal_x, seal_y, 30 * door_scale, 30 * door_scale,
                   skin::kGold, 2 * door_scale);
      put_text(skin::font_title(), skin::kGold, seal_x - 9 * door_scale,
               seal_y - 17 * door_scale,
               ledger_house->name.empty() ? "V"
                                          : ledger_house->name.substr(0, 1));
    }
    put_wrapped(skin::font_heading(), skin::kGold,
                {house_pane.left + 96 * door_scale, house_pane.top + 55 * door_scale,
                 house_pane.right - inset, house_pane.top + 108 * door_scale},
                house_prefixed_name(ledger_house->name), DT_WORDBREAK | DT_END_ELLIPSIS);
    put_text(skin::font_small(), skin::kInkDim,
             house_pane.left + 96 * door_scale,
             house_pane.top + 91 * door_scale,
             ledger_house->campaign_complete ? "THE WAYFINDER ROADS ARE OPEN"
                                             : "A LINEAGE IN THE MAKING");
    rl.push_back({render::Op::Chronicles, static_cast<double>(seal_x),
                  static_cast<double>(seal_y), 0.0, 0,
                  "house " + ledger_house->name});

    const int metrics_y = house_pane.top + 128 * door_scale;
    const int metric_gap = 6 * door_scale;
    const int metric_w =
        (house_pane.right - house_pane.left - inset * 2 - metric_gap * 2) / 3;
    struct Metric { const char* name; int value; };
    const Metric metrics[] = {
        {"LIVING", static_cast<int>(ledger_house->scions.size())},
        {"REMEMBERED", static_cast<int>(ledger_house->crypt.size())},
        {"MAPS", ledger_house->endgame_maps_completed}};
    for (int i = 0; i < 3; ++i) {
      RECT metric{house_pane.left + inset + i * (metric_w + metric_gap),
                  metrics_y,
                  house_pane.left + inset + i * (metric_w + metric_gap) + metric_w,
                  metrics_y + 50 * door_scale};
      skin::slot(dc, metric, i == 2 ? skin::kGold : skin::kVerdigris, false);
      put_text(skin::font_heading(), skin::kInk, metric.left + 10 * door_scale,
               metric.top + 7 * door_scale, std::to_string(metrics[i].value));
      put_text(skin::font_small(), skin::kInkDim, metric.left + 10 * door_scale,
               metric.top + 29 * door_scale, metrics[i].name);
    }

    int list_y = metrics_y + 72 * door_scale;
    put_text(skin::font_small(), skin::kVerdigris, house_pane.left + inset,
             list_y, "LIVING SCIONS");
    list_y += 24 * door_scale;
    const int max_scion_rows = (std::max)(
        1, (static_cast<int>(house_pane.bottom) - list_y - 92 * door_scale) /
               (44 * door_scale));
    const int visible_scions = std::min(
        static_cast<int>(ledger_house->scions.size()), max_scion_rows);
    for (int i = 0; i < visible_scions; ++i) {
      const auto& scion = ledger_house->scions[static_cast<std::size_t>(i)];
      RECT row{house_pane.left + inset, list_y,
               house_pane.right - inset, list_y + 38 * door_scale};
      const bool active = scion.id == model.chronicle.active_scion_id;
      skin::slot(dc, row, active ? skin::kGold : skin::kVerdigris, active);
      put_text(skin::font_heading(), active ? skin::kGold : skin::kInk,
               row.left + 12 * door_scale, row.top + 7 * door_scale,
               scion.name);
      std::string standing = "LV " + std::to_string(scion.level) +
                             (scion.mortal ? "  MORTAL" : "  LIVING");
      RECT standing_rect{row.left, row.top, row.right - 10 * door_scale,
                         row.bottom};
      put_wrapped(skin::font_small(), scion.mortal ? skin::kEmber : skin::kInkDim,
                  standing_rect, standing, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
      rl.push_back({render::Op::Chronicles, static_cast<double>(row.left),
                    static_cast<double>(row.top), 0.0, i,
                    "scion " + scion.id});
      list_y += 44 * door_scale;
    }
    if (visible_scions < static_cast<int>(ledger_house->scions.size()))
      put_text(skin::font_small(), skin::kInkDim, house_pane.left + inset,
               list_y, "+ " + std::to_string(ledger_house->scions.size() - visible_scions) +
                           " more in the living register");

    if (ledger_house->campaign_complete || ledger_house->endgame_masteries > 0) {
      const int mastery_y = house_pane.bottom - 99 * door_scale;
      put_text(skin::font_small(), skin::kGold, house_pane.left + inset,
               mastery_y,
               "WAYFINDER MASTERY  " +
                   std::to_string(ledger_house->endgame_masteries) + " / 64");
      RECT mastery_bar{house_pane.left + inset, mastery_y + 22 * door_scale,
                       house_pane.right - inset,
                       mastery_y + 30 * door_scale};
      skin::progress_bar(dc, mastery_bar,
                         static_cast<double>(ledger_house->endgame_masteries) /
                             64.0,
                         skin::kGold, 16);
      rl.push_back({render::Op::Chronicles,
                    static_cast<double>(mastery_bar.left),
                    static_cast<double>(mastery_bar.top), 0.0,
                    ledger_house->endgame_masteries,
                    "mastery:" +
                        std::to_string(ledger_house->endgame_masteries) +
                        "/64"});
    }

    if (!model.chronicle.fallen.name.empty()) {
      RECT fallen{house_pane.left + inset,
                  house_pane.bottom - 66 * door_scale,
                  house_pane.right - inset,
                  house_pane.bottom - 20 * door_scale};
      skin::slot(dc, fallen, skin::kEmber, true);
      put_text(skin::font_small(), skin::kEmber, fallen.left + 10 * door_scale,
               fallen.top + 6 * door_scale, "LAST FALL");
      put_text(skin::font_heading(), skin::kInk,
               fallen.left + 10 * door_scale, fallen.top + 22 * door_scale,
               model.chronicle.fallen.name + "  -  level " +
                   std::to_string(model.chronicle.fallen.level));
      rl.push_back({render::Op::Chronicles, static_cast<double>(fallen.left),
                    static_cast<double>(fallen.top), 0.0, 0,
                    "fallen:" + model.chronicle.fallen.scion_id});
      if (!ledger_house->crypt.empty()) {
        const auto& remembered = ledger_house->crypt.back();
        rl.push_back({render::Op::Chronicles,
                      static_cast<double>(fallen.left),
                      static_cast<double>(fallen.bottom), 0.0, 0,
                      "crypt " + remembered.id});
      }
    } else if (!ledger_house->crypt.empty()) {
      const auto& remembered = ledger_house->crypt.back();
      put_text(skin::font_small(), skin::kInkDim, house_pane.left + inset,
               house_pane.bottom - 40 * door_scale,
               "CRYPT  " + remembered.name + "  /  " +
                   (remembered.relic_status.empty() ? std::string("remembered")
                                                    : remembered.relic_status));
      rl.push_back({render::Op::Chronicles,
                    static_cast<double>(house_pane.left + inset),
                    static_cast<double>(house_pane.bottom - 40 * door_scale),
                    0.0, 0, "crypt " + remembered.id});
    }
  }

  put_text(skin::font_small(), skin::kInkDim, action_pane.left + inset,
           action_pane.top + 16 * door_scale,
           ledger_house ? "CHOOSE WHO WALKS" : "BEGIN THE CHRONICLE");
  put_text(skin::font_heading(), skin::kInk, action_pane.left + inset,
           action_pane.top + 38 * door_scale,
           ledger_house ? "Prepare the next expedition"
                        : "Your House outlives every Scion");
  RECT intro{action_pane.left + inset, action_pane.top + 66 * door_scale,
             action_pane.right - inset, action_pane.top + 106 * door_scale};
  put_wrapped(skin::font_small(), skin::kInkDim, intro,
              ledger_house ? "Select a living heir, add a new classless Scion, or bind the next journey to the mortal oath."
                           : "Choose a lineage name. Its stores, roads, relics, and mastery persist when a Scion does not.",
              DT_WORDBREAK);

  state.chronicles_menu = chronicle_actions(state);
  if (!state.chronicles_menu.empty())
    state.chronicles_selected = std::min(
        state.chronicles_selected, state.chronicles_menu.size() - 1);
  else
    state.chronicles_selected = 0;
  const int action_top = action_pane.top + 118 * door_scale;
  const int action_bottom = action_pane.bottom - 46 * door_scale;
  const int action_gap = 7 * door_scale;
  const int total_actions = static_cast<int>(state.chronicles_menu.size());
  const int action_space = (std::max)(1, action_bottom - action_top);
  const int minimum_action_h = 44 * door_scale;
  const int visible_capacity = (std::max)(
      1, (action_space + action_gap) / (minimum_action_h + action_gap));
  const int visible_actions = (std::min)(total_actions, visible_capacity);
  const int selected_action = static_cast<int>(state.chronicles_selected);
  const int first_action = (std::clamp)(
      selected_action - visible_actions / 2, 0,
      (std::max)(0, total_actions - visible_actions));
  const int action_h = std::clamp(
      (action_space - action_gap * ((std::max)(1, visible_actions) - 1)) /
          (std::max)(1, visible_actions),
      minimum_action_h, 66 * door_scale);
  for (int visible_index = 0; visible_index < visible_actions;
       ++visible_index) {
    const std::size_t index =
        static_cast<std::size_t>(first_action + visible_index);
    const auto& action = state.chronicles_menu[index];
    const int top = action_top + visible_index * (action_h + action_gap);
    if (top + action_h > action_bottom) break;
    RECT card{action_pane.left + inset, top, action_pane.right - inset,
              top + action_h};
    const bool selected = index == state.chronicles_selected;
    if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot,
                            card))
      skin::slot(dc, card, action.command == "oath-toggle" ? skin::kEmber
                                                            : skin::kGold,
                 selected);
    if (selected)
      outlined(card, action.command == "oath-toggle" ? skin::kEmber
                                                       : skin::kGold,
               2 * door_scale);

    RECT key{card.left + 10 * door_scale, card.top + 9 * door_scale,
             card.left + 42 * door_scale, card.bottom - 9 * door_scale};
    skin::slot(dc, key, selected ? skin::kGold : skin::kVerdigris, selected);
    RECT key_text = key;
    put_wrapped(skin::font_heading(), selected ? skin::kGold : skin::kInk,
                key_text, action.key, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

    std::string description;
    if (action.command == "found-house")
      description = "Name a permanent lineage and open its first page.";
    else if (action.command == "create-scion")
      description = "Add a level-one, classless heir to the living register.";
    else if (action.command == "oath-toggle")
      description = state.chronicles_oath
                        ? "ARMED - the next admitted Scion cannot return from death."
                        : "Soft journey - death may be recovered. Activate for final stakes.";
    else {
      const auto* scion = verdigris::client::find_chronicle_scion(
          model.chronicle, action.arg);
      description = scion ? "Level " + std::to_string(scion->level) +
                                (scion->mortal ? " mortal Scion" : " living Scion") +
                                " - road purse prepared."
                          : "Admit this Scion to the Crossroads.";
    }
    std::string headline = action.label;
    if (action.command == "create-scion") headline = "Name a new Scion";
    if (action.command == "oath-toggle")
      headline = state.chronicles_oath ? "Mortal oath: ARMED"
                                       : "Mortal oath: not taken";
    RECT headline_rect{card.left + 54 * door_scale,
                       card.top + 5 * door_scale,
                       card.right - 12 * door_scale,
                       card.top + 30 * door_scale};
    put_wrapped(skin::font_heading(), selected ? skin::kGold : skin::kInk,
                headline_rect, headline,
                DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    if (action_h >= 52 * door_scale) {
      RECT detail{card.left + 54 * door_scale, card.top + 31 * door_scale,
                  card.right - 12 * door_scale, card.bottom - 5 * door_scale};
      put_wrapped(skin::font_small(), skin::kInkDim, detail, description,
                  DT_SINGLELINE | DT_END_ELLIPSIS);
    }
    state.chronicles_action_hits.push_back({card, index});
    rl.push_back({render::Op::Chronicles, static_cast<double>(card.left),
                  static_cast<double>(card.top), 0.0,
                  static_cast<int>(index),
                  "action:" + action.command +
                      (action.arg.empty() ? "" : ":" + action.arg)});
  }
  rl.push_back({render::Op::Chronicles,
                static_cast<double>(action_pane.left + inset),
                static_cast<double>(action_pane.bottom - 32 * door_scale),
                0.0, 0, state.chronicles_oath ? "oath:on" : "oath:off"});
  put_text(skin::font_small(), skin::kInkDim, action_pane.left + inset,
           action_pane.bottom - 31 * door_scale,
           "UP / DOWN select   ENTER confirm   CLICK choose   ESC quit");
  if (visible_actions < total_actions) {
    RECT page_rect{action_pane.left + inset, action_pane.bottom - 35 * door_scale,
                   action_pane.right - inset,
                   action_pane.bottom - 15 * door_scale};
    put_wrapped(skin::font_small(), skin::kGold, page_rect,
                std::to_string(first_action + 1) + "-" +
                    std::to_string(first_action + visible_actions) + " / " +
                    std::to_string(total_actions),
                DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
  }

  if (state.chronicles_naming != ChronicleNamingMode::None) {
    const int modal_w = 520 * door_scale;
    const int modal_h = 190 * door_scale;
    RECT modal{(bounds.right - modal_w) / 2,
               (bounds.bottom - modal_h) / 2,
               (bounds.right + modal_w) / 2,
               (bounds.bottom + modal_h) / 2};
    if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_panel,
                            modal))
      skin::panel(dc, modal, skin::kGold, 252, 12.0f);
    state.hud_rect_trace.push_back(
        {"chronicles-name-modal",
         {modal.left, modal.top, modal.right - modal.left,
          modal.bottom - modal.top}});
    const bool house =
        state.chronicles_naming == ChronicleNamingMode::House;
    const std::string title = house ? "FOUND A HOUSE" : "NAME A SCION";
    const std::string fallback =
        house ? house_display_name(state) : next_scion_name(state);
    HGDIOBJ old_font = SelectObject(dc, skin::font_title());
    SetTextColor(dc, skin::kGold);
    TextOutA(dc, modal.left + 24 * door_scale,
             modal.top + 22 * door_scale, title.c_str(),
             static_cast<int>(title.size()));
    SelectObject(dc, old_font);

    RECT input{modal.left + 24 * door_scale, modal.top + 72 * door_scale,
               modal.right - 24 * door_scale, modal.top + 112 * door_scale};
    if (!chronicle_raster(state.billboards.fk_chronicle_input, input,
                          "chronicle-input"))
      skin::slot(dc, input, skin::kVerdigris, true);
    state.hud_rect_trace.push_back(
        {"chronicles-name-input",
         {input.left, input.top, input.right - input.left,
          input.bottom - input.top}});
    const bool placeholder = state.chronicles_name_input.empty();
    std::string shown =
        placeholder ? fallback : state.chronicles_name_input + "|";
    old_font = SelectObject(dc, skin::font_heading());
    SetTextColor(dc, placeholder ? RGB(132, 147, 138) : RGB(226, 234, 222));
    // The authored 162px-wide raster contains its own frame/padding. A fixed
    // 12px text offset sat ON that frame once the image was stretched.
    const int inset = (input.right - input.left) * 14 / 162 + 4 * door_scale;
    RECT text_rect{input.left + inset, input.top + 8 * door_scale,
                   input.right - inset, input.bottom - 8 * door_scale};
    if (!placeholder) {
      SIZE extent{};
      while (shown.size() > 1) {
        GetTextExtentPoint32A(dc, shown.c_str(), static_cast<int>(shown.size()), &extent);
        if (extent.cx <= text_rect.right - text_rect.left) break;
        shown.erase(0, 1); // Scroll the visible suffix; never modify the name.
      }
    }
    DrawTextA(dc, shown.c_str(), static_cast<int>(shown.size()), &text_rect,
              DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
    state.hud_rect_trace.push_back(
        {"chronicles-name-text", {text_rect.left, text_rect.top,
          text_rect.right - text_rect.left, text_rect.bottom - text_rect.top}});
    SelectObject(dc, old_font);
    rl.push_back({render::Op::Chronicles, static_cast<double>(input.left),
                  static_cast<double>(input.top), 0.0, 0,
                  house ? "naming:house" : "naming:scion"});
    rl.push_back({render::Op::Chronicles, 0.0, 0.0, 0.0, 0,
                  "naming:value:" + shown});

    const std::string controls =
        "Type a name  |  Enter confirm  |  Esc cancel  |  Backspace edit";
    old_font = SelectObject(dc, skin::font_small());
    SetTextColor(dc, RGB(176, 192, 180));
    TextOutA(dc, modal.left + 24 * door_scale,
             modal.bottom - 42 * door_scale, controls.c_str(),
             static_cast<int>(controls.size()));
    if (!state.chronicles_name_error.empty()) {
      SetTextColor(dc, skin::kEmber);
      TextOutA(dc, modal.left + 24 * door_scale,
               modal.bottom - 66 * door_scale,
               state.chronicles_name_error.c_str(),
               static_cast<int>(state.chronicles_name_error.size()));
      rl.push_back({render::Op::Chronicles, 0.0, 0.0, 0.0, 0,
                    "naming:error:" + state.chronicles_name_error});
    }
    SelectObject(dc, old_font);
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
    if (conn == verdigris::client::ConnectionState::Ready ||
        conn == verdigris::client::ConnectionState::Connected)
      chip_color = RGB(120, 214, 168);
    else if (conn == verdigris::client::ConnectionState::Connecting ||
             conn == verdigris::client::ConnectionState::Retrying)
      chip_color = RGB(239, 208, 116);
    else if (conn == verdigris::client::ConnectionState::Disconnected ||
             conn == verdigris::client::ConnectionState::Rejected ||
             conn == verdigris::client::ConnectionState::ProtocolMismatch)
      chip_color = RGB(255, 80, 70);
    const bool compact_ready =
        conn == verdigris::client::ConnectionState::Ready ||
        conn == verdigris::client::ConnectionState::Connected;
    const int reserved_w = connection_chip_w(static_cast<int>(bounds.bottom));
    const int chip_h = connection_chip_h(static_cast<int>(bounds.bottom));
    const int chip_w = compact_ready ? chip_h : reserved_w;
    if (compact_ready) chip_x += reserved_w - chip_w;
    RECT chip_rect{chip_x, chip_y, chip_x + chip_w, chip_y + chip_h};
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
    if (compact_ready) {
      const int inset = (std::max)(4, chip_h / 3);
      HBRUSH signal = CreateSolidBrush(chip_color);
      HGDIOBJ old_signal = SelectObject(dc, signal);
      Ellipse(dc, chip_rect.left + inset, chip_rect.top + inset,
              chip_rect.right - inset, chip_rect.bottom - inset);
      SelectObject(dc, old_signal);
      DeleteObject(signal);
      rl.push_back({render::Op::Hud, static_cast<double>(chip_x),
                    static_cast<double>(chip_y), 0.0, 1,
                    "connection-ready-icon"});
    } else {
      std::string visible = label;
      std::transform(visible.begin(), visible.end(), visible.begin(),
                     [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
      TextOutA(dc, chip_x + 8, chip_y + 3, visible.c_str(),
               static_cast<int>(visible.size()));
    }
    if (conn == verdigris::client::ConnectionState::Disconnected ||
        conn == verdigris::client::ConnectionState::Rejected ||
        conn == verdigris::client::ConnectionState::ProtocolMismatch) {
      SetTextColor(dc, RGB(255, 80, 70));
      const char* banner = "CONNECTION LOST — not playing offline";
      // TASK-0159: the banner keeps the left column but starts below the
      // minimap panel instead of painting across it.
      const HudRect map = minimap_rect(
          static_cast<int>(bounds.right), static_cast<int>(bounds.bottom),
          active_minimap_side(state));
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
                          MinimapSide minimap_side,
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
  keep_out(minimap_rect(width, height, minimap_side));
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
  const HudRect map = minimap_rect(width, height, minimap_side);
  // The lane opposite/inside the selected map edge is the deterministic
  // second anchor for every region whose preferred pin is crowded. A right
  // corner map therefore never gets covered by a right-aligned status chip.
  const int lane_x = minimap_side == MinimapSide::Left
                         ? map.x + map.w + kTopHudGap
                         : kTopHudGutter + 6;
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
      const int lift = std::max(4, static_cast<int>((c1.y - c0.y) * 0.30));
      // Theme-tinted stone so each road's walls belong to its palette.
      COLORREF slab = RGB(88, 78, 66);
      if (world.theme == "crypt") slab = RGB(78, 82, 96);
      else if (world.theme == "marsh") slab = RGB(72, 88, 64);
      else if (world.theme == "wilds") slab = RGB(96, 80, 58);
      else if (world.theme == "grove") slab = RGB(76, 96, 70);
      // Shadowed face below the slab: a dark shade of the same stone, with
      // mortar seams, so wall rows read as masonry instead of a void band.
      RECT face{c0.x, c1.y - lift, c1.x, c1.y};
      HBRUSH face_brush = CreateSolidBrush(RGB(
          GetRValue(slab) / 3, GetGValue(slab) / 3, GetBValue(slab) / 3));
      FillRect(dc, &face, face_brush);
      DeleteObject(face_brush);
      draw_line(dc, c0.x + (c1.x - c0.x) / 2, c1.y - lift,
                c0.x + (c1.x - c0.x) / 2, c1.y, RGB(14, 12, 10), 1);
      // Raised top slab, clearly lighter than any floor plate.
      RECT top{c0.x, c0.y - lift, c1.x, c1.y - lift};
      HBRUSH top_brush = CreateSolidBrush(slab);
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
  const HudRect geometry = character_pane_rect(
      static_cast<int>(bounds.right), static_cast<int>(bounds.bottom));
  const int left = geometry.x;
  const int top = geometry.y;
  const int pane_w = geometry.w;
  const int bottom = geometry.y + geometry.h;
  RECT pane{left, top, left + pane_w, bottom};
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_panel, pane))
    skin::panel(dc, pane, skin::kVerdigris, 245, 8.0f);
  state.hud_rect_trace.push_back({"character-pane-frame", geometry});
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

  // Eleven authoritative server WearSet seats arranged around the Scion.
  struct DollSeat {
    const char* wire;
    const char* label;
    bool left_column;
    int row;
  };
  static constexpr DollSeat kDollSeats[] = {
      {"head", "HEAD", true, 0},
      {"armor", "ARMOR", true, 1},
      {"back", "BACK", true, 2},
      {"gloves", "GLOVES", true, 3},
      {"belt", "BELT", true, 4},
      {"feet", "FEET", true, 5},
      {"right_hand", "MAIN HAND", false, 0},
      {"left_hand", "OFF HAND", false, 1},
      {"necklace", "NECKLACE", false, 2},
      {"ring", "RING I", false, 3},
      {"ring2", "RING II", false, 4},
  };
  const int doll_top = top + 60 * s;
  const int slot_w = 142 * s;
  const int slot_h = 38 * s;
  const int slot_gap = 5 * s;
  const int left_slot_x = left + 14 * s;
  const int right_slot_x = pane.right - 14 * s - slot_w;
  const auto worn_in = [&](const char* seat) -> const WorldCarriedItem* {
    for (const auto& item : state.world.worn)
      if (item.equip_seat == seat) return &item;
    if (std::strcmp(seat, "right_hand") == 0)
      for (const auto& item : state.world.carried)
        if (item.equipped) return &item;
    return nullptr;
  };
  for (const auto& seat : kDollSeats) {
    const int sx = seat.left_column ? left_slot_x : right_slot_x;
    const int sy = doll_top + seat.row * (slot_h + slot_gap);
    RECT slot_rect{sx, sy, sx + slot_w, sy + slot_h};
    const WorldCarriedItem* worn = worn_in(seat.wire);
    if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot,
                            slot_rect))
      skin::slot(dc, slot_rect, worn ? skin::kGold : skin::kVerdigris, false);
    if (worn) {
      RECT art_rect{slot_rect.left + 3 * s, slot_rect.top + 3 * s,
                    slot_rect.left + 34 * s, slot_rect.bottom - 3 * s};
      const std::string art_key = worn->art_id.empty() ? worn->id : worn->art_id;
      draw_item_art(state.billboards, dc, art_key, art_rect);
    }
    HGDIOBJ slot_font = SelectObject(dc, skin::font_small());
    SetTextColor(dc, worn ? skin::kGold : RGB(112, 132, 122));
    const int text_x = slot_rect.left + (worn ? 39 : 7) * s;
    TextOutA(dc, text_x, slot_rect.top + 3 * s, seat.label,
             static_cast<int>(std::strlen(seat.label)));
    std::string item_name = worn ? worn->name : "empty";
    if (item_name.size() > 15) item_name = item_name.substr(0, 14) + ".";
    SetTextColor(dc, worn ? RGB(222, 232, 220) : RGB(92, 108, 101));
    TextOutA(dc, text_x, slot_rect.top + 19 * s, item_name.c_str(),
             static_cast<int>(item_name.size()));
    SelectObject(dc, slot_font);
    state.hud_rect_trace.push_back(
        {"paperdoll-seat",
         {slot_rect.left, slot_rect.top, slot_rect.right - slot_rect.left,
          slot_rect.bottom - slot_rect.top}});
    rl.push_back({render::Op::Hud, static_cast<double>(slot_rect.left),
                  static_cast<double>(slot_rect.top), 0.0, worn ? 1 : 0,
                  std::string("paperdoll:") + seat.wire + ":" +
                      (worn ? worn->name : "empty")});
  }

  // Portrait plate between the equipment columns.
  RECT portrait{left + 165 * s, doll_top, pane.right - 165 * s,
                doll_top + 6 * slot_h + 5 * slot_gap};
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot,
                          portrait))
    skin::slot(dc, portrait, skin::kVerdigris, false);
  if (state.billboards.player.ready() && state.billboards.alpha_blend) {
    const SpriteBitmap& sprite = state.billboards.player;
    const int available_h = portrait.bottom - portrait.top - 16 * s;
    const int dest_h = std::min(190 * s, available_h);
    const int dest_w = dest_h * sprite.width / std::max(1, sprite.height);
    const BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    state.billboards.alpha_blend(
        dc, (portrait.left + portrait.right - dest_w) / 2,
        portrait.bottom - dest_h - 8 * s, dest_w, dest_h, sprite.dc, 0, 0,
        sprite.width, sprite.height, blend);
  }
  HGDIOBJ caption_font = SelectObject(dc, skin::font_small());
  SetTextColor(dc, RGB(116, 174, 149));
  const char* caption = "SCION LOADOUT";
  SIZE caption_extent{};
  GetTextExtentPoint32A(dc, caption, static_cast<int>(std::strlen(caption)),
                        &caption_extent);
  TextOutA(dc, (portrait.left + portrait.right - caption_extent.cx) / 2,
           portrait.top + 7 * s, caption,
           static_cast<int>(std::strlen(caption)));
  SelectObject(dc, caption_font);

  const WorldActor& player = state.world.player;
  const WorldCarriedItem* main_hand = worn_in("right_hand");
  const int equipped_bonus = main_hand ? main_hand->attack_bonus : 0;
  int attr_str = 10, attr_dex = 10, attr_int = 10;
  if (state.session) {
    const auto& model = state.session->model();
    attr_str = model.attr_strength;
    attr_dex = model.attr_dexterity;
    attr_int = model.attr_intelligence;
  }
  const int stats_top = portrait.bottom + 8 * s;
  RECT stats_card{left + 14 * s, stats_top, pane.right - 14 * s,
                  bottom - 28 * s};
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot,
                          stats_card))
    skin::slot(dc, stats_card, skin::kVerdigris, false);
  const std::string stat_lines[] = {
      "VITALS  LVL " + std::to_string(player.level) + "   LIFE " +
          std::to_string(player.life) + "/" + std::to_string(player.life_max) +
          "   RES " + std::to_string(player.resource) + "/" +
          std::to_string(player.resource_max),
      "ATTRIBUTES  STR " + std::to_string(attr_str) + "   DEX " +
          std::to_string(attr_dex) + "   INT " + std::to_string(attr_int),
      "COMBAT  ATK " + std::to_string(player.attack + equipped_bonus) +
          "   DEF " + std::to_string(player.defense) + "   BLEED " +
          std::to_string(player.bleed_chance) + "%",
      "MOBILITY  SPEED +" + std::to_string(player.movement_speed_percent) +
          "%   REACH +" + std::to_string(player.reach_percent) + "%",
      "WARDS  EMBER " + std::to_string(player.ember_resistance) +
          "   RIVER " + std::to_string(player.river_resistance),
  };
  HGDIOBJ stat_font = SelectObject(dc, skin::font_small());
  const int available_stat_height = static_cast<int>(
      stats_card.bottom - stats_card.top - 10 * s);
  const int stat_row_h = std::clamp(
      available_stat_height / static_cast<int>(std::size(stat_lines)),
      15 * s, 30 * s);
  int stat_y = stats_card.top + 5 * s;
  for (const auto& line : stat_lines) {
    if (stat_y + stat_row_h > stats_card.bottom) break;
    SetTextColor(dc, line.rfind("VITALS", 0) == 0 ? skin::kGold
                                                    : RGB(178, 198, 186));
    TextOutA(dc, stats_card.left + 8 * s, stat_y, line.c_str(),
             static_cast<int>(line.size()));
    rl.push_back({render::Op::Hud, static_cast<double>(stats_card.left),
                  static_cast<double>(stat_y), 0.0, 0, "char:" + line});
    stat_y += stat_row_h;
  }
  SelectObject(dc, stat_font);
  SelectObject(dc, skin::font_small());
  SetTextColor(dc, skin::kInkDim);
  const char* footer = "C / Esc closes   |   I toggles backpack";
  TextOutA(dc, left + 20 * s, bottom - 21 * s, footer,
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
  if (hit.kind == 3) {
    // A chart node: set out on that stretch of road.
    state.session->submit(
        verdigris::client::ClientCommand::enter_zone(hit.ref));
    show_hint(state, "The road takes you");
    return;
  }
  if (hit.kind == 4) {
    state.session->submit(verdigris::client::ClientCommand::menu_action(
        hit.ref, hit.extra, 0));
    return;
  }
  if (hit.kind == 5) {
    if (hit.value <= 0) {
      show_hint(state, hit.extra.empty() ? "This vessel cannot take a Brand"
                                         : hit.extra);
      return;
    }
    state.session->submit(verdigris::client::ClientCommand::menu_action(
        "player:vesselforge:add-brand", hit.ref, hit.value));
    show_hint(state, "Tamar raises the searing iron");
    return;
  }
  if (hit.kind == 6) {
    if (hit.value <= 0) {
      show_hint(state, hit.extra.empty() ? "That trophy is not ready"
                                         : hit.extra);
      return;
    }
    state.session->submit(verdigris::client::ClientCommand::menu_action(
        "player:vesselforge:socket-trophy", hit.ref, 0, hit.choice));
    show_hint(state, "Tamar sets the trophy in bronze");
    return;
  }
  const char* action = hit.kind == 0   ? "player:shop:buy"
                       : hit.kind == 1 ? "player:bank:withdraw"
                                       : "player:bank:deposit";
  state.session->submit(
      verdigris::client::ClientCommand::menu_action(action, hit.ref, hit.value));
}

void paint_dialogue_pane(ClientState& state, HDC dc, const RECT& bounds,
                         render::List& rl) {
  const auto& dialogue = state.session->model().dialogue;
  if (!dialogue.open) return;
  const int s = hud_scale(static_cast<int>(bounds.bottom));
  const int pane_w = 620 * s;
  const int title_h = 52 * s;
  const int body_h = 94 * s;
  const int row_h = 58 * s;
  const int footer_h = 30 * s;
  const int option_count = static_cast<int>(dialogue.options.size());
  const int pane_h = title_h + body_h + option_count * row_h + footer_h + 18 * s;
  const int left = (static_cast<int>(bounds.right) - pane_w) / 2;
  const int top = std::max(20 * s,
      (static_cast<int>(bounds.bottom) - pane_h) / 2 - 12 * s);
  RECT pane{left, top, left + pane_w, top + pane_h};
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_panel, pane))
    skin::panel(dc, pane, skin::kVerdigris, 250, 9.0f);
  rl.push_back({render::Op::Hud, static_cast<double>(left),
                static_cast<double>(top), 0.0, 0, "dialogue-pane"});

  SetBkMode(dc, TRANSPARENT);
  HGDIOBJ old_font = SelectObject(dc, skin::font_heading());
  SetTextColor(dc, skin::kGold);
  TextOutA(dc, left + 18 * s, top + 10 * s, dialogue.name.c_str(),
           static_cast<int>(dialogue.name.size()));
  std::string role = dialogue.role;
  std::replace(role.begin(), role.end(), '_', ' ');
  std::transform(role.begin(), role.end(), role.begin(),
                 [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
  SelectObject(dc, skin::font_small());
  SetTextColor(dc, skin::kVerdigris);
  TextOutA(dc, left + 19 * s, top + 34 * s, role.c_str(),
           static_cast<int>(role.size()));

  SelectObject(dc, skin::font_body());
  SetTextColor(dc, skin::kInk);
  RECT body{left + 18 * s, top + title_h, left + pane_w - 18 * s,
            top + title_h + body_h};
  DrawTextA(dc, dialogue.body.c_str(), static_cast<int>(dialogue.body.size()),
            &body, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

  std::size_t activatable = 0;
  for (const auto& option : dialogue.options)
    if (option.enabled) ++activatable;
  if (activatable == 0) state.trade_selected = 0;
  else if (state.trade_selected >= activatable)
    state.trade_selected = activatable - 1;

  int y = top + title_h + body_h;
  std::size_t active_index = 0;
  for (const auto& option : dialogue.options) {
    RECT line{left + 12 * s, y, left + pane_w - 12 * s, y + row_h - 4 * s};
    const bool selected = option.enabled && active_index == state.trade_selected;
    if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot, line))
      skin::slot(dc, line, skin::kVerdigris, selected);
    if (selected) {
      HPEN pen = CreatePen(PS_SOLID, 2, skin::kGold);
      HGDIOBJ old_pen = SelectObject(dc, pen);
      HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
      Rectangle(dc, line.left, line.top, line.right, line.bottom);
      SelectObject(dc, old_brush); SelectObject(dc, old_pen); DeleteObject(pen);
    }
    SelectObject(dc, skin::font_body_bold());
    SetTextColor(dc, option.enabled ? skin::kInk : skin::kInkDim);
    TextOutA(dc, line.left + 10 * s, line.top + 7 * s, option.label.c_str(),
             static_cast<int>(option.label.size()));
    SelectObject(dc, skin::font_small());
    SetTextColor(dc, skin::kInkDim);
    TextOutA(dc, line.left + 10 * s, line.top + 30 * s, option.hint.c_str(),
             static_cast<int>(option.hint.size()));
    if (option.enabled) {
      ClientState::TradeRowHit hit;
      hit.rect = line; hit.kind = 4; hit.index = active_index;
      hit.ref = option.action; hit.extra = option.id;
      state.trade_row_hits.push_back(std::move(hit));
      ++active_index;
    }
    rl.push_back({render::Op::Hud, static_cast<double>(line.left),
                  static_cast<double>(line.top), 0.0, 0,
                  "dialogue-option:" + option.id});
    y += row_h;
  }
  SelectObject(dc, skin::font_small());
  SetTextColor(dc, skin::kInkDim);
  const char* footer = "Click or Enter to choose  |  Esc closes";
  TextOutA(dc, left + 18 * s, top + pane_h - footer_h, footer,
           static_cast<int>(strlen(footer)));
  SelectObject(dc, old_font);
}

void paint_vesselforge_pane(ClientState& state, HDC dc, const RECT& bounds,
                            render::List& rl) {
  const auto& forge = state.session->model().forge;
  if (!forge.open) return;
  const int s = hud_scale(static_cast<int>(bounds.bottom));
  const int pane_w = 820 * s;
  const int pane_h = 500 * s;
  const int left = (static_cast<int>(bounds.right) - pane_w) / 2;
  const int top = (static_cast<int>(bounds.bottom) - pane_h) / 2;
  RECT pane{left, top, left + pane_w, top + pane_h};
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_panel, pane))
    skin::panel(dc, pane, skin::kGold, 250, 10.0f);
  rl.push_back({render::Op::Hud, static_cast<double>(left),
                static_cast<double>(top), 0.0, 0, "vesselforge-pane"});

  SetBkMode(dc, TRANSPARENT);
  HGDIOBJ old_font = SelectObject(dc, skin::font_heading());
  SetTextColor(dc, skin::kGold);
  const std::string title = forge.name.empty() ? "Tamar's Vesselforge" : forge.name;
  TextOutA(dc, left + 18 * s, top + 12 * s, title.c_str(),
           static_cast<int>(title.size()));
  SelectObject(dc, skin::font_small());
  SetTextColor(dc, skin::kInkDim);
  const std::string purse = "CARRYING  " + std::to_string(forge.carried_coins) + " GOLD";
  SIZE purse_extent{};
  GetTextExtentPoint32A(dc, purse.c_str(), static_cast<int>(purse.size()),
                        &purse_extent);
  TextOutA(dc, left + pane_w - purse_extent.cx - 18 * s, top + 19 * s,
           purse.c_str(), static_cast<int>(purse.size()));

  const int content_top = top + 52 * s;
  const int list_w = 350 * s;
  const int row_h = 70 * s;
  constexpr std::size_t kVisibleRows = 5;
  if (forge.rows.empty()) state.trade_selected = 0;
  else if (state.trade_selected >= forge.rows.size())
    state.trade_selected = forge.rows.size() - 1;
  const std::size_t page_start = forge.rows.empty()
      ? 0
      : (state.trade_selected / kVisibleRows) * kVisibleRows;
  const std::size_t page_end =
      (std::min)(forge.rows.size(), page_start + kVisibleRows);

  if (forge.rows.empty()) {
    RECT empty{left + 18 * s, content_top + 18 * s,
               left + list_w - 10 * s, content_top + 90 * s};
    SelectObject(dc, skin::font_body());
    SetTextColor(dc, skin::kInkDim);
    const char* copy = "Bring carried vessel gear to Tamar. Equipped gear must be returned to your pack first.";
    DrawTextA(dc, copy, static_cast<int>(std::strlen(copy)), &empty,
              DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);
  }

  int y = content_top;
  for (std::size_t i = page_start; i < page_end; ++i) {
    const auto& row = forge.rows[i];
    RECT line{left + 14 * s, y, left + list_w - 4 * s, y + row_h - 6 * s};
    const bool selected = i == state.trade_selected;
    if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot, line))
      skin::slot(dc, line, skin::kGold, selected);
    if (selected) {
      HPEN pen = CreatePen(PS_SOLID, 2, skin::kGold);
      HGDIOBJ old_pen = SelectObject(dc, pen);
      HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
      Rectangle(dc, line.left, line.top, line.right, line.bottom);
      SelectObject(dc, old_brush); SelectObject(dc, old_pen); DeleteObject(pen);
    }
    SelectObject(dc, skin::font_body_bold());
    SetTextColor(dc, row.eligible ? skin::kInk : skin::kInkDim);
    RECT name{line.left + 10 * s, line.top + 7 * s,
              line.right - 10 * s, line.top + 27 * s};
    DrawTextA(dc, row.name.c_str(), static_cast<int>(row.name.size()), &name,
              DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    SelectObject(dc, skin::font_small());
    SetTextColor(dc, row.eligible ? skin::kVerdigris : skin::kEmber);
    const std::string status = row.eligible
        ? "READY  |  " + std::to_string(row.cost) + " GOLD"
        : row.reason;
    RECT status_rect{line.left + 10 * s, line.top + 36 * s,
                     line.right - 10 * s, line.bottom - 4 * s};
    DrawTextA(dc, status.c_str(), static_cast<int>(status.size()), &status_rect,
              DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    ClientState::TradeRowHit hit;
    hit.rect = line; hit.kind = 5; hit.index = i; hit.ref = row.uuid;
    hit.extra = row.reason; hit.value = row.eligible ? row.cost : -1;
    state.trade_row_hits.push_back(std::move(hit));
    rl.push_back({render::Op::Hud, static_cast<double>(line.left),
                  static_cast<double>(line.top), 0.0, static_cast<int>(i),
                  "vesselforge-row:" + row.uuid});
    y += row_h;
  }

  const int detail_left = left + list_w + 18 * s;
  const int detail_right = left + pane_w - 18 * s;
  RECT detail{detail_left, content_top, detail_right, top + pane_h - 48 * s};
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_slot, detail))
    skin::slot(dc, detail, skin::kVerdigris, false);
  if (!forge.rows.empty()) {
    const auto& row = forge.rows[state.trade_selected];
    int dy = detail.top + 14 * s;
    SelectObject(dc, skin::font_heading());
    SetTextColor(dc, skin::kInk);
    RECT detail_name{detail.left + 14 * s, dy, detail.right - 14 * s,
                     dy + 28 * s};
    DrawTextA(dc, row.name.c_str(), static_cast<int>(row.name.size()),
              &detail_name, DT_LEFT | DT_TOP | DT_SINGLELINE |
                            DT_END_ELLIPSIS | DT_NOPREFIX);
    dy += 34 * s;
    SelectObject(dc, skin::font_small());
    SetTextColor(dc, skin::kGold);
    const std::string identity = "LEVEL " + std::to_string(row.item_level) +
        "  |  " + row.material + " " + row.form;
    TextOutA(dc, detail.left + 14 * s, dy, identity.c_str(),
             static_cast<int>(identity.size()));
    dy += 28 * s;
    SelectObject(dc, skin::font_body_bold());
    SetTextColor(dc, skin::kVerdigris);
    const std::string vessel = "VESSEL  " + std::to_string(row.used) + "/" +
        std::to_string(row.vessel) + "     PATIENCE  " +
        std::to_string(row.patience) + "/" +
        std::to_string(row.patience_max);
    TextOutA(dc, detail.left + 14 * s, dy, vessel.c_str(),
             static_cast<int>(vessel.size()));
    dy += 27 * s;
    SelectObject(dc, skin::font_small());
    SetTextColor(dc, row.awakened ? skin::kGold : skin::kVerdigris);
    const std::string memory = row.awakened
        ? "AWAKENED  |  " + std::to_string(row.evolutions) + " EVOLUTIONS"
        : "ATTUNEMENT  " + std::to_string(row.attunement) + "/" +
              std::to_string(row.attunement_next) + "  |  " +
              std::to_string(row.bond_count) + " BONDS  |  " +
              std::to_string(row.trophy_count) + " TROPHIES  |  " +
              std::to_string(row.evolutions) + " EVOLUTIONS";
    TextOutA(dc, detail.left + 14 * s, dy, memory.c_str(),
             static_cast<int>(memory.size()));
    rl.push_back({render::Op::PaneStat, static_cast<double>(detail.left + 14 * s),
                  static_cast<double>(dy), 0.0, row.evolutions,
                  "vesselforge-memory:" + memory});
    dy += 25 * s;
    SelectObject(dc, skin::font_small());
    const int action_y = detail.bottom - 52 * s;
    const int lines_bottom = action_y - 12 * s;
    std::size_t painted_lines = 0;
    for (const auto& forge_line : row.lines) {
      if (painted_lines >= 7 || dy >= lines_bottom) break;
      SetTextColor(dc, forge_line.tone == "inactive" ? skin::kInkDim
                                                       : skin::kInk);
      std::string display_line = forge_line.text;
      if (display_line.rfind("Dormant - BOND:", 0) == 0) {
        const std::size_t effect_break = display_line.find(" - ", 16);
        if (effect_break != std::string::npos)
          display_line.replace(effect_break, 3, "\n");
      }
      RECT measured{detail.left + 14 * s, 0, detail.right - 14 * s, 0};
      DrawTextA(dc, display_line.c_str(),
                static_cast<int>(display_line.size()), &measured,
                DT_LEFT | DT_TOP | DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX);
      const int line_height = std::clamp(
          static_cast<int>(measured.bottom - measured.top), 16 * s, 38 * s);
      if (dy + line_height > lines_bottom) break;
      RECT text_rect{detail.left + 14 * s, dy, detail.right - 14 * s,
                     dy + line_height};
      DrawTextA(dc, display_line.c_str(),
                static_cast<int>(display_line.size()), &text_rect,
                DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);
      rl.push_back({render::Op::PaneStat, static_cast<double>(text_rect.left),
                    static_cast<double>(text_rect.top), 0.0, 0,
                    "vesselforge-line:" + forge_line.text});
      dy += line_height + 4 * s;
      ++painted_lines;
    }
    if (row.lines.size() > painted_lines && dy + 18 * s <= lines_bottom) {
      const std::string more = "+" +
          std::to_string(row.lines.size() - painted_lines) + " more properties";
      SetTextColor(dc, skin::kInkDim);
      TextOutA(dc, detail.left + 14 * s, dy, more.c_str(),
               static_cast<int>(more.size()));
    }
    SelectObject(dc, skin::font_body_bold());
    SetTextColor(dc, row.eligible ? skin::kGold : skin::kEmber);
    const std::string action = row.eligible
        ? "SEAR A NEW BRAND  -  " + std::to_string(row.cost) + " GOLD"
        : row.reason;
    TextOutA(dc, detail.left + 14 * s, action_y, action.c_str(),
             static_cast<int>(action.size()));
    const verdigris::client::ClientForgeTrophyOption* trophy_action = nullptr;
    for (const auto& option : row.trophy_options) {
      if (!trophy_action || option.eligible) trophy_action = &option;
      if (option.eligible) break;
    }
    if (trophy_action) {
      const std::string trophy_copy = trophy_action->eligible
          ? "T  BIND " + trophy_action->name + "  -  " +
                std::to_string(trophy_action->fragments) + "/" +
                std::to_string(trophy_action->required) + " FRAGMENTS"
          : "TROPHY  " + trophy_action->name + "  " +
                std::to_string(trophy_action->fragments) + "/" +
                std::to_string(trophy_action->required) + "  -  " +
                trophy_action->reason;
      SelectObject(dc, skin::font_small());
      SetTextColor(dc, trophy_action->eligible ? skin::kVerdigris
                                                : skin::kInkDim);
      const int trophy_y = action_y + 22 * s;
      RECT trophy_rect{detail.left + 10 * s, trophy_y - 3 * s,
                       detail.right - 10 * s, trophy_y + 17 * s};
      DrawTextA(dc, trophy_copy.c_str(), static_cast<int>(trophy_copy.size()),
                &trophy_rect, DT_LEFT | DT_TOP | DT_SINGLELINE |
                                  DT_END_ELLIPSIS | DT_NOPREFIX);
      ClientState::TradeRowHit hit;
      hit.rect = trophy_rect;
      hit.kind = 6;
      hit.index = state.trade_selected;
      hit.ref = row.uuid;
      hit.extra = trophy_action->reason;
      hit.choice = trophy_action->id;
      hit.value = trophy_action->eligible ? 1 : -1;
      state.trade_row_hits.push_back(std::move(hit));
      rl.push_back({render::Op::PaneStat,
                    static_cast<double>(trophy_rect.left),
                    static_cast<double>(trophy_rect.top), 0.0,
                    trophy_action->eligible ? 1 : 0,
                    "vesselforge-trophy:" + trophy_action->id});
    }
  }

  SelectObject(dc, skin::font_small());
  SetTextColor(dc, skin::kInkDim);
  const std::string footer = forge.rows.size() > kVisibleRows
      ? "Up/Down vessel  |  Enter sears  |  T binds trophy  |  Esc  |  " +
            std::to_string(state.trade_selected + 1) + "/" +
            std::to_string(forge.rows.size())
      : "Up/Down vessel  |  Enter sears  |  T binds trophy  |  Esc closes";
  TextOutA(dc, left + 18 * s, top + pane_h - 30 * s, footer.c_str(),
           static_cast<int>(footer.size()));
  SelectObject(dc, old_font);
}

void paint_trade_pane(ClientState& state, HDC dc, const RECT& bounds,
                      render::List& rl) {
  state.trade_row_hits.clear();
  // The server may take one network round trip to acknowledge CloseScreen.
  // Once the tactical chart is locally open, never paint or hit-test a stale
  // commerce/dialogue pane over it during that acknowledgement window.
  if (state.minimap_mode == MinimapMode::Overlay) return;
  if (!state.session) return;
  const auto& model = state.session->model();
  if (model.dialogue.open) {
    paint_dialogue_pane(state, dc, bounds, rl);
    return;
  }
  if (model.forge.open) {
    paint_vesselforge_pane(state, dc, bounds, rl);
    return;
  }
  if (!model.shop.open && !model.bank.open && !model.chart.open) return;
  const int s = hud_scale(static_cast<int>(bounds.bottom));
  const bool chart = model.chart.open;
  const bool shop = !chart && model.shop.open;
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
  if (chart) {
    for (const auto& node : model.chart.nodes) {
      Row row;
      row.left = "T" + std::to_string(node.tier) + "  " + node.name +
                 (node.warden.empty() ? "" : "  -  " + node.warden);
      row.right = node.status;
      if (node.status == "open") {
        row.hit.kind = 3;
        row.hit.ref = node.id;
      } else {
        row.header = true;  // barred/cleared rows render but do not activate
      }
      rows.push_back(std::move(row));
    }
    if (rows.empty()) {
      Row row;
      row.left = "No stretch of this road is charted yet.";
      row.header = true;
      rows.push_back(std::move(row));
    }
  } else if (shop) {
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
                chart ? "chart-pane" : shop ? "shop-pane" : "bank-pane"});

  SetBkMode(dc, TRANSPARENT);
  HGDIOBJ old_font = SelectObject(dc, skin::font_heading());
  SetTextColor(dc, shop ? skin::kGold : skin::kVerdigris);
  const std::string title = chart ? model.chart.road_name
                            : shop  ? model.shop.name
                                    : "Rhea's Countinghouse";
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
      (chart ? model.chart.blurb
       : shop ? "carrying " + std::to_string(model.shop.carried_coins) + "g"
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
  if (state.startup_creation) {
    paint_startup_creation(state,dc,bounds,rl);
    state.render_list=std::move(rl);
    return;
  }
  if (state.title_open) {
    paint_title_scene(state, dc, bounds, rl);
    state.render_list = std::move(rl);
    return;
  }
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
             &state.floor_cache, world.theme);
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
    const long long hover_dx = state.mouse.x - pad.x;
    const long long hover_dy = state.mouse.y - pad.y;
    const bool hovered = hover_dx * hover_dx + hover_dy * hover_dy <=
                         static_cast<long long>(pad_r + 14) * (pad_r + 14);
    ring_ellipse(dc, pad.x, pad.y, pad_r, pad_r,
                 hovered ? RGB(255, 255, 246) : RGB(120, 214, 168),
                 hovered ? 5 : 3);
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
                          state.scenery[entry.index], rl,
                          world.theme == "town",
                          state.breathe_phase * 2.0 * kPi);
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
        // as motion rather than three poses. The same phase drives the
        // rig's arm swing.
        double attack_phase = 0.0;
        for (const auto& fx : state.effects) {
          if (fx.kind != EffectFx::Kind::Swing &&
              fx.kind != EffectFx::Kind::SweepArc)
            continue;
          const double phase = std::clamp(
              (static_cast<double>(fx.age) + state.tick_accum_ms / 50.0) /
                  std::max(1, fx.ttl),
              0.0, 1.0);
          attack_phase = phase;
          const double push = std::sin(phase * kPi) * kTileUnits * 0.28;
          base.x += static_cast<int>(std::cos(fx.angle) * push * base.scale);
          base.y += static_cast<int>(std::sin(fx.angle) * push * base.scale);
          break;
        }
        {
          const auto& motion = state.motions["player"];
          vector_art::Pose pose;
          pose.walk = motion.walk_phase;
          pose.moving = motion.moving;
          pose.breathe = state.breathe_phase;
          pose.attack = attack_phase;
          pose.mirror = player.facing.x < 0;
          vector_art::humanoid(dc, base.x, base.y,
                               std::max(10, static_cast<int>(kTileUnits * 1.5 *
                                                             base.scale)),
                               vector_art::player_style(), pose,
                               vector_art::Held::Axe);
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
        ScreenPoint base =
            project(state.camera, bounds, monster.position.x, monster.position.y);
        // Presentation-only combat body language, derived entirely from
        // authoritative positions and events: a windup lean away from the
        // player while its telegraph runs, a lunge into the player when a
        // strike lands, and sprite mirroring toward the player.
        const double to_player_x =
            static_cast<double>(world.player.position.x - monster.position.x);
        const double to_player_y =
            static_cast<double>(world.player.position.y - monster.position.y);
        const double to_player_len = std::max(
            1.0, std::sqrt(to_player_x * to_player_x + to_player_y * to_player_y));
        const int mirror_x = to_player_x < 0.0 ? -1 : 1;
        double monster_attack_phase = 0.0;
        {
          const auto telegraph = state.telegraphs.find(monster.id);
          if (telegraph != state.telegraphs.end()) {
            const double windup = std::clamp(
                (static_cast<double>(world.tick - telegraph->second.start_tick) +
                 state.tick_accum_ms / 50.0) /
                    std::max(1, telegraph->second.windup_ticks),
                0.0, 1.0);
            const double lean = windup * kTileUnits * 0.14;
            base.x -= static_cast<int>(to_player_x / to_player_len * lean *
                                       base.scale);
            base.y -= static_cast<int>(to_player_y / to_player_len * lean *
                                       base.scale);
          }
          const auto strike = state.monster_strikes.find(monster.id);
          if (strike != state.monster_strikes.end() &&
              world.tick >= strike->second) {
            const double phase = std::clamp(
                (static_cast<double>(world.tick - strike->second) +
                 state.tick_accum_ms / 50.0) /
                    4.0,
                0.0, 1.0);
            if (phase < 1.0) {
              monster_attack_phase = phase;
              const double push = std::sin(phase * kPi) * kTileUnits * 0.4;
              base.x += static_cast<int>(to_player_x / to_player_len * push *
                                         base.scale);
              base.y += static_cast<int>(to_player_y / to_player_len * push *
                                         base.scale);
            }
          }
        }
        rl.push_back({render::Op::Monster, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, monster.life,
                      monster.elite ? "elite" : "monster"});
        draw_contact_shadow(dc, base, kTileUnits * 0.42);
        draw_team_ring(dc, base, kTileUnits * 0.58,
                       monster.elite ? RGB(239, 208, 116) : RGB(214, 92, 72));
        if (monster.bleeding) {
          const int bleed_r = std::max(
              7, static_cast<int>(kTileUnits * 0.48 * base.scale));
          ring_ellipse(dc, base.x, base.y, bleed_r, bleed_r,
                       RGB(phase_a::kBleedColor.r, phase_a::kBleedColor.g,
                           phase_a::kBleedColor.b), 2);
          rl.push_back({render::Op::TargetFlash, static_cast<double>(base.x),
                        static_cast<double>(base.y), 0.0, 0, "bleeding"});
        }
        const double foe_height =
            monster.elite ? kTileUnits * 1.9 : kTileUnits * 1.5;
        {
          // Animated vector rig, chosen by theme and combat role so every
          // road fields a visually distinct bestiary.
          const auto& motion_it = state.motions[monster.id];
          vector_art::Pose pose;
          pose.walk = motion_it.walk_phase;
          pose.moving = motion_it.moving;
          pose.breathe = std::fmod(
              state.breathe_phase +
                  static_cast<double>(monster.position.x % 97) / 97.0,
              1.0);
          pose.attack = monster_attack_phase;
          pose.mirror = mirror_x < 0;
          const vector_art::Style style =
              vector_art::monster_style(world.theme, monster.elite);
          const int rig_h =
              std::max(10, static_cast<int>(foe_height * base.scale));
          if (monster.behaviour == "buffer") {
            vector_art::totem(dc, base.x, base.y, rig_h, style, pose);
          } else if (monster.behaviour == "ranged") {
            vector_art::humanoid(dc, base.x, base.y, rig_h, style, pose,
                                 vector_art::Held::Bow);
          } else if (world.theme == "crypt") {
            vector_art::wight(dc, base.x, base.y, rig_h, style, pose);
          } else if (world.theme == "wilds") {
            vector_art::beast(dc, base.x, base.y, rig_h, style, pose);
          } else if (world.theme == "marsh") {
            vector_art::ghast(dc, base.x, base.y, rig_h, style, pose);
          } else {
            vector_art::lurker(dc, base.x, base.y, rig_h, style, pose);
          }
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
        const bool vesselwright =
            std::find(npc.services.begin(), npc.services.end(),
                      std::string("vesselforge")) != npc.services.end();
        if (vesselwright) {
          ring = RGB(215, 138, 71);
        } else if (!npc.actions.empty()) {
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
          vector_art::Pose pose;
          pose.breathe = std::fmod(state.breathe_phase + npc.id * 0.23, 1.0);
          pose.mirror = world.player.position.x < npc.position.x;
          vector_art::Held prop = vector_art::Held::None;
          if (vesselwright) {
            prop = vector_art::Held::Club;  // Tamar's heavy searing tool
          } else if (!npc.actions.empty()) {
            const std::string& lead = npc.actions.front();
            if (lead == "talk") prop = vector_art::Held::Staff;
            else if (lead == "trade") prop = vector_art::Held::Scales;
            else if (lead == "bank") prop = vector_art::Held::Ledger;
            else if (std::find(npc.actions.begin(), npc.actions.end(),
                               std::string("trade")) != npc.actions.end())
              prop = vector_art::Held::Sword;  // the weapons trader
          }
          vector_art::humanoid(
              dc, base.x, base.y,
              std::max(10, static_cast<int>(kTileUnits * 1.45 * base.scale)),
              vector_art::npc_style(npc.id), pose, prop);
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
            if (std::find(npc.services.begin(), npc.services.end(),
                          std::string("house_investment")) != npc.services.end()) {
              verb = "Talk";
            } else {
              for (const auto& action : npc.actions) {
                if (action == "talk") { verb = "Talk"; break; }
                if (action == "trade") { verb = "Trade"; break; }
                if (action == "bank") { verb = "Bank"; break; }
              }
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
  if (!(state.gear_overlay && state.character_pane))
    paint_minimap(state, dc, bounds, rl);
  paint_vital_orbs(state.billboards, player, world.tick,
                   state.screen_pulse_ticks, dc, bounds, rl,
                   &state.hud_rect_trace);
  paint_quickbar(state, dc, bounds, rl);
  paint_xp_bar(state, dc, bounds, rl);
  paint_hover_tooltip(state, dc, bounds, rl);
  paint_gear_overlay(state, dc, bounds, rl);
  paint_character_pane(state, dc, bounds, rl);
  paint_tree_pane(state, dc, bounds, rl);
  paint_trade_pane(state, dc, bounds, rl);
  paint_quest_journal(state, dc, bounds, rl);

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
  // C+I is a focused loadout view. The panes already repeat identity,
  // objective context, and controls; suppressing the world HUD prevents its
  // long instruction rail from painting across paper-doll seats at 960x600.
  // The Chronicle journal is likewise modal and supplies its own campaign
  // header, objective, and close hint; top-HUD chips must never overpaint it.
  if (!trade_pane_open(state) &&
      !(state.gear_overlay && state.character_pane) &&
      !state.quest_journal) {
    std::string objective;
    COLORREF accent = RGB(120, 214, 168);
    const bool carrying = !world.carried.empty() || world.carried_trophies > 0;
    if (!world.has_extraction) {
      // In town the NPC roster is the tell; guide toward the story loop
      // instead of the placeholder explore line.
      if (world.endgame.unlocked) {
        objective = "endgame: " + std::to_string(world.endgame.completed) +
                    " clears | mastery " +
                    std::to_string(world.endgame.mastered) + "/" +
                    std::to_string(world.endgame.mastery_total) +
                    " - J ledger, I tablet";
        accent = skin::kGold;
      } else if (state.session && state.session->model().quests.present &&
                 !state.session->model().quests.active_id.empty()) {
        const auto& quest = state.session->model().quests;
        objective = "quest: " + quest.objective + " (" +
                    std::to_string(quest.objective_index + 1) + "/" +
                    std::to_string(quest.objective_count) + ") - J journal";
        accent = RGB(124, 220, 172);
      } else {
        objective = !world.npcs.empty()
                        ? "objective: hail an NPC with T - press N to take the tin road"
                        : "objective: explore the route";
      }
    } else if (world.expedition_phase == ExpeditionPhaseView::SlayWardens) {
      if (world.endgame.active) {
        objective = "charted T" + std::to_string(world.endgame.tier) +
                    (world.endgame.first_clear ? " [new mastery]" : "") +
                    ": slay the Seal-Bound Warden (" +
                    std::to_string(world.monsters.size()) + " remain, +" +
                    std::to_string(world.endgame.goods_found_percent) +
                    "% goods)";
        accent = skin::kGold;
      } else {
        objective = "objective: slay the wardens (" +
                    std::to_string(world.monsters.size()) + " remain)";
        accent = skin::kEmber;
      }
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
        house_prefixed_name(world.house_name) + " - Scion " +
        (world.scion_name.empty() ? std::string("(unnamed)") : world.scion_name);
    static constexpr char kControls[] =
        "WASD move | mouse aim | LMB attack | RMB/Space dash | Q E R skills | "
        "X take | Z names | I gear | J journal | T hail | TAB map | N road";
    const std::string& art_text = state.billboards.framekit_status;

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
        active_minimap_side(state),
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

    paint_status_chip(&state.billboards, &state.billboards.fk_banner, dc,
                      objective_at.x, objective_at.y, objective, accent, rl);
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

    const bool plates = state.billboards.fk_panel_ornate.ready() &&
                        state.billboards.fk_orb_life.ready() &&
                        state.billboards.fk_orb_resource.ready();
    const COLORREF art_accent =
        plates ? RGB(120, 214, 168) : RGB(239, 190, 78);
    paint_status_chip(&state.billboards, &state.billboards.fk_button, dc,
                      art_at.x, art_at.y, art_text, art_accent, rl);
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
    advance_startup_creation(state);
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

  const bool at_front_door = state.title_open || state.startup_creation ||
      (state.screen == Screen::Chronicles && state.session != nullptr);
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

void smooth_remote_monster_positions(ClientState& state, double dt_ms,
                                     bool enabled) {
  if (!enabled) return;
  for (auto& monster : state.world.monsters) {
    auto& motion = state.motions[monster.id];
    const verdigris::Vec2 authoritative = monster.position;
    if (!motion.has_display) {
      motion.has_display = true;
      motion.display_x = static_cast<double>(authoritative.x);
      motion.display_y = static_cast<double>(authoritative.y);
      motion.from_x = motion.display_x;
      motion.from_y = motion.display_y;
      motion.target_pos = authoritative;
    } else if (authoritative.x != motion.target_pos.x ||
               authoritative.y != motion.target_pos.y) {
      const double jump_x = authoritative.x - motion.display_x;
      const double jump_y = authoritative.y - motion.display_y;
      const double jump = std::sqrt(jump_x * jump_x + jump_y * jump_y);
      motion.from_x = motion.display_x;
      motion.from_y = motion.display_y;
      motion.target_pos = authoritative;
      motion.move_elapsed_ms = 0.0;
      motion.move_duration_ms = std::clamp(
          static_cast<double>(monster.move_duration_ms), 50.0, 1000.0);
      if (jump > kTileUnits * 3.0) {
        // Admission/reconnect/scene replacement: snap instead of gliding a
        // newly authoritative actor across unrelated terrain.
        motion.display_x = static_cast<double>(authoritative.x);
        motion.display_y = static_cast<double>(authoritative.y);
        motion.from_x = motion.display_x;
        motion.from_y = motion.display_y;
        motion.move_elapsed_ms = motion.move_duration_ms;
      }
    }
    motion.move_elapsed_ms = std::min(
        motion.move_duration_ms, motion.move_elapsed_ms + std::max(0.0, dt_ms));
    const double linear = motion.move_duration_ms > 0.0
        ? motion.move_elapsed_ms / motion.move_duration_ms : 1.0;
    const double eased = linear * linear * (3.0 - 2.0 * linear);
    motion.display_x = motion.from_x +
        (motion.target_pos.x - motion.from_x) * eased;
    motion.display_y = motion.from_y +
        (motion.target_pos.y - motion.from_y) * eased;
    monster.position.x = static_cast<int>(std::lround(motion.display_x));
    monster.position.y = static_cast<int>(std::lround(motion.display_y));
  }
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
  smooth_remote_monster_positions(state, dt_ms, state.session != nullptr);
  {
    // Advance the vector-art animation clocks: a shared breathe cycle plus
    // per-actor walk phase driven by how far each authoritative position
    // moved this frame (one full cycle per ~0.9 tile).
    state.breathe_phase = std::fmod(state.breathe_phase + dt_ms / 2400.0, 1.0);
    const auto advance = [&](const std::string& id, const verdigris::Vec2& pos) {
      auto& motion = state.motions[id];
      if (motion.has_last) {
        const double dx = static_cast<double>(pos.x - motion.last_pos.x);
        const double dy = static_cast<double>(pos.y - motion.last_pos.y);
        const double moved = std::sqrt(dx * dx + dy * dy);
        motion.walk_phase =
            std::fmod(motion.walk_phase + moved / (kTileUnits * 0.9), 1.0);
        const double target = moved > 0.5 ? 1.0 : 0.0;
        motion.moving += (target - motion.moving) *
                         std::min(1.0, dt_ms / 120.0);
      }
      motion.last_pos = pos;
      motion.has_last = true;
    };
    advance("player", state.world.player.position);
    for (const auto& monster : state.world.monsters)
      advance(monster.id, monster.position);
    if (state.motions.size() > 256) state.motions.clear();  // scene-change purge
  }
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
  if (state.gear_overlay) {
    state.quest_journal = false;
    state.minimap_mode = MinimapMode::Corner;
    show_hint(state, "Gear opened");
  }
}

void toggle_quest_journal(ClientState& state) {
  state.quest_journal = !state.quest_journal;
  if (state.quest_journal) {
    state.gear_overlay = false;
    state.character_pane = false;
    state.tree_pane = false;
    state.minimap_mode = MinimapMode::Corner;
    if (trade_pane_open(state) && state.session)
      state.session->submit(
          verdigris::client::ClientCommand::close_screen());
    show_hint(state, "Campaign journal opened");
  } else {
    show_hint(state, "Campaign journal closed");
  }
}

// TASK-0153: the one Escape contract for every screen. A dismissible pane
// (gear/inventory) consumes the first press and stays in the session; only
// with nothing open does Escape request exit via ClientState::quit_requested,
// which the window procedure turns into PostQuitMessage.
bool trade_pane_open(const ClientState& state) {
  if (!state.session) return false;
  const auto& model = state.session->model();
  return model.shop.open || model.bank.open || model.chart.open ||
         model.dialogue.open || model.forge.open;
}

bool pointer_ui_blocks_world(const ClientState& state, const RECT& bounds,
                             int x, int y) {
  if (state.screen == Screen::Chronicles || state.quest_journal ||
      state.tree_pane || trade_pane_open(state)) return true;
  const auto contains = [&](const HudRect& rect) {
    return x >= rect.x && x < rect.x + rect.w &&
           y >= rect.y && y < rect.y + rect.h;
  };
  if (state.gear_overlay && contains(gear_pane_rect(bounds.right, bounds.bottom)))
    return true;
  if (state.character_pane &&
      contains(character_pane_rect(bounds.right, bounds.bottom))) return true;
  return contains(minimap_rect(bounds.right, bounds.bottom, state.minimap_side)) ||
         contains(vital_orb_rect(bounds.right, bounds.bottom, false)) ||
         contains(vital_orb_rect(bounds.right, bounds.bottom, true));
}

void toggle_minimap_overlay(ClientState& state) {
  const bool opening = state.minimap_mode != MinimapMode::Overlay;
  state.minimap_mode = opening ? MinimapMode::Overlay : MinimapMode::Corner;
  if (opening) {
    // Tactical charting is the one broad world overlay. Dismiss narrower
    // inventory/dialogue surfaces so stacking never hides either contract.
    state.gear_overlay = false;
    state.quest_journal = false;
    state.character_pane = false;
    state.tree_pane = false;
    if (trade_pane_open(state) && state.session)
      state.session->submit(
          verdigris::client::ClientCommand::close_screen());
    show_hint(state, "Tactical chart opened");
  } else {
    show_hint(state, "Tactical chart closed");
  }
}

void adjust_minimap_zoom(ClientState& state, int direction) {
  const int before = state.minimap_zoom_step;
  state.minimap_zoom_step =
      std::clamp(state.minimap_zoom_step + direction, 0,
                 kMinimapSettingSteps - 1);
  if (state.minimap_zoom_step == before && direction != 0) {
    show_hint(state, direction > 0 ? "Map zoom is at maximum"
                                   : "Map zoom is at minimum");
    return;
  }
  save_minimap_preferences(state);
  show_hint(state, "Map zoom " +
                       std::to_string(
                           minimap_zoom_percent(state.minimap_zoom_step)) +
                       "%");
}

void adjust_minimap_opacity(ClientState& state, int direction) {
  const int before = state.minimap_opacity_step;
  state.minimap_opacity_step =
      std::clamp(state.minimap_opacity_step + direction, 0,
                 kMinimapSettingSteps - 1);
  if (state.minimap_opacity_step == before && direction != 0) {
    show_hint(state, direction > 0 ? "Map opacity is at maximum"
                                   : "Map opacity is at minimum");
    return;
  }
  save_minimap_preferences(state);
  show_hint(state, "Map opacity " +
                       std::to_string(minimap_opacity_percent(
                           state.minimap_opacity_step)) +
                       "%");
}

void swap_minimap_side(ClientState& state) {
  state.minimap_side = state.minimap_side == MinimapSide::Left
                           ? MinimapSide::Right
                           : MinimapSide::Left;
  save_minimap_preferences(state);
  show_hint(state, state.minimap_side == MinimapSide::Left
                       ? "Corner map moved left"
                       : "Corner map moved right");
}

void handle_escape_key(ClientState& state) {
  if (state.chronicles_naming != ChronicleNamingMode::None) {
    cancel_chronicle_naming(state);
    return;
  }
  if (trade_pane_open(state)) {
    state.session->submit(verdigris::client::ClientCommand::close_screen());
    state.trade_selected = 0;
    return;
  }
  if (state.minimap_mode == MinimapMode::Overlay) {
    state.minimap_mode = MinimapMode::Corner;
    show_hint(state, "Tactical chart closed");
    return;
  }
  if (state.quest_journal) {
    state.quest_journal = false;
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
    case WM_CHAR:
      if (state && state->startup_creation) {
        if (state->chronicles_ignore_next_char) { state->chronicles_ignore_next_char=false; return 0; }
        if (state->creation_focus!=0 || state->creation_wait!=ClientState::CreationWait::None) return 0;
      }
      if (state && state->screen == Screen::Chronicles &&
          state->chronicles_naming != ChronicleNamingMode::None) {
        handle_chronicle_character(*state, static_cast<unsigned int>(wparam));
        InvalidateRect(window, nullptr, FALSE);
        return 0;
      }
      break;
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
        if (state->startup_creation) { cancel_startup_creation(*state); break; }
        if (state->title_enabled && !state->title_open && state->screen == Screen::Chronicles &&
            state->chronicles_naming == ChronicleNamingMode::None) {
          state->title_open = true;
          break;
        }
        // TASK-0153: dismiss an open pane first; exit only on a bare Escape.
        handle_escape_key(*state);
        if (state->quit_requested) PostQuitMessage(0);
        break;
      }
      if (handle_creation_key(*state,wparam)) break;
      if (state->title_open) {
        if (wparam == VK_UP) state->title_selected = std::max(0,state->title_selected-1);
        if (wparam == VK_DOWN) state->title_selected = std::min(2,state->title_selected+1);
        if (state->title_selected==1 && !title_continue_scion(*state) &&
            (wparam==VK_UP || wparam==VK_DOWN)) state->title_selected=wparam==VK_UP?0:2;
        if (wparam == VK_LEFT) state->title_orbit.yaw -= .1f;
        if (wparam == VK_RIGHT) state->title_orbit.yaw += .1f;
        if (wparam == VK_HOME) state->title_orbit.reset();
        if (wparam == VK_OEM_PLUS || wparam == VK_ADD) state->title_orbit.wheel(WHEEL_DELTA);
        if (wparam == VK_OEM_MINUS || wparam == VK_SUBTRACT) state->title_orbit.wheel(-WHEEL_DELTA);
        if (wparam == VK_RETURN) {
          activate_title_action(*state,state->title_selected);
          if (state->chronicles_naming != ChronicleNamingMode::None) state->chronicles_ignore_next_char=true;
        }
        break;
      }
      if (state->screen == Screen::Chronicles && state->session) {
        if (state->chronicles_naming != ChronicleNamingMode::None &&
            wparam == 'V' && (GetKeyState(VK_CONTROL) & 0x8000)) {
          paste_chronicle_name(window,*state);
          break;
        }
        handle_chronicles_key(*state, wparam);
        break;
      }
      if (wparam == VK_TAB) {
        toggle_minimap_overlay(*state);
        break;
      }
      if (wparam == 'J') {
        toggle_quest_journal(*state);
        break;
      }
      if (state->quest_journal) break;
      if (wparam == 'M' && (GetKeyState(VK_SHIFT) & 0x8000)) {
        swap_minimap_side(*state);
        break;
      }
      if (wparam == VK_OEM_4) {
        adjust_minimap_zoom(*state, -1);
        break;
      }
      if (wparam == VK_OEM_6) {
        adjust_minimap_zoom(*state, 1);
        break;
      }
      if (wparam == VK_OEM_MINUS || wparam == VK_SUBTRACT) {
        adjust_minimap_opacity(*state, -1);
        break;
      }
      if (wparam == VK_OEM_PLUS || wparam == VK_ADD) {
        adjust_minimap_opacity(*state, 1);
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
        if (wparam == 'T' && state->session->model().forge.open) {
          for (const auto& hit : state->trade_row_hits) {
            if (hit.kind == 6 && hit.index == state->trade_selected) {
              activate_trade_row(*state, hit);
              break;
            }
          }
        }
        if (wparam == VK_UP || wparam == VK_DOWN || wparam == VK_RETURN ||
            (wparam == 'T' && state->session->model().forge.open)) {
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
          const bool house_steward =
              std::find(nearest->services.begin(), nearest->services.end(),
                        std::string("house_investment")) != nearest->services.end();
          std::string verb = house_steward
                                 ? "examine"
                                 : nearest->actions.empty() ? "examine"
                                                            : nearest->actions.front();
          if (!house_steward) {
            for (const char* preferred : {"talk", "trade", "bank"}) {
              if (std::find(nearest->actions.begin(), nearest->actions.end(), preferred) !=
                  nearest->actions.end()) { verb = preferred; break; }
            }
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
        state->minimap_mode = MinimapMode::Corner;
        state->quest_journal = false;
        state->character_pane = !state->character_pane;
      }
      if (wparam == 'M' && !(GetKeyState(VK_SHIFT) & 0x8000) &&
          state->audio_sink) {
        state->audio_sink->set_muted(!state->audio_sink->muted());
        show_hint(*state, state->audio_sink->muted() ? "Sound muted"
                                                     : "Sound on");
      }
      if (wparam == 'P') {
        state->minimap_mode = MinimapMode::Corner;
        state->quest_journal = false;
        state->tree_pane = !state->tree_pane;
      }
      // Only the open gear pane needs a fresh view here; a bare sync on
      // every auto-repeating WASD keydown is per-input work the frame loop
      // pays for.
      if (state->gear_overlay) sync_world(*state);
      if (state->gear_overlay && !state->world.carried.empty()) {
        if (wparam == VK_UP) move_inventory_selection(*state, 0, -1);
        if (wparam == VK_DOWN) move_inventory_selection(*state, 0, 1);
        if (wparam == VK_LEFT) move_inventory_selection(*state, -1, 0);
        if (wparam == VK_RIGHT) move_inventory_selection(*state, 1, 0);
        if (wparam == VK_RETURN) equip_selected(*state);
        if (wparam == 'V') rechart_selected_tablet(*state);
        if (wparam == 'U' && state->simulation) {
          state->simulation->dispatch(verdigris::Command::unequip());
          show_hint(*state, "Weapon unequipped");
        }
      }
      if (wparam >= '1' && wparam <= '9' && state->session && state->gear_overlay) {
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
        if (state->title_open) {
          // Constant-time input bookkeeping only; rendering stays on timer.
          state->title_orbit.drag(state->mouse);
          if (!state->title_orbit.dragging)
            for (size_t i=0;i<state->title_action_hits.size();++i)
              if (PtInRect(&state->title_action_hits[i],state->mouse)) state->title_selected=static_cast<int>(i);
        } else if (state->screen == Screen::Chronicles &&
            state->chronicles_naming == ChronicleNamingMode::None) {
          for (const auto& hit : state->chronicles_action_hits) {
            if (state->mouse.x >= hit.rect.left &&
                state->mouse.x < hit.rect.right &&
                state->mouse.y >= hit.rect.top &&
                state->mouse.y < hit.rect.bottom) {
              state->chronicles_selected = hit.index;
              break;
            }
          }
        }
      }
      break;
    case WM_MOUSEWHEEL:
      if (state) {
        if (state->startup_creation) break;
        const int delta = GET_WHEEL_DELTA_WPARAM(wparam);
        if (state->title_open) { state->title_orbit.wheel(delta); break; }
        if (state->minimap_mode == MinimapMode::Overlay) {
          adjust_minimap_zoom(*state, delta > 0 ? 1 : -1);
        } else {
          const double factor = delta > 0 ? 1.1 : 1.0 / 1.1;
          RECT zoom_bounds;
          GetClientRect(window, &zoom_bounds);
          const double zf = zoom_height_factor(static_cast<int>(zoom_bounds.bottom));
          state->camera.zoom =
              std::clamp(state->camera.zoom * factor, kCameraMinZoom * zf,
                         kCameraMaxZoom * zf);
        }
      }
      break;
    case WM_LBUTTONDOWN:
      if (state) {
        if (state->startup_creation) {
          const POINT point{GET_X_LPARAM(lparam),GET_Y_LPARAM(lparam)};
          if (PtInRect(&state->creation_back,point)) activate_creation_control(*state,3);
          else if (PtInRect(&state->creation_confirm,point)) activate_creation_control(*state,2);
          else if (state->creation_wait==ClientState::CreationWait::None && PtInRect(&state->creation_input,point)) state->creation_focus=0;
          else if (PtInRect(&state->creation_oath,point)) { state->creation_focus=1; activate_creation_control(*state,1); }
        } else if (state->title_open) {
          const POINT point{GET_X_LPARAM(lparam),GET_Y_LPARAM(lparam)};
          bool menu=false;
          for (size_t i=0;i<state->title_action_hits.size();++i)
            if (PtInRect(&state->title_action_hits[i],point)) {
              activate_title_action(*state,static_cast<int>(i)); menu=true; break;
            }
          if (!menu) { state->title_orbit.previous=point; state->title_orbit.dragging=true; SetCapture(window); }
        } else if (state->screen == Screen::Chronicles) {
          activate_chronicle_at(*state, GET_X_LPARAM(lparam),
                               GET_Y_LPARAM(lparam));
        } else if (trade_pane_open(*state)) {
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
          const int mx = GET_X_LPARAM(lparam);
          const int my = GET_Y_LPARAM(lparam);
          if (state->rechart_tablet_hit_valid &&
              mx >= state->rechart_tablet_hit.left &&
              mx < state->rechart_tablet_hit.right &&
              my >= state->rechart_tablet_hit.top &&
              my < state->rechart_tablet_hit.bottom)
            rechart_selected_tablet(*state);
          else
            activate_inventory_at(*state, mx, my);
        } else {
          RECT click_bounds;
          GetClientRect(window, &click_bounds);
          if (pointer_ui_blocks_world(*state, click_bounds,
                                      GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)))
            break;
          if (try_world_click(*state, click_bounds, GET_X_LPARAM(lparam),
                              GET_Y_LPARAM(lparam)))
            break;
          // Route through dispatch_skill so LMB gets the same instant
          // swing-arc feedback as the Q/E/R keys — the primary attack was
          // the one input with no animation at all.
          static constexpr SkillInfo kPrimaryStrike{'\0', "Strike",
                                                    verdigris::ActionType::Melee};
          dispatch_skill(*state, kPrimaryStrike);
        }
      }
      break;
    case WM_LBUTTONUP:
      if (state && state->title_orbit.dragging) { state->title_orbit.dragging=false; ReleaseCapture(); }
      break;
    case WM_CAPTURECHANGED:
    case WM_KILLFOCUS:
      if (state) { state->title_orbit.dragging=false; state->w=state->a=state->s=state->d=false; }
      break;
    case WM_RBUTTONDOWN:
      if (state) {
        if (state->title_open || state->startup_creation) break;
        RECT click_bounds;
        GetClientRect(window, &click_bounds);
        if (!pointer_ui_blocks_world(*state, click_bounds,
                                     GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)))
          dispatch_dash(*state);
      }
      break;
    case WM_SIZE:
      if (state) {
        state->creation_input=state->creation_oath=state->creation_confirm=state->creation_back=RECT{};
        state->title_action_hits.clear();
        state->inventory_hits.clear();
        state->rechart_tablet_hit_valid = false;
      }
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

bool reference_present(ClientState& state, int width, int height,
                       const std::string& png_path);

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
  scenario_check(weapon && weapon->label == "(unarmed)",
                 "loot-to-bank: loadout reports unarmed before equip");

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

int scenario_combat_cadence() {
  ClientState state;
  load_billboards(state.billboards);
  state.world.route_id = "instance:dungeon:clearings";
  state.world.theme = "dungeon";
  state.world.house_name = "House Verdigris";
  state.world.scion_name = "Mara";
  state.world.player.id = "combo-scion";
  state.world.player.position = {20 * static_cast<int>(kTileUnits),
                                 20 * static_cast<int>(kTileUnits)};
  state.world.player.facing = {1, 0};
  state.world.player.life = 92;
  state.world.player.life_max = 100;
  state.world.player.resource = 42;
  state.world.player.resource_max = 50;
  state.world.player.level = 7;
  state.world.player.alive = true;
  state.world.player.cooldown_ticks = 10;
  state.world.player.cooldown_total_ticks = 11;
  state.world.player.combo_step = 3;
  state.world.player.combo_window_ticks = 18;
  WorldActor foe;
  foe.id = "combo-foe";
  foe.name = "Ashen Guard";
  foe.kind = "ashen-guard";
  foe.position = {state.world.player.position.x +
                      verdigris::world_scale::kMeleeRange,
                  state.world.player.position.y};
  foe.life = 68;
  foe.life_max = 100;
  foe.alive = true;
  state.world.monsters.push_back(foe);
  state.camera.x = static_cast<double>(state.world.player.position.x);
  state.camera.y = static_cast<double>(state.world.player.position.y);
  state.camera.zoom = kCameraDefaultZoom * zoom_height_factor(768);

  verdigris::client::PresentationFx fx;
  verdigris::client::PresentationEvent hit;
  hit.type = verdigris::client::PresentationEventType::DamageApplied;
  hit.actor_id = foe.id;
  hit.text = "outgoing";
  hit.value = 32;
  hit.style = "slash";
  hit.combo_step = 3;
  hit.combo_window_ms = 900;
  hit.stagger_ms = 700;
  verdigris::client::apply_presentation_event(fx, state.world, hit, 1);
  state.effects = std::move(fx.effects);

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false, "combat-cadence: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string capture_path =
      capture_dir + "\\combat-cadence-finisher-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, capture_path),
                 "combat-cadence: authoritative finisher frame captured");
  std::printf("    capture: %s\n", capture_path.c_str());

  bool finisher_ring = false;
  bool finisher_damage = false;
  bool cadence_hud = false;
  bool radial_cooldown = false;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Impact &&
        item.label == phase_a::kComboFinisherLabel) finisher_ring = true;
    if (item.op == render::Op::Damage && item.label == "finisher:slash")
      finisher_damage = true;
    if (item.op == render::Op::Quickbar &&
        item.label == "combo-cadence:3:18") cadence_hud = true;
    if (item.op == render::Op::Quickbar &&
        item.label.rfind("cooldown-radial:", 0) == 0)
      radial_cooldown = std::abs(item.radius - 10.0 / 11.0) < 0.0001;
  }
  scenario_check(finisher_ring && finisher_damage,
                 "combat-cadence: third beat has a distinct ring and number");
  scenario_check(cadence_hud,
                 "combat-cadence: quickbar mirrors all three authoritative pips");
  scenario_check(radial_cooldown,
                 "combat-cadence: radial clock uses the authoritative finisher duration");
  struct ClockSample { int remaining; int total; double fraction; };
  for (const auto sample : {ClockSample{8, 8, 1.0}, ClockSample{4, 8, 0.5},
                            ClockSample{0, 8, 0.0}, ClockSample{3, 0, 1.0}}) {
    state.world.player.cooldown_ticks = sample.remaining;
    state.world.player.cooldown_total_ticks = sample.total;
    reference_present(state, 1366, 768, "");
    int wipes = 0;
    bool ratios_match = true;
    for (const auto& item : state.render_list) {
      if (item.op != render::Op::Quickbar ||
          item.label.rfind("cooldown-radial:", 0) != 0) continue;
      ++wipes;
      ratios_match = ratios_match && std::abs(item.radius - sample.fraction) < 0.0001;
    }
    scenario_check(ratios_match && wipes == (sample.remaining ? 3 : 0),
                   "combat-cadence: full, half, ready and unknown-duration clocks are honest");
  }
  return 0;
}

int scenario_monster_pressure_roles() {
  ClientState state;
  load_billboards(state.billboards);
  state.world.route_id = "instance:marsh:clearings";
  state.world.theme = "marsh";
  state.world.house_name = "House Verdigris";
  state.world.scion_name = "Mara";
  state.world.player.id = "role-scion";
  state.world.player.position = {20 * static_cast<int>(kTileUnits),
                                 20 * static_cast<int>(kTileUnits)};
  state.world.player.facing = {1, 0};
  state.world.player.life = 78;
  state.world.player.life_max = 100;
  state.world.player.resource = 36;
  state.world.player.resource_max = 50;
  state.world.player.level = 8;
  state.world.player.alive = true;
  WorldActor spitter;
  spitter.id = "role-spitter";
  spitter.name = "Bog Spitter";
  spitter.kind = "marsh-ranged";
  spitter.behaviour = "ranged";
  spitter.position = {16 * static_cast<int>(kTileUnits),
                      19 * static_cast<int>(kTileUnits)};
  spitter.life = 28;
  spitter.life_max = 36;
  spitter.alive = true;
  state.world.monsters.push_back(spitter);
  WorldActor ghast;
  ghast.id = "role-ghast";
  ghast.name = "Mire Ghast";
  ghast.kind = "marsh-melee";
  ghast.behaviour = "melee";
  ghast.position = {22 * static_cast<int>(kTileUnits),
                    19 * static_cast<int>(kTileUnits)};
  ghast.life = 19;
  ghast.life_max = 42;
  ghast.alive = true;
  state.world.monsters.push_back(ghast);
  WorldActor shaman;
  shaman.id = "role-shaman";
  shaman.name = "Rot Shaman";
  shaman.kind = "marsh-buffer";
  shaman.behaviour = "buffer";
  shaman.position = {24 * static_cast<int>(kTileUnits),
                     21 * static_cast<int>(kTileUnits)};
  shaman.life = 30;
  shaman.life_max = 30;
  shaman.alive = true;
  state.world.monsters.push_back(shaman);
  state.camera.x = static_cast<double>(state.world.player.position.x);
  state.camera.y = static_cast<double>(state.world.player.position.y);
  state.camera.zoom = kCameraDefaultZoom * zoom_height_factor(768);

  const int move_from_x = state.world.monsters.front().position.x;
  smooth_remote_monster_positions(state, 0.0, true);
  state.world.monsters.front().position.x += static_cast<int>(kTileUnits);
  state.world.monsters.front().move_duration_ms = 400;
  smooth_remote_monster_positions(state, 200.0, true);
  const int interpolated_x = state.world.monsters.front().position.x;
  const bool movement_smoothed = interpolated_x > move_from_x &&
      interpolated_x < move_from_x + static_cast<int>(kTileUnits);

  verdigris::client::PresentationFx fx;
  verdigris::client::PresentationEvent warning;
  warning.type = verdigris::client::PresentationEventType::Telegraph;
  warning.actor_id = spitter.id;
  warning.text = spitter.name + " ranged:volley";
  warning.value = 800;
  warning.has_position = true;
  warning.x = 20.0;
  warning.y = 20.0;
  warning.radius = 1;
  verdigris::client::apply_presentation_event(fx, state.world, warning, 1);
  verdigris::client::PresentationEvent mend;
  mend.type = verdigris::client::PresentationEventType::HealingApplied;
  mend.actor_id = ghast.id;
  mend.text = shaman.name;
  mend.value = 7;
  verdigris::client::apply_presentation_event(fx, state.world, mend, 1);
  state.effects = std::move(fx.effects);
  state.telegraphs = std::move(fx.telegraphs);

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "monster-pressure-roles: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string capture_path =
      capture_dir + "\\monster-pressure-roles-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, capture_path),
                 "monster-pressure-roles: production role frame captured");
  std::printf("    capture: %s\n", capture_path.c_str());
  bool volley = false;
  bool support = false;
  bool healing = false;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Telegraph && item.label == "volley") volley = true;
    if (item.op == render::Op::WarCry &&
        item.label == phase_a::kSupportMendLabel) support = true;
    if (item.op == render::Op::Damage && item.label == "healing" &&
        item.value == 7) healing = true;
  }
  scenario_check(volley && support && healing,
                 "monster-pressure-roles: volley reticle and mend feedback render");
  scenario_check(movement_smoothed,
                 "monster-pressure-roles: accepted monster step renders between tiles");
  scenario_check(telegraph_avoids_hud(state.render_list, RECT{0, 0, 1366, 768}),
                 "monster-pressure-roles: warning respects HUD safe zones");
  return 0;
}

int scenario_warden_disciplines() {
  ClientState state;
  load_billboards(state.billboards);
  state.world.route_id = "instance:campaign:warden-conclave";
  state.world.theme = "crypt";
  state.world.house_name = "House Verdigris";
  state.world.scion_name = "Mara";
  state.world.player.id = "warden-scion";
  state.world.player.position = {20 * static_cast<int>(kTileUnits),
                                 20 * static_cast<int>(kTileUnits)};
  state.world.player.facing = {1, 0};
  state.world.player.life = 82;
  state.world.player.life_max = 100;
  state.world.player.resource = 41;
  state.world.player.resource_max = 50;
  state.world.player.level = 18;
  state.world.player.alive = true;
  state.world.tick = 9;

  const auto add_warden = [&](const char* id, const char* name,
                              int tile_x, int tile_y, const char* kind) {
    WorldActor warden;
    warden.id = id;
    warden.name = name;
    warden.kind = kind;
    warden.behaviour = "boss";
    warden.position = {tile_x * static_cast<int>(kTileUnits),
                       tile_y * static_cast<int>(kTileUnits)};
    warden.life = 420;
    warden.life_max = 600;
    warden.alive = true;
    state.world.monsters.push_back(std::move(warden));
  };
  add_warden("warden-tin", "Hearthless Bell", 15, 17, "dungeon-elite");
  add_warden("warden-salt", "Flood-Sworn Vicar", 24, 17, "marsh-elite");
  add_warden("warden-chalk", "Ossuary Regent", 16, 24, "crypt-elite");
  add_warden("warden-copper", "Cinder Testament", 24, 24, "forge-elite");
  state.camera.x = static_cast<double>(state.world.player.position.x);
  state.camera.y = static_cast<double>(state.world.player.position.y);
  state.camera.zoom = kCameraMinZoom;

  verdigris::client::PresentationFx fx;
  const auto warn = [&](const char* actor, const char* action,
                        const char* shape, const char* channel, int tile_x,
                        int tile_y, int radius, int inner_radius,
                        int windup_ms) {
    verdigris::client::PresentationEvent event;
    event.type = verdigris::client::PresentationEventType::Telegraph;
    event.actor_id = actor;
    event.action_id = action;
    event.telegraph_shape = shape;
    event.damage_channel = channel;
    event.value = windup_ms;
    event.has_position = true;
    event.x = static_cast<double>(tile_x);
    event.y = static_cast<double>(tile_y);
    event.radius = radius;
    event.inner_radius = inner_radius;
    verdigris::client::apply_presentation_event(fx, state.world, event, 1);
  };
  warn("warden-tin", "boss:stonefall", "circle", "physical", 17, 18, 1,
       0, 900);
  warn("warden-salt", "boss:tidal-mark", "circle", "river", 23, 18, 2,
       0, 1150);
  warn("warden-chalk", "boss:grave-ring", "ring", "physical", 19, 21, 4,
       2, 1300);
  warn("warden-copper", "boss:ember-crucible", "circle", "ember", 22, 22,
       2, 0, 750);
  state.telegraphs = std::move(fx.telegraphs);

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "warden-disciplines: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string capture_path =
      capture_dir + "\\warden-disciplines-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, capture_path),
                 "warden-disciplines: production combat frame captured");
  std::printf("    capture: %s\n", capture_path.c_str());

  std::unordered_map<std::string, double> warnings;
  for (const auto& item : state.render_list)
    if (item.op == render::Op::Telegraph) warnings[item.label] = item.radius;
  scenario_check(warnings.count("stonefall") == 1 &&
                     warnings.count("tidal-mark") == 1 &&
                     warnings.count("grave-ring") == 1 &&
                     warnings.count("ember-crucible") == 1,
                 "warden-disciplines: all four authored warnings render distinctly");
  scenario_check(warnings["grave-ring"] > warnings["tidal-mark"] &&
                     warnings["tidal-mark"] > warnings["stonefall"],
                 "warden-disciplines: authoritative warning scale remains legible");
  scenario_check(telegraph_avoids_hud(state.render_list,
                                      RECT{0, 0, 1366, 768}),
                 "warden-disciplines: warnings respect Framekit HUD safe zones");
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
  bool saw_ready_icon = false;
  const char* conn_label =
      verdigris::client::connection_state_label(state.session->connection_state());
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Hud &&
        item.label == std::string("connection ") + conn_label)
      saw_conn = true;
    if (item.op == render::Op::Hud && item.label == "connection-ready-icon")
      saw_ready_icon = true;
  }
  scenario_check(saw_conn, "remote-render-list: connection chip uses connection_state_label");
  scenario_check(saw_ready_icon,
                 "remote-render-list: ready connection is an icon without redundant text");

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
    advance_startup_creation(state);
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
    if (state.chronicles_naming != ChronicleNamingMode::None)
      confirm_chronicle_naming(state);
    return;
  }
}

void scenario_type_chronicle_name(ClientState& state,
                                  const std::string& name) {
  for (unsigned char ch : name) handle_chronicle_character(state, ch);
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

  // 2) Found the House through the real text-entry seam. A too-short name
  // stays modal, then the corrected owner-authored name crosses the wire.
  handle_chronicles_key(state, 'F');
  scenario_check(state.chronicles_naming == ChronicleNamingMode::House,
                 "founding: F opens the House naming ceremony");
  handle_chronicle_character(state, 'f');
  scenario_check(state.chronicles_name_input.empty(),
                 "founding: the F shortcut never leaks into the name field");
  handle_escape_key(state);
  scenario_check(state.chronicles_naming == ChronicleNamingMode::None &&
                     !state.quit_requested,
                 "founding: Escape cancels naming without quitting");
  handle_chronicles_key(state, 'F');
  handle_chronicle_character(state, 'f');
  scenario_type_chronicle_name(state, "A");
  handle_chronicle_character(state, '\r');
  scenario_present(state);
  scenario_check(
      render_list_has(state, render::Op::Chronicles, "naming:error:"),
      "founding: invalid House name stays visible and actionable");
  handle_chronicle_character(state, '\b');
  scenario_type_chronicle_name(state, "Emberwake");
  const std::string founding_png =
      capture_dir + "\\founding-house-name-960x600.png";
  scenario_check(reference_present(state, 960, 600, founding_png),
                 "founding: Framekit naming ceremony capture written");
  handle_chronicle_character(state, '\r');
  const bool house_ok = chronicles_pump(state, 250, [&] {
    const auto& houses = state.session->model().chronicle.houses;
    return houses.size() == 1 && houses.front().name == "Emberwake";
  });
  scenario_check(house_ok,
                 "front door: owner-authored House name renders in the roster");
  scenario_present(state);
  scenario_check(render_list_has(state, render::Op::Chronicles, "house "),
                 "front door: the new House is named on screen");
  scenario_check(render_list_has(state, render::Op::Chronicles, "action:create-scion"),
                 "front door: naming a Scion is offered");

  // 3) Name the first Scion through the same production character handler;
  // the oath field starts soft.
  handle_chronicles_key(state, 'C');
  scenario_check(state.chronicles_naming == ChronicleNamingMode::Scion,
                 "founding: C opens the Scion naming ceremony");
  handle_chronicle_character(state, 'c');
  scenario_type_chronicle_name(state, "Ilyra");
  handle_chronicle_character(state, '\r');
  const bool scion_ok = chronicles_pump(state, 250, [&] {
    const auto& houses = state.session->model().chronicle.houses;
    return !houses.empty() && houses.front().scions.size() == 1 &&
           houses.front().scions.front().name == "Ilyra";
  });
  scenario_check(scion_ok,
                 "front door: owner-authored Scion name joins the roster");
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

// Compact contract for the unfinished information-density pass: the XP strip
// renders the supplied authoritative fraction, hover exposes a world actor,
// and modal panes suppress world tooltips instead of drawing through them.
int scenario_hud_information() {
  ClientState state;
  scenario_begin(state);
  scenario_follow_camera(state);
  sync_world(state);
  state.world.xp_present = true;
  state.world.xp_fraction = 0.42;

  constexpr int width = 960;
  constexpr int height = 600;
  HDC dc = CreateCompatibleDC(nullptr);
  HBITMAP bitmap = CreateCompatibleBitmap(dc, width, height);
  scenario_check(bitmap != nullptr,
                 "hud-information: presentation surface allocated");
  if (!bitmap) {
    DeleteDC(dc);
    return scenario_failures;
  }
  HGDIOBJ old = SelectObject(dc, bitmap);
  RECT bounds{0, 0, width, height};
  skin::set_ui_scale(hud_scale(height));
  SelectObject(dc, skin::font_body());
  render::List hud_ops;
  paint_xp_bar(state, dc, bounds, hud_ops);

  const render::Item* xp = nullptr;
  bool raster_xp = false;
  for (const auto& item : hud_ops)
    if (item.op == render::Op::Hud && item.label == "xp-bar")
      xp = &item;
    else if (item.label == "framekit-raster:xp-rail")
      raster_xp = true;
  scenario_check(xp && xp->value == 42,
                 "hud-information: XP strip records authoritative progress");
  scenario_check(raster_xp,
                 "hud-information: XP progress uses the authored WIZARD rail");

  scenario_check(!state.world.monsters.empty(),
                 "hud-information: seeded route supplies a hover target");
  if (!state.world.monsters.empty()) {
    const WorldActor& monster = state.world.monsters.front();
    const ScreenPoint base =
        project(state.camera, bounds, monster.position.x, monster.position.y);
    state.mouse.x = base.x;
    state.mouse.y =
        base.y - static_cast<int>(kTileUnits * 0.7 * base.scale);
    paint_hover_tooltip(state, dc, bounds, hud_ops);
  }
  bool saw_tooltip = false;
  bool raster_tooltip = false;
  for (const auto& item : hud_ops)
    if (item.op == render::Op::Hud && item.label.rfind("tooltip:", 0) == 0)
      saw_tooltip = true;
    else if (item.label == "framekit-raster:tooltip-frame")
      raster_tooltip = true;
  scenario_check(saw_tooltip,
                 "hud-information: world hover emits a readable tooltip");
  scenario_check(raster_tooltip,
                 "hud-information: hover uses the authored WIZARD tooltip frame");

  state.gear_overlay = true;
  render::List modal_rl;
  paint_hover_tooltip(state, dc, bounds, modal_rl);
  bool leaked_tooltip = false;
  for (const auto& item : modal_rl)
    if (item.op == render::Op::Hud && item.label.rfind("tooltip:", 0) == 0)
      leaked_tooltip = true;
  scenario_check(!leaked_tooltip,
                 "hud-information: modal pane suppresses world hover");

  SelectObject(dc, old);
  DeleteObject(bitmap);
  DeleteDC(dc);
  return 0;
}

int scenario_endgame_tablet_ui() {
  ClientState state;
  scenario_begin(state);
  load_billboards(state.billboards);
  scenario_follow_camera(state);
  state.simulation.reset();  // freeze the authored presentation fixture
  state.world.has_extraction = false;
  state.world.monsters.clear();
  state.world.endgame.present = true;
  state.world.endgame.unlocked = true;
  state.world.endgame.completed = 4;
  state.world.endgame.mastered = 11;
  state.world.endgame.mastery_total = 64;
  state.world.endgame.highest_tier = 6;
  state.world.endgame.ascent_chance_percent = 40;
  state.world.endgame.mastery_keys = {
      "barrow:1", "barrow:2", "barrow:3", "barrow:4",
      "reeds:1", "reeds:2", "reeds:3", "crown:1", "crown:2",
      "thorns:1", "thorns:2"};
  WorldCarriedItem tablet;
  tablet.id = "tablet-ui-1";
  tablet.name = "Tier 6 Crown Charted Tablet";
  tablet.expedition_map = true;
  tablet.map_tier = 6;
  tablet.map_goods_found_percent = 88;
  tablet.map_family = "Crown";
  tablet.map_objective_key = "crown:6";
  tablet.map_modifiers = {"Furious: monsters deal 33% more damage",
                          "Teeming: 5 additional foes"};
  state.world.carried = {std::move(tablet)};
  state.gear_overlay = true;
  state.selected_item = 0;
  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "endgame-tablet-ui: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string capture_path =
      capture_dir + "\\endgame-tablet-ui-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, capture_path),
                 "endgame-tablet-ui: readable UI evidence captured");
  std::printf("    capture: %s\n", capture_path.c_str());

  bool objective = false;
  bool map_row = false;
  bool mastery_preview = false;
  bool rechart_control = false;
  bool rechart_action = false;
  int modifiers = 0;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Hud && item.label.rfind("endgame: 4", 0) == 0)
      objective = true;
    if (item.op == render::Op::PaneItem &&
        item.label.find("Tier 6") != std::string::npos)
      map_row = true;
    if (item.op == render::Op::PaneStat &&
        item.label.rfind("map-modifier:", 0) == 0)
      ++modifiers;
    if (item.op == render::Op::PaneStat &&
        item.label.find("NEW MASTERY") != std::string::npos &&
        item.label.find("ascent 40%") != std::string::npos)
      mastery_preview = true;
    if (item.op == render::Op::PaneStat &&
        item.label.find("V rechart 50g") != std::string::npos)
      rechart_control = true;
    if (item.op == render::Op::PaneStat &&
        item.label == "gear-action:rechart")
      rechart_action = true;
  }
  scenario_check(objective,
                 "endgame-tablet-ui: town objective exposes the unlocked loop");
  scenario_check(map_row,
                 "endgame-tablet-ui: selected charted tablet renders as an item");
  scenario_check(modifiers == 2,
                 "endgame-tablet-ui: both authoritative risk clauses are inspectable");
  scenario_check(mastery_preview,
                 "endgame-tablet-ui: selection previews mastery and ascent sustain");
  scenario_check(rechart_control && rechart_action &&
                     state.rechart_tablet_hit_valid,
                 "endgame-tablet-ui: paid re-charting is keyboard and pointer discoverable");
  const HudRect* pane = nullptr;
  const HudRect* footer = nullptr;
  const HudRect* rechart = nullptr;
  for (const auto& entry : state.hud_rect_trace) {
    if (entry.first == "pane-frame") pane = &entry.second;
    if (entry.first == "pane-footer") footer = &entry.second;
    if (entry.first == "pane-rechart") rechart = &entry.second;
  }
  scenario_check(pane && footer &&
                     footer->x + footer->w <= pane->x + pane->w - 8,
                 "endgame-tablet-ui: contextual controls stay inside the pane");
  scenario_check(pane && rechart && rechart->x >= pane->x &&
                     rechart->x + rechart->w <= pane->x + pane->w &&
                     rechart->y >= pane->y &&
                     rechart->y + rechart->h <= pane->y + pane->h,
                 "endgame-tablet-ui: re-chart pointer target stays inside Framekit");

  state.world.has_extraction = true;
  reference_present(state, 1366, 768, "");
  bool expedition_seal = false;
  bool leaked_rechart_action = false;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::PaneStat &&
        item.label.find("Tablet sealed until Crossroads") != std::string::npos)
      expedition_seal = true;
    if (item.op == render::Op::PaneStat &&
        item.label == "gear-action:rechart")
      leaked_rechart_action = true;
  }
  scenario_check(expedition_seal && !leaked_rechart_action &&
                     !state.rechart_tablet_hit_valid,
                 "endgame-tablet-ui: re-charting disables outside the Crossroads");

  state.world.has_extraction = false;
  state.gear_overlay = false;
  state.quest_journal = true;
  const std::string mastery_capture_path =
      capture_dir + "\\endgame-mastery-board-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, mastery_capture_path),
                 "endgame-tablet-ui: Wayfinder Mastery board captured");
  std::printf("    capture: %s\n", mastery_capture_path.c_str());
  int family_rows = 0;
  bool mastery_board = false;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Hud &&
        item.label.rfind("endgame-family:", 0) == 0)
      ++family_rows;
    if (item.op == render::Op::Hud && item.label == "endgame-mastery:11/64")
      mastery_board = true;
  }
  scenario_check(mastery_board && family_rows == 4,
                 "endgame-tablet-ui: ledger renders all 64 family-tier objectives");
  return 0;
}

int scenario_character_inventory_diptych() {
  ClientState state;
  load_billboards(state.billboards);
  state.world.route_id = "town:verdigris";
  state.world.theme = "town";
  state.world.house_name = "House Ashwake";
  state.world.scion_name = "Ilyra";
  state.world.player.id = "scion-ilyra";
  state.world.player.position = {20 * static_cast<int>(kTileUnits),
                                 20 * static_cast<int>(kTileUnits)};
  state.world.player.life = 112;
  state.world.player.life_max = 128;
  state.world.player.resource = 41;
  state.world.player.resource_max = 55;
  state.world.player.attack = 21;
  state.world.player.defense = 18;
  state.world.player.level = 17;
  state.world.player.bleed_chance = 24;
  state.world.player.reach_percent = 16;
  state.world.player.movement_speed_percent = 12;
  state.world.player.ember_resistance = 25;
  state.world.player.river_resistance = 40;
  state.world.player.alive = true;
  state.world.xp_present = true;
  state.world.xp_fraction = 0.63;
  state.world.stored_items = 28;
  state.world.stored_trophies = 9;

  const auto worn = [](const char* uuid, const char* art, const char* name,
                       const char* seat, int attack = 0) {
    WorldCarriedItem item;
    item.id = uuid;
    item.art_id = art;
    item.name = name;
    item.attack_bonus = attack;
    item.equipped = true;
    item.equip_seat = seat;
    item.equip_slot = seat;
    return item;
  };
  state.world.worn = {
      worn("wear-main", "bronze-dagger", "Oath-Cut Dagger", "right_hand", 9),
      worn("wear-head", "bronze-med-helm", "Bronze Warcrest", "head"),
      worn("wear-armor", "vessel-wrap", "Ashwake Wrap", "armor"),
      worn("wear-back", "hide-cape", "Roadworn Mantle", "back"),
      worn("wear-gloves", "bronze-gloves", "Riveted Grips", "gloves"),
      worn("wear-feet", "bronze-boots", "Wayfarer Boots", "feet"),
      worn("wear-belt", "hide-girdle", "Relic Girdle", "belt"),
      worn("wear-ring", "gold-ring", "House Signet", "ring"),
      worn("wear-neck", "garnet-amulet", "Garnet Oathstone", "necklace"),
  };

  const auto carried = [](const char* uuid, const char* art, const char* name,
                          int slot, int width, int height, int qty = 1) {
    WorldCarriedItem item;
    item.id = uuid;
    item.art_id = art;
    item.name = name;
    item.inventory_slot = slot;
    item.width = width;
    item.height = height;
    item.quantity = qty;
    return item;
  };
  WorldCarriedItem map = carried("tablet-7", "charted-tablet-crown",
                                 "Tier 7 Crown Tablet", 6, 2, 2);
  map.expedition_map = true;
  map.map_tier = 7;
  map.map_goods_found_percent = 74;
  map.map_family = "Crown";
  map.map_objective_key = "crown:7";
  map.map_modifiers = {"Furious: monsters deal 33% more damage",
                       "Teeming: 5 additional foes"};
  state.world.carried = {
      carried("pack-pike", "bronze-sword", "Bronze Boar Pike", 0, 2, 4),
      carried("pack-coins", "coins", "House Coins", 2, 1, 1, 86),
      carried("pack-knife", "bronze-dagger", "Curator Knife", 4, 1, 3),
      std::move(map),
      carried("pack-amulet", "garnet-amulet", "Loose Garnet", 10, 1, 1),
      carried("pack-shield", "wooden-shield", "Reed Shield", 31, 2, 2),
  };
  state.world.endgame.present = true;
  state.world.endgame.unlocked = true;
  state.world.endgame.mastery_total = 64;
  state.world.endgame.ascent_chance_percent = 38;
  state.selected_item = 3;

  const SpriteBitmap* runtime_framekit[] = {
      &state.billboards.fk_panel_ornate, &state.billboards.fk_banner,
      &state.billboards.fk_tooltip,      &state.billboards.fk_xp_rail,
      &state.billboards.fk_orb_life,     &state.billboards.fk_orb_resource,
      &state.billboards.fk_skill_strike, &state.billboards.fk_skill_thrust,
      &state.billboards.fk_skill_sweep,  &state.billboards.fk_skill_warcry,
      &state.billboards.fk_button,       &state.billboards.fk_chronicle_keyhole,
      &state.billboards.fk_chronicle_knocker,
      &state.billboards.fk_chronicle_laurel,
      &state.billboards.fk_chronicle_input,
  };
  int runtime_ready = 0;
  for (const SpriteBitmap* sprite : runtime_framekit)
    if (sprite->ready()) ++runtime_ready;
  scenario_check(runtime_ready == 15 &&
                     state.billboards.framekit_status ==
                         "art: WIZARD Framekit 15/15",
                 "character-inventory: all authored WIZARD runtime rasters decoded");
  reference_present(state, 1366, 768, "");
  bool objective_banner_drawn = false;
  bool status_button_drawn = false;
  for (const auto& item : state.render_list) {
    if (item.label == "framekit-raster:banner-winged")
      objective_banner_drawn = true;
    if (item.label == "framekit-raster:button-primary")
      status_button_drawn = true;
  }
  scenario_check(objective_banner_drawn && status_button_drawn,
                 "character-inventory: authored objective and status frames were drawn");
  state.gear_overlay = true;
  state.character_pane = true;

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "character-inventory: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  for (const auto size : {POINT{960, 600}, POINT{1366, 768}}) {
    const std::string capture_path =
        capture_dir + "\\character-inventory-diptych-" +
        std::to_string(size.x) + "x" + std::to_string(size.y) + ".png";
    scenario_check(reference_present(state, size.x, size.y, capture_path),
                   "character-inventory: Framekit diptych captured");
    std::printf("    capture: %s\n", capture_path.c_str());
    const HudRect expected_character = character_pane_rect(size.x, size.y);
    const HudRect expected_gear = gear_pane_rect(size.x, size.y);
    scenario_check(!hud_rects_overlap(expected_character, expected_gear),
                   "character-inventory: paper doll and backpack do not overlap");
    int seats = 0;
    int filled = 0;
    int footprints = 0;
    int skill_rasters = 0;
    bool ornate_panel = false;
    bool life_orb = false;
    bool resource_orb = false;
    bool xp_rail = false;
    bool bounded = true;
    const HudRect* grid = nullptr;
    for (const auto& trace : state.hud_rect_trace) {
      if (trace.first == "paperdoll-seat") {
        ++seats;
        bounded = bounded && trace.second.x >= expected_character.x &&
                  trace.second.y >= expected_character.y &&
                  trace.second.x + trace.second.w <=
                      expected_character.x + expected_character.w &&
                  trace.second.y + trace.second.h <=
                      expected_character.y + expected_character.h;
      }
      if (trace.first == "pane-grid") grid = &trace.second;
      if (trace.first == "pane-item-footprint") ++footprints;
    }
    for (const auto& item : state.render_list)
      if (item.op == render::Op::Hud &&
          item.label.rfind("paperdoll:", 0) == 0 && item.value == 1) {
        ++filled;
      } else if (item.label.rfind("framekit-skill:", 0) == 0) {
        ++skill_rasters;
      } else if (item.label == "framekit-raster:panel-tiled-fill") {
        ornate_panel = true;
      } else if (item.label == "framekit-raster:orb-life") {
        life_orb = true;
      } else if (item.label == "framekit-raster:orb-resource") {
        resource_orb = true;
      } else if (item.label == "framekit-raster:xp-rail") {
        xp_rail = true;
      }
    if (grid) {
      for (const auto& trace : state.hud_rect_trace)
        if (trace.first == "pane-item-footprint")
          bounded = bounded && trace.second.x >= grid->x &&
                    trace.second.y >= grid->y &&
                    trace.second.x + trace.second.w <= grid->x + grid->w &&
                    trace.second.y + trace.second.h <= grid->y + grid->h;
    } else {
      bounded = false;
    }
    scenario_check(seats == 11 && filled == 9,
                   "character-inventory: all WearSet seats and fills are explicit");
    scenario_check(footprints == 6 && bounded,
                   "character-inventory: all spatial footprints remain bounded");
    scenario_check(state.billboards.fk_panel_fill.ready() && ornate_panel &&
                       life_orb && resource_orb && xp_rail &&
                       skill_rasters == 4,
                   "character-inventory: tiled panel fill, authored HUD, orb, rail, and skill rasters were drawn");
  }
  for (const auto& trace : state.hud_rect_trace) {
    if (trace.first != "pane-item-footprint") continue;
    state.mouse.x = trace.second.x + trace.second.w / 2;
    state.mouse.y = trace.second.y + trace.second.h / 2;
    break;
  }
  reference_present(state, 1366, 768, "");
  scenario_check(state.selected_item == 0,
                 "character-inventory: footprint hover selects the exact item");
  move_inventory_selection(state, 1, 0);
  scenario_check(state.selected_item == 1,
                 "character-inventory: arrow navigation follows spatial neighbors");
  return 0;
}

int scenario_framekit_pane_tiling() {
  ClientState state;
  load_billboards(state.billboards);
  state.world.route_id = "town:verdigris";
  state.world.theme = "town";
  state.world.house_name = "House Ashwake";
  state.world.scion_name = "Ilyra";
  state.world.player.id = "scion-ilyra";
  state.world.player.alive = true;
  state.world.player.life = 100;
  state.world.player.life_max = 100;
  state.world.player.resource = 40;
  state.world.player.resource_max = 40;
  state.gear_overlay = true;

  scenario_check(state.billboards.fk_panel_fill.ready() &&
                     state.billboards.fk_panel_fill.width == 512 &&
                     state.billboards.fk_panel_fill.height == 512,
                 "framekit-pane-tiling: generated 512px panel surface decoded");

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "framekit-pane-tiling: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string capture_path =
      capture_dir + "\\framekit-pane-tiling-1920x1080.png";
  scenario_check(reference_present(state, 1920, 1080, capture_path),
                 "framekit-pane-tiling: large pane capture written");
  scenario_check(render_list_has(state, render::Op::Hud,
                                 "framekit-raster:panel-tiled-fill"),
                 "framekit-pane-tiling: tiled fill replaced stretched center");
  std::printf("    capture: %s\n", capture_path.c_str());
  return 0;
}

int scenario_vesselforge_active_properties() {
  ClientState state;
  load_billboards(state.billboards);
  state.world.route_id = "instance:marsh:clearings";
  state.world.theme = "marsh";
  state.world.house_name = "House Verdigris";
  state.world.scion_name = "Mara";
  state.world.player.id = "forge-scion";
  state.world.player.position = {20 * static_cast<int>(kTileUnits),
                                 20 * static_cast<int>(kTileUnits)};
  state.world.player.facing = {-1, 0};
  state.world.player.life = 94;
  state.world.player.life_max = 118;
  state.world.player.resource = 38;
  state.world.player.resource_max = 50;
  state.world.player.attack = 18;
  state.world.player.defense = 7;
  state.world.player.level = 12;
  state.world.player.alive = true;
  state.world.player.bleed_chance = 100;
  state.world.player.reach_percent = 16;
  state.world.player.movement_speed_percent = 25;
  state.world.player.ember_resistance = 25;
  state.world.player.river_resistance = 50;
  state.world.player.bond_attack_speed_ticks = 71;
  state.world.player.bond_movement_speed_ticks = 53;
  state.world.player.bond_last_stand_ready = true;

  WorldActor foe;
  foe.id = "forge-foe";
  foe.name = "Bog Spitter";
  foe.kind = "marsh-ranged";
  foe.behaviour = "ranged";
  foe.damage_channel = "river";
  foe.position = {16 * static_cast<int>(kTileUnits),
                  20 * static_cast<int>(kTileUnits)};
  foe.life = 54;
  foe.life_max = 80;
  foe.alive = true;
  foe.bleeding = true;
  state.world.monsters = {foe};

  WorldCarriedItem blade;
  blade.id = "vessel-macuahuitl";
  blade.name = "Mara's Grim Reckoning";
  blade.attack_bonus = 19;
  blade.equipped = true;
  blade.forge_lines = {"Hits cause Bleeding", "+16% increased Reach",
                       "BOND: Battle Rhythm - +18% Attack Speed for 4s after a Kill [Slaughter II]",
                       "AWAKENED: Last Stand - a killing blow instead leaves you at 1 Life, once per battle."};
  state.world.carried = {blade};
  state.world.stored_items = 7;
  state.world.stored_trophies = 3;
  state.gear_overlay = true;
  state.selected_item = 0;
  state.camera.x = static_cast<double>(state.world.player.position.x);
  state.camera.y = static_cast<double>(state.world.player.position.y);
  state.camera.zoom = kCameraDefaultZoom * zoom_height_factor(768);

  verdigris::client::PresentationFx fx;
  verdigris::client::PresentationEvent applied;
  applied.type = verdigris::client::PresentationEventType::DebuffApplied;
  applied.actor_id = foe.id;
  applied.text = "bleed";
  applied.value = 4;
  applied.duration_ms = 3000;
  verdigris::client::apply_presentation_event(fx, state.world, applied, 1);
  verdigris::client::PresentationEvent tick;
  tick.type = verdigris::client::PresentationEventType::DamageApplied;
  tick.actor_id = foe.id;
  tick.text = "outgoing";
  tick.value = 4;
  tick.style = "bleed";
  verdigris::client::apply_presentation_event(fx, state.world, tick, 2);
  verdigris::client::PresentationEvent bond;
  bond.type = verdigris::client::PresentationEventType::BondTriggered;
  bond.actor_id = state.world.player.id;
  bond.text = "Battle Rhythm";
  bond.style = "battle-rhythm";
  bond.value = 18;
  bond.duration_ms = 4000;
  verdigris::client::apply_presentation_event(fx, state.world, bond, 3);
  state.effects = std::move(fx.effects);

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "vesselforge-active: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string capture_path =
      capture_dir + "\\vesselforge-active-properties-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, capture_path),
                 "vesselforge-active: Framekit Bond combat frame captured");
  std::printf("    capture: %s\n", capture_path.c_str());

  int forge_lines = 0;
  bool forge_totals = false;
  bool forge_detail = false;
  bool bleed_apply = false;
  bool bleed_tick = false;
  bool persistent_bleed = false;
  bool dormant_claim = false;
  bool bond_pulse = false;
  int bond_states = 0;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::PaneStat &&
        item.label.rfind("forge-line:", 0) == 0) {
      ++forge_lines;
      if (item.label.find("Dormant") != std::string::npos) dormant_claim = true;
    }
    if (item.op == render::Op::PaneStat &&
        item.label.rfind("FORGE  SPD +0%", 0) == 0)
      forge_totals = true;
    if (item.op == render::Op::PaneStat &&
        item.label.find("BLEED 100%") != std::string::npos &&
        item.label.find("WARDS E25 R50") != std::string::npos)
      forge_detail = true;
    if (item.op == render::Op::Impact && item.label == phase_a::kBleedApplyLabel)
      bleed_apply = true;
    if (item.op == render::Op::Damage && item.label == phase_a::kBleedDamageLabel)
      bleed_tick = true;
    if (item.op == render::Op::TargetFlash && item.label == "bleeding")
      persistent_bleed = true;
    if (item.op == render::Op::WarCry &&
        item.label == "bond-trigger:battle-rhythm")
      bond_pulse = true;
    if (item.op == render::Op::Quickbar &&
        item.label.rfind("bond-state:", 0) == 0)
      ++bond_states;
  }
  scenario_check(forge_lines == 4 && forge_totals && forge_detail &&
                     !dormant_claim,
                 "vesselforge-active: Framekit explains Brands, Bonds, awakening, and totals");
  scenario_check(bleed_apply && bleed_tick && persistent_bleed && bond_pulse &&
                     bond_states == 3,
                 "vesselforge-active: combat ailments, Bond trigger, and live states remain distinct");
  return 0;
}

int scenario_vesselforge_final_implicits() {
  ClientState state;
  load_billboards(state.billboards);
  state.world.route_id = "instance:dungeon:clearings";
  state.world.theme = "dungeon";
  state.world.house_name = "House Emberwake";
  state.world.scion_name = "Ilyra";
  state.world.player.id = "implicit-scion";
  state.world.player.position = {20 * static_cast<int>(kTileUnits),
                                 20 * static_cast<int>(kTileUnits)};
  state.world.player.facing = {1, 0};
  state.world.player.life = 112;
  state.world.player.life_max = 120;
  state.world.player.resource = 44;
  state.world.player.resource_max = 55;
  state.world.player.attack = 20;
  state.world.player.defense = 8;
  state.world.player.level = 14;
  state.world.player.alive = true;
  state.world.player.attack_speed_percent = 8;

  WorldActor foe;
  foe.id = "implicit-warden";
  foe.name = "Copper Bulwark";
  foe.kind = "dungeon-melee";
  foe.behaviour = "melee";
  foe.position = {25 * static_cast<int>(kTileUnits),
                  20 * static_cast<int>(kTileUnits)};
  foe.life = 78;
  foe.life_max = 100;
  foe.armour = 100;
  foe.alive = true;
  state.world.monsters = {foe};
  state.world.stored_items = 11;
  state.world.stored_trophies = 4;
  state.gear_overlay = true;
  state.selected_item = 0;
  state.camera.x = static_cast<double>(state.world.player.position.x);
  state.camera.y = static_cast<double>(state.world.player.position.y);
  state.camera.zoom = kCameraDefaultZoom * zoom_height_factor(768);

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "vesselforge-implicits: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }

  WorldCarriedItem atlatl;
  atlatl.id = "vessel-atlatl";
  atlatl.name = "Bronze Atlatl";
  atlatl.attack_bonus = 14;
  atlatl.equipped = true;
  atlatl.forge_lines = {"+20% Projectile Range"};
  state.world.carried = {atlatl};
  state.world.player.projectile_range_percent = 20;
  state.world.player.armour_penetration_percent = 0;
  const std::string atlatl_capture =
      capture_dir + "\\vesselforge-atlatl-range-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, atlatl_capture),
                 "vesselforge-implicits: Atlatl and Grips Framekit evidence captured");
  std::printf("    capture: %s\n", atlatl_capture.c_str());
  bool atlatl_line = false;
  bool ranged_totals = false;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::PaneStat &&
        item.label == "forge-line:+20% Projectile Range")
      atlatl_line = true;
    if (item.op == render::Op::PaneStat &&
        item.label.rfind("FORGE  SPD +8%", 0) == 0 &&
        item.label.find("SHOT +20%") != std::string::npos)
      ranged_totals = true;
  }
  scenario_check(atlatl_line && ranged_totals,
                 "vesselforge-implicits: range and speed read as active loadout facts");

  WorldCarriedItem sling;
  sling.id = "vessel-sling";
  sling.name = "Quilted Sling";
  sling.attack_bonus = 11;
  sling.equipped = true;
  sling.forge_lines = {"Ignores half of Armour"};
  state.world.carried = {sling};
  state.world.player.projectile_range_percent = 0;
  state.world.player.armour_penetration_percent = 50;
  state.world.player.facing = {-1, 0};
  state.world.monsters[0].position = {
      17 * static_cast<int>(kTileUnits),
      20 * static_cast<int>(kTileUnits)};
  verdigris::client::PresentationFx fx;
  verdigris::client::PresentationEvent hit;
  hit.type = verdigris::client::PresentationEventType::DamageApplied;
  hit.actor_id = foe.id;
  hit.text = "outgoing";
  hit.value = 15;
  hit.style = "range";
  hit.base_amount = 20;
  hit.armour_rating = 100;
  hit.armour_prevented = 5;
  hit.armour_penetration_percent = 50;
  verdigris::client::apply_presentation_event(fx, state.world, hit, 1);
  state.effects = std::move(fx.effects);
  const std::string sling_capture =
      capture_dir + "\\vesselforge-sling-pierce-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, sling_capture),
                 "vesselforge-implicits: Sling piercing frame captured");
  std::printf("    capture: %s\n", sling_capture.c_str());
  bool sling_line = false;
  bool penetration_totals = false;
  bool piercing_number = false;
  const HudRect expected = gear_pane_rect(1366, 768);
  bool inside = true;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::PaneStat &&
        item.label == "forge-line:Ignores half of Armour")
      sling_line = true;
    if (item.op == render::Op::PaneStat &&
        item.label.find("PEN 50%") != std::string::npos)
      penetration_totals = true;
    if (item.op == render::Op::Damage && item.label == "piercing:range")
      piercing_number = true;
  }
  int forge_rows_inside = 0;
  for (const auto& trace : state.hud_rect_trace)
    if (trace.first == "pane-progression" ||
        trace.first == "pane-progression-detail") {
      inside = inside && trace.second.x >= expected.x &&
               trace.second.x + trace.second.w <= expected.x + expected.w;
      ++forge_rows_inside;
    }
  scenario_check(sling_line && penetration_totals && piercing_number,
                 "vesselforge-implicits: Sling line, worn total, and PIERCE beat agree");
  scenario_check(inside && forge_rows_inside == 2,
                 "vesselforge-implicits: two-row forge totals remain inside Framekit");
  return 0;
}

class ScenarioLineageSession final
    : public verdigris::client::IClientSession {
 public:
  ScenarioLineageSession() {
    model_.player.uuid = "lineage-scion";
    model_.player.display_name = "Ilyra";
    model_.player.alive = false;
    model_.chronicles_pending = true;
    model_.chronicle.present = true;
    model_.chronicle.account_name = "Wayfinder-27";
    model_.chronicle.active_house_id = "house-emberwake";
    verdigris::client::ClientHouseEntry house;
    house.id = "house-emberwake";
    house.name = "Emberwake";
    house.campaign_complete = true;
    house.endgame_maps_completed = 17;
    house.endgame_masteries = 23;
    house.scions = {
        {"scion-ilyra", "Ilyra", 18, false},
        {"scion-tareth", "Tareth", 7, true},
    };
    house.crypt = {
        {"crypt-orun", "Orun the First", 21, "recovered", 1},
        {"crypt-sael", "Sael Ash-Hand", 14, "lost", 1},
    };
    model_.chronicle.houses.push_back(std::move(house));
  }
  bool start(std::string*) override { return true; }
  void shutdown() override {}
  void submit(const verdigris::client::ClientCommand& command) override {
    last_command = command;
    submitted = true;
  }
  void poll() override {}
  verdigris::client::ConnectionState connection_state() const override {
    return connection;
  }
  const verdigris::client::ClientModel& model() const override { return model_; }
  std::vector<verdigris::client::PresentationEvent> drain_events() override {
    return {};
  }
  const std::string& last_error() const override { return error_; }

  void add_scion_pages() {
    auto& scions = model_.chronicle.houses.front().scions;
    for (int index = 0; index < 7; ++index) {
      verdigris::client::ClientScionEntry scion;
      scion.id = "scion-reserve-" + std::to_string(index + 1);
      scion.name = "Reserve " + std::to_string(index + 1);
      scion.level = index + 2;
      scions.push_back(std::move(scion));
    }
  }

  void clear_houses() {
    model_.chronicle.houses.clear();
    model_.chronicle.active_house_id.clear();
  }

  bool submitted = false;
  verdigris::client::ConnectionState connection = verdigris::client::ConnectionState::Ready;
  verdigris::client::ClientCommand last_command{};

 private:
  verdigris::client::ClientModel model_;
  std::string error_;
};

int scenario_chronicles_lineage_ui() {
  ClientState state;
  auto owned = std::make_unique<ScenarioLineageSession>();
  ScenarioLineageSession* scenario = owned.get();
  state.session = std::move(owned);
  state.chronicles_mode = true;
  state.screen = Screen::Chronicles;
  state.chronicles_selected = 1;
  load_billboards(state.billboards);

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "chronicles-lineage-ui: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }

  for (const auto [width, height] :
       {std::pair<int, int>{960, 600}, {1366, 768}}) {
    const std::string capture =
        capture_dir + "\\chronicles-lineage-" + std::to_string(width) + "x" +
        std::to_string(height) + ".png";
    scenario_check(reference_present(state, width, height, capture),
                   "chronicles-lineage-ui: responsive Framekit capture written");
    std::printf("    capture: %s\n", capture.c_str());

    const HudRect* house_pane = nullptr;
    const HudRect* action_pane = nullptr;
    const HudRect* title = nullptr;
    const HudRect* subtitle = nullptr;
    for (const auto& trace : state.hud_rect_trace) {
      if (trace.first == "chronicles-house-pane") house_pane = &trace.second;
      if (trace.first == "chronicles-action-pane") action_pane = &trace.second;
      if (trace.first == "chronicles-title") title = &trace.second;
      if (trace.first == "chronicles-subtitle") subtitle = &trace.second;
    }
    scenario_check(house_pane && action_pane &&
                       !hud_rects_overlap(*house_pane, *action_pane),
                   "chronicles-lineage-ui: ledger and admission rail stay distinct");
    scenario_check(title && subtitle &&
                       subtitle->y >= title->y + title->h + 6 * hud_scale(height),
                   "chronicles-lineage-ui: masthead and subtitle keep a readable gap");
    scenario_check(state.chronicles_action_hits.size() == 4,
                   "chronicles-lineage-ui: two Scions, creation, and oath are actionable");
    bool bounded = true;
    for (const auto& hit : state.chronicles_action_hits)
      bounded = bounded && hit.rect.left >= 0 && hit.rect.top >= 0 &&
                hit.rect.right <= width && hit.rect.bottom <= height;
    scenario_check(bounded,
                   "chronicles-lineage-ui: every action remains inside the viewport");
    scenario_check(render_list_has(state, render::Op::Chronicles,
                                   "house Emberwake") &&
                       render_list_has(state, render::Op::Chronicles,
                                       "scion scion-ilyra") &&
                       render_list_has(state, render::Op::Chronicles,
                                       "crypt crypt-sael") &&
                       render_list_has(state, render::Op::Chronicles,
                                       "mastery:23/64"),
                   "chronicles-lineage-ui: House, living, remembered, and Wayfinder state render");
    scenario_check(
        state.billboards.framekit_status == "art: WIZARD Framekit 15/15" &&
            render_list_has(state, render::Op::Chronicles,
                            "framekit-raster:chronicle-laurel") &&
            render_list_has(state, render::Op::Chronicles,
                            "framekit-raster:chronicle-knocker"),
        "chronicles-lineage-ui: WIZARD laurel and House knocker draw at runtime");
  }


  state.chronicles_naming = ChronicleNamingMode::Scion;
  const std::string naming_capture =
      capture_dir + "\\chronicles-lineage-naming-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, naming_capture),
                 "chronicles-lineage-ui: raster naming ceremony captured");
  scenario_check(render_list_has(state, render::Op::Chronicles,
                                 "framekit-raster:chronicle-input"),
                 "chronicles-lineage-ui: WIZARD input field draws under live text");
  for (const auto mode : {ChronicleNamingMode::House, ChronicleNamingMode::Scion}) {
    state.chronicles_naming = mode;
    state.chronicles_name_input = "WWWWWWWWWWWWWWWWWWWWWWWWWWWW";
    for (const auto size : {std::pair{960,600}, std::pair{1727,1395}}) {
      const std::string filename = capture_dir + "\\naming-long-" +
          (mode == ChronicleNamingMode::House ? "house-" : "scion-") +
          std::to_string(size.first) + "x" + std::to_string(size.second) + ".png";
      scenario_check(reference_present(state,size.first,size.second,filename),
                     "chronicles-lineage-ui: long name captured at owner and small-window sizes");
      scenario_check(state.chronicles_name_input.size() == 28,
                     "chronicles-lineage-ui: scrolling does not truncate the actual name");
      scenario_check(render_list_has(state,render::Op::Chronicles,"naming:value:") &&
                     !render_list_has(state,render::Op::Chronicles,
                                      "naming:value:" + state.chronicles_name_input + "|"),
                     "chronicles-lineage-ui: overflowing name scrolls inside authored frame");
    }
  }
  state.chronicles_name_input.clear();
  state.chronicles_naming = ChronicleNamingMode::None;

  state.chronicles_selected = 0;
  handle_chronicles_key(state, VK_DOWN);
  scenario_check(state.chronicles_selected == 1,
                 "chronicles-lineage-ui: arrow keys move the Scion selection");
  scenario_present(state);
  const auto oath_hit = state.chronicles_action_hits.back();
  scenario_check(activate_chronicle_at(
                     state, (oath_hit.rect.left + oath_hit.rect.right) / 2,
                     (oath_hit.rect.top + oath_hit.rect.bottom) / 2) &&
                     state.chronicles_oath,
                 "chronicles-lineage-ui: pointer arms the same mortal-oath action");
  scenario_present(state);
  const auto scion_hit = state.chronicles_action_hits.front();
  scenario_check(activate_chronicle_at(
                     state, (scion_hit.rect.left + scion_hit.rect.right) / 2,
                     (scion_hit.rect.top + scion_hit.rect.bottom) / 2) &&
                     scenario->submitted &&
                     scenario->last_command.type ==
                         verdigris::client::ClientCommand::Type::SelectScion &&
                     scenario->last_command.target == "scion-ilyra" &&
                     scenario->last_command.value == 1,
                 "chronicles-lineage-ui: pointer admission uses the oath-bearing server command");
  scenario->add_scion_pages();
  state.chronicles_selected = 10;
  scenario_present(state);
  scenario_check(state.chronicles_menu.size() == 11 &&
                     state.chronicles_action_hits.size() <
                         state.chronicles_menu.size() &&
                     !state.chronicles_action_hits.empty() &&
                     state.chronicles_action_hits.back().index == 10,
                 "chronicles-lineage-ui: a large lineage pages around the selected action");
  scenario->clear_houses();
  state.chronicles_selected = 0;
  const std::string unfounded_capture =
      capture_dir + "\\chronicles-unfounded-960x600.png";
  scenario_check(reference_present(state, 960, 600, unfounded_capture),
                 "chronicles-lineage-ui: unfounded House lock captured");
  scenario_check(render_list_has(state, render::Op::Chronicles,
                                 "framekit-raster:chronicle-keyhole"),
                 "chronicles-lineage-ui: an unfounded House shows the WIZARD keyhole");
  return 0;
}

int scenario_guided_creation() {
  using namespace verdigris::client;
  std::unique_ptr<verdigris::networking::WebSocketServer> server;
  unsigned short port=0;
  for (unsigned short p=6780;p<=6799;++p) {
    auto candidate=std::make_unique<verdigris::networking::WebSocketServer>(p);
    std::string error;
    if (candidate->start(&error)) { server=std::move(candidate); port=p; break; }
  }
  scenario_check(server!=nullptr,"creation: isolated real server starts");
  if (!server) return scenario_failures;
  ClientState state;
  state.title_enabled=state.title_open=state.chronicles_mode=true;
  state.screen=Screen::Chronicles;
  state.session=std::make_unique<RemoteProtocolSession>("127.0.0.1",port,"guided-creation-proof",false);
  std::string error;
  scenario_check(state.session->start(&error),"creation: real remote session connects");
  scenario_check(chronicles_pump(state,250,[&]{return title_account_ready(state);}),"creation: saved roster arrives before admission");
  activate_title_action(state,0);
  scenario_check(state.startup_creation && state.chronicles_naming==ChronicleNamingMode::House,"creation: first action asks only for House name");
  activate_creation_control(state,2);
  scenario_check(state.creation_wait==ClientState::CreationWait::None && !state.chronicles_name_error.empty(),"creation: blank name cannot silently create a default House");
  scenario_type_chronicle_name(state,"The Copper Lantern");
  activate_creation_control(state,2);
  activate_creation_control(state,2); // double-click must not emit another founder
  scenario_check(chronicles_pump(state,250,[&]{return state.chronicles_naming==ChronicleNamingMode::Scion;}),"creation: saved House leads directly to Scion naming, without a ledger detour");
  scenario_check(state.session->model().chronicle.houses.size()==1,"creation: repeated confirm does not duplicate the House");
  scenario_type_chronicle_name(state,"Ilyra Copper-Hand");
  scenario_check(state.chronicles_name_input=="Ilyra Copper-Hand","creation: next-step first character is not swallowed");
  activate_creation_control(state,1);
  scenario_check(state.chronicles_oath,"creation: one control arms Hardcore");
  std::string capture_dir;
  const int override_state=capture_root_override(&capture_dir);
  if (override_state<0) { state.session->shutdown(); server->stop(); scenario_check(false,"creation: invalid capture root"); return scenario_failures; }
  if (!override_state) capture_dir="native/build/creation-captures";
  std::filesystem::create_directories(capture_dir);
  load_billboards(state.billboards);
  for (POINT size : {POINT{960,600},POINT{1727,1395},POINT{3440,1440}}) {
    reference_present(state,size.x,size.y,capture_dir+"/creation-hardcore-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png");
    scenario_check(render_list_has(state,render::Op::Chronicles,"creation:hardcore:on"),"creation: Hardcore choice is visible on the name card");
    scenario_check(!render_list_has(state,render::Op::Chronicles,"action:"),"creation: legacy ledger actions do not crowd guided creation");
    scenario_check(state.creation_back.left>=0 && state.creation_confirm.right<=size.x && state.creation_confirm.bottom<=size.y,"creation: buttons fit resized windows");
  }
  activate_creation_control(state,2);
  activate_creation_control(state,2);
  scenario_check(chronicles_pump(state,250,[&]{return !state.startup_creation && state.screen==Screen::Expedition;}),"creation: exact saved Scion is admitted automatically");
  const auto& model=state.session->model();
  const auto* scion=find_chronicle_scion(model.chronicle,model.chronicle.active_scion_id);
  scenario_check(scion && scion->name=="Ilyra Copper-Hand" && scion->mortal,"creation: server roster preserves chosen name and Hardcore oath");
  scenario_check(model.chronicle.houses.size()==1 && model.chronicle.houses.front().scions.size()==1,"creation: repeated confirmation creates only one character");
  // Return to title only in the fixture; production never teleports the live player here.
  state.title_open=true;
  activate_title_action(state,1);
  scenario_check(!state.chronicles_oath,"creation: a separate new Scion does not inherit the prior oath selection");
  scenario_type_chronicle_name(state,"Second Copper");
  activate_creation_control(state,2);
  cancel_startup_creation(state); // cancel only presentation, never undo a saved request
  scenario_check(chronicles_pump(state,250,[&]{return state.session->model().chronicle.houses.front().scions.size()==2;}),"creation: late save receipt remains in the roster after Back");
  scenario_check(state.title_open && !state.startup_creation,"creation: a late receipt cannot reopen a canceled wizard or auto-admit");
  const auto* second=find_chronicle_scion(state.session->model().chronicle,state.session->model().chronicle.created_scion_id);
  scenario_check(second && second->name=="Second Copper" && !second->mortal,"creation: unchecked Hardcore is stored as a soft character");
  state.session->shutdown(); server->stop();
  return scenario_failures;
}

int scenario_title_island() {
  ClientState state;
  auto session = std::make_unique<ScenarioLineageSession>();
  auto* recorder = session.get();
  state.session = std::move(session);
  state.title_open = state.title_enabled = state.chronicles_mode = true;
  state.screen = Screen::Chronicles;
  load_billboards(state.billboards);
  recorder->connection = verdigris::client::ConnectionState::Connected;
  scenario_check(title_account_ready(state), "title: saved roster enables menu before world admission");
  recorder->connection = verdigris::client::ConnectionState::Retrying;
  scenario_check(!activate_title_action(state,0) && state.title_open && !recorder->submitted,
                 "title: reconnect cannot submit stale admissions");
  recorder->connection = verdigris::client::ConnectionState::Connected;
  scenario_check(activate_title_action(state,0) && recorder->submitted &&
                 recorder->last_command.type == verdigris::client::ClientCommand::Type::SelectScion &&
                 recorder->last_command.target == "scion-ilyra" && recorder->last_command.value == 0,
                 "title: continue keeps the living Scion identity and soft oath");
  state.title_open = true;
  scenario_check(activate_title_action(state,1) && state.chronicles_naming == ChronicleNamingMode::Scion,
                 "title: create opens real Scion naming");
  append_chronicle_text(state,L"O'Rin\r\n-Copper",14);
  scenario_check(state.chronicles_name_input == "O'Rin-Copper" && state.chronicles_naming == ChronicleNamingMode::Scion,
                 "naming: paste accepts a name but cannot submit embedded newlines");
  append_chronicle_text(state,L" Long Unbounded Clipboard Payload",33);
  scenario_check(state.chronicles_name_input.size()==28 && !state.chronicles_name_error.empty(),
                 "naming: clipboard insertion obeys the same length cap as typing");
  cancel_chronicle_naming(state);
  recorder->clear_houses();
  state.title_open = true;
  scenario_check(activate_title_action(state,0) && state.chronicles_naming == ChronicleNamingMode::House,
                 "title: first-time begin opens House founding");
  cancel_chronicle_naming(state);
  state.title_open = true;
  scenario_check(activate_title_action(state,2) && !state.title_open && state.screen==Screen::Chronicles,
                 "title: secondary menu opens the lineage ledger");
  state.title_open = true;
  std::string capture_dir;
  const int override_state = capture_root_override(&capture_dir);
  if (override_state < 0) { scenario_check(false,"title: invalid capture root"); return scenario_failures; }
  if (!override_state) capture_dir = "native/build/title-captures";
  std::filesystem::create_directories(capture_dir);
  for (POINT size : {POINT{960,600},POINT{1727,1395},POINT{3440,1440}}) {
    scenario_check(reference_present(state,size.x,size.y,capture_dir+"/title-"+std::to_string(size.x)+"x"+std::to_string(size.y)+".png"),
                   "title: responsive native scene captured");
    bool inside=state.title_action_hits.size()==3;
    for (RECT hit : state.title_action_hits)
      inside=inside && hit.left>=0 && hit.top>=0 && hit.right<=size.x && hit.bottom<=size.y;
    scenario_check(inside,"title: all menu targets stay inside the resized window");
    scenario_check(state.title_scene.error().empty() && state.title_scene.triangles()==119259,
                   "title: actual WIZARD mesh rendered, not a fallback image");
    if (!state.title_scene.error().empty()) std::printf("    title graphics: %s\n",state.title_scene.error().c_str());
  }
  constexpr int width=3440,height=1440;
  BITMAPINFO info{}; info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER); info.bmiHeader.biWidth=width;
  info.bmiHeader.biHeight=-height; info.bmiHeader.biPlanes=1; info.bmiHeader.biBitCount=32;
  void* bits=nullptr; HDC dc=CreateCompatibleDC(nullptr);
  HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,nullptr,0);
  if (!bitmap) { scenario_check(false,"title: frame surface allocation"); DeleteDC(dc); return scenario_failures; }
  const auto old=SelectObject(dc,bitmap); RECT bounds{0,0,width,height};
  paint_scene(state,dc,bounds);
  auto fingerprint=[&]() {
    std::uint64_t value=1469598103934665603ULL;
    auto* bytes=static_cast<unsigned char*>(bits);
    for(size_t i=0;i<static_cast<size_t>(width)*height*4;i+=127) value=(value^bytes[i])*1099511628211ULL;
    return value;
  };
  const auto before=fingerprint();
  state.title_orbit.previous={0,0}; state.title_orbit.dragging=true; state.title_orbit.drag({200,40}); state.title_orbit.dragging=false;
  paint_scene(state,dc,bounds);
  scenario_check(before!=fingerprint(),"title: orbit changes actual rendered geometry pixels");
  for(int i=0;i<100;++i) state.title_orbit.wheel(WHEEL_DELTA);
  scenario_check(state.title_orbit.zoom==.78f,"title: close zoom clamps safely");
  for(int i=0;i<100;++i) state.title_orbit.wheel(-WHEEL_DELTA);
  scenario_check(state.title_orbit.zoom==1.35f,"title: far zoom clamps safely");
  state.title_orbit.reset();
  scenario_check(state.title_orbit.yaw==0 && state.title_orbit.pitch==0 && state.title_orbit.zoom==1 && !state.title_orbit.dragging,
                 "title: Home restores the authored view");
  const auto begin=std::chrono::steady_clock::now();
  for(int i=0;i<20;++i) paint_scene(state,dc,bounds);
  const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count()/20;
  std::printf("    title frame-budget: %.1f ms, 20 real 3440x1440 frames\n",ms);
  scenario_check(ms<40,"title: interactive native composition stays below 40ms");
  verdigris::client::TitleScene missing;
  scenario_check(!missing.draw(dc,bounds,"native/build/nonexistent-title-assets",state.title_orbit,0) && !missing.error().empty(),
                 "title: missing assets produce an explicit error, not fake success");
  SelectObject(dc,old); DeleteObject(bitmap); DeleteDC(dc);
  return scenario_failures;
}

class ScenarioForgeSession final
    : public verdigris::client::IClientSession {
 public:
  ScenarioForgeSession() {
    model_.player.uuid = "scenario-forge-scion";
    model_.player.display_name = "Ilyra";
    model_.player.scene_id = "town:verdigris";
    model_.player.x = 42.0;
    model_.player.y = 121.0;
    model_.player.life = model_.player.life_max = 120;
    model_.player.resource = model_.player.resource_max = 55;
    model_.scene.id = "town:verdigris";
    model_.scene.type = "town";
    model_.scene.name = "The Crossroads";
    model_.theme = "town";
    model_.house_name = "House Emberwake";
    model_.npcs = {
        {5, "tamar-vesselwright", "Tamar the Vesselwright", "vesselwright",
         "A low forge and copper tools.", 42.0, 121.0,
         {"vesselforge", "brand_searing", "trophy_socketing"},
         {"talk", "examine"}},
    };
    model_.forge.open = true;
    model_.forge.name = "Tamar's Vesselforge";
    model_.forge.npc_id = 5;
    model_.forge.carried_coins = 145;
    const auto add = [&](const char* uuid, const char* name, const char* material,
                         const char* form, int vessel, int used, int patience,
                         int patience_max, bool eligible, const char* reason,
                         std::vector<verdigris::client::ClientForgeLine> lines) {
      verdigris::client::ClientForgeRow row;
      row.uuid = uuid; row.name = name; row.material = material; row.form = form;
      row.item_level = 40; row.vessel = vessel; row.used = used;
      row.free_slots = (std::max)(0, vessel - used);
      row.patience = patience; row.patience_max = patience_max;
      row.brand_count = (std::max)(0, used); row.cost = 100;
      row.eligible = eligible; row.reason = reason; row.lines = std::move(lines);
      model_.forge.rows.push_back(std::move(row));
    };
    add("vf-handaxe", "Bronze Handaxe", "Bronze", "Handaxe", 4, 2, 3, 5,
        true, "", {{"implicit", "+12% Physical Damage", "normal"},
                    {"brand", "+9% Chance to Bleed", "normal"},
                    {"vessel", "Vessel 4 - Patience 3/5", "normal"}});
    model_.forge.rows.back().trophy_options = {
        {"boar_tusk", "Boar Tusk", "+10% increased Physical Damage", "",
         5, 5, true},
        {"wolf_fang", "Wolf Fang", "+6% Attack Speed",
         "Needs 3 more fragments.", 2, 5, false},
    };
    add("vf-atlatl", "Copper Atlatl", "Copper", "Atlatl", 3, 3, 2, 4,
        false, "Its vessel is full.",
        {{"implicit", "+20% Projectile Range", "normal"}});
    add("vf-wrap", "Quilted Wrap", "Quilted", "Wrap", 4, 1, 0, 4,
        false, "Its patience is spent.",
        {{"implicit", "+18 Maximum Health", "normal"}});
    add("vf-sling", "Obsidian Sling", "Obsidian", "Sling", 5, 2, 2, 3,
        true, "", {{"implicit", "Ignores half of Armour", "normal"},
                    {"dormant", "Dormant - BOND: Battle Rhythm - +14% Attack Speed after a Kill [Slaughter I]", "inactive"}});
    model_.forge.rows.back().brand_count = 1;
    model_.forge.rows.back().bond_count = 1;
    model_.forge.rows.back().attunement = 42;
    model_.forge.rows.back().attunement_next = 135;
    model_.forge.rows.back().evolutions = 1;
    model_.forge.rows.back().trophy_options = {
        {"boar_tusk", "Boar Tusk", "+10% increased Physical Damage", "",
         5, 5, true},
        {"wolf_fang", "Wolf Fang", "+6% Attack Speed",
         "Needs 3 more fragments.", 2, 5, false},
    };
    add("vf-crest", "Jade Crest", "Jade", "Crest", 5, 1, 4, 6,
        true, "", {{"implicit", "+10 to all Attributes", "normal"}});
    add("vf-gorget", "Amber Gorget", "Amber", "Gorget", 5, 2, 4, 6,
        true, "", {{"brand", "+14% Goods Found", "normal"}});
  }
  bool start(std::string*) override { return true; }
  void shutdown() override {}
  void submit(const verdigris::client::ClientCommand& command) override {
    last_type = command.type;
    last_target = command.target;
    last_extra = command.extra;
    last_auxiliary = command.auxiliary;
    last_value = command.value;
    if (command.type == verdigris::client::ClientCommand::Type::CloseScreen)
      model_.forge.open = false;
  }
  void poll() override {}
  verdigris::client::ConnectionState connection_state() const override {
    return verdigris::client::ConnectionState::Ready;
  }
  const verdigris::client::ClientModel& model() const override { return model_; }
  std::vector<verdigris::client::PresentationEvent> drain_events() override {
    return {};
  }
  const std::string& last_error() const override { return error_; }

  verdigris::client::ClientCommand::Type last_type =
      verdigris::client::ClientCommand::Type::Move;
  std::string last_target;
  std::string last_extra;
  std::string last_auxiliary;
  int last_value = 0;

 private:
  verdigris::client::ClientModel model_;
  std::string error_;
};

class ScenarioDialogueSession final
    : public verdigris::client::IClientSession {
 public:
  ScenarioDialogueSession() {
    model_.player.uuid = "scenario-scion";
    model_.player.display_name = "Edda";
    model_.player.scene_id = "town:verdigris";
    model_.player.x = 31.0;
    model_.player.y = 121.0;
    model_.player.life = model_.player.life_max = 110;
    model_.player.resource = model_.player.resource_max = 50;
    model_.scene.id = "town:verdigris";
    model_.scene.type = "town";
    model_.scene.name = "The Crossroads";
    model_.theme = "town";
    model_.house_name = "House Ashwake";
    model_.npcs = {
        {1, "aldwyn-guide", "Aldwyn the Guide", "elder", "Wayfinder.",
         34.0, 116.0, {"guidance", "expedition_access"}, {"talk", "examine"}},
        {2, "ludovicus-weapons", "Ludovicus, Weapons Trader",
         "weapons_tools_trainer", "Road iron.", 19.0, 113.0, {"shop"},
         {"trade", "examine"}},
        {3, "selene-rite", "Selene of the Rite", "armor_ritual_merchant",
         "Ritual armor.", 45.0, 108.0, {"shop"}, {"trade", "examine"}},
        {4, "rhea-countinghouse", "Rhea of the Countinghouse", "steward",
         "House ledger.", 31.0, 121.0, {"storage", "house_investment"},
         {"bank", "examine"}},
        {5, "tamar-vesselwright", "Tamar the Vesselwright", "vesselwright",
         "The town forge.", 42.0, 121.0,
         {"vesselforge", "brand_searing", "trophy_socketing"},
         {"talk", "examine"}},
    };
    model_.dialogue.open = true;
    model_.dialogue.npc_id = 4;
    model_.dialogue.npc_key = "rhea-countinghouse";
    model_.dialogue.name = "Rhea of the Countinghouse";
    model_.dialogue.role = "steward";
    model_.dialogue.body =
        "Your first cleared road has earned one founding investment. Choose for this Scion now, or build a yield every future clear returns to the House.";
    model_.dialogue.options = {
        {"bank", "Open the Countinghouse",
         "Move carried goods and gold into House keeping.",
         "player:screen:bank", true},
        {"scion_gear", "Commission named Scion gear",
         "Immediate tier-one Vesselforge gear, bound to this Scion.",
         "house:investment:choose", true},
        {"house_production", "Build House road production",
         "+5 House treasury after every future floor clear.",
         "house:investment:choose", true},
    };
  }
  bool start(std::string*) override { return true; }
  void shutdown() override {}
  void submit(const verdigris::client::ClientCommand& command) override {
    if (command.type == verdigris::client::ClientCommand::Type::CloseScreen)
      model_.dialogue.open = false;
  }
  void poll() override {}
  verdigris::client::ConnectionState connection_state() const override {
    return verdigris::client::ConnectionState::Ready;
  }
  const verdigris::client::ClientModel& model() const override { return model_; }
  std::vector<verdigris::client::PresentationEvent> drain_events() override {
    return {};
  }
  const std::string& last_error() const override { return error_; }

 private:
  verdigris::client::ClientModel model_;
  std::string error_;
};

class ScenarioCampaignSession final
    : public verdigris::client::IClientSession {
 public:
  explicit ScenarioCampaignSession(bool deep_roads = false,
                                   bool crownless_marches = false,
                                   bool verdigris_crown = false) {
    model_.player.uuid = "scenario-campaign-scion";
    model_.player.display_name = "Ilyra";
    model_.player.scene_id = "town:verdigris";
    model_.player.x = 31.0;
    model_.player.y = 121.0;
    model_.player.life = model_.player.life_max = 120;
    model_.player.resource = model_.player.resource_max = 55;
    model_.scene.id = "town:verdigris";
    model_.scene.type = "town";
    model_.scene.name = "The Crossroads";
    model_.theme = "town";
    model_.house_name = "House Emberwake";
    model_.quests.present = true;
    model_.quests.quest_points = 4;
    model_.quests.campaign_quest_total = 23;
    model_.quests.house_renown = 50;
    model_.quests.act_number = 2;
    model_.quests.act_title = "THE FOUR-ROAD COVENANT";
    model_.quests.act_completed = 0;
    model_.quests.act_total = 4;
    model_.quests.active_id = "oath-of-tin";
    model_.quests.title = "Oath of Tin";
    model_.quests.giver = "Aldwyn the Guide";
    model_.quests.summary =
        "Carry the House standard north and prove the first road open.";
    model_.quests.objective = "Break its Warden and open the road onward.";
    model_.quests.reward = "+1 quest point / +25 House renown";
    model_.quests.objective_index = 1;
    model_.quests.objective_count = 3;
    model_.quests.completed = {
        {"aldwyns-charge", "Aldwyn's Charge", "Answered Aldwyn's Charge"},
        {"proof-of-temper", "Proof of Temper",
         "Proved their temper in the old realms"},
        {"the-pale-crown", "The Pale Crown",
         "Broke the Pale Sovereign's seal"},
        {"rot-in-the-reeds", "Rot in the Reeds",
         "Ended the rot beneath the reeds"},
    };
    if (deep_roads) {
      model_.quests.quest_points = 10;
      model_.quests.house_renown = 275;
      model_.quests.act_number = 3;
      model_.quests.act_title = "THE DEEP ROADS";
      model_.quests.act_completed = 2;
      model_.quests.active_id = "bell-beneath-chalk";
      model_.quests.title = "The Bell Beneath Chalk";
      model_.quests.giver = "Selene of the Rite";
      model_.quests.summary =
          "Descend past the named graves and still the bell that calls them awake.";
      model_.quests.objective = "Defeat the Ossuary Bell.";
      model_.quests.reward = "+1 quest point / +55 House renown";
      model_.quests.completed.insert(model_.quests.completed.end(), {
          {"oath-of-tin", "Oath of Tin", "Swore the Oath of Tin"},
          {"salt-reckoning", "The Salt Reckoning",
           "Settled the Salt Reckoning"},
          {"chalk-vigil", "The Chalk Vigil", "Kept the Chalk Vigil"},
          {"copper-testament", "The Copper Testament",
           "Sealed the Copper Testament"},
          {"quarry-saints-canon", "The Quarry Saint's Canon",
           "Silenced the Quarry Saint"},
          {"brine-widows-tithe", "The Brine Widow's Tithe",
           "Refused the Brine Widow's tithe"},
      });
    }
    if (crownless_marches) {
      model_.quests.quest_points = 14;
      model_.quests.house_renown = 535;
      model_.quests.act_number = 4;
      model_.quests.act_title = "THE CROWNLESS MARCHES";
      model_.quests.act_completed = 2;
      model_.quests.active_id = "white-harrow";
      model_.quests.title = "The White Harrow";
      model_.quests.giver = "Selene of the Rite";
      model_.quests.summary =
          "Cross the nameless furrows and silence the thing harvesting their dead.";
      model_.quests.objective = "Defeat the White Harrow.";
      model_.quests.reward = "+1 quest point / +80 House renown";
      model_.quests.completed.insert(model_.quests.completed.end(), {
          {"bell-beneath-chalk", "The Bell Beneath Chalk",
           "Stilled the Ossuary Bell"},
          {"cinder-judgment", "The Cinder Judgment",
           "Passed the Cinder Judge's sentence"},
          {"iron-abbots-rule", "The Iron Abbot's Rule",
           "Broke the Iron Abbot's rule"},
          {"drowned-factors-toll", "The Drowned Factor's Toll",
           "Cancelled the Drowned Factor's toll"},
      });
    }
    if (verdigris_crown) {
      model_.quests.quest_points = 22;
      model_.quests.house_renown = 1330;
      model_.quests.act_number = 6;
      model_.quests.act_title = "THE VERDIGRIS CROWN";
      model_.quests.act_completed = 2;
      model_.quests.act_total = 3;
      model_.quests.active_id = "crown-without-king";
      model_.quests.title = "A Crown Without a King";
      model_.quests.giver = "Selene of the Rite";
      model_.quests.summary =
          "Seal the last grave, break the western usurper, and crown the roads themselves.";
      model_.quests.objective =
          "Enter the Empty Throne on the tier-five Copper Road.";
      model_.quests.reward =
          "+1 quest point / +150 House renown / Wayfinder access";
      model_.quests.objective_index = 3;
      model_.quests.objective_count = 6;
      model_.quests.completed.insert(model_.quests.completed.end(), {
          {"white-harrow", "The White Harrow", "Turned the White Harrow"},
          {"ash-castellan", "The Ash Castellan",
           "Cast down the Ash Castellan"},
          {"chain-regent", "The Chain Regent", "Unmade the Chain Regent"},
          {"mire-leviathan", "The Mire Leviathan",
           "Balanced the Mire Leviathan's account"},
          {"nameless-bishop", "The Nameless Bishop",
           "Denied the Nameless Bishop a House"},
          {"furnace-king", "The Furnace King", "Quenched the Furnace King"},
          {"claim-of-iron", "The Claim of Iron", "Won the Claim of Iron"},
          {"claim-of-salt", "The Claim of Salt", "Won the Claim of Salt"},
      });
    }
  }
  bool start(std::string*) override { return true; }
  void shutdown() override {}
  void submit(const verdigris::client::ClientCommand&) override {}
  void poll() override {}
  verdigris::client::ConnectionState connection_state() const override {
    return verdigris::client::ConnectionState::Ready;
  }
  const verdigris::client::ClientModel& model() const override { return model_; }
  std::vector<verdigris::client::PresentationEvent> drain_events() override {
    return {};
  }
  const std::string& last_error() const override { return error_; }

 private:
  verdigris::client::ClientModel model_;
  std::string error_;
};

int scenario_town_social_hub() {
  ClientState state;
  state.session = std::make_unique<ScenarioDialogueSession>();
  load_billboards(state.billboards);
  sync_world(state);
  generate_scenery(state);
  scenario_follow_camera(state);

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "town-social-hub: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string capture_path =
      capture_dir + "\\town-social-hub-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, capture_path),
                 "town-social-hub: Framekit dialogue evidence captured");
  std::printf("    capture: %s\n", capture_path.c_str());

  bool pane = false;
  int options = 0;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Hud && item.label == "dialogue-pane") pane = true;
    if (item.op == render::Op::Hud &&
        item.label.rfind("dialogue-option:", 0) == 0)
      ++options;
  }
  scenario_check(pane, "town-social-hub: authoritative dialogue pane is visible");
  scenario_check(options == 3 && state.trade_row_hits.size() == 3,
                 "town-social-hub: bank and both founding choices are actionable");
  bool inside = true;
  for (const auto& hit : state.trade_row_hits)
    inside = inside && hit.rect.left >= 0 && hit.rect.top >= 0 &&
             hit.rect.right <= 1366 && hit.rect.bottom <= 768;
  scenario_check(inside,
                 "town-social-hub: every choice remains inside the compact viewport");
  return 0;
}

int scenario_town_vesselforge() {
  ClientState state;
  auto owned = std::make_unique<ScenarioForgeSession>();
  ScenarioForgeSession* scenario = owned.get();
  state.session = std::move(owned);
  load_billboards(state.billboards);
  sync_world(state);
  generate_scenery(state);
  scenario_follow_camera(state);

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "town-vesselforge: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string compact =
      capture_dir + "\\town-vesselforge-960x600.png";
  scenario_check(reference_present(state, 960, 600, compact),
                 "town-vesselforge: compact Framekit service captured");
  std::printf("    capture: %s\n", compact.c_str());
  bool compact_inside = state.trade_row_hits.size() == 6;
  for (const auto& hit : state.trade_row_hits)
    compact_inside = compact_inside && hit.rect.left >= 0 && hit.rect.top >= 0 &&
                     hit.rect.right <= 960 && hit.rect.bottom <= 600;
  scenario_check(compact_inside,
                 "town-vesselforge: five-row page stays inside 960x600");

  state.trade_selected = 3;
  const std::string large =
      capture_dir + "\\town-vesselforge-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, large),
                 "town-vesselforge: selected vessel detail captured");
  std::printf("    capture: %s\n", large.c_str());
  bool pane = false;
  bool active_line = false;
  bool memory = false;
  bool dormant_bond = false;
  bool trophy_action = false;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Hud && item.label == "vesselforge-pane") pane = true;
    if (item.op == render::Op::PaneStat &&
        item.label == "vesselforge-line:Ignores half of Armour")
      active_line = true;
    if (item.op == render::Op::PaneStat &&
        item.label.rfind("vesselforge-memory:ATTUNEMENT  42/135", 0) == 0)
      memory = true;
    if (item.op == render::Op::PaneStat &&
        item.label.rfind("vesselforge-line:Dormant - BOND:", 0) == 0)
      dormant_bond = true;
    if (item.op == render::Op::PaneStat &&
        item.label == "vesselforge-trophy:boar_tusk")
      trophy_action = true;
  }
  scenario_check(pane && active_line && memory && dormant_bond &&
                     trophy_action && state.trade_row_hits.size() == 6,
                 "town-vesselforge: Framekit list, progress, Bond, trophy, and paging evidence agree");
  for (const auto& hit : state.trade_row_hits) {
    if (hit.index == state.trade_selected) {
      activate_trade_row(state, hit);
      break;
    }
  }
  scenario_check(
      scenario->last_type == verdigris::client::ClientCommand::Type::MenuAction &&
          scenario->last_target == "player:vesselforge:add-brand" &&
          scenario->last_extra == "vf-sling" && scenario->last_value == 100,
      "town-vesselforge: Enter/click routes the exact authoritative item and cost");
  for (const auto& hit : state.trade_row_hits) {
    if (hit.kind == 6 && hit.index == state.trade_selected) {
      activate_trade_row(state, hit);
      break;
    }
  }
  scenario_check(
      scenario->last_type == verdigris::client::ClientCommand::Type::MenuAction &&
          scenario->last_target == "player:vesselforge:socket-trophy" &&
          scenario->last_extra == "vf-sling" &&
          scenario->last_auxiliary == "boar_tusk",
      "town-vesselforge: T/click routes the exact vessel and trophy identities");
  return 0;
}

int scenario_campaign_journal() {
  ClientState state;
  state.session = std::make_unique<ScenarioCampaignSession>();
  load_billboards(state.billboards);
  sync_world(state);
  generate_scenery(state);
  scenario_follow_camera(state);
  state.gear_overlay = true;
  state.character_pane = true;
  state.tree_pane = true;
  toggle_quest_journal(state);
  scenario_check(state.quest_journal && !state.gear_overlay &&
                     !state.character_pane && !state.tree_pane,
                 "campaign-journal: opening dismisses narrower character panes");

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "campaign-journal: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string capture_path =
      capture_dir + "\\campaign-journal-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, capture_path),
                 "campaign-journal: Framekit campaign evidence captured");
  std::printf("    capture: %s\n", capture_path.c_str());

  bool pane = false;
  bool active = false;
  bool objective = false;
  int completed = 0;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Hud && item.label == "quest-journal") pane = true;
    if (item.op == render::Op::Hud &&
        item.label == "quest-active:oath-of-tin") active = true;
    if (item.op == render::Op::Hud &&
        item.label ==
            "quest-objective:Break its Warden and open the road onward.")
      objective = true;
    if (item.op == render::Op::Hud &&
        item.label.rfind("quest-complete:", 0) == 0)
      ++completed;
  }
  scenario_check(pane && active && objective && completed == 4,
                 "campaign-journal: active rite and completed deeds use authoritative copy");
  const HudRect expected = quest_journal_rect(1366, 768);
  const HudRect* footer = nullptr;
  for (const auto& entry : state.hud_rect_trace)
    if (entry.first == "quest-pane-footer") footer = &entry.second;
  scenario_check(expected.x >= 0 && expected.y >= 0 &&
                     expected.x + expected.w <= 1366 &&
                     expected.y + expected.h <= 768 && footer &&
                     footer->x + footer->w <= expected.x + expected.w,
                 "campaign-journal: journal and controls fit the compact viewport");
  state.quit_requested = false;
  handle_escape_key(state);
  scenario_check(!state.quest_journal && !state.quit_requested,
                 "campaign-journal: Escape closes journal before exiting");
  return 0;
}

int scenario_deep_roads_campaign() {
  ClientState state;
  state.session = std::make_unique<ScenarioCampaignSession>(true);
  load_billboards(state.billboards);
  sync_world(state);
  generate_scenery(state);
  scenario_follow_camera(state);
  toggle_quest_journal(state);

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "deep-roads-campaign: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string capture_path =
      capture_dir + "\\deep-roads-campaign-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, capture_path),
                 "deep-roads-campaign: Act III Framekit journal captured");
  std::printf("    capture: %s\n", capture_path.c_str());

  bool act = false;
  bool active = false;
  bool objective = false;
  bool earlier = false;
  int completed = 0;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Hud &&
        item.label == "campaign-act:3:THE DEEP ROADS")
      act = true;
    if (item.op == render::Op::Hud &&
        item.label == "quest-active:bell-beneath-chalk")
      active = true;
    if (item.op == render::Op::Hud &&
        item.label == "quest-objective:Defeat the Ossuary Bell.")
      objective = true;
    if (item.op == render::Op::Hud && item.label == "quest-earlier-deeds" &&
        item.value == 1)
      earlier = true;
    if (item.op == render::Op::Hud &&
        item.label.rfind("quest-complete:", 0) == 0)
      ++completed;
  }
  scenario_check(act && active && objective && earlier && completed == 9,
                 "deep-roads-campaign: chapter, boss rite, and bounded deed history are readable");
  const HudRect expected = quest_journal_rect(1366, 768);
  bool inside = expected.x >= 0 && expected.y >= 0 &&
                expected.x + expected.w <= 1366 &&
                expected.y + expected.h <= 768;
  for (const auto& trace : state.hud_rect_trace)
    if (trace.first.rfind("quest-pane", 0) == 0)
      inside = inside && trace.second.x >= expected.x &&
               trace.second.y >= expected.y &&
               trace.second.x + trace.second.w <= expected.x + expected.w &&
               trace.second.y + trace.second.h <= expected.y + expected.h;
  scenario_check(inside,
                 "deep-roads-campaign: growing Chronicle remains inside Framekit");
  return 0;
}

int scenario_crownless_campaign() {
  ClientState state;
  state.session = std::make_unique<ScenarioCampaignSession>(true, true);
  load_billboards(state.billboards);
  sync_world(state);
  generate_scenery(state);
  scenario_follow_camera(state);
  toggle_quest_journal(state);

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "crownless-campaign: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string capture_path =
      capture_dir + "\\crownless-campaign-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, capture_path),
                 "crownless-campaign: Act IV Framekit journal captured");
  std::printf("    capture: %s\n", capture_path.c_str());

  bool act = false;
  bool active = false;
  bool objective = false;
  bool earlier = false;
  int completed = 0;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Hud &&
        item.label == "campaign-act:4:THE CROWNLESS MARCHES")
      act = true;
    if (item.op == render::Op::Hud &&
        item.label == "quest-active:white-harrow")
      active = true;
    if (item.op == render::Op::Hud &&
        item.label == "quest-objective:Defeat the White Harrow.")
      objective = true;
    if (item.op == render::Op::Hud && item.label == "quest-earlier-deeds" &&
        item.value == 5)
      earlier = true;
    if (item.op == render::Op::Hud &&
        item.label.rfind("quest-complete:", 0) == 0)
      ++completed;
  }
  scenario_check(act && active && objective && earlier && completed == 9,
                 "crownless-campaign: expanded act and bounded House record remain readable");
  const HudRect expected = quest_journal_rect(1366, 768);
  bool inside = expected.x >= 0 && expected.y >= 0 &&
                expected.x + expected.w <= 1366 &&
                expected.y + expected.h <= 768;
  for (const auto& trace : state.hud_rect_trace)
    if (trace.first.rfind("quest-pane", 0) == 0)
      inside = inside && trace.second.x >= expected.x &&
               trace.second.y >= expected.y &&
               trace.second.x + trace.second.w <= expected.x + expected.w &&
               trace.second.y + trace.second.h <= expected.y + expected.h;
  scenario_check(inside,
                 "crownless-campaign: twenty-three-point Chronicle fits Framekit");
  return 0;
}

int scenario_verdigris_crown_campaign() {
  ClientState state;
  state.session = std::make_unique<ScenarioCampaignSession>(true, true, true);
  load_billboards(state.billboards);
  sync_world(state);
  generate_scenery(state);
  scenario_follow_camera(state);
  toggle_quest_journal(state);

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "verdigris-crown-campaign: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string compact_path =
      capture_dir + "\\verdigris-crown-campaign-960x600.png";
  scenario_check(reference_present(state, 960, 600, compact_path),
                 "verdigris-crown-campaign: compact Act VI journal captured");
  std::printf("    capture: %s\n", compact_path.c_str());
  const std::string wide_path =
      capture_dir + "\\verdigris-crown-campaign-1366x768.png";
  scenario_check(reference_present(state, 1366, 768, wide_path),
                 "verdigris-crown-campaign: wide Act VI journal captured");
  std::printf("    capture: %s\n", wide_path.c_str());

  bool act = false;
  bool active = false;
  bool objective = false;
  bool earlier = false;
  int completed = 0;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Hud &&
        item.label == "campaign-act:6:THE VERDIGRIS CROWN")
      act = true;
    if (item.op == render::Op::Hud &&
        item.label == "quest-active:crown-without-king")
      active = true;
    if (item.op == render::Op::Hud &&
        item.label == "quest-objective:Enter the Empty Throne on the tier-five Copper Road.")
      objective = true;
    if (item.op == render::Op::Hud && item.label == "quest-earlier-deeds" &&
        item.value == 13)
      earlier = true;
    if (item.op == render::Op::Hud &&
        item.label.rfind("quest-complete:", 0) == 0)
      ++completed;
  }
  scenario_check(act && active && objective && earlier && completed == 9,
                 "verdigris-crown-campaign: six-part finale and 22 deeds are readable");
  const HudRect compact = quest_journal_rect(960, 600);
  const HudRect wide = quest_journal_rect(1366, 768);
  bool top_hud_leaked = false;
  for (const auto& trace : state.hud_rect_trace)
    if (trace.first == "objective" || trace.first == "controls" ||
        trace.first == "controls-second" || trace.first == "identity" ||
        trace.first == "connection" || trace.first == "art")
      top_hud_leaked = true;
  scenario_check(compact.x >= 0 && compact.y >= 0 &&
                     compact.x + compact.w <= 960 &&
                     compact.y + compact.h <= 600 && wide.x >= 0 &&
                     wide.y >= 0 && wide.x + wide.w <= 1366 &&
                     wide.y + wide.h <= 768 && !top_hud_leaked,
                 "verdigris-crown-campaign: modal Framekit fits without top-HUD overpaint");
  return 0;
}

int scenario_tactical_map_overlay() {
  ClientState state;
  scenario_begin(state);
  load_billboards(state.billboards);
  state.simulation.reset();  // hold the authored navigation fixture still

  constexpr int kWidth = 1366;
  constexpr int kHeight = 768;
  constexpr int kMapSize = 64;
  state.world.player.position = {32 * static_cast<int>(kTileUnits),
                                 32 * static_cast<int>(kTileUnits)};
  state.world.player.facing = {1, -1};
  state.world.player.life = 94;
  state.world.player.life_max = 110;
  state.world.player.resource = 37;
  state.world.player.resource_max = 50;
  state.world.house_name = "House Emberwake";
  state.world.scion_name = "Ilyra";
  state.world.route_id = "route:copper:3:2";
  state.world.theme = "crypt";
  state.world.map_width = kMapSize;
  state.world.map_height = kMapSize;
  state.world.map_walkable.assign(kMapSize * kMapSize, 0);
  const auto open = [&](int x, int y) {
    if (x >= 0 && x < kMapSize && y >= 0 && y < kMapSize)
      state.world.map_walkable[static_cast<std::size_t>(y) * kMapSize + x] = 1;
  };
  // Crossroads, side chambers, and a loop produce recognizable topology
  // rather than a capture-only rectangle.
  for (int x = 18; x <= 48; ++x)
    for (int y = 30; y <= 34; ++y) open(x, y);
  for (int y = 14; y <= 50; ++y)
    for (int x = 30; x <= 34; ++x) open(x, y);
  for (int x = 20; x <= 27; ++x)
    for (int y = 22; y <= 27; ++y) open(x, y);
  for (int x = 39; x <= 46; ++x)
    for (int y = 38; y <= 45; ++y) open(x, y);
  for (int x = 23; x <= 43; ++x) {
    open(x, 18);
    open(x, 19);
    open(x, 47);
    open(x, 48);
  }
  for (int y = 18; y <= 48; ++y) {
    open(23, y);
    open(24, y);
    open(42, y);
    open(43, y);
  }
  state.world.has_extraction = true;
  state.world.extraction = {43 * static_cast<int>(kTileUnits),
                            43 * static_cast<int>(kTileUnits)};
  state.world.monsters.clear();
  WorldActor foe;
  foe.id = "map-foe";
  foe.name = "Ossuary Wight";
  foe.position = {36 * static_cast<int>(kTileUnits),
                  32 * static_cast<int>(kTileUnits)};
  foe.life = foe.life_max = 80;
  state.world.monsters.push_back(foe);
  WorldActor elite = foe;
  elite.id = "map-elite";
  elite.name = "Sepulchral Keeper";
  elite.position = {42 * static_cast<int>(kTileUnits),
                    39 * static_cast<int>(kTileUnits)};
  elite.elite = true;
  state.world.monsters.push_back(elite);
  state.scenery = {
      {SceneryKind::Shrine,
       {24 * static_cast<int>(kTileUnits),
        24 * static_cast<int>(kTileUnits)},
       kTileUnits * 0.3, 1.0, false},
      {SceneryKind::Ruin,
       {32 * static_cast<int>(kTileUnits),
        18 * static_cast<int>(kTileUnits)},
       kTileUnits * 0.3, 1.0, false},
  };

  state.gear_overlay = true;
  state.character_pane = true;
  state.tree_pane = true;
  toggle_minimap_overlay(state);
  scenario_check(state.minimap_mode == MinimapMode::Overlay &&
                     !state.gear_overlay && !state.character_pane &&
                     !state.tree_pane,
                 "tactical-map: opening the broad chart dismisses narrower panes");

  const int zoom_before = state.minimap_zoom_step;
  adjust_minimap_zoom(state, 1);
  scenario_check(state.minimap_zoom_step == zoom_before + 1,
                 "tactical-map: zoom setting advances through production control");
  adjust_minimap_zoom(state, -1);
  const int opacity_before = state.minimap_opacity_step;
  adjust_minimap_opacity(state, -1);
  scenario_check(state.minimap_opacity_step == opacity_before - 1,
                 "tactical-map: opacity setting advances through production control");
  adjust_minimap_opacity(state, 1);
  for (int i = 0; i < 12; ++i) adjust_minimap_zoom(state, 1);
  scenario_check(state.minimap_zoom_step == kMinimapSettingSteps - 1,
                 "tactical-map: zoom clamps at the supported maximum");
  state.minimap_zoom_step = kDefaultMinimapZoomStep;
  state.hint.clear();
  state.hint_ticks = 0;

  std::string capture_dir;
  const int capture_override = capture_root_override(&capture_dir);
  if (capture_override < 0) {
    scenario_check(false,
                   "tactical-map: capture root rejected before any write");
    return 0;
  }
  if (capture_override == 0) {
    CreateDirectoryA("captures", nullptr);
    capture_dir = "captures";
  }
  const std::string settings_path =
      capture_dir + "\\tactical-map-preferences.ini";
  state.minimap_side = MinimapSide::Right;
  state.minimap_zoom_step = 3;
  state.minimap_opacity_step = 1;
  const bool settings_written =
      write_minimap_preferences(state, settings_path);
  ClientState restored;
  const bool settings_read =
      read_minimap_preferences(restored, settings_path);
  scenario_check(settings_written && settings_read &&
                     restored.minimap_side == MinimapSide::Right &&
                     restored.minimap_zoom_step == 3 &&
                     restored.minimap_opacity_step == 1,
                 "tactical-map: side, zoom, and opacity survive a client restart");
  (void)DeleteFileA(settings_path.c_str());
  state.minimap_side = MinimapSide::Left;
  state.minimap_zoom_step = kDefaultMinimapZoomStep;
  state.minimap_opacity_step = kDefaultMinimapOpacityStep;
  const std::string capture_path =
      capture_dir + "\\tactical-map-overlay-1366x768.png";
  scenario_check(reference_present(state, kWidth, kHeight, capture_path),
                 "tactical-map: translucent Framekit overlay captured");
  std::printf("    capture: %s\n", capture_path.c_str());

  const render::Item* overlay_op = nullptr;
  bool controls = false;
  for (const auto& item : state.render_list) {
    if (item.op == render::Op::Minimap &&
        item.label.rfind("overlay:", 0) == 0)
      overlay_op = &item;
    if (item.op == render::Op::Hud && item.label.rfind("map-control:", 0) == 0)
      controls = true;
  }
  scenario_check(overlay_op &&
                     overlay_op->label.find(":cells=0") == std::string::npos,
                 "tactical-map: overlay renders authoritative walkability topology");
  scenario_check(controls,
                 "tactical-map: zoom, opacity, side, and close controls are discoverable");
  const HudRect expected = minimap_overlay_rect(kWidth, kHeight);
  scenario_check(expected.x >= 0 && expected.y >= 0 &&
                     expected.x + expected.w <= kWidth &&
                     expected.y + expected.h <= kHeight - 96,
                 "tactical-map: overlay clears the viewport and combat HUD");

  toggle_minimap_overlay(state);
  swap_minimap_side(state);
  reference_present(state, kWidth, kHeight, "");
  const render::Item* corner = render::first(state.render_list, render::Op::Minimap);
  scenario_check(corner && corner->x > kWidth / 2 &&
                     corner->label.rfind("corner:right", 0) == 0,
                 "tactical-map: compact map honors right-side placement");

  toggle_minimap_overlay(state);
  state.quit_requested = false;
  handle_escape_key(state);
  scenario_check(state.minimap_mode == MinimapMode::Corner &&
                     !state.quit_requested,
                 "tactical-map: Escape closes the chart before requesting exit");
  return 0;
}

// Machine-checkable presentation budget: paints real fullscreen-sized 32bpp
// frames through the production paint_scene path and fails when the average
// frame cost would visibly stutter the 20 Hz tick. The bound is deliberately
// generous (regressions of the kind this gate exists for cost hundreds of
// milliseconds); the measured value prints so drift is visible in every run.
int scenario_scene_transition_hygiene() {
  WorldView world;
  verdigris::client::PresentationFx fx;
  fx.effects.push_back(EffectFx{});
  fx.telegraphs["departed-foe"] = ActiveTelegraph{};
  fx.loot_positions["departed-loot"] = {20, 22};
  fx.known_monsters.insert("departed-foe");
  fx.monster_strikes["departed-foe"] = 10;
  fx.last_death_pos = {4, 5};
  fx.loot_scatter = 2;
  fx.screen_pulse_ticks = 3;
  fx.event_log.push_back("Road cleared");
  fx.hint = "Safe return";
  verdigris::client::PresentationEvent event;
  event.type = verdigris::client::PresentationEventType::SceneChanged;
  apply_presentation_event(fx, world, event, 11);
  scenario_check(fx.effects.empty() && fx.telegraphs.empty() &&
                     fx.loot_positions.empty() && fx.known_monsters.empty() &&
                     fx.monster_strikes.empty() && fx.last_death_pos.x == 0 &&
                     fx.last_death_pos.y == 0 && fx.loot_scatter == 0 &&
                     fx.screen_pulse_ticks == 0,
                 "scene-transition-hygiene: world-space remnants are retired");
  scenario_check(fx.event_log.size() == 1 && fx.hint == "Safe return",
                 "scene-transition-hygiene: durable guidance survives");
  return scenario_failures;
}

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

int scenario_warning_transparency() {
  skin::ensure_started();
  BITMAPINFO info{};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = 960;
  info.bmiHeader.biHeight = -600;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  void* bits = nullptr;
  HBITMAP bitmap = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
  HDC dc = CreateCompatibleDC(nullptr);
  HGDIOBJ old = SelectObject(dc, bitmap);
  RECT bounds{0, 0, 960, 600};
  Camera camera;
  camera.x = camera.y = 0;
  camera.zoom = 1;
  const auto center = project(camera, bounds, 0, 0);
  ActiveTelegraph warning;
  warning.position = {0, 0};
  warning.facing = {1, 0};
  warning.action = "sweep";
  warning.shape = "circle";
  warning.radius_tiles = 4;
  render::List list;
  for (const COLORREF ground : {RGB(180, 190, 150), RGB(50, 100, 70)}) {
    const auto reset = [&] {
      HBRUSH brush = CreateSolidBrush(ground);
      FillRect(dc, &bounds, brush);
      DeleteObject(brush);
      list.clear();
    };
    reset();
    draw_sweep_telegraph(dc, camera, bounds, warning, 1, 80,
                         MinimapSide::Left, list);
    const COLORREF sample = GetPixel(dc, center.x, center.y);
    const int expected_green = (GetGValue(ground) * 203 + 52 * 52) / 255;
    scenario_check(std::abs(static_cast<int>(GetGValue(sample)) - expected_green) <= 3,
                   "warning-transparency: circular warning preserves terrain through real alpha");
    reset();
    warning.shape = "ring";
    warning.inner_radius_tiles = 2;
    draw_sweep_telegraph(dc, camera, bounds, warning, 1, 80,
                         MinimapSide::Left, list);
    scenario_check(GetPixel(dc, center.x, center.y) == ground,
                   "warning-transparency: Warden safe eye leaves terrain untouched");
    reset();
    draw_thrust_telegraph(dc, camera, bounds, warning, 0, 80,
                          MinimapSide::Left, list);
    // Stay off the edge and centerline, inside the forward half-plane.
    scenario_check(GetPixel(dc, center.x + 25, center.y + 10) == ground,
                   "warning-transparency: zero-opacity fan leaves terrain untouched");
    reset();
    draw_thrust_telegraph(dc, camera, bounds, warning, 1, 80,
                          MinimapSide::Left, list);
    scenario_check(std::abs(static_cast<int>(GetGValue(
                       GetPixel(dc, center.x + 25, center.y + 10))) - expected_green) <= 3,
                   "warning-transparency: thrust fan also blends with terrain");
    warning.shape = "circle";
    warning.inner_radius_tiles = 0;
  }
  SelectObject(dc, old);
  DeleteObject(bitmap);
  DeleteDC(dc);
  return 0;
}

class ScenarioPointerSession final : public verdigris::client::IClientSession {
 public:
  verdigris::client::ClientModel data;
  std::vector<verdigris::client::ClientCommand> commands;
  std::string error;
  verdigris::client::ConnectionState status = verdigris::client::ConnectionState::Ready;
  bool start(std::string*) override { return true; }
  void shutdown() override {}
  void poll() override {}
  void submit(const verdigris::client::ClientCommand& command) override {
    commands.push_back(command);
  }
  verdigris::client::ConnectionState connection_state() const override {
    return status;
  }
  const verdigris::client::ClientModel& model() const override { return data; }
  const std::string& last_error() const override { return error; }
  std::vector<verdigris::client::PresentationEvent> drain_events() override { return {}; }
};

int scenario_combat_input_feedback() {
  ClientState state;
  auto session = std::make_unique<ScenarioPointerSession>();
  auto* recorder = session.get();
  recorder->data.scene.type = "instance";
  recorder->data.player.uuid = "feedback-scion";
  state.session = std::move(session);
  const SkillInfo thrust{'Q', "Thrust", verdigris::ActionType::Thrust};
  auto press = [&] {
    ++state.world.tick;
    state.effects.clear();
    recorder->commands.clear();
    dispatch_skill(state, thrust);
    scenario_check(recorder->commands.size() == 1 &&
                       recorder->commands.front().target == "thrust",
                   "combat-feedback: input remains a server-authoritative request");
    return !state.effects.empty();
  };
  scenario_check(press(), "combat-feedback: eligible whiff has immediate swing feedback");
  recorder->data.scene.type = "town";
  scenario_check(!press(), "combat-feedback: town input cannot draw a phantom attack");
  recorder->data.scene.type = "instance";
  recorder->data.player.alive = false;
  scenario_check(!press(), "combat-feedback: dead Scion cannot draw a phantom attack");
  recorder->data.player.alive = true;
  recorder->data.player.cooldown_ticks = 3;
  scenario_check(!press(), "combat-feedback: recovery cannot draw a phantom attack");
  recorder->data.player.cooldown_ticks = 0;
  recorder->data.player.resource = 0;
  scenario_check(!press(), "combat-feedback: unfunded skill cannot draw a phantom attack");
  recorder->data.player.resource = 50;
  recorder->status = verdigris::client::ConnectionState::Connected;
  scenario_check(!press(), "combat-feedback: unadmitted session cannot draw a phantom attack");
  recorder->status = verdigris::client::ConnectionState::Ready;
  scenario_check(press(), "combat-feedback: ready skill resumes immediate feedback");
  dispatch_skill(state, thrust);
  scenario_check(state.effects.size() == 1,
                 "combat-feedback: repeated input in one tick remains bounded");
  return 0;
}

int scenario_inventory_pointer_safety() {
  ClientState state;
  auto session = std::make_unique<ScenarioPointerSession>();
  auto* recorder = session.get();
  recorder->data.player.uuid = "pointer-scion";
  recorder->data.player.alive = true;
  verdigris::client::ClientItemSlot item;
  item.id = "bronze-sword";
  item.uuid = "clicked-weapon";
  item.name = "Bronze Sword";
  item.slot = 0;
  item.width = 2;
  item.height = 3;
  recorder->data.inventory.push_back(item);
  item.uuid = "other-weapon";
  item.slot = 5;
  recorder->data.inventory.push_back(item);
  state.session = std::move(session);
  state.gear_overlay = true;
  sync_world(state);
  load_billboards(state.billboards);
  for (const auto size : {POINT{960, 600}, POINT{1366, 768}}) {
    reference_present(state, size.x, size.y, "");
    scenario_check(state.inventory_hits.size() == 2,
                   "inventory-pointer: painter exposes both exact item footprints");
    if (state.inventory_hits.size() != 2) continue;
    const auto hit = state.inventory_hits.front();
    recorder->commands.clear();
    state.selected_item = 1;
    activate_inventory_at(state, hit.rect.right - 2, hit.rect.bottom - 2);
    scenario_check(recorder->commands.size() == 1 &&
                       recorder->commands.back().type == verdigris::client::ClientCommand::Type::Equip &&
                       recorder->commands.back().target == "clicked-weapon",
                   "inventory-pointer: click uses whole footprint and UUID, not prior selection");
    recorder->commands.clear();
    activate_inventory_at(state, 1, 1);
    const auto pane = gear_pane_rect(size.x, size.y);
    activate_inventory_at(state, pane.x + 15, pane.y + 15);
    scenario_check(recorder->commands.empty(),
                   "inventory-pointer: world and pane chrome cannot equip a selected item");
    std::swap(recorder->data.inventory[0], recorder->data.inventory[1]);
    activate_inventory_at(state, hit.rect.left + 2, hit.rect.top + 2);
    scenario_check(recorder->commands.size() == 1 &&
                       recorder->commands.back().target == "clicked-weapon",
                   "inventory-pointer: snapshot reorder cannot change clicked identity");
    std::swap(recorder->data.inventory[0], recorder->data.inventory[1]);
  }
  const auto stale_hit = state.inventory_hits.front();
  recorder->data.inventory.erase(recorder->data.inventory.begin());
  recorder->commands.clear();
  activate_inventory_at(state, stale_hit.rect.left + 2, stale_hit.rect.top + 2);
  scenario_check(recorder->commands.empty(),
                 "inventory-pointer: removed item cannot equip or consume its replacement");
  const RECT bounds{0, 0, 1366, 768};
  state.gear_overlay = false;
  state.character_pane = true;
  const auto pane = character_pane_rect(1366, 768);
  scenario_check(pointer_ui_blocks_world(state, bounds, pane.x + 20, pane.y + 20) &&
                     !pointer_ui_blocks_world(state, bounds, 700, 400),
                 "inventory-pointer: character pane consumes mouse actions but exposed world stays interactive");
  state.quest_journal = true;
  scenario_check(pointer_ui_blocks_world(state, bounds, 700, 400),
                 "inventory-pointer: journal prevents mouse attacks and dashes behind it");
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
      {"warning-transparency", scenario_warning_transparency},
      {"inventory-pointer-safety", scenario_inventory_pointer_safety},
      {"combat-juice", scenario_combat_juice},
      {"combat-cadence", scenario_combat_cadence},
      {"combat-input-feedback", scenario_combat_input_feedback},
      {"monster-pressure-roles", scenario_monster_pressure_roles},
      {"warden-disciplines", scenario_warden_disciplines},
      {"remote-render-list", scenario_remote_render_list},
      {"zoom-invariance", scenario_zoom_invariance},
      {"chronicles-gate-b", scenario_chronicles_gate_b},
      {"chronicles-lineage-ui", scenario_chronicles_lineage_ui},
      {"title-island", scenario_title_island},
      {"guided-creation", scenario_guided_creation},
      {"first-session-clarity", scenario_first_session_clarity},
      {"animation-vfx-phase-a", scenario_animation_vfx_phase_a},
      {"progression-surface", scenario_progression_surface},
      {"hud-pane-readability", scenario_hud_pane_readability},
      {"hud-information", scenario_hud_information},
      {"endgame-tablet-ui", scenario_endgame_tablet_ui},
      {"character-inventory-diptych", scenario_character_inventory_diptych},
      {"framekit-pane-tiling", scenario_framekit_pane_tiling},
      {"vesselforge-active-properties", scenario_vesselforge_active_properties},
      {"vesselforge-final-implicits", scenario_vesselforge_final_implicits},
      {"town-social-hub", scenario_town_social_hub},
      {"town-vesselforge", scenario_town_vesselforge},
      {"campaign-journal", scenario_campaign_journal},
      {"deep-roads-campaign", scenario_deep_roads_campaign},
      {"crownless-campaign", scenario_crownless_campaign},
      {"verdigris-crown-campaign", scenario_verdigris_crown_campaign},
      {"tactical-map", scenario_tactical_map_overlay},
      {"scene-transition-hygiene", scenario_scene_transition_hygiene},
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
  state->title_open = state->title_enabled = chronicles_mode;
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
  load_minimap_preferences(*state);

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
  load_minimap_preferences(*state);

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
