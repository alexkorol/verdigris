#include "remote_play.hpp"

#include "camera2d.hpp"
#include "remote_session.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#ifdef VERDIGRIS_NATIVE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

namespace {

constexpr double kZoom = 28.0;

struct Spark {
  double x = 0.0;
  double y = 0.0;
  int ttl = 12;
  int value = 0;
  bool incoming = false;
  bool telegraph = false;
};

struct RemotePlay {
  std::unique_ptr<verdigris::client::RemoteProtocolSession> session;
  bool w = false;
  bool a = false;
  bool s = false;
  bool d = false;
  std::vector<Spark> sparks;
  std::vector<std::string> log;
  std::string hint;
  int pulse = 0;
};

void push_log(RemotePlay& play, const std::string& line) {
  play.log.push_back(line);
  if (play.log.size() > 10) play.log.erase(play.log.begin());
}

void handle_events(RemotePlay& play) {
  using T = verdigris::client::PresentationEventType;
  const auto& model = play.session->model();
  for (const auto& event : play.session->drain_events()) {
    Spark spark;
    spark.x = model.player.x;
    spark.y = model.player.y;
    spark.value = event.value;
    switch (event.type) {
      case T::ConnectionEstablished:
        push_log(play, "Connected");
        break;
      case T::SessionReady:
        push_log(play, "Guest login accepted");
        play.hint = "E enter route  |  WASD move  |  click/space fight  |  X take  |  1-9 equip  |  walk stairs to extract";
        break;
      case T::ConnectionLost:
        push_log(play, "DISCONNECTED: " + event.text);
        play.hint = event.text;
        play.pulse = 16;
        break;
      case T::AttackStarted:
        spark.ttl = 8;
        play.sparks.push_back(spark);
        break;
      case T::DamageApplied:
        spark.incoming = event.text == "incoming";
        spark.ttl = 10;
        play.sparks.push_back(spark);
        push_log(play, (spark.incoming ? "Taken " : "Hit ") + std::to_string(event.value));
        break;
      case T::Telegraph:
        spark.telegraph = true;
        spark.ttl = 18;
        play.sparks.push_back(spark);
        play.pulse = 12;
        push_log(play, "TELEGRAPH " + event.text);
        break;
      case T::ActorDied:
        spark.ttl = 14;
        play.sparks.push_back(spark);
        push_log(play, "Kill: " + event.text);
        break;
      case T::ItemDropped:
        spark.ttl = 16;
        play.sparks.push_back(spark);
        push_log(play, "Loot at your feet");
        break;
      case T::ItemPickedUp:
        push_log(play, "Picked up " + event.text);
        break;
      case T::ItemEquipped:
        push_log(play, "Equipped " + event.text +
                           (event.value > 0 ? ("  +" + std::to_string(event.value) + " attack") : ""));
        break;
      case T::ExtractionCompleted:
        push_log(play, "Returned to the surface — inventory kept");
        play.hint = "Banked on the surface. Esc to quit.";
        break;
      case T::Message:
        push_log(play, event.text);
        break;
      case T::ScionDied:
        push_log(play, "You died");
        play.hint = "You died. Esc to quit — this is not offline play.";
        break;
      default:
        break;
    }
  }
}

void paint(HWND window, RemotePlay& play) {
  RECT client{};
  GetClientRect(window, &client);
  const int width = client.right - client.left;
  const int height = client.bottom - client.top;
  HDC screen = GetDC(window);
  HDC dc = CreateCompatibleDC(screen);
  HBITMAP bitmap = CreateCompatibleBitmap(screen, width, height);
  HGDIOBJ old = SelectObject(dc, bitmap);

  HBRUSH bg = CreateSolidBrush(RGB(12, 18, 16));
  FillRect(dc, &client, bg);
  DeleteObject(bg);

  play.session->poll();
  handle_events(play);
  const auto& model = play.session->model();
  const auto state = play.session->connection_state();

  camera2d::Camera camera;
  camera.x = model.player.x;
  camera.y = model.player.y;
  camera.zoom = kZoom;
  camera2d::Screen screen_size{width, height};

  auto at = [&](double wx, double wy) {
    return camera2d::project(camera, screen_size, wx, wy);
  };

  // Ground grid around the scion.
  const int gx0 = static_cast<int>(model.player.x) - 12;
  const int gy0 = static_cast<int>(model.player.y) - 8;
  for (int gy = gy0; gy <= gy0 + 16; ++gy) {
    for (int gx = gx0; gx <= gx0 + 24; ++gx) {
      const auto p = at(gx, gy);
      RECT tile{p.x - 12, p.y - 12, p.x + 12, p.y + 12};
      const bool checker = ((gx + gy) & 1) != 0;
      HBRUSH brush = CreateSolidBrush(checker ? RGB(28, 42, 34) : RGB(22, 34, 28));
      FillRect(dc, &tile, brush);
      DeleteObject(brush);
    }
  }

  if (model.scene.has_stairs_up) {
    const auto p = at(model.scene.stairs_up_x, model.scene.stairs_up_y);
    HBRUSH stairs = CreateSolidBrush(RGB(210, 190, 90));
    RECT pad{p.x - 14, p.y - 14, p.x + 14, p.y + 14};
    FillRect(dc, &pad, stairs);
    DeleteObject(stairs);
  }

  for (auto& spark : play.sparks) {
    const auto p = at(spark.x, spark.y);
    const int radius = spark.telegraph ? 48 : 10 + spark.ttl;
    HBRUSH brush = CreateSolidBrush(spark.telegraph   ? RGB(220, 80, 40)
                                    : spark.incoming  ? RGB(200, 60, 60)
                                                      : RGB(240, 210, 80));
    HPEN pen = CreatePen(PS_SOLID, spark.telegraph ? 3 : 1, RGB(255, 255, 255));
    HGDIOBJ old_brush = SelectObject(dc, brush);
    HGDIOBJ old_pen = SelectObject(dc, pen);
    Ellipse(dc, p.x - radius, p.y - radius, p.x + radius, p.y + radius);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(brush);
    DeleteObject(pen);
    if (spark.value > 0) {
      char number[16];
      std::snprintf(number, sizeof(number), "%d", spark.value);
      SetBkMode(dc, TRANSPARENT);
      SetTextColor(dc, spark.incoming ? RGB(255, 120, 120) : RGB(255, 230, 140));
      TextOutA(dc, p.x + 8, p.y - 28 - (12 - spark.ttl), number, static_cast<int>(std::strlen(number)));
    }
    --spark.ttl;
  }
  play.sparks.erase(std::remove_if(play.sparks.begin(), play.sparks.end(),
                                   [](const Spark& spark) { return spark.ttl <= 0; }),
                    play.sparks.end());

  const auto player = at(model.player.x, model.player.y);
  HBRUSH body = CreateSolidBrush(model.player.alive ? RGB(80, 200, 140) : RGB(80, 80, 80));
  HGDIOBJ old_brush = SelectObject(dc, body);
  Ellipse(dc, player.x - 16, player.y - 16, player.x + 16, player.y + 16);
  SelectObject(dc, old_brush);
  DeleteObject(body);

  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, RGB(230, 236, 220));
  HFONT font = CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                           DEFAULT_PITCH, "Segoe UI");
  HFONT old_font = static_cast<HFONT>(SelectObject(dc, font));

  char hud[512];
  std::snprintf(hud, sizeof(hud),
                "REMOTE  %s  %s:%s   HP %d/%d   strike %d   kills %d   %s",
                verdigris::client::connection_state_label(state),
                model.scene.name.empty() ? model.scene.id.c_str() : model.scene.name.c_str(),
                model.scene.type.c_str(), model.player.life, model.player.life_max,
                model.player.attack, model.kills,
                model.extracted ? "SURFACE/BANKED" : "");
  TextOutA(dc, 16, 12, hud, static_cast<int>(std::strlen(hud)));

  if (!model.equipped.uuid.empty()) {
    char worn[256];
    std::snprintf(worn, sizeof(worn), "Equipped: %s   attack %d   crit %d   +HP %d",
                  model.equipped.name.empty() ? model.equipped.id.c_str()
                                              : model.equipped.name.c_str(),
                  model.equipped.attack_rating, model.equipped.critical_chance,
                  model.equipped.bonus_health);
    SetTextColor(dc, RGB(180, 220, 255));
    TextOutA(dc, 16, 32, worn, static_cast<int>(std::strlen(worn)));
  }

  SetTextColor(dc, RGB(200, 210, 190));
  int inventory_y = height - 28 - static_cast<int>(model.inventory.size()) * 16;
  TextOutA(dc, 16, inventory_y - 18, "Backpack", 8);
  for (std::size_t i = 0; i < model.inventory.size() && i < 12; ++i) {
    char line[160];
    std::snprintf(line, sizeof(line), "%zu  %s", i + 1,
                  model.inventory[i].name.empty() ? model.inventory[i].id.c_str()
                                                  : model.inventory[i].name.c_str());
    TextOutA(dc, 16, inventory_y + static_cast<int>(i) * 16, line, static_cast<int>(std::strlen(line)));
  }

  int log_y = 56;
  for (const auto& line : play.log) {
    TextOutA(dc, width - 420, log_y, line.c_str(), static_cast<int>(line.size()));
    log_y += 16;
  }

  if (!play.hint.empty()) {
    SetTextColor(dc, RGB(240, 220, 140));
    TextOutA(dc, 16, height - 22, play.hint.c_str(), static_cast<int>(play.hint.size()));
  }

  if (state == verdigris::client::ConnectionState::Disconnected ||
      state == verdigris::client::ConnectionState::Rejected ||
      state == verdigris::client::ConnectionState::ProtocolMismatch) {
    SetTextColor(dc, RGB(255, 80, 70));
    const char* banner = "CONNECTION LOST — not playing offline";
    TextOutA(dc, 16, 52, banner, static_cast<int>(std::strlen(banner)));
  }

  if (play.pulse > 0) {
    HBRUSH flash = CreateSolidBrush(RGB(120, 20, 16));
    RECT band{0, 0, width, 8};
    FillRect(dc, &band, flash);
    band = RECT{0, height - 8, width, height};
    FillRect(dc, &band, flash);
    DeleteObject(flash);
    --play.pulse;
  }

  SelectObject(dc, old_font);
  DeleteObject(font);
  BitBlt(screen, 0, 0, width, height, dc, 0, 0, SRCCOPY);
  SelectObject(dc, old);
  DeleteObject(bitmap);
  DeleteDC(dc);
  ReleaseDC(window, screen);
}

