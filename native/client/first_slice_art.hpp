#pragma once

// Accepted art only. One manifest row is one complete authored clip, never a
// guessed filename sequence. Coordinates are source pixels, not alpha bounds.
#include <fstream>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <map>
#include <string>
#include <vector>

namespace first_slice_art {
struct Clip {
  std::string identity, action, direction;
  double fps=8, pixels_per_metre=48;
  bool loop=true;
  int width=96, height=96, anchor_x=48, anchor_y=80;
  std::vector<std::string> frames;
  const std::string& frame(double phase) const {
    const double p=loop?phase-std::floor(phase):std::clamp(phase,0.0,1.0);
    return frames[std::min(frames.size()-1,std::size_t(p*frames.size()))];
  }
};
inline std::string key(const std::string& id,const std::string& action,const std::string& dir) {
  return id+"/"+action+"/"+dir;
}
inline const char* direction(double x,double y) {
  // Cardinal reference sheets have no diagonal art. Select nearest actual
  // facing; do not relabel a front frame as a diagonal or mirror handedness.
  if(std::abs(x)>std::abs(y)) return x<0?"left":"right";
  return y<0?"back":"front";
}
struct Registry {
  std::map<std::string,Clip> clips;
  std::vector<std::string> errors;
  const Clip* find(const std::string& id,const std::string& action,const std::string& dir) const {
    const auto it=clips.find(key(id,action,dir));
    return it==clips.end()?nullptr:&it->second;
  }
  bool ready(const std::string& id) const {
    for(const auto* dir:{"front","right","back","left"}) {
      for(const auto* action:{"idle","walk","attack","hit","death"})
        if(!find(id,action,dir)) return false;
      if(id.rfind("player_",0)==0&&!find(id,"sprint",dir))return false;
    }
    return true;
  }
  void read(std::istream& input) {
    clips.clear();errors.clear();std::string line;int row=0;
    while(std::getline(input,line)) {
      ++row;if(line.empty()||line[0]=='#')continue;
      Clip c;int loop;std::istringstream stream(line);
      if(!(stream>>c.identity>>c.action>>c.direction>>c.fps>>loop>>c.width>>c.height>>c.anchor_x>>c.anchor_y>>c.pixels_per_metre)) {
        errors.push_back("invalid clip row "+std::to_string(row));continue;
      }
      c.loop=loop!=0;std::string frame;
      while(stream>>frame)c.frames.push_back(frame);
      const bool names=std::all_of(c.frames.begin(),c.frames.end(),[](const auto& n){
        return n.rfind("fs_",0)==0&&!raster_art::detail::asset_key(n.c_str()).empty();});
      if(c.frames.empty()||c.frames.size()>64||!names||c.width<1||c.height<1||c.width>1024||c.height>1024||
          c.anchor_x<0||c.anchor_x>c.width||c.anchor_y<0||c.anchor_y>c.height||
          c.pixels_per_metre!=48||!std::isfinite(c.fps)||c.fps<=0||c.fps>60||loop<0||loop>1) {
        errors.push_back("invalid clip geometry or frame list at row "+std::to_string(row));continue;
      }
      const auto id=key(c.identity,c.action,c.direction);
      if(!clips.emplace(id,std::move(c)).second) errors.push_back("duplicate clip "+id);
    }
    if(!errors.empty())clips.clear();
  }
};
inline Registry& registry() {
  static Registry result;static std::uint64_t generation=0;
  if(generation!=raster_art::asset_generation()) {
    generation=raster_art::asset_generation();
    std::ifstream stream(raster_art::first_slice_root()+L"\\manifest.tsv");
    result.read(stream);
    for(const auto& [name,clip]:result.clips) {
      // Each clip keeps one canvas and pivot. Actions may add transparent
      // padding (e.g.96x96 idle to128x128 death) without changing48px/metre.
      for(const auto& frame:clip.frames) {
        const auto size=raster_art::dimensions(frame.c_str());
        if(size.width!=clip.width||size.height!=clip.height)result.errors.push_back("missing or incorrect frame: "+frame);
      }
    }
    if(!result.errors.empty())result.clips.clear();
  }
  return result;
}
} // namespace first_slice_art
