#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "verdigris/core.hpp"
#include "verdigris/seasonal.hpp"

#ifdef VERDIGRIS_NATIVE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kTileUnits = 100.0;  // one move command covers 100 world units
// These costs are presentation metadata only.  The simulation remains the
// authority for affordability and resource mutation.
constexpr int kThrustResourceCost = 10;
constexpr int kSweepResourceCost = 15;
constexpr int kWarCryResourceCost = 20;

// Adjustable 2.5D camera: pitch foreshortens the ground plane, zoom scales
// world units to pixels, and perspective grows near rows / shrinks far rows.
struct Camera {
  double x = 0.0;
  double y = 0.0;
  double zoom = 0.5;          // pixels per world unit
  double pitch_deg = 55.0;    // 90 = top-down, lower = flatter horizon
  double perspective = 0.00035;  // depth scale per world unit toward the viewer

  double ground_squash() const { return std::cos(pitch_deg * kPi / 180.0); }
  double depth_scale(double rel_y) const {
    return std::clamp(1.0 + rel_y * perspective, 0.65, 1.55);
  }
};

struct ScreenPoint {
  int x = 0;
  int y = 0;
  double scale = 1.0;
};

struct EffectFx {
  enum class Kind {
    Swing,
    SweepArc,
    WarCryAura,
    Impact,
    DeathRing,
    Dust,
    Sparkle
  };
  Kind kind = Kind::Impact;
  double wx = 0.0;
  double wy = 0.0;
  double angle = 0.0;
  int age = 0;
  int ttl = 8;
};

struct ClientState {
  std::unique_ptr<verdigris::Simulation> simulation;
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
  std::unordered_map<std::string, verdigris::Vec2> loot_positions;
  verdigris::Vec2 last_death_pos{0, 0};
  std::size_t processed_events = 0;
  std::vector<std::string> event_log;
  int loot_scatter = 0;
  bool loot_labels = false;
  bool gear_overlay = false;
  std::size_t selected_item = 0;
  std::string hint;
  int hint_ticks = 0;
};

ClientState* state_from(HWND window) {
  return reinterpret_cast<ClientState*>(GetWindowLongPtr(window, GWLP_USERDATA));
}

void show_hint(ClientState& state, const std::string& message) {
  state.hint = message;
  state.hint_ticks = 80;
}

struct SkillInfo {
  char key;
  const char* name;
  verdigris::ActionType action;
  int resource_cost;
};

constexpr SkillInfo kSkills[] = {
    {'Q', "Thrust", verdigris::ActionType::Thrust, kThrustResourceCost},
    {'E', "Sweep", verdigris::ActionType::Sweep, kSweepResourceCost},
    {'R', "WarCry", verdigris::ActionType::WarCry, kWarCryResourceCost},
};

const SkillInfo* skill_for_key(WPARAM key) {
  for (const auto& skill : kSkills)
    if (key == static_cast<WPARAM>(skill.key)) return &skill;
  return nullptr;
}

void dispatch_skill(ClientState& state, const SkillInfo& skill) {
  // Do not duplicate target/range/cooldown rules in the client.  A key press
  // is a presentation request; the core decides whether it resolves.
  state.simulation->dispatch(verdigris::Command::action_use(skill.action));
  show_hint(state, std::string(skill.name) + " requested");
}

std::string nearest_pickup_id(const ClientState& state) {
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
  const auto& items = state.simulation->scion().carried_items;
  if (items.empty()) {
    show_hint(state, "Gear empty: pick up an item first");
    return;
  }
  state.selected_item = std::min(state.selected_item, items.size() - 1);
  state.simulation->dispatch(verdigris::Command::equip(items[state.selected_item].id));
  show_hint(state, "Equipped " + items[state.selected_item].name);
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
  const double rel_x = wx - camera.x;
  const double rel_y = wy - camera.y;
  const double depth = camera.depth_scale(rel_y);
  ScreenPoint out;
  out.x = bounds.right / 2 + static_cast<int>(rel_x * camera.zoom * depth);
  out.y = bounds.bottom / 2 +
          static_cast<int>(rel_y * camera.zoom * camera.ground_squash() * depth);
  out.scale = camera.zoom * depth;
  return out;
}

