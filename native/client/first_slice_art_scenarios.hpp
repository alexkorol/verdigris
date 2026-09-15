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

void first_slice_retained_death_lifecycle() {
  using namespace verdigris::client;
  auto& art=first_slice_art::registry();
  const auto* source=art.find("pack-wolf","walk","right");
  if(!source) {
    std::puts("    first-slice-art: retained death GPU routing probe requires the accepted pack-wolf source");
    return;
  }
  // Diagnostic routing data only: reuse accepted pixels without claiming they
  // depict death. Never install these test clips or save them as accepted art.
  struct Restore { first_slice_art::Registry& target; first_slice_art::Registry saved;
    ~Restore(){target=std::move(saved);} } restore{art,art};
  const auto fixture=*source;
  for(const auto* action:{"idle","walk","attack","hit","death"})
    for(const auto* direction:{"front","right","back","left"}) {
      auto c=fixture;c.action=action;c.direction=direction;
      if(c.action=="death"){c.loop=false;c.anchor_y=76;c.frames.resize(4);}
      art.clips[first_slice_art::key(c.identity,c.action,c.direction)]=std::move(c);
    }
  auto state=make_product_client();
  load_billboards(state->billboards);
  auto session=std::make_unique<LocalCoreSession>(0xC011AB1EULL);
  auto* core_session=session.get();state->session=std::move(session);
  std::string error;
  scenario_check(core_session->start(&error),"first-slice-art: retained death session starts");
  core_session->submit(ClientCommand::enter_zone("route:tin:1:0"));core_session->advance_fixed_tick();
  auto* sim=core_session->simulation_for_scenarios();
  auto* player=sim->actor(sim->scion().actor_id);player->position={0,0};player->facing={1,0};
  const auto foe=sim->spawn_monster({1,0},1,false);sim->actor(foe)->stats.life=1;
  core_session->poll();sync_world(*state);ingest_session_events(*state);
  for(auto& actor:state->event_world.monsters)if(actor.id==foe)actor.kind="pack-wolf";
  core_session->submit(ClientCommand::use_action("melee"));core_session->advance_fixed_tick();
  core_session->poll();sync_world(*state);
  scenario_check(std::none_of(state->world.monsters.begin(),state->world.monsters.end(),
      [&](const auto& actor){return actor.id==foe;}),"first-slice-art: real lethal hit removes the live actor before event drain");
  ingest_session_events(*state);
  auto retained=std::find_if(state->effects.begin(),state->effects.end(),[&](const auto& fx){
    return fx.kind==EffectFx::Kind::ActorFall&&fx.actor_id==foe;});
  scenario_check(retained!=state->effects.end()&&retained->actor_art_identity=="pack-wolf",
      "first-slice-art: removed actor retains exact authored identity through session event ingestion");
  if(retained!=state->effects.end()) {
    const auto* clip=art.find("pack-wolf","death",first_slice_art::direction(retained->actor_facing.x,retained->actor_facing.y));
    const auto position=std::pair(retained->wx,retained->wy);
    // Advance cadence, then beyond the death clip while the corpse persists.
    for(int age:{0,2,4,6,12}) {
      retained->age=age;state->tick_accum_ms=0;scenario_follow_camera(*state);scenario_present(*state);
      const auto frame=clip->frame(std::min(.999999,age*.05*clip->fps/clip->frames.size()));
      const auto wanted="art:actor-fall:"+foe+":"+frame;
      scenario_check(std::count_if(state->render_list.begin(),state->render_list.end(),[&](const auto& op){
          return op.label==wanted&&op.x==position.first&&op.y==position.second;})==1&&
          std::none_of(state->render_list.begin(),state->render_list.end(),[&](const auto& op){
            return op.label.find("raider_death")!=std::string::npos;}),
          "first-slice-art: removed actor draws one same-family death frame at its retained pivot and holds the last frame");
    }
    scenario_check(clip->anchor_y==76&&clip->width==96&&clip->pixels_per_metre==48,
        "first-slice-art: retained death resolves its action-specific pivot without refitting");
  }
  core_session->shutdown();
}

