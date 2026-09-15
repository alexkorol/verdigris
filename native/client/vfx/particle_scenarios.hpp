#pragma once
int scenario_menu_particles() {
  using namespace verdigris::client::vfx;
  using E=verdigris::client::PresentationEventType;
  ClientState state;scenario_begin(state);scenario_follow_camera(state);ensure_particle_assets(state);
  state.camera.perspective=true;state.lineage_art=true;state.camera.zoom=kCameraDefaultZoom*zoom_height_factor(1080);
  const auto dir=art_wave_capture_dir();
  scenario_check(state.particles.assets.ready() && state.particles.assets.effects.size()==11,"vfx: eleven external recipes and tiny atlas load");
  if(!state.particles.assets.ready()) {std::printf("%s\n",state.particles.assets.error.c_str());return scenario_failures;}
  scenario_check(state.billboards.menu_gateway.ready() && state.billboards.menu_control.ready(),"menu: both authored production images loaded");
  scenario_check(state.billboards.menu_title.ready()&&state.billboards.menu_title_ink.right>state.billboards.menu_title_ink.left,"menu: fantasy title asset has visible alpha ink");
  const auto title_ink=state.billboards.menu_title_ink;
  scenario_check(title_ink.right-title_ink.left>4*(title_ink.bottom-title_ink.top),"menu: nearly invisible canvas specks do not shrink the wide title lettering");
  for(const auto size:std::array<std::array<int,2>,3>{{{960,600},{1920,1080},{640,480}}}) {
    open_frontend(state,Frontend::Title);
    scenario_check(reference_present(state,size[0],size[1],dir+"/relic-title-"+std::to_string(size[0])+".png"),"menu: exact production title captured at supported size");
    scenario_check(render_list_has(state,render::Op::Hud,"frontend:title-art"),"menu: main title renders generated art instead of font fallback");
    bool contained=true;
    for(const auto& hit:state.menu_hits)contained&=hit.rect.left>=0&&hit.rect.right<=size[0]&&hit.rect.top>=0&&hit.rect.bottom<=size[1];
    scenario_check(contained && state.menu_hits.size()==4,"menu: four existing controls stay within viewport");
  }
  open_frontend(state,Frontend::Settings);
  scenario_check(reference_present(state,1920,1080,dir+"/relic-settings.png"),"menu: settings production capture");
  open_frontend(state,Frontend::None);
  clear_particles(state);
  particle_event(state,{E::LevelUp,state.world.player.id,"","",2},state.world);
  const auto resolve=[&](const Attachment& a){return particle_view::resolve(state,a);};
  for(int tick=0;tick<40;++tick) {
    state.particles.tick(resolve);
    if(tick==1||tick==6||tick==13||tick==24)
      scenario_check(reference_present(state,1920,1080,dir+"/level-up-"+std::to_string(tick)+".png"),"vfx: level-up layers captured in production perspective world");
  }
  scenario_check(state.camera.perspective && render_list_has(state,render::Op::Hud,"vfx:atlas-billboards"),"vfx: captures used perspective GPU atlas quads, never legacy fallback");
  scenario_check(state.particles.emitters.empty() && state.particles.particles.empty(),"vfx: level-up fully retires without a persistent light or emitter");
  auto& a=state.particles;ParticleSystem b;b.assets=a.assets;
  a.clear();Attachment at{"",Anchor::World,{20,30,0}};
  a.play("melee_hit_small",at,1234);b.play("melee_hit_small",at,1234);
  auto world=[](const Attachment& t)->std::optional<Vec3>{return t.point;};
  bool same=true;
  for(int n=0;n<5;++n){a.tick(world);b.tick(world);same&=a.particles.size()==b.particles.size();
    for(std::size_t i=0;i<a.particles.size()&&i<b.particles.size();++i)same&=a.particles[i].pos.x==b.particles[i].pos.x&&a.particles[i].pos.z==b.particles[i].pos.z;}
  scenario_check(same,"vfx: seeded replay produces identical positions and lifetimes");
  a.clear();auto loop=a.play("bowl_ember_idle",{state.world.player.id,Anchor::MainHand},3);
  for(int n=0;n<40;++n)a.tick(resolve);
  scenario_check(a.playing(loop)&&!a.particles.empty(),"vfx: attached continuous loop survives duration boundaries");
  a.stop(loop);for(int n=0;n<20;++n)a.tick(resolve);
  scenario_check(a.particles.empty()&&!a.playing(loop),"vfx: explicit stop lets existing particles retire");
  a.play("level_up",{"missing",Anchor::Root},4);a.tick(resolve);
  scenario_check(a.emitters.empty()&&a.particles.empty(),"vfx: missing attachment retires safely");
  a.clear();for(int i=0;i<80;++i)a.play("level_up",{state.world.player.id,Anchor::Root},i+1);
  for(int i=0;i<30;++i)a.tick(resolve);
  scenario_check(a.emitters.size()<=ParticleSystem::max_emitters&&a.particles.size()<=ParticleSystem::max_particles&&a.dropped>0,"vfx: stress rejects overflow within fixed budgets");
  a.clear();auto trail=a.play("projectile_trail_simple",at,5);a.tick(world);
  const auto first=a.particles.front().pos;
  a.attach(trail,{"",Anchor::World,{120,30,0}});a.tick(world);
  scenario_check(a.particles.front().pos.x<40 && a.particles.back().pos.x>100,"vfx: moving projectile origin leaves detached world-space trail behind");
  a.stop(trail);for(int n=0;n<15;++n)a.tick(world);
  scenario_check(a.particles.empty(),"vfx: projectile trail retires after impact stop");
  for(const char* name:{"melee_hit_small","burning_touch_contact","simple_death_puff","bowl_ember_idle","projectile_trail_simple"}) {
    clear_particles(state);a.play(name,{state.world.player.id,Anchor::MainHand},77);
    for(int n=0;n<3;++n)a.tick(resolve);
    scenario_check(reference_present(state,1920,1080,dir+"/"+name+".png"),"vfx: companion effect rendered with the production atlas");
  }
  clear_particles(state);
  const auto initial=state.world.player.position;state.particle_last_step=initial;state.particle_step_known=true;
  state.world.player.position.x+=45;tick_particles(state);
  scenario_check(!a.particles.empty(),"vfx: locomotion sample emits dust");
  state.world.player.position=initial;clear_particles(state);
  WorldCarriedItem bowl;bowl.name="Fire bowl";bowl.equipped=true;bowl.equip_seat="right_hand";state.world.carried.push_back(bowl);
  for(int n=0;n<5;++n)tick_particles(state);
  scenario_check(a.playing(state.bowl_emitter)&&!a.particles.empty(),"vfx: equipped bowl starts its hand-bound idle effect");
  state.world.carried.pop_back();tick_particles(state);
  scenario_check(state.bowl_emitter==0,"vfx: unequipping stops the idle emitter");
  particle_event(state,{E::ConnectionLost},state.world);
  scenario_check(a.particles.empty()&&a.emitters.empty(),"vfx: disconnect clears all transient effects");
  return scenario_failures;
}

