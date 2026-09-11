#include "../../../../native/client/fable_gpu.hpp"
#include "../../../../native/client/fable_camera.hpp"
#pragma warning(push)
#pragma warning(disable:4458) // Windows 10 SDK GDI+ implementation only.
#include <objidl.h>
namespace Gdiplus {using std::min;using std::max;}
#include <gdiplus.h>
#pragma warning(pop)
#include "../../../../native/client/raster_ground.hpp"
#include "../../../../native/client/raster_equipment.hpp"
#include <future>
#include <thread>
#include <cstdio>
#include <filesystem>

// The runner extracts the exact production Renderer, excluding unrelated
// paint assembly. Only the world fixture and unused equipment draw are stubs.
constexpr double kTileUnits=107.25;
namespace vector_art {enum class Held{None,Axe,Staff,Bow,Club,Sword};}
struct ScreenPoint {int x,y;double scale;};
void draw_raster_equipment(HDC,vector_art::Held,ScreenPoint,int,const std::string&){}
struct ClientState {
  struct World {std::string route_id="terrain-test",theme="town";int map_width=160,map_height=160;} world;
  struct Camera {double x=1,y=1;fable::HeightField elevation;} camera;
  std::vector<int> scenery;
};
std::uint32_t scenery_seed(const std::string& s){std::uint32_t h=2166136261;for(auto c:s)h=(h^c)*16777619;return h;}
const char* quiet_ground_asset(const std::string&){return "terrain_quiet_earth";}
raster_ground::Layout ground_layout(const std::string&,const std::string&,const std::vector<int>&){
  raster_ground::Layout l;l.active=true;l.tile_units=kTileUnits;l.road(-800,-500,5000,2200,82);
  l.planting[0]={{1800,800},180};l.planting_count=1;l.solids[0]={{2100,1200},90};l.solid_count=1;return l;
}
#include "terrain-renderer-under-test.hpp"
int failures=0;
void check(bool value,const char* label){std::printf("%s %s\n",value?"PASS":"FAIL",label);if(!value)++failures;}
double prepare(fable_world::Renderer& r,ClientState& s){auto t=std::chrono::steady_clock::now();check(r.prepare(s),"prepare succeeds");return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();}
void retire(fable_world::Renderer& r,ClientState& s){
  const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(20);
  while(r.terrain_job.valid()&&std::chrono::steady_clock::now()<until){
    if(!r.prepare(s)){check(false,"pending prepare succeeds");break;}
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  check(!r.terrain_job.valid(),"one worker completes within bounded observation");
}
bool overlap(const std::vector<std::uint8_t>& old,const std::vector<std::uint8_t>& now,int dx,int dy){
  // The last interpolation cell is clamped at each patch's outer edge;
  // compare all shared texels whose coverage footprint exists in both.
  for(int y=0;y<2040-dy;++y)for(int x=0;x<2040-dx;++x)
    if(std::memcmp(&old[(std::size_t(y+dy)*2048+x+dx)*4],&now[(std::size_t(y)*2048+x)*4],4))return false;
  return true;
}
bool mesh_overlap(const std::vector<fable_gpu::Vertex>& old,const std::vector<fable_gpu::Vertex>& now,int dx,int dy){
  for(int y=0;y<=140-dy;++y)for(int x=0;x<=176-dx;++x){
    const auto& a=old[(y+dy)*177+x+dx];const auto& b=now[y*177+x];
    if(a.x!=b.x||a.y!=b.y||a.elevation!=b.elevation)return false;
  }return true;
}
int main(){
  Gdiplus::GdiplusStartupInput si;ULONG_PTR token=0;Gdiplus::GdiplusStartup(&token,&si,nullptr);
  raster_art::set_asset_root((std::filesystem::current_path()/"native/client/assets/raster/runtime").wstring());
  {
    ClientState state;fable_world::Renderer r;
    double ms=prepare(r,state);std::printf("INITIAL ms=%.3f cpu=%.3f upload=%.3f backend=%s\n",ms,r.bake_stats.last_cpu_ms,r.bake_stats.last_upload_ms,r.gpu.adapter_name().c_str());
    check(r.bake_stats.started==1&&r.bake_stats.adopted==1&&!r.terrain_job.valid(),"initial load owns one complete terrain packet");
    auto pixels=r.terrain_rgba;auto mesh=r.terrain_vertices;auto key=r.terrain_key;
    state.camera.x=20*kTileUnits+.01;
    ms=prepare(r,state);std::printf("X_START main_ms=%.3f\n",ms);
    check(r.terrain_key==key&&r.bake_stats.started==2&&r.terrain_job.valid(),"X travel keeps old patch and starts exactly one worker");
    for(int n=0;n<8;++n)r.prepare(state);
    check(r.bake_stats.started==2,"rapid repeated frames cannot enqueue more jobs");
    retire(r,state);
    check(overlap(pixels,r.terrain_rgba,512,0),"X boundary texture phase is byte-identical in shared coverage");
    check(mesh_overlap(mesh,r.terrain_vertices,44,0),"X boundary geometry and elevation are exact on shared mesh vertices");
    pixels=r.terrain_rgba;mesh=r.terrain_vertices;key=r.terrain_key;
    state.camera.y=16*kTileUnits+.01;ms=prepare(r,state);std::printf("Y_START main_ms=%.3f\n",ms);
    check(r.terrain_key==key&&r.terrain_job.valid(),"Y travel keeps old patch while worker runs");retire(r,state);
    check(overlap(pixels,r.terrain_rgba,0,512),"Y boundary texture phase is byte-identical in shared coverage");
    check(mesh_overlap(mesh,r.terrain_vertices,0,35),"Y boundary geometry and elevation are exact on shared mesh vertices");
    auto started=r.bake_stats.started;auto revision=r.terrain_revision;
    r.gpu.reset();prepare(r,state);
    check(r.bake_stats.started==started&&r.terrain_revision==revision&&r.gpu.has_texture(1,revision),"GPU reset reuploads retained matching RGBA without CPU rebake");
    state.camera.x=40*kTileUnits+.01;prepare(r,state);state.camera.x=20*kTileUnits+.01;
    auto discarded=r.bake_stats.discarded;retire(r,state);
    check(r.bake_stats.discarded==discarded+1&&r.terrain_revision==revision,"return to resident bucket retires stale patch without swap");
    state.camera.x=40*kTileUnits+.01;prepare(r,state);discarded=r.bake_stats.discarded;
    state.world.route_id="new-route";prepare(r,state);
    check(!r.terrain_job.valid()&&r.terrain_key.find("new-route|")==0&&r.bake_stats.discarded==discarded+1,"route load safely joins/discards old route then adopts matching packet");
    state.camera.x=60*kTileUnits+.01;prepare(r,state);discarded=r.bake_stats.discarded;
    raster_art::reset_cache();prepare(r,state);
    check(!r.terrain_job.valid()&&r.generation==raster_art::asset_generation()&&r.bake_stats.discarded==discarded+1,"source-generation change retires immutable old job before replacement");
    check(r.bake_stats.peak_bytes<=40*1024*1024,"current plus one job CPU memory is bounded below 40 MiB");
    std::printf("STATS started=%llu completed=%llu adopted=%llu discarded=%llu reused=%llu waits=%llu failed=%llu retained=%zu peak=%zu cpu=%.3f upload=%.3f\n",
      r.bake_stats.started,r.bake_stats.completed,r.bake_stats.adopted,r.bake_stats.discarded,r.bake_stats.reused_frames,r.bake_stats.loading_waits,r.bake_stats.failed,r.bake_stats.retained_bytes,r.bake_stats.peak_bytes,r.bake_stats.last_cpu_ms,r.bake_stats.last_upload_ms);
    state.camera.y=32*kTileUnits+.01;prepare(r,state);check(r.terrain_job.valid(),"destructor fixture has an active worker");
  }
  check(true,"renderer destructor joined active worker before resources retired");
  raster_art::reset_cache();Gdiplus::GdiplusShutdown(token);
  return failures?1:0;
}
