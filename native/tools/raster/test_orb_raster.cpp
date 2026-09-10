#ifdef NDEBUG
#error Orb raster checks require assertions; compile with /UNDEBUG.
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../../client/raster_art.hpp"
#include "../../client/ui_skin.hpp"
#include <cassert>
#include <cstdio>
#include <set>
#include <fstream>

void save(raster_art::detail::Surface& surface,const wchar_t* path){
  GdiFlush();Gdiplus::Bitmap image(surface.bitmap,nullptr);
  const CLSID png{0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
  assert(image.Save(path,&png)==Gdiplus::Ok);
}

// Reproduce the old constant-color mask in the same plate/globe geometry for
// a visual comparison. The new side calls the actual production skin helper.
void legacy(HDC dc,bool life,int cx,int cy,int radius,double ratio,const std::string& caption){
  namespace orb=skin::raster_orb_detail;
  auto& cache=orb::cache();const RECT art=orb::art_rect(life),globe=orb::globe_rect(life);
  const int h=radius*5/2,w=(art.right-art.left)*h/(art.bottom-art.top);
  raster_art::detail::Surface plate;assert(plate.create(w,h));
  auto* pixels=static_cast<std::uint32_t*>(plate.pixels);
  const double level=globe.top+(globe.bottom-globe.top)*(1-ratio);
  for(int y=0;y<h;++y)for(int x=0;x<w;++x){
    const double sx=art.left+(x+.5)*(art.right-art.left)/w-.5;
    const double sy=art.top+(y+.5)*(art.bottom-art.top)/h-.5;
    auto c=cache.art.at(sx,sy);
    if(ratio>0&&sy>=level){const double m=cache.mask.at(sx,sy).r/255;
      c=orb::mix(c,life?orb::Sample{208,69,69,255}:orb::Sample{91,146,239,255},m);}
    const auto a=static_cast<std::uint32_t>(std::lround(c.a));
    pixels[y*w+x]=(a<<24)|(static_cast<std::uint32_t>(std::lround(c.r*a/255))<<16)|
       (static_cast<std::uint32_t>(std::lround(c.g*a/255))<<8)|static_cast<std::uint32_t>(std::lround(c.b*a/255));
  }
  const int left=cx-w/2,top=cy+radius-h;
  const BLENDFUNCTION blend{AC_SRC_OVER,0,255,AC_SRC_ALPHA};
  assert(AlphaBlend(dc,left,top,w,h,plate.dc,0,0,w,h,blend));
  const double sx=w/double(art.right-art.left),sy=h/double(art.bottom-art.top);
  const int tx=left+static_cast<int>((globe.left+globe.right-2*art.left)*sx*.5);
  const int ty=top+static_cast<int>((globe.top+globe.bottom-2*art.top)*sy*.5);
  SetBkMode(dc,TRANSPARENT);SetTextColor(dc,skin::kInk);HGDIOBJ old=SelectObject(dc,skin::font_small());
  SIZE extent{};GetTextExtentPoint32A(dc,caption.c_str(),static_cast<int>(caption.size()),&extent);
  TextOutA(dc,tx-extent.cx/2,ty-extent.cy/2,caption.c_str(),static_cast<int>(caption.size()));SelectObject(dc,old);
}

int main(){
  namespace orb=skin::raster_orb_detail;
  assert(orb::load_plates());skin::set_ui_scale(2);
  raster_art::detail::Surface sheet;assert(sheet.create(896,896));
  std::fill_n(static_cast<std::uint32_t*>(sheet.pixels),896*896,0xff2d2821u);
  const int buckets[]{0,5,10,20};
  std::ofstream report(L".ci-artifacts/orb-raster/metrics.csv");
  report<<"kind,fill,chromatic_pixels,unique_interior_colors,preserved_glass_and_statue_pixels\n";
  for(int row=0;row<4;++row){
    const double ratio=buckets[row]/20.0;
    const std::string caption=std::to_string(static_cast<int>(ratio*100))+"/100";
    for(int col=0;col<4;++col){
      const bool life=col<2;const int cx=col*224+112,cy=row*224+130;
      if(col%2)assert(skin::raster_orb(sheet.dc,life,cx,cy,68,ratio,caption,false));
      else legacy(sheet.dc,life,cx,cy,68,ratio,caption);
      SetTextColor(sheet.dc,skin::kInk);SetBkMode(sheet.dc,TRANSPARENT);SelectObject(sheet.dc,skin::font_small());
      const std::string label=std::string(life?"Life: ":"Resource: ")+(col%2?"art":"old");
      TextOutA(sheet.dc,col*224+12,row*224+206,label.c_str(),static_cast<int>(label.size()));
    }
  }
  save(sheet,L".ci-artifacts/orb-raster/before-after-0-25-50-100.png");
  skin::set_ui_scale(1);
  raster_art::detail::Surface compact;assert(compact.create(400,232));
  std::fill_n(static_cast<std::uint32_t*>(compact.pixels),400*232,0xff2d2821u);
  for(int row=0;row<2;++row)for(int col=0;col<2;++col)
    assert(skin::raster_orb(compact.dc,col==0,100+col*200,68+row*116,34,
                            row==0?1:.25,row==0?"100/100":"25/100",false));
  save(compact,L".ci-artifacts/orb-raster/hud-scale1-1366.png");
  skin::set_ui_scale(2);
  for(bool life:{true,false}){
    std::size_t previous=0;
    for(int bucket:buckets){
      const auto* image=orb::layer(life,68,bucket,false);assert(image);
      const RECT art=orb::art_rect(life),globe=orb::globe_rect(life);
      const auto* pixels=static_cast<const std::uint32_t*>(image->pixels);
      std::size_t chromatic=0,preserved=0;std::set<std::uint32_t> colors;
      for(int y=0;y<image->h;++y)for(int x=0;x<image->w;++x){
        const double sx=art.left+(x+.5)*(art.right-art.left)/image->w-.5;
        const double sy=art.top+(y+.5)*(art.bottom-art.top)/image->h-.5;
        const double u=(sx-(globe.left+globe.right)*.5)/((globe.right-globe.left)*.5);
        const double v=(sy-(globe.top+globe.bottom)*.5)/((globe.bottom-globe.top)*.5);
        if(u*u+v*v>=.81)continue;
        const auto p=pixels[y*image->w+x];const int r=(p>>16)&255,g=(p>>8)&255,b=p&255;
        assert((p>>24)==255);colors.insert(p);
        chromatic+=life?r>b*1.5&&r>55:b>r*1.5&&b>55;
        const auto original=orb::cache().art.at(sx,sy);
        const double hi=std::max({original.r,original.g,original.b}),lo=std::min({original.r,original.g,original.b});
        if(hi>130&&(hi-lo)/hi<.12){
          assert(std::abs(r-original.r)<=1&&std::abs(g-original.g)<=1&&std::abs(b-original.b)<=1);
          ++preserved;
        }
      }
      assert(colors.size()>200&&preserved>10);
      if(bucket>0)assert(chromatic>previous);
      previous=chromatic;
      report<<(life?"life":"resource")<<','<<bucket*5<<','<<chromatic<<','<<colors.size()<<','<<preserved<<'\n';
    }
  }
  const auto* quiet=orb::layer(true,68,5,false);
  const auto* pulse=orb::layer(true,68,5,true);
  assert(quiet&&pulse&&quiet!=pulse);
  const auto* normal_pixels=static_cast<const std::uint32_t*>(quiet->pixels);
  const auto* pulse_pixels=static_cast<const std::uint32_t*>(pulse->pixels);
  int pulse_changes=0;
  for(int i=0;i<quiet->w*quiet->h;++i){
    assert((normal_pixels[i]>>24)==(pulse_pixels[i]>>24));
    pulse_changes+=normal_pixels[i]!=pulse_pixels[i];
  }
  assert(pulse_changes>20);
  const auto builds=orb::cache().builds,hits=orb::cache().hits;
  const DWORD handles=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
  const COLORREF original_text=GetTextColor(sheet.dc);
  const HGDIOBJ original_font=GetCurrentObject(sheet.dc,OBJ_FONT);
  RECT original_clip{};GetClipBox(sheet.dc,&original_clip);
  for(int i=0;i<500;++i){
    assert(skin::raster_orb(sheet.dc,true,112,130,68,.5,"50/100",false));
    assert(skin::raster_orb(sheet.dc,false,336,130,68,1,"100/100",false));
  }
  assert(orb::cache().builds==builds&&orb::cache().hits==hits+1000);
  assert(handles==GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS));
  RECT final_clip{};GetClipBox(sheet.dc,&final_clip);
  assert(EqualRect(&original_clip,&final_clip)&&GetTextColor(sheet.dc)==original_text&&
         GetCurrentObject(sheet.dc,OBJ_FONT)==original_font);
  for(int radius:{34,68,102,136})for(bool life:{false,true})for(int bucket=0;bucket<=20;++bucket)
    assert(orb::layer(life,radius,bucket,false));
  assert(orb::cache().layers.size()<=orb::kMaxLayers&&orb::cache().bytes<=orb::kMaxBytes);
  assert(!skin::raster_orb(nullptr,true,112,130,68,1,"100/100",false));
  assert(!skin::raster_orb(sheet.dc,true,112,130,0,1,"100/100",false));
  assert(!skin::raster_orb(sheet.dc,true,112,130,257,1,"100/100",false));
  assert(!skin::raster_orb(sheet.dc,true,112,130,68,std::numeric_limits<double>::quiet_NaN(),"",false));
  std::printf("PASS: four actual fill states, retained authored neutral glass/statue pixels, >200 interior colors, monotonic life/resource liquid,1000 hits with zero rebuilds/handle growth; bounded cache %zu layers / %zu bytes.\n",orb::cache().layers.size(),orb::cache().bytes);
}
