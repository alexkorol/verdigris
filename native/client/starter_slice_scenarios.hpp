#pragma once

// This scenario uses the same socket, reducer and production painter as the
// packaged game. Developer verbs only position the inspection camera and clear
// encounters already covered by the real-combat authority tests.
int scenario_starter_slice() {
  using verdigris::client::ClientCommand;
  using verdigris::client::RemoteProtocolSession;
  std::unique_ptr<verdigris::networking::WebSocketServer> server;
  for (std::uint16_t port = 6780; port <= 6799; ++port) {
    auto probe = std::make_unique<verdigris::networking::WebSocketServer>(port);
    std::string error;
    if (probe->start(&error)) { server = std::move(probe); break; }
  }
  scenario_check(bool(server), "starter: loopback authority starts");
  if (!server) return scenario_failures;
  auto owner = make_product_client();
  auto& state = *owner;
  state.chronicles_mode = true;
  state.screen = Screen::Chronicles;
  load_billboards(state.billboards);
  state.session = std::make_unique<RemoteProtocolSession>("127.0.0.1", server->port(), "starter-slice-scenario", false);
  std::string error;
  scenario_check(state.session->start(&error), "starter: native session starts");
  const auto await = [&](const std::function<bool()>& predicate, const char* label) {
    const bool ready = chronicles_pump(state, 250, predicate);
    scenario_check(ready, label); return ready;
  };
  if (!await([&] { return state.session->model().chronicle.present; }, "starter: account roster arrives")) return scenario_failures;
  state.session->submit(ClientCommand::found_house("House of the First Stand"));
  if (!await([&] { return state.session->model().chronicle.houses.size() == 1; }, "starter: House is founded through real authority")) return scenario_failures;
  state.session->submit(ClientCommand::create_scion("Alda", "female"));
  if (!await([&] { return state.session->model().chronicle.houses.front().scions.size() == 1; }, "starter: female Scion is created")) return scenario_failures;
  const auto scion = state.session->model().chronicle.houses.front().scions.front().id;
  state.session->submit(ClientCommand::set_out(scion, true));
  if (!await([&] { return state.session->model().starter.phase == "occupation" && state.screen == Screen::Expedition; },
             "starter: normal admission reaches village occupation")) return scenario_failures;
  scenario_check(state.session->model().player.scene_id == "owner-demo-prologue", "starter: scene comes from village authority");
  scenario_check(state.session->model().player.appearance == "female", "starter: admission preserves the selected character");
  send_dev_envelope(state, "dev:state", {{"requestId", "starter-map"}, {"includeMap", true}});
  if (!await([&] { return state.session->model().map_scene_id == "owner-demo-prologue" && !state.session->model().map_walkable.empty(); },
             "starter: collision map belongs to current village")) return scenario_failures;
  const auto walkable = [&](int x, int y) {
    const auto& model = state.session->model();
    return x >= 0 && y >= 0 && x < model.map_width && y < model.map_height &&
        model.map_walkable[std::size_t(y * model.map_width + x)] != 0;
  };
  scenario_check(!walkable(7,15) && !walkable(25,15) && !walkable(18,18) &&
      walkable(16,22) && walkable(16,20) && walkable(16,8),
      "starter: houses and well block their footprints while interaction and passage tiles remain open");
  sync_world(state); generate_scenery(state); scenario_follow_camera(state);
  scenario_check(std::none_of(state.scenery.begin(),state.scenery.end(),[](const auto& item){return item.kind == SceneryKind::Gate;}),
      "starter: village uses an ordinary passage without the old portal");
  const auto prop_at = [&](const char* id, double x, double y) {
    return std::any_of(state.scenery.begin(),state.scenery.end(),[&](const auto& item) {
      return item.art_identity == id && std::abs(item.position.x-x*kTileUnits) < 1 && std::abs(item.position.y-y*kTileUnits) < 1;
    });
  };
  scenario_check(prop_at("village-longhouse",7,17) && prop_at("village-longhouse",25,17) && prop_at("village-well",18.5,19.5),
      "starter: authored landmarks align with server collision footprints");
  std::string captures;
  const int override_status = capture_root_override(&captures);
  if (override_status < 0) { scenario_check(false,"starter: capture root is valid"); return scenario_failures; }
  if (override_status == 0) { captures = executable_directory()+"\\starter-slice-captures"; CreateDirectoryA(captures.c_str(),nullptr); }
  scenario_check(reference_present(state,1280,800,captures+"\\starter-occupation-1280x800.png"), "starter: occupation captured through production painter");
  scenario_check(state.camera.perspective && fable_world::renderer().gpu.error().empty(), "starter: capture retains production GPU renderer");
  const auto has_action = [&](const char* action) {
    return std::count_if(state.starter_hits.begin(),state.starter_hits.end(),[&](const auto& hit) { return hit.action == action; }) == 1;
  };
  scenario_check(state.starter_hits.size() == 3 && has_action("field_hand") && has_action("scout") && has_action("scribe"),
      "starter: painted occupation choices have exactly three actionable hit targets");
  for (const auto& hit : state.starter_hits)
    scenario_check(hit.rect.left >= 0 && hit.rect.top >= 0 && hit.rect.right <= 1280 && hit.rect.bottom <= 800,
        "starter: occupation control fits the native viewport");
  state.session->submit(ClientCommand::starter_action("scout"));
  if (!await([&] { return state.session->model().starter.phase == "tool"; }, "starter: typed occupation command advances authority")) return scenario_failures;
  scenario_present_size(state,1280,800);
  scenario_check(state.session->model().starter.occupation == "scout" && state.starter_hits.size() == 1 && has_action("interact"),
      "starter: selected occupation is retained and tool interaction is visible");
  state.session->submit(ClientCommand::extract());
  if (!await([&] { return state.session->model().starter.phase == "wave" && state.session->model().monsters.size() == 2; },
             "starter: general interact routes to the first authoritative pack instead of exit stairs")) return scenario_failures;
  send_dev_envelope(state,"dev:teleport",{{"x",16},{"y",19}});
  if (!await([&] { return std::abs(state.session->model().player.y-19)<.01; }, "starter: encounter inspection reaches well approach")) return scenario_failures;
  scenario_follow_camera(state);
  scenario_check(reference_present(state,1280,800,captures+"\\starter-wave-1280x800.png"), "starter: first pack captured through production painter");
  scenario_check(state.starter_hits.empty(), "starter: active combat has no stale interaction or occupation hit targets");
  for (int wave=1; wave<=3; ++wave) {
    send_dev_envelope(state,"dev:clear-floor");
    const std::string expected = wave == 3 ? "victory" : "rally";
    if (!await([&] { return state.session->model().starter.phase == expected; }, "starter: authority advances cleared encounter")) return scenario_failures;
    scenario_present_size(state,1280,800);
    scenario_check(state.starter_hits.size()==1 && has_action("interact"), "starter: rally or victory exposes one interaction target");
    if (wave < 3) {
      state.session->submit(ClientCommand::starter_action("interact"));
      if (!await([&] { return state.session->model().starter.phase == "wave" && state.session->model().starter.wave == wave+1; },
                 "starter: regroup starts the next server-owned encounter")) return scenario_failures;
    }
  }
  scenario_check(state.session->model().player.level == 2, "starter: client receives earned first level");
  scenario_check(state.session->model().progression.present && state.session->model().progression.unspent_points == 1,
      "starter: victory immediately exposes the first spendable skill point");
  send_dev_envelope(state,"dev:teleport",{{"x",16},{"y",8}});
  if (!await([&] { return std::abs(state.session->model().player.y-8)<.01; }, "starter: defended passage is reachable")) return scenario_failures;
  state.session->submit(ClientCommand::starter_action("interact"));
  if (!await([&] { return state.session->model().starter.phase == "departed" && state.session->model().scene.type == "town"; },
             "starter: victory passage transitions to Crossroads")) return scenario_failures;
  scenario_present_size(state,1280,800);
  scenario_check(!state.session->model().starter.active && state.starter_hits.empty(), "starter: departure removes village controls");
  state.session->shutdown(); server->stop();
  return scenario_failures;
}
