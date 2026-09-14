#pragma once

// The shipped client, production Win32 handlers and a real native server.
// Dev grants only arrange fixtures; all equips, rejects and unequips use the
// normal protocol. Run this scenario from the final packaged executable too.
int scenario_inventory_equipment() {
  std::setvbuf(stdout, nullptr, _IONBF, 0);
  using namespace verdigris::client;
  using JV = verdigris::networking::JsonValue;
  std::unique_ptr<verdigris::networking::WebSocketServer> server;
  unsigned short port = 0;
  const auto qa_saves=std::filesystem::path(art_wave_capture_dir())/("inventory-profile-"+std::to_string(GetTickCount64()));
  std::filesystem::create_directories(qa_saves);
  for (unsigned short p = 6780; p < 6800; ++p) {
    auto probe = std::make_unique<verdigris::networking::WebSocketServer>(p,qa_saves);
    std::string error;
    if (probe->start(&error)) { port = p; server = std::move(probe); break; }
  }
  scenario_check(server != nullptr, "inventory: isolated server starts");
  if (!server) return scenario_failures;
  ClientState state;
  state.screen = Screen::Expedition;
  state.frontend = Frontend::None;
  state.camera.perspective = true;
  state.lineage_art = true;
  state.session = std::make_unique<RemoteProtocolSession>("127.0.0.1", port, "inventory-qa", true);
  auto* remote = static_cast<RemoteProtocolSession*>(state.session.get());
  std::string error;
  scenario_check(state.session->start(&error), "inventory: normal client connects");
  auto pump = [&](auto done) { return chronicles_pump(state, 250, done); };
  scenario_check(pump([&] { return state.session->connection_state() == ConnectionState::Ready; }),
                 "inventory: admitted with authoritative combat fields");
  scenario_check(state.session->model().player.combat_stats_present, "inventory: login has combat stats");
  auto grant = [&](const char* item, int qty = 1) {
    int seed=42;
    // Choose an existing, art-backed head material for the visual fixture.
    if(std::string(item)=="vessel-crest") {
      for(int candidate=0;candidate<100;++candidate) {
        verdigris::Mulberry32 rng(candidate);verdigris::VesselForge forge;verdigris::CreateItemOptions options;options.rng=&rng;options.item_level=12;options.forge=&forge;
        auto trial=verdigris::create_game_item(item,options);
        if(trial && trial->vessel && trial->vessel->item.material_id=="hide") {seed=candidate;break;}
      }
    }
    const auto before = state.session->model().inventory.size();
    remote->send_raw("dev:give", JV::Object{{"itemId", item}, {"qty", qty}, {"seed", seed}, {"itemLevel", 12}});
    scenario_check(pump([&] { return state.session->model().inventory.size() > before; }), "inventory: fixture reaches real backpack");
    for (const auto& row : state.session->model().inventory)
      if (row.id == item) return row.uuid;
    return std::string{};
  };
  const auto spear = grant("vessel-spear");
  const auto shield = grant("vessel-shield");
  const auto wrap = grant("vessel-wrap");
  const auto crest = grant("vessel-crest");
  const auto ring = grant("vessel-ring");
  const auto ring2 = grant("gold-ring");
  const auto axe = grant("vessel-handaxe");
  const auto feet = grant("vessel-sandals");
  const auto belt = grant("hide-girdle");
  const auto neck = grant("vessel-gorget");
  const auto hands = grant("bronze-gloves");
  load_billboards(state.billboards);
  state.gear_overlay = true;
  scenario_follow_camera(state);
  WNDCLASSA wc{}; wc.hInstance = GetModuleHandle(nullptr);
  wc.lpfnWndProc = window_proc; wc.lpszClassName = "VerdigrisInventoryTest";
  RegisterClassA(&wc);
  HWND window = CreateWindowExA(0, wc.lpszClassName, "Inventory acceptance", WS_POPUP,
                                0, 0, 1366, 768, nullptr, nullptr, wc.hInstance, &state);
  scenario_present_size(state,1366,768);
  scenario_check(std::none_of(state.hud_rect_trace.begin(),state.hud_rect_trace.end(),[](const auto& hit){return hit.first=="inventory-extension-button";}),
      "extensions: no auxiliary controls appear before skill-tree unlock");
  for(std::size_t seat=11;seat<14;++seat){const auto r=make_pack_geom(1366,768).seats[seat];
    scenario_check(r.right<=r.left,"extensions: reserved seats are absent from main paperdoll");}
  auto worn = [&](const std::string& id, const std::string& seat) {
    for (const auto& item : state.session->model().worn)
      if (item.item.uuid == id && item.seat == seat) return true;
    return false;
  };
  int visual_drag=0;
  auto drag_to_seat = [&](const std::string& id, paper_doll::Slot seat) {
    sync_world(state); reconcile_pack_grid(state);
    const auto i = inventory_grid::find_index(state.pack_grid, pack_stable_id(id));
    if (i == inventory_grid::kMaxItems) { scenario_check(false, "inventory: drag item exists"); return; }
    const auto geom = make_pack_geom(1366, 768);
    const auto item = state.pack_grid.items[i];
    const int x = geom.grid_left + (item.x+item.width-1) * (geom.cell_w + geom.gap) + geom.cell_w / 2;
    const int y = geom.grid_top + (item.y+item.height-1) * (geom.cell_h + geom.gap) + geom.cell_h / 2;
    const auto target = geom.seats[paper_doll::slot_index(seat)];
    SendMessage(window, WM_LBUTTONDOWN, 0, MAKELPARAM(x, y));
    SendMessage(window, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM((target.left+target.right)/2,(target.top+target.bottom)/2));
    if(visual_drag<2) {
      scenario_check(reference_present(state,1366,768,art_wave_capture_dir()+
          (visual_drag==0?"/ui-invalid-destination.png":"/ui-valid-destination.png")),
          "inventory: actual held drag and destination captured before the drop");
    }
    SendMessage(window, WM_LBUTTONUP, 0, MAKELPARAM((target.left + target.right) / 2, (target.top + target.bottom) / 2));
    if(visual_drag++==0)scenario_check(reference_present(state,1366,768,art_wave_capture_dir()+"/ui-placement-rejected.png"),
        "inventory: real rejected placement and reason captured");
    scenario_check(!state.primary_down && !state.held_gameplay_attacks.contains(VK_LBUTTON),
        "inventory: UI drag and release do not leave an attack latched");
  };
  sync_world(state); reconcile_pack_grid(state);
  const auto si = inventory_grid::find_index(state.pack_grid, pack_stable_id(spear));
  const auto spear_row = std::find_if(state.session->model().inventory.begin(), state.session->model().inventory.end(),
                                     [&](const auto& row) { return row.uuid == spear; });
  scenario_check(si < inventory_grid::kMaxItems && spear_row != state.session->model().inventory.end() &&
                 state.pack_grid.items[si].width == spear_row->width &&
                 state.pack_grid.items[si].height == spear_row->height && spear_row->height > 1,
                 "inventory: spear occupies its authoritative multi-cell footprint");
  scenario_check(!pack_can_land(state.pack_grid, pack_stable_id(spear), 11, 6),
                 "inventory: full footprint rejects bottom-right overflow");
  auto identity_snapshot=[&] {
    std::vector<std::string> all;
    for(const auto& row:state.session->model().inventory) all.push_back(row.uuid+":"+std::to_string(row.quantity));
    for(const auto& row:state.session->model().worn) all.push_back(row.item.uuid+":"+std::to_string(row.item.quantity));
    std::sort(all.begin(),all.end());return all;
  };
  const auto identities_before=identity_snapshot();
  select_inventory_index(state,state.world.carried.size());
  scenario_present(state);
  scenario_check(!render_list_has(state,render::Op::Hud,"inventory-action:Select an item"),"inventory: redundant equipment action strip is removed");
  scenario_check(std::none_of(state.world.carried.begin(),state.world.carried.end(),[](const auto& item){return item.name=="Coins";}),"inventory: currency is not draggable backpack content");
  state.character_pane=true;
  scenario_check(reference_present(state,1366,768,art_wave_capture_dir()+"/handover-before-equip.png"),"handover: actual server fixture before equipment");
  state.character_pane=false;
  const int attacks_before=state.combat_requests;
  const auto scene_before_ui=state.session->model().scene.id;
  const auto zoom_before_ui=state.camera.zoom;
  SendMessage(window,WM_KEYDOWN,'N',0);SendMessage(window,WM_KEYUP,'N',0);
  SendMessage(window,WM_KEYDOWN,'T',0);SendMessage(window,WM_KEYUP,'T',0);
  SendMessage(window,WM_MOUSEWHEEL,MAKEWPARAM(0,WHEEL_DELTA),0);
  chronicles_pump(state,20,[]{return false;});
  scenario_check(state.session->model().scene.id==scene_before_ui && !trade_pane_open(state) && state.camera.zoom==zoom_before_ui,"handover: open inventory owns route, talk and wheel input");
  drag_to_seat(wrap, paper_doll::Slot::MainHand);
  scenario_check(state.pack_last_drop == "reject" && !state.equip_view.pending,
                 "inventory: wrong seat rejects without sending a fake equip");
  for (const auto& pair : std::vector<std::pair<std::string, paper_doll::Slot>>{
       {wrap, paper_doll::Slot::Body}, {crest, paper_doll::Slot::Head},
       {feet, paper_doll::Slot::Boots}, {belt, paper_doll::Slot::Belt},
       {neck, paper_doll::Slot::Amulet}, {hands, paper_doll::Slot::Gloves},
       {ring, paper_doll::Slot::Ring1}, {ring2, paper_doll::Slot::Ring2}, {spear, paper_doll::Slot::MainHand}}) {
    drag_to_seat(pair.first, pair.second);
    scenario_check(pump([&] { return worn(pair.first, kDollSeats[paper_doll::slot_index(pair.second)]); }),
                   "inventory: drag equips exact requested seat on server");
    scenario_check(!state.equip_view.pending && state.equip_view.acknowledged_id == pair.first,
                   "inventory: server acknowledgement completes equip feedback");
    scenario_check(state.selected_item_id==pair.first && state.selected_item<state.world.carried.size() &&
        state.world.carried[state.selected_item].id==pair.first,"handover: selected UUID survives inventory-to-wear reordering");
    scenario_check(identity_snapshot()==identities_before,"handover: each equip conserves all item UUIDs and quantities");
  }
  sync_world(state); reconcile_pack_grid(state);
  scenario_check(inventory_grid::find_index(state.pack_grid, pack_stable_id(spear)) == inventory_grid::kMaxItems,
                 "inventory: worn weapon does not duplicate in backpack");
  // A raw invalid seat tests the authority, independently of the UI filter.
  submit_equip(state, shield, "head");
  scenario_check(pump([&] { return !state.equip_view.pending; }) && !worn(shield, "head"),
                 "inventory: server rejects wrong seat and clears pending state");
  drag_to_seat(shield, paper_doll::Slot::OffHand);
  scenario_check(pump([&] { return !state.equip_view.pending; }) && !worn(shield, "left_hand") && worn(spear, "right_hand"),
                 "inventory: two-handed conflict retains both owned items");
  // Drag the worn item directly into a chosen free footprint using the same
  // production window handlers as normal play; no Unequip button exists.
  const auto main_seat = make_pack_geom(1366, 768).seat;
  const auto at = MAKELPARAM((main_seat.left + main_seat.right) / 2, (main_seat.top + main_seat.bottom) / 2);
  SendMessage(window, WM_LBUTTONDOWN, 0, at);
  int destination=-1;
  for(int cell=verdigris::PlayerInventory::kSlotCount-1;cell>=0;--cell)
    if(pack_drag_can_land(state,cell%kPackColumns,cell/kPackColumns)){destination=cell;break;}
  scenario_check(destination>=0 && state.pack_drag_live,"inventory: worn seat begins a drag and has an exact backpack destination");
  const auto drag_geom=make_pack_geom(1366,768);
  const auto pack_at=MAKELPARAM(drag_geom.grid_left+(destination%kPackColumns)*drag_geom.cell_w+state.pack_grab_pixel_x,
      drag_geom.grid_top+(destination/kPackColumns)*drag_geom.cell_h+state.pack_grab_pixel_y);
  SendMessage(window,WM_MOUSEMOVE,MK_LBUTTON,pack_at);
  SendMessage(window,WM_LBUTTONUP,0,pack_at);
  scenario_check(state.equip_view.pending,"inventory: seat-to-backpack drag waits for authority");
  scenario_check(pump([&] { return !worn(spear, "right_hand"); }), "inventory: seat-to-backpack drag returns the exact item through server");
  const auto landed=std::find_if(state.session->model().inventory.begin(),state.session->model().inventory.end(),[&](const auto& item){return item.uuid==spear;});
  scenario_check(landed!=state.session->model().inventory.end() && landed->slot==destination,"inventory: server accepts the exact dragged destination");
  scenario_check(identity_snapshot()==identities_before,"inventory: seat-to-backpack conserves identities and quantities");
  drag_to_seat(shield, paper_doll::Slot::OffHand);
  scenario_check(pump([&] { return worn(shield, "left_hand"); }), "inventory: shield can use freed off hand");
  sync_world(state);reconcile_pack_grid(state);
  const auto axe_index=inventory_grid::find_index(state.pack_grid,pack_stable_id(axe));
  scenario_check(axe_index<inventory_grid::kMaxItems,"inventory: handstone is selectable in the backpack");
  if(axe_index<inventory_grid::kMaxItems) {
    const auto& item=state.pack_grid.items[axe_index];const auto g=make_pack_geom(1366,768);
    const auto pick=MAKELPARAM(g.grid_left+item.x*g.cell_w+g.cell_w/2,g.grid_top+item.y*g.cell_h+g.cell_h/2);
    SendMessage(window,WM_LBUTTONDOWN,0,pick);SendMessage(window,WM_LBUTTONUP,0,pick);
    drag_to_seat(axe,paper_doll::Slot::MainHand);
  }
  scenario_check(pump([&] { return worn(axe, "right_hand"); }), "inventory: one-handed weapon coexists with shield");
  scenario_check(!state.primary_down && !state.held_gameplay_attacks.contains(VK_LBUTTON),"inventory: drag equip consumes combat input");
  sync_world(state);
  scenario_check(state.combat_requests==attacks_before,"handover: equipment controls submit zero world attacks");
  scenario_check(state.session->model().player.total_ratings_present,"handover: server explicitly supplies displayed total ratings");
  state.character_pane=true;
  scenario_check(reference_present(state,1366,768,art_wave_capture_dir()+"/handover-after-equip.png"),"handover: actual equipment and server stats after equip");
  scenario_check(render_list_has(state,render::Op::Hud,"char:Attack rating:"+std::to_string(state.session->model().player.attack_rating)),"handover: displayed rating exactly matches server field");
  state.character_pane=false;
  scenario_check(state.world.player.combat_stats_present &&
                 state.world.player.gear_attack == state.session->model().player.gear_attack &&
                 state.world.player.gear_attack > 0 && state.world.player.defense > 0,
                 "inventory: gear stats reach presentation from authority");
  const auto dir = art_wave_capture_dir();
  for(const auto& row : state.session->model().worn)
    std::printf("    inventory art: %s -> %s (%s)\n",row.seat.c_str(),row.item.art_key.c_str(),row.item.name.c_str());
  state.hint_ticks=0;state.mouse={0,0};state.debug_overlay=false;
  for (const auto size : {std::pair{960, 600}, std::pair{1280, 800}, std::pair{1366, 768}, std::pair{3440, 1440}}) {
    state.camera.zoom = kCameraDefaultZoom * zoom_height_factor(size.second);
    scenario_check(reference_present(state, size.first, size.second,
        dir + "\\inventory-" + std::to_string(size.first) + ".png"), "inventory: actual production panel captured");
    const auto pane = gear_pane_rect(size.first, size.second);
    for (const auto& trace : state.hud_rect_trace) if (trace.first == "pane-cell")
      scenario_check(trace.second.x >= pane.x && trace.second.y >= pane.y &&
          trace.second.x + trace.second.w <= pane.x + pane.w &&
          trace.second.y + trace.second.h <= pane.y + pane.h, "inventory: complete item rectangle stays inside panel");
  }
  state.character_pane=true;
  state.camera.zoom=kCameraDefaultZoom*zoom_height_factor(800);
  scenario_check(reference_present(state,1280,800,dir+"/ui-equipment-and-stats.png"),"inventory: acknowledged equipment and authoritative stats captured together");
  const auto hover_rect=make_pack_geom(1280,800).seats[1];
  state.mouse={(hover_rect.left+hover_rect.right)/2,(hover_rect.top+hover_rect.bottom)/2};
  scenario_check(reference_present(state,1280,800,dir+"/ui-contextual-tooltip.png"),"inventory: equipment tooltip and compact character sheet captured together");
  const HudRect* tooltip=nullptr;const HudRect* sheet=nullptr;
  for(const auto& entry:state.hud_rect_trace) {
    if(entry.first=="compare-plate")tooltip=&entry.second;
    if(entry.first=="character-pane-frame")sheet=&entry.second;
  }
  scenario_check(tooltip && sheet && !hud_rects_overlap(*tooltip,*sheet),"inventory: contextual tooltip does not cover character values");
  state.character_pane=false;
  // Select/rearrange through the full footprint, then require authoritative coordinates.
  sync_world(state);reconcile_pack_grid(state);
  const auto move_id=spear;
  auto moving_index=inventory_grid::find_index(state.pack_grid,pack_stable_id(move_id));
  if(moving_index<inventory_grid::kMaxItems) {
    const auto item=state.pack_grid.items[moving_index];const auto g=make_pack_geom(1366,768);
    int tx=-1,ty=-1;
    for(int y=0;y<kPackRows && tx<0;++y) for(int x=0;x<kPackColumns;++x)
      if((x!=item.x || y!=item.y) && inventory_grid::can_place(state.pack_grid,x,y,item.width,item.height,pack_stable_id(move_id))) {tx=x;ty=y;break;}
    scenario_check(tx>=0,"handover: fixture has a real free destination");
    if(tx>=0) {
      SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM(g.grid_left+(item.x+item.width-1)*g.cell_w+g.cell_w/2,g.grid_top+(item.y+item.height-1)*g.cell_h+g.cell_h/2));
      SendMessage(window,WM_LBUTTONUP,0,MAKELPARAM(g.grid_left+(tx+item.width-1)*g.cell_w+g.cell_w/2,g.grid_top+(ty+item.height-1)*g.cell_h+g.cell_h/2));
      scenario_check(state.equip_view.pending,"handover: backpack move waits for server");
      scenario_check(pump([&]{return !state.equip_view.pending;}),"handover: backpack operation acknowledged");
      sync_world(state);reconcile_pack_grid(state);
      const auto i=inventory_grid::find_index(state.pack_grid,pack_stable_id(move_id));
      scenario_check(i<inventory_grid::kMaxItems && state.pack_grid.items[i].x==tx && state.pack_grid.items[i].y==ty,"handover: drawn backpack uses accepted server coordinates");
      scenario_check(identity_snapshot()==identities_before,"handover: rearrangement conserves every item and quantity");
    }
  }
  // Actual button controls and drag cancellation in the production HWND.
  const auto saved_count=state.session->model().inventory.size();
  const auto geom=make_pack_geom(1366,768);
  const auto first=state.pack_grid.items[0];
  const int pickx=geom.grid_left+first.x*geom.cell_w+geom.cell_w/2;
  const int picky=geom.grid_top+first.y*geom.cell_h+geom.cell_h/2;
  SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM(pickx,picky));
  SendMessage(window,WM_KEYDOWN,VK_ESCAPE,0);
  scenario_check(!state.pack_drag_live && state.gear_overlay,"handover: Escape cancels drag before closing another view");
  SendMessage(window,WM_LBUTTONUP,0,MAKELPARAM(-20,-20));
  SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM(pickx,picky));
  SendMessage(window,WM_KILLFOCUS,0,0);
  scenario_check(!state.pack_drag_live && state.session->model().inventory.size()==saved_count,"inventory: focus-loss cancellation conserves authoritative inventory");
  SendMessage(window,WM_LBUTTONUP,0,MAKELPARAM(pickx,picky));
  const auto close=gear_close_rect(1366,768);
  SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM((close.left+close.right)/2,(close.top+close.bottom)/2));
  SendMessage(window,WM_LBUTTONUP,0,0);
  scenario_check(!state.gear_overlay && !state.primary_down,"inventory: clickable Close consumes input and closes the pane");
  const auto open=player_menu_rect(1366,768,0);
  SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM((open.left+open.right)/2,(open.top+open.bottom)/2));
  SendMessage(window,WM_LBUTTONUP,0,0);
  scenario_check(state.gear_overlay && !state.primary_down,"inventory: Equipment button opens the same inventory without attacking");
  // A large worn spear cannot swap into the two cells freed by a sling.
  ClientCommand free_hand; free_hand.type = ClientCommand::Type::Unequip; free_hand.target = "left_hand";
  state.session->submit(free_hand);
  scenario_check(pump([&] { return !worn(shield, "left_hand"); }), "inventory: shield unequips independently");
  drag_to_seat(spear, paper_doll::Slot::MainHand);
  scenario_check(pump([&] { return worn(spear, "right_hand"); }), "inventory: swap restores larger spear");
  const auto sling = grant("vessel-sling");
  // Fill every remaining cell with 1x1 rings, then prove swaps and U cannot lose gear.
  grant("ring", 100);
  sync_world(state); reconcile_pack_grid(state);
  scenario_check(state.pack_grid.count == std::count_if(state.session->model().inventory.begin(),state.session->model().inventory.end(),[](const auto& item){return item.id!="coins";}),
                 "inventory: full backpack exposes every authoritative item");
  const auto before_count = state.session->model().inventory.size();
  const auto before_ground = state.session->model().ground.size();
  submit_equip(state, sling, "right_hand");
  scenario_check(pump([&] { return !state.equip_view.pending; }) && worn(spear, "right_hand") &&
                 state.session->model().inventory.size() == before_count &&
                 state.session->model().ground.size() == before_ground,
                 "inventory: full-pack swap rejects atomically without spilling displaced gear");
  const auto body=make_pack_geom(1366,768).seats[3];
  const auto body_at=MAKELPARAM((body.left+body.right)/2,(body.top+body.bottom)/2);
  SendMessage(window,WM_LBUTTONDOWN,0,body_at);
  SendMessage(window,WM_LBUTTONUP,0,body_at);
  SendMessage(window,WM_KEYDOWN,'U',0);SendMessage(window,WM_KEYUP,'U',0);
  scenario_check(state.equip_view.pending,"handover: keyboard unequip waits for server, including full-pack rejection");
  scenario_check(pump([&] { return state.session->model().last_message.find("Make room") != std::string::npos; }) &&
                 worn(wrap, "armor") && state.session->model().inventory.size() == before_count,
                 "inventory: full backpack rejects unequip without losing or spilling item");
  scenario_check(pump([&]{return !state.equip_view.pending;}),"handover: rejected Unequip re-enables action");
  state.character_pane=true;
  scenario_check(reference_present(state,1366,768,dir+"/handover-full-pack-rejection.png"),"handover: rejected operation and retained equipment captured");
  const auto persisted_items=identity_snapshot();
  std::vector<std::string> persisted_seats;
  for(const auto& row:state.session->model().worn) persisted_seats.push_back(row.seat+":"+row.item.uuid);
  std::vector<std::string> persisted_grid;
  for(const auto& row:state.session->model().inventory) persisted_grid.push_back(row.uuid+":"+std::to_string(row.slot));
  state.session->shutdown(); server->stop();server.reset();
  server=std::make_unique<verdigris::networking::WebSocketServer>(port,qa_saves);
  scenario_check(server->start(&error),"handover: fresh server reloads isolated persisted profile");
  state.session=std::make_unique<RemoteProtocolSession>("127.0.0.1",port,"inventory-qa",true);
  scenario_check(state.session->start(&error) && pump([&]{return state.session->connection_state()==ConnectionState::Ready;}),"handover: fresh client reconnects to reloaded profile");
  scenario_check(identity_snapshot()==persisted_items,"handover: restart retains all owned UUIDs and quantities");
  std::vector<std::string> restored_seats,restored_grid;
  for(const auto& row:state.session->model().worn) restored_seats.push_back(row.seat+":"+row.item.uuid);
  for(const auto& row:state.session->model().inventory) restored_grid.push_back(row.uuid+":"+std::to_string(row.slot));
  scenario_check(restored_seats==persisted_seats && restored_grid==persisted_grid,"handover: restart retains exact equipment seats and backpack positions");
  // Allocate real connected paths through production tree clicks. The dev
  // command supplies only the fixture's earned level, never an unlock flag.
  remote=static_cast<RemoteProtocolSession*>(state.session.get());
  remote->send_raw("dev:setlevel",JV::Object{{"level",100}});
  scenario_check(pump([&]{return state.session->model().player.level==100;}),"extensions: fixture has earned allocation budget");
  state.character_pane=false;state.gear_overlay=false;state.tree_pane=true;
  auto tree_click=[&](const std::string& id,bool allocate) {
    sync_world(state);scenario_present_size(state,1366,768);
    auto hit=std::find_if(state.tree_seat_hits.begin(),state.tree_seat_hits.end(),[&](const auto& row){return row.node_id==id;});
    if(hit==state.tree_seat_hits.end()){scenario_check(false,"extensions: next connected tree node is reachable in pane");return;}
    const int x=hit->x,y=hit->y;
    SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM(x,y));SendMessage(window,WM_LBUTTONUP,0,MAKELPARAM(x,y));
    if(allocate)scenario_check(pump([&]{const auto& nodes=state.session->model().progression.nodes;return std::find(nodes.begin(),nodes.end(),id)!=nodes.end();}),
        "extensions: production tree click receives authoritative allocation");
  };
  for(const auto axis:std::vector<std::pair<int,int>>{{-1,1},{0,-1},{1,0},{-1,0},{1,-1},{0,1}}) {
    for(int i=1;i<=10;++i)tree_click(std::to_string(axis.first*i)+","+std::to_string(axis.second*i),true);
    for(int i=9;i>=0;--i)tree_click(std::to_string(axis.first*i)+","+std::to_string(axis.second*i),false);
  }
  scenario_check(state.session->model().progression.inventory_unlocks.size()==6,"extensions: real tree paths unlock six drawers");
  state.tree_pane=false;state.gear_overlay=true;state.hint_ticks=0;
  state.character_pane=true;
  const auto extension_sheet=character_pane_rect(1366,768,0);
  scenario_check(!inventory_world_drop_allowed(state,1366,768,POINT{extension_sheet.x+10,extension_sheet.y+10}),"inventory: another visible pane rejects world drop");
  scenario_check(inventory_world_drop_allowed(state,1366,768,POINT{10,700}),"inventory: uncovered world remains a drop target with character pane open");
  state.character_pane=false;
  const int attacks_before_drawers=state.combat_requests;
  for(const auto size:{std::pair{960,600},std::pair{1280,800},std::pair{3440,1440}}) {
    SetWindowPos(window,nullptr,0,0,size.first,size.second,SWP_NOACTIVATE|SWP_NOZORDER);
    for(int index=0;index<6;++index) {
      const auto button=inventory_aux_button(size.first,size.second,index);
      const int bx=(button.left+button.right)/2,by=(button.top+button.bottom)/2;
      SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM(bx,by));SendMessage(window,WM_LBUTTONUP,0,MAKELPARAM(bx,by));
      scenario_check(state.inventory_aux==index,"extensions: left-edge button opens its own drawer");
      scenario_check(reference_present(state,size.first,size.second,dir+"/extension-"+std::to_string(index)+"-"+std::to_string(size.first)+".png"),
          "extensions: actual production drawer captured");
      const auto box=inventory_aux_rect(state,size.first,size.second);
      scenario_check(box.left>=0 && box.top>=0 && box.right<=size.first && box.bottom<=size.second,"extensions: drawer stays inside viewport");
      const auto bar=quickbar_strip_rect(size.first,size.second);
      scenario_check(box.right<=bar.x || box.left>=bar.x+bar.w || box.bottom<=bar.y || box.top>=bar.y+bar.h,
          "extensions: open drawer never covers action-bar controls");
      const auto geometry=inventory_aux_geom(state,size.first,size.second);
      scenario_check(index<3 ? geometry.columns==0 && geometry.seats[11+index].right>geometry.seats[11+index].left : geometry.columns==4 && geometry.rows==4,
          "extensions: equipment and 4x4 storage have distinct geometry");
      SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM(box.left+5,box.top+5));SendMessage(window,WM_LBUTTONUP,0,MAKELPARAM(box.left+5,box.top+5));
      SendMessage(window,WM_KEYDOWN,VK_ESCAPE,0);SendMessage(window,WM_KEYUP,VK_ESCAPE,0);
      scenario_check(state.gear_overlay && state.inventory_aux==-1,"extensions: Escape closes only the topmost drawer");
    }
  }
  scenario_check(state.combat_requests==attacks_before_drawers && !state.primary_down,"extensions: drawer and tab clicks do not attack through UI");
  SetWindowPos(window,nullptr,0,0,1366,768,SWP_NOACTIVATE|SWP_NOZORDER);
  state.session->shutdown();server->stop();server.reset();
  server=std::make_unique<verdigris::networking::WebSocketServer>(port,qa_saves);
  // Isolated category fixtures exercise the real drawer transport. They are
  // explicitly labeled QA objects, not new loot/content or owner-save grants.
  const auto fixture_file=qa_saves/"inventory-qa.json";
  JV fixture_save;
  {std::ifstream input(fixture_file);const std::string bytes((std::istreambuf_iterator<char>(input)),{});
    scenario_check(verdigris::networking::parse_json(bytes,fixture_save),"extensions: isolated fixture profile is readable");}
  if(fixture_save.object()) {
    const auto key=*fixture_save["activeHouseId"].string()+":"+*fixture_save["activeScionId"].string();
    auto& loadout=(*(*fixture_save.object())["scionLoadouts"].object())[key];
    JV::Array fixture_items;
    for(const auto& row:*loadout["inventory"].array())if(row["id"].string() && *row["id"].string()=="coins")fixture_items.push_back(row);
    const char* kinds[]={"warcall","quickrig","attendant","trophy","reagent","relic"};
    const char* seats[]={"warhorn","quick_rig","attendant","","",""};
    const char* names[]={"QA Warhorn","QA Quick Rig","QA Attendant","QA Trophy","QA Reagent","QA Relic"};
    for(int i=0;i<6;++i) {
      const int size=i<3?2:1;
      fixture_items.push_back(JV::Object{{"id","extension-qa"},{"uuid","extension-ui-"+std::to_string(i)},
        {"name",names[i]},{"displayName",names[i]},{"qty",1},{"packId","main"},{"slot",i<3?i*2:30+i},
        {"size",JV::Object{{"width",size},{"height",size}}},{"equipSlot",seats[i]},
        {"vessel",JV::Object{{"item",JV::Object{{"kind",kinds[i]},{"w",size},{"h",size}}}}}});
    }
    (*loadout.object())["inventory"]=std::move(fixture_items);
    std::ofstream output(fixture_file);output<<fixture_save.stringify();
  }
  scenario_check(server->start(&error),"extensions: server restarts with same QA profile");
  state.session=std::make_unique<RemoteProtocolSession>("127.0.0.1",port,"inventory-qa",true);
  scenario_check(state.session->start(&error) && pump([&]{return state.session->connection_state()==ConnectionState::Ready && state.session->model().progression.inventory_unlocks.size()==6;}),
      "extensions: fresh client reloads all six authoritative unlocks");
  auto refresh_inventory=[&]{sync_world(state);reconcile_pack_grid(state);};
  auto open_aux=[&](int index){
    if(state.inventory_aux==index)return;
    const auto r=inventory_aux_button(1366,768,index);const int x=(r.left+r.right)/2,y=(r.top+r.bottom)/2;
    SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM(x,y));SendMessage(window,WM_LBUTTONUP,0,MAKELPARAM(x,y));
  };
  auto row_for=[&](int index)->const ClientItemSlot* {
    const auto& inventory=state.session->model().inventory;
    const auto it=std::find_if(inventory.begin(),inventory.end(),[&](const auto& row){return row.uuid=="extension-ui-"+std::to_string(index);});
    return it==inventory.end()?nullptr:&*it;
  };
  auto point_for=[&](const ClientItemSlot& row){const auto g=row.pack_id=="main"?make_pack_geom(1366,768):inventory_aux_geom(state,1366,768);
    return POINT{g.grid_left+(row.slot%g.columns)*g.cell_w+g.cell_w/2,g.grid_top+(row.slot/g.columns)*g.cell_h+g.cell_h/2};};
  auto drag_between=[&](POINT from,POINT to){
    refresh_inventory();SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM(from.x,from.y));
    scenario_check(state.pack_drag_live,"extensions: full item footprint starts actual drag");
    SendMessage(window,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(to.x,to.y));
    SendMessage(window,WM_LBUTTONUP,0,MAKELPARAM(to.x,to.y));
    scenario_check(state.equip_view.pending,"extensions: drawer transfer waits for authority");
    scenario_check(pump([&]{return !state.equip_view.pending;}),"extensions: drawer transfer receives server acknowledgement");
  };
  for(int i=0;i<3;++i) {
    open_aux(i);refresh_inventory();const auto* row=row_for(i);scenario_check(row!=nullptr,"extensions: auxiliary equipment fixture is carried");
    if(!row)continue;
    const auto seat=inventory_aux_geom(state,1366,768).seats[11+i];const POINT target{(seat.left+seat.right)/2,(seat.top+seat.bottom)/2};
    drag_between(point_for(*row),target);
    scenario_check(worn("extension-ui-"+std::to_string(i),kDollSeats[11+i]),"extensions: actual drag equips auxiliary seat");
    const auto pack=make_pack_geom(1366,768);const int cell=60+i*2;
    drag_between(target,POINT{pack.grid_left+(cell%12+1)*pack.cell_w+pack.cell_w/2,pack.grid_top+(cell/12+1)*pack.cell_h+pack.cell_h/2});
    const auto* back=row_for(i);
    scenario_check(back && back->pack_id=="main","extensions: worn auxiliary item returns to backpack through drag");
    if(back)drag_between(point_for(*back),target);
  }
  for(int i=3;i<6;++i) {
    open_aux(i);refresh_inventory();const auto* row=row_for(i);scenario_check(row!=nullptr,"extensions: category fixture is carried");
    if(!row)continue;
    const auto pack=inventory_aux_geom(state,1366,768);
    drag_between(point_for(*row),POINT{pack.grid_left+pack.cell_w+pack.cell_w/2,pack.grid_top+pack.cell_h+pack.cell_h/2});
    const auto* placed=row_for(i);
    scenario_check(placed && placed->pack_id==pack.pack_id && placed->slot==5,"extensions: drag lands in exact compartment cell");
    if(placed) {
      const auto main=make_pack_geom(1366,768);
      drag_between(point_for(*placed),POINT{main.grid_left+main.cell_w/2,main.grid_top+6*main.cell_h+main.cell_h/2});
      const auto* returned=row_for(i);
      scenario_check(returned && returned->pack_id=="main" && returned->slot==72,"extensions: extra storage returns exact item to main backpack");
      if(returned)drag_between(point_for(*returned),POINT{pack.grid_left+pack.cell_w+pack.cell_w/2,pack.grid_top+pack.cell_h+pack.cell_h/2});
    }
    scenario_check(reference_present(state,1366,768,dir+"/extension-filled-"+std::to_string(i)+".png"),"extensions: labeled QA item in real compartment captured");
  }
  state.session->shutdown();server->stop();server.reset();
  server=std::make_unique<verdigris::networking::WebSocketServer>(port,qa_saves);
  scenario_check(server->start(&error),"extensions: populated compartments restart on same profile");
  state.session=std::make_unique<RemoteProtocolSession>("127.0.0.1",port,"inventory-qa",true);
  scenario_check(state.session->start(&error) && pump([&]{return state.session->connection_state()==ConnectionState::Ready;}),"extensions: populated profile reconnects");
  for(int i=0;i<3;++i)scenario_check(worn("extension-ui-"+std::to_string(i),kDollSeats[11+i]),"extensions: worn auxiliary item persists through process restart");
  for(int i=3;i<6;++i){const auto* row=row_for(i);scenario_check(row && row->pack_id==verdigris::inventory_extensions::definitions[i].pack && row->slot==5,"extensions: exact compartment UUID and cell persist through restart");}
  state.character_pane=true;
  // Sustained actual-window input + production paint. Every frame consumes a
  // burst of pointer events; the ghost must use the newest point without moving
  // authoritative items before release. This is not a static mockup/timing loop.
  state.character_pane=false;state.inventory_aux=-1;state.gear_keyboard_focus=false;
  SetWindowPos(window,nullptr,0,0,3440,1440,SWP_NOACTIVATE|SWP_NOZORDER);
  refresh_inventory();
  const auto performance_items=identity_snapshot();
  const auto main_geom=make_pack_geom(3440,1440);
  state.inventory_aux=0;
  // Drag the actual authored weapon artwork, not an unmapped QA extension.
  const auto worn_rect=main_geom.seats[2];
  const int start_x=(worn_rect.left+worn_rect.right)/2,start_y=(worn_rect.top+worn_rect.bottom)/2;
  SendMessage(window,WM_LBUTTONDOWN,0,MAKELPARAM(start_x,start_y));
  scenario_check(state.pack_drag_live,"drag-frame: equipped item begins a real sustained drag");
  double drag_total=0,drag_peak=0,drag_floor=0,drag_world=0,drag_hud=0;int measured=0;
  for(int frame=0;frame<45;++frame) {
    const int x=main_geom.grid_left+main_geom.cell_w*(2+frame%7),y=main_geom.grid_top+main_geom.cell_h*(1+frame%4);
    for(int event=0;event<32;++event)SendMessage(window,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(x-31+event,y));
    scenario_present_size(state,3440,1440);
    const auto ghost=std::find_if(state.hud_rect_trace.begin(),state.hud_rect_trace.end(),[](const auto& entry){return entry.first=="inventory-drag-ghost";});
    scenario_check(ghost!=state.hud_rect_trace.end() && ghost->second.x==x-state.pack_grab_pixel_x && ghost->second.y==y-state.pack_grab_pixel_y,
        "drag-frame: painted ghost follows the latest pointer with stable grab offset");
    if(frame>=5){drag_total+=state.last_paint_ms;drag_peak=std::max(drag_peak,state.last_paint_ms);
      drag_floor+=state.paint_ms_floor;drag_world+=state.paint_ms_world;drag_hud+=state.paint_ms_hud;++measured;}
  }
  std::printf("    drag-frame: %.3f ms average, %.3f ms peak; 40 measured 3440x1440 frames, 1440 pointer events\n",drag_total/std::max(1,measured),drag_peak);
  std::printf("    drag-frame sections: floor %.3f world %.3f hud %.3f ms\n",drag_floor/measured,drag_world/measured,drag_hud/measured);
  scenario_check(drag_total/std::max(1,measured)<40.0,"drag-frame: sustained inventory dragging stays under unchanged 40ms frame budget");
  scenario_check(identity_snapshot()==performance_items && state.combat_requests==attacks_before_drawers,"drag-frame: pointer bursts neither mutate inventory nor attack");
  scenario_check(reference_present(state,3440,1440,dir+"/inventory-sustained-drag.png"),"drag-frame: actual final dragged-item frame captured");
  SendMessage(window,WM_KEYDOWN,VK_ESCAPE,0);SendMessage(window,WM_KEYUP,VK_ESCAPE,0);
  scenario_check(!state.pack_drag_live && state.gear_overlay,"drag-frame: Escape cancels drag without closing inventory");
  SetWindowPos(window,nullptr,0,0,1366,768,SWP_NOACTIVATE|SWP_NOZORDER);
  state.inventory_aux=-1;state.character_pane=true;
  // Presentation stress only: freeze a copy of the real session snapshot. No
  // synthetic names or quantities are sent to authority or saved into its profile.
  struct ReadabilitySnapshot final : IClientSession {
    ClientModel value;std::string error;
    bool start(std::string*) override {return true;} void shutdown() override {}
    void submit(const ClientCommand&) override {} void poll() override {}
    ConnectionState connection_state() const override {return ConnectionState::Ready;}
    const ClientModel& model() const override {return value;}
    std::vector<PresentationEvent> drain_events() override {return {};}
    const std::string& last_error() const override {return error;}
  };
  auto stress=std::make_unique<ReadabilitySnapshot>();stress->value=state.session->model();
  state.session->shutdown();
  auto stress_item=std::find_if(stress->value.inventory.begin(),stress->value.inventory.end(),[](const auto& item){return item.id!="coins";});
  scenario_check(stress_item!=stress->value.inventory.end(),"inventory: readability fixture uses an actual backpack item");
  const auto stress_uuid=stress_item==stress->value.inventory.end()?std::string{}:stress_item->uuid;
  if(stress_item!=stress->value.inventory.end()) {
    stress_item->name="An exceptionally long inventory item name with its distinguishing final words still available in the tooltip";
    stress_item->quantity=12345678;
  }
  stress->value.attributes_present=true;
  stress->value.attr_strength=stress->value.attr_dexterity=stress->value.attr_intelligence=1000000;
  state.session=std::move(stress);state.hint_ticks=0;
  sync_world(state);reconcile_pack_grid(state);
  if(!state.world.carried.empty()) {
    const auto index=carried_index_for_pack_id(state,pack_stable_id(stress_uuid));
    if(index<state.world.carried.size()) {
      state.gear_keyboard_focus=true;select_inventory_index(state,index);
      for(const auto size:{std::pair{960,600},std::pair{1280,800},std::pair{3440,1440}}) {
        scenario_check(reference_present(state,size.first,size.second,dir+"/handover-long-name-"+std::to_string(size.first)+".png"),"handover: labeled long-name/count presentation stress captured");
        scenario_check(render_list_has(state,render::Op::Hud,"compare:An exceptionally long inventory item name"),"handover: stress capture really uses the full synthetic name");
        bool tooltip=false;
        for(const auto& trace:state.hud_rect_trace)if(trace.first=="compare-plate") {
          tooltip=true;const auto& r=trace.second;
          scenario_check(r.x>=0 && r.y>=0 && r.x+r.w<=size.first && r.y+r.h<=size.second,"handover: full-name tooltip remains within viewport");
        }
        scenario_check(tooltip,"handover: selected item tooltip stays keyboard-accessible");
      }
    }
  }
  SendMessage(window,WM_KEYDOWN,VK_ESCAPE,0);SendMessage(window,WM_KEYUP,VK_ESCAPE,0);
  scenario_present_size(state,1366,768);
  scenario_check(state.gear_overlay && state.inventory_aux==-1,"extensions: Escape keeps a keyboard-revealed drawer closed on subsequent paint");
  state.session->shutdown(); DestroyWindow(window); server->stop();
  return scenario_failures;
}
