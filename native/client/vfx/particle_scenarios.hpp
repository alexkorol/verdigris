#pragma once
int scenario_menu_particles() {
  using namespace verdigris::client::vfx;
  using E=verdigris::client::PresentationEventType;
  ClientState state;scenario_begin(state);scenario_follow_camera(state);ensure_particle_assets(state);
  state.camera.perspective=true;state.lineage_art=true;state.camera.zoom=kCameraDefaultZoom*zoom_height_factor(1080);
  const auto dir=art_wave_capture_dir();
  scenario_check(state.particles.assets.ready() && state.particles.assets.effects.size()==7,"vfx: seven external recipes and tiny atlas load");
  if(!state.particles.assets.ready()) {std::printf("%s\n",state.particles.assets.error.c_str());return scenario_failures;}
  scenario_check(state.billboards.menu_gateway.ready() && state.billboards.menu_control.ready(),"menu: both authored production images loaded");
  for(const auto size:std::array<std::array<int,2>,3>{{{960,600},{1920,1080},{640,480}}}) {
    open_frontend(state,Frontend::Title);
    scenario_check(reference_present(state,size[0],size[1],dir+"/relic-title-"+std::to_string(size[0])+".png"),"menu: exact production title captured at supported size");
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
