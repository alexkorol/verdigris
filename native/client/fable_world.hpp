#pragma once

// Native scene assembly for the Fable GPU passes. Included after client drawing
// helpers. This layer reads WorldView; it never owns collision or gameplay.
namespace fable_world {
struct TextureInfo {
  std::uint64_t id = 0;
  int width = 0, height = 0, native_height = 0;
  float anchor_x = .5f, anchor_y = 1;
};
// One immutable CPU-only packet. Capturing these values prevents worker reads
// from renderer caches, mutable scenery, GPU resources or the live camera.
struct TerrainBakeInput {
  std::string key, scene_key;
  std::uint64_t generation = 0;
  double min_x = 0, min_y = 0, max_x = 0, max_y = 0;
  bool interior = false;
  raster_ground::detail::Pixels earth{}, moss{};
  raster_ground::Layout layout{};
  fable::HeightField elevation{};
};
struct TerrainBakeResult {
  std::string key, scene_key;
  std::uint64_t generation = 0;
  double min_x = 0, min_y = 0, max_x = 0, max_y = 0, cpu_ms = 0;
  std::vector<std::uint8_t> rgba;
  std::vector<fable_gpu::Vertex> vertices;
  std::vector<std::uint32_t> indices;
};
struct TerrainBakeStats {
  std::uint64_t started = 0, completed = 0, adopted = 0, discarded = 0;
  std::uint64_t reused_frames = 0, loading_waits = 0, failed = 0;
  double last_cpu_ms = 0, last_upload_ms = 0;
  std::size_t retained_bytes = 0, peak_bytes = 0;
  bool running = false;
};
// One world tile always contains 48 authored ground texels in both axes.
inline constexpr int kTerrainPixelsPerTile = 48;
inline constexpr int kTerrainTilesX = 80, kTerrainTilesY = 64;
inline constexpr int kTerrainWidth = kTerrainTilesX*kTerrainPixelsPerTile;
inline constexpr int kTerrainHeight = kTerrainTilesY*kTerrainPixelsPerTile;
inline constexpr int kTerrainNx = 176, kTerrainNy = 140;
inline constexpr int kTerrainFieldStep = 8;
inline constexpr int kTerrainFieldWidth = kTerrainWidth/kTerrainFieldStep+1;
inline constexpr int kTerrainFieldHeight = kTerrainHeight/kTerrainFieldStep+1;
// GPU upload_texture accepts rectangular dimensions up to 8192. Keep this
// bound explicit so a future world extent cannot silently exceed that API.
static_assert(kTerrainWidth<=8192 && kTerrainHeight<=8192);
// Conservative CPU budget: resident packet plus one job, including coverage,
// two reusable interpolation rows and the logical tile's material lookup.
inline constexpr std::size_t kTerrainJobBytes =
    std::size_t(kTerrainWidth)*kTerrainHeight*4 +
    std::size_t(kTerrainNx+1)*(kTerrainNy+1)*sizeof(fable_gpu::Vertex) +
    std::size_t(kTerrainNx)*kTerrainNy*6*sizeof(std::uint32_t) +
    (std::size_t(kTerrainFieldWidth)*kTerrainFieldHeight+2*kTerrainWidth)*sizeof(raster_ground::Coverage) +
    std::size_t(kTerrainPixelsPerTile)*kTerrainPixelsPerTile*144*sizeof(std::uint32_t) +
    std::size_t(kTerrainWidth)*sizeof(int) + sizeof(TerrainBakeInput);

inline TerrainBakeResult bake_terrain(const TerrainBakeInput& input) {
  const auto started = std::chrono::steady_clock::now();
  TerrainBakeResult result;
  result.key=input.key;result.scene_key=input.scene_key;result.generation=input.generation;
  result.min_x=input.min_x;result.min_y=input.min_y;result.max_x=input.max_x;result.max_y=input.max_y;
  const double min_x=input.min_x,min_y=input.min_y,max_x=input.max_x,max_y=input.max_y;
  constexpr int width=kTerrainWidth,height=kTerrainHeight,block=kTerrainFieldStep;
  constexpr int field_width=kTerrainFieldWidth,field_height=kTerrainFieldHeight;
  result.rgba.resize(std::size_t(width)*height*4);
  std::vector<raster_ground::Coverage> fields(std::size_t(field_width)*field_height);
  // Lattice nodes coincide with output pixel centers, including the guard
  // row/column. Tile-aligned patch changes preserve identical world samples.
  for(int y=0;y<field_height;++y) for(int x=0;x<field_width;++x)
    fields[y*field_width+x]=raster_ground::coverage(input.layout,
        min_x+(x*block+.5)*(max_x-min_x)/width,
        min_y+(y*block+.5)*(max_y-min_y)/height);
  // Nearest-neighbor sample the old 64px material into one 48px logical tile.
  // Repeating this discrete lookup never introduces fractional texture detail.
  std::vector<std::array<std::uint32_t,144>> colors(kTerrainPixelsPerTile*kTerrainPixelsPerTile);
  for(int y=0;y<kTerrainPixelsPerTile;++y) for(int x=0;x<kTerrainPixelsPerTile;++x) {
    const int px=(2*x+1)*raster_ground::kTilePixels/(2*kTerrainPixelsPerTile);
    const int py=(2*y+1)*raster_ground::kTilePixels/(2*kTerrainPixelsPerTile);
    auto& palette=colors[y*kTerrainPixelsPerTile+x];
    for(int shade=0;shade<4;++shade) for(int planting=0;planting<6;++planting) for(int road=0;road<6;++road)
      palette[shade*36+planting*6+road]=raster_ground::detail::material(
          input.earth[py*raster_ground::kTilePixels+px],input.moss[py*raster_ground::kTilePixels+px],
          {road/5.0,planting/5.0,shade/3.0});
  }
  const auto logical_coordinate=[](double world) {
    const auto pixel=static_cast<long long>(std::floor(world/kTileUnits*kTerrainPixelsPerTile));
    return static_cast<int>((pixel%kTerrainPixelsPerTile+kTerrainPixelsPerTile)%kTerrainPixelsPerTile);
  };
  std::vector<int> columns(width);
  for(int x=0;x<width;++x) columns[x]=logical_coordinate(min_x+(x+.5)*(max_x-min_x)/width);
  const auto mix=[](const auto& a,const auto& b,double t){return raster_ground::Coverage{
      a.road+(b.road-a.road)*t,a.planting+(b.planting-a.planting)*t,a.shade+(b.shade-a.shade)*t};};
  std::vector<raster_ground::Coverage> top(width),bottom(width);
  const auto horizontal=[&](int row,auto& destination) {
    const auto* source=fields.data()+row*field_width;
    for(int x=0;x<width;++x) destination[x]=mix(source[x/block],source[x/block+1],double(x%block)/block);
  };
  horizontal(0,top);horizontal(1,bottom);
  for(int y=0;y<height;++y) {
    if(y>0 && y%block==0) {top.swap(bottom);horizontal(y/block+1,bottom);}
    const int py=logical_coordinate(min_y+(y+.5)*(max_y-min_y)/height);
    const double fy=double(y%block)/block;
    auto* output=result.rgba.data()+std::size_t(y)*width*4;
    for(int x=0;x<width;++x) {
      auto field=mix(top[x],bottom[x],fy);
      if(!input.interior) field.planting=std::max(field.planting,.72*(1-field.road));
      const int road=std::clamp(static_cast<int>(field.road*5+.5),0,5);
      const int planting=std::clamp(static_cast<int>(field.planting*5+.5),0,5);
      const int shade=std::clamp(static_cast<int>(field.shade*3+.5),0,3);
      const auto pixel=colors[py*kTerrainPixelsPerTile+columns[x]][shade*36+planting*6+road];
      output[x*4]=static_cast<std::uint8_t>(pixel>>16);output[x*4+1]=static_cast<std::uint8_t>(pixel>>8);
      output[x*4+2]=static_cast<std::uint8_t>(pixel);output[x*4+3]=255;
    }
  }
  result.vertices.reserve(std::size_t(kTerrainNx+1)*(kTerrainNy+1));
  result.indices.reserve(std::size_t(kTerrainNx)*kTerrainNy*6);
  for(int y=0;y<=kTerrainNy;++y) for(int x=0;x<=kTerrainNx;++x) {
    const double wx=min_x+(max_x-min_x)*x/kTerrainNx,wy=min_y+(max_y-min_y)*y/kTerrainNy;
    result.vertices.push_back({static_cast<float>(wx),static_cast<float>(wy),
        static_cast<float>(fable::sample_height(input.elevation,wx,wy)),float(x)/kTerrainNx,float(y)/kTerrainNy});
  }
  for(int y=0;y<kTerrainNy;++y) for(int x=0;x<kTerrainNx;++x) {
    const auto n=static_cast<std::uint32_t>(y*(kTerrainNx+1)+x);
    result.indices.insert(result.indices.end(),{n,n+1,n+kTerrainNx+1,n+1,n+kTerrainNx+2,n+kTerrainNx+1});
  }
  result.cpu_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
  return result;
}

struct Renderer {
  fable_gpu::Renderer gpu;
  std::map<std::string, TextureInfo> textures;
  std::uint64_t generation = 0, next_id = 16;
  std::string terrain_key;
  double min_x = 0, min_y = 0, max_x = 0, max_y = 0;
  std::vector<fable_gpu::Vertex> terrain_vertices;
  std::vector<std::uint32_t> terrain_indices;

