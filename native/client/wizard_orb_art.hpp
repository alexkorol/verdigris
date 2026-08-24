// wizard_orb_art.hpp — Owner Demo integration of the WIZARD orb raster pack
// (TASK-0168 assets, TASK-0181 planner contract, TASK-0185 HUD layout seam).
//
// Loads the derived circular HUD crops from assets/wizard/orbs/hud and
// composites them as faithful vital orbs: empty glass base, fill plate
// clipped to a bottom liquid band, reserved band clipped from the top.
// Replaces the flat two-color circle fallback whenever the art is present.
// No simulation coupling: pure presentation, deterministic given the same
// inputs and asset files.
#pragma once

#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace wizard_orb_art {

struct OrbBitmap {
  HBITMAP bitmap = nullptr;
  HDC dc = nullptr;
  void* old_bitmap = nullptr;
  int width = 0;
  int height = 0;

  [[nodiscard]] bool ok() const { return bitmap != nullptr && dc != nullptr; }
};

struct OrbArtSet {
  OrbBitmap life_full;
  OrbBitmap life_low;
  OrbBitmap life_empty;
  OrbBitmap life_reserved;
  OrbBitmap mana_full;
  OrbBitmap mana_low;
  OrbBitmap mana_empty;
  OrbBitmap mana_reserved;

  [[nodiscard]] bool ok() const {
    return life_full.ok() && life_empty.ok() && mana_full.ok() &&
           mana_empty.ok();
  }
};

// Minimal GDI+ entry points needed to decode the PNG plates, mirroring the
// dynamic-loading pattern used by the billboard pipeline.
struct GdiPlusProcs {
  using CreateBitmapFromFileProc = int(WINAPI*)(const WCHAR*, void**);
  using GetImageWidthProc = int(WINAPI*)(void*, UINT*);
  using GetImageHeightProc = int(WINAPI*)(void*, UINT*);
  using CreateHBITMAPFromBitmapProc = int(WINAPI*)(void*, HBITMAP*, UINT);
  using DisposeImageProc = int(WINAPI*)(void*);

  CreateBitmapFromFileProc create_bitmap_from_file = nullptr;
  GetImageWidthProc image_width = nullptr;
  GetImageHeightProc image_height = nullptr;
  CreateHBITMAPFromBitmapProc create_hbitmap = nullptr;
  DisposeImageProc dispose_image = nullptr;

  [[nodiscard]] bool ok() const {
    return create_bitmap_from_file != nullptr && image_width != nullptr &&
           image_height != nullptr && create_hbitmap != nullptr &&
           dispose_image != nullptr;
  }
};

using AlphaBlendProc = BOOL(WINAPI*)(HDC, int, int, int, int, HDC, int, int,
                                     int, int, BLENDFUNCTION);

