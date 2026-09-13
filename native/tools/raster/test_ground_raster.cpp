// Focused Win32 probe of the actual ground tile cache; no client rebuild.
#ifdef NDEBUG
#error Ground raster checks require assertions; compile with /UNDEBUG.
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../../client/raster_ground.hpp"
#include "../../client/camera2d.hpp"
#include "../../include/verdigris/core.hpp"
#include <cassert>
#include <chrono>
#include <cstdio>

namespace rg = raster_ground;
using Surface = raster_art::detail::Surface;
using Clock = std::chrono::steady_clock;
constexpr double kTile = verdigris::world_scale::kArenaHalfExtent / 8.0;

std::uint64_t fingerprint(const Surface& surface) {
  GdiFlush();
  const auto* p = static_cast<const std::uint32_t*>(surface.pixels);
  std::uint64_t hash = 14695981039346656037ull;
  for (int i = 0; i < surface.width * surface.height; ++i) {
    hash ^= p[i] & 0xffffffu;  // Opaque GDI may leave alpha unspecified.
    hash *= 1099511628211ull;
  }
  return hash;
}

rg::Layout layout() {
  rg::Layout result;
  result.active = true;
  result.key = 901;
  result.tile_units = kTile;
  // The current village road shape, with a deterministic obstacle/planting set.
  result.road(160,850,235,420,49);
  result.road(235,420,200,180,49);
  result.road(200,180,40,-40,52);
  result.road(40,-40,-60,-105,42);
  result.road(-60,-105,-290,-125,42);
  result.road(40,-40,190,-130,42);
  result.road(190,-130,340,-180,42);
  result.road(40,-40,-20,75,38);
  result.road(-20,75,-260,70,38);
  result.road(-260,70,-330,90,38);
  result.road(15,-15,15,-15,80);
  for (int i = 0; i < 14; ++i)
    result.solids[result.solid_count++] = {{-650.0 + (i % 7)*200, -400.0 + (i/7)*800},71};
  for (int i = 0; i < 9; ++i)
    result.planting[result.planting_count++] = {{-700.0 + (i % 3)*650, -550.0 + (i/3)*550},125};
  return result;
}

void paint_grid(Surface& target, const rg::Layout& field, int first_x,
                int first_y, int columns = 20, int rows = 20,
                const char* earth = "terrain_quiet_earth") {
  for (int y = 0; y < rows; ++y) for (int x = 0; x < columns; ++x) {
    const RECT rect{x*64,y*64,(x+1)*64,(y+1)*64};
    assert(rg::draw(target.dc,field,first_x+x,first_y+y,rect,earth));
  }
  GdiFlush();
}

double elapsed(Clock::time_point start) {
  return std::chrono::duration<double,std::milli>(Clock::now()-start).count();
}