// Real server commands feed the production event adapter and perspective paint.
// Only scene/position and one test item are staged; actions resolve normally.
int scenario_gameplay_particles() {
  using namespace verdigris::client;
  using namespace verdigris::client::vfx;
  using E=PresentationEventType;
  using J=verdigris::networking::JsonValue;
  std::setvbuf(stdout,nullptr,_IONBF,0);
  std::unique_ptr<verdigris::networking::WebSocketServer> server;
  unsigned short port=0;
  for(unsigned short candidate=6800;candidate<6820;++candidate) {
    auto probe=std::make_unique<verdigris::networking::WebSocketServer>(candidate);std::string error;
    if(probe->start(&error)){port=candidate;server=std::move(probe);break;}
  }
  scenario_check(bool(server),"particles-play: isolated real server starts");if(!server)return scenario_failures;
  ClientState state;load_billboards(state.billboards);state.camera.perspective=true;state.lineage_art=true;
  state.camera.zoom=kCameraDefaultZoom*zoom_height_factor(1080);state.frontend=Frontend::None;
  auto session=std::make_unique<RemoteProtocolSession>("127.0.0.1",port,"particles-play",true);
  auto* remote=session.get();state.session=std::move(session);std::string error;
  scenario_check(remote->start(&error),"particles-play: socket connects");
  const auto pump=[&](const std::function<bool()>& ready){return chronicles_pump(state,180,ready);};
  scenario_check(pump([&]{return remote->connection_state()==ConnectionState::Ready;}),"particles-play: authoritative admission");
  remote->send_raw("instance:enterSolo",J::Object{{"template","forest"},{"layout","clearings"}});
  scenario_check(pump([&]{return remote->model().scene.type=="instance";}),"particles-play: real expedition entered");
  remote->send_raw("dev:teleport",J::Object{{"x",7},{"y",7}});
  scenario_check(pump([&]{return remote->model().player.x==7 && remote->model().player.y==7;}),"particles-play: contained open-ground capture position");
  scenario_follow_camera(state);clear_particles(state);ensure_particle_assets(state);
  scenario_check(state.particles.assets.ready(),"particles-play: complete external asset set loads");
  if(!state.particles.assets.ready()) {
    std::printf("%s\n",state.particles.assets.error.c_str());remote->shutdown();server->stop();return scenario_failures;
  }
  const auto has=[&](const char* name){return std::any_of(state.particles.emitters.begin(),state.particles.emitters.end(),[&](const auto& e){return e.effect->name==name;});};
  const auto render=[&](const char* name,int ticks){
    for(int n=0;n<ticks;++n)state.particles.tick([&](const Attachment& a){return particle_view::resolve(state,a);});
    scenario_check(reference_present(state,1920,1080,art_wave_capture_dir()+"/"+name+".png"),"particles-play: production perspective capture saved");
    scenario_check(state.camera.perspective&&render_list_has(state,render::Op::Hud,"vfx:atlas-billboards"),"particles-play: GPU atlas path rendered the real event");
  };
  remote->submit(ClientCommand::use_action("war-cry"));
  scenario_check(pump([&]{return has("war_cry");}),"particles-play: accepted War Cry reaches production particle adapter over socket");
  render("war-cry-early",2);render("war-cry-expansion",5);
  clear_particles(state);
  remote->submit(ClientCommand::aim(1,0));remote->submit(ClientCommand::use_action("dash"));
  scenario_check(pump([&]{return has("dash_dust");}),"particles-play: accepted dash emits dust along actual travel");
  const auto dash_count=state.particles.emitters.size();
  scenario_check(dash_count>=2&&dash_count<=6,"particles-play: dash trail uses a bounded number of origins");
  remote->submit(ClientCommand::use_action("dash"));
  // Drain the rejected repeat before capture; presentation simulation remains fixed-step.
  for(int n=0;n<4;++n){std::this_thread::sleep_for(std::chrono::milliseconds(20));remote->poll();ingest_session_events(state);}
  scenario_check(state.particles.emitters.size()==dash_count,"particles-play: cooldown rejection adds no trail");
  scenario_follow_camera(state);render("dash-confirmed",2);clear_particles(state);
  remote->send_raw("dev:give",J::Object{{"itemId","garnet-amulet"}});
  std::string item;
  scenario_check(pump([&]{for(const auto& i:remote->model().inventory)if(i.id=="garnet-amulet"){item=i.uuid;return true;}return false;}),"particles-play: disposable item fixture created");
  scenario_check(!has("pickup_motes"),"particles-play: inventory refresh does not pretend to be a pickup");
  if(!item.empty()) {
    ClientCommand drop;drop.type=ClientCommand::Type::DropInventory;drop.target=item;remote->submit(drop);
    scenario_check(pump([&]{for(const auto& i:remote->model().ground)if(i.uuid==item)return true;return false;}),"particles-play: actual drop creates ground item");
    remote->submit(ClientCommand::pick_up(item));
    scenario_check(pump([&]{return has("pickup_motes");}),"particles-play: accepted ground pickup emits gathering motes");
    render("pickup-confirmed",3);
  }
  clear_particles(state);
  PresentationEvent contact{E::DamageApplied,state.world.player.id,"","outgoing",12};contact.critical=true;
  particle_event(state,contact,state.world);render("critical-contact",4);
  scenario_check(state.particles.spawned>0,"particles-play: critical contact produces atlas particles");
  clear_particles(state);contact.value=0;particle_event(state,contact,state.world);
  scenario_check(state.particles.emitters.empty(),"particles-play: zero damage does not emit impact sparks");
  // A ring is emitted evenly, then expands in world XY without following actor motion.
  state.particles.play("war_cry",{state.world.player.id,Anchor::Feet},19);
  state.particles.tick([&](const Attachment& a){return particle_view::resolve(state,a);});
  bool ring=true;int radial=0;const auto center=state.world.player.displayed_position();
  for(const auto& p:state.particles.particles)if(p.layer->radial){++radial;ring&=std::abs(std::hypot(p.pos.x-center.x,p.pos.y-center.y)-18)<.02f;}
  scenario_check(ring&&radial==40,"particles-play: radial geometry keeps a clean world-space ring");
  if(state.particles.particles.empty()){remote->shutdown();server->stop();return scenario_failures;}
  const auto prior=state.particles.particles.front().pos;
  state.world.player.position.x+=40;state.world.player.has_display_position=false;
  state.particles.tick([&](const Attachment& a){return particle_view::resolve(state,a);});
  scenario_check(std::abs(state.particles.particles.front().pos.x-prior.x)<10,"particles-play: detached ring is not dragged by actor movement");
  clear_particles(state);
  for(int n=0;n<3;++n)state.particles.play("war_cry",{state.world.player.id,Anchor::Feet},n+1);
  for(int n=0;n<5;++n)state.particles.tick([&](const Attachment& a){return particle_view::resolve(state,a);});
  const auto begin=std::chrono::steady_clock::now();
  for(int n=0;n<20;++n)scenario_present_size(state,3440,1440);
  const auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count()/20;
  std::printf("particles-play: 3440x1440 %zu particles average %.3f ms (bound 40)\n",state.particles.particles.size(),ms);
  scenario_check(ms<40,"particles-play: populated particle frame meets unchanged production budget");
  for(int n=0;n<40;++n)state.particles.tick([&](const Attachment& a){return particle_view::resolve(state,a);});
  scenario_check(state.particles.particles.empty()&&state.particles.emitters.empty(),"particles-play: burst particles and emitters retire completely");
  state.particles.play("war_cry",{state.world.player.id,Anchor::Feet});state.world.route_id+="-return";tick_particles(state);
  scenario_check(state.particles.particles.empty()&&state.particles.emitters.empty(),"particles-play: route transition clears old combat effects");
  remote->shutdown();server->stop();return scenario_failures;
}