void first_slice_player_death_equipment_lifecycle() {
  using namespace verdigris::client;
  auto& art=first_slice_art::registry();
  struct Restore { first_slice_art::Registry& target; first_slice_art::Registry saved;
    ~Restore(){target=std::move(saved);} } restore{art,art};
  const auto* source=art.find("pack-wolf","walk","right");
  std::string club_frame,unarmed_frame;
  if(source&&source->frames.size()>=8) {
    const auto fixture=*source;
    // Distinct existing pixels stand in for diagnostic equipment/direction
    // branches. No manufactured player art is installed or published.
    for(const auto* equipment:{"club","unarmed"}) {
      int index=std::string(equipment)=="club"?0:4;
      for(const auto* direction:{"front","right","back","left"}) {
        for(const auto* action:{"idle","walk","sprint","attack","hit","death"}) {
          auto c=fixture;c.identity=std::string("player_male_")+equipment;c.direction=direction;c.action=action;
          c.loop=false;c.frames={fixture.frames[index]};
          art.clips[first_slice_art::key(c.identity,c.action,c.direction)]=std::move(c);
        }
        ++index;
      }
    }
    club_frame=fixture.frames[1];unarmed_frame=fixture.frames[5];
  }
  for(bool remote_adapter:{false,true}) {
    auto state=make_product_client();
    load_billboards(state->billboards);
    LocalCoreSession* session=nullptr;
    verdigris::Simulation* sim=nullptr;
    if(remote_adapter) {
      auto owner=std::make_unique<LocalCoreSession>(0xC011AB1EULL);session=owner.get();
      state->session=std::move(owner);std::string error;
      scenario_check(session->start(&error),"first-slice-art: player death session starts");
      sim=session->simulation_for_scenarios();
    } else {state->simulation=std::make_unique<verdigris::Simulation>(0xC011AB1EULL);sim=state->simulation.get();}
    sim->dispatch(verdigris::Command::enter("route:tin:1:0"));
    // Fixture grants one real core item; equip, loss and successor rules are
    // exercised through the existing core rather than simulated UI inventory.
    auto& scion=const_cast<verdigris::Scion&>(sim->scion());
    scion.carried_items.push_back({"wooden_club","Wooden club",0});
    sim->dispatch(verdigris::Command::equip("wooden_club"));
    auto* player=sim->actor(sim->scion().actor_id);player->position={0,0};player->facing={1,0};
    if(session)session->poll();sync_world(*state);
    scenario_check(state->player_art_snapshot.valid&&state->player_art_snapshot.held==vector_art::Held::Club,
        "first-slice-art: living local/session player records actual equipped club appearance");
    player->stats.life=1;player->stats.defense=0;
    const auto foe=sim->spawn_monster({1,0},1,false);sim->actor(foe)->cooldown_ticks=0;
    player=sim->actor(sim->scion().actor_id);
    for(int attempt=0;attempt<4&&player->alive;++attempt)sim->dispatch_tick({});
    if(session)session->poll();sync_world(*state);
    scenario_check(!state->world.player.alive&&state->world.carried.empty()&&sim->scion().carried_items.empty()&&
        equipped_held(*state)==vector_art::Held::None&&state->player_art_snapshot.held==vector_art::Held::Club&&
        state->player_art_snapshot.appearance=="male"&&state->player_art_snapshot.facing.x==1&&state->player_art_snapshot.facing.y==0,
        "first-slice-art: real player death clears gameplay items while preserving only prior appearance/equipment/facing");
    if(!club_frame.empty()) {
      scenario_follow_camera(*state);scenario_present(*state);
      scenario_check(render_list_has(*state,render::Op::Hud,("art:"+club_frame).c_str())&&
          !render_list_has(*state,render::Op::Hud,("art:"+unarmed_frame).c_str()),
          "first-slice-art: cleared inventory renders the retained club death instead of unarmed death");
    }
    state->motions["player"].death_age_ms=750;
    sim->create_successor("Visual successor");sim->dispatch(verdigris::Command::enter("route:tin:1:0"));
    if(session)session->poll();sync_world(*state);
    scenario_check(state->world.player.alive&&state->player_art_snapshot.actor_id==state->world.player.id&&
        state->player_art_snapshot.held==vector_art::Held::None&&state->motions["player"].death_age_ms==0,
        "first-slice-art: new life resets retained equipment and death clock without restoring lost items");
    state->world.player.alive=false;sync_player_art_snapshot(*state);
    state->world.route_id+="/changed";sync_player_art_snapshot(*state);
    scenario_check(!state->player_art_snapshot.valid,"first-slice-art: scene change clears old player death appearance");
    if(session)session->shutdown();
  }
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
  first_slice_retained_death_lifecycle();
  first_slice_player_death_equipment_lifecycle();
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