void save_preview(Surface& surface,const wchar_t* path) {
  GdiFlush();
  Gdiplus::Bitmap image(surface.bitmap,nullptr);
  const CLSID png{0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
  assert(image.Save(path,&png)==Gdiplus::Ok);
}

int main() {
  std::setvbuf(stdout,nullptr,_IONBF,0);
  bool okay = true;
  auto& cache = rg::detail::cache();
  rg::Layout field = layout();
  Surface sheet;
  assert(sheet.create(1280,1280));
  assert(rg::detail::patterns("terrain_quiet_earth"));
  const DWORD base_gdi = GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
  cache.tiles.clear();
  cache.builds = cache.hits = 0;

  // Inclusive floor/ceil at a fractional tile center, plus the floor cache's
  // one tile skirt, must fit. This is the actual arena-clamped worst case.
  const double camera = kTile*.25;
  const double span = verdigris::world_scale::kArenaHalfExtent;
  const int first = static_cast<int>(std::floor((camera-span)/kTile))-1;
  const int last = static_cast<int>(std::ceil((camera+span)/kTile))+1;
  assert(last-first+1 == 20);
  auto start = Clock::now();
  paint_grid(sheet,field,first,first);
  const double cold_ms = elapsed(start);
  const auto stable_hash = fingerprint(sheet);
  const auto initial_builds = cache.builds;
  start = Clock::now();
  paint_grid(sheet,field,first,first);
  const double warm_ms = elapsed(start);
  const auto warm_builds = cache.builds-initial_builds;
  const DWORD warm_gdi = GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
  std::printf("working_set=400 cap=%zu cold_ms=%.3f warm_ms=%.3f warm_builds=%llu hits=%llu gdi_delta=%ld\n",
      rg::kMaxTiles,cold_ms,warm_ms,static_cast<unsigned long long>(warm_builds),
      static_cast<unsigned long long>(cache.hits),static_cast<long>(warm_gdi)-base_gdi);
  assert(fingerprint(sheet)==stable_hash);
  save_preview(sheet,L".ci-artifacts/ground-raster/logical-road-preview-1x.png");
  if (warm_builds) { std::puts("FAIL: warmed skirt evicts its own working set"); okay=false; }
  const auto before_pan_builds=cache.builds;
  start=Clock::now();
  paint_grid(sheet,field,first+2,first);
  std::printf("two_column_pan_new_tiles=%llu pan_ms=%.3f\n",
      static_cast<unsigned long long>(cache.builds-before_pan_builds),elapsed(start));
  assert(cache.builds-before_pan_builds==40);

  // Cross several region/layout keys. The LRU must stay bounded, own exactly
  // one DC and one bitmap per tile, and regenerate identical negative tiles.
  DWORD peak_gdi=warm_gdi;
  for (int i=0;i<3;++i) {
    field.key=902+i;
    paint_grid(sheet,field,30*i,20*i);
    assert(cache.tiles.size()<=rg::kMaxTiles);
    peak_gdi=std::max(peak_gdi,GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS));
  }
  assert(peak_gdi<=base_gdi+rg::kMaxTiles*2+4);
  field.key=901;
  paint_grid(sheet,field,first,first);
  assert(fingerprint(sheet)==stable_hash);
  cache.tiles.clear();
  GdiFlush();
  const DWORD cleared_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
  std::printf("eviction_peak_gdi_delta=%ld clear_gdi_delta=%ld negative_regeneration=identical\n",
      static_cast<long>(peak_gdi)-base_gdi,static_cast<long>(cleared_gdi)-base_gdi);
  assert(cleared_gdi<=base_gdi+4);

  // Direct tile draws versus cropping a cached world region at an integer
  // logical-pixel scale. Negative world positions and camera pan are included.
  // Arbitrary-zoom floor-cache rounding remains the client scenario's scope.
  {
    Surface direct,from_cache;
    assert(direct.create(448,320)&&from_cache.create(448,320));
    paint_grid(sheet,field,-10,-10);
    const double zoom=64/kTile;
    for (int pan : {-23,0,31}) {
      const int offset_x=325+pan,offset_y=263-pan;
      camera2d::Camera camera2{-10*kTile+(offset_x+224)/zoom,
                               -10*kTile+(offset_y+160)/zoom,zoom};
      for (int y=-10;y<10;++y) for (int x=-10;x<10;++x) {
        const auto a=camera2d::project(camera2,{448,320},x*kTile,y*kTile);
        const auto b=camera2d::project(camera2,{448,320},(x+1)*kTile,(y+1)*kTile);
        const RECT cell{a.x,a.y,b.x,b.y};
        assert(rg::draw(direct.dc,field,x,y,cell,"terrain_quiet_earth"));
      }
      assert(BitBlt(from_cache.dc,0,0,448,320,sheet.dc,offset_x,offset_y,SRCCOPY));
      assert(fingerprint(direct)==fingerprint(from_cache));
    }
    std::puts("negative_camera_pan_direct_vs_cached=identical_at_integer_pixel_scale");
  }

  // A material-name switch is supported by the ground cache and must not keep
  // the prior pattern. Return to the original name must reproduce its pixels.
  Surface single;
  assert(single.create(64,64));
  paint_grid(single,field,-1,-1,1,1);
  const auto original=fingerprint(single);
  paint_grid(single,field,-1,-1,1,1,"terrain_packed_earth");
  assert(fingerprint(single)!=original);
  paint_grid(single,field,-1,-1,1,1);
  assert(fingerprint(single)==original);
  std::puts("material_name_reload=stable");

  // Existing raster API reset/root-switch invalidates decoded assets. A ground
  // draw must not bypass the newly selected (deliberately missing) asset root.
  const auto original_root=raster_art::asset_root();
  raster_art::set_asset_root(original_root+L"\\__ground_probe_missing_root__");
  const RECT cell{0,0,64,64};
  const bool stale_draw=rg::draw(single.dc,field,-1,-1,cell,"terrain_quiet_earth");
  std::printf("missing_root_after_asset_reset_draw=%s\n",stale_draw?"stale success":"fallback");
  if (stale_draw) { std::puts("FAIL: ground pattern survives raster asset root reset"); okay=false; }
  raster_art::set_asset_root(original_root);
  paint_grid(single,field,-1,-1,1,1);
  assert(fingerprint(single)==original);
  GdiFlush();
  const DWORD reload_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
  for(int cycle=0;cycle<4;++cycle) {
    const auto before_generation=raster_art::asset_generation();
    raster_art::set_asset_root(original_root);
    assert(raster_art::asset_generation()>before_generation);
    paint_grid(single,field,-1,-1,1,1);
    assert(fingerprint(single)==original);
    assert(cache.tiles.size()==1);
    assert(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=reload_gdi+2);
  }
  std::puts("same_name_asset_reload_4_cycles=pixels_identical_tiles_rebuilt");
  cache.tiles.clear();
  std::puts(okay?"PASS ground raster cache probe":"FAIL ground raster cache probe");
  return okay?0:1;
}
