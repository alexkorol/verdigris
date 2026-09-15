#include "../../../../native/client/fable_gpu.hpp"
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <numeric>
using namespace fable_gpu;
int failures=0;
void check(bool value,const char* label){std::printf("%s %s\n",value?"PASS":"FAIL",label);if(!value)++failures;}
void save(Renderer& r,const char* name){std::ofstream o(std::string(".ci-artifacts/fable-renderer/probes/")+name+".bgra",std::ios::binary);auto p=r.pixels_bgra();o.write((const char*)p.data(),p.size());}
std::array<int,3> rgb(const Renderer& r,int x,int y){auto p=r.pixels_bgra();auto i=(y*r.width()+x)*4;return {p[i+2],p[i+1],p[i]};}
std::uint64_t hash(const Renderer& r){std::uint64_t h=14695981039346656037ull;for(auto c:r.pixels_bgra())h=(h^c)*1099511628211ull;return h;}
Camera camera(int w,int h,float zoom){Camera c;c.width=w;c.height=h;c.cam_x=c.cam_y=0;c.horizon=-.45f*h;c.dzp=(.65f*h-c.horizon)/zoom;c.k=zoom*c.dzp;c.a=(.65f*h-c.horizon)*c.dzp;c.d0=c.dzp;c.near_depth=40;c.far_depth=c.dzp*3.2f;return c;}
int main(int argc,char** argv){
 const bool benchmark=argc<2||std::strcmp(argv[1],"--skip-benchmark")!=0;
 Renderer r;check(r.initialize(),"real hardware D3D11 initializes");if(!r.error().empty())std::printf("%s\n",r.error().c_str());if(!r.stats().hardware)return 1;
 std::printf("ADAPTER %s vendor=%04x device=%04x feature=%x\n",r.adapter_name().c_str(),r.stats().vendor_id,r.stats().device_id,r.stats().feature_level);
 std::vector<std::uint8_t> ground(128*128*4),sprite(16*24*4),stone(8*8*4);
 for(int y=0;y<128;y++)for(int x=0;x<128;x++){auto i=(y*128+x)*4;int c=((x/4+y/4)%2)?90:65;ground[i]=static_cast<std::uint8_t>(c);ground[i+1]=static_cast<std::uint8_t>(c+22);ground[i+2]=static_cast<std::uint8_t>(c-5);ground[i+3]=255;}
 for(int y=0;y<24;y++)for(int x=0;x<16;x++){auto i=(y*16+x)*4;sprite[i]=x<3?20:240;sprite[i+1]=x<3?220:30;sprite[i+2]=20;sprite[i+3]=(x>=6&&x<10&&y>=10&&y<14)?0:255;}
 for(int i=0;i<64;i++){stone[i*4]=45;stone[i*4+1]=100;stone[i*4+2]=180;stone[i*4+3]=255;}
 check(r.upload_texture(1,128,128,ground.data(),512,true,1),"mipmapped terrain upload");
 check(r.upload_texture(2,16,24,sprite.data(),64,false,1),"straight alpha sprite upload");
 check(r.upload_texture(3,8,8,stone.data(),32,false,1),"point wall texture upload");
 const auto uploads=r.stats().texture_uploads;check(r.upload_texture(2,16,24,sprite.data(),64,false,1)&&r.stats().texture_uploads==uploads,"same revision reuses texture");
 std::vector<Vertex> vertices;std::vector<std::uint32_t> indices;
 for(int y=0;y<=40;y++)for(int x=0;x<=40;x++){float wx=-1200+x*60.f,wy=-1500+y*45.f;vertices.push_back({wx,wy,0,x/40.f,y/40.f});}
 for(int y=0;y<40;y++)for(int x=0;x<40;x++){unsigned a=y*41+x;for(unsigned q:{a,a+41,a+1,a+1,a+41,a+42})indices.push_back(q);}
 Scene scene;scene.camera=camera(640,480,1.5f);scene.terrain={1,vertices,indices};scene.dof_strength=0;scene.vignette=0;
 check(r.render(scene,nullptr),"offscreen terrain renders and reads back");if(!r.error().empty())std::printf("%s\n",r.error().c_str());save(r,"ground");
 const auto hole_ground=rgb(r,320,240),foot_ground=rgb(r,320,314);
 Sprite actor;actor.texture=2;actor.width=64;actor.height=96;
 scene.sprites={&actor,1};check(r.render(scene,nullptr),"shared projected billboard renders");save(r,"actor");
 check(rgb(r,320,280)==std::array<int,3>{240,30,20},"focus sprite point samples exact RGB");
 check(rgb(r,320,240)==hole_ground,"transparent hole preserves terrain");
 check(rgb(r,320,311)==std::array<int,3>{240,30,20}&&rgb(r,320,314)==foot_ground,"bottom-center anchor agrees with terrain focus projection");
 check(rgb(r,278,280)==std::array<int,3>{20,220,20},"left texels retain nearest sampling");
 const auto framehash=hash(r);const auto before=r.stats();bool stable=true;
 for(int i=0;i<12;i++){stable&=r.render(scene,nullptr);stable&=hash(r)==framehash;}
 check(stable&&r.stats().texture_uploads==before.texture_uploads&&r.stats().texture_bytes==before.texture_bytes,"repeat frames are deterministic without texture allocation");
 actor.flip=true;check(r.render(scene,nullptr)&&rgb(r,362,280)==std::array<int,3>{20,220,20},"flip moves source texels about same pivot");actor.flip=false;
 actor.flash=1;check(r.render(scene,nullptr)&&rgb(r,320,280)[0]==255&&rgb(r,320,240)==hole_ground,"flash silhouette preserves alpha holes");actor.flash=0;
 actor.rotation=1.57079632679f;check(r.render(scene,nullptr),"weapon/effect screen-plane rotation renders");save(r,"rotation");actor.rotation=0;
 std::array<Vertex,4> wall_v{{{-30,40,90,0,0},{30,40,90,1,0},{-30,40,0,0,1},{30,40,0,1,1}}};
 std::array<std::uint32_t,6> wall_i{{0,2,1,1,2,3}};Mesh wall{3,wall_v,wall_i,true,1};scene.world_meshes={&wall,1};
 check(r.render(scene,nullptr)&&rgb(r,320,280)==std::array<int,3>{45,100,180},"near wall mesh depth occludes sprite");save(r,"occlusion");
 wall.opacity=.3f;check(r.render(scene,nullptr)&&rgb(r,320,280)[0]>150&&rgb(r,320,280)[2]>40,"transparent wall blends above retained actor without depth deletion");save(r,"cutaway");scene.world_meshes={};
 scene.ambient_r=.35f;scene.ambient_g=.4f;scene.ambient_b=.65f;
 check(r.render(scene,nullptr),"world grade renders");const auto dark=rgb(r,320,280);
 Light light;light.radius=200;light.r=1;light.g=.7f;light.b=.3f;light.intensity=.9f;scene.lights={&light,1};
 check(r.render(scene,nullptr)&&rgb(r,320,280)[0]>dark[0]+40,"quarter-resolution projected light brightens graded world");save(r,"lighting");scene.lights={};scene.ambient_r=scene.ambient_g=scene.ambient_b=1;
 scene.dof_strength=1;actor.y=-250;check(r.render(scene,nullptr),"continuous defocus/fog render distant sprite");save(r,"dof");
 auto old=hash(r);actor.y=-251;check(r.render(scene,nullptr)&&hash(r)!=old,"small depth change produces distinct continuous DoF frame");actor.y=0;scene.dof_strength=0;
 auto bad=scene;bad.camera.width=0;check(!r.render(bad,nullptr)&&!r.error().empty(),"zero viewport fails explicitly without poisoning renderer");
 bad=scene;bad.camera.a=std::numeric_limits<float>::quiet_NaN();check(!r.render(bad,nullptr),"NaN camera rejected");
 auto invalid=indices;invalid[0]=999999;bad=scene;bad.terrain.indices=invalid;check(!r.render(bad,nullptr),"out-of-range mesh index rejected");
 bad=scene;bad.terrain.texture=999999;check(!r.render(bad,nullptr),"missing texture rejected explicitly");
 std::vector<Light> excess(33);bad=scene;bad.lights=excess;check(!r.render(bad,nullptr),"light packet count bounded");
 check(r.render(scene,nullptr),"valid frame recovers after rejected inputs");


 std::array<Vertex,4> decal_v{{{-90,-80,.05f,0,0},{90,-80,.05f,1,0},{-90,80,.05f,0,1},{90,80,.05f,1,1}}};
 std::array<std::uint32_t,6> decal_i{0,2,1,1,2,3};Mesh decal{3,decal_v,decal_i,true,.3f};
 scene.ground_meshes={&decal,1};check(r.render(scene,nullptr)&&rgb(r,320,280)==std::array<int,3>{240,30,20}&&rgb(r,320,240)!=hole_ground,
     "translucent ground mesh blends through alpha hole before standing sprite");save(r,"ground-decal");scene.ground_meshes={};
 // A foreground-Y corpse/decal must remain beneath a standing actor. Input
 // order deliberately puts the actor first so this exercises the sort.
 Sprite foreground;foreground.texture=3;foreground.y=40;foreground.width=100;foreground.height=160;foreground.ground_layer=true;
 std::array<Sprite,2> layered{actor,foreground};scene.sprites=layered;
 check(r.render(scene,nullptr)&&rgb(r,320,280)==std::array<int,3>{240,30,20}&&rgb(r,320,240)==std::array<int,3>{45,100,180},
       "foreground-Y ground billboard sorts beneath actor and remains visible through alpha hole");save(r,"ground-layer");
 layered[1].ground_layer=false;
 check(r.render(scene,nullptr)&&rgb(r,320,280)==std::array<int,3>{45,100,180},
       "ordinary foreground-Y billboard still occludes actor");save(r,"standing-layer");scene.sprites={&actor,1};
 // Clip a world triangle that crosses the near plane. The vertex endpoints
 // are constructed from screen coordinates; expected crossing is x250 at y100.
 auto clipped_vertex=[&](float sx,float sy,float dz){const auto& c=scene.camera;return Vertex{
   c.cam_x+(sx-c.width*.5f)*dz/c.k,c.d0-dz,(c.horizon*dz+c.a-sy*dz)/c.k,0,0};};
 std::array<Vertex,3> near_v{{clipped_vertex(100,100,100),clipped_vertex(500,100,20),clipped_vertex(100,400,100)}};
 std::array<std::uint32_t,3> near_i{0,1,2};Mesh near_mesh{3,near_v,near_i,true,1};
 Scene clipped=scene;clipped.terrain={};clipped.sprites={};clipped.world_meshes={&near_mesh,1};
 check(r.render(clipped,nullptr)&&rgb(r,150,150)==std::array<int,3>{45,100,180}&&rgb(r,300,110)!=std::array<int,3>{45,100,180},
       "actual GPU triangle clips at affine near plane with unchanged x/y projection");save(r,"near-clip");
 Scene blur_scene=scene;blur_scene.terrain={};blur_scene.dof_strength=1;
 Sprite blur_actor=actor;blur_scene.sprites={&blur_actor,1};
 auto blur_depth=[&](float ratio){const auto& c=blur_scene.camera;const float dz=c.dzp*ratio;
   blur_actor.y=c.d0-dz;blur_actor.width=96*dz/c.k;blur_actor.height=144*dz/c.k;
   blur_actor.elevation=(c.horizon*dz+c.a-312*dz)/c.k;};
 blur_depth(1.10f);check(r.render(blur_scene,nullptr),"continuous blur reference depth renders");
 std::vector<std::uint8_t> blur_before(r.pixels_bgra().begin(),r.pixels_bgra().end());
 blur_depth(1.101f);check(r.render(blur_scene,nullptr),"continuous blur adjacent depth renders");
 int max_blur_delta=0;std::size_t changed_blur=0;
 for(std::size_t i=0;i<blur_before.size();++i){const int d=std::abs(int(blur_before[i])-int(r.pixels_bgra()[i]));max_blur_delta=std::max(max_blur_delta,d);changed_blur+=d!=0;}
 check(changed_blur>0&&max_blur_delta<=4,"subpixel DoF change remains continuous with fixed projected pose");
 std::printf("DOF adjacent_depth_changed_bytes=%zu max_channel_delta=%d\n",changed_blur,max_blur_delta);
 check(!r.upload_texture(9999,8192,8192,ground.data(),32768,true,1),"oversized mip chain rejected before allocation or source read");

 // Clouds are a screen-space ambient multiply, independent of world camera.
 Scene cloud_scene;cloud_scene.camera=scene.camera;cloud_scene.vignette=0;cloud_scene.dof_strength=0;
 cloud_scene.sky_r=cloud_scene.sky_g=cloud_scene.sky_b=.8f;
 check(r.render(cloud_scene,nullptr),"cloud-free reference renders");const auto no_cloud_hash=hash(r);
 const auto clear_center=rgb(r,192,240),clear_edge=rgb(r,308,240),clear_outside=rgb(r,520,240);
 Cloud cloud{192,240,160};cloud_scene.clouds={&cloud,1};
 check(r.render(cloud_scene,nullptr),"screen-space cloud lightmap renders");save(r,"cloud-shadow");
 const auto cloudy_center=rgb(r,192,240),cloudy_edge=rgb(r,308,240);
 check(std::abs(cloudy_center[0]-clear_center[0]*196/255)<=1&&
       std::abs(cloudy_center[1]-clear_center[1]*198/255)<=1&&
       std::abs(cloudy_center[2]-clear_center[2]*208/255)<=1&&rgb(r,520,240)==clear_outside,
       "cloud has exact reference inner tint and clear exterior");
 check(std::abs(cloudy_edge[0]-clear_edge[0]*(196+255)/510)<=2&&cloudy_edge[0]>cloudy_center[0],
       "cloud radius0.45 to1 uses continuous linear radial fade");

 Light cloud_lamp;cloud_lamp.x=(192-scene.camera.width*.5f)*scene.camera.dzp/scene.camera.k;
 cloud_lamp.y=scene.camera.d0-scene.camera.dzp;
 cloud_lamp.elevation=(scene.camera.horizon+scene.camera.a/scene.camera.dzp-240)*scene.camera.dzp/scene.camera.k;
 cloud_lamp.radius=200;cloud_lamp.intensity=.15f;
 cloud_scene.clouds={};cloud_scene.lights={&cloud_lamp,1};check(r.render(cloud_scene,nullptr),"local light under clear sky renders");
 const auto lit_clear=rgb(r,192,240);cloud_scene.clouds={&cloud,1};check(r.render(cloud_scene,nullptr),"local light beneath cloud renders");
 const auto lit_cloud=rgb(r,192,240);
 check(std::abs((lit_cloud[0]-cloudy_center[0])-(lit_clear[0]-clear_center[0]))<=1,
       "cloud multiplies ambient before lights without dimming local additive contribution");
 cloud_scene.lights={};check(r.render(cloud_scene,nullptr),"cloud ambient-only state restored");
 const auto cloud_hash=hash(r);const auto cloud_stats=r.stats();
 cloud_scene.camera.cam_x+=300;cloud_scene.camera.cam_y-=700;cloud_scene.camera.d0-=700;
 check(r.render(cloud_scene,nullptr)&&hash(r)==cloud_hash&&r.stats().texture_uploads==cloud_stats.texture_uploads,
       "cloud pixels remain screen-stationary under camera pan without resource uploads");
 cloud_scene.camera=scene.camera;cloud.strength=0;
 check(r.render(cloud_scene,nullptr)&&hash(r)==no_cloud_hash,"zero-strength cloud preserves original pixels");cloud.strength=1;
 std::array<Cloud,2> pair{cloud,cloud};cloud_scene.clouds=pair;
 check(r.render(cloud_scene,nullptr)&&std::abs(rgb(r,192,240)[0]-clear_center[0]*196*196/(255*255))<=1,
       "overlapping clouds multiply rather than add shadow tint");
 std::vector<Cloud> too_many_clouds(9,cloud);cloud_scene.clouds=too_many_clouds;
 check(!r.render(cloud_scene,nullptr),"cloud count is bounded at8");cloud_scene.clouds={&cloud,1};
 cloud.radius=0;check(!r.render(cloud_scene,nullptr),"zero cloud radius rejected");cloud.radius=160;
 cloud.screen_x=std::numeric_limits<float>::quiet_NaN();check(!r.render(cloud_scene,nullptr),"NaN screen cloud rejected");cloud.screen_x=192;
 cloud.strength=1.1f;check(!r.render(cloud_scene,nullptr),"cloud strength outside0to1 rejected");cloud.strength=1;
 cloud_scene.clouds={};check(r.render(cloud_scene,nullptr)&&hash(r)==no_cloud_hash,"empty cloud packet retains prior renderer pixels after errors");
 // Real HDC path, not a separate exporter transform.
 BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=640;info.bmiHeader.biHeight=-480;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
 void* bits=nullptr;HDC dc=CreateCompatibleDC(nullptr);HBITMAP bm=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,nullptr,0);auto prior=SelectObject(dc,bm);
 check(r.render(scene,dc),"actual HDC composite succeeds");GdiFlush();check(std::memcmp(bits,r.pixels_bgra().data(),640*480*4)==0,"HDC bytes equal exact GPU BGRA readback");SelectObject(dc,prior);DeleteObject(bm);DeleteDC(dc);
 // Fullscreen readback cost on actual hardware, including mesh/sprites/light/post.
 if(benchmark) {
 std::vector<Sprite> crowd(80,actor);for(int i=0;i<80;i++){crowd[i].x=(i%10-5)*90.f;crowd[i].y=(i/10-4)*70.f;crowd[i].width=30;crowd[i].height=45;}
 scene.camera=camera(3440,1440,2.04f);scene.sprites=crowd;scene.dof_strength=1;scene.lights={&light,1};
 check(r.render(scene,nullptr),"3440x1440 world target allocation and render");save(r,"fullscreen");double total=0;
 for(int i=0;i<20;i++){check(r.render(scene,nullptr),"fullscreen frame succeeds");total+=r.stats().render_readback_ms;}
 std::printf("PERF 3440x1440 frames=20 mean_ms=%.3f draws=%u triangles=%u sprites=%u\n",total/20,r.stats().draw_calls,r.stats().terrain_triangles,r.stats().sprites);
 }
 std::array<std::uint8_t,4> pixel{255,255,255,255};
 for(std::uint64_t i=10;i<525;i++)if(!r.upload_texture(i,1,1,pixel.data(),4,false,1))check(false,"bounded cache upload");
 check(r.stats().texture_count<=512&&r.stats().texture_bytes<=256u*1024u*1024u&&r.stats().texture_evictions>0,"texture LRU enforces count and byte bounds");
 r.reset();check(r.stats().texture_count==0&&!r.has_texture(1,1)&&r.initialize(),"explicit device reset clears device-bound cache and recovers hardware");
 std::printf("RESULT failures=%d\n",failures);return failures?1:0;
}
