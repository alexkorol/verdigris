#pragma once
// An explicitly arranged authoritative-model fixture. Tests native paint and
// Win32 input ownership; transport/multiplayer authority has separate gates.
class CoopScenarioSession final : public verdigris::client::IClientSession {
 public:
  verdigris::client::ClientModel data;
  std::vector<verdigris::client::ClientCommand> commands;
  std::string error;
  bool start(std::string*) override{return true;} void shutdown()override{}
  void submit(const verdigris::client::ClientCommand& c)override{commands.push_back(c);}
  void poll()override{}
  verdigris::client::ConnectionState connection_state()const override{return verdigris::client::ConnectionState::Ready;}
  const verdigris::client::ClientModel& model()const override{return data;}
  std::vector<verdigris::client::PresentationEvent> drain_events()override{return {};}
  const std::string& last_error()const override{return error;}
};
int scenario_coop_presentation() {
  ClientState state;scenario_begin(state);scenario_follow_camera(state);
  state.camera.perspective=true;state.lineage_art=true;
  const auto dir=art_wave_capture_dir();
  scenario_check(!dir.empty(),"coop fixture: contained evidence root");
  auto session=std::make_unique<CoopScenarioSession>();auto* fixture=session.get();
  auto& model=fixture->data;model.scene.id=state.world.route_id;model.theme=state.world.theme;
  model.player.uuid="actor:alice";model.player.display_name="Alice";
  model.player.x=double(state.world.player.position.x)/kTileUnits;
  model.player.y=double(state.world.player.position.y)/kTileUnits;
  model.player.appearance="male";
  auto peer=model.player;peer.uuid="actor:bea";peer.display_name="Beatrice";
  peer.x-=.9;peer.y+=.4;peer.appearance="female";peer.life=63;peer.facing="left";peer.held_item="handaxe";
  model.peers.push_back(peer);
  state.simulation.reset();
  state.session=std::move(session);sync_world_from_model(state.world,model);state.world.monsters.clear();
  state.party_open=true;advance_actor_motion(state,50);
  scenario_check(state.world.peers.size()==1 && state.world.player.id=="actor:alice" && state.world.player.life==100 && state.world.peers[0].life==63,
      "coop fixture: distinct public actors preserve local health and identity");
  const auto camera=state.camera;
  scenario_check(reference_present(state,1280,800,dir+"/coop-nearby-fixture.png"),"coop fixture: actual perspective nearby-player pixels captured");
  scenario_check(render::count(state.render_list,render::Op::Player)==2,"coop fixture: two authored Scion sprites reached production render list");
  WNDCLASSA klass{};klass.hInstance=GetModuleHandle(nullptr);klass.lpfnWndProc=window_proc;klass.lpszClassName="VerdigrisCoopScenario";RegisterClassA(&klass);
  HWND window=CreateWindowExA(0,klass.lpszClassName,"Co-op input fixture",WS_OVERLAPPEDWINDOW,0,0,1280,800,nullptr,nullptr,klass.hInstance,&state);
  scenario_check(window!=nullptr,"coop fixture: real native window accepts ordinary pointer input");
  if(!window)return scenario_failures;
  const auto click=[&](const std::string& verb) {
    RECT r{};GetClientRect(window,&r);const auto layout=party_layout(state,r.right,r.bottom);
    for(const auto& hit:layout.hits)if(hit.verb==verb) {
      SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM((hit.rect.left+hit.rect.right)/2,(hit.rect.top+hit.rect.bottom)/2));
      SendMessage(window,WM_LBUTTONUP,0,0);return true;
    }return false;
  };
  const auto attacks=state.combat_requests;
  scenario_check(click("party:create") && !fixture->commands.empty() && fixture->commands.back().target=="party:create","coop fixture: Create button sends typed party intent");
  model.party.id="party:fixture";model.party.leader_id=model.player.uuid;model.party.state="forming";
  model.party.members={{model.player.uuid,"Alice",false}};sync_world_from_model(state.world,model);
  scenario_check(click("party:invite") && fixture->commands.back().extra==peer.uuid,"coop fixture: visible intended player is invited by actor UUID");
  model.party.members.push_back({peer.uuid,"Beatrice",true});sync_world_from_model(state.world,model);
  scenario_check(click("party:ready") && fixture->commands.back().value==1,"coop fixture: ready intent uses true");
  model.party.members[0].ready=true;sync_world_from_model(state.world,model);
  scenario_check(click("party:ready") && fixture->commands.back().value==0,"coop fixture: unready intent uses false");
  scenario_check(click("party:startInstance") && fixture->commands.back().target=="party:startInstance","coop fixture: ready leader can request expedition");
  model.party.error="Another member is reconnecting. Try again shortly.";sync_world_from_model(state.world,model);
  scenario_check(reference_present(state,1280,800,dir+"/coop-party-fixture.png"),"coop fixture: member status and readable error captured");
  scenario_check(reference_present(state,960,600,dir+"/coop-party-small-fixture.png"),"coop fixture: compact viewport captured");
  model.party={};model.party.invite_id="party:invitation";model.party.invited_by="Beatrice";sync_world_from_model(state.world,model);
  scenario_check(click("party:invite:accept") && fixture->commands.back().extra=="party:invitation","coop fixture: accept uses authoritative invitation ID");
  scenario_check(click("party:invite:decline") && fixture->commands.back().target=="party:invite:decline","coop fixture: decline is usable");
  scenario_check(state.combat_requests==attacks && !state.primary_down,"coop fixture: every party click avoids gameplay attack");
  state.party_open=false;
  for(int frame=0;frame<8;++frame) {
    model.peers[0].x+=.12;model.peers[0].facing="right";sync_world_from_model(state.world,model);advance_actor_motion(state,50);
    scenario_check(reference_present(state,1280,800,dir+"/coop-motion-fixture-"+std::to_string(frame)+".png"),"coop fixture: peer motion frame captured");
  }
  scenario_check(state.motions["peer:"+peer.uuid].moving>.2 && state.world.player.id=="actor:alice","coop fixture: peer has independent bounded walk state");
  verdigris::client::PresentationFx fx;verdigris::client::PresentationEvent event;
  event.type=verdigris::client::PresentationEventType::AttackStarted;event.actor_id=peer.uuid;
  verdigris::client::apply_presentation_event(fx,state.world,event,10);
  scenario_check(verdigris::client::actor_strike(fx.effects,peer.uuid)!=nullptr && !verdigris::client::actor_strike(fx.effects,model.player.uuid),"coop fixture: peer strike never poses local Scion");
  scenario_check(particle_view::actor(state.world,peer.uuid)==&state.world.peers[0],"coop fixture: particle anchors resolve peer actor");
  model.peers.clear();sync_world_from_model(state.world,model);advance_actor_motion(state,50);
  scenario_check(!state.motions.contains("peer:"+peer.uuid),"coop fixture: departing peer motion is evicted");
  SetWindowLongPtr(window,GWLP_USERDATA,0);DestroyWindow(window);
  return scenario_failures;
}
