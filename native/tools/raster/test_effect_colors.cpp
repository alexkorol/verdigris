// Actual production GDI helper/color/export regression. Run test_effect_colors.ps1.
#ifdef NDEBUG
#error Effect color checks require active assertions; compile with /UNDEBUG.
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../../client/raster_art.hpp"
#include "../../client/vector_art.hpp"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {
constexpr int kWidth = 192, kHeight = 200;
constexpr COLORREF kColors[]{RGB(255, 0, 0), RGB(0, 0, 255), RGB(255, 214, 120)};
constexpr const char* kRows[]{"raw-control", "dc_color-brush", "fill_ell", "line", "fill_poly"};
constexpr CLSID kPng{0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};

void draw_swatches(HDC dc) {
  const RECT area{0, 0, kWidth, kHeight};
  FillRect(dc, &area, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
  for (int column = 0; column < 3; ++column) {
    const int x = 32 + 64 * column;
    const COLORREF color = kColors[column];
    for (int row = 0; row < 5; ++row) {
      const int y = 20 + 40 * row;
      if (row < 2) {
        // Row 0 bypasses the helper only as an independent export control.
        const COLORREF paint = row == 0 ? color : vector_art::dc_color(dc, color);
        HBRUSH brush = CreateSolidBrush(paint);
        const RECT box{x - 20, y - 12, x + 20, y + 12};
        assert(brush && FillRect(dc, &box, brush));
        DeleteObject(brush);
      } else if (row == 2) {
        vector_art::fill_ell(dc, x, y, 20, 12, color, color);
      } else if (row == 3) {
        vector_art::line(dc, x - 20, y, x + 20, y, color, 9);
      } else {
        const POINT points[]{{x - 20, y - 12}, {x + 20, y - 12},
                             {x + 20, y + 12}, {x - 20, y + 12}};
        vector_art::fill_poly(dc, points, 4, color, color);
      }
    }
  }
  GdiFlush();
}

bool check_surface(bool dib, const std::filesystem::path& output, std::ofstream& csv) {
  HDC display = GetDC(nullptr);
  assert(display);
  HDC dc = CreateCompatibleDC(display);
  assert(dc);
  BITMAPINFO info{};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = kWidth;
  info.bmiHeader.biHeight = -kHeight;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  void* dib_bits = nullptr;
  // The DDB must be compatible with the DISPLAY, not a fresh memory DC's
  // default monochrome bitmap. This exercises the live backbuffer category.
  HBITMAP bitmap = dib ? CreateDIBSection(display, &info, DIB_RGB_COLORS, &dib_bits, nullptr, 0)
                       : CreateCompatibleBitmap(display, kWidth, kHeight);
  assert(bitmap);
  HGDIOBJ old = SelectObject(dc, bitmap);
  assert(old && old != HGDI_ERROR);
  BITMAP properties{};
  assert(GetObject(bitmap, sizeof(properties), &properties) == sizeof(properties));
  assert(dib ? properties.bmBitsPixel == 32 && properties.bmBits != nullptr
             : properties.bmBitsPixel >= 24 && properties.bmBits == nullptr);
  assert(vector_art::dc_is_32bpp_dib(dc) == dib);
  draw_swatches(dc);

  COLORREF displayed[15]{};
  for (int row = 0; row < 5; ++row) for (int col = 0; col < 3; ++col)
    displayed[row * 3 + col] = GetPixel(dc, 32 + col * 64, 20 + row * 40);
  std::vector<BYTE> bytes(kWidth * kHeight * 4);
  if (dib) std::memcpy(bytes.data(), dib_bits, bytes.size());
  SelectObject(dc, old); // GetDIBits requires an unselected bitmap.
  if (!dib)
    assert(GetDIBits(display, bitmap, 0, kHeight, bytes.data(), &info, DIB_RGB_COLORS) == kHeight);

  const char* name = dib ? "dib32" : "display-ddb";
  const auto path = output / (std::string(name) + ".png");
  {
    // These are the exact GDI+ APIs used by main.cpp's save_hbitmap_png:
    // Bitmap(HBITMAP) calls GdipCreateBitmapFromHBITMAP; Save calls
    // GdipSaveImageToFile. No channel swapping, buffer conversion, or correction.
    Gdiplus::Bitmap encoded(bitmap, nullptr);
    assert(encoded.GetLastStatus() == Gdiplus::Ok);
    assert(encoded.Save(path.c_str(), &kPng, nullptr) == Gdiplus::Ok);
  }
  Gdiplus::Bitmap png(path.c_str());
  assert(png.GetLastStatus() == Gdiplus::Ok);
  bool correct = true;
  for (int row = 0; row < 5; ++row) for (int col = 0; col < 3; ++col) {
    const COLORREF expected = kColors[col], actual = displayed[row * 3 + col];
    const int x = 32 + col * 64, y = 20 + row * 40;
    const BYTE* bgra = bytes.data() + (y * kWidth + x) * 4;
    Gdiplus::Color saved;
    assert(png.GetPixel(x, y, &saved) == Gdiplus::Ok);
    const bool matches = actual == expected && bgra[0] == GetBValue(expected) &&
        bgra[1] == GetGValue(expected) && bgra[2] == GetRValue(expected) &&
        saved.GetRed() == GetRValue(expected) && saved.GetGreen() == GetGValue(expected) &&
        saved.GetBlue() == GetBValue(expected);
    correct &= matches;
    // The high byte of a BI_RGB GDI drawing is unused; report it without
    // pretending GDI brush operations authored premultiplied alpha.
    std::printf("%s %-14s RGB(%3u,%3u,%3u) GetPixel(%3u,%3u,%3u) BGRA[%3u,%3u,%3u,%3u] PNG(%3u,%3u,%3u) %s\n",
        name,kRows[row],GetRValue(expected),GetGValue(expected),GetBValue(expected),
        GetRValue(actual),GetGValue(actual),GetBValue(actual),bgra[0],bgra[1],bgra[2],bgra[3],
        saved.GetRed(),saved.GetGreen(),saved.GetBlue(),matches ? "PASS" : "FAIL");
    csv << name << ',' << kRows[row] << ',' << unsigned(GetRValue(expected)) << ','
        << unsigned(GetGValue(expected)) << ',' << unsigned(GetBValue(expected)) << ','
        << unsigned(GetRValue(actual)) << ',' << unsigned(GetGValue(actual)) << ','
        << unsigned(GetBValue(actual)) << ',' << unsigned(bgra[0]) << ',' << unsigned(bgra[1]) << ','
        << unsigned(bgra[2]) << ',' << unsigned(bgra[3]) << ',' << unsigned(saved.GetRed()) << ','
        << unsigned(saved.GetGreen()) << ',' << unsigned(saved.GetBlue()) << ','
        << (matches ? "PASS" : "FAIL") << '\n';
  }
  DeleteObject(bitmap); DeleteDC(dc); ReleaseDC(nullptr, display);
  return correct;
}

void check_sprite_flash(const std::filesystem::path& output) {
  raster_art::set_asset_root(output.wstring());
  constexpr BYTE alpha[16]{0,255,128,0, 255,0,255,0, 0,255,255,255, 0,0,0,0};
  {
    Gdiplus::Bitmap fixture(4, 4, PixelFormat32bppARGB);
    for (int y = 0; y < 4; ++y) for (int x = 0; x < 4; ++x)
      assert(fixture.SetPixel(x, y, Gdiplus::Color(alpha[y * 4 + x],
          static_cast<BYTE>(30 + x * 35), static_cast<BYTE>(30 + y * 35), 70)) == Gdiplus::Ok);
    assert(fixture.Save((output / "flash-fixture.png").c_str(), &kPng) == Gdiplus::Ok);
  }
  raster_art::detail::Surface ordinary, flash;
  assert(ordinary.create(32,32) && flash.create(32,32));
  auto* source = raster_art::detail::asset("flash-fixture");
  assert(source);
  for (int height : {4,8,13}) for (bool flip : {false,true}) {
    std::memset(ordinary.pixels,0,ordinary.bytes());
    std::memset(flash.pixels,0,flash.bytes());
    assert(raster_art::draw_sprite(ordinary.dc,"flash-fixture",16,24,height,flip));
    assert(raster_art::draw_sprite_flash(flash.dc,"flash-fixture",16,24,height,flip));
    GdiFlush();
    const auto* before = static_cast<const std::uint32_t*>(ordinary.pixels);
    const auto* after = static_cast<const std::uint32_t*>(flash.pixels);
    for (int i = 0; i < 32 * 32; ++i) {
      const std::uint32_t a = before[i] >> 24;
      const std::uint32_t expected = (a << 24) | (a << 16) |
          (((240u * a + 127u) / 255u) << 8) | ((194u * a + 127u) / 255u);
      assert(after[i] == expected); // Exact alpha silhouette AND empty margins.
    }
    auto* normal_cache = raster_art::detail::scaled(*source,height,height,flip);
    auto* flash_cache = raster_art::detail::scaled(*source,height,height,flip,0,true);
    assert(normal_cache && flash_cache && normal_cache != flash_cache);
    assert(std::memcmp(normal_cache->pixels,flash_cache->pixels,normal_cache->bytes()) != 0);
  }
  // Explicit source-pixel checks protect holes and facing independently of
  // ordinary draw_sprite. Source (1,1) is a hole enclosed by opaque neighbors.
  for (bool flip : {false,true}) {
    std::memset(flash.pixels,0,flash.bytes());
    assert(raster_art::draw_sprite_flash(flash.dc,"flash-fixture",16,24,8,flip));
    GdiFlush();
    const auto* pixels = static_cast<const std::uint32_t*>(flash.pixels);
    for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x) {
      const int sx = flip ? 3 - x / 2 : x / 2;
      assert((pixels[(16 + y) * 32 + 12 + x] >> 24) == alpha[(y / 2) * 4 + sx]);
    }
    // On a nonempty scene, fully transparent texels leave every channel intact.
    std::fill_n(static_cast<std::uint32_t*>(flash.pixels),32*32,0xff102040u);
    assert(raster_art::draw_sprite_flash(flash.dc,"flash-fixture",16,24,8,flip));
    GdiFlush();
    const int hole_x = flip ? 17 : 15;
    assert(static_cast<const std::uint32_t*>(flash.pixels)[19 * 32 + hole_x] == 0xff102040u);
  }
  std::memset(flash.pixels,0,flash.bytes());
  assert(raster_art::draw_sprite_flash(flash.dc,"flash-fixture",16,24,8,false,0.5f));
  GdiFlush();
  const auto pixel = static_cast<const std::uint32_t*>(flash.pixels)[16*32+14];
  assert(pixel == 0x80807861u); // Opaque ivory through SourceConstantAlpha=128.
  assert(static_cast<const std::uint32_t*>(flash.pixels)[19*32+15] == 0);
  const auto before_zero = raster_art::cache_stats();
  assert(raster_art::draw_sprite_flash(flash.dc,"flash-fixture",16,24,8,false,0));
  assert(raster_art::cache_stats().draws == before_zero.draws);
  assert(!raster_art::draw_sprite_flash(nullptr,"flash-fixture",16,24,8));
  assert(!raster_art::draw_sprite_flash(flash.dc,"flash-fixture",16,24,0));
  assert(!raster_art::draw_sprite_flash(flash.dc,"flash-fixture",16,24,2049));
  assert(!raster_art::draw_sprite_flash(flash.dc,"flash-fixture",16,24,8,false,
                                       std::numeric_limits<float>::quiet_NaN()));
  assert(!raster_art::draw_sprite_flash(flash.dc,"flash-fixture",std::numeric_limits<int>::max(),24,8));
  assert(!raster_art::draw_sprite_flash(flash.dc,"missing-flash",16,24,8));
  const auto warm = raster_art::cache_stats();
  const DWORD handles = GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
  for (int repeat = 0; repeat < 1000; ++repeat)
    assert(raster_art::draw_sprite_flash(flash.dc,"flash-fixture",16,24,8,false,0.5f));
  const auto repeated = raster_art::cache_stats();
  assert(repeated.image_loads == warm.image_loads && repeated.scale_builds == warm.scale_builds);
  assert(repeated.cache_hits == warm.cache_hits + 1000);
  assert(handles == GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS));
  // Both variants compete for the SAME bounded cache, with no shadow tint map.
  for (int height = 1; height <= 160; ++height) {
    assert(raster_art::draw_sprite(flash.dc,"flash-fixture",16,24,height));
    assert(raster_art::draw_sprite_flash(flash.dc,"flash-fixture",16,24,height));
  }
  const auto evicted = raster_art::cache_stats();
  assert(evicted.scaled_bitmaps == 192 && evicted.scaled_bytes <= 64u*1024u*1024u);
  std::printf("PASS: fixed #fff0c2 flash, exact alpha/margins at 3 scales, source holes/flip, half-opacity BGRA, zero-opacity/invalid inputs, 1000 repeat hits with no loads/builds/handles, shared 192-entry eviction (%zu bytes).\n",evicted.scaled_bytes);

  // Production art review: ordinary hero beside the same hero with a 65% flash.
  // This writes only an artifact; source/runtime PNGs remain unchanged.
  raster_art::set_asset_root(L"native/client/assets/raster/runtime");
  raster_art::detail::Surface preview;
  assert(preview.create(576,320));
  std::fill_n(static_cast<std::uint32_t*>(preview.pixels),576*320,0xff36302au);
  assert(raster_art::draw_sprite(preview.dc,"hero_se",144,304,288));
  assert(raster_art::draw_sprite(preview.dc,"hero_se",432,304,288));
  assert(raster_art::draw_sprite_flash(preview.dc,"hero_se",432,304,288,false,0.65f));
  GdiFlush();
  Gdiplus::Bitmap image(preview.bitmap,nullptr);
  assert(image.Save((output / "actor-flash-3x.png").c_str(),&kPng) == Gdiplus::Ok);
}
} // namespace

int wmain(int argc, wchar_t** argv) {
  assert(argc == 2);
  const std::filesystem::path output(argv[1]);
  assert(std::filesystem::is_directory(output));
  assert(raster_art::detail::cache().token != 0);
  std::ofstream csv(output / "colors.csv");
  assert(csv.good());
  csv << "surface,helper,input_r,input_g,input_b,getpixel_r,getpixel_g,getpixel_b,byte_b,byte_g,byte_r,byte_a,png_r,png_g,png_b,result\n";
  const bool dib = check_surface(true, output, csv);
  const bool ddb = check_surface(false, output, csv);
  check_sprite_flash(output);
  std::printf("%s: production helper colors agree with RGB inputs, GetPixel, BGRA storage and PNGs on DIB/DDB.\n",
               dib && ddb ? "PASS" : "FAIL");
  return dib && ddb ? 0 : 1;
}
