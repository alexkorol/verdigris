#include "../content/expedition_generator.hpp"
#include <iostream>
#include <set>
using namespace verdigris::cartography;
int main(int argc,char** argv){
  const bool fingerprints=argc>1&&std::string(argv[1])=="--fingerprints";
  int count=0;
  for(const std::string recipe:{"necropolis","causeway","quarry","sanctuary"})for(int size=0;size<3;++size)for(std::uint32_t seed=0;seed<100;++seed){
    Plan p{seed,recipe,size==0?4:size==1?6:10,size==0?3:size==1?4:7,size==0?0:size==1?5:18,size==0?0:size==1?2:8};
    const auto m=generate(p);if(!valid(m)){std::cerr<<"Invalid map "<<recipe<<" "<<seed<<"\n";return 1;}
    if(m.tiles!=generate(p).tiles){std::cerr<<"Non-deterministic tiles\n";return 1;}
    for(const auto& n:m.rooms)for(const auto& s:n.sockets){const auto d=kDirections[s.direction];for(int q=-1;q<=1;++q)if(!m.can_walk(s.x+(d.y?q:0),s.y+(d.x?q:0))){std::cerr<<"Blocked socket width\n";return 1;}}
    for(std::size_t i=1;i<m.main_path.size();++i)if(std::abs(m.main_path[i].x-m.main_path[i-1].x)+std::abs(m.main_path[i].y-m.main_path[i-1].y)!=1)return 1;
    std::uint32_t hash=2166136261u;auto mix=[&](std::uint32_t v){hash=(hash^v)*16777619u;};
    for(auto t:m.tiles)mix(t);for(const auto& n:m.rooms){mix(n.cx);mix(n.cy);mix(n.variant);mix(n.rotation);mix(n.depth);mix(n.tier);for(auto s:n.sockets){mix(s.direction);mix(s.to);}}
    for(auto e:m.edges){mix(e.a);mix(e.b);}for(auto s:m.spawns){mix(s.position.x);mix(s.position.y);mix(s.count);mix(s.tier);}
    if(fingerprints)std::cout<<recipe<<","<<size<<","<<seed<<","<<hash<<"\n";
    ++count;
  }
  if(!fingerprints)std::cout<<count<<" native expedition invariants passed\n";
}
