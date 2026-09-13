#pragma once

// Real Win32 input handlers and authoritative socket session, with disposable
// identities. This can be run from the packaged client, not just a test build.
int scenario_consolidated_flow() {
  std::unique_ptr<verdigris::networking::WebSocketServer> server;
  unsigned short port = 0;
  for (unsigned short candidate = 6780; candidate < 6800; ++candidate) {
    auto probe = std::make_unique<verdigris::networking::WebSocketServer>(candidate);
    std::string error;
    if (probe->start(&error)) { port = candidate; server = std::move(probe); break; }
  }
  scenario_check(server != nullptr, "consolidation: isolated native server starts");
  if (!server) return scenario_failures;
  ClientState state;
  state.chronicles_mode = true;
  state.camera.perspective = true;
  state.lineage_art = true;
  state.screen = Screen::Chronicles;
  state.frontend = Frontend::Title;
  load_billboards(state.billboards);
  state.session = std::make_unique<verdigris::client::RemoteProtocolSession>(
      "127.0.0.1", port, "consolidation-flow", false);
  std::string error;
  scenario_check(state.session->start(&error), "consolidation: session connects");
  scenario_check(chronicles_pump(state, 250, [&] { return state.session->model().chronicle.present; }),
                 "consolidation: authoritative roster arrives");
  WNDCLASSA wc{};
  wc.hInstance = GetModuleHandle(nullptr);
  wc.lpfnWndProc = window_proc;
  wc.lpszClassName = "VerdigrisConsolidationTest";
  RegisterClassA(&wc);
  HWND window = CreateWindowExA(0, wc.lpszClassName, "Consolidation flow", WS_POPUP,
                                0, 0, 960, 600, nullptr, nullptr, wc.hInstance, &state);
  auto click_rect = [&](RECT box) {
    auto at = MAKELPARAM((box.left + box.right) / 2, (box.top + box.bottom) / 2);
    SendMessage(window, WM_LBUTTONDOWN, 0, at);
    SendMessage(window, WM_LBUTTONUP, 0, at);
  };
  auto menu = [&](std::size_t row) {
    scenario_present(state);
    for (const auto& hit : state.menu_hits) if (hit.index == row && hit.direction == 0) {
      const RECT box = hit.rect; click_rect(box); return;
    }
    scenario_check(false, "consolidation: title button exists");
  };
  auto card = [&](const std::string& command, const std::string& arg = "") {
    scenario_present(state);
    for (const auto& hit : state.chronicle_hits)
      if (hit.action.command == command && (arg.empty() || hit.action.arg == arg)) {
        const RECT box = hit.rect; click_rect(box); return;
      }
    scenario_check(false, ("consolidation: button exists " + command).c_str());
  };
  auto type = [&](const char* value) {
    for (const char* c = value; *c; ++c) {
      SendMessage(window, WM_KEYDOWN, static_cast<WPARAM>(std::toupper(*c)), 0);
      SendMessage(window, WM_CHAR, *c, 0);
      SendMessage(window, WM_KEYUP, static_cast<WPARAM>(std::toupper(*c)), 0);
    }
  };
  menu(0);
  card("house-name"); type("Cedar Kin"); card("found-house");
  scenario_check(chronicles_pump(state, 250, [&] { return !state.session->model().chronicle.houses.empty(); }),
                 "consolidation: House button creates named House");
  if (state.session->model().chronicle.houses.empty()) {
    DestroyWindow(window); state.session->shutdown(); server->stop(); return scenario_failures;
  }
  scenario_check(state.session->model().chronicle.houses.front().name == "Cedar Kin",
                 "consolidation: name field submits exact text without hotkey side effects");
  card("scion-name"); type("Mira"); card("appearance", "female"); card("create-scion");
  scenario_check(chronicles_pump(state, 250, [&] { return !state.session->model().chronicle.houses.front().scions.empty(); }),
                 "consolidation: Scion button creates named character");
  const auto& scions = state.session->model().chronicle.houses.front().scions;
  if (scions.empty()) { DestroyWindow(window); state.session->shutdown(); server->stop(); return scenario_failures; }
  const std::string scion_id = scions.front().id;
  scenario_check(scions.front().name == "Mira" && scions.front().appearance == "female",
                 "consolidation: name and appearance reach authoritative roster");
  card("set-out");
  scenario_check(chronicles_pump(state, 250, [&] { return state.screen == Screen::Expedition; }),
                 "consolidation: clicked Scion reaches normal play");
  scenario_follow_camera(state); scenario_present(state);
  scenario_check(state.camera.perspective && fable_world::renderer().gpu.stats().hardware &&
                 render_list_has(state, render::Op::Player, "hero_female_"),
                 "consolidation: same flow reaches hardware perspective and selected animated actor");
  SendMessage(window, WM_KEYDOWN, VK_ESCAPE, 0);
  menu(2);
  scenario_check(state.frontend == Frontend::Title, "consolidation: pause button returns to title");
  menu(3);
  scenario_check(state.screen == Screen::Chronicles && state.manage_lineage,
                 "consolidation: title exposes character management after admission");
  card("back-title"); menu(0);
  scenario_check(state.screen == Screen::Expedition && state.frontend == Frontend::None &&
                 state.session->model().chronicle.active_scion_id == scion_id,
                 "consolidation: Continue resumes living Scion without old prompts");
  // A fresh client connection must also Continue without repeating selection.
  state.session->shutdown();
  state.session = std::make_unique<verdigris::client::RemoteProtocolSession>(
      "127.0.0.1", port, "consolidation-flow", false);
  state.session->start(&error); state.frontend = Frontend::Title;
  scenario_check(chronicles_pump(state, 250, [&] { return state.session->model().chronicle.present; }),
                 "consolidation: returning connection loads roster");
  menu(0);
  scenario_check(chronicles_pump(state, 250, [&] { return state.screen == Screen::Expedition; }),
                 "consolidation: returning Continue admits saved Scion directly");
  state.session->shutdown(); DestroyWindow(window); server->stop();
  return scenario_failures;
}
