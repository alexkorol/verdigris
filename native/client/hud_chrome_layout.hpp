#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

// Shared fixed HUD geometry. Text heights come from the selected production
// font; no windowing, asset, or simulation dependencies live in this planner.
namespace hud_chrome_layout {
struct Rect { int x=0,y=0,w=0,h=0; };
inline int scale(int height) { return std::clamp(height/700,1,4); }
inline bool contains(const Rect& outer,const Rect& inner) {
  return inner.w>=0&&inner.h>=0&&inner.x>=outer.x&&inner.y>=outer.y&&
    inner.x+inner.w<=outer.x+outer.w&&inner.y+inner.h<=outer.y+outer.h;
}
inline Rect route_card(int height) {
  const int s=scale(height);
  return {12*s,126*s,176*s,108*s};
}
struct TextCard {
  Rect bounds{};
  std::array<Rect,8> lines{};
  std::size_t count=0;
};
inline TextCard text_card(int width,int s,const std::array<int,8>& heights,
                           std::size_t count) {
  TextCard result;
  if(s<1||s>4||width<=20*s||count==0||count>result.lines.size())return result;
  const int inset=10*s,gap=3*s;
  int y=inset;
  for(std::size_t i=0;i<count;++i){
    if(heights[i]<=0)return {};
    result.lines[i]={inset,y,width-2*inset,heights[i]};
    y+=heights[i]+(i+1<count?gap:0);
  }
  result.bounds={0,0,width,y+inset};result.count=count;
  return result;
}
struct AudioStack {
  Rect bounds{},art{},mute{},mixer{},lost{};
};
inline AudioStack audio_stack(int s,Rect art,Rect mute,Rect mixer,Rect lost) {
  AudioStack result;
  if(s<1||s>4)return result;
  int y=0;
  const auto append=[&](Rect input,Rect& output){
    if(input.w<=0||input.h<=0)return;
    if(y)y+=6*s;
    output={0,y,input.w,input.h};y+=input.h;
    result.bounds.w=std::max(result.bounds.w,input.w);
  };
  append(art,result.art);append(mute,result.mute);append(mixer,result.mixer);
  append(lost,result.lost);result.bounds.h=y;
  return result;
}
} // namespace hud_chrome_layout
