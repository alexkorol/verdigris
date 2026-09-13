// Compare actual GDI+ sampled source coordinates with the attachment clip.
// Fixtures are synthetic coordinate maps; runtime artwork is never modified.
#ifdef NDEBUG
#error Raster equipment checks require active assertions; compile with /UNDEBUG.
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../../client/raster_equipment.hpp"
#include <cassert>
#include <cstdio>
int main() {
 raster_art::set_asset_root(L".ci-artifacts/raster-equipment/sampling");
 const CLSID png{0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
 Gdiplus::Bitmap source(80,96,PixelFormat32bppARGB);
 // Red/green encode the native x/y pixel, allowing measurement of GDI+'s
 // actual sampling without duplicating the helper's rounding formula.
 for(int y=0;y<96;++y)for(int x=0;x<80;++x)
  source.SetPixel(x,y,Gdiplus::Color(255,static_cast<BYTE>(x+1),static_cast<BYTE>(y+1),0));
 assert(source.Save(L".ci-artifacts/raster-equipment/sampling/mapping.png",&png)==Gdiplus::Ok);
 assert(source.Save(L".ci-artifacts/raster-equipment/sampling/hero_se.png",&png)==Gdiplus::Ok);
 Gdiplus::Bitmap weapon(32,64,PixelFormat32bppARGB);
 assert(weapon.Save(L".ci-artifacts/raster-equipment/sampling/weapon_axe.png",&png)==Gdiplus::Ok);
 for(int h:{96,97,111,173,192,205,287,288}) {
  int w=int(std::lround(h*80.0/96));
  raster_art::detail::Surface out;assert(out.create(w,h));
  assert(raster_art::draw_sprite(out.dc,"mapping",w/2,h,h));GdiFlush();
  auto* pixels=static_cast<std::uint32_t*>(out.pixels);
  int left=w,right=0,top=h,bottom=0;
  for(int x=0;x<w;++x) {int sx=int((pixels[x]>>16)&255)-1;if(sx>=49&&sx<52){left=std::min(left,x);right=x+1;}}
  for(int y=0;y<h;++y) {int sy=int((pixels[y*w]>>8)&255)-1;if(sy>=65&&sy<69){top=std::min(top,y);bottom=y+1;}}
  const auto plan=raster_equipment::compute("hero_se","weapon_axe",w/2,h,h);
  assert(plan.valid());const RECT measured{left,top,right,bottom};
  assert(EqualRect(&measured,&plan.occlusion_bounds));
  std::printf("h%d w%d measured=(%d,%d,%d,%d) ceil edges=(%d,%d,%d,%d) half edges=(%d,%d,%d,%d)\n",h,w,left,top,right,bottom,
    int(ceil(49*w/80.0)),int(ceil(65*h/96.0)),int(ceil(52*w/80.0)),int(ceil(69*h/96.0)),
    int(ceil(49*w/80.0-0.5)),int(ceil(65*h/96.0-0.5)),int(ceil(52*w/80.0-0.5)),int(ceil(69*h/96.0-0.5)));
 }
 std::puts("PASS: measured GDI+ nearest-neighbor source coordinates match helper finger clipping at all eight tested sizes.");
}
