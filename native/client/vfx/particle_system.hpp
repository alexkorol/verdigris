#pragma once
// Bounded, CPU-only presentation simulation. No damage, input or game rules.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "verdigris/networking.hpp"

namespace verdigris::client::vfx {
struct Vec3 {float x=0,y=0,z=0;}; // native coordinates: z is elevation
inline Vec3 operator+(Vec3 a,Vec3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
inline Vec3 operator-(Vec3 a,Vec3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
inline Vec3 operator*(Vec3 a,float b){return {a.x*b,a.y*b,a.z*b};}
using Color=std::array<float,4>;
enum class Anchor {Root,Head,MainHand,OffHand,Feet,ProjectileCenter,World};
struct Attachment {std::string entity;Anchor socket=Anchor::World;Vec3 point{};};
struct Layer {
  std::string name;bool continuous=false,local=false,additive=false,glow=false;
  float delay=0,duration=.1f,rate=0,life_min=.2f,life_max=.4f,drag=0,spin=0,fps=0;
  int count=1;Vec3 pos_min{},pos_max{},vel_min{},vel_max{},accel{};
  bool radial=false;float radius=0,radial_speed=0;
  std::array<float,2> size_start{3,3},size_end{1,1};
  Color color_start{1,1,1,1},color_end{1,1,1,0};
  std::vector<std::string> frames;
};
struct Effect {std::string name;float duration=1;bool loop=false;std::vector<Layer> layers;};
struct AtlasRegion {int x=0,y=0,w=0,h=0;};
struct Assets {
  std::map<std::string,Effect> effects;std::map<std::string,AtlasRegion> regions;
  std::vector<std::uint8_t> rgba;int width=0,height=0;std::string error;
  bool ready()const{return !effects.empty() && !rgba.empty();}
};
namespace data {
using J=networking::JsonValue;
inline float num(const J& j,const char* k,float fallback){const auto* v=j.get(k);return v&&v->number()?static_cast<float>(*v->number()):fallback;}
inline std::string str(const J& j,const char* k,std::string fallback={}){const auto* v=j.get(k);return v&&v->string()?*v->string():fallback;}
inline bool flag(const J& j,const char* k){const auto* v=j.get(k);return v&&v->boolean()&&*v->boolean();}
template<std::size_t N>std::array<float,N> values(const J& j,const char* key,std::array<float,N> fallback) {
  const auto* v=j.get(key);if(!v||!v->array()||v->array()->size()!=N)return fallback;
  for(std::size_t i=0;i<N;++i){const auto n=(*v->array())[i].number();if(!n||!std::isfinite(*n))return fallback;fallback[i]=static_cast<float>(*n);}return fallback;
}
inline Vec3 vec(const J& j,const char* key){const auto v=values<3>(j,key,{0,0,0});return {v[0],v[1],v[2]};}
inline bool read(const std::string& path,J& j,std::string& error) {
  std::ifstream f(path,std::ios::binary);if(!f){error="Missing VFX asset: "+path;return false;}
  f.seekg(0,std::ios::end);const auto bytes=f.tellg();f.seekg(0);
  if(bytes<0 || bytes>65536){error="VFX asset exceeds 64 KiB";return false;}
  std::string text(static_cast<std::size_t>(bytes),'\0');
  if(!f.read(text.data(),bytes)){error="VFX asset read failed";return false;}
  return networking::parse_json(text,j,&error);
}
}
inline bool load_assets(const std::string& root,Assets& destination) {
  Assets a;data::J atlas;
  const auto fail=[&](std::string message){destination.error=std::move(message);return false;};
  if(!data::read(root+"/particles.atlas.json",atlas,a.error))return fail(a.error);
  a.width=static_cast<int>(data::num(atlas,"width",0));a.height=static_cast<int>(data::num(atlas,"height",0));
  if(a.width<8||a.height<8||a.width>256||a.height>256)return fail("Invalid VFX atlas dimensions");
  a.rgba.assign(static_cast<std::size_t>(a.width)*a.height*4,0);
  const auto* tiles=atlas.get("tiles");if(!tiles||!tiles->array()||tiles->array()->size()>32)return fail("Invalid VFX atlas tiles");
  for(const auto& tile:*tiles->array()) {
    const auto name=data::str(tile,"name");const auto* rows=tile.get("rows");
    if(name.empty()||!rows||!rows->array()||rows->array()->empty())return fail("Invalid VFX atlas tile");
    AtlasRegion region{static_cast<int>(data::num(tile,"x",0)),static_cast<int>(data::num(tile,"y",0)),0,static_cast<int>(rows->array()->size())};
    if(!rows->array()->front().string())return fail("Invalid VFX pixel row");
    region.w=static_cast<int>(rows->array()->front().string()->size());
    if(region.x<0||region.y<0||region.w<1||region.x+region.w>a.width||region.y+region.h>a.height)return fail("VFX atlas tile out of bounds");
    for(int y=0;y<region.h;++y) {
      const auto* row=(*rows->array())[y].string();if(!row||row->size()!=static_cast<std::size_t>(region.w))return fail("Inconsistent VFX pixel rows");
      for(int x=0;x<region.w;++x){const char value=(*row)[x];if(value<'0'||value>'9')return fail("VFX alpha must be a digit");
        const auto at=((region.y+y)*a.width+region.x+x)*4;a.rgba[at]=a.rgba[at+1]=a.rgba[at+2]=255;a.rgba[at+3]=static_cast<std::uint8_t>((value-'0')*255/9);}
    }
    if(!a.regions.emplace(name,region).second)return fail("Duplicate VFX tile");
  }
  for(const char* name: {"level_up","melee_hit_small","foot_dust","bowl_ember_idle","burning_touch_contact","simple_death_puff","projectile_trail_simple","war_cry","dash_dust","critical_hit","pickup_motes"}) {
    data::J doc;if(!data::read(root+"/"+name+".effect.json",doc,a.error))return fail(a.error);
    Effect effect;effect.name=name;effect.duration=data::num(doc,"duration",1);effect.loop=data::flag(doc,"loop");
    if(!std::isfinite(effect.duration)||effect.duration<=0||effect.duration>10)return fail("Invalid VFX effect duration");
    const auto* layers=doc.get("layers");if(!layers||!layers->array()||layers->array()->empty()||layers->array()->size()>8)return fail("Invalid VFX layers");
    for(const auto& j:*layers->array()) {
      Layer l;l.name=data::str(j,"name");l.continuous=data::str(j,"spawn")=="continuous";
      l.local=data::str(j,"space")=="local";l.additive=data::str(j,"blend")=="additive";l.glow=data::flag(j,"glow");
      l.delay=data::num(j,"delay",0);l.duration=data::num(j,"duration",effect.duration);
      l.rate=data::num(j,"rate",0);l.count=static_cast<int>(data::num(j,"count",1));
      l.radial=data::str(j,"shape")=="ring";l.radius=data::num(j,"radius",0);l.radial_speed=data::num(j,"radialSpeed",0);
      if(!std::isfinite(l.radius)||!std::isfinite(l.radial_speed)||l.radius<0||l.radius>200||std::abs(l.radial_speed)>400)return fail("Invalid radial VFX bounds");
      const auto life=data::values<2>(j,"lifetime",{.2f,.4f});l.life_min=life[0];l.life_max=life[1];
      l.drag=data::num(j,"drag",0);l.spin=data::num(j,"spin",0);l.fps=data::num(j,"fps",0);
      l.pos_min=data::vec(j,"positionMin");l.pos_max=data::vec(j,"positionMax");
      l.vel_min=data::vec(j,"velocityMin");l.vel_max=data::vec(j,"velocityMax");l.accel=data::vec(j,"acceleration");
      l.size_start=data::values<2>(j,"sizeStart",{3,3});l.size_end=data::values<2>(j,"sizeEnd",{1,1});
      l.color_start=data::values<4>(j,"colorStart",{1,1,1,1});l.color_end=data::values<4>(j,"colorEnd",{1,1,1,0});
      for(float f: {l.delay,l.duration,l.rate,l.life_min,l.life_max,l.drag,l.spin,l.fps})if(!std::isfinite(f))return fail("Nonfinite VFX parameter");
      if(l.delay<0||l.duration<=0||l.duration>10||l.rate<0||l.rate>120||l.count<0||l.count>96||l.life_min<=0||l.life_max<l.life_min||l.life_max>5||l.drag<0||l.drag>20)return fail("VFX layer exceeds budget");
      for(float size: {l.size_start[0],l.size_start[1],l.size_end[0],l.size_end[1]})if(size<0||size>400)return fail("Invalid particle size");
      for(float color:l.color_start)if(!std::isfinite(color)||color<0||color>2)return fail("Invalid VFX color");
      for(float color:l.color_end)if(!std::isfinite(color)||color<0||color>2)return fail("Invalid VFX color");
      if(l.color_start[3]>1||l.color_end[3]>1||l.fps<0||l.fps>60||std::abs(l.spin)>30)return fail("Invalid VFX curve");
      for(Vec3 v:{l.pos_min,l.pos_max,l.vel_min,l.vel_max,l.accel})
        for(float f:{v.x,v.y,v.z})if(!std::isfinite(f)||std::abs(f)>2000)return fail("Invalid VFX vector");
      const auto* frames=j.get("frames");if(!frames||!frames->array()||frames->array()->empty()||frames->array()->size()>8)return fail("Invalid particle frames");
      for(const auto& frame:*frames->array()){if(!frame.string()||!a.regions.contains(*frame.string()))return fail("Unknown particle frame");l.frames.push_back(*frame.string());}
      effect.layers.push_back(std::move(l));
    }
    a.effects.emplace(name,std::move(effect));
  }
  destination=std::move(a);return true;
}

class ParticleSystem {
public:
  static constexpr std::size_t max_particles=384,max_emitters=32;
  using Resolve=std::function<std::optional<Vec3>(const Attachment&)>;
  struct Particle {Vec3 pos{},previous{},velocity{};float age=0,life=1,rotation=0;std::uint32_t emitter=0;const Layer* layer=nullptr;Attachment attachment{};};
  struct Emitter {std::uint32_t id=0,rng=1;const Effect* effect=nullptr;Attachment attachment{};float age=0;std::array<float,8> carry{};std::array<bool,8> burst{};};
  Assets assets;std::vector<Particle> particles;std::vector<Emitter> emitters;std::uint64_t spawned=0,dropped=0;
  std::uint32_t play(const std::string& name,Attachment anchor,std::uint32_t seed=1) {
    const auto effect=assets.effects.find(name);if(effect==assets.effects.end())return 0;
    if(emitters.size()>=max_emitters){++dropped;return 0;}
    const auto id=++serial_;emitters.push_back({id,seed?seed:1,&effect->second,std::move(anchor)});return id;
  }
  bool attach(std::uint32_t id,Attachment anchor) {
    for(auto& e:emitters)if(e.id==id) {
      e.attachment=anchor;
      for(auto& p:particles)if(p.emitter==id && p.layer->local)p.attachment=anchor;
      return true;
    }
    return false;
  }
  void stop(std::uint32_t id){std::erase_if(emitters,[&](const Emitter& e){return e.id==id;});}
  bool playing(std::uint32_t id)const{return std::any_of(emitters.begin(),emitters.end(),[&](const Emitter& e){return e.id==id;});}
  void clear(){particles.clear();emitters.clear();}
  void tick(const Resolve& resolve) {
    constexpr float dt=.05f;
    for(auto& p:particles){p.previous=p.pos;p.age+=dt;const auto& l=*p.layer;
      p.velocity=(p.velocity+l.accel*dt)*std::max(0.f,1-l.drag*dt);p.pos=p.pos+p.velocity*dt;p.rotation+=l.spin*dt;
      if(l.local&&!resolve(p.attachment))p.age=p.life;}
    std::erase_if(particles,[](const Particle& p){return p.age>=p.life;});
    for(auto& e:emitters) {
      const auto origin=resolve(e.attachment);if(!origin){e.age=100;continue;}
      for(std::size_t index=0;index<e.effect->layers.size();++index) {
        const auto& l=e.effect->layers[index];const float age=e.age-l.delay;
        if(age<0||age>=l.duration)continue;
        int count=0;
        if(l.continuous){e.carry[index]+=l.rate*dt;count=static_cast<int>(e.carry[index]);e.carry[index]-=count;}
        else if(!e.burst[index]){count=l.count;e.burst[index]=true;}
        const float phase=l.radial?unit(e)*6.2831853f:0;
        for(int n=0;n<count;++n) {
          if(particles.size()>=max_particles){++dropped;continue;}
          Particle p;p.layer=&l;p.emitter=e.id;p.attachment=e.attachment;
          p.pos=random(e,l.pos_min,l.pos_max)+(l.local?Vec3{}:*origin);p.previous=p.pos;
          p.velocity=random(e,l.vel_min,l.vel_max);p.life=mix(l.life_min,l.life_max,unit(e));
          if(l.radial) {
            const float angle=phase+6.2831853f*float(n)/float(std::max(1,count));
            const Vec3 direction{std::cos(angle),std::sin(angle),0};
            p.pos=p.pos+direction*l.radius;p.previous=p.pos;p.velocity=p.velocity+direction*l.radial_speed;
          }
          p.rotation=0;particles.push_back(std::move(p));++spawned;
        }
      }
      e.age+=dt;
      if(e.effect->loop && e.age>=e.effect->duration && e.age<100){e.age=0;e.carry={};e.burst={};}
    }
    std::erase_if(emitters,[](const Emitter& e){return e.age>=100||(!e.effect->loop&&e.age>=e.effect->duration);});
  }
  static float mix(float a,float b,float t){return a+(b-a)*t;}
  static float unit(Emitter& e){e.rng^=e.rng<<13;e.rng^=e.rng>>17;e.rng^=e.rng<<5;return (e.rng&0xffffff)/16777216.f;}
  static Vec3 random(Emitter& e,Vec3 a,Vec3 b){return {mix(a.x,b.x,unit(e)),mix(a.y,b.y,unit(e)),mix(a.z,b.z,unit(e))};}
private:std::uint32_t serial_=0;
};
}