  TextureInfo upload(const std::string& key, const raster_art::detail::Surface& surface) {
    auto found = textures.find(key);
    if (found != textures.end()) {
      if(gpu.has_texture(found->second.id,generation)) return found->second;
      textures.erase(found);
    }
    const auto* src = static_cast<const std::uint32_t*>(surface.pixels);
    if (!src) return {};
    GdiFlush();
    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(surface.width) * surface.height * 4);
    for (std::size_t n = 0; n < rgba.size() / 4; ++n) {
      const unsigned a = src[n] >> 24;
      for (int c = 0; c < 3; ++c)
        rgba[n*4+c] = static_cast<std::uint8_t>(a ? std::min(255u,
            (((src[n] >> (16-c*8)) & 255u) * 255u + a/2) / a) : 0);
      rgba[n*4+3] = static_cast<std::uint8_t>(a);
    }
    TextureInfo out{next_id++, surface.width, surface.height};
    if (!gpu.upload_texture(out.id, out.width, out.height, rgba.data(), out.width*4, false, generation)) return {};
    if(textures.size()>=512) textures.clear();
    textures.emplace(key, out);
    return out;
  }

  TextureInfo texture(const std::string& name, vector_art::Held held = vector_art::Held::None) {
    const std::string key = name + ":" + std::to_string(static_cast<int>(held));
    const auto found = textures.find(key);
    if (found != textures.end() && gpu.has_texture(found->second.id,generation)) return found->second;
    const auto size = raster_art::dimensions(name.c_str());
    if (!size.valid()) return {};
    RECT canvas{0,0,size.width,size.height};
    if(held!=vector_art::Held::None) {
      const char* weapon=held==vector_art::Held::Handstone?"weapon_handstone":held==vector_art::Held::Axe?"weapon_axe":held==vector_art::Held::Staff?"weapon_staff":
          held==vector_art::Held::Bow?"weapon_bow":held==vector_art::Held::Club?"weapon_club":"weapon_sword";
      const auto plan=raster_equipment::compute(name.c_str(),weapon,size.width/2,size.height,size.height);
      if(plan.valid()) {RECT combined{};UnionRect(&combined,&canvas,&plan.weapon_bounds);canvas=combined;}
    }
    raster_art::detail::Surface surface;
    if (!surface.create(canvas.right-canvas.left, canvas.bottom-canvas.top)) return {};
    const int cx=size.width/2-canvas.left,feet=size.height-canvas.top;
    std::memset(surface.pixels, 0, surface.bytes());
    raster_art::draw_sprite(surface.dc, name.c_str(), cx, feet, size.height);
    if (held != vector_art::Held::None)
      draw_raster_equipment(surface.dc, held, {cx, feet, 1}, size.height, name);
    auto result=upload(key,surface);
    if(result.id) {
      const bool lineage=name.rfind("hero_male_",0)==0||name.rfind("hero_female_",0)==0;
      result.native_height=size.height;result.anchor_x=float(cx)/surface.width;
      result.anchor_y=float(feet-(lineage?32:0))/surface.height;
      textures[key]=result;
    }
    return result;
  }

  TerrainBakeStats bake_stats;
  std::string bake_error;
  std::string terrain_scene_key, job_key, job_scene_key, failed_job_key;
  std::future<TerrainBakeResult> terrain_job;
  std::vector<std::uint8_t> terrain_rgba;

