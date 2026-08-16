#include <iostream>
#include <memory>
#include <string>
#include <cstring>

#include "verdigris/core.hpp"
#include "verdigris/seasonal.hpp"

#ifdef VERDIGRIS_NATIVE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
struct ClientState {
  std::unique_ptr<verdigris::Simulation> simulation;
  bool w = false;
  bool a = false;
  bool s = false;
  bool d = false;
  POINT mouse{0, 0};
};

ClientState* state_from(HWND window) {
  return reinterpret_cast<ClientState*>(GetWindowLongPtr(window, GWLP_USERDATA));
}

void status_text(const ClientState& state, char* buffer, std::size_t size) {
  const auto& sim = *state.simulation;
  const auto* player = sim.actor(sim.scion().actor_id);
  const int life = player ? player->stats.life : 0;
  const std::string line =
      "House " + sim.house().name + " | Scion " + sim.scion().name +
      " | Life " + std::to_string(life) + " | Stored trophies " +
      std::to_string(sim.house().stored_trophies.size()) + " | Stored items " +
      std::to_string(sim.house().stored_items.size());
  strncpy_s(buffer, size, line.c_str(), _TRUNCATE);
}

void paint(HWND window, HDC dc) {
  ClientState* state = state_from(window);
  if (!state) return;
  RECT bounds;
  GetClientRect(window, &bounds);
  HBRUSH background = CreateSolidBrush(RGB(23, 29, 32));
  FillRect(dc, &bounds, background);
  DeleteObject(background);

  const auto& sim = *state->simulation;
  const auto* player = sim.actor(sim.scion().actor_id);
  if (player) {
    const int center_x = bounds.right / 2;
    const int player_x = center_x + player->position.x / 10;
    const int player_y = bounds.bottom / 2;
    HBRUSH player_brush = CreateSolidBrush(RGB(104, 189, 154));
    HBRUSH enemy_brush = CreateSolidBrush(RGB(191, 91, 76));
    HBRUSH item_brush = CreateSolidBrush(RGB(230, 181, 74));
    HBRUSH extraction_brush = CreateSolidBrush(RGB(88, 132, 190));
    SelectObject(dc, player_brush);
    Ellipse(dc, player_x - 14, player_y - 14, player_x + 14, player_y + 14);
    SelectObject(dc, enemy_brush);
    for (const auto& actor : sim.actors()) {
      if (actor.kind == verdigris::ActorKind::Monster && actor.alive) {
        const int x = center_x + actor.position.x / 10;
        Ellipse(dc, x - 14, player_y - 14, x + 14, player_y + 14);
      }
    }
    SelectObject(dc, item_brush);
    for (std::size_t i = 0; i < sim.ground_items().size(); ++i) {
      Rectangle(dc, center_x + 30 + static_cast<int>(i) * 24, player_y - 10,
                center_x + 46 + static_cast<int>(i) * 24, player_y + 6);
    }
    SelectObject(dc, extraction_brush);
    Rectangle(dc, center_x - 8, player_y + 50, center_x + 8, player_y + 66);
    DeleteObject(player_brush);
    DeleteObject(enemy_brush);
    DeleteObject(item_brush);
    DeleteObject(extraction_brush);
  }

  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, RGB(230, 235, 220));
  char status[512]{};
  status_text(*state, status, sizeof(status));
  TextOutA(dc, 18, 16, status, static_cast<int>(strlen(status)));
  const char* help = "WASD move | LMB melee | RMB/Space dash | P pickup | E equip | X extract";
  TextOutA(dc, 18, 40, help, static_cast<int>(strlen(help)));
}

