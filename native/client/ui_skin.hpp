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
#include <cmath>
#include <string>

#include <map>

#include <objidl.h>
namespace Gdiplus {
using std::max;
using std::min;
}  // namespace Gdiplus
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "msimg32.lib")

namespace skin {

// ── palette ────────────────────────────────────────────────────────────
// Shared with the browser client's dark bronze, ruby, sapphire, and ledger
// gold language so the native HUD reads as the same game rather than a debug
// shell wrapped around the WIZARD plates.
inline constexpr COLORREF kPanelTop = RGB(30, 28, 25);
inline constexpr COLORREF kPanelMid = RGB(17, 18, 20);
inline constexpr COLORREF kPanelBottom = RGB(10, 11, 12);
inline constexpr COLORREF kPanelBorder = RGB(177, 143, 80);
inline constexpr COLORREF kVerdigris = RGB(95, 168, 147);
inline constexpr COLORREF kGold = RGB(225, 193, 116);
inline constexpr COLORREF kAccent = RGB(183, 146, 79);
inline constexpr COLORREF kEmber = RGB(185, 72, 69);
inline constexpr COLORREF kRuby = RGB(139, 48, 52);
inline constexpr COLORREF kSapphire = RGB(49, 91, 122);
inline constexpr COLORREF kInk = RGB(238, 226, 197);
inline constexpr COLORREF kInkDim = RGB(182, 169, 141);

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

// Ground warnings blend with the already painted terrain. Darkening an RGB
// colour and painting it opaquely destroys that terrain and reads as a hole.
inline void warning_fill(HDC dc, Gdiplus::GraphicsPath& path,
                         COLORREF colour, double visibility) {
  ensure_started();
  const BYTE alpha = static_cast<BYTE>(std::lround(
      52.0 * std::clamp(visibility, 0.0, 1.0)));
  if (!alpha) return;
  Gdiplus::Graphics graphics(dc);
  graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  Gdiplus::SolidBrush brush(gp(colour, alpha));
  graphics.FillPath(&brush, &path);
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

// ── cached layers ──────────────────────────────────────────────────────
// GDI+ antialiased chrome is expensive to re-render 60x/s (measured 13+ ms
// of a fullscreen frame). Static elements render ONCE into premultiplied
// 32bpp bitmaps keyed by their parameters and AlphaBlend per frame.
struct CachedLayer {
  HDC dc = nullptr;
  HBITMAP bitmap = nullptr;
  HGDIOBJ old_bitmap = nullptr;
  int w = 0;
  int h = 0;
};

inline std::map<unsigned long long, CachedLayer>& layer_cache() {
  static std::map<unsigned long long, CachedLayer> cache;
  return cache;
}

// Renders via `painter(Graphics&, w, h)` into a PARGB surface and caches the
// resulting HBITMAP. Returns nullptr on failure (caller falls back to the
// direct draw path).
template <typename Painter>
inline const CachedLayer* cached_layer(unsigned long long key, int w, int h,
                                       Painter painter) {
  auto& cache = layer_cache();
  auto found = cache.find(key);
  if (found != cache.end()) return &found->second;
  if (cache.size() > 256) return nullptr;  // runaway-key safety valve
  ensure_started();
  Gdiplus::Bitmap canvas(w, h, PixelFormat32bppPARGB);
  {
    Gdiplus::Graphics g(&canvas);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    painter(g, w, h);
  }
  HBITMAP hbm = nullptr;
  if (canvas.GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hbm) != Gdiplus::Ok ||
      !hbm)
    return nullptr;
  CachedLayer layer;
  layer.dc = CreateCompatibleDC(nullptr);
  if (!layer.dc) {
    DeleteObject(hbm);
    return nullptr;
  }
  layer.bitmap = hbm;
  layer.old_bitmap = SelectObject(layer.dc, hbm);
  layer.w = w;
  layer.h = h;
  return &cache.emplace(key, layer).first->second;
}

inline void blend_layer(HDC dc, const CachedLayer& layer, int x, int y) {
  const BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  ::AlphaBlend(dc, x, y, layer.w, layer.h, layer.dc, 0, 0, layer.w, layer.h,
               blend);
}

inline unsigned long long layer_key(int kind, int w, int h, COLORREF accent,
                                    int extra) {
  return (static_cast<unsigned long long>(kind) << 58) ^
         (static_cast<unsigned long long>(w & 0xFFFF) << 40) ^
         (static_cast<unsigned long long>(h & 0xFFFF) << 24) ^
         (static_cast<unsigned long long>(accent & 0xFFFFFF)) ^
         (static_cast<unsigned long long>(extra & 0xFF) << 16);
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

// Shared panel painter at an arbitrary origin: soft drop shadow, vertical
// bronze gradient body, patina border, one-pixel top-highlight bevel.
inline void paint_panel_into(Gdiplus::Graphics& g, const Gdiplus::RectF& r,
                             COLORREF accent, BYTE body_alpha, float radius) {
  const float hard_radius = std::min(radius, 3.0f);
  {  // shadow
    Gdiplus::RectF s = r;
    s.Offset(0.0f, 2.0f);
    Gdiplus::GraphicsPath shadow;
    rounded_path(shadow, s, hard_radius);
    Gdiplus::SolidBrush brush(Gdiplus::Color(110, 0, 0, 0));
    g.FillPath(&brush, &shadow);
  }
  Gdiplus::GraphicsPath body;
  rounded_path(body, r, hard_radius);
  Gdiplus::LinearGradientBrush fill(r, gp(kPanelTop, body_alpha),
                                    gp(kPanelBottom, body_alpha),
                                    Gdiplus::LinearGradientModeVertical);
  g.FillPath(&fill, &body);
  {
    Gdiplus::RectF left_wash(r.X, r.Y, r.Width * 0.34f, r.Height * 0.6f);
    Gdiplus::LinearGradientBrush ruby(left_wash, gp(kRuby, 26),
                                      Gdiplus::Color(0, 0, 0, 0),
                                      Gdiplus::LinearGradientModeHorizontal);
    g.FillRectangle(&ruby, left_wash);
    Gdiplus::RectF right_wash(r.X + r.Width * 0.66f, r.Y,
                              r.Width * 0.34f, r.Height * 0.6f);
    Gdiplus::LinearGradientBrush sapphire(
        right_wash, Gdiplus::Color(0, 0, 0, 0), gp(kSapphire, 24),
        Gdiplus::LinearGradientModeHorizontal);
    g.FillRectangle(&sapphire, right_wash);
  }
  Gdiplus::Pen border(gp(accent, 210), 1.0f);
  g.DrawPath(&border, &body);
  if (r.Width > 24.0f && r.Height > 24.0f) {
    Gdiplus::Pen inner(Gdiplus::Color(230, 8, 7, 6), 1.0f);
    g.DrawRectangle(&inner, r.X + 4.0f, r.Y + 4.0f, r.Width - 8.0f,
                    r.Height - 8.0f);
  }
  {  // warm bevel across the top edge
    Gdiplus::Pen highlight(Gdiplus::Color(52, 218, 184, 112), 1.0f);
    g.DrawLine(&highlight, r.X + hard_radius, r.Y + 1.0f,
               r.X + r.Width - hard_radius, r.Y + 1.0f);
  }
}

// A layered panel. Rendered once per (size, accent, alpha, radius) into a
// premultiplied layer and blended per frame; identical direct draw fallback.
inline void panel(HDC dc, const RECT& rect, COLORREF accent = kPanelBorder,
                  BYTE body_alpha = 235, float radius = 6.0f) {
  ensure_started();
  const int w = rect.right - rect.left;
  const int h = rect.bottom - rect.top;
  if (w <= 0 || h <= 0) return;
  const unsigned long long key =
      layer_key(10 + static_cast<int>(radius), w, h, accent, body_alpha);
  const CachedLayer* layer =
      cached_layer(key, w, h + 3, [&](Gdiplus::Graphics& g, int lw, int lh) {
        (void)lh;
        paint_panel_into(g,
                         Gdiplus::RectF(0.0f, 0.0f, static_cast<float>(lw),
                                        static_cast<float>(h)),
                         accent, body_alpha, radius);
      });
  if (layer) {
    blend_layer(dc, *layer, rect.left, rect.top);
    return;
  }
  Gdiplus::Graphics g(dc);
  g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  paint_panel_into(g,
                   Gdiplus::RectF(static_cast<float>(rect.left),
                                  static_cast<float>(rect.top),
                                  static_cast<float>(w), static_cast<float>(h)),
                   accent, body_alpha, radius);
}

// Status chip: a compact panel with an accent-tinted left tick so scanned
// rows key by colour before text. Fully cached per (size, accent).
inline void chip(HDC dc, const RECT& rect, COLORREF accent) {
  ensure_started();
  const int w = rect.right - rect.left;
  const int h = rect.bottom - rect.top;
  if (w <= 0 || h <= 0) return;
  const auto paint_chip = [&](Gdiplus::Graphics& g, int lw, int lh) {
    (void)lh;
    paint_panel_into(g,
                     Gdiplus::RectF(0.0f, 0.0f, static_cast<float>(lw),
                                    static_cast<float>(h)),
                     accent, 225, 5.0f);
    Gdiplus::SolidBrush tick(gp(accent, 220));
    g.FillRectangle(&tick, 3.0f, 4.0f, 2.5f, static_cast<float>(h) - 8.0f);
  };
  const CachedLayer* layer =
      cached_layer(layer_key(40, w, h, accent, 0), w, h + 3, paint_chip);
  if (layer) {
    blend_layer(dc, *layer, rect.left, rect.top);
    return;
  }
  Gdiplus::Graphics g(dc);
  g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  g.TranslateTransform(static_cast<float>(rect.left),
                       static_cast<float>(rect.top));
  paint_chip(g, w, h + 3);
}

// ARPG-style clock wipe for an action that is cooling down. The 60-step
// cache keeps the animated overlay inexpensive while preserving an obvious
// radial read at compact quickbar sizes.
inline void cooldown_wedge(HDC dc, const RECT& rect, double remaining_ratio) {
  const double bounded = std::clamp(remaining_ratio, 0.0, 1.0);
  const int bucket = static_cast<int>(std::lround(bounded * 60.0));
  const int w = rect.right - rect.left;
  const int h = rect.bottom - rect.top;
  if (bucket <= 0 || w <= 0 || h <= 0) return;
  const CachedLayer* layer = cached_layer(
      layer_key(41, w, h, kGold, bucket), w, h,
      [&](Gdiplus::Graphics& g, int lw, int lh) {
        const float sweep = 360.0f * static_cast<float>(bucket) / 60.0f;
        const Gdiplus::RectF face(0.0f, 0.0f, static_cast<float>(lw),
                                 static_cast<float>(lh));
        Gdiplus::SolidBrush shade(Gdiplus::Color(178, 2, 5, 6));
        g.FillPie(&shade, face, -90.0f, sweep);
        const float angle = (-90.0f + sweep) * 3.14159265f / 180.0f;
        const float cx = static_cast<float>(lw) * 0.5f;
        const float cy = static_cast<float>(lh) * 0.5f;
        const float radius = (std::min)(cx, cy) - 2.0f;
        Gdiplus::Pen hand(Gdiplus::Color(220, 239, 208, 116), 1.5f);
        g.DrawLine(&hand, cx, cy, cx + std::cos(angle) * radius,
                   cy + std::sin(angle) * radius);
      });
  if (layer) blend_layer(dc, *layer, rect.left, rect.top);
}

// Vital orb: dark glass sphere, gradient liquid clipped to the level,
// specular highlight, and a rim that carries the pulse state. Every layer
// is cached (the liquid at 21 quantized levels), so a per-frame orb is a
// handful of AlphaBlends instead of PathGradient re-renders.
inline void orb(HDC dc, int cx, int cy, int radius, double ratio, COLORREF deep,
                COLORREF bright, COLORREF rim, bool pulse) {
  ensure_started();
  const int pad = 6;
  const int box = radius * 2 + pad * 2;
  const float fx = static_cast<float>(pad);
  const float fy = static_cast<float>(pad);
  const float size = static_cast<float>(radius * 2);
  const int origin_x = cx - radius - pad;
  const int origin_y = cy - radius - pad;

  const CachedLayer* backing = cached_layer(
      layer_key(20, box, box, 0, 0), box, box,
      [&](Gdiplus::Graphics& g, int, int) {
        Gdiplus::GraphicsPath sphere;
        sphere.AddEllipse(fx, fy, size, size);
        Gdiplus::PathGradientBrush brush(&sphere);
        brush.SetCenterColor(Gdiplus::Color(255, 26, 32, 33));
        Gdiplus::Color edge(255, 8, 11, 12);
        INT count = 1;
        brush.SetSurroundColors(&edge, &count);
        g.FillEllipse(&brush, fx, fy, size, size);
      });

  const double bounded = std::clamp(ratio, 0.0, 1.0);
  const int bucket = static_cast<int>(std::lround(bounded * 20.0));
  const CachedLayer* liquid =
      bucket <= 0 ? nullptr
                  : cached_layer(
                        layer_key(22, box, box, deep, bucket), box, box,
                        [&](Gdiplus::Graphics& g, int, int) {
                          const float level =
                              size * (1.0f - static_cast<float>(bucket) / 20.0f);
                          Gdiplus::Region keep(Gdiplus::RectF(
                              fx, fy + level, size, size - level));
                          g.SetClip(&keep);
                          Gdiplus::RectF body(fx + 2.0f, fy + 2.0f, size - 4.0f,
                                              size - 4.0f);
                          Gdiplus::LinearGradientBrush fill(
                              body, gp(bright, 235), gp(deep, 245),
                              Gdiplus::LinearGradientModeVertical);
                          g.FillEllipse(&fill, body);
                          g.ResetClip();
                        });

  const CachedLayer* gleam = cached_layer(
      layer_key(21, box, box, 0, 0), box, box,
      [&](Gdiplus::Graphics& g, int, int) {
        Gdiplus::RectF shine_rect(fx + size * 0.22f, fy + size * 0.10f,
                                  size * 0.42f, size * 0.26f);
        Gdiplus::LinearGradientBrush shine(
            shine_rect, Gdiplus::Color(90, 255, 255, 255),
            Gdiplus::Color(0, 255, 255, 255),
            Gdiplus::LinearGradientModeVertical);
        g.FillEllipse(&shine, shine_rect);
      });

  const COLORREF ring_color = pulse ? kEmber : rim;
  const CachedLayer* ring = cached_layer(
      layer_key(23, box, box, ring_color, pulse ? 1 : 0), box, box,
      [&](Gdiplus::Graphics& g, int, int) {
        const float grow = pulse ? 3.0f : 0.0f;
        Gdiplus::Pen pen(gp(ring_color, 235), pulse ? 3.0f : 2.0f);
        g.DrawEllipse(&pen, fx - grow, fy - grow, size + grow * 2.0f,
                      size + grow * 2.0f);
      });

  if (backing && gleam && ring) {
    blend_layer(dc, *backing, origin_x, origin_y);
    if (liquid) blend_layer(dc, *liquid, origin_x, origin_y);
    blend_layer(dc, *gleam, origin_x, origin_y);
    blend_layer(dc, *ring, origin_x, origin_y);
    return;
  }

  // Fallback: direct draw (rare - layer allocation failure only).
  Gdiplus::Graphics g(dc);
  g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  g.TranslateTransform(static_cast<float>(origin_x),
                       static_cast<float>(origin_y));
  Gdiplus::GraphicsPath sphere;
  sphere.AddEllipse(fx, fy, size, size);
  Gdiplus::PathGradientBrush brush(&sphere);
  brush.SetCenterColor(Gdiplus::Color(255, 26, 32, 33));
  Gdiplus::Color edge(255, 8, 11, 12);
  INT count = 1;
  brush.SetSurroundColors(&edge, &count);
  g.FillEllipse(&brush, fx, fy, size, size);
  if (bounded > 0.0) {
    const float level = static_cast<float>(size * (1.0 - bounded));
    Gdiplus::Region keep(Gdiplus::RectF(fx, fy + level, size, size - level));
    g.SetClip(&keep);
    Gdiplus::RectF body(fx + 2.0f, fy + 2.0f, size - 4.0f, size - 4.0f);
    Gdiplus::LinearGradientBrush fill(body, gp(bright, 235), gp(deep, 245),
                                      Gdiplus::LinearGradientModeVertical);
    g.FillEllipse(&fill, body);
    g.ResetClip();
  }
  const float grow = pulse ? 3.0f : 0.0f;
  Gdiplus::Pen pen(gp(ring_color, 235), pulse ? 3.0f : 2.0f);
  g.DrawEllipse(&pen, fx - grow, fy - grow, size + grow * 2.0f,
                size + grow * 2.0f);
}

// Quickbar cell: sunken slot with an accent underline when armed. Cached
// per (size, accent, armed).
inline void slot(HDC dc, const RECT& rect, COLORREF accent, bool armed) {
  ensure_started();
  const int w = rect.right - rect.left;
  const int h = rect.bottom - rect.top;
  if (w <= 0 || h <= 0) return;
  const auto paint_slot = [&](Gdiplus::Graphics& g, int lw, int lh) {
    const Gdiplus::RectF r(0.0f, 0.0f, static_cast<float>(lw),
                           static_cast<float>(lh));
    Gdiplus::GraphicsPath body;
    rounded_path(body, r, 4.0f);
    Gdiplus::LinearGradientBrush fill(r, gp(kPanelBottom, 240),
                                      gp(kPanelTop, 240),
                                      Gdiplus::LinearGradientModeVertical);
    g.FillPath(&fill, &body);
    Gdiplus::Pen border(gp(armed ? accent : kPanelBorder, armed ? 235 : 150),
                        1.0f);
    g.DrawPath(&border, &body);
    if (armed) {
      Gdiplus::Pen underline(gp(accent, 220), 2.0f);
      g.DrawLine(&underline, r.X + 5.0f, r.Y + r.Height - 3.0f,
                 r.X + r.Width - 5.0f, r.Y + r.Height - 3.0f);
    }
  };
  const CachedLayer* layer = cached_layer(
      layer_key(30, w, h, accent, armed ? 1 : 0), w, h, paint_slot);
  if (layer) {
    blend_layer(dc, *layer, rect.left, rect.top);
    return;
  }
  Gdiplus::Graphics g(dc);
  g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  g.TranslateTransform(static_cast<float>(rect.left),
                       static_cast<float>(rect.top));
  paint_slot(g, w, h);
}

// Thin segmented progression meter used by the bottom-edge XP strip. Keeping
// the body, fill, gleam, border, and notches here preserves the binding rule
// that HUD chrome goes through the shared skin instead of raw rectangles in
// presentation code.
inline void progress_bar(HDC dc, const RECT& rect, double ratio,
                         COLORREF fill = kGold, int segments = 10) {
  ensure_started();
  const int w = rect.right - rect.left;
  const int h = rect.bottom - rect.top;
  if (w <= 2 || h <= 2) return;
  Gdiplus::Graphics g(dc);
  g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  const Gdiplus::RectF frame(static_cast<float>(rect.left),
                             static_cast<float>(rect.top),
                             static_cast<float>(w - 1),
                             static_cast<float>(h - 1));
  Gdiplus::SolidBrush backing(gp(kPanelBottom, 248));
  g.FillRectangle(&backing, frame);
  const double bounded = std::clamp(ratio, 0.0, 1.0);
  const float fill_w = static_cast<float>((w - 2) * bounded);
  if (fill_w > 0.0f) {
    const Gdiplus::RectF fill_rect(static_cast<float>(rect.left + 1),
                                   static_cast<float>(rect.top + 1), fill_w,
                                   static_cast<float>(h - 2));
    Gdiplus::LinearGradientBrush liquid(fill_rect, gp(kInk, 245), gp(fill, 245),
                                        Gdiplus::LinearGradientModeVertical);
    g.FillRectangle(&liquid, fill_rect);
  }
  Gdiplus::Pen border(gp(kPanelBorder, 220), 1.0f);
  g.DrawRectangle(&border, frame);
  Gdiplus::Pen notch(gp(kPanelBottom, 230), 1.0f);
  for (int index = 1; index < std::max(1, segments); ++index) {
    const float x = static_cast<float>(rect.left) +
                    static_cast<float>(w * index) /
                        static_cast<float>(segments);
    g.DrawLine(&notch, x, static_cast<float>(rect.top + 1), x,
               static_cast<float>(rect.bottom - 2));
  }
}

}  // namespace skin

#endif  // _WIN32
