#ifdef NDEBUG
#error HUD checks require assertions.
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../../client/raster_art.hpp"
#include "../../client/ui_skin.hpp"
#include "../../client/ui/implement-map-and-route-explanation.hpp"
#include "../../client/add-one-environment-ambience-layer.hpp"
#include <cassert>
#include <cstdio>
#include <set>
#include <fstream>

void save(raster_art::detail::Surface& surface,const std::wstring& path){
  GdiFlush();Gdiplus::Bitmap bitmap(surface.bitmap,nullptr);
  const CLSID png{0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
  assert(bitmap.Save(path.c_str(),&png)==Gdiplus::Ok);
}
int main(){
  assert(skin::hud_detail::load_frame());
  std::ofstream report(L".ci-artifacts/hud-chrome/bounds.csv");
  report<<"viewport,region,x,y,width,height,measured_text_height,contained\n";
  for(const auto size: {std::pair{3440,1440},std::pair{1366,768},std::pair{960,600}}){
    const auto [w,h]=size;const int s=hud_chrome_layout::scale(h);skin::set_ui_scale(s);
    LOGFONTA body{},caption{},heading{};
    assert(GetObjectA(skin::font_body(),sizeof(body),&body));
    assert(GetObjectA(skin::font_small(),sizeof(caption),&caption));
    assert(GetObjectA(skin::font_heading(),sizeof(heading),&heading));
    assert(std::string(body.lfFaceName)=="Segoe UI"&&body.lfHeight==-15*s);
    assert(std::string(caption.lfFaceName)=="Segoe UI"&&caption.lfHeight==-12*s);
    assert(std::string(heading.lfFaceName)=="Georgia");
    raster_art::detail::Surface canvas;assert(canvas.create(w,h));
    std::fill_n(static_cast<std::uint32_t*>(canvas.pixels),static_cast<std::size_t>(w)*h,0xff554532u);
    const skin::HudTextLines route{{"Tin village",skin::kInk},{"Town road",skin::kInkDim},
      {"Risk: wardens",skin::kInkDim},{"Return: press F at the pad",skin::kInkDim}};
    const auto card=hud_chrome_layout::route_card(h);
    const auto routePlan=skin::measure_hud_card(canvas.dc,card.w,route);
    for(const char* id:{"route:tin:1:0","route:tin:2:0","route:salt:1:0","town:1","branch:1","surface"})
      for(const char* theme:{"town","grove","crypt","wilds","marsh","dungeon",""})
        for(const char* risk:{"risk wardens","risk extract","risk none posted"})
          for(const char* ret:{"return town","return press F there","return walk onto it"}){
            namespace ui=verdigris::client::ui;
            const skin::HudTextLines variant{{ui::route_owner_title(id),skin::kInk},
              {ui::route_theme_label(theme),skin::kInkDim},{ui::route_risk_owner_line(risk),skin::kInkDim},
              {ui::route_return_owner_line(ret),skin::kInkDim}};
            const auto measured=skin::measure_hud_card(canvas.dc,card.w,variant);
            assert(measured.count==4&&measured.bounds.h<=card.h);
          }
    assert(routePlan.bounds.h<=card.h);
    assert(hud_chrome_layout::contains({0,0,w,h},card));
    assert(skin::hud_text_card(canvas.dc,{card.x,card.y,card.x+card.w,card.y+card.h},routePlan,route));
    report<<w<<",route,"<<card.x<<','<<card.y<<','<<card.w<<','<<card.h<<','<<routePlan.bounds.h<<",1\n";
    for(std::size_t i=0;i<routePlan.count;++i)
      assert(hud_chrome_layout::contains(routePlan.bounds,routePlan.lines[i]));
    const skin::HudTextLines audio{{"Muted",skin::kInk},{"SFX 100",skin::kInkDim},
      {"Music 100",skin::kInkDim},{"Theme Combat",skin::kGold},
      {verdigris::client::ambience::owner_loop_label("route:tin:1:0"),skin::kVerdigris}};
    const auto audioPlan=skin::measure_hud_card(canvas.dc,180*s,audio);
    const auto selectedFont=GetCurrentObject(canvas.dc,OBJ_FONT);
    auto invalid=audioPlan;invalid.lines[4].x=audioPlan.bounds.w;
    assert(!skin::hud_text_card(canvas.dc,{0,0,audioPlan.bounds.w,audioPlan.bounds.h},invalid,audio));
    assert(!skin::hud_text_card(canvas.dc,{0,0,10,10},audioPlan,audio));
    assert(!skin::measure_hud_card(canvas.dc,20*s,audio).count);
    const auto stack=hud_chrome_layout::audio_stack(s,{}, {0,0,132*s,24*s},audioPlan.bounds,{});
    const int ax=w-12*s-stack.bounds.w,ay=12*s;
    assert(hud_chrome_layout::contains({0,0,w,h},{ax,ay,stack.bounds.w,stack.bounds.h}));
    assert(hud_chrome_layout::contains(stack.bounds,stack.mixer));
    const RECT audioRect{ax+stack.mixer.x,ay+stack.mixer.y,ax+stack.mixer.x+stack.mixer.w,
                         ay+stack.mixer.y+stack.mixer.h};
    assert(skin::hud_text_card(canvas.dc,audioRect,audioPlan,audio));
    assert(GetCurrentObject(canvas.dc,OBJ_FONT)==selectedFont);
    report<<w<<",audio,"<<audioRect.left<<','<<audioRect.top<<','<<stack.mixer.w<<','<<stack.mixer.h<<','<<audioPlan.bounds.h<<",1\n";
    const int sw=4*58*s+3*8*s,left=(w-sw)/2,top=h-18-52*s;
    assert(skin::hud_panel(canvas.dc,{left-10,top-8,left+sw+10,h-14},240,8*s));
    for(int i=0;i<4;++i){
      const int x=left+i*66*s;
      skin::slot(canvas.dc,{x,top,x+58*s,h-18},skin::kGold,true);
      SetBkMode(canvas.dc,TRANSPARENT);SelectObject(canvas.dc,skin::font_body());
      const char* keys[]{"LMB","Q","E","R"};const char* names[]{"Strike","Thrust","Sweep","WarCry"};
      SIZE keyExtent{},nameExtent{};
      GetTextExtentPoint32A(canvas.dc,keys[i],static_cast<int>(strlen(keys[i])),&keyExtent);
      GetTextExtentPoint32A(canvas.dc,names[i],static_cast<int>(strlen(names[i])),&nameExtent);
      assert(keyExtent.cx+6*s<=56*s&&nameExtent.cx+6*s<=56*s);
      assert(keyExtent.cy+4*s<=26*s&&nameExtent.cy+26*s<=52*s);
      SetTextColor(canvas.dc,skin::kGold);TextOutA(canvas.dc,x+6*s,top+4*s,keys[i],static_cast<int>(strlen(keys[i])));
      SetTextColor(canvas.dc,skin::kInk);TextOutA(canvas.dc,x+6*s,top+26*s,names[i],static_cast<int>(strlen(names[i])));
    }
    assert(skin::raster_orb(canvas.dc,true,18+34*s,h-18-34*s,34*s,1,"100/100",false));
    assert(skin::raster_orb(canvas.dc,false,w-18-34*s,h-18-34*s,34*s,1,"50/50",false));
    const RECT backing{240*s,12*s,520*s,32*s};
    assert(skin::hud_text_backing(canvas.dc,backing));
    const auto backingBuilds=skin::hud_detail::cache().builds;
    assert(GetPixel(canvas.dc,backing.left-1,backing.top-1)==RGB(85,69,50));
    assert(GetPixel(canvas.dc,backing.right,backing.bottom)==RGB(85,69,50));
    SelectObject(canvas.dc,skin::font_body());SetTextColor(canvas.dc,skin::kInkDim);
    const char* controls="WASD | LMB strike | Space dash";
    TextOutA(canvas.dc,backing.left,backing.top,controls,static_cast<int>(strlen(controls)));
    save(canvas,L".ci-artifacts/hud-chrome/hud-"+std::to_wstring(w)+L".png");
    for(int i=0;i<100;++i)assert(skin::hud_text_backing(canvas.dc,backing));
    assert(skin::hud_detail::cache().builds==backingBuilds);
    const auto builds=skin::hud_detail::cache().builds;
    for(int i=0;i<100;++i)assert(skin::hud_text_card(canvas.dc,audioRect,audioPlan,audio));
    assert(skin::hud_detail::cache().builds==builds);
    std::printf("PASS %dx%d route text=%d <= %d; audio=%dx%d; repeated paints add0 layers\n",w,h,routePlan.bounds.h,card.h,audioPlan.bounds.w,audioPlan.bounds.h);
  }
  for(int i=0;i<100;++i)assert(skin::hud_detail::layer(300+i,120+i,12,240));
  auto& cache=skin::hud_detail::cache();
  assert(cache.layers.size()<=skin::hud_detail::kMaxLayers&&cache.bytes<=skin::hud_detail::kMaxBytes);
  assert(!skin::hud_detail::layer(0,20,2,240));assert(!skin::hud_detail::layer(4097,20,2,240));
  assert(!skin::hud_detail::layer(4096,4096,2,240));
  std::printf("PASS bounded cache: %zu layers/%zu bytes; invalid dimensions rejected\n",cache.layers.size(),cache.bytes);
}