void timer_step(ClientState& state) {
  int dx = (state.d ? 1 : 0) - (state.a ? 1 : 0);
  int dy = (state.s ? 1 : 0) - (state.w ? 1 : 0);
  if (dx != 0 || dy != 0) state.simulation->dispatch(verdigris::Command::move(dx, dy));
  else state.simulation->dispatch(verdigris::Command::action_use(verdigris::ActionType::Wait));
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  ClientState* state = state_from(window);
  switch (message) {
    case WM_NCCREATE: {
      auto* create = reinterpret_cast<CREATESTRUCT*>(lparam);
      SetWindowLongPtr(window, GWLP_USERDATA,
                       reinterpret_cast<LONG_PTR>(create->lpCreateParams));
      return TRUE;
    }
    case WM_KEYDOWN:
      if (!state) break;
      if (wparam == 'W') state->w = true;
      if (wparam == 'A') state->a = true;
      if (wparam == 'S') state->s = true;
      if (wparam == 'D') state->d = true;
      if (wparam == VK_SPACE)
        state->simulation->dispatch(verdigris::Command::action_use(verdigris::ActionType::Dash));
      if (wparam == 'P') {
        if (!state->simulation->ground_items().empty())
          state->simulation->dispatch(verdigris::Command::pick_up(
              state->simulation->ground_items().front().id));
        else if (!state->simulation->ground_trophies().empty())
          state->simulation->dispatch(verdigris::Command::pick_up(
              state->simulation->ground_trophies().front().id));
      }
      if (wparam == 'E' && !state->simulation->scion().carried_items.empty())
        state->simulation->dispatch(verdigris::Command::equip(
            state->simulation->scion().carried_items.front().id));
      if (wparam == 'X') state->simulation->dispatch(verdigris::Command::extract());
      InvalidateRect(window, nullptr, FALSE);
      break;
    case WM_KEYUP:
      if (!state) break;
      if (wparam == 'W') state->w = false;
      if (wparam == 'A') state->a = false;
      if (wparam == 'S') state->s = false;
      if (wparam == 'D') state->d = false;
      break;
    case WM_LBUTTONDOWN:
      if (state) state->simulation->dispatch(
          verdigris::Command::action_use(verdigris::ActionType::Melee));
      InvalidateRect(window, nullptr, FALSE);
      break;
    case WM_RBUTTONDOWN:
      if (state) state->simulation->dispatch(
          verdigris::Command::action_use(verdigris::ActionType::Dash));
      InvalidateRect(window, nullptr, FALSE);
      break;
    case WM_TIMER:
      if (state) {
        timer_step(*state);
        InvalidateRect(window, nullptr, FALSE);
      }
      break;
    case WM_PAINT: {
      PAINTSTRUCT paint_struct;
      HDC dc = BeginPaint(window, &paint_struct);
      paint(window, dc);
      EndPaint(window, &paint_struct);
      return 0;
    }
    case WM_DESTROY:
      KillTimer(window, 1);
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(window, message, wparam, lparam);
}
}  // namespace

int run_headless_demo() {
  verdigris::Simulation simulation(0xC011AB1EULL, "House Verdigris");
  verdigris::EmberHunt seasonal;
  simulation.set_seasonal_mechanic(&seasonal);
  simulation.dispatch(verdigris::Command::enter("route:tin:1:0"));
  for (int i = 0; i < 4; ++i) simulation.dispatch(verdigris::Command::move(1, 0));
  for (int i = 0; i < 8; ++i)
    simulation.dispatch(verdigris::Command::action_use(verdigris::ActionType::Melee));
  if (!simulation.ground_items().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_items().front().id));
  if (!simulation.ground_trophies().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_trophies().front().id));
  for (int i = 0; i < 4; ++i) simulation.dispatch(verdigris::Command::move(-1, 0));
  simulation.dispatch(verdigris::Command::extract());
  std::cout << "Verdigris native client shell\n"
            << "House: " << simulation.house().name
            << " | trophies stored: " << simulation.house().stored_trophies.size()
            << " | items stored: " << simulation.house().stored_items.size() << "\n";
  return 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR command_line, int show) {
  if (command_line && std::strstr(command_line, "--headless")) return run_headless_demo();
  auto state = std::make_unique<ClientState>();
  state->simulation = std::make_unique<verdigris::Simulation>(0xC011AB1EULL, "House Verdigris");
  verdigris::EmberHunt seasonal;
  state->simulation->set_seasonal_mechanic(&seasonal);
  state->simulation->dispatch(verdigris::Command::enter("route:tin:1:0"));

  WNDCLASSA window_class{};
  window_class.hInstance = instance;
  window_class.lpfnWndProc = window_proc;
  window_class.lpszClassName = "VerdigrisNativeClient";
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassA(&window_class);

  HWND window = CreateWindowExA(0, window_class.lpszClassName, "Verdigris - Native Expedition",
                                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 960, 600,
                                nullptr, nullptr, instance, state.get());
  ShowWindow(window, show);
  SetTimer(window, 1, 50, nullptr);

  MSG message{};
  while (GetMessage(&message, nullptr, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  return static_cast<int>(message.wParam);
}
#else
int run_headless_demo() {
  verdigris::Simulation simulation(0xC011AB1EULL, "House Verdigris");
  verdigris::EmberHunt seasonal;
  simulation.set_seasonal_mechanic(&seasonal);
  simulation.dispatch(verdigris::Command::enter("route:tin:1:0"));
  for (int i = 0; i < 4; ++i) simulation.dispatch(verdigris::Command::move(1, 0));
  for (int i = 0; i < 8; ++i)
    simulation.dispatch(verdigris::Command::action_use(verdigris::ActionType::Melee));
  if (!simulation.ground_items().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_items().front().id));
  if (!simulation.ground_trophies().empty())
    simulation.dispatch(verdigris::Command::pick_up(simulation.ground_trophies().front().id));
  for (int i = 0; i < 4; ++i) simulation.dispatch(verdigris::Command::move(-1, 0));
  simulation.dispatch(verdigris::Command::extract());
  std::cout << "Verdigris native client shell\n"
            << "House: " << simulation.house().name
            << " | trophies stored: " << simulation.house().stored_trophies.size()
            << " | items stored: " << simulation.house().stored_items.size() << "\n";
  return 0;
}

int main() {
  return run_headless_demo();
}
#endif