  ~Renderer() {
    // std::async owns exactly one worker. It captures no this/state pointers;
    // join before any renderer/asset lifetimes can end.
    if(terrain_job.valid()) terrain_job.wait();
  }

  void update_bake_bytes() {
    bake_stats.running=terrain_job.valid();
    bake_stats.retained_bytes=terrain_rgba.capacity()+terrain_vertices.capacity()*sizeof(fable_gpu::Vertex)+
        terrain_indices.capacity()*sizeof(std::uint32_t);
    const auto estimated=bake_stats.retained_bytes+(terrain_job.valid()?kTerrainJobBytes:0);
    bake_stats.peak_bytes=std::max(bake_stats.peak_bytes,estimated);
  }

  bool adopt_terrain(TerrainBakeResult&& result) {
    const auto started=std::chrono::steady_clock::now();
    const auto revision=terrain_revision+1;
    if(!gpu.upload_texture(1,kTerrainWidth,kTerrainHeight,result.rgba.data(),kTerrainWidth*4,false,revision)) {
      bake_error=gpu.error();++bake_stats.failed;return false;
    }
    // Publish bounds, geometry and matching texture together on the main
    // thread. Retain one RGBA packet for GPU eviction/reset recovery.
    terrain_revision=revision;terrain_key=std::move(result.key);terrain_scene_key=std::move(result.scene_key);
    min_x=result.min_x;min_y=result.min_y;max_x=result.max_x;max_y=result.max_y;
    terrain_vertices=std::move(result.vertices);terrain_indices=std::move(result.indices);terrain_rgba=std::move(result.rgba);
    ++bake_stats.adopted;bake_stats.last_upload_ms=
        std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
    bake_error.clear();update_bake_bytes();return true;
  }

  bool finish_terrain_job(const std::string& scene_key,const std::string& desired_key,bool wait) {
    if(!terrain_job.valid()) return true;
    if(terrain_job.wait_for(std::chrono::milliseconds(0))!=std::future_status::ready) {
      if(!wait) return true;
      ++bake_stats.loading_waits;terrain_job.wait();
    }
    try {
      auto result=terrain_job.get();++bake_stats.completed;bake_stats.last_cpu_ms=result.cpu_ms;
      // Never publish an old route, source generation or height-field extent.
      // A camera that returned to the already resident bucket needs no swap.
      if(result.scene_key!=scene_key || result.generation!=generation ||
          (terrain_scene_key==scene_key && terrain_key==desired_key && result.key!=desired_key)) {
        ++bake_stats.discarded;update_bake_bytes();return true;
      }
      if(!adopt_terrain(std::move(result))) { failed_job_key=job_key;update_bake_bytes();return false; }
    } catch(const std::exception& error) {
      bake_error=std::string("terrain CPU bake: ")+error.what();failed_job_key=job_key;++bake_stats.failed;
      update_bake_bytes();return false;
    }
    update_bake_bytes();return true;
  }

  bool prepare(ClientState& state) {
    if(!gpu.initialize()) return false;
    const auto source_generation=raster_art::asset_generation();
    if(generation!=source_generation) {
      textures.clear();generation=source_generation;failed_job_key.clear();
    }
    const auto& world=state.world;
    const int cx=static_cast<int>(std::floor(state.camera.x/(kTileUnits*20)))*20;
    const int cy=static_cast<int>(std::floor(state.camera.y/(kTileUnits*16)))*16;
    const std::string key=world.route_id+"|"+world.theme+"|"+std::to_string(cx)+":"+std::to_string(cy);
    const std::string scene_key=world.route_id+"|"+world.theme+"|"+std::to_string(generation)+"|"+
        std::to_string(world.map_width)+":"+std::to_string(world.map_height);
    const bool interior=world.theme=="crypt"||world.theme=="dungeon";
    state.camera.elevation=fable::make_height_field(interior?fable::SceneElevation::Interior:
        fable::SceneElevation::Outdoor,-.5*kTileUnits,-.5*kTileUnits,
        world.map_width>0?world.map_width*kTileUnits:1800,
        world.map_height>0?world.map_height*kTileUnits:1800,scenery_seed(world.route_id),28);
    // New scene/source lifetime may wait. Ordinary same-scene travel only
    // polls; its existing80x64 tile patch stays visible throughout the bake.
    const bool loading=terrain_scene_key!=scene_key||terrain_rgba.empty();
    if(!finish_terrain_job(scene_key,key,loading)) return false;
    if(terrain_scene_key==scene_key && !terrain_rgba.empty() && !gpu.has_texture(1,terrain_revision)) {
      const auto started=std::chrono::steady_clock::now();
      if(!gpu.upload_texture(1,kTerrainWidth,kTerrainHeight,terrain_rgba.data(),kTerrainWidth*4,false,terrain_revision)) return false;
      bake_stats.last_upload_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count();
    }
    if(terrain_scene_key==scene_key && terrain_key==key) { update_bake_bytes();return true; }
    if(terrain_job.valid()) { ++bake_stats.reused_frames;update_bake_bytes();return true; }
    if(failed_job_key==scene_key+"|"+key) return terrain_scene_key==scene_key&&!terrain_rgba.empty();
    const char* ground=quiet_ground_asset(world.theme);
    if(!ground) ground=interior?"terrain_stone":"terrain_quiet_earth";
    if(!raster_ground::detail::patterns(ground)) return false;
    TerrainBakeInput input;
    input.key=key;input.scene_key=scene_key;input.generation=generation;input.interior=interior;
    input.min_x=(cx-32)*kTileUnits;input.min_y=(cy-40)*kTileUnits;
    input.max_x=input.min_x+kTerrainTilesX*kTileUnits;input.max_y=input.min_y+kTerrainTilesY*kTileUnits;
    // Snapshot these main-thread caches before launching. The worker uses
    // only pure coverage/material/height functions and its owned vectors.
    const auto& patterns=raster_ground::detail::cache().patterns;
    input.earth=patterns.earth;input.moss=patterns.moss;
    input.layout=ground_layout(world.route_id,world.theme,state.scenery);input.elevation=state.camera.elevation;
    job_key=scene_key+"|"+key;job_scene_key=scene_key;
    try {
      terrain_job=std::async(std::launch::async,[input=std::move(input)](){return bake_terrain(input);});
      ++bake_stats.started;update_bake_bytes();
    } catch(const std::exception& error) {
      bake_error=std::string("terrain worker launch: ")+error.what();failed_job_key=job_key;++bake_stats.failed;
      update_bake_bytes();return terrain_scene_key==scene_key&&!terrain_rgba.empty();
    }
    if(terrain_scene_key!=scene_key||terrain_rgba.empty()) return finish_terrain_job(scene_key,key,true);
    ++bake_stats.reused_frames;return true;
  }
  std::uint64_t terrain_revision = 0;
  bool shadow_ready = false;
  ULONGLONG clock_start = GetTickCount64(), hitstop_until = 0;
  std::uint64_t impact_key = 0;
  std::string feedback_scene;
  std::vector<fable_gpu::Sprite> contact_sprites;
};
inline Renderer& renderer() { static Renderer r; return r; }