void tick_input(RemotePlay& play) {
  if (!play.session) return;
  if (play.session->connection_state() != verdigris::client::ConnectionState::Ready) return;
  int dx = 0;
  int dy = 0;
  if (play.a) dx -= 1;
  if (play.d) dx += 1;
  if (play.w) dy -= 1;
  if (play.s) dy += 1;
  if (dx != 0 || dy != 0) play.session->submit(verdigris::client::ClientCommand::move(dx, dy));
}

LRESULT CALLBACK remote_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  auto* play = reinterpret_cast<RemotePlay*>(GetWindowLongPtr(window, GWLP_USERDATA));
  switch (message) {
    case WM_CREATE: {
      auto* create = reinterpret_cast<CREATESTRUCTA*>(lparam);
      SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
      return 0;
    }
    case WM_TIMER:
      if (play) {
        tick_input(*play);
        InvalidateRect(window, nullptr, FALSE);
      }
      return 0;
    case WM_PAINT: {
      PAINTSTRUCT ps;
      BeginPaint(window, &ps);
      if (play) paint(window, *play);
      EndPaint(window, &ps);
      return 0;
    }
    case WM_KEYDOWN:
      if (!play || !play->session) return 0;
      if (wparam == VK_ESCAPE) {
        PostQuitMessage(0);
        return 0;
      }
      if (wparam == 'W') play->w = true;
      if (wparam == 'A') play->a = true;
      if (wparam == 'S') play->s = true;
      if (wparam == 'D') play->d = true;
      if (wparam == 'E')
        play->session->submit(verdigris::client::ClientCommand::enter_zone("tin:1:0"));
      if (wparam == 'X') play->session->submit(verdigris::client::ClientCommand::pick_up(""));
      if (wparam == 'F') play->session->submit(verdigris::client::ClientCommand::extract());
      if (wparam == VK_SPACE)
        play->session->submit(verdigris::client::ClientCommand::use_action("melee"));
      if (wparam >= '1' && wparam <= '9') {
        const std::size_t index = static_cast<std::size_t>(wparam - '1');
        if (index < play->session->model().inventory.size()) {
          play->session->submit(
              verdigris::client::ClientCommand::equip(play->session->model().inventory[index].uuid));
        }
      }
      return 0;
    case WM_KEYUP:
      if (!play) return 0;
      if (wparam == 'W') play->w = false;
      if (wparam == 'A') play->a = false;
      if (wparam == 'S') play->s = false;
      if (wparam == 'D') play->d = false;
      return 0;
    case WM_LBUTTONDOWN:
      if (play && play->session)
        play->session->submit(verdigris::client::ClientCommand::use_action("melee"));
      return 0;
    case WM_MOUSEMOVE:
      if (play && play->session) {
        RECT client{};
        GetClientRect(window, &client);
        const int mx = GET_X_LPARAM(lparam);
        const int my = GET_Y_LPARAM(lparam);
        const int cx = (client.right - client.left) / 2;
        const int cy = (client.bottom - client.top) / 2;
        int dx = 0;
        int dy = 0;
        if (std::abs(mx - cx) > std::abs(my - cy)) dx = mx >= cx ? 1 : -1;
        else dy = my >= cy ? 1 : -1;
        play->session->submit(verdigris::client::ClientCommand::aim(dx, dy));
      }
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProcA(window, message, wparam, lparam);
  }
}

}  // namespace

