#pragma once

// GDI+-backed skin layer for the native client HUD. This is the styling
// vocabulary the raw-GDI presentation never had: alpha blending, vertical
// gradients, anti-aliased rounded frames, radial orb shading, and a real
// type ramp. Draw helpers take the paint HDC directly so existing call
// sites keep their structure; only the pixels change. Windows-only, like
// the rest of the Win32 shell.
//
// Palette: warm bronze frames, neutral charcoal panels, linen-colored text,
// ledger gold for value, and ember red for danger. Restrained teal accents
// retain their existing state cues. Keep HUD colors in this shared table.

#ifdef _WIN32

#include <algorithm>
#include <cmath>
#include <string>

#include <map>
#include <cstdint>
#include <cstring>
#include <memory>
#include <tuple>
#include <vector>
#include <array>
#include "hud_chrome_layout.hpp"

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
// Mirrors the web client's design tokens (src/assets/scss/abstracts/
// _tokens.scss) so both clients read as one product.
inline constexpr COLORREF kPanelTop = RGB(30, 28, 25);      // #1e1c19
inline constexpr COLORREF kPanelMid = RGB(17, 18, 20);      // #111214
inline constexpr COLORREF kPanelBottom = RGB(10, 11, 12);   // #0a0b0c
inline constexpr COLORREF kPanelBorder = RGB(177, 143, 80); // border-strong gold
inline constexpr COLORREF kVerdigris = RGB(95, 168, 147);   // #5fa893 corner tick green
inline constexpr COLORREF kGold = RGB(225, 193, 116);       // accent-strong #e1c174
inline constexpr COLORREF kAccent = RGB(183, 146, 79);      // accent #b7924f
inline constexpr COLORREF kEmber = RGB(185, 72, 69);        // danger #b94845
inline constexpr COLORREF kRuby = RGB(139, 48, 52);         // #8b3034
inline constexpr COLORREF kSapphire = RGB(49, 91, 122);     // #315b7a
inline constexpr COLORREF kInk = RGB(238, 226, 197);        // text-primary #eee2c5
inline constexpr COLORREF kInkDim = RGB(182, 169, 141);     // text-secondary #b6a98d

// VG-UI-007: WCAG-style contrast against HUD plates. Tooltips always set
// title glyphs to kInk; accent lives on a shape, not on the letters.
inline double relative_luminance(COLORREF color) {
  const auto channel = [](int value) {
    const double s = static_cast<double>(value) / 255.0;
    return s <= 0.04045 ? s / 12.92 : std::pow((s + 0.055) / 1.055, 2.4);
  };
  return 0.2126 * channel(GetRValue(color)) +
         0.7152 * channel(GetGValue(color)) +
         0.0722 * channel(GetBValue(color));
}

inline double contrast_ratio(COLORREF foreground, COLORREF background) {
  double a = relative_luminance(foreground);
  double b = relative_luminance(background);
  if (a < b) std::swap(a, b);
  return (a + 0.05) / (b + 0.05);
}

inline Gdiplus::Color gp(COLORREF c, BYTE alpha = 255) {
  return Gdiplus::Color(alpha, GetRValue(c), GetGValue(c), GetBValue(c));
}

