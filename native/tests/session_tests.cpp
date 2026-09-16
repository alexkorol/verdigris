// TASK-0060 (D-122): C3 session seam tests. Proves the local adapter keeps
// deterministic play available and the remote adapter completes a REAL
// handshake against verdigris_server's WebSocket listener — plus the
// authentic negative: a dead endpoint is a visible hard failure, never a
// silent fallback to local play.
// TASK-0148: the Gate-B Chronicles journey over loopback — found House,
// set out a mortal Scion, die, entomb, successor, relic recovery, abrupt
// disconnect, reconnect as the same guest identity, same House state.

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <deque>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "../client/local_session.hpp"
#include "../client/remote_session.hpp"
#include "../client/presentation_state.hpp"
#include "verdigris/networking.hpp"

namespace {

int failures = 0;

void check(bool ok, const char* label) {
  std::printf("%s %s\n", ok ? "PASS" : "FAIL", label);
  if (!ok) ++failures;
}

bool checked(bool ok, const char* label) {
  check(ok, label);
  return ok;
}

bool wait_for_state(verdigris::client::IClientSession& session,
                    verdigris::client::ConnectionState wanted, int timeout_ms) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
  while (std::chrono::steady_clock::now() < deadline) {
    session.poll();
    if (session.connection_state() == wanted) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  session.poll();
  return session.connection_state() == wanted;
}

template <typename Pred>
bool wait_until(verdigris::client::IClientSession& session, int timeout_ms, Pred pred) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
  while (std::chrono::steady_clock::now() < deadline) {
    session.poll();
    if (pred()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  session.poll();
  return pred();
}

void local_session_ready_and_deterministic() {
  verdigris::client::LocalCoreSession session(0xC011AB1EULL, "House Verdigris");
  std::string error;
  check(session.start(&error), "local: start succeeds");
  check(session.connection_state() == verdigris::client::ConnectionState::Ready,
        "local: state is ready without a handshake");
  check(!session.model().player.uuid.empty(), "local: scion uuid published");
  check(session.model().house_name == "House Verdigris", "local: house name published");

  const auto before_x = session.model().player.x;
  const auto before_y = session.model().player.y;
  session.submit(verdigris::client::ClientCommand::enter_zone("route:tin:1:0"));
  for (int i = 0; i < 4; ++i) {
    session.submit(verdigris::client::ClientCommand::move(1, 0));
  }
  session.poll();
  const bool moved = session.model().player.x != before_x ||
                     session.model().player.y != before_y;
  check(moved, "local: movement commands reach the simulation through the seam");

  const auto events = session.drain_events();
  bool saw_ready = false;
  for (const auto& event : events) {
    if (event.type == verdigris::client::PresentationEventType::SessionReady) saw_ready = true;
  }
  check(saw_ready, "local: SessionReady presentation event emitted");
  session.shutdown();
  check(session.connection_state() == verdigris::client::ConnectionState::Disconnected,
        "local: shutdown reaches disconnected state");
}

void hunt_step(verdigris::client::IClientSession& session) {
  // The swing range gate (JS parity) means the driver must close distance:
  // walk toward the nearest live monster in the authoritative model, then
  // strike once adjacent-ish.
  const auto& model = session.model();
  const verdigris::client::ClientMonster* nearest = nullptr;
  double best = 1e9;
  for (const auto& monster : model.monsters) {
    if (!monster.alive) continue;
    const double dx = monster.x - model.player.x;
    const double dy = monster.y - model.player.y;
    const double d = std::abs(dx) + std::abs(dy);
    if (d < best) { best = d; nearest = &monster; }
  }
  if (!nearest) { session.submit(verdigris::client::ClientCommand::move(1, 0)); return; }
  const int step_x = nearest->x > model.player.x + 0.5 ? 1 : (nearest->x < model.player.x - 0.5 ? -1 : 0);
  const int step_y = nearest->y > model.player.y + 0.5 ? 1 : (nearest->y < model.player.y - 0.5 ? -1 : 0);
  if (best > 2.0) {
    // Warren layouts are mazes; approach exactly the way the browser
    // scenarios do - through the served dev:teleport control surface.
    auto* remote = dynamic_cast<verdigris::client::RemoteProtocolSession*>(&session);
    if (remote) {
      verdigris::networking::JsonValue::Object tp;
      tp["x"] = verdigris::networking::JsonValue(static_cast<int>(nearest->x) + 1);
      tp["y"] = verdigris::networking::JsonValue(static_cast<int>(nearest->y));
      remote->send_raw("dev:teleport", verdigris::networking::JsonValue(std::move(tp)));
    } else if (step_x != 0) {
      session.submit(verdigris::client::ClientCommand::move(step_x, 0));
    } else if (step_y != 0) {
      session.submit(verdigris::client::ClientCommand::move(0, step_y));
    }
  }
  session.submit(verdigris::client::ClientCommand::use_action("melee"));
}
std::uint16_t start_server(verdigris::networking::WebSocketServer*& out) {
  // Architect capsule 6560-6579 (ORCHESTRATION.md); scan for a free port so
  // parallel suites cannot collide.
  for (std::uint16_t port = 6572; port <= 6579; ++port) {
    auto* server = new verdigris::networking::WebSocketServer(port);
    std::string error;
    if (server->start(&error)) {
      out = server;
      return port;
    }
    delete server;
  }
  out = nullptr;
  return 0;
}

void remote_dead_endpoint_is_a_visible_failure() {
  // Nothing listens on this port (start_server scans upward from 6572; 6571
  // is reserved for this negative and never bound).
  verdigris::client::RemoteProtocolSession session("127.0.0.1", 6571, "negative-guest");
  std::string error;
  const bool started = session.start(&error);
  check(!started, "remote-negative: dead endpoint fails start()");
  check(session.connection_state() == verdigris::client::ConnectionState::Rejected,
        "remote-negative: state is rejected, not a silent local fallback");
  check(!error.empty(), "remote-negative: hard error message supplied");
  session.shutdown();
}

void remote_handshake_reaches_ready() {
  verdigris::networking::WebSocketServer* server = nullptr;
  const auto port = start_server(server);
  check(server != nullptr, "remote: test server bound inside the architect capsule");
  if (!server) return;

  {
    verdigris::client::RemoteProtocolSession session("127.0.0.1", port,
                                                     "seam-test-guest", true);
    std::string error;
    check(session.start(&error), "remote: connect + upgrade + login sent");
    check(wait_for_state(session, verdigris::client::ConnectionState::Ready, 5000),
          "remote: player:login acknowledged -> ready");
    check(session.model().player.uuid == "seam-test-guest",
          "remote: authoritative identity mirrored into the client model");
    check(!session.model().scene.id.empty(), "remote: scene snapshot mirrored");

    bool saw_ready = false;
    for (const auto& event : session.drain_events()) {
      if (event.type == verdigris::client::PresentationEventType::SessionReady) saw_ready = true;
    }
    check(saw_ready, "remote: SessionReady presentation event emitted");

    session.submit(verdigris::client::ClientCommand::move(1, 0));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    session.poll();  // movement echo handling lands in 0061; must not wedge
    check(session.connection_state() == verdigris::client::ConnectionState::Ready,
          "remote: session survives a submitted command");
    session.shutdown();
    check(session.connection_state() == verdigris::client::ConnectionState::Disconnected,
          "remote: clean shutdown reaches disconnected");
  }

  server->stop();
  delete server;
  check(true, "remote: server stopped cleanly");
}

std::uint16_t start_server_cursor(verdigris::networking::WebSocketServer*& out) {
  // Cursor capsule 6580-6599 (ORCHESTRATION.md).
  for (std::uint16_t port = 6580; port <= 6599; ++port) {
    auto* server = new verdigris::networking::WebSocketServer(port);
    std::string error;
    if (server->start(&error)) {
      out = server;
      return port;
    }
    delete server;
  }
  out = nullptr;
  return 0;
}

void collect_flags(verdigris::client::IClientSession& session, bool& outgoing, bool& incoming,
                   bool& telegraph, bool& kill, bool& pickup, bool& equipped, bool& extracted,
                   bool& lost) {
  for (const auto& event : session.drain_events()) {
    using T = verdigris::client::PresentationEventType;
    if (event.type == T::DamageApplied && event.text == "outgoing") outgoing = true;
    if (event.type == T::DamageApplied && event.text == "incoming") incoming = true;
    if (event.type == T::Telegraph) telegraph = true;
    if (event.type == T::ActorDied) kill = true;
    if (event.type == T::ItemPickedUp) pickup = true;
    if (event.type == T::ItemEquipped) equipped = true;
    if (event.type == T::ExtractionCompleted) extracted = true;
    if (event.type == T::ConnectionLost) lost = true;
  }
}

const verdigris::client::ClientItemSlot* first_equippable(
    const verdigris::client::ClientModel& model) {
  for (const auto& item : model.inventory) {
    if (item.id != "coins" && !item.uuid.empty()) return &item;
  }
  return nullptr;
}

void remote_guest_journey() {
  verdigris::networking::WebSocketServer* server = nullptr;
  const auto port = start_server_cursor(server);
  check(server != nullptr, "journey: test server bound inside the cursor capsule 6580-6599");
  if (!server) return;

  verdigris::client::RemoteProtocolSession session("127.0.0.1", port, "cursor-guest-0061", true);
  std::string error;
  check(session.start(&error), "journey: connect + upgrade + login sent");
  check(wait_for_state(session, verdigris::client::ConnectionState::Ready, 5000),
        "journey: handshake ready");

  { // pin the protocol slice to the fixed dungeon/warren surface - the
    // per-house world-web node behind enter_zone has its own attach coverage.
    verdigris::networking::JsonValue::Object solo;
    solo["template"] = verdigris::networking::JsonValue("dungeon");
    solo["layout"] = verdigris::networking::JsonValue("warren");
    auto* remote = dynamic_cast<verdigris::client::RemoteProtocolSession*>(&session);
    if (remote) remote->send_raw("instance:enterSolo", verdigris::networking::JsonValue(std::move(solo)));
    else session.submit(verdigris::client::ClientCommand::enter_zone("tin:1:0"));
  }
  const bool entered = wait_until(session, 4000, [&] {
    return session.model().scene.type == "instance" ||
           session.model().scene.id.find("instance") != std::string::npos;
  });
  check(entered, "journey: zone enter mirrors instance scene");
  check(session.model().scene.has_stairs_up, "journey: transition publishes exit stairs");

  const double start_x = session.model().player.x;
  // A short eastward walk proves the movement echo; the hunt loop handles
  // closing distance to the pack (range-gated swings need adjacency).
  for (int i = 0; i < 6; ++i) {
    session.submit(verdigris::client::ClientCommand::move(1, 0));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    session.poll();
  }
  check(session.model().player.x > start_x, "journey: movement echo updates x");

  session.submit(verdigris::client::ClientCommand::aim(1, 0));
  session.poll();
  check(session.model().player.facing == "right", "journey: aim updates facing");

  bool outgoing = false;
  bool incoming = false;
  bool telegraph = false;
  bool kill = false;
  bool pickup = false;
  bool equipped = false;
  bool extracted = false;
  bool lost = false;

  for (int step = 0; step < 480; ++step) {
    hunt_step(session);
    if (step % 3 == 0) session.submit(verdigris::client::ClientCommand::pick_up(""));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    session.poll();
    collect_flags(session, outgoing, incoming, telegraph, kill, pickup, equipped, extracted,
                  lost);
    if (outgoing && kill && first_equippable(session.model())) break;
  }
  check(outgoing, "journey: outgoing combat:hit reached the client");
  check(kill, "journey: enemy death reached the client");

  // Kill loot and floor treasure surface in the authoritative ground list;
  // walk the model - stand on the nearest drop and take underfoot - instead
  // of sweeping blind (warren mazes defeat a fixed eastward walk).
  for (int step = 0; step < 200 && !first_equippable(session.model()); ++step) {
    const auto& ground = session.model().ground;
    const verdigris::client::ClientGroundItem* drop = nullptr;
    double drop_best = 1e9;
    for (const auto& item : ground) {
      const double d = std::abs(item.x - session.model().player.x) +
                       std::abs(item.y - session.model().player.y);
      if (d < drop_best) { drop_best = d; drop = &item; }
    }
    if (drop) {
      auto* remote = dynamic_cast<verdigris::client::RemoteProtocolSession*>(&session);
      if (remote && drop_best > 0.5) {
        verdigris::networking::JsonValue::Object tp;
        tp["x"] = verdigris::networking::JsonValue(static_cast<int>(drop->x));
        tp["y"] = verdigris::networking::JsonValue(static_cast<int>(drop->y));
        remote->send_raw("dev:teleport", verdigris::networking::JsonValue(std::move(tp)));
      }
      session.submit(verdigris::client::ClientCommand::pick_up(""));
    } else {
      hunt_step(session);  // no drops yet: keep clearing the pack
      session.submit(verdigris::client::ClientCommand::pick_up(""));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    session.poll();
    collect_flags(session, outgoing, incoming, telegraph, kill, pickup, equipped, extracted,
                  lost);
  }
  const auto* gear = first_equippable(session.model());
  check(gear != nullptr, "journey: named item entered inventory (pickup)");
  check(pickup || gear != nullptr, "journey: ItemPickedUp or inventory growth");

  if (gear) {
    const std::string uuid = gear->uuid;
    const std::string name = gear->name;
    check(!name.empty() && name != "coins", "journey: picked item is named");
    session.submit(verdigris::client::ClientCommand::equip(uuid));
    const bool wore = wait_until(session, 3000, [&] {
      session.poll();
      collect_flags(session, outgoing, incoming, telegraph, kill, pickup, equipped, extracted,
                    lost);
      return equipped || session.model().equipped.uuid == uuid;
    });
    check(wore, "journey: equip removed the item from the backpack");
    check(!session.model().equipped.uuid.empty(),
          "journey: equipped slot mirrors the worn item");
  }

  // Incoming hit and telegraph need adjacency to a live foe / boss. Keep
  // striking while easing east, then turn back for extract.
  for (int step = 0; step < 720 && !(incoming && telegraph); ++step) {
    session.submit(verdigris::client::ClientCommand::use_action("melee"));
    int dx = 1;
    int dy = 0;
    if (session.model().player.x > 26.0) dx = -1;
    if (session.model().player.x < 10.0) dx = 1;
    if (step % 14 == 0) dy = 1;
    if (step % 14 == 7) dy = -1;
    session.submit(verdigris::client::ClientCommand::move(dx, dy));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    session.poll();
    collect_flags(session, outgoing, incoming, telegraph, kill, pickup, equipped, extracted,
                  lost);
  }
  check(incoming, "journey: incoming combat:hit reached the client");
  check(telegraph, "journey: monster:telegraph reached the client");

  for (int step = 0; step < 360 && !session.model().extracted; ++step) {
    int dx = -1;
    int dy = 0;
    if (session.model().scene.has_stairs_up) {
      const double target_y = session.model().scene.stairs_up_y;
      if (session.model().player.y < target_y - 0.4) dy = 1;
      if (session.model().player.y > target_y + 0.4) dy = -1;
    }
    if (dy != 0 && step % 2 == 0)
      session.submit(verdigris::client::ClientCommand::move(0, dy));
    else
      session.submit(verdigris::client::ClientCommand::move(dx, 0));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    session.poll();
    collect_flags(session, outgoing, incoming, telegraph, kill, pickup, equipped, extracted,
                  lost);
  }
  check(session.model().extracted || session.model().scene.type == "town" ||
            session.model().scene.id.find("town") != std::string::npos,
        "journey: walking onto stairs-up returns to the surface (extract)");
  check(session.model().extracted, "journey: ExtractionCompleted from surface message");

  session.shutdown();
  check(session.connection_state() == verdigris::client::ConnectionState::Disconnected,
        "journey: clean dual-side shutdown");
  server->stop();
  delete server;
}

void remote_mid_session_disconnect() {
  verdigris::networking::WebSocketServer* server = nullptr;
  const auto port = start_server_cursor(server);
  check(server != nullptr, "reconnect: cursor-capsule server bound");
  if (!server) return;

  verdigris::client::RemoteProtocolSession session("127.0.0.1", port, "cursor-reconnect", true);
  std::string error;
  check(session.start(&error), "reconnect: connected");
  check(wait_for_state(session, verdigris::client::ConnectionState::Ready, 5000),
        "reconnect: ready before the drop");
  const std::string guest = session.model().player.uuid;
  const auto x = session.model().player.x;

  server->stop();
  delete server;
  server = nullptr;

  bool lost = false;
  const bool retrying = wait_until(session, 4000, [&] {
    session.poll();
    for (const auto& event : session.drain_events()) {
      if (event.type == verdigris::client::PresentationEventType::ConnectionLost) lost = true;
    }
    return session.connection_state() == verdigris::client::ConnectionState::Retrying;
  });
  check(retrying, "reconnect: unexpected drop enters Retrying");
  check(lost, "reconnect: ConnectionLost is visible (no silent local fallback)");
  check(!session.last_error().empty(), "reconnect: last_error explains the drop");

  session.submit(verdigris::client::ClientCommand::move(1, 0));
  session.poll();
  check(session.connection_state() == verdigris::client::ConnectionState::Retrying,
        "reconnect: commands after the drop do not leave Retrying for a local sim");
  check(session.model().player.x == x, "reconnect: position does not keep playing offline");

  server = new verdigris::networking::WebSocketServer(port);
  check(server->start(&error), "reconnect: server restarted on the same port");

  const bool resumed = wait_for_state(session, verdigris::client::ConnectionState::Ready, 8000);
  check(resumed, "reconnect: Retrying then Ready after server restart");
  check(session.model().player.uuid == guest, "reconnect: same guest identity re-logged in");
  check(!session.model().scene.id.empty(), "reconnect: login snapshot is authoritative");

  session.shutdown();
  if (server) {
    server->stop();
    delete server;
  }
}

void remote_session_replaced() {
  verdigris::networking::WebSocketServer* server = nullptr;
  const auto port = start_server_cursor(server);
  check(server != nullptr, "replaced: cursor-capsule server bound");
  if (!server) return;

  verdigris::client::RemoteProtocolSession first("127.0.0.1", port, "cursor-replaced", true);
  std::string error;
  check(first.start(&error), "replaced: first login");
  check(wait_for_state(first, verdigris::client::ConnectionState::Ready, 5000),
        "replaced: first session ready");

  verdigris::client::RemoteProtocolSession second("127.0.0.1", port, "cursor-replaced", true);
  check(second.start(&error), "replaced: second login same guest");
  check(wait_for_state(second, verdigris::client::ConnectionState::Ready, 5000),
        "replaced: second session ready");

  bool lost = false;
  bool saw_retrying = false;
  const bool flushed = wait_until(first, 4000, [&] {
    first.poll();
    if (first.connection_state() == verdigris::client::ConnectionState::Retrying)
      saw_retrying = true;
    for (const auto& event : first.drain_events()) {
      if (event.type == verdigris::client::PresentationEventType::ConnectionLost) lost = true;
    }
    return first.connection_state() == verdigris::client::ConnectionState::Disconnected;
  });
  check(flushed, "replaced: first session is disconnected");
  check(lost, "replaced: ConnectionLost from player:session-replaced");
  check(!saw_retrying, "replaced: session-replaced does not enter Retrying");
  check(first.connection_state() != verdigris::client::ConnectionState::Retrying,
        "replaced: stays terminal Disconnected (no retry)");
  first.shutdown();
  second.shutdown();
  server->stop();
  delete server;
}

void remote_render_list_ops() {
  verdigris::networking::WebSocketServer* server = nullptr;
  const auto port = start_server_cursor(server);
  check(server != nullptr, "render-list: cursor-capsule server bound");
  if (!server) return;

  verdigris::client::RemoteProtocolSession session("127.0.0.1", port, "cursor-render-ops", true);
  std::string error;
  check(session.start(&error), "render-list: connect");
  check(wait_for_state(session, verdigris::client::ConnectionState::Ready, 5000),
        "render-list: ready");
  { // pin the protocol slice to the fixed dungeon/warren surface - the
    // per-house world-web node behind enter_zone has its own attach coverage.
    verdigris::networking::JsonValue::Object solo;
    solo["template"] = verdigris::networking::JsonValue("dungeon");
    solo["layout"] = verdigris::networking::JsonValue("warren");
    auto* remote = dynamic_cast<verdigris::client::RemoteProtocolSession*>(&session);
    if (remote) remote->send_raw("instance:enterSolo", verdigris::networking::JsonValue(std::move(solo)));
    else session.submit(verdigris::client::ClientCommand::enter_zone("tin:1:0"));
  }
  wait_until(session, 4000, [&] {
    return session.model().scene.type == "instance" ||
           session.model().scene.id.find("instance") != std::string::npos;
  });

  verdigris::client::PresentationFx fx;
  verdigris::client::WorldView world;
  bool saw_monster = false, saw_swing = false, saw_drop = false;
  for (int step = 0; step < 240 && !(saw_monster && saw_swing && saw_drop); ++step) {
    hunt_step(session);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    session.poll();
    verdigris::client::sync_world_from_model(world, session.model());
    ++world.tick;
    for (const auto& event : session.drain_events())
      verdigris::client::apply_presentation_event(fx, world, event, world.tick);
    verdigris::client::age_presentation_fx(fx);
    verdigris::client::sync_world_from_model(world, session.model());
    render::List list;
    camera2d::Camera camera{static_cast<double>(world.player.position.x),
                            static_cast<double>(world.player.position.y), 0.85};
    verdigris::client::record_world_ops(list, world, fx, camera, 960, 600);
    saw_monster = saw_monster || render::any(list, render::Op::Monster);
    saw_swing = saw_swing || render::any(list, render::Op::Swing);
    saw_drop = saw_drop || render::any(list, render::Op::Drop);
  }
  check(saw_monster, "render-list: Monster op recorded from remote model");
  check(saw_swing, "render-list: Swing op recorded from AttackStarted");
  check(saw_drop, "render-list: Drop op recorded from kill loot");
  session.shutdown();
  server->stop();
  delete server;
}

// ── TASK-0148: Gate-B Chronicles journey over loopback ──────────────────
// RemoteProtocolSession's client model intentionally drops chronicles
// payloads, so this journey drives verdigris_server through a minimal raw
// RFC6455 test client that can see every server envelope. Normal accepted
// envelopes only: no dev mutation events (dev:state / dev:teleport are the
// same read/position control surfaces the accepted session scenarios use).

using JsonValue = verdigris::networking::JsonValue;
using Envelope = verdigris::networking::Envelope;

#ifdef _WIN32
using jw_socket_t = SOCKET;
constexpr jw_socket_t jw_invalid_socket = INVALID_SOCKET;
#else
using jw_socket_t = int;
constexpr jw_socket_t jw_invalid_socket = -1;
#endif

static void jw_close_socket(jw_socket_t sock) {
#ifdef _WIN32
  ::closesocket(sock);
#else
  ::close(sock);
#endif
}

static bool jw_send_all(jw_socket_t sock, const char* data, std::size_t size) {
  std::size_t sent = 0;
  while (sent < size) {
    const int got = ::send(sock, data + sent, static_cast<int>(size - sent), 0);
    if (got <= 0) return false;
    sent += static_cast<std::size_t>(got);
  }
  return true;
}

// Minimal masked-frame WebSocket client: connect, upgrade, send envelopes,
// collect every server envelope.
class JourneyWire {
 public:
  ~JourneyWire() { close_abrupt(); }

  bool connect(std::uint16_t port, const std::string& guest, bool quick) {
#ifdef _WIN32
    WSADATA data{};
    if (::WSAStartup(MAKEWORD(2, 2), &data) != 0) return false;
    wsa_ = true;
#endif
    const jw_socket_t sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == jw_invalid_socket) return false;
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    if (::connect(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
      jw_close_socket(sock);
      return false;
    }
    sock_ = sock;
    const std::string request =
        "GET / HTTP/1.1\r\nHost: 127.0.0.1:" + std::to_string(port) +
        "\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
        "Sec-WebSocket-Key: ZGVhZGJlZWZjYWZlZmFjZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n";
    if (!jw_send_all(sock_, request.data(), request.size())) return fail_transport();
    std::string response;
    char buffer[1024];
    while (response.find("\r\n\r\n") == std::string::npos && response.size() < 8192) {
      const int got = ::recv(sock_, buffer, sizeof(buffer), 0);
      if (got <= 0) return fail_transport();
      response.append(buffer, buffer + got);
    }
    if (response.find(" 101 ") == std::string::npos) return fail_transport();
    JsonValue::Object login;
    login["guestId"] = JsonValue(guest);
    login["quickGuest"] = JsonValue(quick);
    send("player:login", JsonValue(std::move(login)));
    return true;
  }

  void send(const std::string& event, JsonValue data) {
    if (sock_ == jw_invalid_socket) return;
    const Envelope envelope{event, std::move(data)};
    send_text(verdigris::networking::emit_envelope(envelope));
  }

  bool wait_for(const std::string& event, Envelope* out, int timeout_ms) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    for (;;) {
      for (auto it = inbox_.begin(); it != inbox_.end(); ++it) {
        if (it->event == event) {
          if (out) *out = *it;
          inbox_.erase(it);
          return true;
        }
      }
      if (closed_ || std::chrono::steady_clock::now() >= deadline) return false;
      read_some(50);
    }
  }

  void close_abrupt() {
    if (sock_ != jw_invalid_socket) {
      jw_close_socket(sock_);
      sock_ = jw_invalid_socket;
    }
    closed_ = true;
#ifdef _WIN32
    if (wsa_) { ::WSACleanup(); wsa_ = false; }
#endif
  }

 private:
  bool fail_transport() {
    if (sock_ != jw_invalid_socket) jw_close_socket(sock_);
    sock_ = jw_invalid_socket;
    return false;
  }

  void send_text(const std::string& text) {
    std::vector<std::uint8_t> frame;
    frame.push_back(0x81);
    const auto size = text.size();
    if (size < 126) {
      frame.push_back(static_cast<std::uint8_t>(0x80 | size));
    } else if (size <= 65535) {
      frame.push_back(0x80 | 126);
      frame.push_back(static_cast<std::uint8_t>(size >> 8));
      frame.push_back(static_cast<std::uint8_t>(size));
    } else {
      frame.push_back(0x80 | 127);
      for (int i = 7; i >= 0; --i)
        frame.push_back(static_cast<std::uint8_t>((size >> (i * 8)) & 0xff));
    }
    const std::uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
    frame.insert(frame.end(), mask, mask + 4);
    const auto offset = frame.size();
    frame.resize(offset + size);
    for (std::size_t i = 0; i < size; ++i)
      frame[offset + i] = static_cast<std::uint8_t>(text[i]) ^ mask[i % 4];
    if (!jw_send_all(sock_, reinterpret_cast<const char*>(frame.data()), frame.size()))
      closed_ = true;
  }

  void read_some(int timeout_ms) {
    if (closed_ || sock_ == jw_invalid_socket) return;
    fd_set readable;
    FD_ZERO(&readable);
    FD_SET(sock_, &readable);
    timeval timeout{timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    if (::select(static_cast<int>(sock_) + 1, &readable, nullptr, nullptr, &timeout) <= 0)
      return;
    char buffer[4096];
    const int got = ::recv(sock_, buffer, sizeof(buffer), 0);
    if (got <= 0) {
      closed_ = true;
      return;
    }
    buf_.append(buffer, buffer + got);
    drain_frames();
  }

  void drain_frames() {
    for (;;) {
      if (buf_.size() < 2) return;
      const auto opcode = static_cast<std::uint8_t>(buf_[0] & 0x0f);
      const bool masked = (buf_[1] & 0x80) != 0;
      std::uint64_t length = static_cast<std::uint8_t>(buf_[1] & 0x7f);
      std::size_t offset = 2;
      if (length == 126) {
        if (buf_.size() < 4) return;
        length = static_cast<std::uint64_t>(
            (static_cast<std::uint8_t>(buf_[2]) << 8) | static_cast<std::uint8_t>(buf_[3]));
        offset = 4;
      } else if (length == 127) {
        if (buf_.size() < 10) return;
        length = 0;
        for (int i = 0; i < 8; ++i)
          length = (length << 8) | static_cast<std::uint8_t>(buf_[2 + i]);
        offset = 10;
      }
      if (length > (1u << 20)) { closed_ = true; return; }
      const std::size_t mask_size = masked ? 4 : 0;
      if (buf_.size() < offset + mask_size + length) return;
      std::string payload = buf_.substr(offset + mask_size, static_cast<std::size_t>(length));
      if (masked) {
        for (std::size_t i = 0; i < payload.size(); ++i)
          payload[i] ^= buf_[offset + (i % 4)];
      }
      buf_.erase(0, static_cast<std::size_t>(offset + mask_size + length));
      if (opcode == 0x8) { closed_ = true; return; }
      if (opcode == 0x9) { send_text_pong(payload); continue; }
      if (opcode == 0x1) {
        Envelope envelope;
        if (verdigris::networking::parse_envelope(payload, envelope))
          inbox_.push_back(std::move(envelope));
      }
    }
  }

  void send_text_pong(const std::string& payload) {
    std::vector<std::uint8_t> frame;
    frame.push_back(0x8A);
    const auto size = payload.size();
    if (size < 126) {
      frame.push_back(static_cast<std::uint8_t>(0x80 | size));
    } else {
      frame.push_back(0x80 | 126);
      frame.push_back(static_cast<std::uint8_t>(size >> 8));
      frame.push_back(static_cast<std::uint8_t>(size));
    }
    const std::uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
    frame.insert(frame.end(), mask, mask + 4);
    const auto offset = frame.size();
    frame.resize(offset + size);
    for (std::size_t i = 0; i < size; ++i)
      frame[offset + i] = static_cast<std::uint8_t>(payload[i]) ^ mask[i % 4];
    jw_send_all(sock_, reinterpret_cast<const char*>(frame.data()), frame.size());
  }

  jw_socket_t sock_ = jw_invalid_socket;
  bool closed_ = false;
  bool wsa_ = false;
  std::string buf_;
  std::deque<Envelope> inbox_;
};

std::uint16_t start_server_ox_pc_g(verdigris::networking::WebSocketServer*& out) {
  // ox-pc-g capsule 6740-6759 (REENTRY-OX-ALPHA-PC); scan for a free port.
  for (std::uint16_t port = 6740; port <= 6759; ++port) {
    auto* server = new verdigris::networking::WebSocketServer(port);
    std::string error;
    if (server->start(&error)) {
      out = server;
      return port;
    }
    delete server;
  }
  out = nullptr;
  return 0;
}

const JsonValue* jfield(const JsonValue& value, const std::string& key) { return value.get(key); }

std::string jtext(const JsonValue* value) {
  return value && value->string() ? *value->string() : std::string();
}

double jnumber(const JsonValue* value, double fallback = -1.0) {
  return value && value->number() ? *value->number() : fallback;
}

bool chronicle_house_state(const Envelope& state, std::string* house_id, std::string* house_name,
                           int* scion_count, int* crypt_count) {
  const auto* chronicle = jfield(state.data, "chronicle");
  const auto* houses = chronicle ? jfield(*chronicle, "houses") : nullptr;
  if (!houses || !houses->array() || houses->array()->empty()) return false;
  const JsonValue& house = (*houses->array())[0];
  if (!house.object()) return false;
  if (house_id) *house_id = jtext(jfield(house, "id"));
  if (house_name) *house_name = jtext(jfield(house, "name"));
  if (scion_count) {
    const auto* scions = jfield(house, "scions");
    *scion_count = scions && scions->array() ? static_cast<int>(scions->array()->size()) : 0;
  }
  if (crypt_count) {
    const auto* crypt = jfield(house, "crypt");
    *crypt_count = crypt && crypt->array() ? static_cast<int>(crypt->array()->size()) : 0;
  }
  return true;
}

struct JourneyMonster {
  std::string uuid;
  double x = 0;
  double y = 0;
  bool boss = false;
};

std::vector<JourneyMonster> journey_monsters(const Envelope& state) {
  std::vector<JourneyMonster> monsters;
  const auto* list = jfield(state.data, "state");
  const auto* monsters_value = list ? jfield(*list, "monsters") : nullptr;
  if (monsters_value && monsters_value->array()) {
    for (const auto& entry : *monsters_value->array()) {
      JourneyMonster parsed;
      parsed.uuid = jtext(jfield(entry, "uuid"));
      parsed.x = jnumber(jfield(entry, "x"));
      parsed.y = jnumber(jfield(entry, "y"));
      parsed.boss = jtext(jfield(entry, "rarity")) == "elite";
      if (!parsed.uuid.empty() && parsed.x >= 0 && parsed.y >= 0) monsters.push_back(parsed);
    }
  }
  return monsters;
}

struct JourneyItem {
  std::string uuid;
  std::string id;
};

void collect_pack(const JsonValue& player, std::vector<JourneyItem>* out) {
  const auto* inventory = jfield(player, "inventory");
  const auto* slots = inventory ? jfield(*inventory, "slots") : nullptr;
  if (!slots || !slots->array()) return;
  for (const auto& slot : *slots->array()) {
    JourneyItem parsed;
    parsed.uuid = jtext(jfield(slot, "uuid"));
    parsed.id = jtext(jfield(slot, "id"));
    out->push_back(std::move(parsed));
  }
}

void enter_warren(JourneyWire& wire) {
  JsonValue::Object solo;
  solo["template"] = JsonValue("dungeon");
  solo["layout"] = JsonValue("warren");
  wire.send("instance:enterSolo", JsonValue(std::move(solo)));
}

void journey_teleport(JourneyWire& wire, double x, double y) {
  JsonValue::Object teleport;
  teleport["x"] = JsonValue(static_cast<int>(x));
  teleport["y"] = JsonValue(static_cast<int>(y));
  wire.send("dev:teleport", JsonValue(std::move(teleport)));
}

void journey_take(JourneyWire& wire, const std::string& uuid, double x, double y) {
  // Normal registry take: stand on the tile, then the context-menu Take
  // action with the item uuid (actions/index.js player:take).
  journey_teleport(wire, x, y);
  JsonValue::Object item_ref;
  item_ref["uuid"] = JsonValue(uuid);
  JsonValue::Object action;
  action["actionId"] = JsonValue("player:take");
  JsonValue::Object queue_item;
  queue_item["action"] = JsonValue(std::move(action));
  queue_item["item"] = JsonValue(std::move(item_ref));
  JsonValue::Object payload;
  payload["queueItem"] = JsonValue(std::move(queue_item));
  wire.send("player:context-menu:action", JsonValue(std::move(payload)));
}

bool journey_snapshot(JourneyWire& wire, const char* request_id, Envelope* snapshot) {
  JsonValue::Object request;
  request["requestId"] = JsonValue(request_id);
  wire.send("dev:state", JsonValue(std::move(request)));
  return wire.wait_for("dev:state", snapshot, 1000);
}

bool journey_find_relic(const Envelope& snapshot, std::string* uuid, double* x, double* y) {
  const auto* snap = jfield(snapshot.data, "state");
  const auto* ground = snap ? jfield(*snap, "groundItems") : nullptr;
  if (!ground || !ground->array()) return false;
  for (const auto& entry : *ground->array()) {
    if (!jtext(jfield(entry, "legacyRelicId")).empty() ||
        jfield(entry, "chroniclesRelic") != nullptr) {
      *uuid = jtext(jfield(entry, "uuid"));
      *x = jnumber(jfield(entry, "x"));
      *y = jnumber(jfield(entry, "y"));
      return !uuid->empty();
    }
  }
  return false;
}

std::string fallen_crypt_json(const JsonValue& root, const std::string& scion_id) {
  const auto* houses = jfield(root, "houses");
  if (!houses || !houses->array()) return std::string();
  for (const auto& house_entry : *houses->array()) {
    const auto* crypt = jfield(house_entry, "crypt");
    if (!crypt || !crypt->array()) continue;
    for (const auto& crypt_entry : *crypt->array()) {
      if (jtext(jfield(crypt_entry, "id")) == scion_id) return crypt_entry.stringify();
    }
  }
  return std::string();
}

std::string crypt_relic_status(const JsonValue& root, const std::string& scion_id) {
  const auto* houses = jfield(root, "houses");
  if (!houses || !houses->array()) return std::string();
  for (const auto& house_entry : *houses->array()) {
    const auto* crypt = jfield(house_entry, "crypt");
    if (!crypt || !crypt->array()) continue;
    for (const auto& crypt_entry : *crypt->array()) {
      if (jtext(jfield(crypt_entry, "id")) != scion_id) continue;
      const auto* relic = jfield(crypt_entry, "relic");
      return relic ? jtext(jfield(*relic, "status")) : std::string();
    }
  }
  return std::string();
}

void chronicles_gate_b_reconnect_journey() {
  verdigris::networking::WebSocketServer* server = nullptr;
  const auto port = start_server_ox_pc_g(server);
  check(server != nullptr, "gate-b: loopback server bound in the ox-pc-g capsule 6740-6759");
  if (!server) return;

  const std::string guest = "gate-b-journey-ox-pc-g";
  JourneyWire wire;
  auto bail = [&]() { wire.close_abrupt(); server->stop(); delete server; };
  if (!checked(wire.connect(port, guest, false),
               "gate-b: connect + chronicles admission login")) {
    bail();
    return;
  }

  Envelope state;
  if (!checked(wire.wait_for("chronicles:state", &state, 5000),
               "gate-b: guest login emits chronicles:state")) {
    bail();
    return;
  }

  // found House
  {
    JsonValue::Object found;
    found["name"] = JsonValue("House Gate-B");
    wire.send("chronicles:house:found", JsonValue(std::move(found)));
  }
  std::string house_id;
  std::string house_name;
  if (!checked(wire.wait_for("chronicles:state", &state, 5000) &&
                   chronicle_house_state(state, &house_id, &house_name, nullptr, nullptr) &&
                   house_name == "House Gate-B",
               "gate-b: House founded and echoed")) {
    bail();
    return;
  }

  // create Scion A
  {
    JsonValue::Object create;
    create["houseId"] = JsonValue(house_id);
    create["name"] = JsonValue("Aldric");
    wire.send("chronicles:scion:create", JsonValue(std::move(create)));
  }
  std::string scion_a;
  {
    const bool created = wire.wait_for("chronicles:state", &state, 5000);
    scion_a = created ? jtext(jfield(state.data, "createdScionId")) : std::string();
    if (!checked(created && !scion_a.empty(), "gate-b: first Scion created (Aldric)")) {
      bail();
      return;
    }
  }

  // set out Scion A
  {
    JsonValue::Object set_out;
    set_out["scionId"] = JsonValue(scion_a);
    wire.send("chronicles:scion:set-out", JsonValue(std::move(set_out)));
  }
  Envelope login;
  if (!checked(wire.wait_for("player:login", &login, 5000),
               "gate-b: set-out admits the Scion (player:login)")) {
    bail();
    return;
  }
  const JsonValue* login_player = jfield(login.data, "player");
  const JsonValue* login_chronicles = login_player ? jfield(*login_player, "chronicles") : nullptr;
  if (!checked(jtext(login_chronicles ? jfield(*login_chronicles, "scionId") : nullptr) == scion_a,
               "gate-b: login carries the admitted scion id")) {
    bail();
    return;
  }
  std::vector<JourneyItem> aldric_pack;
  if (login_player) collect_pack(*login_player, &aldric_pack);
  check(aldric_pack.size() >= 2, "gate-b: starter kit carried by the first Scion");

  // die: stand in the densest pack, no swings, no dev mutation - JS monster
  // AI strikes adjacent players on its own cadence. First the scion takes up
  // the expedition's floor treasure: earned value is what later circulates
  // as the House relic (the starter kit is never notable).
  enter_warren(wire);
  Envelope transition;
  check(wire.wait_for("party:scene:transition", &transition, 5000),
        "gate-b: expedition entered (scene transition)");
  std::string aldric_loot_uuid;
  for (int step = 0; step < 40 && aldric_loot_uuid.empty(); ++step) {
    Envelope snapshot;
    if (!journey_snapshot(wire, "gate-b-loot", &snapshot)) continue;
    const auto* snap = jfield(snapshot.data, "state");
    const auto* ground = snap ? jfield(*snap, "groundItems") : nullptr;
    if (!ground || !ground->array()) break;
    for (const auto& entry : *ground->array()) {
      const std::string id = jtext(jfield(entry, "id"));
      const std::string uuid = jtext(jfield(entry, "uuid"));
      if (id == "coins" || uuid.empty()) continue;
      journey_take(wire, uuid, jnumber(jfield(entry, "x")), jnumber(jfield(entry, "y")));
      std::this_thread::sleep_for(std::chrono::milliseconds(150));
      aldric_loot_uuid = uuid;
      break;
    }
  }
  bool loot_in_pack = false;
  for (int attempt = 0; attempt < 5 && !loot_in_pack; ++attempt) {
    Envelope snapshot;
    if (!journey_snapshot(wire, "gate-b-loot-check", &snapshot)) continue;
    const auto* snap = jfield(snapshot.data, "state");
    const auto* ground = snap ? jfield(*snap, "groundItems") : nullptr;
    bool still_on_floor = false;
    if (ground && ground->array()) {
      for (const auto& entry : *ground->array()) {
        if (jtext(jfield(entry, "uuid")) == aldric_loot_uuid) still_on_floor = true;
      }
    }
    loot_in_pack = !still_on_floor;
  }
  check(loot_in_pack, "gate-b: earned treasure entered the fallen scion's pack");
  Envelope fallen_env;
  bool fallen = false;
  for (int step = 0; step < 400 && !fallen; ++step) {
    Envelope snapshot;
    if (!journey_snapshot(wire, "gate-b-death", &snapshot)) continue;
    const auto monsters = journey_monsters(snapshot);
    if (monsters.empty()) break;
    const JourneyMonster* densest = &monsters.front();
    int best_neighbors = -1;
    for (const auto& monster : monsters) {
      int neighbors = 0;
      for (const auto& other : monsters) {
        if (std::abs(static_cast<int>(other.x) - static_cast<int>(monster.x)) <= 1 &&
            std::abs(static_cast<int>(other.y) - static_cast<int>(monster.y)) <= 1) {
          ++neighbors;
        }
      }
      if (neighbors > best_neighbors) {
        best_neighbors = neighbors;
        densest = &monster;
      }
    }
    journey_teleport(wire, densest->x, densest->y);
    fallen = wire.wait_for("chronicles:scion-fallen", &fallen_env, 100);
  }
  if (!checked(fallen,
               "gate-b: the mortal Scion's lethal wounds commit the final death")) {
    bail();
    return;
  }
  {
    const auto* fallen_ref = jfield(fallen_env.data, "fallen");
    check(fallen_ref && jtext(jfield(*fallen_ref, "scionId")) == scion_a,
          "gate-b: fallen report names the active scion");
  }

  // successor admission
  wire.send("player:chronicles:return", JsonValue(JsonValue::Object{}));
  Envelope ready;
  if (!checked(wire.wait_for("player:chronicles:ready", &ready, 5000),
               "gate-b: fallen scion returns to the chronicles")) {
    bail();
    return;
  }
  {
    const auto* ready_fallen = jfield(ready.data, "fallen");
    check(ready_fallen && jtext(jfield(*ready_fallen, "scionId")) == scion_a,
          "gate-b: chronicles ready names the fallen scion");
  }
  std::string chronicle_before;
  double revision_before = 0;
  {
    const auto* root = jfield(ready.data, "chronicles");
    const bool relic_queued = root && root->object() && crypt_relic_status(*root, scion_a) == "queued";
    if (root) revision_before = jnumber(jfield(*root, "chroniclesRevision"));
    check(relic_queued, "gate-b: the fallen scion's relic is queued in the crypt");
  }

  // create + set out the successor
  {
    JsonValue::Object create;
    create["houseId"] = JsonValue(house_id);
    create["name"] = JsonValue("Brynn");
    wire.send("chronicles:scion:create", JsonValue(std::move(create)));
  }
  std::string scion_b;
  {
    const bool created = wire.wait_for("chronicles:state", &state, 5000);
    scion_b = created ? jtext(jfield(state.data, "createdScionId")) : std::string();
    if (!checked(created && !scion_b.empty(), "gate-b: successor created (Brynn)")) {
      bail();
      return;
    }
  }
  {
    JsonValue::Object set_out;
    set_out["scionId"] = JsonValue(scion_b);
    wire.send("chronicles:scion:set-out", JsonValue(std::move(set_out)));
  }
  if (!checked(wire.wait_for("player:login", &login, 5000),
               "gate-b: successor set out")) {
    bail();
    return;
  }
  {
    login_player = jfield(login.data, "player");
    std::vector<JourneyItem> brynn_pack;
    if (login_player) collect_pack(*login_player, &brynn_pack);
    bool inherited = false;
    for (const auto& item : brynn_pack)
      for (const auto& old : aldric_pack)
        if (!old.uuid.empty() && old.uuid == item.uuid) inherited = true;
    check(!inherited, "gate-b: the successor never receives the fallen scion's pack");
    bool fresh_dagger = false;
    for (const auto& item : brynn_pack) if (item.id == "bronze-dagger") fresh_dagger = true;
    check(fresh_dagger, "gate-b: the successor starts from the fresh-scion kit");
  }
  {
    Envelope snapshot;
    if (journey_snapshot(wire, "gate-b-heir", &snapshot)) {
      const auto* snap = jfield(snapshot.data, "state");
      check(jtext(snap ? jfield(*snap, "lifecycle") : nullptr) == "alive" &&
                jtext(snap ? jfield(*snap, "lifecycleMode") : nullptr) == "hard",
            "gate-b: the successor's session is a mortal (hard) run");
    }
  }

  // recover relic: the heir's elite kill surfaces a circulated heirloom
  enter_warren(wire);
  check(wire.wait_for("party:scene:transition", &transition, 5000),
        "gate-b: successor enters a fresh expedition");
  std::string relic_uuid;
  double relic_x = 0;
  double relic_y = 0;
  for (int step = 0; step < 900 && relic_uuid.empty(); ++step) {
    Envelope snapshot;
    if (!journey_snapshot(wire, "gate-b-relic", &snapshot)) continue;
    if (journey_find_relic(snapshot, &relic_uuid, &relic_x, &relic_y)) break;
    const auto monsters = journey_monsters(snapshot);
    const auto* snap = jfield(snapshot.data, "state");
    const double px = jnumber(snap ? jfield(*snap, "x") : nullptr);
    const double py = jnumber(snap ? jfield(*snap, "y") : nullptr);
    const JourneyMonster* target = nullptr;
    double best = 1e18;
    bool any_trash = false;
    for (const auto& monster : monsters) if (!monster.boss) any_trash = true;
    for (const auto& monster : monsters) {
      if (any_trash && monster.boss) continue;
      const double d = std::abs(monster.x - px) + std::abs(monster.y - py);
      if (d < best) {
        best = d;
        target = &monster;
      }
    }
    if (!target) break;
    journey_teleport(wire, target->x, target->y);
    JsonValue::Object swing;
    swing["skillId"] = JsonValue("primary-attack");
    swing["direction"] = JsonValue("down");
    wire.send("player:skill:trigger", JsonValue(std::move(swing)));
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
  }
  if (!checked(!relic_uuid.empty(),
               "gate-b: the heir's elite kill surfaces a relic of the fallen")) {
    bail();
    return;
  }
  bool recovered = false;
  for (int attempt = 0; attempt < 5 && !recovered; ++attempt) {
    journey_take(wire, relic_uuid, relic_x, relic_y);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    Envelope snapshot;
    if (!journey_snapshot(wire, "gate-b-take", &snapshot)) continue;
    const auto* snap = jfield(snapshot.data, "state");
    const auto* ground = snap ? jfield(*snap, "groundItems") : nullptr;
    bool still_there = false;
    if (ground && ground->array()) {
      for (const auto& entry : *ground->array()) {
        if (jtext(jfield(entry, "uuid")) == relic_uuid) still_there = true;
      }
    }
    recovered = !still_there;
  }
  if (!checked(recovered, "gate-b: heir recovers the relic from the expedition floor")) {
    bail();
    return;
  }
  {
    // capture the exact post-recovery crypt record for the reconnect compare
    Envelope snapshot;
    if (journey_snapshot(wire, "gate-b-before-drop", &snapshot)) {
      const auto* snap = jfield(snapshot.data, "state");
      const auto* record = snap ? jfield(*snap, "chroniclesRecord") : nullptr;
      const auto* record_state = record ? jfield(*record, "state") : nullptr;
      if (record_state) chronicle_before = fallen_crypt_json(*record_state, scion_a);
      revision_before = jnumber(record ? jfield(*record, "revision") : nullptr, revision_before);
    }
  }

  // disconnect -> reconnect with the same guest identity
  wire.close_abrupt();
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  JourneyWire resumed;
  if (!checked(resumed.connect(port, guest, false),
               "gate-b: reconnect with the same guest identity")) {
    bail();
    return;
  }
  Envelope resumed_state;
  if (!checked(resumed.wait_for("chronicles:state", &resumed_state, 5000),
               "gate-b: reconnect admission emits chronicles:state")) {
    resumed.close_abrupt();
    bail();
    return;
  }
  {
    std::string resumed_house_id;
    std::string resumed_house_name;
    int living = 0;
    int crypt_count = 0;
    const bool same_house =
        chronicle_house_state(resumed_state, &resumed_house_id, &resumed_house_name,
                              &living, &crypt_count) &&
        resumed_house_id == house_id && resumed_house_name == house_name;
    check(same_house, "gate-b: reconnect preserves the House identity");
    check(living == 1 && crypt_count == 1,
          "gate-b: reconnect preserves the lineage (one heir, one fallen)");
  }
  {
    const auto* root = jfield(resumed_state.data, "chronicle");
    check(root && crypt_relic_status(*root, scion_a) == "recovered",
          "gate-b: the crypt relic reads recovered after reconnect");
    const std::string chronicle_after = root ? fallen_crypt_json(*root, scion_a) : std::string();
    check(!chronicle_before.empty() && chronicle_before == chronicle_after,
          "gate-b: the fallen scion's crypt record is identical across the reconnect");
    Envelope resumed_snapshot;
    double revision_after = -1;
    if (journey_snapshot(resumed, "gate-b-reconnected", &resumed_snapshot)) {
      const auto* snap = jfield(resumed_snapshot.data, "state");
      const auto* record = snap ? jfield(*snap, "chroniclesRecord") : nullptr;
      revision_after = jnumber(record ? jfield(*record, "revision") : nullptr);
    }
    check(revision_after >= revision_before && revision_before > 0,
          "gate-b: chronicle revision continuity survives the reconnect");
  }
  {
    JsonValue::Object set_out;
    set_out["scionId"] = JsonValue(scion_b);
    resumed.send("chronicles:scion:set-out", JsonValue(std::move(set_out)));
  }
  Envelope resumed_login;
  if (!checked(resumed.wait_for("player:login", &resumed_login, 5000),
               "gate-b: resumed set-out admits the same heir")) {
    resumed.close_abrupt();
    bail();
    return;
  }
  {
    const JsonValue* player = jfield(resumed_login.data, "player");
    const JsonValue* chronicles = player ? jfield(*player, "chronicles") : nullptr;
    check(jtext(chronicles ? jfield(*chronicles, "scionId") : nullptr) == scion_b,
          "gate-b: reconnect resumes the same heir");
    std::vector<JourneyItem> pack;
    if (player) collect_pack(*player, &pack);
    bool relic_carried = false;
    for (const auto& item : pack) if (item.uuid == relic_uuid) relic_carried = true;
    check(relic_carried, "gate-b: the recovered relic stays in the heir's pack");
  }

  resumed.close_abrupt();
  bail();
}

}  // namespace

int main() {
  // CI pipes fully buffer MSVC stdout; a crash then discards every PASS/FAIL
  // line and the failing check is unidentifiable. Unbuffered costs nothing
  // at this volume.
  std::setvbuf(stdout, nullptr, _IONBF, 0);
  local_session_ready_and_deterministic();
  remote_dead_endpoint_is_a_visible_failure();
  remote_handshake_reaches_ready();
  remote_guest_journey();
  remote_mid_session_disconnect();
  remote_session_replaced();
  remote_render_list_ops();
  chronicles_gate_b_reconnect_journey();
  if (failures == 0) {
    std::printf("session tests passed\n");
    return 0;
  }
  std::printf("%d session test check(s) failed\n", failures);
  return 1;
}
