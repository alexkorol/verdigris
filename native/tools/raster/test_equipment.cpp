// Production grip, occlusion, and cache regression checks. Run test_equipment.ps1.
#ifdef NDEBUG
#error Raster equipment checks require active assertions; compile with /UNDEBUG.
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../../client/raster_equipment.hpp"
#include <cassert>
#include <cstdio>
#include <vector>

void save(raster_art::detail::Surface& target,const wchar_t* path) {
 GdiFlush();Gdiplus::Bitmap bitmap(target.bitmap,nullptr);
 const CLSID png{0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
 assert(bitmap.Save(path,&png,nullptr)==Gdiplus::Ok);
}

void contact_sheet(const char*const* names,int count,int scale,const wchar_t* path) {
 raster_art::detail::Surface target; assert(target.create(96*scale*5,112*scale*count));
 HBRUSH bg=CreateSolidBrush(RGB(54,48,42));RECT area{0,0,target.width,target.height};
 FillRect(target.dc,&area,bg);DeleteObject(bg);
 for(int row=0;row<count;++row) for(int col=0;col<5;++col) {
  const int ox=col*96*scale,oy=row*112*scale;
  assert(raster_art::draw_sprite(target.dc,names[row],ox+48*scale,oy+96*scale,96*scale));
  assert(raster_equipment::draw_front(target.dc,names[row],raster_equipment::detail::kWeapons[col].name,ox+48*scale,oy+96*scale,96*scale));
  if(scale>1) {
   std::string label=std::string(names[row])+" / "+raster_equipment::detail::kWeapons[col].name;
   SetTextColor(target.dc,RGB(235,223,204));SetBkMode(target.dc,TRANSPARENT);
   TextOutA(target.dc,ox+4,oy+98*scale,label.c_str(),int(label.size()));
  }
 }
 save(target,path);
}

int main() {
 raster_art::set_asset_root(L"native/client/assets/raster/runtime");
 using namespace raster_equipment;
 assert(!compute(nullptr,"weapon_axe",0,0,288).valid());
 assert(!compute("unknown_pose","weapon_axe",0,0,288).valid());
 assert(!compute("hero_se","unknown",0,0,288).valid());
 assert(!compute("hero_se","weapon_axe",0,0,0).valid());
 auto axe=compute("hero_se","weapon_axe",120,288,288);
 assert(axe.valid()&&axe.flip&&axe.weapon_height==192);
 assert(axe.weapon_bounds.left==123&&axe.weapon_bounds.top==27);
 assert(axe.weapon_bounds.right==219&&axe.weapon_bounds.bottom==219);
 for(const auto& pose:detail::kPoses) for(const auto& weapon:detail::kWeapons)
  for(int height:{96,173,288}) {
   auto p=compute(pose.name,weapon.name,200,300,height);assert(p.valid());
   double gx=p.flip?weapon.canvas.width-weapon.grip.x:weapon.grip.x;
   double grip_x=p.weapon_bounds.left+gx*(p.weapon_bounds.right-p.weapon_bounds.left)/weapon.canvas.width;
   double grip_y=p.weapon_bounds.top+weapon.grip.y*p.weapon_height/weapon.canvas.height;
   assert(std::abs(grip_x-p.hand_screen.x)<=0.500001);
   assert(std::abs(grip_y-p.hand_screen.y)<=0.500001);
  }

 raster_art::detail::Surface target;assert(target.create(360,360));
 const auto check_occlusion=[&](const char* pose_name) {
  auto plan=compute(pose_name,"weapon_axe",120,288,288);assert(plan.valid());
  std::memset(target.pixels,80,target.bytes());
  assert(raster_art::draw_sprite(target.dc,pose_name,120,288,288));GdiFlush();
  std::vector<std::uint32_t> before(static_cast<std::uint32_t*>(target.pixels),static_cast<std::uint32_t*>(target.pixels)+360*360);
  assert(draw_front(target.dc,plan));GdiFlush();
  auto* source=raster_art::detail::asset(pose_name);
  const RECT box=plan.pose->behind_actor?RECT{0,0,80,96}:plan.pose->fingers;
  for(int y=box.top;y<box.bottom;++y) for(int x=box.left;x<box.right;++x) {
   Gdiplus::Color color;assert(source->image->GetPixel(x,y,&color)==Gdiplus::Ok);
   assert(color.GetAlpha()==0||color.GetAlpha()==255);
   if(color.GetAlpha()!=255) continue;
   for(int sy=0;sy<3;++sy)for(int sx=0;sx<3;++sx) {
    const int tx=x*3+sx,ty=y*3+sy,index=ty*360+tx;
    assert((static_cast<std::uint32_t*>(target.pixels)[index]&0xffffff)==(before[index]&0xffffff));
   }
  }
 };
 for(const auto& pose:detail::kPoses)check_occlusion(pose.name);
 const auto cached=raster_art::cache_stats();
 const DWORD gdi_before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
 RECT prior_clip{0,0,301,302};IntersectClipRect(target.dc,0,0,301,302);
 for(int i=0;i<1000;++i)assert(draw_front(target.dc,axe));
 RECT after_clip{};GetClipBox(target.dc,&after_clip);assert(EqualRect(&prior_clip,&after_clip));
 const DWORD gdi_after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
 auto repeated=raster_art::cache_stats();
 assert(cached.image_loads==repeated.image_loads&&cached.scale_builds==repeated.scale_builds);
 assert(gdi_after==gdi_before);
 assert(repeated.scaled_bitmaps<=192&&repeated.scaled_bytes<=64u*1024u*1024u);

 const char* idles[]={"hero_se","hero_sw","hero_ne","hero_nw"};
 const char* walks[]={"hero_walk0_se","hero_walk1_se","hero_walk2_se","hero_walk3_se"};
 const char* attacks[]={"hero_attack_se","hero_attack_sw","hero_attack_ne","hero_attack_nw"};
 contact_sheet(idles,4,1,L".ci-artifacts/raster-equipment/idle-composites-native.png");
 contact_sheet(idles,4,3,L".ci-artifacts/raster-equipment/idle-composites-3x.png");
 contact_sheet(walks,4,3,L".ci-artifacts/raster-equipment/walk-composites-3x.png");
 contact_sheet(attacks,4,3,L".ci-artifacts/raster-equipment/attack-composites-3x.png");
 std::printf("PASS: %zu poses x %zu weapons at three scales; grip error <=0.5 px, exact actor occlusion, cache reuse, bounded cache and unchanged GDI handle count.\n",std::size(detail::kPoses),std::size(detail::kWeapons));
}