inline std::string actor_pose(const char* family,double x,double y,double attack,double moving,double walk) {
  const std::string dir=actor_raster_direction(family,x,y);
  const int strikes=raster_strike_frames(family,dir), walks=raster_walk_frames(family,dir);
  std::string pose;
  if(strikes && attack>=0 && attack<1) pose="_strike"+std::to_string(std::clamp(int(attack*strikes),0,strikes-1));
  else if(attack>.18 && attack<.82 && (std::strcmp(family,"hero")==0 || std::strcmp(family,"raider")==0)) pose="_attack";
  else if(moving>.2 && walks && (attack<0 || attack>=1)) pose="_walk"+std::to_string(std::clamp(int((walk-std::floor(walk))*walks),0,walks-1));
  const std::string name=std::string(family)+pose+"_"+dir;
  return raster_art::available(name.c_str()) ? name : std::string(family)+"_"+dir;
}

inline bool paint(ClientState& state,HDC dc,const RECT& bounds,render::List& trace,
                  std::span<std::uint8_t> frame_pixels = {}) {
  auto& r=renderer();
  if(!r.prepare(state)) return false;
  const auto now=GetTickCount64();
  const double time=(now-r.clock_start)/1000.0;
  if(r.feedback_scene!=state.world.route_id) {
    r.feedback_scene=state.world.route_id;r.hitstop_until=0;r.contact_sprites.clear();r.impact_key=0;
  }
  state.camera.shake_x=state.camera.shake_y=0;
  const double focus_scale=fable_projection(state.camera,bounds).zoom;
  for(const auto& fx:state.effects) if(fx.kind==EffectFx::Kind::Impact && fx.ttl>0) {
    const double remaining=std::clamp(1.0-(fx.age+state.tick_accum_ms/50)/fx.ttl,0.0,1.0);
    const double shake=4.0*remaining*remaining/std::max(.05,focus_scale);
    state.camera.shake_x+=std::sin(time*73)*shake;
    state.camera.shake_y+=std::cos(time*61)*shake*.65;
  }
  const auto p=fable_projection(state.camera,bounds);
  if(!p.valid) return false;
  std::vector<fable_gpu::Sprite> sprites;
  std::vector<fable_gpu::Light> lights;
  std::vector<std::array<float,3>> shadows;
  const auto height=[&](double x,double y){return static_cast<float>(fable::sample_height(state.camera.elevation,x,y));};
  const auto sprite=[&](const std::string& name,double x,double y,double h,
      vector_art::Held held=vector_art::Held::None)->fable_gpu::Sprite* {
    const auto at=fable::project_ground(p,state.camera.elevation,x,y);
    if(!at.visible || at.x < -600 || at.x > bounds.right+600 || at.y < -100 || at.y-h*at.scale>bounds.bottom+100) return nullptr;
    const auto tex=r.texture(name,held); if(!tex.id) return nullptr;
    const double native=tex.native_height>0?tex.native_height:tex.height;
    sprites.push_back({tex.id,float(x),float(y),height(x,y),float(h*tex.width/native),float(h*tex.height/native),tex.anchor_x,tex.anchor_y});
    return &sprites.back();
  };
  const auto authored_sprite=[&](const first_slice_art::Clip& clip,double phase,double x,double y)->fable_gpu::Sprite* {
    const auto& name=clip.frame(phase);
    const auto size=raster_art::dimensions(name.c_str());
    if(size.width!=clip.width||size.height!=clip.height)return nullptr;
    // One world metre is one base tile. Frame padding, body pose and object
    // size never influence this scale or the artist-authored ground pivot.
    auto* result=sprite(name,x,y,clip.height*kTileUnits/clip.pixels_per_metre);
    if(result) {
      result->anchor_x=float(clip.anchor_x)/clip.width;
      result->anchor_y=float(clip.anchor_y)/clip.height;
      result->crisp=true;
      result->ground_layer=clip.identity=="worn-earth-patch";
      const auto ink=raster_art::content_bounds(name.c_str());
      const double below=std::max(0L,ink.bottom-clip.anchor_y)*kTileUnits/clip.pixels_per_metre;
      const double ground_projection=p.a-result->elevation*p.k;
      if(below>0&&ground_projection>0) {
        const double dz=p.d0-y;
        const double front_dz=dz*ground_projection/(ground_projection+below*p.k);
        result->depth_bias=float(std::max(0.0,std::min(dz-p.near_depth,dz-front_dz+kTileUnits*.02)));
      }
      trace.push_back({render::Op::Hud,0,0,0,0,"art:"+name});
    }
    return result;
  };
  // A stair marker at the same world Y is part of the floor layer, so its
  // transparent billboard must sort before the player standing on it.
  if(state.world.has_extraction)
    if(auto* s=sprite("exit_stairs",state.world.extraction.x,state.world.extraction.y,66)) s->ground_layer=true;
  // Billboard feet, terrain vertices and light anchors share this sampler.
  for(const auto& item:state.scenery) {
    if(!item.art_identity.empty()) {
      if(const auto* clip=first_slice_art::registry().find(item.art_identity,"idle","front"))
        authored_sprite(*clip,0,item.position.x,item.position.y);
      continue;
    }
    const char* name=item.kind==SceneryKind::Tree?"tree":item.kind==SceneryKind::Ruin?"column":
        item.kind==SceneryKind::Shrine?"shrine":item.kind==SceneryKind::Gate?"gate":
        raster_scenery::dwelling_asset(state.world.route_id,item.position.x,item.position.y,kTileUnits);
    if(const auto* clip=first_slice_art::registry().find(name,"idle","front")) {
      authored_sprite(*clip,0,item.position.x,item.position.y);
      shadows.push_back({float(item.position.x),float(item.position.y),float(item.radius*.8)});
      continue;
    }
    const auto size=raster_art::dimensions(name); const auto ink=raster_art::content_bounds(name);
    const double visible_height=scenery_height(item.kind)*item.scale*.78;
    shadows.push_back({float(item.position.x),float(item.position.y),float(item.radius*.8)});
    if(size.valid() && ink.bottom>ink.top)
      if(auto* s=sprite(name,item.position.x,item.position.y,visible_height*size.height/(ink.bottom-ink.top)))
        s->anchor_y=float(ink.bottom)/size.height;
    if(item.kind==SceneryKind::Shrine || item.kind==SceneryKind::Gate)
      lights.push_back({float(item.position.x),float(item.position.y),height(item.position.x,item.position.y)+30,210,1,.63f,.28f,.4f});
  }
  const auto actor=[&](const WorldActor& a,const char* family,const char* motion_key,bool player) {
    const auto held=player?equipped_held(state):vector_art::Held::None;
    const char* equipment=held==vector_art::Held::None?"unarmed":held==vector_art::Held::Club?"club":
        held==vector_art::Held::Handstone?"handstone":held==vector_art::Held::Axe?"axe":
        held==vector_art::Held::Staff?"staff":held==vector_art::Held::Bow?"bow":"sword";
    const auto identity=player?std::string("player_")+a.appearance+"_"+equipment:a.kind;
    const auto& art=first_slice_art::registry();
    const bool active=art.ready(identity)&&(!player||
        (art.ready(std::string("player_")+a.appearance+"_unarmed")&&art.ready(std::string("player_")+a.appearance+"_club")));
    if(!a.alive) {
      if(active) {
        const auto* clip=art.find(identity,"death",first_slice_art::direction(a.facing.x,a.facing.y));
        const auto& pos=a.displayed_position();
        authored_sprite(*clip,std::min(.999999,state.motions[motion_key].death_age_ms*clip->fps/1000/clip->frames.size()),pos.x,pos.y);
      }
      return;
    }
    const auto& pos=a.displayed_position(); const auto& motion=state.motions[motion_key];
    shadows.push_back({float(pos.x),float(pos.y),player?27.f:30.f});
    double ax=a.facing.x,ay=a.facing.y,attack=-1;
    if((!player||state.lineage_art) && motion.moving>.2){ax=motion.travel_direction.x;ay=motion.travel_direction.y;}
    if(const auto* strike=verdigris::client::actor_strike(state.effects,a.id)) {
      attack=verdigris::client::strike_phase(*strike,state.tick_accum_ms/50.0);ax=std::cos(strike->angle);ay=std::sin(strike->angle);
    } else if(const auto t=state.telegraphs.find(a.id);t!=state.telegraphs.end()) {
      ax=t->second.facing.x;ay=t->second.facing.y;
      attack=std::clamp(double(state.world.tick-t->second.start_tick)/std::max(1,t->second.windup_ticks)*.4,0.0,.4);
    }
    if(active) {
      const auto* dir=first_slice_art::direction(ax,ay);
      const EffectFx* hit=nullptr;
      for(const auto& fx:state.effects)if(fx.kind==EffectFx::Kind::TargetFlash&&fx.actor_id==a.id&&fx.ttl>0)hit=&fx;
      const auto action=hit?"hit":attack>=0&&attack<1?"attack":motion.moving>.2?(player&&motion.tiles_per_second>5?"sprint":"walk"):"idle";
      const auto* clip=art.find(identity,action,dir);
      if(!clip) {
        // Missing coverage is explicit in the trace, and retains this same
        // character's idle rather than stitching an older sheet into it.
        trace.push_back({render::Op::Hud,0,0,0,0,"art-missing:"+identity+"/"+action+"/"+dir});
        clip=art.find(identity,"idle",dir);
      }
      const double phase=clip->action=="hit"?std::min(.999999,(hit->age+state.tick_accum_ms/50.0)/hit->ttl):clip->action=="attack"?attack:clip->action=="idle"?time*clip->fps/clip->frames.size():motion.walk_phase;
      if(auto* sp=authored_sprite(*clip,phase,pos.x,pos.y)) {
        for(const auto& fx:state.effects) if(fx.kind==EffectFx::Kind::TargetFlash && fx.actor_id==a.id && fx.ttl>0)
          sp->flash=std::max(sp->flash,float(std::clamp(1.0-(fx.age+state.tick_accum_ms/50)/fx.ttl,0.0,1.0)*.85));
        trace.push_back({player?render::Op::Player:render::Op::Monster,double(pos.x),double(pos.y),0,0,clip->frame(phase)});
      }
      return;
    }
    const auto name=actor_pose(family,ax,ay,attack,motion.moving,motion.walk_phase);
    // Fixed canvas scale across all frames: the hero has65 ink rows inside
    // its96-row canvas, the raider57. Never resize each pose to its own ink.
    if(auto* s=sprite(name,pos.x,pos.y,player?(state.lineage_art?184:140):a.elite?184:160,player?equipped_held(state):vector_art::Held::None)) {
      for(const auto& fx:state.effects) if(fx.kind==EffectFx::Kind::TargetFlash && fx.actor_id==a.id && fx.ttl>0)
        s->flash=std::max(s->flash,float(std::clamp(1.0-(fx.age+state.tick_accum_ms/50)/fx.ttl,0.0,1.0)*.85));
      trace.push_back({player?render::Op::Player:render::Op::Monster,double(pos.x),double(pos.y),0,0,name});
    }
  };
  actor(state.world.player,player_raster_family(state),"player",true);
  for(const auto& a:state.world.monsters) actor(a,verdigris::client::monster_art_family(a,state.world),a.id.c_str(),false);
  for(const auto& npc:state.world.npcs) {
    if(!npc.art_identity.empty()) {
      if(const auto* clip=first_slice_art::registry().find(npc.art_identity,"idle","front"))
        authored_sprite(*clip,time*clip->fps/clip->frames.size(),npc.position.x,npc.position.y);
    } else sprite("artisan_sw",npc.position.x,npc.position.y,144);
  }
  for(const auto& [id,pos]:state.loot_positions) {
    const auto n=state.world.loot_names.find(id);
    sprite(raster_loot::sprite(id,n==state.world.loot_names.end()?"":n->second),pos.x,pos.y,25);
  }
  for(const auto& fx:state.effects) {
    if(fx.ttl<=0 || fx.age>=fx.ttl) continue;
    const float life=float(std::clamp(1.0-(fx.age+state.tick_accum_ms/50)/fx.ttl,0.0,1.0));
    fable_gpu::Sprite* s=nullptr;
    if(fx.kind==EffectFx::Kind::Impact) {
      s=sprite("effect_hit_spark",fx.wx,fx.wy,fx.critical?70:46);
      if(s){s->elevation+=35;s->anchor_y=.5f;s->additive=true;}
      lights.push_back({float(fx.wx),float(fx.wy),height(fx.wx,fx.wy)+30,140,1,.74f,.34f,life*.35f});
    } else if(fx.kind==EffectFx::Kind::Dust || fx.kind==EffectFx::Kind::DeathRing) {
      s=sprite("effect_dust"+std::to_string(std::min(3,int((1-life)*4))),fx.wx,fx.wy,52+(1-life)*20);
      if(s) s->anchor_y=.65f;
    } else if(fx.kind==EffectFx::Kind::Swing || fx.kind==EffectFx::Kind::SweepArc) {
      s=sprite("effect_slash0",fx.wx+std::cos(fx.angle)*42,fx.wy+std::sin(fx.angle)*42,fx.kind==EffectFx::Kind::SweepArc?110:70);
      if(s){s->anchor_y=.5f;s->elevation+=38;s->rotation=float(fx.angle);s->additive=true;}
    } else if(fx.kind==EffectFx::Kind::ActorFall && fx.actor_family=="raider" &&
        std::string(raster_direction(std::cos(fx.angle),std::sin(fx.angle)))=="sw") {
      s=sprite("raider_death"+std::to_string(std::min(3,int(verdigris::client::actor_fall_phase(fx)*4)))+"_sw",fx.wx,fx.wy,(fx.actor_elite?184:160)*112.0/96);
      if(s){s->anchor_y=96.f/112;s->opacity=float(verdigris::client::actor_fall_opacity(fx));s->ground_layer=fx.age>=verdigris::client::kActorFallMotionTicks;}
      continue;
    }
    if(s) s->opacity=life;
  }
  // The small shared atlas stays sharp at every perspective depth. No bloom
  // or extra fullscreen pass; particles use the existing billboard pipeline.
  const auto& vfx=state.particles;
  constexpr std::uint64_t particle_texture=0x56465841544c4153ull;
  if(vfx.assets.ready() && !vfx.particles.empty()) {
    if(!r.gpu.has_texture(particle_texture,1))
      r.gpu.upload_texture(particle_texture,vfx.assets.width,vfx.assets.height,vfx.assets.rgba.data(),vfx.assets.width*4,false,1);
    if(r.gpu.has_texture(particle_texture,1)) {
      int glows=0;
      for(const auto& part:vfx.particles) {
        const auto& layer=*part.layer;
        const float interpolation=float(std::clamp(state.tick_accum_ms/50.0,0.0,1.0));
        auto position=part.previous+(part.pos-part.previous)*interpolation;
        if(layer.local) {
          const auto anchor=particle_view::resolve(state,part.attachment);if(!anchor)continue;position=position+*anchor;
        }
        const float t=std::clamp((part.age+interpolation*.05f)/part.life,0.f,1.f);
        if(t>=1)continue;
        const auto mix=[&](float a,float b){return a+(b-a)*t;};
        const float width=mix(layer.size_start[0],layer.size_end[0]),h=mix(layer.size_start[1],layer.size_end[1]);
        if(width<=0 || h<=0)continue;
        const auto frame=std::min(layer.frames.size()-1,static_cast<std::size_t>(part.age*layer.fps));
        const auto uv=vfx.assets.regions.at(layer.frames[frame]);
        fable_gpu::Sprite sp;sp.texture=particle_texture;sp.x=position.x;sp.y=position.y;
        sp.elevation=height(position.x,position.y)+position.z;sp.width=width;sp.height=h;sp.anchor_y=.5f;
        sp.tint_r=mix(layer.color_start[0],layer.color_end[0]);sp.tint_g=mix(layer.color_start[1],layer.color_end[1]);
        sp.tint_b=mix(layer.color_start[2],layer.color_end[2]);sp.opacity=mix(layer.color_start[3],layer.color_end[3]);
        sp.additive=layer.additive;sp.crisp=true;sp.rotation=part.rotation;
        sp.uv={float(uv.x)/vfx.assets.width,float(uv.y)/vfx.assets.height,float(uv.x+uv.w)/vfx.assets.width,float(uv.y+uv.h)/vfx.assets.height};
        sprites.push_back(sp);
        if(layer.glow && glows<2 && lights.size()<24) {++glows;lights.push_back({position.x,position.y,sp.elevation,95,sp.tint_r,sp.tint_g,sp.tint_b,sp.opacity*.3f});}
      }
      trace.push_back({render::Op::Hud,0,0,0,int(vfx.particles.size()),"vfx:atlas-billboards"});
    }
  }
  std::vector<fable_gpu::Vertex> wall_vertices,cut_vertices;
  std::vector<std::uint32_t> wall_indices,cut_indices;
  const auto quad=[](auto& vertices,auto& indices,fable_gpu::Vertex a,fable_gpu::Vertex b,fable_gpu::Vertex c,fable_gpu::Vertex d) {
    const auto n=static_cast<std::uint32_t>(vertices.size());vertices.insert(vertices.end(),{a,b,c,d});
    indices.insert(indices.end(),{n,n+1,n+2,n,n+2,n+3});
  };
  const raster_walls::Grid grid{state.world.map_width,state.world.map_height,state.world.map_walkable};
  if(grid.valid()) {
    const int x0=std::max(0,int(std::floor((state.camera.x-2200)/kTileUnits))),x1=std::min(grid.width-1,int((state.camera.x+2200)/kTileUnits));
    const int y0=std::max(0,int(std::floor((state.camera.y-2500)/kTileUnits))),y1=std::min(grid.height-1,int((state.camera.y+1100)/kTileUnits));
    if(state.world.theme=="dungeon" || state.world.theme=="crypt") {
      for(int y=y0;y<=y1;++y) for(int x=x0;x<=x1;++x) {
        if(!grid.open(x,y) || grid.open(x,y-1)) continue;
        const auto hash=raster_ground::hash(x,y);
        const double wx=x*kTileUnits,wy=(y-.30)*kTileUnits;
        if(hash%7==0 && grid.open(x,y+1)) {
          if(sprite("brazier",wx,wy,78))
            lights.push_back({float(wx),float(wy),height(wx,wy)+42,235,1,.61f,.27f,float(.68+.04*std::sin(time*11+x))});
        } else if(hash%5==0) sprite("pottery",wx,wy,45);
      }
    }
    const auto pp=project(state.camera,bounds,state.world.player.displayed_position().x,state.world.player.displayed_position().y);
    const RECT player_ink{pp.x-int(42*pp.scale),pp.y-int(96*pp.scale),pp.x+int(42*pp.scale),pp.y};
    for(int y=y0;y<=y1;++y) for(int x=x0;x<=x1;++x) if(!grid.open(x,y)) {
      const float l=float((x-.5)*kTileUnits),rr=float((x+.5)*kTileUnits),t=float((y-.5)*kTileUnits),b=float((y+.5)*kTileUnits);
      const auto a=project(state.camera,bounds,l,t),c=project(state.camera,bounds,rr,b);
      if(a.scale==0 || c.scale==0) continue;
      const RECT box{std::min(a.x,c.x),std::min(a.y,c.y)-int(75*c.scale),std::max(a.x,c.x),std::max(a.y,c.y)};
      if(box.right<0 || box.left>bounds.right || box.bottom<0 || box.top>bounds.bottom) continue;
      const bool cut=y*kTileUnits>state.world.player.displayed_position().y && raster_walls::overlaps(box,player_ink);
      auto& v=cut?cut_vertices:wall_vertices; auto& ix=cut?cut_indices:wall_indices;
      const float h=height(x*kTileUnits,y*kTileUnits),top=h+60;
      quad(v,ix,{l,t,top,0,0},{rr,t,top,1,0},{rr,b,top,1,2.f/3},{l,b,top,0,2.f/3});
      if(grid.open(x,y+1)) quad(v,ix,{l,b,top,0,2.f/3},{rr,b,top,1,2.f/3},{rr,b,h,1,1},{l,b,h,0,1});
      if(grid.open(x-1,y)) quad(v,ix,{l,t,top,0,2.f/3},{l,b,top,1,2.f/3},{l,b,h,1,1},{l,t,h,0,1});
      if(grid.open(x+1,y)) quad(v,ix,{rr,b,top,0,2.f/3},{rr,t,top,1,2.f/3},{rr,t,h,1,1},{rr,b,h,0,1});
    }
  }
  const auto wall=r.texture("wall_stone_cutaway");
  std::vector<fable_gpu::Mesh> meshes;
  if(wall.id && !wall_indices.empty()) meshes.push_back({wall.id,wall_vertices,wall_indices,true,1});
  if(wall.id && !cut_indices.empty()) meshes.push_back({wall.id,cut_vertices,cut_indices,true,.28f});
  if(!r.shadow_ready || !r.gpu.has_texture(2,1)) {
    std::array<std::uint8_t,64*64*4> pixels{};
    for(int y=0;y<64;++y) for(int x=0;x<64;++x) {
      const double dx=(x+.5-32)/32,dy=(y+.5-32)/32;
      const double weight=std::clamp(1-dx*dx-dy*dy,0.0,1.0);
      const int at=(y*64+x)*4;pixels[at]=21;pixels[at+1]=22;pixels[at+2]=19;
      pixels[at+3]=static_cast<std::uint8_t>(weight*weight*145);
    }
    r.shadow_ready=r.gpu.upload_texture(2,64,64,pixels.data(),64*4,false,1);
  }
  std::vector<fable_gpu::Vertex> shadow_vertices;
  std::vector<std::uint32_t> shadow_indices;
  for(const auto& sh:shadows) {
    const float l=sh[0]-sh[2],rr=sh[0]+sh[2],t=sh[1]-sh[2]*.8f,b=sh[1]+sh[2]*.8f;
    quad(shadow_vertices,shadow_indices,{l,t,height(l,t)+.8f,0,0},{rr,t,height(rr,t)+.8f,1,0},
        {rr,b,height(rr,b)+.8f,1,1},{l,b,height(l,b)+.8f,0,1});
  }
  const std::array<fable_gpu::Mesh,1> ground_meshes{{{2,shadow_vertices,shadow_indices,true,.8f}}};
  fable_gpu::Scene scene;
  if(state.gear_overlay) {
    // inventory_surface paints an opaque gradient over this entire rectangle.
    // Keep rendering the visible world and all animation at full resolution.
    const auto pane=gear_pane_rect(bounds.right,bounds.bottom);
    scene.opaque_rect={float(pane.x),float(pane.y),float(pane.x+pane.w),float(pane.y+pane.h)};
  }
  scene.camera={int(p.width),int(p.height),float(p.cam_x),float(p.cam_y),float(p.d0),float(p.k),float(p.a),float(p.horizon),float(p.dzp),float(p.near_depth),float(p.far_depth)};
  // Explicit nearest mesh chooses the point-sampled shader path, which does
  // not require a generated mip chain or blend ground pixels with neighbours.
  scene.terrain={1,r.terrain_vertices,r.terrain_indices,true};scene.sprites=sprites;scene.lights=lights;scene.world_meshes=meshes;
  if(r.shadow_ready) scene.ground_meshes=ground_meshes;
  const bool interior=state.world.theme=="crypt" || state.world.theme=="dungeon";
  // Fable's exact90-second ambient keyframes, starting in its daylight phase.
  constexpr double keys[]{0,.30,.45,.58,.80,.90,1};
  constexpr float colors[][3]={{255,244,224},{255,240,214},{255,205,150},{150,140,205},{110,120,190},{210,180,175},{255,244,224}};
  const double day=std::fmod(time+22,90)/90;
  float ambient[3]{1,1,1};
  for(int n=0;n<6;++n) if(day>=keys[n] && day<=keys[n+1])
    for(int c=0;c<3;++c) ambient[c]=float((colors[n][c]+(colors[n+1][c]-colors[n][c])*(day-keys[n])/(keys[n+1]-keys[n]))/255);
  scene.ambient_r=interior?.74f:ambient[0];scene.ambient_g=interior?.72f:ambient[1];scene.ambient_b=interior?.78f:ambient[2];
  const float night=1-std::max({ambient[0],ambient[1],ambient[2]})+
      (1-std::min({ambient[0],ambient[1],ambient[2]}))*.5f;
  const auto player_pos=state.world.player.displayed_position();
  lights.push_back({float(player_pos.x),float(player_pos.y),height(player_pos.x,player_pos.y)+30,
      220,1,205.f/255,140.f/255,(.25f+night*.65f)*.55f});
  scene.lights=lights;
  std::array<fable_gpu::Cloud,4> clouds{};
  if(!interior) {
    for(int n=0;n<4;++n) {
      const auto seed=scenery_seed(state.world.route_id+":cloud:"+std::to_string(n));
      const double radius=(700+(seed%451))*bounds.right/2600.0;
      const double span=bounds.right*.25+radius*.5;
      const double speed=1.5+(seed%171)/100.0;
      const double sx=(std::fmod((seed%1800)*.6+time*speed,span)-radius*.25)*4;
      const double sy=((seed>>12)%1000)/1000.0*bounds.bottom+
          std::sin(time*.05+n*1.7)*bounds.bottom*.06;
      clouds[n]={float(sx),float(sy),float(radius)};
    }
    scene.clouds=clouds;
  }
  scene.sky_r=interior?.18f:.40f;scene.sky_g=interior?.18f:.43f;scene.sky_b=interior?.21f:.43f;
  scene.dof_strength=.55f;scene.vignette=.16f;
  // A confirmed contact holds its visible pose for the reference55ms. Network
  // polling, input and authority keep advancing; this is only a sprite packet.
  std::string contacts;
  for(const auto& fx:state.effects) if(fx.kind==EffectFx::Kind::Impact && fx.age==0)
    contacts+=":"+std::to_string(fx.wx)+":"+std::to_string(fx.wy);
  if(!contacts.empty()) {
    const auto key=scenery_seed(state.world.route_id+":"+std::to_string(state.world.tick)+contacts);
    if(key!=r.impact_key){r.impact_key=key;r.hitstop_until=now+55;r.contact_sprites=sprites;}
  }
  if(now<r.hitstop_until && !r.contact_sprites.empty()) scene.sprites=r.contact_sprites;
  if(!r.gpu.render(scene,dc,frame_pixels)) return false;
  trace.push_back({render::Op::Hud,0,0,0,0,"fable:hardware:"+r.gpu.adapter_name()});
  for(const auto& fx:state.effects) if(fx.kind==EffectFx::Kind::DamageNumber) draw_effect(dc,state.camera,bounds,fx,trace);
  paint_telegraphs(state,dc,bounds,trace);
  for(const auto& a:state.world.monsters) {
    if(!a.alive) continue;
    const auto pos=a.displayed_position(); const auto at=project(state.camera,bounds,pos.x,pos.y);
    if(at.scale<=0 || at.x<0 || at.x>bounds.right || at.y<0 || at.y>bounds.bottom) continue;
    double label_lift=a.elite?121:106;
    const auto& art=first_slice_art::registry();
    if(art.ready(a.kind))if(const auto* clip=art.find(a.kind,"idle","front"))
      label_lift=clip->anchor_y*kTileUnits/clip->pixels_per_metre+8;
    const int width=std::max(28,int(54*at.scale)),top=at.y-int(label_lift*at.scale);
    const RECT back{at.x-width/2-1,top-1,at.x+width/2+1,top+5};
    FillRect(dc,&back,cached_brush(RGB(24,24,23)));
    const int fill=static_cast<int>(width*std::clamp(double(a.life)/std::max(1,a.life_max),0.0,1.0));
    const RECT bar{at.x-width/2,top,at.x-width/2+fill,top+4};
    FillRect(dc,&bar,cached_brush(a.elite?RGB(210,151,73):RGB(169,74,51)));
  }
  if(state.world.has_extraction) {
    const auto at=project(state.camera,bounds,state.world.extraction.x,state.world.extraction.y);
    if(at.scale>0 && at.x>=0 && at.x<bounds.right && at.y>=0 && at.y<bounds.bottom) {
      const bool is_near=std::max(std::abs(state.world.player.position.x-state.world.extraction.x),
          std::abs(state.world.player.position.y-state.world.extraction.y))<=kTileUnits;
      const char* label=is_near?"F  Return to House":"Return to House";
      SIZE size{};skin::text_extent(dc,label,int(std::strlen(label)),&size);
      skin::hud_text_backing(dc,{at.x-size.cx/2-5,at.y+8,at.x+size.cx/2+5,at.y+size.cy+14});
      SetTextColor(dc,skin::kInk);SetBkMode(dc,TRANSPARENT);
      skin::text_out(dc,at.x-size.cx/2,at.y+11,label,int(std::strlen(label)));
    }
  }
  for(const auto& npc:state.world.npcs) {
    const auto at=project(state.camera,bounds,npc.position.x,npc.position.y);
    if(at.scale<=0 || at.x<0 || at.x>bounds.right || at.y<0 || at.y>bounds.bottom) continue;
    SetBkMode(dc,TRANSPARENT);SetTextColor(dc,skin::kInk);
    SIZE size{};skin::text_extent(dc,npc.name.c_str(),int(npc.name.size()),&size);
    double label_lift=106;
    if(!npc.art_identity.empty())if(const auto* clip=first_slice_art::registry().find(npc.art_identity,"idle","front"))
      label_lift=clip->anchor_y*kTileUnits/clip->pixels_per_metre+size.cy/std::max(.1,at.scale);
    const int y=at.y-int(label_lift*at.scale);
    SetTextColor(dc,RGB(15,12,9));
    skin::text_out(dc,at.x-size.cx/2+skin::ui_scale(),y+skin::ui_scale(),npc.name.c_str(),int(npc.name.size()));
    SetTextColor(dc,skin::kInk);
    skin::text_out(dc,at.x-size.cx/2,y,npc.name.c_str(),int(npc.name.size()));
  }
  if(state.loot_labels) for(const auto& [id,pos]:state.loot_positions) {
    const auto it=state.world.loot_names.find(id);if(it==state.world.loot_names.end()) continue;
    const auto at=project(state.camera,bounds,pos.x,pos.y);
    SetTextColor(dc,RGB(248,229,172));skin::text_out(dc,at.x,at.y+5,it->second.c_str(),int(it->second.size()));
  }
  return true;
}
} // namespace fable_world