// WIZARD's quiet recessed inventory wells. These are surfaces, not buttons;
// only selection/drop focus gains a strong border. Kept in the shared skin.
inline void inventory_surface(HDC dc, const RECT& r, int focus = 0) {
  Gdiplus::Graphics g(dc);
  Gdiplus::LinearGradientBrush fill(Gdiplus::Point(r.left, r.top),
      Gdiplus::Point(r.left, r.bottom), Gdiplus::Color(255, 22, 20, 17),
      Gdiplus::Color(255, 9, 8, 7));
  g.FillRectangle(&fill, Gdiplus::Rect(r.left,r.top,r.right-r.left,r.bottom-r.top));
  Gdiplus::Pen line(focus < 0 ? Gdiplus::Color(255, 210, 110, 96) :
      focus > 0 ? Gdiplus::Color(255, 209, 179, 105) : Gdiplus::Color(255, 57, 51, 40),
      focus ? 2.0f : 1.0f);
  g.DrawRectangle(&line, Gdiplus::Rect(r.left,r.top,r.right-r.left-1,r.bottom-r.top-1));
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

inline int ui_scale() { return ui_scale_ref(); }

// VG-UI-007: overflow is solved by layout wrap, never by dropping below
// these glyph floors.
inline constexpr int kMinSmallPx = 10;
inline constexpr int kMinBodyPx = 12;

inline const char* owner_type_floor_label() { return "Type floor"; }
inline const char* owner_ink_contrast_label() { return "Ink contrast"; }
inline bool type_floor_strip_covers_hud_fails_review(bool overlap) {
  return overlap;
}

// Registers the web client's pixel fonts for this process so both clients
// share one typeface. Safe to call often; loads once.
inline void ensure_game_fonts() {
  static bool tried = false;
  if (tried) return;
  tried = true;
  const char* candidates[] = {
      "src/assets/fonts/pixelmix.ttf",
      "src/assets/fonts/pixelmix_bold.ttf",
      "src/assets/fonts/PxPlus_IBM_VGA8.ttf",
      "../../../src/assets/fonts/pixelmix.ttf",
      "../../../src/assets/fonts/pixelmix_bold.ttf",
      "../../../src/assets/fonts/PxPlus_IBM_VGA8.ttf",
  };
  for (const char* path : candidates)
    AddFontResourceExA(path, FR_PRIVATE, nullptr);
}

inline bool game_font_available() {
  static int available = -1;
  if (available < 0) {
    ensure_game_fonts();
    LOGFONTA probe{};
    probe.lfCharSet = DEFAULT_CHARSET;
    strncpy_s(probe.lfFaceName, "Pixelmix", _TRUNCATE);
    available = 0;
    HDC screen = GetDC(nullptr);
    EnumFontFamiliesExA(
        screen, &probe,
        [](const LOGFONTA*, const TEXTMETRICA*, DWORD, LPARAM ctx) -> int {
          *reinterpret_cast<int*>(ctx) = 1;
          return 0;
        },
        reinterpret_cast<LPARAM>(&available), 0);
    ReleaseDC(nullptr, screen);
  }
  return available == 1;
}

inline HFONT cached_font(HFONT (&cache)[5], int base_height, int weight,
                         const char* face) {
  const int s = ui_scale_ref();
  if (!cache[s]) {
    const int requested = std::abs(base_height) * s;
    const int floor =
        std::abs(base_height) <= 10 ? kMinSmallPx : kMinBodyPx;
    const int px = std::max(requested, floor);
    cache[s] = CreateFontA(-px, 0, 0, 0, weight, FALSE, FALSE, FALSE,
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
// bronze gradient body, restrained accent border, one-pixel top-highlight bevel.
inline void paint_panel_into(Gdiplus::Graphics& g, const Gdiplus::RectF& r,
                             COLORREF accent, BYTE body_alpha, float radius) {
  // Web parity: near-square corners, the token panel-surface gradient with
  // its faint ruby (left) / sapphire (right) heraldic tints, a gold outer
  // border, and the recurring inner #080706 outline inset 4px.
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
  {  // heraldic tints
    Gdiplus::RectF left_wash(r.X, r.Y, r.Width * 0.34f, r.Height * 0.6f);
    Gdiplus::LinearGradientBrush ruby(left_wash, gp(kRuby, 26),
                                      Gdiplus::Color(0, 0, 0, 0),
                                      Gdiplus::LinearGradientModeHorizontal);
    g.FillRectangle(&ruby, left_wash);
    Gdiplus::RectF right_wash(r.X + r.Width * 0.66f, r.Y, r.Width * 0.34f,
                              r.Height * 0.6f);
    Gdiplus::LinearGradientBrush sapphire(
        right_wash, Gdiplus::Color(0, 0, 0, 0), gp(kSapphire, 24),
        Gdiplus::LinearGradientModeHorizontal);
    g.FillRectangle(&sapphire, right_wash);
  }
  Gdiplus::Pen border(gp(accent, 210), 1.0f);
  g.DrawPath(&border, &body);
  if (r.Width > 24.0f && r.Height > 24.0f) {  // inner double-frame outline
    Gdiplus::Pen inner(Gdiplus::Color(230, 8, 7, 6), 1.0f);
    g.DrawRectangle(&inner, r.X + 4.0f, r.Y + 4.0f, r.Width - 8.0f,
                    r.Height - 8.0f);
  }
  {  // bevel: light top edge
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

// D2-style XP strip: gold fill over a sunken plate, quantized so the
// cached layer does not rebuild every fraction tick.
inline void xp_meter(HDC dc, const RECT& rect, double fraction) {
  ensure_started();
  const int w = rect.right - rect.left;
  const int h = rect.bottom - rect.top;
  if (w <= 0 || h <= 0) return;
  const int bucket = static_cast<int>(std::lround(std::clamp(fraction, 0.0, 1.0) * 40.0));
  const auto paint_meter = [&](Gdiplus::Graphics& g, int lw, int lh) {
    Gdiplus::RectF r(0.0f, 0.0f, static_cast<float>(lw), static_cast<float>(lh));
    Gdiplus::SolidBrush pit(gp(kPanelBottom, 240));
    g.FillRectangle(&pit, r);
    if (bucket > 0) {
      const float fill_w = (static_cast<float>(lw) - 2.0f) * (static_cast<float>(bucket) / 40.0f);
      Gdiplus::RectF fill(1.0f, 1.0f, fill_w, static_cast<float>(lh) - 2.0f);
      Gdiplus::LinearGradientBrush gold(fill, gp(RGB(238, 206, 110), 255),
                                        gp(kGold, 255),
                                        Gdiplus::LinearGradientModeVertical);
      g.FillRectangle(&gold, fill);
    }
    Gdiplus::Pen border(gp(kAccent, 210), 1.0f);
    g.DrawRectangle(&border, 0.0f, 0.0f, static_cast<float>(lw - 1),
                    static_cast<float>(lh - 1));
    Gdiplus::Pen notch(Gdiplus::Color(180, 10, 11, 10), 1.0f);
    for (int i = 1; i < 10; ++i) {
      const float x = static_cast<float>(lw) * static_cast<float>(i) / 10.0f;
      g.DrawLine(&notch, x, 0.0f, x, static_cast<float>(lh));
    }
  };
  const CachedLayer* layer =
      cached_layer(layer_key(41, w, h, kGold, bucket), w, h, paint_meter);
  if (layer) {
    blend_layer(dc, *layer, rect.left, rect.top);
    return;
  }
  Gdiplus::Graphics g(dc);
  g.SetSmoothingMode(Gdiplus::SmoothingModeNone);
  g.TranslateTransform(static_cast<float>(rect.left),
                       static_cast<float>(rect.top));
  paint_meter(g, w, h);
}

namespace raster_orb_detail {

struct Sample { double r=0,g=0,b=0,a=0; };
struct Plate {
  int w=0,h=0;
  std::vector<std::uint32_t> pixels; // Decoded PARGB, no retained file lock.
  bool load(const std::wstring& path) {
    Gdiplus::Bitmap image(path.c_str());
    if (image.GetLastStatus()!=Gdiplus::Ok || image.GetWidth()>2048 || image.GetHeight()>2048)
      return false;
    w=static_cast<int>(image.GetWidth());h=static_cast<int>(image.GetHeight());
    if(w<=0 || h<=0)return false;
    Gdiplus::BitmapData data{};const Gdiplus::Rect area(0,0,w,h);
    if(image.LockBits(&area,Gdiplus::ImageLockModeRead,PixelFormat32bppPARGB,&data)!=Gdiplus::Ok)
      return false;
    pixels.resize(static_cast<std::size_t>(w)*h);
    for(int y=0;y<h;++y)std::memcpy(pixels.data()+y*w,
        static_cast<const BYTE*>(data.Scan0)+y*data.Stride,static_cast<std::size_t>(w)*4);
    image.UnlockBits(&data);return true;
  }
  Sample at(double x,double y)const {
    x=std::clamp(x,0.0,double(w-1));y=std::clamp(y,0.0,double(h-1));
    const int x0=static_cast<int>(x),y0=static_cast<int>(y);
    const double tx=x-x0,ty=y-y0;
    Sample value;
    for(int j=0;j<2;++j)for(int i=0;i<2;++i){
      const auto p=pixels[std::min(y0+j,h-1)*w+std::min(x0+i,w-1)];
      const double weight=(i?tx:1-tx)*(j?ty:1-ty);
      value.r+=((p>>16)&255)*weight;value.g+=((p>>8)&255)*weight;
      value.b+=(p&255)*weight;value.a+=(p>>24)*weight;
    }
    if(value.a>0){value.r*=255/value.a;value.g*=255/value.a;value.b*=255/value.a;}
    return value;
  }
};

struct Layer : CachedLayer {
  void* pixels=nullptr;
  std::uint64_t used=0;
  ~Layer(){if(dc&&old_bitmap&&old_bitmap!=HGDI_ERROR)SelectObject(dc,old_bitmap);if(bitmap)DeleteObject(bitmap);if(dc)DeleteDC(dc);}
  std::size_t bytes()const{return static_cast<std::size_t>(w)*h*4;}
  bool create(int width,int height){
    BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    dc=CreateCompatibleDC(nullptr);if(!dc)return false;
    bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);if(!bitmap||!pixels)return false;
    old_bitmap=SelectObject(dc,bitmap);w=width;h=height;
    return old_bitmap && old_bitmap!=HGDI_ERROR;
  }
};
inline constexpr std::size_t kMaxLayers=64,kMaxBytes=16u*1024u*1024u;
using Key=std::tuple<bool,int,int,bool>; // Side, radius, 0..20 fill, pulse.
struct Cache {
  Plate art,empty,mask;
  bool attempted=false,ready=false;
  std::map<Key,std::unique_ptr<Layer>> layers;
  std::size_t bytes=0;
  std::uint64_t clock=0,builds=0,hits=0;
};
inline Cache& cache(){static Cache value;return value;}

inline bool load_plates(){
  auto& c=cache();if(c.attempted)return c.ready;c.attempted=true;ensure_started();
  wchar_t path[32768]{};
  std::vector<std::wstring> starts;
  if(GetCurrentDirectoryW(32768,path))starts.emplace_back(path);
  if(GetModuleFileNameW(nullptr,path,32768)){
    std::wstring exe(path);const auto slash=exe.find_last_of(L"\\/");
    if(slash!=std::wstring::npos)starts.push_back(exe.substr(0,slash));
  }
  for(auto root:starts)for(int depth=0;depth<7&&!root.empty();++depth){
    const std::wstring base=root+L"\\src\\assets\\orbs\\wizard\\";
    if(GetFileAttributesW((base+L"art.png").c_str())!=INVALID_FILE_ATTRIBUTES){
      c.ready=c.art.load(base+L"art.png")&&c.empty.load(base+L"empty_aligned.jpg")&&
              c.mask.load(base+L"mask_fullres.png");
      // Fixed authored geometry; a different plate requires an explicit review.
      c.ready=c.ready&&c.art.w==1672&&c.art.h==941&&c.mask.w==1672&&c.mask.h==941;
      return c.ready;
    }
    const auto slash=root.find_last_of(L"\\/");
    if(slash==std::wstring::npos)break;root.resize(slash);
  }
  return false;
}

inline double smooth(double low,double high,double value){
  const double t=std::clamp((value-low)/(high-low),0.0,1.0);return t*t*(3-2*t);
}
inline Sample mix(const Sample& a,const Sample& b,double t){
  return {a.r+(b.r-a.r)*t,a.g+(b.g-a.g)*t,a.b+(b.b-a.b)*t,a.a+(b.a-a.a)*t};
}
inline RECT art_rect(bool life){return life?RECT{20,130,830,800}:RECT{842,130,1652,800};}
inline RECT globe_rect(bool life){return life?RECT{290,204,792,708}:RECT{876,206,1380,710};}

inline const Layer* layer(bool life,int radius,int bucket,bool pulse){
  auto& c=cache();const Key key{life,radius,bucket,pulse};
  const auto found=c.layers.find(key);
  if(found!=c.layers.end()){++c.hits;found->second->used=++c.clock;return found->second.get();}
  if(!load_plates())return nullptr;
  const RECT art=art_rect(life),globe=globe_rect(life);
  const int h=radius*5/2,w=(art.right-art.left)*h/(art.bottom-art.top);
  const std::size_t bytes=static_cast<std::size_t>(w)*h*4;
  while(!c.layers.empty()&&(c.layers.size()>=kMaxLayers||bytes>kMaxBytes-c.bytes)){
    const auto oldest=std::min_element(c.layers.begin(),c.layers.end(),[](const auto& a,const auto& b){return a.second->used<b.second->used;});
    c.bytes-=oldest->second->bytes();c.layers.erase(oldest);
  }
  auto result=std::make_unique<Layer>();if(!result->create(w,h))return nullptr;
  auto* dest=static_cast<std::uint32_t*>(result->pixels);
  const double gw=globe.right-globe.left,gh=globe.bottom-globe.top;
  const double center_x=(globe.left+globe.right)*.5,center_y=(globe.top+globe.bottom)*.5;
  const double ratio=bucket/20.0,level=globe.top+gh*(1-ratio);
  const double edge=(art.bottom-art.top)/double(h);
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){
    const double sx=art.left+(x+.5)*(art.right-art.left)/w-.5;
    const double sy=art.top+(y+.5)*(art.bottom-art.top)/h-.5;
    const Sample original=c.art.at(sx,sy);
    Sample color=original;
    const double coverage=c.mask.at(sx,sy).r/255.0;
    if(coverage>0){
      const double u=(sx-center_x)/(gw*.5),v=(sy-center_y)/(gh*.5);
      const double rr=std::sqrt(u*u+v*v);
      const Sample glass=c.empty.at((sx+.5)*c.empty.w/c.art.w-.5,(sy+.5)*c.empty.h/c.art.h-.5);
      Sample interior=glass;
      const double wet=bucket==0?0:smooth(level-edge*.5,level+edge*.5,sy)*(1-smooth(.92,.975,rr));
      if(wet>0){
        // Reuse the authored lower-hemisphere liquid texture. Project its rows
        // into the current fill, keeping the glass/rim and foreground separate.
        const double t=std::clamp((sy-level)/std::max(1.0,globe.bottom-level),0.0,1.0);
        const double texture_v=2*(.44+.49*t)-1;
        const double chord=std::sqrt(std::max(.02,1-v*v));
        const double source_chord=std::sqrt(std::max(.02,1-texture_v*texture_v));
        const double tx=center_x+std::clamp(u/chord,-1.0,1.0)*source_chord*gw*.41;
        const double ty=center_y+texture_v*gh*.5;
        Sample liquid=c.art.at(tx,ty);
        const double depth=(1.12-.24*rr*rr)*(.88+.12*(1-t));
        liquid.r=liquid.r*depth+glass.r*.15;liquid.g=liquid.g*depth+glass.g*.15;
        liquid.b=liquid.b*depth+glass.b*.15;
        // Retain the actual empty glass reflections over the liquid.
        const double reflection=.50*(1-smooth(.20,.80,v));
        liquid.r+=glass.r*glass.r/255*reflection;
        liquid.g+=glass.g*glass.g/255*reflection;
        liquid.b+=glass.b*glass.b/255*reflection;
        if(bucket<20){
          const double meniscus=std::exp(-std::abs(sy-level)/(edge*.65))*60;
          liquid.r+=meniscus*(life?1.0:.45);liquid.g+=meniscus*(life?.30:.75);
          liquid.b+=meniscus*(life?.22:1.0);
        }
        interior=mix(glass,liquid,wet);
      }
      // The supplied analytic mask includes statue fingers and the bright rim.
      // Preserve their neutral highlights from the actual art, not a flat disc.
      const double hi=std::max({original.r,original.g,original.b});
      const double lo=std::min({original.r,original.g,original.b});
      const double saturation=(hi-lo)/std::max(hi,1.0);
      const double foreground=(1-smooth(.12,.38,saturation))*smooth(45,105,hi);
      interior=mix(interior,original,foreground);
      if(pulse&&life){const double rim=(1-smooth(.98,1.01,rr))*smooth(.91,.96,rr)*30;
        interior.r+=rim;interior.g+=rim*.22;interior.b+=rim*.14;}
      color=mix(original,interior,coverage);
      color.a=std::max(original.a,coverage*255);
    }
    const auto a=static_cast<std::uint32_t>(std::lround(std::clamp(color.a,0.0,255.0)));
    const auto channel=[&](double value){return static_cast<std::uint32_t>(std::lround(std::clamp(value,0.0,255.0)*a/255));};
    dest[y*w+x]=(a<<24)|(channel(color.r)<<16)|(channel(color.g)<<8)|channel(color.b);
  }
  result->used=++c.clock;const auto* image=result.get();c.layers.emplace(key,std::move(result));
  c.bytes+=bytes;++c.builds;return image;
}
} // namespace raster_orb_detail

// Authored raster glass/statue presentation, using the existing WIZARD plates.
// Geometry matches draw_wizard_orb's HUD contract. 21 fill levels, bounded
// 64-layer/16MiB LRU; warm draws are one AlphaBlend plus the readable value.
inline bool raster_orb(HDC dc,bool life,int cx,int cy,int radius,double ratio,
                       const std::string& caption,bool pulse){
  if(!dc||radius<1||radius>256||!std::isfinite(ratio))return false;
  const int bucket=static_cast<int>(std::lround(std::clamp(ratio,0.0,1.0)*20));
  const auto* image=raster_orb_detail::layer(life,radius,bucket,pulse);
  if(!image)return false;
  const int left=cx-image->w/2,top=cy+radius-image->h;
  blend_layer(dc,*image,left,top);
  const RECT art=raster_orb_detail::art_rect(life),globe=raster_orb_detail::globe_rect(life);
  const double sx=image->w/double(art.right-art.left),sy=image->h/double(art.bottom-art.top);
  const int gx=left+static_cast<int>((globe.left-art.left)*sx);
  const int gy=top+static_cast<int>((globe.top-art.top)*sy);
  const int gw=static_cast<int>((globe.right-globe.left)*sx),gh=static_cast<int>((globe.bottom-globe.top)*sy);
  const int saved=SaveDC(dc);if(!saved)return false;
  // Keep the value's type size stable as digits change at the compact HUD scale.
  const bool compact_value=gw<96;
  SetBkMode(dc,TRANSPARENT);SelectObject(dc,compact_value?font_small():font_body_bold());SIZE extent{};
  GetTextExtentPoint32A(dc,caption.c_str(),static_cast<int>(caption.size()),&extent);
  if(!compact_value&&extent.cx>gw-6){SelectObject(dc,font_small());GetTextExtentPoint32A(dc,caption.c_str(),static_cast<int>(caption.size()),&extent);}
  const int tx=gx+gw/2-extent.cx/2,ty=gy+gh/2-extent.cy/2;
  SetTextColor(dc,RGB(8,8,10));
  for(const POINT offset:{POINT{-1,-1},POINT{1,-1},POINT{-1,1},POINT{1,1}})
    TextOutA(dc,tx+offset.x,ty+offset.y,caption.c_str(),static_cast<int>(caption.size()));
  SetTextColor(dc,kInk);TextOutA(dc,tx,ty,caption.c_str(),static_cast<int>(caption.size()));
  RestoreDC(dc,saved);return true;
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

// Detailed HUD chrome reuses the existing inventory frame. Its source and
// scaled layers are bounded independently of the older generic panel cache.
namespace hud_detail {
using Layer=raster_orb_detail::Layer;
using Key=std::tuple<int,int,int,BYTE>;
inline constexpr std::size_t kMaxLayers=48,kMaxBytes=16u*1024u*1024u;
struct Cache {
  raster_orb_detail::Plate frame;
  bool attempted=false,ready=false;
  std::map<Key,std::unique_ptr<Layer>> layers;
  std::size_t bytes=0;
  std::uint64_t clock=0,builds=0,hits=0;
};
inline Cache& cache(){static Cache value;return value;}
inline bool load_frame(){
  auto& c=cache();if(c.attempted)return c.ready;c.attempted=true;ensure_started();
  wchar_t path[32768]{};std::vector<std::wstring> starts;
  if(GetCurrentDirectoryW(32768,path))starts.emplace_back(path);
  if(GetModuleFileNameW(nullptr,path,32768)){
    std::wstring exe(path);const auto slash=exe.find_last_of(L"\\/");
    if(slash!=std::wstring::npos)starts.push_back(exe.substr(0,slash));
  }
  for(auto root:starts)for(int depth=0;depth<7&&!root.empty();++depth){
    const auto file=root+L"\\src\\assets\\inventory\\frame_ornate.png";
    if(GetFileAttributesW(file.c_str())!=INVALID_FILE_ATTRIBUTES){
      c.ready=c.frame.load(file)&&c.frame.w>236&&c.frame.h>236;return c.ready;
    }
    const auto slash=root.find_last_of(L"\\/");
    if(slash==std::wstring::npos)break;root.resize(slash);
  }
  return false;
}
inline const Layer* layer(int w,int h,int border,BYTE opacity){
  if(w<8||h<8||w>4096||h>4096||border<0||!load_frame())return nullptr;
  auto& c=cache();const Key key{w,h,border,opacity};
  if(auto found=c.layers.find(key);found!=c.layers.end()){
    found->second->used=++c.clock;++c.hits;return found->second.get();
  }
  const std::size_t bytes=static_cast<std::size_t>(w)*h*4;
  if(bytes>kMaxBytes)return nullptr;
  while(!c.layers.empty()&&(c.layers.size()>=kMaxLayers||c.bytes+bytes>kMaxBytes)){
    auto oldest=std::min_element(c.layers.begin(),c.layers.end(),
      [](const auto& a,const auto& b){return a.second->used<b.second->used;});
    c.bytes-=oldest->second->bytes();c.layers.erase(oldest);
  }
  auto surface=std::make_unique<Layer>();if(!surface->create(w,h))return nullptr;
  std::fill_n(static_cast<std::uint32_t*>(surface->pixels),static_cast<std::size_t>(w)*h,0u);
  Gdiplus::Bitmap canvas(w,h,w*4,PixelFormat32bppPARGB,static_cast<BYTE*>(surface->pixels));
  Gdiplus::Graphics g(&canvas);
  g.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
  const float inset=border?2.f:0.f;
  Gdiplus::RectF pit(inset,inset,float(w)-2*inset,float(h)-2*inset);
  Gdiplus::LinearGradientBrush fill(pit,gp(kPanelTop,opacity),gp(kPanelBottom,opacity),
                                  Gdiplus::LinearGradientModeVertical);
  g.FillRectangle(&fill,pit);
  Gdiplus::Bitmap frame(c.frame.w,c.frame.h,c.frame.w*4,PixelFormat32bppPARGB,
                       reinterpret_cast<BYTE*>(c.frame.pixels.data()));
  g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
  g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
  const int edge=std::min({border,w/2,h/2});
  const int sx[]{0,118,c.frame.w-118,c.frame.w};
  const int sy[]{0,118,c.frame.h-118,c.frame.h};
  const int dx[]{0,edge,w-edge,w},dy[]{0,edge,h-edge,h};
  for(int y=0;border>0&&y<3;++y)for(int x=0;x<3;++x){
    if(x==1&&y==1)continue;
    g.DrawImage(&frame,Gdiplus::Rect(dx[x],dy[y],dx[x+1]-dx[x],dy[y+1]-dy[y]),
                sx[x],sy[y],sx[x+1]-sx[x],sy[y+1]-sy[y],Gdiplus::UnitPixel);
  }
  g.Flush(Gdiplus::FlushIntentionSync);
  surface->used=++c.clock;c.bytes+=bytes;++c.builds;
  auto* result=surface.get();c.layers.emplace(key,std::move(surface));return result;
}
} // namespace hud_detail

inline bool hud_panel(HDC dc,const RECT& rect,BYTE opacity=240,int border=0){
  if(!dc)return false;
  if(border==0)border=10*ui_scale();
  const auto* layer=hud_detail::layer(rect.right-rect.left,rect.bottom-rect.top,border,opacity);
  if(!layer){panel(dc,rect,kPanelBorder,opacity);return false;}
  blend_layer(dc,*layer,rect.left,rect.top);return true;
}

// Readable text over roofs and foliage, contained in the caller's existing
// text bounds. Border zero is a fixed variant in the same bounded HUD cache.
inline bool hud_text_backing(HDC dc,const RECT& rect){
  if(!dc)return false;
  const auto* layer=hud_detail::layer(rect.right-rect.left,rect.bottom-rect.top,0,224);
  if(!layer)return false;
  blend_layer(dc,*layer,rect.left,rect.top);return true;
}

struct HudTextLine { std::string text;COLORREF color=kInkDim; };
using HudTextLines=std::vector<HudTextLine>;
inline hud_chrome_layout::TextCard measure_hud_card(HDC dc,int width,
                                                  const HudTextLines& lines){
  const int s=ui_scale();
  if(!dc||lines.empty()||lines.size()>8||width<=20*s)return {};
  const int saved=SaveDC(dc);SelectObject(dc,font_small());
  std::array<int,8> heights{};
  for(std::size_t i=0;i<lines.size();++i){
    RECT measured{0,0,width-20*s,0};
    DrawTextA(dc,lines[i].text.c_str(),static_cast<int>(lines[i].text.size()),&measured,
              DT_CALCRECT|DT_WORDBREAK|DT_NOPREFIX);
    heights[i]=std::max(12*s,static_cast<int>(measured.bottom));
  }
  RestoreDC(dc,saved);
  return hud_chrome_layout::text_card(width,s,heights,lines.size());
}
inline bool hud_text_card(HDC dc,const RECT& plate,
                           const hud_chrome_layout::TextCard& plan,
                           const HudTextLines& lines){
  if(!dc||plan.count!=lines.size()||plan.count>plan.lines.size()||
     plan.bounds.w<=0||plan.bounds.h<=0||plan.bounds.w>plate.right-plate.left||
     plan.bounds.h>plate.bottom-plate.top||plan.count==0)return false;
  for(std::size_t i=0;i<plan.count;++i)
    if(!hud_chrome_layout::contains(plan.bounds,plan.lines[i]))return false;
  hud_panel(dc,plate);
  const int saved=SaveDC(dc);SelectObject(dc,font_small());SetBkMode(dc,TRANSPARENT);
  IntersectClipRect(dc,plate.left,plate.top,plate.right,plate.bottom);
  for(std::size_t i=0;i<plan.count;++i){
    const auto& row=plan.lines[i];
    RECT text{plate.left+row.x,plate.top+row.y,plate.left+row.x+row.w,
              plate.top+row.y+row.h};
    SetTextColor(dc,lines[i].color);
    DrawTextA(dc,lines[i].text.c_str(),static_cast<int>(lines[i].text.size()),&text,
              DT_WORDBREAK|DT_NOPREFIX);
  }
  RestoreDC(dc,saved);return true;
}

}  // namespace skin

#endif  // _WIN32