// Inverse of project() on the ground plane, good enough for mouse aim.
void unproject(const Camera& camera, const RECT& bounds, int sx, int sy, double& wx,
               double& wy) {
  const double squash = std::max(0.08, camera.ground_squash());
  double rel_y = (sy - bounds.bottom / 2) / (camera.zoom * squash);
  rel_y = (sy - bounds.bottom / 2) / (camera.zoom * squash * camera.depth_scale(rel_y));
  const double rel_x =
      (sx - bounds.right / 2) / (camera.zoom * camera.depth_scale(rel_y));
  wx = camera.x + rel_x;
  wy = camera.y + rel_y;
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

void draw_contact_shadow(HDC dc, const Camera& camera, const ScreenPoint& base,
                         double world_radius) {
  const int rx = std::max(3, static_cast<int>(world_radius * base.scale));
  const int ry = std::max(
      2, static_cast<int>(world_radius * base.scale * camera.ground_squash() * 0.8));
  fill_ellipse(dc, base.x, base.y, rx, ry, RGB(14, 18, 20));
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

void draw_ground_grid(HDC dc, const Camera& camera, const RECT& bounds) {
  const double range = 8.0 * kTileUnits;
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

void draw_effect(HDC dc, const Camera& camera, const RECT& bounds, const EffectFx& fx) {
  const ScreenPoint base = project(camera, bounds, fx.wx, fx.wy);
  const double life = 1.0 - static_cast<double>(fx.age) / fx.ttl;
  const double grow = static_cast<double>(fx.age) / fx.ttl;
  switch (fx.kind) {
    case EffectFx::Kind::Swing: {
      // A readable melee arc sweeping toward the aim angle.
      const int radius = static_cast<int>(kTileUnits * 1.1 * base.scale);
      const COLORREF color = fade_to_background(RGB(226, 220, 180), life);
      const double spread = kPi * 0.45;
      const double sweep = fx.angle - spread * 0.5 + spread * grow;
      for (int i = 0; i < 3; ++i) {
        const double a = sweep - i * 0.12;
        const int x1 = base.x + static_cast<int>(std::cos(a) * radius);
        const int y1 = base.y - static_cast<int>(kTileUnits * 0.7 * base.scale) +
                       static_cast<int>(std::sin(a) * radius * camera.ground_squash());
        draw_line(dc, base.x,
                  base.y - static_cast<int>(kTileUnits * 0.7 * base.scale), x1, y1,
                  color, i == 0 ? 3 : 1);
      }
      break;
    }
    case EffectFx::Kind::SweepArc: {
      // Sweep is an area action in the core.  A complete ellipse keeps the
      // presentation honest about that area instead of implying a single
      // facing direction that the deterministic action does not own.
      const int radius = static_cast<int>(kTileUnits * (0.72 + grow * 0.62) *
                                          base.scale);
      const int ry = std::max(3, static_cast<int>(radius * camera.ground_squash()));
      const COLORREF color = fade_to_background(RGB(116, 204, 208), life);
      ring_ellipse(dc, base.x, base.y, radius, ry, color, 3);
      if (radius > 8)
        ring_ellipse(dc, base.x, base.y, radius - 7, std::max(2, ry - 4), color, 1);
      break;
    }
    case EffectFx::Kind::WarCryAura: {
      // A short-lived, expanding aura communicates the buff event without
      // turning the renderer into a second source of gameplay state.
      const int radius = static_cast<int>(kTileUnits * (0.38 + grow * 0.72) *
                                          base.scale);
      const int ry = std::max(3, static_cast<int>(radius * camera.ground_squash()));
      const COLORREF color = fade_to_background(RGB(239, 190, 78), life);
      ring_ellipse(dc, base.x, base.y, radius, ry, color, 3);
      ring_ellipse(dc, base.x, base.y, std::max(3, radius - 8),
                   std::max(2, ry - 4), fade_to_background(RGB(255, 224, 128), life), 1);
      break;
    }
    case EffectFx::Kind::Impact: {
      const int r = std::max(4, static_cast<int>(kTileUnits * 0.35 * base.scale));
      fill_ellipse(dc, base.x,
                   base.y - static_cast<int>(kTileUnits * 0.7 * base.scale), r, r,
                   fade_to_background(RGB(255, 214, 120), life));
      break;
    }
    case EffectFx::Kind::DeathRing: {
      const int rx = static_cast<int>(kTileUnits * (0.3 + grow * 1.5) * base.scale);
      const int ry = static_cast<int>(rx * camera.ground_squash());
      ring_ellipse(dc, base.x, base.y, rx, std::max(2, ry),
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
      const int lift = static_cast<int>(kTileUnits * 0.5 * base.scale);
      const COLORREF color = fade_to_background(RGB(240, 214, 120), life);
      draw_line(dc, base.x - r, base.y - lift, base.x + r, base.y - lift, color, 1);
      draw_line(dc, base.x, base.y - lift - r, base.x, base.y - lift + r, color, 1);
      break;
    }
  }
}

const char* event_label(verdigris::EventType type) {
  using verdigris::EventType;
  switch (type) {
    case EventType::AttackStarted: return "attack";
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
  const auto* player = state.simulation->actor(state.simulation->scion().actor_id);
  if (!player || !player->alive) return;
  const verdigris::Vec2 direction = quantized_mouse_aim(state, bounds, *player);
  if (direction.x == 0 && direction.y == 0) return;
  if (!force && state.aim_direction_initialized &&
      state.last_aim_direction.x == direction.x &&
      state.last_aim_direction.y == direction.y) {
    return;
  }
  // The quantized heading is the only client-side state used for throttling;
  // facing and all consequences remain authoritative in the core.
  state.simulation->dispatch(verdigris::Command::aim(direction.x, direction.y));
  state.last_aim_direction = direction;
  state.aim_direction_initialized = true;
}

// Turn new simulation events into procedural presentation effects.
void ingest_events(ClientState& state, const RECT& bounds) {
  const auto& sim = *state.simulation;
  const auto& events = sim.events();
  for (; state.processed_events < events.size(); ++state.processed_events) {
    const auto& event = events[state.processed_events];
    const verdigris::Actor* subject =
        event.actor_id.empty() ? nullptr : sim.actor(event.actor_id);
    const double ex = subject ? subject->position.x : state.last_death_pos.x;
    const double ey = subject ? subject->position.y : state.last_death_pos.y;
    switch (event.type) {
      case verdigris::EventType::AttackStarted:
        state.effects.push_back({event.text == "sweep" ? EffectFx::Kind::SweepArc
                                                         : EffectFx::Kind::Swing,
                                 ex, ey, aim_angle(state, bounds, ex, ey), 0,
                                 event.text == "sweep" ? 8 : 6});
        break;
      case verdigris::EventType::BuffApplied:
        if (event.text == "war-cry")
          state.effects.push_back({EffectFx::Kind::WarCryAura, ex, ey, 0.0, 0, 14});
        break;
      case verdigris::EventType::DamageApplied:
        state.effects.push_back({EffectFx::Kind::Impact, ex, ey, 0.0, 0, 4});
        break;
      case verdigris::EventType::ActorDied:
        if (subject) state.last_death_pos = subject->position;
        state.effects.push_back({EffectFx::Kind::DeathRing, ex, ey, 0.0, 0, 12});
        state.effects.push_back({EffectFx::Kind::Dust, ex, ey, 0.7, 0, 10});
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
}

struct DepthDraw {
  double depth = 0.0;
  int order = 0;
  enum class What { Player, Monster, Loot, Effect } what = What::Player;
  std::size_t index = 0;
};

std::string loot_label(const verdigris::Simulation& simulation,
                       const std::string& id) {
  for (const auto& item : simulation.ground_items())
    if (item.id == id) return item.name;
  for (const auto& trophy : simulation.ground_trophies())
    if (trophy.id == id) return trophy.name;
  return id;
}

void paint_gear_overlay(const ClientState& state, HDC dc, const RECT& bounds) {
  if (!state.gear_overlay) return;
  const int left = std::max(24, static_cast<int>(bounds.right) - 360);
  const int top = 118;
  const int right = static_cast<int>(bounds.right) - 24;
  const int bottom = std::min(static_cast<int>(bounds.bottom) - 36, top + 300);
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
  SetTextColor(dc, RGB(230, 235, 220));
  const std::string title = "Gear / House  |  " + state.simulation->house().name;
  TextOutA(dc, left + 16, top + 14, title.c_str(), static_cast<int>(title.size()));
  const auto& items = state.simulation->scion().carried_items;
  if (items.empty()) {
    const char* empty = "No carried items. X picks up the nearest drop.";
    TextOutA(dc, left + 16, top + 48, empty, static_cast<int>(strlen(empty)));
  } else {
    for (std::size_t i = 0; i < items.size(); ++i) {
      const auto& item = items[i];
      const bool selected = i == std::min(state.selected_item, items.size() - 1);
      SetTextColor(dc, selected ? RGB(239, 208, 116) : RGB(205, 215, 204));
      std::string line = (selected ? "> " : "  ") + item.name +
                         (item.equipped ? "  [equipped]" : "");
      TextOutA(dc, left + 16, top + 48 + static_cast<int>(i) * 22, line.c_str(),
               static_cast<int>(line.size()));
    }
  }
  SetTextColor(dc, RGB(150, 170, 158));
  const char* controls = "Up/Down select | Enter or LMB equip | I close";
  TextOutA(dc, left + 16, bottom - 28, controls, static_cast<int>(strlen(controls)));
}

void paint_skill_strip(const ClientState& state, HDC dc) {
  const auto* player =
      state.simulation->actor(state.simulation->scion().actor_id);
  for (int i = 0; i < 3; ++i) {
    const SkillInfo& skill = kSkills[i];
    const bool cooldown =
        player && skill.action != verdigris::ActionType::WarCry &&
        player->cooldown_ticks > 0;
    const bool affordable =
        player && player->stats.resource >= skill.resource_cost;
    const bool available = player && affordable && !cooldown;
    const bool active = player && skill.action == verdigris::ActionType::WarCry &&
                        player->war_cry_ticks_remaining > 0;
    const int left = 18 + i * 116;
    RECT slot{left, 202, left + 106, 256};
    HBRUSH fill = CreateSolidBrush(available ? RGB(35, 42, 44) : RGB(29, 33, 34));
    FillRect(dc, &slot, fill);
    DeleteObject(fill);
    HPEN border = CreatePen(PS_SOLID, 1,
                            available ? RGB(86, 116, 104) : RGB(63, 70, 68));
    HGDIOBJ old_pen = SelectObject(dc, border);
    HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(dc, slot.left, slot.top, slot.right, slot.bottom);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(border);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, available ? RGB(239, 208, 116) : RGB(112, 119, 115));
    TextOutA(dc, left + 8, slot.top + 7, &skill.key, 1);
    SetTextColor(dc, available ? RGB(205, 221, 207) : RGB(112, 119, 115));
    const std::string name_and_cost = std::string(skill.name) + "  " +
                                      std::to_string(skill.resource_cost);
    TextOutA(dc, left + 25, slot.top + 7, name_and_cost.c_str(),
             static_cast<int>(name_and_cost.size()));
    std::string state_text;
    if (active) {
      state_text = "active " + std::to_string(player->war_cry_ticks_remaining);
    } else if (cooldown) {
      state_text = "cooldown " + std::to_string(player->cooldown_ticks);
    } else if (!affordable) {
      state_text = "need " + std::to_string(skill.resource_cost);
    } else {
      state_text = "ready";
    }
    TextOutA(dc, left + 25, slot.top + 29, state_text.c_str(),
             static_cast<int>(state_text.size()));
  }
}

void paint_resource_hud(const verdigris::Actor* player, HDC dc) {
  if (!player) return;
  constexpr int left = 18;
  constexpr int width = 240;
  constexpr int height = 8;
  const auto draw_bar = [&](int y, const char* label, int value, int maximum,
                            COLORREF color) {
    const int bounded_maximum = std::max(1, maximum);
    const double ratio = std::clamp(static_cast<double>(value) / bounded_maximum,
                                    0.0, 1.0);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(185, 198, 188));
    const std::string caption = std::string(label) + " " + std::to_string(value) +
                                "/" + std::to_string(maximum);
    TextOutA(dc, left, y - 15, caption.c_str(), static_cast<int>(caption.size()));
    RECT background{left, y, left + width, y + height};
    HBRUSH back_brush = CreateSolidBrush(RGB(45, 51, 50));
    FillRect(dc, &background, back_brush);
    DeleteObject(back_brush);
    RECT fill{left, y, left + static_cast<int>(width * ratio), y + height};
    HBRUSH fill_brush = CreateSolidBrush(color);
    FillRect(dc, &fill, fill_brush);
    DeleteObject(fill_brush);
    HPEN border = CreatePen(PS_SOLID, 1, RGB(92, 104, 99));
    HGDIOBJ old_pen = SelectObject(dc, border);
    HGDIOBJ old_brush = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
    Rectangle(dc, background.left, background.top, background.right, background.bottom);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(border);
  };
  draw_bar(122, "LIFE", player->stats.life, player->stats.life_max,
           RGB(177, 82, 75));
  draw_bar(146, "RESOURCE", player->stats.resource, player->stats.resource_max,
           RGB(72, 168, 191));
}

void paint_scene(ClientState& state, HDC dc, const RECT& bounds) {
  const auto& sim = *state.simulation;
  HBRUSH background = CreateSolidBrush(RGB(23, 29, 32));
  FillRect(dc, &bounds, background);
  DeleteObject(background);

  draw_ground_grid(dc, state.camera, bounds);

  // Ground decals render before anything that stands on the plane.
  const auto& extraction = sim.instance().extraction_point;
  const ScreenPoint pad = project(state.camera, bounds, extraction.x, extraction.y);
  const int pad_rx = static_cast<int>(kTileUnits * 0.6 * pad.scale);
  const int pad_ry =
      std::max(3, static_cast<int>(pad_rx * state.camera.ground_squash()));
  fill_ellipse(dc, pad.x, pad.y, pad_rx, pad_ry, RGB(41, 62, 88));
  ring_ellipse(dc, pad.x, pad.y, pad_rx, pad_ry, RGB(88, 132, 190), 2);

  const verdigris::Actor* player = sim.actor(sim.scion().actor_id);

  // Collect every standing element, then draw back-to-front by world depth.
  std::vector<DepthDraw> order;
  if (player && player->alive)
    order.push_back({static_cast<double>(player->position.y), 0,
                     DepthDraw::What::Player, 0});
  const auto& actors = sim.actors();
  for (std::size_t i = 0; i < actors.size(); ++i) {
    if (actors[i].kind == verdigris::ActorKind::Monster && actors[i].alive)
      order.push_back({static_cast<double>(actors[i].position.y), 1,
                       DepthDraw::What::Monster, i});
  }
  std::vector<std::pair<std::string, verdigris::Vec2>> loot(
      state.loot_positions.begin(), state.loot_positions.end());
  std::sort(loot.begin(), loot.end(),
            [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
  for (std::size_t i = 0; i < loot.size(); ++i)
    order.push_back({static_cast<double>(loot[i].second.y), 2,
                     DepthDraw::What::Loot, i});
  for (std::size_t i = 0; i < state.effects.size(); ++i)
    order.push_back({state.effects[i].wy + 1.0, 3, DepthDraw::What::Effect, i});
  std::sort(order.begin(), order.end(), [](const DepthDraw& lhs, const DepthDraw& rhs) {
    if (lhs.depth != rhs.depth) return lhs.depth < rhs.depth;
    return lhs.order < rhs.order;
  });

  for (const auto& entry : order) {
    switch (entry.what) {
      case DepthDraw::What::Player: {
        const ScreenPoint base =
            project(state.camera, bounds, player->position.x, player->position.y);
        draw_contact_shadow(dc, state.camera, base, kTileUnits * 0.42);
        draw_billboard(dc, base, kTileUnits * 0.62, kTileUnits * 1.35,
                       RGB(84, 158, 128), RGB(140, 208, 172));
        // Draw the authoritative simulation facing, rather than a client-only
        // mouse hint. The core owns the integer heading used by Thrust.
        const double angle =
            std::atan2(static_cast<double>(player->facing.y),
                       static_cast<double>(player->facing.x));
        const int fx = base.x + static_cast<int>(std::cos(angle) * kTileUnits * 0.6 *
                                                 base.scale);
        const int fy = base.y + static_cast<int>(std::sin(angle) * kTileUnits * 0.6 *
                                                 base.scale *
                                                 state.camera.ground_squash());
        draw_line(dc, base.x, base.y, fx, fy, RGB(140, 208, 172), 2);
        break;
      }
      case DepthDraw::What::Monster: {
        const auto& monster = actors[entry.index];
        const ScreenPoint base =
            project(state.camera, bounds, monster.position.x, monster.position.y);
        draw_contact_shadow(dc, state.camera, base, kTileUnits * 0.42);
        draw_billboard(dc, base, kTileUnits * 0.68, kTileUnits * 1.25,
                       RGB(168, 84, 70), RGB(212, 122, 96));
        // Compact life bar instead of a menu: readable elite math at a glance.
        const int bar_w = static_cast<int>(kTileUnits * 0.7 * base.scale);
        const int bar_y =
            base.y - static_cast<int>(kTileUnits * 1.5 * base.scale);
        const double ratio =
            std::clamp(static_cast<double>(monster.stats.life) /
                           std::max(1, monster.stats.life_max),
                       0.0, 1.0);
        draw_line(dc, base.x - bar_w / 2, bar_y, base.x + bar_w / 2, bar_y,
                  RGB(52, 40, 38), 3);
        if (ratio > 0.0)
          draw_line(dc, base.x - bar_w / 2, bar_y,
                    base.x - bar_w / 2 + static_cast<int>(bar_w * ratio), bar_y,
                    RGB(214, 118, 86), 3);
        break;
      }
      case DepthDraw::What::Loot: {
        const auto& entry_loot = loot[entry.index];
        const ScreenPoint base = project(state.camera, bounds, entry_loot.second.x,
                                         entry_loot.second.y);
        draw_contact_shadow(dc, state.camera, base, kTileUnits * 0.2);
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
          const std::string label = loot_label(sim, entry_loot.first);
          SetBkMode(dc, TRANSPARENT);
          SetTextColor(dc, color);
          TextOutA(dc, base.x + r + 4, base.y - lift - r - 5, label.c_str(),
                   static_cast<int>(label.size()));
        }
        break;
      }
      case DepthDraw::What::Effect:
        draw_effect(dc, state.camera, bounds, state.effects[entry.index]);
        break;
    }
  }

  // HUD and debug overlay.
  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, RGB(230, 235, 220));
  const int life = player ? player->stats.life : 0;
  const std::string status =
      "House " + sim.house().name + " | Scion " + sim.scion().name + " | Life " +
      std::to_string(life) + " | Resource " +
      std::to_string(player ? player->stats.resource : 0) + " | Stored trophies " +
      std::to_string(sim.house().stored_trophies.size()) + " | Stored items " +
      std::to_string(sim.house().stored_items.size()) + " | Carried " +
      std::to_string(sim.scion().carried_items.size() +
                     sim.scion().carried_trophies.size());
  TextOutA(dc, 18, 16, status.c_str(), static_cast<int>(status.size()));
  const char* help =
      "WASD move | Mouse aim | LMB melee | RMB/Space dash | Q Thrust | E Sweep | R WarCry";
  TextOutA(dc, 18, 40, help, static_cast<int>(strlen(help)));
  const char* help2 =
      "X nearest pickup | Z loot labels | F contextual extract | I gear/House overlay";
  TextOutA(dc, 18, 64, help2, static_cast<int>(strlen(help2)));
  const char* camera_help =
      "Wheel zoom | PgUp/PgDn pitch | -/= perspective | Home reset camera";
  TextOutA(dc, 18, 88, camera_help, static_cast<int>(strlen(camera_help)));

  paint_resource_hud(player, dc);

  SetTextColor(dc, RGB(150, 160, 150));
  char debug_line[256];
  std::snprintf(debug_line, sizeof(debug_line),
                "tick %llu | zoom %.2f | pitch %.0f | persp %.5f | effects %zu",
                static_cast<unsigned long long>(sim.tick()), state.camera.zoom,
                state.camera.pitch_deg, state.camera.perspective,
                state.effects.size());
  TextOutA(dc, 18, 168, debug_line, static_cast<int>(strlen(debug_line)));
  if (state.hint_ticks > 0 && !state.hint.empty()) {
    SetTextColor(dc, RGB(239, 208, 116));
    TextOutA(dc, 18, 184, state.hint.c_str(), static_cast<int>(state.hint.size()));
  }
  paint_skill_strip(state, dc);
  paint_gear_overlay(state, dc, bounds);
  int log_y = bounds.bottom - 24;
  for (auto it = state.event_log.rbegin(); it != state.event_log.rend(); ++it) {
    TextOutA(dc, 18, log_y, it->c_str(), static_cast<int>(it->size()));
    log_y -= 20;
  }
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

  int dx = (state.d ? 1 : 0) - (state.a ? 1 : 0);
  int dy = (state.s ? 1 : 0) - (state.w ? 1 : 0);
  const bool moving = dx != 0 || dy != 0;
  if (moving)
    state.simulation->dispatch(verdigris::Command::move(dx, dy));
  else
    state.simulation->dispatch(verdigris::Command::action_use(verdigris::ActionType::Wait));

  // Movement owns facing while the actor is moving. Re-send the throttled
  // mouse heading on the first idle tick so aiming immediately takes over
  // again without sending a command every frame.
  dispatch_aim_if_changed(state, bounds, !moving && state.was_moving);
  state.was_moving = moving;

  ingest_events(state, bounds);

  for (auto& fx : state.effects) ++fx.age;
  state.effects.erase(std::remove_if(state.effects.begin(), state.effects.end(),
                                     [](const EffectFx& fx) { return fx.age >= fx.ttl; }),
                      state.effects.end());
  if (state.hint_ticks > 0) --state.hint_ticks;

  // The camera eases toward the player so dashes read as motion, not teleports.
  const auto* player = state.simulation->actor(state.simulation->scion().actor_id);
  if (player) {
    state.camera.x += (player->position.x - state.camera.x) * 0.2;
    state.camera.y += (player->position.y - state.camera.y) * 0.2;
  }
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
      if (wparam == 'W') state->w = true;
      if (wparam == 'A') state->a = true;
      if (wparam == 'S') state->s = true;
      if (wparam == 'D') state->d = true;
      if (wparam == VK_SPACE)
        state->simulation->dispatch(verdigris::Command::action_use(verdigris::ActionType::Dash));
      if (const SkillInfo* skill = skill_for_key(wparam)) dispatch_skill(*state, *skill);
      if (wparam == 'X') {
        const std::string target = nearest_pickup_id(*state);
        if (target.empty()) {
          show_hint(*state, "No ground item or trophy nearby");
        } else {
          state->simulation->dispatch(verdigris::Command::pick_up(target));
          show_hint(*state, "Picked up nearest drop");
        }
      }
      if (wparam == 'Z') {
        state->loot_labels = !state->loot_labels;
        show_hint(*state, state->loot_labels ? "Loot names shown" : "Loot names hidden");
      }
      if (wparam == 'F') {
        // Ask the simulation to resolve the contextual interaction.  The
        // core owns the extraction pad/range rule; the client does not mirror
        // that gameplay decision.
        state->simulation->dispatch(verdigris::Command::extract());
        show_hint(*state, "Contextual interaction requested");
      }
      if (wparam == 'I') {
        state->gear_overlay = !state->gear_overlay;
        state->selected_item = 0;
        if (state->gear_overlay) show_hint(*state, "Gear opened");
      }
      if (state->gear_overlay && wparam == VK_UP &&
          !state->simulation->scion().carried_items.empty()) {
        if (state->selected_item > 0) --state->selected_item;
      }
      if (state->gear_overlay && wparam == VK_DOWN &&
          !state->simulation->scion().carried_items.empty()) {
        state->selected_item = std::min(
            state->selected_item + 1,
            state->simulation->scion().carried_items.size() - 1);
      }
      if (state->gear_overlay && wparam == VK_RETURN) equip_selected(*state);
      if (wparam == VK_PRIOR)
        state->camera.pitch_deg = std::min(85.0, state->camera.pitch_deg + 5.0);
      if (wparam == VK_NEXT)
        state->camera.pitch_deg = std::max(25.0, state->camera.pitch_deg - 5.0);
      if (wparam == VK_OEM_MINUS)
        state->camera.perspective = std::max(0.0, state->camera.perspective - 0.0001);
      if (wparam == VK_OEM_PLUS)
        state->camera.perspective = std::min(0.0012, state->camera.perspective + 0.0001);
      if (wparam == VK_HOME) {
        state->camera.zoom = 0.5;
        state->camera.pitch_deg = 55.0;
        state->camera.perspective = 0.00035;
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
        state->camera.zoom = std::clamp(state->camera.zoom * factor, 0.15, 2.0);
        InvalidateRect(window, nullptr, FALSE);
      }
      break;
    case WM_LBUTTONDOWN:
      if (state) {
        if (state->gear_overlay)
          equip_selected(*state);
        else
          state->simulation->dispatch(
              verdigris::Command::action_use(verdigris::ActionType::Melee));
      }
      InvalidateRect(window, nullptr, FALSE);
      break;
    case WM_RBUTTONDOWN:
      if (state) state->simulation->dispatch(
          verdigris::Command::action_use(verdigris::ActionType::Dash));
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
}  // namespace

int run_headless_demo() {
  verdigris::Simulation simulation(0xC011AB1EULL, "House Verdigris");
  verdigris::EmberHunt seasonal;
  simulation.set_seasonal_mechanic(&seasonal);
  simulation.dispatch(verdigris::Command::enter("route:tin:1:0"));
  for (int i = 0; i < 4; ++i) simulation.dispatch(verdigris::Command::move(1, 0));
  for (int i = 0; i < 8; ++i)
    simulation.dispatch(verdigris::Command::action_use(verdigris::ActionType::Melee));
  if (!simulation.ground_items().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_items().front().id));
  if (!simulation.ground_trophies().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_trophies().front().id));
  for (int i = 0; i < 4; ++i) simulation.dispatch(verdigris::Command::move(-1, 0));
  simulation.dispatch(verdigris::Command::extract());
  std::cout << "Verdigris native client shell\n"
            << "House: " << simulation.house().name
            << " | trophies stored: " << simulation.house().stored_trophies.size()
            << " | items stored: " << simulation.house().stored_items.size() << "\n";
  return 0;
}

// A standard main() keeps the console subsystem so --headless output reaches
// stdout; the interactive window is created explicitly from the module handle.
int main(int argc, char** argv) {
  for (int i = 1; i < argc; ++i)
    if (std::strcmp(argv[i], "--headless") == 0) return run_headless_demo();
  HINSTANCE instance = GetModuleHandle(nullptr);
  auto state = std::make_unique<ClientState>();
  state->simulation = std::make_unique<verdigris::Simulation>(0xC011AB1EULL, "House Verdigris");
  verdigris::EmberHunt seasonal;
  state->simulation->set_seasonal_mechanic(&seasonal);
  state->simulation->dispatch(verdigris::Command::enter("route:tin:1:0"));

  WNDCLASSA window_class{};
  window_class.hInstance = instance;
  window_class.lpfnWndProc = window_proc;
  window_class.lpszClassName = "VerdigrisNativeClient";
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassA(&window_class);

  HWND window = CreateWindowExA(0, window_class.lpszClassName, "Verdigris - Native Expedition",
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
  for (int i = 0; i < 4; ++i) simulation.dispatch(verdigris::Command::move(1, 0));
  for (int i = 0; i < 8; ++i)
    simulation.dispatch(verdigris::Command::action_use(verdigris::ActionType::Melee));
  if (!simulation.ground_items().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_items().front().id));
  if (!simulation.ground_trophies().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_trophies().front().id));
  for (int i = 0; i < 4; ++i) simulation.dispatch(verdigris::Command::move(-1, 0));
  simulation.dispatch(verdigris::Command::extract());
  std::cout << "Verdigris native client shell\n"
            << "House: " << simulation.house().name
            << " | trophies stored: " << simulation.house().stored_trophies.size()
            << " | items stored: " << simulation.house().stored_items.size() << "\n";
  return 0;
}

int main() {
  return run_headless_demo();
}
#endif
