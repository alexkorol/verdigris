#pragma once

// GDI+-backed skin layer for the native client HUD. This is the styling
// vocabulary the raw-GDI presentation never had: alpha blending, vertical
// gradients, anti-aliased rounded frames, radial orb shading, and a real
// type ramp. Draw helpers take the paint HDC directly so existing call
// sites keep their structure; only the pixels change. Windows-only, like
// the rest of the Win32 shell.
//
// Palette: the Verdigris bronze-and-patina language — deep smoked bronze
// panels, verdigris (oxidised copper) accents, ledger gold for value,
// ember red for danger. Keep every HUD surface inside this table so the
// client reads as one crafted object instead of debug rectangles.

#ifdef _WIN32

#include <algorithm>
#include <string>

#include <objidl.h>
namespace Gdiplus {
using std::max;
using std::min;
}  // namespace Gdiplus
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

namespace skin {

// ── palette ────────────────────────────────────────────────────────────
inline constexpr COLORREF kPanelTop = RGB(31, 38, 36);      // smoked bronze-green
inline constexpr COLORREF kPanelBottom = RGB(16, 20, 19);   // pit shadow
inline constexpr COLORREF kPanelBorder = RGB(74, 108, 94);  // patina edge
inline constexpr COLORREF kVerdigris = RGB(120, 214, 168);  // accent
inline constexpr COLORREF kGold = RGB(239, 208, 116);       // ledger gold
inline constexpr COLORREF kEmber = RGB(214, 92, 72);        // danger
inline constexpr COLORREF kInk = RGB(226, 232, 222);        // body text
inline constexpr COLORREF kInkDim = RGB(150, 164, 152);     // secondary text

inline Gdiplus::Color gp(COLORREF c, BYTE alpha = 255) {
  return Gdiplus::Color(alpha, GetRValue(c), GetGValue(c), GetBValue(c));
}

// ── GDI+ lifetime ──────────────────────────────────────────────────────
// Started lazily on the first draw; shut down with the process. Headless
// scenario runs through memory DCs work identically.
inline void ensure_started() {
  static ULONG_PTR token = [] {
    Gdiplus::GdiplusStartupInput input;
    ULONG_PTR value = 0;
    Gdiplus::GdiplusStartup(&value, &input, nullptr);
    return value;
  }();
  (void)token;
}

// ── type ramp ──────────────────────────────────────────────────────────
// Cached fonts per role and UI scale. Georgia carries the chronicle voice
// for titles; Segoe UI carries the working HUD. set_ui_scale() is called
// once per painted frame from the window height, so fullscreen doubles the
// glyphs while the shipped test resolutions keep scale 1.
inline int& ui_scale_ref() {
  static int scale = 1;
  return scale;
}

inline void set_ui_scale(int scale) {
  ui_scale_ref() = std::clamp(scale, 1, 4);
}

inline HFONT cached_font(HFONT (&cache)[5], int base_height, int weight,
                         const char* face) {
  const int s = ui_scale_ref();
  if (!cache[s]) {
    cache[s] = CreateFontA(base_height * s, 0, 0, 0, weight, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, VARIABLE_PITCH, face);
  }
  return cache[s];
}

inline HFONT font_body() {
  static HFONT cache[5] = {};
  return cached_font(cache, -15, FW_NORMAL, "Segoe UI");
}

inline HFONT font_body_bold() {
  static HFONT cache[5] = {};
  return cached_font(cache, -15, FW_SEMIBOLD, "Segoe UI");
}

inline HFONT font_small() {
  static HFONT cache[5] = {};
  return cached_font(cache, -12, FW_NORMAL, "Segoe UI");
}

inline HFONT font_title() {
  static HFONT cache[5] = {};
  return cached_font(cache, -34, FW_BOLD, "Georgia");
}

inline HFONT font_heading() {
  static HFONT cache[5] = {};
  return cached_font(cache, -19, FW_BOLD, "Georgia");
}

// ── primitives ─────────────────────────────────────────────────────────

inline void rounded_path(Gdiplus::GraphicsPath& path, const Gdiplus::RectF& r,
                         float radius) {
  const float d = radius * 2.0f;
  path.AddArc(r.X, r.Y, d, d, 180.0f, 90.0f);
  path.AddArc(r.X + r.Width - d, r.Y, d, d, 270.0f, 90.0f);
  path.AddArc(r.X + r.Width - d, r.Y + r.Height - d, d, d, 0.0f, 90.0f);
  path.AddArc(r.X, r.Y + r.Height - d, d, d, 90.0f, 90.0f);
  path.CloseFigure();
}

// A layered panel: soft drop shadow, vertical bronze gradient body, patina
// border, and a one-pixel top highlight that sells the bevel.
inline void panel(HDC dc, const RECT& rect, COLORREF accent = kPanelBorder,
                  BYTE body_alpha = 235, float radius = 6.0f) {
  ensure_started();
  Gdiplus::Graphics g(dc);
  g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  const Gdiplus::RectF r(static_cast<float>(rect.left), static_cast<float>(rect.top),
                         static_cast<float>(rect.right - rect.left),
                         static_cast<float>(rect.bottom - rect.top));
  {  // shadow
    Gdiplus::RectF s = r;
    s.Offset(0.0f, 2.0f);
    Gdiplus::GraphicsPath shadow;
    rounded_path(shadow, s, radius);
    Gdiplus::SolidBrush brush(Gdiplus::Color(90, 0, 0, 0));
    g.FillPath(&brush, &shadow);
  }
  Gdiplus::GraphicsPath body;
  rounded_path(body, r, radius);
  Gdiplus::LinearGradientBrush fill(r, gp(kPanelTop, body_alpha),
                                    gp(kPanelBottom, body_alpha),
                                    Gdiplus::LinearGradientModeVertical);
  g.FillPath(&fill, &body);
  Gdiplus::Pen border(gp(accent, 200), 1.0f);
  g.DrawPath(&border, &body);
  {  // top highlight
    Gdiplus::Pen highlight(Gdiplus::Color(46, 255, 255, 255), 1.0f);
    g.DrawLine(&highlight, r.X + radius, r.Y + 1.0f, r.X + r.Width - radius,
               r.Y + 1.0f);
  }
}

// Status chip: a compact panel with an accent-tinted left tick so scanned
// rows key by colour before text.
inline void chip(HDC dc, const RECT& rect, COLORREF accent) {
  panel(dc, rect, accent, 225, 5.0f);
  ensure_started();
  Gdiplus::Graphics g(dc);
  g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  Gdiplus::SolidBrush tick(gp(accent, 220));
  g.FillRectangle(&tick, static_cast<float>(rect.left) + 3.0f,
                  static_cast<float>(rect.top) + 4.0f, 2.5f,
                  static_cast<float>(rect.bottom - rect.top) - 8.0f);
}

// Vital orb: dark glass sphere, gradient liquid fill clipped to the level,
// specular highlight, and a rim that carries the pulse state.
inline void orb(HDC dc, int cx, int cy, int radius, double ratio, COLORREF deep,
                COLORREF bright, COLORREF rim, bool pulse) {
  ensure_started();
  Gdiplus::Graphics g(dc);
  g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  const float fx = static_cast<float>(cx - radius);
  const float fy = static_cast<float>(cy - radius);
  const float size = static_cast<float>(radius * 2);
  {  // glass backing
    Gdiplus::GraphicsPath sphere;
    sphere.AddEllipse(fx, fy, size, size);
    Gdiplus::PathGradientBrush backing(&sphere);
    backing.SetCenterColor(Gdiplus::Color(255, 26, 32, 33));
    Gdiplus::Color edge(255, 8, 11, 12);
    INT count = 1;
    backing.SetSurroundColors(&edge, &count);
    g.FillEllipse(&backing, fx, fy, size, size);
  }
  const double bounded = std::clamp(ratio, 0.0, 1.0);
  if (bounded > 0.0) {  // liquid fill, clipped to the level line
    const float level = static_cast<float>(size * (1.0 - bounded));
    Gdiplus::Region keep(Gdiplus::RectF(fx, fy + level, size, size - level));
    Gdiplus::GraphicsContainer saved = g.BeginContainer();
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetClip(&keep);
    Gdiplus::RectF body(fx + 2.0f, fy + 2.0f, size - 4.0f, size - 4.0f);
    Gdiplus::LinearGradientBrush liquid(body, gp(bright, 235), gp(deep, 245),
                                        Gdiplus::LinearGradientModeVertical);
    g.FillEllipse(&liquid, body);
    g.EndContainer(saved);
  }
  {  // specular highlight
    Gdiplus::RectF gleam(fx + size * 0.22f, fy + size * 0.10f, size * 0.42f,
                         size * 0.26f);
    Gdiplus::LinearGradientBrush shine(gleam, Gdiplus::Color(90, 255, 255, 255),
                                       Gdiplus::Color(0, 255, 255, 255),
                                       Gdiplus::LinearGradientModeVertical);
    g.FillEllipse(&shine, gleam);
  }
  const float ring_grow = pulse ? 3.0f : 0.0f;
  Gdiplus::Pen ring(gp(pulse ? kEmber : rim, 235), pulse ? 3.0f : 2.0f);
  g.DrawEllipse(&ring, fx - ring_grow, fy - ring_grow, size + ring_grow * 2.0f,
                size + ring_grow * 2.0f);
}

// Quickbar cell: sunken slot with an accent underline when armed.
inline void slot(HDC dc, const RECT& rect, COLORREF accent, bool armed) {
  ensure_started();
  Gdiplus::Graphics g(dc);
  g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  const Gdiplus::RectF r(static_cast<float>(rect.left), static_cast<float>(rect.top),
                         static_cast<float>(rect.right - rect.left),
                         static_cast<float>(rect.bottom - rect.top));
  Gdiplus::GraphicsPath body;
  rounded_path(body, r, 4.0f);
  Gdiplus::LinearGradientBrush fill(r, gp(kPanelBottom, 240), gp(kPanelTop, 240),
                                    Gdiplus::LinearGradientModeVertical);
  g.FillPath(&fill, &body);
  Gdiplus::Pen border(gp(armed ? accent : kPanelBorder, armed ? 235 : 150), 1.0f);
  g.DrawPath(&border, &body);
  if (armed) {
    Gdiplus::Pen underline(gp(accent, 220), 2.0f);
    g.DrawLine(&underline, r.X + 5.0f, r.Y + r.Height - 3.0f,
               r.X + r.Width - 5.0f, r.Y + r.Height - 3.0f);
  }
}

}  // namespace skin

#endif  // _WIN32
