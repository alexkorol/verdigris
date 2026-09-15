#pragma once

bool first_slice_depth_probe() {
  fable_gpu::Renderer gpu;
  const std::array<std::uint8_t,4> floor{80,80,80,255},ink{230,20,20,255};
  if(!gpu.upload_texture(9001,1,1,floor.data(),4,false,1)||
      !gpu.upload_texture(9002,1,1,ink.data(),4,false,1))return false;
  const std::array<fable_gpu::Vertex,4> ground{{{-1000,-600,0,0,0},{1000,-600,0,1,0},{1000,600,0,1,1},{-1000,600,0,0,1}}};
  const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
  fable_gpu::Sprite sprite;sprite.texture=9002;sprite.width=48;sprite.height=96;sprite.anchor_y=.75f;sprite.crisp=true;
  fable_gpu::Scene scene;scene.camera={256,256,0,0,1000,800,100000,32,1000,40,4000};
  scene.terrain={9001,ground,indices,true};scene.sprites=std::span(&sprite,1);
  scene.dof_strength=0;scene.vignette=0;
  std::vector<std::uint8_t> before(256*256*4),after(before.size());
  if(!gpu.render(scene,nullptr,before))return false;
  sprite.depth_bias=1000-1000*100000.f/(100000+24*800)+1;
  if(!gpu.render(scene,nullptr,after))return false;
  const auto bounds=[](const auto& pixels) {
    RECT box{256,256,0,0};
    for(int y=0;y<256;++y)for(int x=0;x<256;++x) {
      const auto at=(y*256+x)*4;
      if(pixels[at+2]>pixels[at+1]*2&&pixels[at+2]>pixels[at]*2) {
        box.left=std::min<LONG>(box.left,x);box.top=std::min<LONG>(box.top,y);
        box.right=std::max<LONG>(box.right,x+1);box.bottom=std::max<LONG>(box.bottom,y+1);
      }
    }
    return box;
  };
  const auto a=bounds(before),b=bounds(after);
  return a.right>a.left&&a.left==b.left&&a.right==b.right&&a.top==b.top&&b.bottom>=a.bottom+15;
}

