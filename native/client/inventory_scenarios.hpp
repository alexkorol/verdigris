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
  for (unsigned short p = 6780; p < 6800; ++p) {
    auto probe = std::make_unique<verdigris::networking::WebSocketServer>(p);
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
    const int x = geom.grid_left + item.x * (geom.cell_w + geom.gap) + geom.cell_w / 2;
    const int y = geom.grid_top + item.y * (geom.cell_h + geom.gap) + geom.cell_h / 2;
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
  // Click an equipped seat, then the shipped U key returns that exact item.
  const auto main_seat = make_pack_geom(1366, 768).seat;
  const auto at = MAKELPARAM((main_seat.left + main_seat.right) / 2, (main_seat.top + main_seat.bottom) / 2);
  SendMessage(window, WM_LBUTTONDOWN, 0, at); SendMessage(window, WM_LBUTTONUP, 0, at);
  const auto unequip_button=gear_action_rect(1366,768,0);
  const auto button_at=MAKELPARAM((unequip_button.left+unequip_button.right)/2,(unequip_button.top+unequip_button.bottom)/2);
  SendMessage(window,WM_LBUTTONDOWN,0,button_at);SendMessage(window,WM_LBUTTONUP,0,button_at);
  scenario_check(pump([&] { return !worn(spear, "right_hand"); }), "inventory: visible Unequip button returns the selected item through server");
  drag_to_seat(shield, paper_doll::Slot::OffHand);
  scenario_check(pump([&] { return worn(shield, "left_hand"); }), "inventory: shield can use freed off hand");
  sync_world(state);reconcile_pack_grid(state);
  const auto axe_index=inventory_grid::find_index(state.pack_grid,pack_stable_id(axe));
  scenario_check(axe_index<inventory_grid::kMaxItems,"inventory: handstone is selectable in the backpack");
  if(axe_index<inventory_grid::kMaxItems) {
    const auto& item=state.pack_grid.items[axe_index];const auto g=make_pack_geom(1366,768);
    const auto pick=MAKELPARAM(g.grid_left+item.x*g.cell_w+g.cell_w/2,g.grid_top+item.y*g.cell_h+g.cell_h/2);
    SendMessage(window,WM_LBUTTONDOWN,0,pick);SendMessage(window,WM_LBUTTONUP,0,pick);
    SendMessage(window,WM_LBUTTONDOWN,0,button_at);SendMessage(window,WM_LBUTTONUP,0,button_at);
  }
  scenario_check(pump([&] { return worn(axe, "right_hand"); }), "inventory: one-handed weapon coexists with shield");
  scenario_check(!state.primary_down && !state.held_gameplay_attacks.contains(VK_LBUTTON),"inventory: visible Equip button consumes combat input");
  sync_world(state);
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
  // Actual button controls and drag cancellation in the production HWND.
  const auto saved_count=state.session->model().inventory.size();
  const auto geom=make_pack_geom(1366,768);
  const auto first=state.pack_grid.items[0];
  const int pickx=geom.grid_left+first.x*geom.cell_w+geom.cell_w/2;
  const int picky=geom.grid_top+first.y*geom.cell_h+geom.cell_h/2;
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
  scenario_check(state.pack_grid.count == state.session->model().inventory.size(),
                 "inventory: full backpack exposes every authoritative item");
  const auto before_count = state.session->model().inventory.size();
  const auto before_ground = state.session->model().ground.size();
  submit_equip(state, sling, "right_hand");
  scenario_check(pump([&] { return !state.equip_view.pending; }) && worn(spear, "right_hand") &&
                 state.session->model().inventory.size() == before_count &&
                 state.session->model().ground.size() == before_ground,
                 "inventory: full-pack swap rejects atomically without spilling displaced gear");
  ClientCommand unequip; unequip.type = ClientCommand::Type::Unequip; unequip.target = "armor";
  state.session->submit(unequip);
  scenario_check(pump([&] { return state.session->model().last_message.find("Make room") != std::string::npos; }) &&
                 worn(wrap, "armor") && state.session->model().inventory.size() == before_count,
                 "inventory: full backpack rejects unequip without losing or spilling item");
  state.session->shutdown(); DestroyWindow(window); server->stop();
  return scenario_failures;
}