int run_remote_native_client(const char* host, unsigned short port, const char* guest_id) {
  RemotePlay play;
  play.session = std::make_unique<verdigris::client::RemoteProtocolSession>(
      host ? host : "127.0.0.1", port, guest_id ? guest_id : "cursor-guest", true);
  std::string error;
  if (!play.session->start(&error)) {
    std::fprintf(stderr, "verdigris_client --remote: %s\n", error.c_str());
    play.hint = error;
  }

  HINSTANCE instance = GetModuleHandle(nullptr);
  WNDCLASSA window_class{};
  window_class.hInstance = instance;
  window_class.lpfnWndProc = remote_proc;
  window_class.lpszClassName = "VerdigrisRemoteClient";
  window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassA(&window_class);

  HWND window =
      CreateWindowExA(0, window_class.lpszClassName, "Verdigris Remote Guest", WS_OVERLAPPEDWINDOW,
                      CW_USEDEFAULT, CW_USEDEFAULT, 1100, 720, nullptr, nullptr, instance, &play);
  ShowWindow(window, SW_SHOW);
  SetTimer(window, 1, 50, nullptr);

  MSG message{};
  while (GetMessage(&message, nullptr, 0, 0) > 0) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  if (play.session) play.session->shutdown();
  return static_cast<int>(message.wParam);
}

#else

int run_remote_native_client(const char*, unsigned short, const char*) { return 1; }

#endif
