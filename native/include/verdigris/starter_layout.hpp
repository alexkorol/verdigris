#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace verdigris::starter_layout {
// World metres, independent of rendering. Both authority and presentation use
// this seed/layout; reconnecting or resizing the window never replants a map.
enum class Kind { Tree, Shrub, Rock, Grass };
struct Prop { Kind kind; double x, y; bool solid; };
struct Path { double ax, ay, bx, by, width; };
struct Layout { std::vector<Prop> props; std::vector<Path> paths; };
inline constexpr std::uint64_t kSeed = 0x50414c4953414445ull;
inline double distance(double x, double y, const Path& p) {
  const double dx=p.bx-p.ax,dy=p.by-p.ay;
  const double t=std::clamp(((x-p.ax)*dx+(y-p.ay)*dy)/(dx*dx+dy*dy),0.,1.);
  return std::hypot(x-p.ax-t*dx,y-p.ay-t*dy);
}
inline Layout generate(std::uint64_t seed=kSeed) {
  Layout out;
  out.paths={{16,28,15,24,1.2},{15,24,16,20,1.5},{16,20,16,16,2.1},
      {16,16,15,12,1.5},{15,12,16,8,1.4},{16,8,16,2,1.2},
      {16,20,10,19,.8},{10,19,8,24,.7},{16,16,23,15,.8},{23,15,25,22,.7}};
  const auto unit=[&]() {
    seed+=0x9e3779b97f4a7c15ull; auto z=seed;
    z=(z^(z>>30))*0xbf58476d1ce4e5b9ull;z=(z^(z>>27))*0x94d049bb133111ebull;
    return double((z^(z>>31))>>11)/9007199254740992.;
  };
  const auto clear=[&](double x,double y,double margin) {
    if(y>6.5 && y<9.5) return false; // palisade and breach approach
    if(x>12 && x<22 && y>13 && y<27) return false; // combat, NPCs and well
    for(const auto& p:out.paths) if(distance(x,y,p)<p.width+margin)return false;
    return true;
  };
  // Several loose copses, with open walks between them. Rejection sampling is
  // bounded; no runtime retries, per-frame randomness or uniform prop rows.
  const double centers[][2]={{5,4},{26,3},{5,13},{28,13},{4,23},{28,26},{9,29}};
  for(int attempt=0;attempt<500 && out.props.size()<25;++attempt) {
    const auto& c=centers[attempt%7];
    const double x=c[0]+(unit()-.5)*7,y=c[1]+(unit()-.5)*7;
    if(x<2 || x>30 || y<2 || y>30 || !clear(x,y,2.1))continue;
    bool spaced=true;
    for(const auto& p:out.props)if(std::hypot(p.x-x,p.y-y)<2.5){spaced=false;break;}
    if(spaced)out.props.push_back({Kind::Tree,x,y,true});
  }
  const auto trees=out.props;
  for(const auto& tree:trees)for(int i=0;i<3;++i) {
    const double angle=unit()*6.28318530718,r=.9+unit()*1.6;
    const double x=tree.x+std::cos(angle)*r,y=tree.y+std::sin(angle)*r;
    if(x>3 && x<29 && y>3 && y<29 && clear(x,y,.3))out.props.push_back({Kind::Shrub,x,y,false});
  }
  for(int i=0;i<120;++i) {
    const double x=3+unit()*26,y=3+unit()*26;
    if(!clear(x,y,.45))continue;
    const auto kind=i%4==0?Kind::Rock:Kind::Grass;
    bool spaced=true;
    for(const auto& p:out.props)if(std::hypot(p.x-x,p.y-y)<.9){spaced=false;break;}
    if(spaced)out.props.push_back({kind,x,y,kind==Kind::Rock});
  }
  return out;
}
inline const Layout& village() { static const auto layout=generate();return layout; }
} // namespace verdigris::starter_layout