inline HBITMAP make_orb_dib(const std::vector<std::uint8_t>& pixels, int width,
                            int height, HDC* dc, void** old_bitmap) {
  BITMAPINFO info{};
  info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth = width;
  info.bmiHeader.biHeight = -height;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;
  void* bits = nullptr;
  HDC screen = GetDC(nullptr);
  HDC memory = CreateCompatibleDC(screen);
  ReleaseDC(nullptr, screen);
  HBITMAP bitmap =
      CreateDIBSection(memory, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
  if (!bitmap) {
    DeleteDC(memory);
    return nullptr;
  }
  memcpy(bits, pixels.data(),
         static_cast<std::size_t>(width) * height * 4);
  *old_bitmap = SelectObject(memory, bitmap);
  *dc = memory;
  return bitmap;
}

inline void release_orb_bitmap(OrbBitmap* orb) {
  if (orb->dc) {
    if (orb->old_bitmap) SelectObject(orb->dc, static_cast<HGDIOBJ>(orb->old_bitmap));
    DeleteDC(orb->dc);
    orb->dc = nullptr;
  }
  if (orb->bitmap) {
    DeleteObject(orb->bitmap);
    orb->bitmap = nullptr;
  }
  orb->width = 0;
  orb->height = 0;
}

// Loads one orb plate: decode via GDI+, pull 32-bit BGRA pixels, premultiply
// for AlphaBlend, and wrap in a memory DIB. The derived HUD PNGs carry
// straight alpha with a feathered circular mask; black padding outside the
// circle is fully transparent, so no color key is applied here.
inline bool load_orb_plate(const GdiPlusProcs& procs, const std::string& path,
                           OrbBitmap* out) {
  if (!procs.ok()) return false;
  const int wide = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
  if (wide <= 0) return false;
  std::wstring wpath(static_cast<std::size_t>(wide), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, wpath.data(), wide);

  void* image = nullptr;
  if (procs.create_bitmap_from_file(wpath.c_str(), &image) != 0 || !image) {
    return false;
  }
  UINT width = 0;
  UINT height = 0;
  const bool dims_ok = procs.image_width(image, &width) == 0 &&
                       procs.image_height(image, &height) == 0 && width > 0 &&
                       height > 0;
  HBITMAP source = nullptr;
  const bool bitmap_ok =
      dims_ok && procs.create_hbitmap(image, &source, 0) == 0 && source;
  procs.dispose_image(image);
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
  const int copied =
      GetDIBits(screen, source, 0, height, pixels.data(), &info, DIB_RGB_COLORS);
  ReleaseDC(nullptr, screen);
  DeleteObject(source);
  if (copied == 0) return false;

  for (std::size_t index = 0; index < pixels.size(); index += 4) {
    const std::uint8_t alpha = pixels[index + 3];
    pixels[index] = static_cast<std::uint8_t>(pixels[index] * alpha / 255);
    pixels[index + 1] =
        static_cast<std::uint8_t>(pixels[index + 1] * alpha / 255);
    pixels[index + 2] =
        static_cast<std::uint8_t>(pixels[index + 2] * alpha / 255);
  }

  out->width = static_cast<int>(width);
  out->height = static_cast<int>(height);
  out->bitmap = make_orb_dib(pixels, out->width, out->height, &out->dc,
                             &out->old_bitmap);
  return out->ok();
}

// Walks upward from the executable directory and the current directory to
// find native/client/assets/wizard/orbs/hud, mirroring the billboard root
// discovery contract (installed-style layouts win, repository checkouts
// follow; misses are cheap and silent).
inline std::vector<std::string> orb_art_roots() {
  std::vector<std::string> roots;
  char exe_buffer[MAX_PATH];
  const DWORD exe_len = GetModuleFileNameA(nullptr, exe_buffer, MAX_PATH);
  std::string executable;
  if (exe_len > 0 && exe_len < MAX_PATH) {
    std::string path(exe_buffer, exe_len);
    const std::size_t slash = path.find_last_of("\\/");
    executable = slash == std::string::npos ? "." : path.substr(0, slash);
  }
  const std::string relative =
      "native\\client\\assets\\wizard\\orbs\\hud";
  std::vector<std::string> bases;
  if (!executable.empty()) {
    bases.push_back(executable);
    std::string prefix = executable;
    for (int depth = 1; depth <= 6; ++depth) {
      prefix += "\\..";
      bases.push_back(prefix);
    }
    roots.push_back(executable + "\\assets\\wizard\\orbs\\hud");
  }
  bases.push_back(".");
  std::string walk = ".";
  for (int depth = 0; depth <= 4; ++depth) {
    bases.push_back(walk);
    walk += "\\..";
  }
  for (const auto& base : bases) {
    roots.push_back(base + "\\" + relative);
  }
  return roots;
}

inline bool load_orb_art_set(const GdiPlusProcs& procs, OrbArtSet* set) {
  for (const auto& root : orb_art_roots()) {
    char found[MAX_PATH];
    if (GetFullPathNameA((root + "\\life_full.png").c_str(), MAX_PATH, found,
                         nullptr) == 0) {
      continue;
    }
    DWORD attributes = GetFileAttributesA(root.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
      continue;
    }
    struct Plate {
      const char* name;
      OrbBitmap* slot;
    };
    const Plate plates[] = {
        {"life_full.png", &set->life_full},     {"life_low.png", &set->life_low},
        {"life_empty.png", &set->life_empty},   {"life_reserved.png", &set->life_reserved},
        {"mana_full.png", &set->mana_full},     {"mana_low.png", &set->mana_low},
        {"mana_empty.png", &set->mana_empty},   {"mana_reserved.png", &set->mana_reserved},
    };
    bool all = true;
    for (const Plate& plate : plates) {
      if (!load_orb_plate(procs, root + "\\" + plate.name, plate.slot)) {
        all = false;
        break;
      }
    }
    if (all) return true;
    for (Plate plate : plates) release_orb_bitmap(plate.slot);
  }
  return false;
}

inline void release_orb_art_set(OrbArtSet* set) {
  OrbBitmap* plates[] = {&set->life_full,  &set->life_low,
                         &set->life_empty, &set->life_reserved,
                         &set->mana_full,  &set->mana_low,
                         &set->mana_empty, &set->mana_reserved};
  for (OrbBitmap* plate : plates) release_orb_bitmap(plate);
}

struct OrbPaint {
  int cx = 0;
  int cy = 0;
  int radius = 0;
  double fill_ratio = 0.0;     // 0..1 authoritative current/max
  double reserve_ratio = 0.0;  // 0..1 reserved/max, drawn from the top
};

// Draws one orb: empty glass base, fill plate clipped to a bottom liquid
// band scaled by fill_ratio (low-state plate blended in below the
// threshold), reserved plate clipped to a top band scaled by
// reserve_ratio. Deterministic; no text or pulse ring (callers keep their
// existing caption/pulse contracts on top).
inline void draw_orb_plate(HDC dst, AlphaBlendProc alpha_blend,
                           const OrbBitmap& empty_plate,
                           const OrbBitmap& full_plate,
                           const OrbBitmap& low_plate,
                           const OrbBitmap& reserved_plate,
                           const OrbPaint& paint) {
  if (!alpha_blend) return;
  const int diameter = paint.radius * 2;
  const int left = paint.cx - paint.radius;
  const int top = paint.cy - paint.radius;
  if (diameter <= 0) return;

  BLENDFUNCTION blend{};
  blend.BlendOp = AC_SRC_OVER;
  blend.SourceConstantAlpha = 255;
  blend.AlphaFormat = AC_SRC_ALPHA;

  auto stretch = [&](const OrbBitmap& plate, int dest_x, int dest_y, int dest_w,
                     int dest_h, std::uint8_t src_alpha) {
    if (!plate.ok() || dest_w <= 0 || dest_h <= 0 || src_alpha == 0) return;
    BLENDFUNCTION b = blend;
    b.SourceConstantAlpha = src_alpha;
    alpha_blend(dst, dest_x, dest_y, dest_w, dest_h, plate.dc, 0, 0,
                plate.width, plate.height, b);
  };

  // Base: empty glass sphere.
  stretch(empty_plate, left, top, diameter, diameter, 255);

  // Fill: bottom-anchored liquid band. Below the low threshold the low-state
  // plate fades in so depletion visibly cracks and dims the globe.
  const int fill_h = static_cast<int>(paint.fill_ratio * diameter + 0.5);
  if (fill_h > 0) {
    HRGN band = CreateRectRgn(left, top + diameter - fill_h, left + diameter,
                              top + diameter);
    SelectClipRgn(dst, band);
    const std::uint8_t low_alpha =
        paint.fill_ratio >= 0.35
            ? 0
            : static_cast<std::uint8_t>((0.35 - paint.fill_ratio) / 0.35 *
                                        255.0);
    stretch(full_plate, left, top, diameter, diameter, 255);
    if (low_alpha > 0) stretch(low_plate, left, top, diameter, diameter, low_alpha);
    SelectClipRgn(dst, nullptr);
    DeleteObject(band);
  }

  // Reserved: top-anchored band (PoE-style reservation reads from the top).
  const int reserve_h = static_cast<int>(paint.reserve_ratio * diameter + 0.5);
  if (reserve_h > 0 && reserved_plate.ok()) {
    HRGN band = CreateRectRgn(left, top, left + diameter, top + reserve_h);
    SelectClipRgn(dst, band);
    stretch(reserved_plate, left, top, diameter, diameter, 220);
    SelectClipRgn(dst, nullptr);
    DeleteObject(band);
  }
}

}  // namespace wizard_orb_art
