#pragma once
// Two independently launched native processes use the real service and real
// account/Chronicles/party/keyboard paths. No local Simulation or dev command.
namespace service_coop_qa {
using Json=verdigris::networking::JsonValue;
inline std::string env(const char* name) {const auto* value=std::getenv(name);return value?value:"";}
inline std::string read(const std::filesystem::path& path) {
  std::ifstream stream(path,std::ios::binary);std::string text;char c;
  while(stream.get(c) && text.size()<8192)text+=c;return text;
}
inline void write(const std::filesystem::path& path,const std::string& text) {
  const auto tmp=path.string()+".tmp";{std::ofstream out(tmp,std::ios::binary|std::ios::trunc);out<<text;if(!out)throw std::runtime_error("Cannot write QA evidence");}
  std::filesystem::rename(tmp,path);
}
inline Json parse(const std::filesystem::path& path) {Json result;verdigris::networking::parse_json(read(path),result);return result;}
inline std::string field(const Json& value,const std::string& name) {const auto* item=value[name].string();return item?*item:"";}
struct Run {
  ClientState state;HWND window=nullptr;raster_art::detail::Surface surface;
  std::filesystem::path root,output;std::string role,other,actor_id,other_id,house_id,scion_id,town_id,instance_id;
  Json::Array checks;int failures=0;bool own_hit=false;int move_x=0,move_y=0;double max_frame_ms=0,total_frame_ms=0;int frames=0;
  bool charge_accepted=false,warden_credit=false,reward_received=false,reconnected=false;
  std::vector<double> steady_samples;std::string measured_scene;int scene_frames=0;
  int points_before=0,points_after=0;std::string inventory_after;
  explicit Run(std::filesystem::path path,std::string side):root(std::move(path)),output(root/side),role(std::move(side)),other(role=="A"?"B":"A") {}
  ~Run(){if(state.session)state.session->shutdown();if(window){SetWindowLongPtr(window,GWLP_USERDATA,0);DestroyWindow(window);}}
  void require(bool ok,const std::string& label) {
    checks.push_back(Json::Object{{"check",label},{"passed",ok}});std::printf("service-client %s: %s %s\n",role.c_str(),ok?"PASS":"FAIL",label.c_str());std::fflush(stdout);
    if(!ok){++failures;throw std::runtime_error(label);}
  }
  const verdigris::client::ClientModel& model()const{return state.session->model();}
  void tick() {
    const auto until=std::chrono::steady_clock::now()+std::chrono::milliseconds(50);
    timer_step(window,state);paint_scene(state,surface.dc,RECT{0,0,1280,800});
    own_hit=own_hit||model().last_outgoing_hit>0;
    charge_accepted=charge_accepted||model().last_message.find("No road holds past a living Warden")!=std::string::npos;
    warden_credit=warden_credit||model().last_message.find("Your party defeated the first Warden")!=std::string::npos||model().last_message.find("Your first Warden is down")!=std::string::npos;
    reward_received=reward_received||model().last_message.find("You kept your word")!=std::string::npos;
    if(measured_scene!=model().scene.id){measured_scene=model().scene.id;scene_frames=0;}
    if(state.screen==Screen::Expedition&&state.frontend==Frontend::None&&!state.party_open) {
      if(++scene_frames>20 && steady_samples.size()<12000)steady_samples.push_back(state.last_paint_ms);
    }
    total_frame_ms+=state.last_paint_ms;max_frame_ms=std::max(max_frame_ms,state.last_paint_ms);++frames;
    std::this_thread::sleep_until(until);
  }
  bool wait(const std::function<bool()>& done,int milliseconds=20000) {
    const auto until=std::chrono::steady_clock::now()+std::chrono::milliseconds(milliseconds);
    while(std::chrono::steady_clock::now()<until){tick();if(done())return true;if(std::filesystem::exists(root/(other+"-failed")))return false;}
    return done();
  }
  void settle(int milliseconds=400){const auto until=std::chrono::steady_clock::now()+std::chrono::milliseconds(milliseconds);while(std::chrono::steady_clock::now()<until)tick();}
  void mark(const std::string& phase,const std::string& value="ready"){write(root/(role+"-"+phase),value);}
  void barrier(const std::string& phase){mark(phase);require(wait([&]{return std::filesystem::exists(root/(other+"-"+phase));}),"both clients reached "+phase);}
  void click(RECT rect){SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM((rect.left+rect.right)/2,(rect.top+rect.bottom)/2));SendMessage(window,WM_LBUTTONUP,0,0);}
  void key(int key,bool down){SendMessage(window,down?WM_KEYDOWN:WM_KEYUP,key,0);}
  void text(const std::string& value){for(unsigned char c:value)SendMessage(window,WM_CHAR,c,0);}
  void movement(int dx,int dy) {
    if(move_x==dx && move_y==dy)return;
    for(int code:{'W','A','S','D'})key(code,false);move_x=dx;move_y=dy;
    if(dx)key(dx>0?'D':'A',true);if(dy)key(dy>0?'S':'W',true);
  }
  void capture(const std::string& name,int width=1280,int height=800) {
    require(reference_present(state,width,height,(output/(name+".png")).string()),"capture "+name);
    const auto& m=model();Json::Array peers;
    for(const auto& p:m.peers)peers.push_back(Json::Object{{"id",p.uuid},{"scene",p.scene_id},{"appearance",p.appearance},{"x",p.x},{"y",p.y},{"hp",p.life}});
    write(output/(name+".json"),Json(Json::Object{{"build",VERDIGRIS_BUILD_ID},{"pid",int(GetCurrentProcessId())},{"role",role},{"actor",m.player.uuid},{"house",m.chronicle.active_house_id},{"scion",m.chronicle.active_scion_id},{"scene",m.scene.id},{"appearance",m.player.appearance},{"x",m.player.x},{"y",m.player.y},{"hp",m.player.life},{"peers",peers},{"party",m.party.id},{"party_state",m.party.state},{"has_stairs_up",m.scene.has_stairs_up},{"stairs_up_x",m.scene.stairs_up_x},{"stairs_up_y",m.scene.stairs_up_y},{"living_enemies",int(std::count_if(m.monsters.begin(),m.monsters.end(),[](const auto& monster){return monster.alive;}))},{"player_render_ops",int(render::count(state.render_list,render::Op::Player))}}).stringify());
  }
  bool chronicle(const std::string& verb,const std::string& arg="") {
    paint_scene(state,surface.dc,RECT{0,0,1280,800});
    for(const auto& hit:state.chronicle_hits)if(hit.action.command==verb&&(arg.empty()||arg==hit.action.arg)){click(hit.rect);return true;}return false;
  }
  void party_open(bool open) {
    if(state.party_open!=open)click(party_layout(state,1280,800).toggle);
  }
  bool party(const std::string& verb,const std::string& arg="") {
    party_open(true);const auto layout=party_layout(state,1280,800);
    for(const auto& hit:layout.hits)if(hit.verb==verb&&hit.enabled&&(arg.empty()||hit.extra==arg)){click(hit.rect);return true;}return false;
  }
  const verdigris::client::ClientPlayer* peer()const {for(const auto& p:model().peers)if(p.uuid==other_id)return &p;return nullptr;}
  Json identity()const{return Json::Object{{"actor",model().player.uuid},{"house",model().chronicle.active_house_id},{"scion",model().chronicle.active_scion_id},{"scene",model().scene.id}};}
  bool walkable(int x,int y)const {
    const auto& m=model();return x>=0&&y>=0&&x<m.map_width&&y<m.map_height&&int(m.map_walkable.size())==m.map_width*m.map_height&&m.map_walkable[y*m.map_width+x]!=0;
  }
  std::pair<int,int> step_toward(double target_x,double target_y,bool allow_extraction=false)const {
    const auto& m=model();const int sx=int(std::lround(m.player.x)),sy=int(std::lround(m.player.y)),tx=int(std::lround(target_x)),ty=int(std::lround(target_y));
    const int w=m.map_width,h=m.map_height;
    if(w<=0||h<=0||w*h>262144||!walkable(sx,sy))return {0,0};
    std::vector<int> parent(w*h,-1);std::vector<int> queue;std::size_t head=0;const int start=sy*w+sx;queue.push_back(start);parent[start]=start;int finish=-1;
    while(head<queue.size()) {
      const int at=queue[head++];const int x=at%w,y=at/w;
      if(x==tx&&y==ty){finish=at;break;}
      for(const auto [dx,dy]:{std::pair{1,0},{0,1},{-1,0},{0,-1}}) {
        const int nx=x+dx,ny=y+dy;if(!walkable(nx,ny))continue;
        // Entering the rounded entrance tile extracts immediately. It is
        // traversable terrain, but never a combat/loot route intermediate.
        // A character spawned there may walk off; only re-entry is excluded.
        if(!allow_extraction&&m.scene.type=="instance"&&m.scene.has_stairs_up&&
            nx==int(std::lround(m.scene.stairs_up_x))&&ny==int(std::lround(m.scene.stairs_up_y)))continue;
        const int next=ny*w+nx;
        if(parent[next]>=0)continue;parent[next]=at;queue.push_back(next);
      }
    }
    if(finish<0||finish==start)return {0,0};while(parent[finish]!=start)finish=parent[finish];return {finish%w-sx,finish/w-sy};
  }
  std::string inventory_signature()const {
    std::vector<std::string> rows;
    for(const auto& item:model().inventory)rows.push_back(item.uuid+":"+item.id+":"+std::to_string(item.quantity));
    for(const auto& worn:model().worn)rows.push_back(worn.item.uuid+":"+worn.item.id+":"+std::to_string(worn.item.quantity)+":"+worn.seat);
    std::sort(rows.begin(),rows.end());std::string signature;for(const auto& row:rows)signature+=row+"\n";return signature;
  }
  bool approach(double x,double y,double reach=0.65,int timeout_ms=30000,bool allow_extraction=false) {
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(timeout_ms);const auto origin_scene=model().scene.id;
    while(std::chrono::steady_clock::now()<deadline&&model().player.alive) {
      if(model().scene.id!=origin_scene){movement(0,0);return allow_extraction;}
      if(std::hypot(x-model().player.x,y-model().player.y)<=reach){movement(0,0);settle(200);return true;}
      const auto [dx,dy]=step_toward(x,y,allow_extraction);if(!dx&&!dy){movement(0,0);return std::hypot(x-model().player.x,y-model().player.y)<=1.0;}
      movement(dx,dy);settle(100);
    }
    movement(0,0);return false;
  }
  void accept_charge_and_frame_town() {
    const verdigris::client::ClientNpc* guide=nullptr;
    for(const auto& npc:model().npcs)if(npc.id==1&&npc.name.find("Aldwyn")!=std::string::npos){guide=&npc;break;}
    require(guide!=nullptr,"public Aldwyn roster entry is available");const double gx=guide->x,gy=guide->y;
    require(approach(gx,gy,1.0),"ordinary movement reaches Aldwyn");settle();
    const auto at=project(state.camera,RECT{0,0,1280,800},verdigris::client::protocol_to_world(gx),verdigris::client::protocol_to_world(gy));
    const int body=at.y-int(kTileUnits*.7*at.scale);click(RECT{at.x-1,body-1,at.x+1,body+1});
    require(wait([&]{return charge_accepted;}),"ordinary NPC interaction accepts Aldwyn's charge");
    points_before=model().progression.earned_points;
    require(approach(gx+(role=="A"?0:1),gy+1,0.65),"ordinary movement frames separate Scions in arrival court");
    barrier("charge-and-court");settle(1000);capture("town-peers-clear");
  }
  void equip_available_weapon() {
    std::string weapon;
    for(const auto& item:model().inventory)if(item.id=="bronze-dagger"||item.id=="bronze-sword"){weapon=item.uuid;break;}
    if(weapon.empty())return; // No kit is created or assumed by this scenario.
    movement(0,0);key('I',true);key('I',false);settle();
    require(state.gear_overlay,"native Equipment keyboard control opens backpack");
    bool selected=false;
    for(std::size_t i=0;i<=state.world.carried.size();++i) {
      if(state.selected_item_id==weapon){selected=true;break;}
      key(VK_TAB,true);key(VK_TAB,false);
    }
    require(selected,"native inventory focus selects the owned weapon");
    key(VK_RETURN,true);key(VK_RETURN,false);
    require(wait([&]{return std::any_of(model().worn.begin(),model().worn.end(),[&](const auto& worn){return worn.item.uuid==weapon;});}),"native Equip action receives owned weapon acknowledgement");
    capture("owned-weapon-equipped");key('I',true);key('I',false);settle();
  }
  bool combat_step(int frame,const std::string& prefix) {
    if(model().scene.id!=instance_id)require(false,"combat remains in the shared instance until intentional extraction");
    const verdigris::client::ClientMonster* target=nullptr;double nearest=1e9;
    for(const auto& m:model().monsters)if(m.alive){const double d=std::hypot(m.x-model().player.x,m.y-model().player.y);if(d<nearest){nearest=d;target=&m;}}
    if(!target)return false;const double tx=target->x,ty=target->y;
    if(nearest>1.25){const auto [dx,dy]=step_toward(tx,ty);movement(dx,dy);}
    else {
      movement(0,0);const auto at=project(state.camera,RECT{0,0,1280,800},verdigris::client::protocol_to_world(tx),verdigris::client::protocol_to_world(ty));
      SendMessage(window,WM_MOUSEMOVE,0,MAKELPARAM(at.x,at.y));settle(100);click(RECT{at.x-1,at.y-1,at.x+1,at.y+1});
    }
    settle(100);
    // Snapshot encoding can be slower than an authority tick. Release real
    // keys before capture so a route step cannot run past its next corner.
    movement(0,0);settle(100);
    if(model().scene.id!=instance_id)require(false,"combat movement preserves the shared instance");
    if(frame<12||frame%30==0)capture(prefix+"-"+std::to_string(frame));return true;
  }
  void complete_and_reconnect(const std::string& endpoint) {
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(120);int frame=0;
    key('R',true);key('R',false);
    while(std::chrono::steady_clock::now()<deadline&&model().player.alive) {
      if(!combat_step(frame++,"warden-fight"))break;
    }
    movement(0,0);settle();require(model().player.alive,"own Scion survives shared Warden encounter");
    // Full snapshots retire defeated monsters, so a cleared room may have
    // an empty roster. Exact scene identity and the Warden acknowledgement
    // below distinguish that legitimate state from accidental extraction.
    require(model().scene.id==instance_id&&std::none_of(model().monsters.begin(),model().monsters.end(),[](const auto& m){return m.alive;}),"shared first floor is cleared by ordinary combat");
    require(wait([&]{return warden_credit;},5000),"this House receives shared first-Warden credit");barrier("warden-cleared");capture("warden-cleared");
    if(!model().ground.empty()&&(role=="A"||model().ground.size()>1)) {
      const auto drop=role=="A"?model().ground.front():model().ground.back();
      if(approach(drop.x,drop.y,1.0,15000)) {key('X',true);key('X',false);settle(1000);}
    }
    capture("owned-loot");barrier("loot-observed");
    const auto exit=model().scene;require(exit.has_stairs_up,"authoritative extraction position exists");
    require(approach(exit.stairs_up_x,exit.stairs_up_y,1.0,45000,true)||model().scene.type!="instance","ordinary return movement reaches extraction");
    if(model().scene.type=="instance")require(party("party:returnToTown"),"native Return to town control requests extraction");
    require(wait([&]{return model().scene.type!="instance";}),"own Scion returns to town without replacing ally world");party_open(false);
    require(wait([&]{return reward_received;}),"own House reward is acknowledged after return");
    require(wait([&]{return model().progression.present&&model().progression.earned_points>points_before;}),"authoritative passive budget includes earned progression");
    settle(1000);points_after=model().progression.earned_points;inventory_after=inventory_signature();capture("returned-house-progress");barrier("returned");
    state.session->shutdown();state.frontend=Frontend::Title;state.screen=Screen::Chronicles;
    state.session=verdigris::client::RemoteProtocolSession::online(endpoint);std::string error;
    require(state.session->start(&error),"new connection starts without enrollment credentials");
    require(wait([&]{return model().authenticated&&model().player.uuid==actor_id&&!model().chronicles_pending;},20000),"cached protected credential reconnects original actor");
    require(wait([&]{return inventory_signature()==inventory_after&&model().progression.earned_points==points_after;}),"reconnect preserves exact owned items and passive budget");
    require(model().chronicle.active_house_id==house_id&&model().chronicle.active_scion_id==scion_id&&model().player.alive,"reconnect retains owned living House and Scion");
    key(VK_RETURN,true);key(VK_RETURN,false);settle();reconnected=true;capture("reconnected");barrier("reconnected");
  }
  void move_phase(const std::string& moving_role) {
    party_open(false);settle();const auto self_before=model().player;const auto peer_before=peer()?*peer():verdigris::client::ClientPlayer{};
    if(role!=moving_role)mark("watch-"+moving_role);
    else require(wait([&]{return std::filesystem::exists(root/(other+"-watch-"+moving_role));}),"ally observes independent movement");
    if(role==moving_role) {
      const int x=int(std::lround(self_before.x)),y=int(std::lround(self_before.y));std::pair<int,int> direction{0,0};
      for(const auto d:{std::pair{1,0},{0,1},{-1,0},{0,-1}})if(walkable(x+d.first,y+d.second)){direction=d;break;}
      require(direction.first||direction.second,"ordinary movement has a walkable direction");
      movement(direction.first,direction.second);
      for(int frame=0;frame<6;++frame){settle(100);capture("move-"+moving_role+"-"+std::to_string(frame));}
      movement(0,0);settle();require(std::hypot(model().player.x-self_before.x,model().player.y-self_before.y)>.05,"local actor moves from native keyboard intent");mark("moved");
    } else {
      for(int frame=0;frame<6;++frame){settle(100);capture("observe-"+moving_role+"-"+std::to_string(frame));}
      require(wait([&]{return std::filesystem::exists(root/(other+"-moved"));}),"moving ally finished");settle();
      require(std::hypot(model().player.x-self_before.x,model().player.y-self_before.y)<.01,"ally movement leaves this actor stationary");
      require(peer()&&std::hypot(peer()->x-peer_before.x,peer()->y-peer_before.y)>.05,"remote peer motion arrived through service");
    }
    barrier("movement-"+moving_role+"-observed");
  }
  void execute(const std::string& endpoint,const std::filesystem::path& code_file) {
    std::filesystem::create_directories(output);require(!std::filesystem::exists(root/(role+"-started")),"fresh per-role evidence directory");mark("started");
    state.camera.perspective=true;state.lineage_art=true;state.chronicles_mode=true;state.frontend=Frontend::Title;state.screen=Screen::Chronicles;
    state.pad.inject=true;state.pad.connected=false;load_billboards(state.billboards);warm_combat_glyphs();
    require(surface.create(1280,800),"native paint surface allocated");
    WNDCLASSA klass{};klass.hInstance=GetModuleHandle(nullptr);klass.lpfnWndProc=window_proc;klass.lpszClassName="VerdigrisServiceClientQA";RegisterClassA(&klass);
    window=CreateWindowExA(0,klass.lpszClassName,"Verdigris service QA",WS_POPUP,0,0,1280,800,nullptr,nullptr,klass.hInstance,&state);
    require(window!=nullptr,"actual graphical client owns its input window");
    state.camera.zoom=kCameraDefaultZoom*zoom_height_factor(800);state.session=verdigris::client::RemoteProtocolSession::online(endpoint);
    std::string error;require(state.session&&state.session->start(&error),"independent service session starts");
    require(wait([&]{return state.session->connection_state()==verdigris::client::ConnectionState::Connected;}),"service transport connected before account admission");
    std::string code=read(code_file);while(!code.empty()&&(code.back()=='\r'||code.back()=='\n'||code.back()==' '))code.pop_back();
    require(!code.empty()&&code.size()<=512,"operator enrollment file is present");text(code);SecureZeroMemory(code.data(),code.size());code.clear();
    click(service_account_layout(1280,800).enroll);
    require(wait([&]{return model().authenticated&&model().chronicle.present;}),"real service authenticates enrollment and private Chronicle");
    require(state.account_credential.empty(),"enrollment UI clears credential after submit");
    key(VK_RETURN,true);key(VK_RETURN,false);settle();
    require(chronicle("house-name"),"House name field is actionable");text("Service House "+role);
    require(chronicle("found-house"),"native House founding control");require(wait([&]{return model().chronicle.houses.size()==1;}),"private owned House persisted");
    require(chronicle("scion-name"),"Scion name field is actionable");text(role=="A"?"Alder":"Beatrice");
    require(chronicle("appearance",role=="A"?"male":"female"),"authored appearance chosen through native card");
    require(chronicle("create-scion"),"native Scion creation control");require(wait([&]{return !model().chronicle.houses.empty()&&model().chronicle.houses.front().scions.size()==1;}),"owned living Scion persisted");
    scion_id=model().chronicle.houses.front().scions.front().id;
    require(chronicle("set-out",scion_id)||chronicle("select-scion",scion_id),"native owned-Scion admission control");
    require(wait([&]{return state.session->connection_state()==verdigris::client::ConnectionState::Ready&&!model().player.uuid.empty()&&!model().chronicles_pending&&state.screen==Screen::Expedition;}),"owned Scion admitted into town");
    actor_id=model().player.uuid;house_id=model().chronicle.active_house_id;town_id=model().scene.id;mark("admitted",identity().stringify());
    require(wait([&]{return std::filesystem::exists(root/(other+"-admitted"));}),"other native process admitted");const auto ally=parse(root/(other+"-admitted"));other_id=field(ally,"actor");
    require(!actor_id.empty()&&!other_id.empty()&&actor_id!=other_id&&house_id!=field(ally,"house")&&scion_id!=field(ally,"scion"),"distinct actors, Houses and Scions across processes");
    require(wait([&]{return peer()!=nullptr&&peer()->scene_id==model().scene.id;}),"same-room peer arrives from authoritative service");
    require(peer()->appearance!=(model().player.appearance),"both real accounts retain distinct authored appearances");
    move_phase("A");move_phase("B");accept_charge_and_frame_town();equip_available_weapon();capture("town-peers");require(render::count(state.render_list,render::Op::Player)>=2,"native GPU paints both live service actors");
    if(role=="A") {
      require(party("party:create"),"native Create party control");require(wait([&]{return !model().party.id.empty();}),"party creation acknowledged");
      require(party("party:invite",other_id),"native Invite targets visible intended actor");
    } else {
      require(wait([&]{return !model().party.invite_id.empty();}),"recipient receives stored invitation");require(party("party:invite:accept"),"native Accept invitation control");
    }
    require(wait([&]{return model().party.members.size()==2;}),"both members appear in authoritative party");
    require(party("party:ready"),"native Ready control");require(wait([&]{return std::all_of(model().party.members.begin(),model().party.members.end(),[](const auto& m){return m.ready;});}),"both members ready");capture("party-ready");
    barrier("party-ready");if(role=="A")require(party("party:startInstance"),"leader starts expedition through native control");
    require(wait([&]{return model().scene.type=="instance"&&model().scene.id!=town_id&&model().party.state=="instance";}),"party enters fresh authoritative instance");party_open(false);settle();
    instance_id=model().scene.id;mark("instance",identity().stringify());require(wait([&]{return std::filesystem::exists(root/(other+"-instance"));}),"other process reports instance");
    require(instance_id==field(parse(root/(other+"-instance")),"scene"),"two native processes share exact authoritative instance ID");
    require(wait([&]{return peer()&&peer()->scene_id==instance_id&&!model().monsters.empty()&&model().map_width>0;}),"shared actor, enemy and navigation snapshots arrive");capture("shared-expedition");
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(35);int frame=0;
    while(std::chrono::steady_clock::now()<deadline&&!own_hit&&model().player.alive) {
      if(!combat_step(frame,"fight-motion"))break;++frame;
    }
    movement(0,0);require(own_hit,"this actor lands a real shared-encounter hit through native controls");mark("fought");
    require(wait([&]{return std::filesystem::exists(root/(other+"-fought"));},40000),"both native players participate in shared combat");capture("shared-fight");
    const double old_zoom=state.camera.zoom;state.camera.zoom=kCameraDefaultZoom*zoom_height_factor(1440);capture("shared-fight-fullscreen",3440,1440);state.camera.zoom=old_zoom;
    complete_and_reconnect(endpoint);barrier("finished");
  }
  void report(const std::string& error) {
    auto sorted=steady_samples;std::sort(sorted.begin(),sorted.end());double sum=0;for(double sample:sorted)sum+=sample;
    const auto percentile=[&](double p){return sorted.empty()?0.0:sorted[std::min(sorted.size()-1,std::size_t((sorted.size()-1)*p))];};
    Json result=Json::Object{{"status",error.empty()?"passed":"failed"},{"role",role},{"pid",int(GetCurrentProcessId())},{"build",VERDIGRIS_BUILD_ID},{"actor",actor_id},{"peer",other_id},{"house",house_id},{"scion",scion_id},{"town",town_id},{"instance",instance_id},{"own_hit",own_hit},{"charge_accepted",charge_accepted},{"warden_credit",warden_credit},{"reward_received",reward_received},{"reconnected",reconnected},{"points_before",points_before},{"points_after",points_after},{"inventory_signature",inventory_after},{"steady_paint_samples",int(sorted.size())},{"steady_average_paint_ms",sorted.empty()?0.0:sum/sorted.size()},{"steady_p50_paint_ms",percentile(.50)},{"steady_p95_paint_ms",percentile(.95)},{"steady_peak_paint_ms",sorted.empty()?0.0:sorted.back()},{"steady_sample_rule","First 20 gameplay frames per authoritative scene excluded; frontends and open party panels excluded; concurrent clients on one machine"},{"checks",checks},{"error",error},{"last_message",state.session?model().last_message:std::string{}},{"account_error",state.session?model().account_error:std::string{}},{"frames",frames},{"average_paint_ms",frames?total_frame_ms/frames:0},{"peak_paint_ms",max_frame_ms},{"topology","two separate native client processes and independent service; app-owned hidden windows"}};
    write(output/"result.json",result.stringify());
  }
};
}
int scenario_service_client() {
  using namespace service_coop_qa;
  const auto endpoint=env("VERDIGRIS_QA_ENDPOINT"),code=env("VERDIGRIS_QA_CODE_FILE"),role=env("VERDIGRIS_QA_ROLE"),sync=env("VERDIGRIS_QA_SYNC_DIR");
  if(endpoint.empty()||code.empty()||(role!="A"&&role!="B")||sync.empty()) {std::fprintf(stderr,"service-client needs QA endpoint, code file, role A/B and fresh sync directory\n");++scenario_failures;return 1;}
  Run run(sync,role);
  try {run.execute(endpoint,code);run.report("");return 0;}
  catch(const std::exception& error) {try{run.mark("failed",error.what());if(run.window&&run.surface.dc&&run.state.session)run.capture("failure");run.report(error.what());}catch(...){}std::fprintf(stderr,"service-client %s failed: %s\n",role.c_str(),error.what());++scenario_failures;return 1;}
}