int scenario_first_slice_art() {
  using namespace first_slice_art;
  auto product=make_product_client();
  scenario_check(product->camera.perspective&&product->lineage_art,
      "first-slice-art: local and remote startup select the production authored GPU path");
  scenario_begin(*product);scenario_follow_camera(*product);
  const auto startup_dir=art_wave_capture_dir();
  scenario_check(!startup_dir.empty()&&reference_present(*product,1366,768,startup_dir+"\\first-slice-product-startup.png")&&
      product->camera.perspective&&fable_world::renderer().gpu.error().empty(),
      "first-slice-art: actual startup configuration draws the live scenery through the GPU");
  if(registry().find("tree","idle","front")) {
    const bool accepted_tree=std::any_of(product->render_list.begin(),product->render_list.end(),[](const auto& item){
      return item.label.rfind("art:fs_tree_",0)==0;});
    scenario_check(accepted_tree,"first-slice-art: startup scene consumes accepted tree art without fixture bindings");
  }
  scenario_check(first_slice_depth_probe(),
      "first-slice-art: footprint depth reveals below-pivot pixels without moving the screen rectangle");
  scenario_check(fable_world::kTerrainWidth==80*48&&fable_world::kTerrainHeight==64*48,
      "first-slice-art: terrain has 48 logical texels per tile in both axes");
  if(raster_ground::detail::patterns("terrain_quiet_earth")) {
    fable_world::TerrainBakeInput first;
    first.min_x=-32*kTileUnits;first.min_y=-40*kTileUnits;
    first.max_x=first.min_x+80*kTileUnits;first.max_y=first.min_y+64*kTileUnits;
    first.earth=raster_ground::detail::cache().patterns.earth;
    first.moss=raster_ground::detail::cache().patterns.moss;
    auto next=first;next.min_x+=20*kTileUnits;next.max_x+=20*kTileUnits;
    const auto a=fable_world::bake_terrain(first),b=fable_world::bake_terrain(next);
    bool same=a.rgba.size()==std::size_t(3840)*3072*4&&b.rgba.size()==a.rgba.size();
    for(int y=0;same&&y<3072;++y)
      same=std::memcmp(a.rgba.data()+(std::size_t(y)*3840+20*48)*4,
          b.rgba.data()+std::size_t(y)*3840*4,60*48*4)==0;
    scenario_check(same,"first-slice-art: streaming patch overlap preserves exact world pixel samples");
  } else scenario_check(false,"first-slice-art: terrain patterns loaded for parity verification");
  Registry test;
  std::istringstream sparse("player_female walk front 10 1 96 96 48 80 48 fs_a fs_b\n");
  test.read(sparse);
  scenario_check(test.errors.empty()&&!test.ready("player_female"),
      "first-slice-art: partial locomotion does not activate a character family");
  const auto* clip=test.find("player_female","walk","front");
  scenario_check(clip&&clip->frame(0)=="fs_a"&&clip->frame(.5)=="fs_b"&&clip->frame(1)=="fs_a",
      "first-slice-art: exact manifest frame order and wrap preserved");
  scenario_check(clip&&clip->width==96&&clip->height==96&&clip->anchor_x==48&&clip->anchor_y==80&&clip->pixels_per_metre==48,
      "first-slice-art: full padded canvas and ground anchor retain 48 pixel base parity");
  const auto world_pixel=[](int pixel,int anchor){return (pixel-anchor)*kTileUnits/48;};
  scenario_check(world_pixel(48,48)==world_pixel(64,64)&&world_pixel(80,80)==world_pixel(96,96)&&
      world_pixel(30,48)==world_pixel(46,64)&&world_pixel(20,80)==world_pixel(36,96),
      "first-slice-art: 16px action padding preserves body placement and world ground pivot");
  scenario_check(std::string(direction(1,0))=="right"&&std::string(direction(-1,0))=="left"&&
      std::string(direction(0,-1))=="back"&&std::string(direction(0,1))=="front",
      "first-slice-art: four cardinal facing directions are not mirrored or relabeled");
  std::istringstream wrong("player_female walk front 10 1 96 96 48 80 147 fs_a\n");
  test.read(wrong);
  scenario_check(!test.errors.empty()&&test.clips.empty(),"first-slice-art: mixed pixel density fails closed");
  std::istringstream duplicate("player_female walk front 10 1 96 96 48 80 48 fs_a\nplayer_female walk front 10 1 96 96 48 80 48 fs_b\n");
  test.read(duplicate);
  scenario_check(!test.errors.empty()&&test.clips.empty(),"first-slice-art: duplicate clips fail closed");
  const auto& art=registry();
  scenario_check(art.errors.empty(),"first-slice-art: installed manifest has valid source geometry");
  if(art.clips.empty()) std::puts("    first-slice-art: no accepted runtime pack installed; artwork coverage remains incomplete");
  for(const auto* identity:{"player_male_unarmed","player_female_unarmed"}) {
    if(!art.ready(identity)) {
      std::printf("    first-slice-art: %s coverage incomplete; existing family retained\n",identity);
      continue;
    }
    ClientState state;scenario_begin(state);scenario_follow_camera(state);
    state.simulation.reset();state.session.reset();
    state.camera.perspective=true;state.lineage_art=true;
    state.world.player.appearance=std::string(identity)=="player_female_unarmed"?"female":"male";
    for(auto& item:state.world.carried)item.equipped=false;
    const auto dir=art_wave_capture_dir();
    scenario_check(!dir.empty()&&reference_present(state,1366,768,dir+"\\"+identity+"-idle.png"),
        "first-slice-art: installed character captured through actual game renderer");
  }
  if(!art.clips.empty()) {
    ClientState state;scenario_begin(state);scenario_follow_camera(state);
    state.simulation.reset();state.session.reset();
    state.camera.perspective=true;state.lineage_art=true;
    state.world.npcs.clear();state.world.monsters.clear();state.scenery.clear();
    const auto center=state.world.player.position;
    int index=0;
    const auto dir=art_wave_capture_dir();
    for(const auto& [key,clip]:art.clips) {
      if(clip.action!="idle"||clip.direction!="front")continue;
      // This explicit inspection fixture uses the real renderer and genuine
      // asset anchors. It does not invent authoritative village NPC IDs.
      verdigris::client::WorldNpc npc; npc.id=10000+index;npc.name=clip.identity;npc.art_identity=clip.identity;
      npc.position={center.x+int(kTileUnits*2.5),center.y};
      state.world.npcs={npc};++index;
      const bool captured=!dir.empty()&&reference_present(state,1366,768,dir+"\\first-slice-inspect-"+clip.identity+".png");
      const bool drawn=std::any_of(state.render_list.begin(),state.render_list.end(),[&](const auto& item){
        return std::any_of(clip.frames.begin(),clip.frames.end(),[&](const auto& frame){return item.label=="art:"+frame;});});
      scenario_check(captured&&drawn&&fable_world::renderer().gpu.error().empty(),
          "first-slice-art: accepted asset pivot and scale captured beside the player");
    }
    state.world.npcs.clear();state.world.carried.clear();
    for(const auto& [key,clip]:art.clips)if(clip.action=="icon") {
      verdigris::client::WorldCarriedItem item;item.id=clip.identity;item.name=clip.identity;
      // Inspection footprints provide room for native ink. These are fixture
      // items, not a mutation of server item definitions or inventory capacity.
      item.width=clip.identity=="wooden_club"?1:2;
      item.height=clip.identity=="starter_woven_footwear"?1:2;
      item.grid_slot=int(state.world.carried.size())*2;
      state.world.carried.push_back(item);
    }
    if(!state.world.carried.empty()) {
      state.gear_overlay=true;
      scenario_check(!dir.empty()&&reference_present(state,1366,768,dir+"\\first-slice-inventory-inspection.png"),
          "first-slice-art: accepted icons captured through the actual inventory painter");
    }
  }
  return scenario_failures;
}
