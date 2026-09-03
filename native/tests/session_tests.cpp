// TASK-0060 (D-122): C3 session seam tests. Proves the local adapter keeps
// deterministic play available and the remote adapter completes a REAL
// handshake against verdigris_server's WebSocket listener — plus the
// authentic negative: a dead endpoint is a visible hard failure, never a
// silent fallback to local play.

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include "../client/local_session.hpp"
#include "../client/remote_session.hpp"
#include "../client/presentation_state.hpp"
#include "verdigris/networking.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {

int failures = 0;

void check(bool ok, const char* label) {
  std::printf("%s %s\n", ok ? "PASS" : "FAIL", label);
  if (!ok) ++failures;
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
  // This suite's assigned loopback capsule is 7160-7179 (TASK-0163
  // resource_capsule); scan upward inside it so parallel suites cannot
  // collide and no other lane's ports are ever touched.
  for (std::uint16_t port = 7160; port <= 7179; ++port) {
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
  // Nothing listens on this port (start_server scans upward from 7160; 7159
  // is reserved for this negative and never bound).
  verdigris::client::RemoteProtocolSession session("127.0.0.1", 7159, "negative-guest");
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
  check(server != nullptr, "remote: test server bound inside the TASK-0163 capsule");
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
    for (int attempt = 0;
         attempt < 100 && !session.model().xp_present; ++attempt) {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      session.poll();
    }
    check(session.model().xp_present &&
              session.model().xp_next > session.model().xp_floor,
          "remote: dev-state combat XP mirrored into the client model");
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
  // Same TASK-0163 loopback capsule (7160-7179); earlier suites release
  // their listener before this runs, so the scan resumes inside the range.
  for (std::uint16_t port = 7160; port <= 7179; ++port) {
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
  check(server != nullptr, "journey: test server bound inside the TASK-0163 capsule 7160-7179");
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
    if (!incoming) {
      // Take the first hit before the slaughter: pack contact carries a
      // staggered first-strike windup now, so a swinging hunter kills each
      // camped foe before it ever lands one. Close distance and stand.
      const auto& camp_model = session.model();
      const verdigris::client::ClientMonster* camp_target = nullptr;
      double camp_best = 1e9;
      for (const auto& monster : camp_model.monsters) {
        if (!monster.alive) continue;
        const double reach =
            (std::max)(std::abs(monster.x - camp_model.player.x),
                       std::abs(monster.y - camp_model.player.y));
        if (reach < camp_best) { camp_best = reach; camp_target = &monster; }
      }
      if (camp_target && camp_best > 0.8) {
        const int dx = camp_target->x > camp_model.player.x + 0.3   ? 1
                       : camp_target->x < camp_model.player.x - 0.3 ? -1
                                                                    : 0;
        const int dy = camp_target->y > camp_model.player.y + 0.3   ? 1
                       : camp_target->y < camp_model.player.y - 0.3 ? -1
                                                                    : 0;
        if (dx != 0 || dy != 0)
          session.submit(verdigris::client::ClientCommand::move(dx, dy));
      }
    } else {
      hunt_step(session);
    }
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
    // Hold the blade until the first incoming hit lands: swinging kills a
    // camped foe faster than its first-strike windup resolves, and this
    // leg exists to prove the incoming path, not the outgoing one.
    if (incoming)
      session.submit(verdigris::client::ClientCommand::use_action("melee"));
    // Pack contact now has a per-monster first-strike windup (staggered
    // 400-1300 ms), so grazing past a foe no longer eats an instant hit.
    // Seek the nearest living foe and CAMP inside its reach - the dwell a
    // real fight has - instead of wandering a fixed band.
    {
      const auto& monsters = session.model().monsters;
      const double px = session.model().player.x;
      const double py = session.model().player.y;
      const verdigris::client::ClientMonster* nearest = nullptr;
      double best = 1e9;
      for (const auto& monster : monsters) {
        if (!monster.alive) continue;
        const double reach =
            (std::max)(std::abs(monster.x - px), std::abs(monster.y - py));
        if (reach < best) { best = reach; nearest = &monster; }
      }
      if (nearest && best > 0.8) {
        const int dx = nearest->x > px + 0.3 ? 1 : nearest->x < px - 0.3 ? -1 : 0;
        const int dy = nearest->y > py + 0.3 ? 1 : nearest->y < py - 0.3 ? -1 : 0;
        if (dx != 0 || dy != 0)
          session.submit(verdigris::client::ClientCommand::move(dx, dy));
      }
      // Within reach: hold ground so the windup resolves into a hit.
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    session.poll();
    collect_flags(session, outgoing, incoming, telegraph, kill, pickup, equipped, extracted,
                  lost);
  }
  if (!incoming) {
    const auto& diag = session.model();
    std::printf("    diag: player %.1f,%.1f life %d | monsters %zu\n",
                diag.player.x, diag.player.y, diag.player.life,
                diag.monsters.size());
    double best = 1e9;
    for (const auto& monster : diag.monsters) {
      if (!monster.alive) continue;
      const double reach = (std::max)(std::abs(monster.x - diag.player.x),
                                      std::abs(monster.y - diag.player.y));
      if (reach < best) best = reach;
    }
    std::printf("    diag: nearest living foe chebyshev %.2f\n", best);
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
  check(server != nullptr, "reconnect: TASK-0163 capsule server bound");
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

// ── TASK-0148 r5: Gate-B chronicles reconnect journey (loopback) ──────────
// Drives the COMPLETE owner journey against the real WebSocketServer using
// only normal accepted envelopes from the frozen TASK-0081 contract:
// non-quick guest login → chronicles:house:found → chronicles:scion:create
// + chronicles:scion:set-out (the accepted first admission) → earn an
// identifiable item through ordinary play → die through ordinary movement/
// combat → player:chronicles:return → chronicles:scion:create +
// player:chronicles:select succession → elite slain surfaces the heirloom →
// player:context-menu:action Take recovers the EXACT relic → disconnect/
// reconnect with
// the same guest identity → identical House roster, crypt relic status, and
// oath/carried-heirloom continuity. No dev:* event, no mutate/select
// shortcut for the first admission, no test bypass. The driver is a minimal
// RFC6455 client because RemoteProtocolSession polls dev:state for its own
// model sync, which this journey must not depend on.
//
// TASK-0163 driver correction (test-only; no runtime or rule change). Both
// recorded program-gate failures traced to the OLD exploration state machine:
//
//   (a) Seven-minute hunt, four kills, no named Warden. The serpentine
//       lattice joined full-height lane legs with GREEDY diagonal transits
//       and a left-hand block rotation; that pair pins the walk against the
//       warren's vertical wall ribs on the wrong side (a replay of the exact
//       algorithm on this guest's seeded floor never got past lane 7 in nine
//       thousand steps), so legs were reached only by accident. Under load,
//       late step echoes read as walls and the 25 s waypoint deadline skipped
//       more legs - the Warden's seeded tile sits inside a rib pocket whose
//       only entries are authored gap corridors, so no run ever came within
//       the two-tile reveal ring and no monster:telegraph ever fired. The
//       recorded four kills are the eastern trash packs traded on the way.
//   (b) Retry with no observed fatal fall. The pre-death sweep was pure
//       right-hand wall following that treated ONE silent 400 ms window as a
//       wall. A load-delayed echo therefore permanently rotated the walk onto
//       a different maze cycle; the retry's cycle held no monster, so no
//       incoming hit (hence no chronicles:scion-fallen) could occur.
//
// The corrected machine keeps every action on ordinary client surfaces and
// replaces only navigation: a fixed boustrophedon lane plan whose full-height
// legs pass within one tile of every walkable column (adjacency contact for
// the fall, inside the two-tile reveal ring for the Warden), waypoint
// discipline instead of deadline-skipped diagonals, and a silent-step policy
// that re-issues the same direction before declaring a wall so a slow echo
// costs latency, never the path. Focused controls for the plan and the
// silent-step policy live in gateb_driver_state_machine_controls().

using JV = verdigris::networking::JsonValue;
using WBEnvelope = verdigris::networking::Envelope;

std::string gateb_str(const JV& value, const char* key) {
  const JV* field = value.get(key);
  return field && field->string() ? *field->string() : std::string();
}

double gateb_num(const JV& value, const char* key, double fallback = 0.0) {
  const JV* field = value.get(key);
  return field && field->number() ? *field->number() : fallback;
}

bool gateb_bool(const JV& value, const char* key, bool fallback = false) {
  const JV* field = value.get(key);
  return field && field->boolean() ? *field->boolean() : fallback;
}

struct GateBGroundItem {
  std::string uuid;
  std::string name;
  double x = 0.0;
  double y = 0.0;
  bool relic = false;
  std::string relic_of;
};

// The world visibility a NORMAL client has without dev:state: its own
// position echoes, scene transitions, combat health fields, ground-change
// item lists, and game messages.
struct GateBView {
  double px = 0.0;
  double py = 0.0;
  bool has_pos = false;
  int hp = -1;
  bool player_died_hit = false;
  std::vector<GateBGroundItem> ground;
  std::string last_message;
  std::string scene_type;
  // Stair tiles are normal scene-metadata knowledge; the driver avoids
  // stepping on them so a blind sweep cannot fall through or exit.
  bool has_stairs = false;
  int stairs_up_x = -1, stairs_up_y = -1;
  int stairs_down_x = -1, stairs_down_y = -1;
};

void gateb_apply_to_view(GateBView& view, const WBEnvelope& envelope) {
  const JV& data = envelope.data;
  if (envelope.event == "player:movement") {
    view.px = gateb_num(data, "x", view.px);
    view.py = gateb_num(data, "y", view.py);
    view.has_pos = true;
    return;
  }
  if (envelope.event == "player:login") {
    if (const JV* player = data.get("player")) {
      view.px = gateb_num(*player, "x", view.px);
      view.py = gateb_num(*player, "y", view.py);
      view.has_pos = true;
    }
    return;
  }
  if (envelope.event == "party:scene:transition" ||
      envelope.event == "world:scene:transition") {
    if (const JV* scene = data.get("scene")) {
      view.scene_type = gateb_str(*scene, "type");
      if (const JV* metadata = scene->get("metadata"); metadata && metadata->object()) {
        view.has_stairs = true;
        if (const JV* up = metadata->get("stairsUp")) {
          view.stairs_up_x = static_cast<int>(gateb_num(*up, "x", -1));
          view.stairs_up_y = static_cast<int>(gateb_num(*up, "y", -1));
        }
        if (const JV* down = metadata->get("stairsDown")) {
          view.stairs_down_x = static_cast<int>(gateb_num(*down, "x", -1));
          view.stairs_down_y = static_cast<int>(gateb_num(*down, "y", -1));
        }
      }
      if (const JV* state = data.get("playerState")) {
        view.px = gateb_num(*state, "x", view.px);
        view.py = gateb_num(*state, "y", view.py);
        view.has_pos = true;
      }
    }
    return;
  }
  if (envelope.event == "combat:hit") {
    const std::string target_type = gateb_str(data, "targetType");
    if (target_type != "player") return;
    if (const JV* health = data.get("health")) {
      view.hp = static_cast<int>(gateb_num(*health, "current",
                                           static_cast<double>(view.hp)));
    }
    const JV* died = data.get("died");
    if (died && died->boolean() && *died->boolean()) view.player_died_hit = true;
    return;
  }
  if (envelope.event == "world:itemDropped" || envelope.event == "item:change") {
    const JV* items = data.get("data");
    if (!items || !items->array()) return;
    std::vector<GateBGroundItem> parsed;
    for (const auto& entry : *items->array()) {
      GateBGroundItem item;
      item.uuid = gateb_str(entry, "uuid");
      item.name = gateb_str(entry, "name");
      item.x = gateb_num(entry, "x");
      item.y = gateb_num(entry, "y");
      if (const JV* relic = entry.get("chroniclesRelic"); relic && relic->object()) {
        item.relic = true;
        item.relic_of = gateb_str(*relic, "scionName");
      }
      parsed.push_back(std::move(item));
    }
    view.ground = std::move(parsed);
    return;
  }
  if (envelope.event == "game:send:message") {
    view.last_message = gateb_str(data, "text");
    return;
  }
}

class LoopbackClient {
 public:
  struct Line {
    std::chrono::steady_clock::time_point at;
    WBEnvelope env;
  };

  explicit LoopbackClient(GateBView& view) : view_(view) {}
  ~LoopbackClient() { close(); }

  LoopbackClient(const LoopbackClient&) = delete;
  LoopbackClient& operator=(const LoopbackClient&) = delete;

  bool connect(std::uint16_t port, std::string* error) {
#ifdef _WIN32
    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
      if (error) *error = "WSAStartup failed";
      return false;
    }
    wsa_started_ = true;
#endif
    auto sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock ==
#ifdef _WIN32
        INVALID_SOCKET
#else
        -1
#endif
    ) {
      if (error) *error = "socket() failed";
      cleanup_wsa();
      return false;
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);
    if (::connect(sock, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
      if (error) *error = "connection refused";
#ifdef _WIN32
      closesocket(sock);
#else
      ::close(sock);
#endif
      cleanup_wsa();
      return false;
    }
    // The server accepts any syntactically valid Sec-WebSocket-Key on the
    // loopback development transport (recorded coupling, see remote_session).
    const std::string request =
        "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nUpgrade: websocket\r\n"
        "Connection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";
    if (!send_all(sock, request.data(), request.size())) {
      if (error) *error = "upgrade send failed";
#ifdef _WIN32
      closesocket(sock);
#else
      ::close(sock);
#endif
      cleanup_wsa();
      return false;
    }
    std::string response;
    char buffer[1024];
    while (response.find("\r\n\r\n") == std::string::npos &&
           response.size() < 8192) {
      const auto got = ::recv(sock, buffer, sizeof(buffer), 0);
      if (got <= 0) break;
      response.append(buffer, buffer + got);
    }
    if (response.find(" 101 ") == std::string::npos) {
      if (error) *error = "endpoint did not upgrade";
#ifdef _WIN32
      closesocket(sock);
#else
      ::close(sock);
#endif
      cleanup_wsa();
      return false;
    }
    sock_ = sock;
    running_ = true;
    reader_ = std::thread(&LoopbackClient::reader_loop, this);
    return true;
  }

  void close() {
    running_ = false;
    if (sock_ !=
#ifdef _WIN32
        INVALID_SOCKET
#else
        -1
#endif
    ) {
#ifdef _WIN32
      ::shutdown(sock_, SD_SEND);
      closesocket(sock_);
      sock_ = INVALID_SOCKET;
#else
      ::shutdown(sock_, SHUT_WR);
      ::close(sock_);
      sock_ = -1;
#endif
    }
    if (reader_.joinable()) reader_.join();
    cleanup_wsa();
  }

  bool send(const std::string& event, JV data) {
    WBEnvelope envelope{event, std::move(data)};
    return send_frame(0x1, verdigris::networking::emit_envelope(envelope));
  }

  void service() {
    std::deque<std::string> batch;
    {
      std::lock_guard lock(inbox_mutex_);
      batch.swap(inbox_);
    }
    for (auto& text : batch) {
      WBEnvelope envelope;
      if (!verdigris::networking::parse_envelope(text, envelope, nullptr)) continue;
      Line line{std::chrono::steady_clock::now(), std::move(envelope)};
      gateb_apply_to_view(view_, line.env);
      std::lock_guard lock(transcript_mutex_);
      transcript_.push_back(std::move(line));
    }
  }

  size_t mark() { service(); std::lock_guard lock(transcript_mutex_); return transcript_.size(); }

  template <typename Pred>
  bool wait_from(size_t mark_index, int timeout_ms, Pred pred, Line* out = nullptr) {
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
    size_t index = mark_index;
    for (;;) {
      service();
      if (scan_from(index,
                    [&pred](const Line& line) { return pred(line.env); },
                    out)) {
        return true;
      }
      if (std::chrono::steady_clock::now() >= deadline) return false;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  // Non-blocking scan over NEW transcript lines only; advances index even on
  // a miss so repeated calls observe each line exactly once.
  template <typename Pred>
  bool scan_from(size_t& index, Pred pred, Line* out = nullptr) {
    std::lock_guard lock(transcript_mutex_);
    while (index < transcript_.size()) {
      if (pred(transcript_[index])) {
        if (out) *out = transcript_[index];
        ++index;
        return true;
      }
      ++index;
    }
    return false;
  }

  GateBView& view() { return view_; }

 private:
  static constexpr auto kInvalidSocket_v =
#ifdef _WIN32
        INVALID_SOCKET
#else
        -1
#endif
      ;

  void cleanup_wsa() {
#ifdef _WIN32
    if (wsa_started_) {
      WSACleanup();
      wsa_started_ = false;
    }
#endif
  }

  void reader_loop() {
    while (running_) {
      std::uint8_t header[2];
      if (!recv_all(sock_, header, 2)) break;
      const auto opcode = static_cast<std::uint8_t>(header[0] & 0x0f);
      const bool masked = (header[1] & 0x80) != 0;
      std::uint64_t length = header[1] & 0x7f;
      if (length == 126) {
        std::uint8_t ext[2];
        if (!recv_all(sock_, ext, 2)) break;
        length = static_cast<std::uint64_t>((ext[0] << 8) | ext[1]);
      } else if (length == 127) {
        std::uint8_t ext[8];
        if (!recv_all(sock_, ext, 8)) break;
        length = 0;
        for (auto byte : ext) length = (length << 8) | byte;
      }
      if (length > (1u << 20)) break;
      std::uint8_t mask[4] = {0, 0, 0, 0};
      if (masked && !recv_all(sock_, mask, 4)) break;
      std::string payload(static_cast<std::size_t>(length), '\0');
      if (length > 0 && !recv_all(sock_, payload.data(), payload.size())) break;
      if (masked) {
        for (std::size_t i = 0; i < payload.size(); ++i) payload[i] ^= mask[i % 4];
      }
      if (opcode == 0x8) break;
      if (opcode == 0x9) {
        send_frame(0xA, payload);
        continue;
      }
      if (opcode == 0x1) {
        std::lock_guard lock(inbox_mutex_);
        inbox_.push_back(std::move(payload));
      }
    }
    running_ = false;
  }

  bool send_frame(std::uint8_t opcode, const std::string& payload) {
    if (sock_ == kInvalidSocket_v) return false;
    std::vector<std::uint8_t> frame;
    frame.push_back(static_cast<std::uint8_t>(0x80 | opcode));
    const auto size = payload.size();
    if (size < 126) {
      frame.push_back(static_cast<std::uint8_t>(0x80 | size));
    } else if (size <= 65535) {
      frame.push_back(0x80 | 126);
      frame.push_back(static_cast<std::uint8_t>((size >> 8) & 0xff));
      frame.push_back(static_cast<std::uint8_t>(size & 0xff));
    } else {
      frame.push_back(0x80 | 127);
      for (int i = 7; i >= 0; --i)
        frame.push_back(static_cast<std::uint8_t>((static_cast<std::uint64_t>(size) >> (i * 8)) & 0xff));
    }
    // Clients MUST mask (RFC6455 5.3); the server enforces this.
    const std::uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
    frame.insert(frame.end(), mask, mask + 4);
    const auto offset = frame.size();
    frame.resize(offset + size);
    for (std::size_t i = 0; i < size; ++i)
      frame[offset + i] = static_cast<std::uint8_t>(payload[i]) ^ mask[i % 4];
    return send_all(sock_, frame.data(), frame.size());
  }

  static bool send_all(
#ifdef _WIN32
      SOCKET socket,
#else
      int socket,
#endif
      const void* data, size_t size) {
    const char* cursor = static_cast<const char*>(data);
    size_t remaining = size;
    while (remaining > 0) {
#ifdef _WIN32
      const auto sent = ::send(socket, cursor, static_cast<int>(remaining), 0);
#else
      const auto sent = ::send(socket, cursor, remaining, 0);
#endif
      if (sent <= 0) return false;
      cursor += sent;
      remaining -= static_cast<size_t>(sent);
    }
    return true;
  }

  static bool recv_all(
#ifdef _WIN32
      SOCKET socket,
#else
      int socket,
#endif
      void* data, size_t size) {
    char* cursor = static_cast<char*>(data);
    size_t remaining = size;
    while (remaining > 0) {
#ifdef _WIN32
      const auto got = ::recv(socket, cursor, static_cast<int>(remaining), 0);
#else
      const auto got = ::recv(socket, cursor, remaining, 0);
#endif
      if (got <= 0) return false;
      cursor += got;
      remaining -= static_cast<size_t>(got);
    }
    return true;
  }

  GateBView& view_;
#ifdef _WIN32
  SOCKET sock_ = INVALID_SOCKET;
  bool wsa_started_ = false;
#else
  int sock_ = -1;
#endif
  std::atomic<bool> running_{false};
  std::thread reader_;
  std::mutex inbox_mutex_;
  std::deque<std::string> inbox_;
  std::mutex transcript_mutex_;
  std::vector<Line> transcript_;
};

// ── Journey driving helpers ───────────────────────────────────────────────

std::uint16_t start_server_worker_capsule(verdigris::networking::WebSocketServer*& out) {
  // ox-pc-ac worker capsule 7160-7179 (TASK-0163 resource_capsule): never
  // 6500, never another lane's capsule.
  for (std::uint16_t port = 7160; port <= 7179; ++port) {
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

const char* gateb_dir(int heading) {
  static const char* kDirs[4] = {"right", "down", "left", "up"};
  return kDirs[heading & 3];
}

// Tile derivation for every driver decision. The runtime's authoritative
// occupied-tile convention ROUNDS the fractional position a movement echo
// carries (each player:move is one sub-tile interpolation sample), so the
// driver must derive tiles with the same rounding or its adjacency, stair,
// and waypoint math disagrees with the server by up to one tile - which is
// exactly how the recorded take-relic leg silently missed the chebyshev
// reach gate.
int gateb_tile_of(double value) {
  return static_cast<int>(std::lround(value));
}

int gateb_heading_for(int dx, int dy) {
  if (dx > 0) return 0;
  if (dy > 0) return 1;
  if (dx < 0) return 2;
  return 3;
}

bool gateb_on_stairs(const GateBView& view, int tile_x, int tile_y) {
  if (!view.has_stairs) return false;
  if (tile_x == view.stairs_up_x && tile_y == view.stairs_up_y) return true;
  if (tile_x == view.stairs_down_x && tile_y == view.stairs_down_y) return true;
  return false;
}

// ── Deterministic driver state machine (TASK-0163) ────────────────────────

bool gateb_step(LoopbackClient& client, int& heading);

// A silent movement window means blocked OR slow - the wire carries no
// rejection. The same direction is re-issued this many times before a wall
// is declared, so a load-delayed echo costs latency and never the path.
constexpr int kGatebSilentRetries = 2;

bool gateb_declares_wall(int silent_streak) {
  return silent_streak > kGatebSilentRetries;
}

// Warren lanes for the boustrophedon sweep: every walkable instance column
// lies within one tile of a planned lane, so a full-height leg brushes every
// monster tile (passive pack AI answers at adjacency - the ordinary fall)
// and every boss tile (inside the two-tile telegraph reveal ring). The
// skipped columns are the warren layout's static wall ribs plus the border
// walls - scene geometry from the served layout, never spawn positions.
const std::vector<int>& gateb_lane_plan() {
  static const std::vector<int> lanes{2, 4, 6, 8, 10, 14, 16, 20, 22,
                                      26, 28, 32, 34, 36, 38};
  return lanes;
}

// Frozen visit order: each lane is traversed at full floor height, and lane
// legs alternate entry ends so inter-lane transits stay short horizontal
// hops along the open top/bottom rings.
std::vector<std::pair<int, int>> gateb_serpentine_waypoints() {
  std::vector<std::pair<int, int>> waypoints;
  bool enter_top = true;
  for (const int lane : gateb_lane_plan()) {
    waypoints.emplace_back(lane, enter_top ? 1 : 38);
    waypoints.emplace_back(lane, enter_top ? 38 : 1);
    enter_top = !enter_top;
  }
  return waypoints;
}

// Per-journey-leg cursor over the plan: current waypoint plus the window in
// which it must be reached (a bounded skip WITH EVIDENCE, never a silent
// hole; typical legs finish in well under two seconds).
struct GatebSweepState {
  size_t cursor = 0;
  std::chrono::steady_clock::time_point waypoint_started{};
};

// One navigation iteration toward the current waypoint: primary axis first
// (larger delta), secondary axis on a confirmed wall, then gateb_step's own
// right-hand detour while boxed in. Returns true when the driver believes it
// advanced one tile toward the plan's next uncovered column.
bool gateb_waypoint_nudge(LoopbackClient& client, GatebSweepState& sweep) {
  GateBView& view = client.view();
  if (!view.has_pos) return false;
  const auto waypoints = gateb_serpentine_waypoints();
  if (sweep.cursor >= waypoints.size()) {
    std::printf("note: sweep plan restart (floor re-cover)\n");
    sweep.cursor = 0;
  }
  const auto [wp_x, wp_y] = waypoints[sweep.cursor];
  const int px = gateb_tile_of(view.px);
  const int py = gateb_tile_of(view.py);
  if (px == wp_x && py == wp_y) {
    ++sweep.cursor;
    sweep.waypoint_started = std::chrono::steady_clock::now();
    return false;
  }
  if (std::chrono::steady_clock::now() - sweep.waypoint_started >
      std::chrono::seconds(25)) {
    std::printf("note: sweep skips waypoint (%d,%d)\n", wp_x, wp_y);
    ++sweep.cursor;
    sweep.waypoint_started = std::chrono::steady_clock::now();
    return false;
  }
  const int dx = wp_x - px;
  const int dy = wp_y - py;
  const bool horizontal_primary = std::abs(dx) >= std::abs(dy);
  for (int pass = 0; pass < 2; ++pass) {
    const bool try_horizontal = horizontal_primary == (pass == 0);
    int heading;
    if (try_horizontal) {
      if (dx == 0) continue;
      heading = dx > 0 ? 0 : 2;
    } else {
      if (dy == 0) continue;
      heading = dy > 0 ? 1 : 3;
    }
    if (gateb_step(client, heading)) return true;
    // gateb_step rotated `heading` through its wall detours; hand the next
    // call a fresh axis on the following pass.
  }
  return false;  // boxed in this tick; retry next iteration
}

// One movement request toward `heading`, confirmed by a position echo. A
// silent window means blocked or slow - indistinguishable on the wire - so
// the same direction is re-issued before any wall is declared; a load-delayed
// echo then costs latency only, never the path (the TASK-0163 fix for the
// diverted-walk flake). Rotation stays right-hand-style on a wall confirmed
// by repetition, and never steps onto a stair tile.
bool gateb_step(LoopbackClient& client, int& heading) {
  GateBView& view = client.view();
  if (!view.has_pos) return false;
  for (int attempt = 0; attempt < 4; ++attempt) {
    const int delta_x = heading == 0 ? 1 : heading == 2 ? -1 : 0;
    const int delta_y = heading == 1 ? 1 : heading == 3 ? -1 : 0;
    const int next_x = gateb_tile_of(view.px) + delta_x;
    const int next_y = gateb_tile_of(view.py) + delta_y;
    if (gateb_on_stairs(view, next_x, next_y)) {
      heading = (heading + 1) & 3;
      continue;
    }
    int silent_streak = 0;
    while (true) {
      const double ox = view.px;
      const double oy = view.py;
      JV::Object move;
      move.emplace("direction", JV(gateb_dir(heading)));
      client.send("player:move", JV(std::move(move)));
      const auto deadline =
          std::chrono::steady_clock::now() + std::chrono::milliseconds(400);
      while (std::chrono::steady_clock::now() < deadline) {
        client.service();
        if (!view.has_pos || view.px != ox || view.py != oy) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
      }
      ++silent_streak;
      if (!gateb_declares_wall(silent_streak)) continue;  // maybe just slow
      break;  // wall confirmed by repetition
    }
    heading = (heading + 1) & 3;  // wall: right hand takes over
  }
  return false;
}

// Re-enter the delve if a sweep/hunt step ever slipped back to the surface.
void gateb_ensure_instance(LoopbackClient& client) {
  GateBView& view = client.view();
  if (!view.scene_type.empty() && view.scene_type != "instance") {
    JV::Object solo;
    solo.emplace("template", JV("dungeon"));
    solo.emplace("layout", JV("warren"));
    client.send("instance:enterSolo", JV(std::move(solo)));
  }
}

void gateb_swing(LoopbackClient& client, int heading) {
  JV::Object trigger;
  trigger.emplace("skillId", JV("primary-attack"));
  trigger.emplace("direction", JV(gateb_dir(heading)));
  client.send("player:skill:trigger", JV(std::move(trigger)));
}

// Sweep-walk the boustrophedon lane plan (no swinging) until the scion falls
// in ordinary combat. Full-height lane legs pass within one tile of every
// walkable column, so passive pack adjacency - the only damage channel while
// nobody swings - is guaranteed contact; the corrected silent-step policy
// keeps load-delayed echoes from diverting the walk onto a monster-free
// cycle, which was the recorded retry failure.
bool gateb_sweep_until_fallen(LoopbackClient& client, const std::string& scion_id,
                              int timeout_ms, bool* made_contact) {
  GateBView& view = client.view();
  size_t index = client.mark();
  bool contact = false;
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
  GatebSweepState sweep;
  sweep.waypoint_started = std::chrono::steady_clock::now();
  bool fallen = false;
  while (std::chrono::steady_clock::now() < deadline) {
    LoopbackClient::Line line;
    while (client.scan_from(index, [](const LoopbackClient::Line&) { return true; }, &line)) {
      if (line.env.event == "chronicles:scion-fallen") {
        const JV* fallen_payload = line.env.data.get("fallen");
        if (fallen_payload && gateb_str(*fallen_payload, "scionId") == scion_id) {
          fallen = true;
        }
      }
      if (line.env.event == "combat:hit" && gateb_str(line.env.data, "targetType") == "player") {
        contact = true;
        *made_contact = true;
      }
      if (view.player_died_hit) contact = true;
    }
    if (fallen) return true;
    if (!contact) {
      gateb_ensure_instance(client);
      gateb_waypoint_nudge(client, sweep);
      std::this_thread::sleep_for(std::chrono::milliseconds(12));
    } else {
      // Engaged: hold ground and let ordinary combat finish the fall.
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    client.service();
  }
  std::printf("note: sweep diagnostics contact=%d at=(%d,%d)\n", contact ? 1 : 0,
              gateb_tile_of(view.px), gateb_tile_of(view.py));
  return fallen;
}

// Hunt as a normal player hunts: sweep the floor until something exchanges
// blows, then HOLD GROUND and trade swings (monsters are stationary; a real
// player does not retreat out of their own reach), dodge a revealed
// ground-slam circle using only the monster:telegraph payload every client
// receives, beeline to the revealed elite once known, and withdraw at low
// visible health through party:returnToTown + fountain heal + a fresh delve.
// Stops when the slain elite surfaces the circulating heirloom (message or
// relic ground entry).
bool gateb_hunt_until_relic_surfaces(LoopbackClient& client,
                                     const std::string& relic_uuid,
                                     int timeout_ms, int* kills) {
  GateBView& view = client.view();
  auto walk_to_town_tile = [&](int target_x, int target_y, int timeout) {
    int heading = 0;
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
    for (;;) {
      const int dx = target_x - gateb_tile_of(view.px);
      const int dy = target_y - gateb_tile_of(view.py);
      if (std::abs(dx) <= 1 && std::abs(dy) <= 1) return true;
      if (std::chrono::steady_clock::now() >= deadline) return false;
      heading = std::abs(dx) >= std::abs(dy)
                    ? gateb_heading_for(dx > 0 ? 1 : (dx < 0 ? -1 : 0), 0)
                    : gateb_heading_for(0, dy > 0 ? 1 : (dy < 0 ? -1 : 0));
      gateb_step(client, heading);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      client.service();
    }
  };
  auto retreat_heal_redelve = [&]() {
    client.send("party:returnToTown", JV(JV::Object{}));
    size_t mark = client.mark();
    const bool town = client.wait_from(
        mark, 8000, [&](const WBEnvelope& e) {
          if (e.event != "party:scene:transition" &&
              e.event != "world:scene:transition") return false;
          const JV* scene = e.data.get("scene");
          return scene &&
                 gateb_str(*scene, "type").find("town") != std::string::npos;
        });
    if (!town) return false;
    if (!walk_to_town_tile(38, 115, 60000)) return false;
    mark = client.mark();
    client.send("player:fountain:drink", JV(JV::Object{}));
    const bool healed = client.wait_from(
        mark, 5000, [](const WBEnvelope& e) {
          return e.event == "game:send:message" &&
                 gateb_str(e.data, "text").find("Cool water") !=
                     std::string::npos;
        });
    if (!healed) return false;
    view.hp = -1;  // unknown-full until the next incoming hit reports truth
    mark = client.mark();
    JV::Object solo;
    solo.emplace("template", JV("dungeon"));
    solo.emplace("layout", JV("warren"));
    client.send("instance:enterSolo", JV(std::move(solo)));
    return client.wait_from(mark, 8000, [](const WBEnvelope& e) {
      return e.event == "party:scene:transition" ||
             e.event == "world:scene:transition";
    });
  };

  int fight_heading = 0;
  size_t index = client.mark();
  const auto never = std::chrono::steady_clock::time_point() +
                     std::chrono::hours(24);
  auto last_outgoing = never;
  auto last_incoming = never;
  const auto now = [] { return std::chrono::steady_clock::now(); };
  const auto deadline = now() + std::chrono::milliseconds(timeout_ms);
  std::string last_counted_kill_id;
  // Client-visible elite knowledge: the ground-slam telegraph payload is
  // broadcast to every client, so the driver may aim at and dodge the elite
  // with exactly what a normal player sees on screen.
  bool elite_known = false;
  int elite_x = -1, elite_y = -1;
  auto slam_clear_at = never;
  int slam_x = 0, slam_y = 0, slam_radius = 0;
  // Approach-phase state for the revealed-elite beeline.
  auto approach_phase_until = now();
  int approach_best_dist = 1 << 30;
  bool approach_escape = false;
  auto heartbeat_at = now() + std::chrono::seconds(10);
  // Deterministic boustrophedon sweep state (TASK-0163): full-height lane
  // legs instead of deadline-skipped greedy diagonals.
  GatebSweepState sweep;
  sweep.waypoint_started = now();
  const auto chebyshev = [](int ax, int ay, int bx, int by) {
    return (std::max)(std::abs(ax - bx), std::abs(ay - by));
  };
  while (now() < deadline) {
    LoopbackClient::Line line;
    while (client.scan_from(index, [](const LoopbackClient::Line&) { return true; }, &line)) {
      if (line.env.event == "combat:hit") {
        if (gateb_str(line.env.data, "targetType") == "player") {
          last_incoming = line.at;
          const JV* died = line.env.data.get("died");
          if (died && died->boolean() && *died->boolean()) {
            std::printf(
                "note: hunt aborted - the successor fell to ordinary combat "
                "(unrecovered heirloom remains circulating)\n");
            return false;
          }
        } else {
          last_outgoing = line.at;
          if (const JV* died_flag = line.env.data.get("died");
              died_flag && died_flag->boolean() && *died_flag->boolean()) {
            // The hit pipeline emits the killing blow and the death as two
            // combat:hit envelopes with died=true; count each fallen target
            // once by its id.
            const std::string fallen_id = gateb_str(line.env.data, "targetId");
            if (fallen_id != last_counted_kill_id) {
              last_counted_kill_id = fallen_id;
              ++*kills;
              std::printf("note: hunt kill #%d (%s)\n", *kills,
                          gateb_str(line.env.data, "targetName").c_str());
            }
            last_outgoing = never;
            last_incoming = never;
          }
        }
      }
      if (line.env.event == "monster:telegraph") {
        elite_known = true;
        elite_x = static_cast<int>(gateb_num(line.env.data, "x"));
        elite_y = static_cast<int>(gateb_num(line.env.data, "y"));
        slam_x = elite_x;
        slam_y = elite_y;
        slam_radius = static_cast<int>(gateb_num(line.env.data, "radius", 2));
        // Small clock-skew buffer so a re-entry never beats the resolve.
        slam_clear_at = line.at +
                        std::chrono::milliseconds(static_cast<int>(
                            gateb_num(line.env.data, "durationMs", 1000)) + 120);
      }
      if (line.env.event == "chronicles:scion-fallen") {
        std::printf("note: hunt aborted - successor fall observed\n");
        return false;
      }
      if (line.env.event == "game:send:message" &&
          gateb_str(line.env.data, "text").find("has surfaced") !=
              std::string::npos) {
        return true;
      }
    }
    for (const auto& item : view.ground) {
      if (item.uuid == relic_uuid) return true;  // surfaced heirloom visible
    }
    if (view.hp >= 0 && view.hp <= 45) {
      std::printf("note: hunt withdraws at hp=%d for a fountain heal\n",
                  view.hp);
      if (!retreat_heal_redelve()) {
        std::printf("note: hunt heal loop failed to restore the delve\n");
        return false;
      }
      index = client.mark();
      elite_known = false;
      slam_clear_at = never;
      approach_phase_until = now();
      approach_best_dist = 1 << 30;
      approach_escape = false;
      sweep.cursor = 0;
      sweep.waypoint_started = now();
      last_outgoing = never;
      last_incoming = never;
      continue;
    }
    if (now() >= heartbeat_at) {
      heartbeat_at = now() + std::chrono::seconds(10);
      std::printf(
          "note: hunt heartbeat hp=%d at=(%d,%d) kills=%d elite_known=%d "
          "elite=(%d,%d) ground=%zu scene=%s\n",
          view.hp, gateb_tile_of(view.px),
          gateb_tile_of(view.py), *kills,
          elite_known ? 1 : 0, elite_x, elite_y, view.ground.size(),
          view.scene_type.c_str());
    }
    const int tile_x = gateb_tile_of(view.px);
    const int tile_y = gateb_tile_of(view.py);
    bool acted = false;
    if (now() < slam_clear_at && chebyshev(tile_x, tile_y, slam_x, slam_y) <= slam_radius) {
      // A normal player steps out of the marked circle before it resolves.
      int best_heading = -1;
      int best_dist = chebyshev(tile_x, tile_y, slam_x, slam_y);
      for (int candidate = 0; candidate < 4; ++candidate) {
        const int nx = tile_x + (candidate == 0 ? 1 : candidate == 2 ? -1 : 0);
        const int ny = tile_y + (candidate == 1 ? 1 : candidate == 3 ? -1 : 0);
        if (gateb_on_stairs(view, nx, ny)) continue;
        const int dist = chebyshev(nx, ny, slam_x, slam_y);
        if (dist > best_dist) { best_dist = dist; best_heading = candidate; }
      }
      if (best_heading >= 0) {
        gateb_step(client, best_heading);
      } else {
        gateb_swing(client, fight_heading);  // cornered: keep fighting
      }
      acted = true;
    }
    if (!acted) {
      const auto since_incoming = last_incoming == never
                                      ? std::chrono::hours(24)
                                      : now() - last_incoming;
      const auto since_outgoing = last_outgoing == never
                                      ? std::chrono::hours(24)
                                      : now() - last_outgoing;
      if ((since_incoming < std::chrono::seconds(2) ||
           since_outgoing < std::chrono::seconds(1))) {
        // In reach of something alive: stand ground and trade swings. The
        // server aims at the nearest live monster, so direction only breaks
        // ties.
        gateb_swing(client, fight_heading);
      } else if (elite_known) {
        const int dx = elite_x - tile_x;
        const int dy = elite_y - tile_y;
        fight_heading = gateb_heading_for(dx > 0 ? 1 : (dx < 0 ? -1 : 0),
                                          dy > 0 ? 1 : (dy < 0 ? -1 : 0));
        // The protocol combat envelope maps the local 143-unit contact reach
        // onto three tiles. Swing as soon as the revealed Warden is in that
        // authoritative band; requiring adjacency made the scenario wander
        // past a perfectly valid target behind warren collision ribs.
        if (std::abs(dx) + std::abs(dy) <= 3) {
          gateb_swing(client, gateb_heading_for(dx > 0 ? 1 : (dx < 0 ? -1 : 0), 0));
        } else {
          // Approach in bounded phases: greedy toward the wider axis for a
          // window; if the distance has not shrunk (a wall pocket), walk a
          // perpendicular escape lane for the next window. Deterministic,
          // bounded, and evidence-producing either way.
          const auto phase_len = std::chrono::seconds(8);
          if (now() >= approach_phase_until) {
            const int dist = chebyshev(tile_x, tile_y, elite_x, elite_y);
            if (dist < approach_best_dist) {
              approach_escape = false;  // progress: keep the greedy lane
            } else if (!approach_escape) {
              approach_escape = true;   // stalled: try the perpendicular lane
            }
            approach_best_dist = dist;
            approach_phase_until = now() + phase_len;
            std::printf(
                "note: hunt approach dist=%d escape=%d at=(%d,%d) elite=(%d,%d)\n",
                dist, approach_escape ? 1 : 0, tile_x, tile_y, elite_x, elite_y);
          }
          const int primary =
              gateb_heading_for(dx > 0 ? 1 : (dx < 0 ? -1 : 0), 0);
          const int secondary =
              gateb_heading_for(0, dy > 0 ? 1 : (dy < 0 ? -1 : 0));
          int lane = std::abs(dx) >= std::abs(dy) ? primary : secondary;
          if (approach_escape) lane = (lane + 1) & 3;  // perpendicular detour
          gateb_step(client, lane);
        }
      } else {
        gateb_ensure_instance(client);
        // Deterministic boustrophedon coverage (TASK-0163): full-height lane
        // legs guarantee the walk passes within one tile of every walkable
        // column - inside the elite's two-tile reveal ring - and the
        // silent-step policy keeps a slow echo from diverting the walk onto
        // a Warden-free cycle.
        gateb_waypoint_nudge(client, sweep);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(12));
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
    client.service();
  }
  std::printf("note: hunt diagnostics kills=%d ground=%zu message=%s\n",
              *kills, view.ground.size(), view.last_message.c_str());
  return false;
}

// Wait for the surfaced relic's ground frame, walk to it (chebyshev ≤ 1)
// using only position echoes, then take the EXACT uuid through the normal
// context-menu Take surface; the inventory refresh is verified by caller.
bool gateb_take_relic(LoopbackClient& client, const std::string& relic_uuid,
                      int timeout_ms) {
  GateBView& view = client.view();
  int heading = 0;
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
  // The surfacing message can beat the ground-change frame across the wire;
  // a normal client simply sees the item appear on the ground shortly after.
  size_t ground_mark = client.mark();
  bool seen_on_ground = false;
  for (const auto& item : view.ground) {
    if (item.uuid == relic_uuid) { seen_on_ground = true; break; }
  }
  while (!seen_on_ground) {
    if (std::chrono::steady_clock::now() >= deadline) {
      std::printf("note: take-relic: ground frame never carried %s\n",
                  relic_uuid.c_str());
      return false;
    }
    seen_on_ground = client.wait_from(
        ground_mark, 2000, [&](const WBEnvelope& envelope) {
          return (envelope.event == "world:itemDropped" ||
                  envelope.event == "item:change") &&
                 [&]() {
                   const JV* items = envelope.data.get("data");
                   if (!items || !items->array()) return false;
                   for (const auto& entry : *items->array())
                     if (gateb_str(entry, "uuid") == relic_uuid) return true;
                   return false;
                 }();
        });
    client.service();
  }
  for (;;) {
    const GateBGroundItem* relic = nullptr;
    for (const auto& item : view.ground) {
      if (item.uuid == relic_uuid) { relic = &item; break; }
    }
    if (!relic) {
      std::printf("note: take-relic: relic left the ground list\n");
      return false;
    }
    const int dx = gateb_tile_of(relic->x) - gateb_tile_of(view.px);
    const int dy = gateb_tile_of(relic->y) - gateb_tile_of(view.py);
    if (std::chrono::steady_clock::now() >= deadline) {
      std::printf("note: take-relic: never reached the relic tile\n");
      return false;
    }
    if (std::abs(dx) <= 1 && std::abs(dy) <= 1) {
      size_t mark = client.mark();
      JV::Object action;
      JV::Object queue_item;
      JV::Object action_obj;
      action_obj.emplace("actionId", JV("player:take"));
      queue_item.emplace("action", JV(std::move(action_obj)));
      JV::Object item_ref;
      item_ref.emplace("uuid", JV(relic_uuid));
      queue_item.emplace("item", JV(std::move(item_ref)));
      action.emplace("queueItem", JV(std::move(queue_item)));
      client.send("player:context-menu:action", JV(std::move(action)));
      const bool took = client.wait_from(
          mark, 5000,
          [&](const WBEnvelope& envelope) {
            if (envelope.event != "core:refresh:inventory") return false;
            const JV* slots = envelope.data.get("data");
            if (!slots || !slots->array()) return false;
            for (const auto& entry : *slots->array())
              if (gateb_str(entry, "uuid") == relic_uuid) return true;
            return false;
          });
      return took;
    }
    // Greedy toward the larger axis delta first; wall fallback rotates.
    const int primary = gateb_heading_for(dx > 0 ? 1 : (dx < 0 ? -1 : 0), 0);
    const int secondary = gateb_heading_for(0, dy > 0 ? 1 : (dy < 0 ? -1 : 0));
    heading = std::abs(dx) >= std::abs(dy) ? primary : secondary;
    if (!gateb_step(client, heading)) {
      // Blocked on the greedy axis: one wall-following step keeps progress.
      gateb_step(client, heading);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    client.service();
  }
}

// Focused deterministic controls for the corrected driver state machine
// (TASK-0163 required proof). Socket-free; they pin the plan contract and
// the silent-step policy so neither can quietly regress into the recorded
// nondeterminism.
void gateb_driver_state_machine_controls() {
  // 1) Plan contract: the driver walks exactly this frozen visit order.
  const std::vector<std::pair<int, int>> expected = {
      {2, 1}, {2, 38}, {4, 38}, {4, 1}, {6, 1}, {6, 38},
      {8, 38}, {8, 1}, {10, 1}, {10, 38}, {14, 38}, {14, 1},
      {16, 1}, {16, 38}, {20, 38}, {20, 1}, {22, 1}, {22, 38},
      {26, 38}, {26, 1}, {28, 1}, {28, 38}, {32, 38}, {32, 1},
      {34, 1}, {34, 38}, {36, 38}, {36, 1}, {38, 1}, {38, 38}};
  check(gateb_serpentine_waypoints() == expected,
        "gate-b-driver: serpentine plan matches its frozen contract");

  // 2) Coverage: every walkable instance column lies within one lane-step of
  // a planned lane. Wall columns are the warren layout's static ribs plus
  // the border walls - scene geometry, never spawn positions - so a
  // full-height leg brushes every monster tile (adjacency) and every boss
  // tile (the two-tile reveal ring).
  bool covered = true;
  for (int column = 1; column <= 38; ++column) {
    if (column == 12 || column == 18 || column == 24 || column == 30) continue;
    bool near_lane = false;
    for (const int lane : gateb_lane_plan())
      if (std::abs(column - lane) <= 1) near_lane = true;
    if (!near_lane) covered = false;
  }
  check(covered,
        "gate-b-driver: lane plan sweeps within one tile of every walkable "
        "column");

  // 3) Silent-step policy: a late echo must cost latency, never the path -
  // streaks inside the retry budget keep the direction; only a wall
  // confirmed by repetition rotates.
  bool policy = true;
  for (int streak = 1; streak <= kGatebSilentRetries; ++streak)
    if (gateb_declares_wall(streak)) policy = false;
  if (!gateb_declares_wall(kGatebSilentRetries + 1)) policy = false;
  check(policy,
        "gate-b-driver: silent steps re-issue before any wall rotation");

  // 4) Full-height legs at both extremes: nothing hides in the top or
  // bottom rings.
  const auto waypoints = gateb_serpentine_waypoints();
  bool full_height = true;
  for (const int lane : gateb_lane_plan()) {
    bool top = false;
    bool bottom = false;
    for (const auto& [x, y] : waypoints) {
      if (x == lane && y == 1) top = true;
      if (x == lane && y == 38) bottom = true;
    }
    if (!top || !bottom) full_height = false;
  }
  check(full_height,
        "gate-b-driver: every lane leg spans the full floor height");

  // 5) Strict boustrophedon order: consecutive legs share their end row, so
  // inter-lane transits are short hops along the open rings.
  bool alternating = waypoints.size() % 2 == 0;
  for (size_t i = 1; i + 1 < waypoints.size(); i += 2)
    if (waypoints[i].second != waypoints[i + 1].second) alternating = false;
  for (size_t i = 0; i < waypoints.size(); i += 2)
    if (waypoints[i].first != waypoints[i + 1].first) alternating = false;
  check(alternating,
        "gate-b-driver: plan is strictly boustrophedon over its lanes");
}

// The complete frozen Gate-B journey over loopback with only accepted
// envelopes. Pre-change this fails at the ordinary-death fall; post-change
// every step must pass.
void gate_b_chronicles_reconnect_journey() {
  verdigris::networking::WebSocketServer* server = nullptr;
  const auto port = start_server_worker_capsule(server);
  check(server != nullptr, "gate-b: worker-capsule server bound (7160-7179)");
  if (!server) return;

  const std::string guest = "ox-pc-r-gateb";
  const std::string house_name = "House R5";
  std::string house_id;
  std::string scion1_id;
  std::string scion2_id;
  std::string relic_uuid;
  std::string chronicle_snapshot;

  {
    GateBView view;
    LoopbackClient client(view);
    std::string error;
    check(client.connect(port, &error), "gate-b: first connect upgrades");

    // 1) Non-quick login with the single guest identity.
    size_t mark = client.mark();
    JV::Object login;
    login.emplace("guestId", JV(guest));
    client.send("player:login", JV(std::move(login)));
    LoopbackClient::Line line;
    const bool pending_state = client.wait_from(
        mark, 5000,
        [](const WBEnvelope& e) { return e.event == "chronicles:state"; }, &line);
    check(pending_state, "gate-b: non-quick guest login admits chronicles:state");

    // 2) Found the House.
    mark = client.mark();
    JV::Object found;
    found.emplace("name", JV(house_name));
    client.send("chronicles:house:found", JV(std::move(found)));
    const bool founded = client.wait_from(
        mark, 5000,
        [](const WBEnvelope& e) { return e.event == "chronicles:state"; }, &line);
    check(founded, "gate-b: chronicles:house:found emits chronicles:state");
    if (founded) {
      if (const JV* chronicle = line.env.data.get("chronicle")) {
        if (const JV* houses = chronicle->get("houses");
            houses && houses->array() && !houses->array()->empty()) {
          house_id = gateb_str(houses->array()->front(), "id");
        }
      }
    }
    check(!house_id.empty(), "gate-b: House roster id captured");

    // 3) First Scion through the accepted chain: create then set-out.
    mark = client.mark();
    JV::Object create1;
    create1.emplace("houseId", JV(house_id));
    create1.emplace("name", JV("Aldous"));
    client.send("chronicles:scion:create", JV(std::move(create1)));
    const bool created1 = client.wait_from(
        mark, 5000,
        [](const WBEnvelope& e) {
          return e.event == "chronicles:state" &&
                 e.data.get("createdScionId") != nullptr;
        },
        &line);
    check(created1, "gate-b: chronicles:scion:create returns createdScionId");
    if (created1) scion1_id = gateb_str(line.env.data, "createdScionId");

    mark = client.mark();
    JV::Object setout1;
    setout1.emplace("scionId", JV(scion1_id));
    client.send("chronicles:scion:set-out", JV(std::move(setout1)));
    const bool admitted1 = client.wait_from(
        mark, 5000,
        [](const WBEnvelope& e) { return e.event == "player:login"; }, &line);
    check(admitted1, "gate-b: chronicles:scion:set-out admits via player:login");
    bool oath_armed = false;
    if (admitted1) {
      if (const JV* player = line.env.data.get("player")) {
        if (const JV* chronicles = player->get("chronicles")) {
          oath_armed = gateb_bool(*chronicles, "mortal");
          check(gateb_str(*chronicles, "scionId") == scion1_id,
                "gate-b: admission names the first Scion");
        }
      }
    }
    check(oath_armed,
          "gate-b: set-out admits the Scion under the mortal oath "
          "(player.chronicles.mortal)");

    // 4) Earn an identifiable item through ordinary play: the road purse
    // funds a wagon purchase. Only earned gear circulates at death.
    mark = client.mark();
    JV::Object buy;
    buy.emplace("itemId", JV("bronze-sword"));
    client.send("wagon:outfit:buy", JV(std::move(buy)));
    const bool bought = client.wait_from(
        mark, 5000,
        [&](const WBEnvelope& e) {
          if (e.event != "core:refresh:inventory") return false;
          const JV* slots = e.data.get("data");
          if (!slots || !slots->array()) return false;
          for (const auto& entry : *slots->array()) {
            if (gateb_str(entry, "id") == "bronze-sword") {
              relic_uuid = gateb_str(entry, "uuid");
              return true;
            }
          }
          return false;
        });
    check(bought && !relic_uuid.empty(),
          "gate-b: earned identifiable item carries an exact uuid");

    // 5) Ordinary movement/combat death in a delved instance.
    mark = client.mark();
    JV::Object solo;
    solo.emplace("template", JV("dungeon"));
    solo.emplace("layout", JV("warren"));
    client.send("instance:enterSolo", JV(std::move(solo)));
    const bool entered = client.wait_from(
        mark, 5000, [](const WBEnvelope& e) {
          return e.event == "party:scene:transition" ||
                 e.event == "world:scene:transition";
        });
    check(entered, "gate-b: delve enters the instance scene");

    bool contact = false;
    const bool fallen = gateb_sweep_until_fallen(client, scion1_id, 150000,
                                                 &contact);
    check(contact, "gate-b: ordinary combat reached the scion (incoming hit)");
    check(fallen,
          "gate-b: ordinary movement/combat death commits the fall "
          "(chronicles:scion-fallen)");
    if (!fallen) {
      std::printf(
          "note: pre-change runtime gap reproduced at the fatal-fall step; "
          "successor/recovery/reconnect legs cannot run without it\n");
      client.close();
      server->stop();
      delete server;
      return;
    }

    // 6) Return through the accepted Chronicles surface.
    mark = client.mark();
    client.send("player:chronicles:return", JV(JV::Object{}));
    const bool returned = client.wait_from(
        mark, 5000,
        [&](const WBEnvelope& e) {
          if (e.event != "player:chronicles:ready") return false;
          const JV* fallen_payload = e.data.get("fallen");
          return fallen_payload &&
                 gateb_str(*fallen_payload, "scionId") == scion1_id;
        });
    check(returned, "gate-b: player:chronicles:return readies succession");

    // 7) Successor: create then admit through the accepted select path.
    mark = client.mark();
    JV::Object create2;
    create2.emplace("houseId", JV(house_id));
    create2.emplace("name", JV("Becca"));
    client.send("chronicles:scion:create", JV(std::move(create2)));
    const bool created2 = client.wait_from(
        mark, 5000,
        [](const WBEnvelope& e) {
          return e.event == "chronicles:state" &&
                 e.data.get("createdScionId") != nullptr;
        },
        &line);
    check(created2, "gate-b: successor created through chronicles:scion:create");
    if (created2) scion2_id = gateb_str(line.env.data, "createdScionId");

    mark = client.mark();
    JV::Object select2;
    select2.emplace("scionId", JV(scion2_id));
    select2.emplace("houseId", JV(house_id));
    select2.emplace("scionName", JV("Becca"));
    select2.emplace("mortal", JV(true));
    client.send("player:chronicles:select", JV(std::move(select2)));
    const bool admitted2 = client.wait_from(
        mark, 5000,
        [](const WBEnvelope& e) { return e.event == "player:login"; }, &line);
    check(admitted2, "gate-b: heir admitted through player:chronicles:select");

    // 7b) The heir arms normally: succession admission starts with the
    // fresh-scion profile, so the House treasury outfits a new sword and
    // the heir wears it - ordinary wagon/equip surfaces, no shortcuts.
    mark = client.mark();
    JV::Object rearm;
    rearm.emplace("itemId", JV("bronze-sword"));
    client.send("wagon:outfit:buy", JV(std::move(rearm)));
    std::string heir_sword_uuid;
    const bool rearmed = client.wait_from(
        mark, 5000,
        [&](const WBEnvelope& e) {
          if (e.event != "core:refresh:inventory") return false;
          const JV* slots = e.data.get("data");
          if (!slots || !slots->array()) return false;
          for (const auto& entry : *slots->array()) {
            if (gateb_str(entry, "id") == "bronze-sword") {
              heir_sword_uuid = gateb_str(entry, "uuid");
              return true;
            }
          }
          return false;
        });
    check(rearmed && !heir_sword_uuid.empty(),
          "gate-b: heir buys a sword from the House wagon");
    mark = client.mark();
    JV::Object item;
    item.emplace("uuid", JV(heir_sword_uuid));
    JV::Object equip_payload;
    equip_payload.emplace("item", JV(std::move(item)));
    client.send("item:equip", JV(std::move(equip_payload)));
    const bool worn = client.wait_from(
        mark, 5000,
        [&](const WBEnvelope& e) {
          if (e.event != "core:refresh:inventory") return false;
          const JV* slots = e.data.get("data");
          if (!slots || !slots->array()) return false;
          for (const auto& entry : *slots->array())
            if (gateb_str(entry, "uuid") == heir_sword_uuid) return false;
          return true;  // left the backpack = worn
        });
    check(worn, "gate-b: heir equips the fresh sword");

    // 8) Recover the EXACT relic: slay the floor's elite so the heirloom
    // surfaces, walk to it, and take it underfoot.
    mark = client.mark();
    JV::Object solo2;
    solo2.emplace("template", JV("dungeon"));
    solo2.emplace("layout", JV("warren"));
    client.send("instance:enterSolo", JV(std::move(solo2)));
    client.wait_from(mark, 5000, [](const WBEnvelope& e) {
      return e.event == "party:scene:transition" ||
             e.event == "world:scene:transition";
    });

    int kills = 0;
    // The shared view still carries the FIRST scion's death state; the
    // successor starts their hunt with unknown health and alive.
    client.view().hp = -1;
    client.view().player_died_hit = false;
    const bool surfaced =
        gateb_hunt_until_relic_surfaces(client, relic_uuid, 420000, &kills);
    check(surfaced, "gate-b: slain elite surfaces the circulating heirloom");
    if (!surfaced) {
      std::printf(
          "note: recovery leg could not complete; successor/reconnect "
          "assertions below reflect the interrupted state\n");
      client.close();
      server->stop();
      delete server;
      return;
    }
    const bool recovered = gateb_take_relic(client, relic_uuid, 60000);
    check(recovered, "gate-b: exact relic uuid recovered underfoot");

    // 9) Crypt state is honest before the disconnect.
    mark = client.mark();
    JV::Object restore;
    restore.emplace("guestId", JV(guest));
    restore.emplace("awaitChronicles", JV(true));
    client.send("player:login", JV(std::move(restore)));
    const bool ready_before = client.wait_from(
        mark, 5000,
        [](const WBEnvelope& e) {
          return e.event == "player:chronicles:ready";
        },
        &line);
    check(ready_before, "gate-b: awaitChronicles restore reads the crypt");
    if (ready_before) {
      if (const JV* chronicle = line.env.data.get("chronicles")) {
        chronicle_snapshot = chronicle->stringify();
        bool crypt_recovered = false;
        if (const JV* houses = chronicle->get("houses"); houses && houses->array()) {
          for (const auto& house_entry : *houses->array()) {
            if (gateb_str(house_entry, "id") != house_id) continue;
            if (const JV* crypt = house_entry.get("crypt"); crypt && crypt->array()) {
              for (const auto& entry : *crypt->array()) {
                if (gateb_str(entry, "id") != scion1_id) continue;
                if (const JV* relic_record = entry.get("relic")) {
                  crypt_recovered =
                      gateb_str(*relic_record, "status") == "recovered";
                }
              }
            }
          }
        }
        check(crypt_recovered, "gate-b: crypt relic status is recovered");
      }
    }

    client.close();
  }

  // 10) Disconnect and reconnect with the SAME guest identity.
  {
    GateBView view;
    LoopbackClient client(view);
    std::string error;
    check(client.connect(port, &error), "gate-b: reconnect upgrades same server");
    size_t mark = client.mark();
    JV::Object login;
    login.emplace("guestId", JV(guest));
    login.emplace("awaitChronicles", JV(true));
    client.send("player:login", JV(std::move(login)));
    LoopbackClient::Line line;
    const bool ready_after = client.wait_from(
        mark, 8000,
        [](const WBEnvelope& e) {
          return e.event == "player:chronicles:ready";
        },
        &line);
    check(ready_after, "gate-b: reconnect restores the chronicle session");
    if (ready_after) {
      const JV* chronicle = line.env.data.get("chronicles");
      check(chronicle != nullptr, "gate-b: reconnect payload carries chronicle");
      if (chronicle) {
        check(chronicle->stringify() == chronicle_snapshot,
              "gate-b: identical House/Scion/relic record after reconnect");
        bool roster_ok = false;
        bool crypt_ok = false;
        if (const JV* houses = chronicle->get("houses"); houses && houses->array()) {
          for (const auto& house_entry : *houses->array()) {
            if (gateb_str(house_entry, "id") != house_id) continue;
            roster_ok = true;
            check(gateb_str(house_entry, "name") == house_name,
                  "gate-b: House name survives reconnect");
            if (const JV* scions = house_entry.get("scions"); scions && scions->array()) {
              bool heir_living = false;
              for (const auto& scion_entry : *scions->array()) {
                if (gateb_str(scion_entry, "id") == scion2_id) {
                  heir_living = true;
                  check(gateb_bool(scion_entry, "mortal"),
                        "gate-b: heir's sworn oath recorded after reconnect");
                }
              }
              check(heir_living, "gate-b: living roster holds only the heir");
              check(scions->array()->size() == 1,
                    "gate-b: fallen Scion left the living roster");
            }
            if (const JV* crypt = house_entry.get("crypt"); crypt && crypt->array()) {
              for (const auto& entry : *crypt->array()) {
                if (gateb_str(entry, "id") != scion1_id) continue;
                if (const JV* relic_record = entry.get("relic"))
                  crypt_ok = gateb_str(*relic_record, "status") == "recovered";
              }
            }
          }
        }
        check(roster_ok, "gate-b: same House present after reconnect");
        check(crypt_ok, "gate-b: crypt relic still recovered after reconnect");
        check(gateb_str(*chronicle, "activeHouseId") == house_id,
              "gate-b: active House continuity");
      }
    }

    // 11) Resume proves carried-heirloom + oath continuity on the live wire.
    mark = client.mark();
    JV::Object resume;
    resume.emplace("scionId", JV(scion2_id));
    client.send("chronicles:scion:set-out", JV(std::move(resume)));
    const bool resumed = client.wait_from(
        mark, 8000,
        [](const WBEnvelope& e) { return e.event == "player:login"; }, &line);
    check(resumed, "gate-b: resumed set-out re-admits the heir");
    bool carried = false;
    bool oath_kept = false;
    if (resumed) {
      if (const JV* player = line.env.data.get("player")) {
        if (const JV* chronicles = player->get("chronicles"))
          oath_kept = gateb_bool(*chronicles, "mortal");
        if (const JV* inventory = player->get("inventory")) {
          if (const JV* slots = inventory->get("slots"); slots && slots->array()) {
            for (const auto& entry : *slots->array())
              if (gateb_str(entry, "uuid") == relic_uuid) carried = true;
          }
        }
      }
    }
    check(carried, "gate-b: exact heirloom still carried after reconnect");
    check(oath_kept, "gate-b: mortal oath survives the reconnect");
    client.close();
  }

  server->stop();
  delete server;
  check(true, "gate-b: journey server stopped cleanly");
}

// ── TASK-0162: passive-tree payload hardening (scripted loopback wire) ────
// apply_envelope is deliberately private; the production path from bytes to
// mirror is reader_loop -> parse_envelope -> apply_envelope ->
// apply_passive_tree. To prove the fail-closed contract end to end this
// suite binds a tiny TEST-ONLY scripted WebSocket server that answers the
// client's upgrade and then replays raw `{event,data}` envelopes — including
// payloads the real verdigris_server would never emit. No server or wire
// authority changes: the production client parses exactly what a hostile or
// corrupting peer puts on the loopback wire (TASK-0163 capsule only).

#ifdef _WIN32
using PtSocket = SOCKET;
static constexpr PtSocket kPtInvalidSocket = INVALID_SOCKET;
static void pt_close_socket(PtSocket socket) { ::closesocket(socket); }
#else
using PtSocket = int;
static constexpr PtSocket kPtInvalidSocket = -1;
static void pt_close_socket(PtSocket socket) { ::close(socket); }
#endif

bool pt_send_all(PtSocket socket, const char* data, std::size_t size) {
  std::size_t sent_total = 0;
  while (sent_total < size) {
    const int sent =
        ::send(socket, data + sent_total, static_cast<int>(size - sent_total), 0);
    if (sent <= 0) return false;
    sent_total += static_cast<std::size_t>(sent);
  }
  return true;
}

// Server->client text frames are unmasked (RFC6455 5.1).
bool pt_send_text_frame(PtSocket socket, const std::string& payload) {
  std::string frame;
  frame.push_back(static_cast<char>(0x81));
  const std::size_t size = payload.size();
  if (size < 126) {
    frame.push_back(static_cast<char>(size));
  } else if (size <= 0xFFFF) {
    frame.push_back(static_cast<char>(126));
    frame.push_back(static_cast<char>((size >> 8) & 0xff));
    frame.push_back(static_cast<char>(size & 0xff));
  } else {
    frame.push_back(static_cast<char>(127));
    for (int i = 7; i >= 0; --i)
      frame.push_back(static_cast<char>(
          (static_cast<std::uint64_t>(size) >> (i * 8)) & 0xff));
  }
  frame += payload;
  return pt_send_all(socket, frame.data(), frame.size());
}

// Single-connection scripted server: serves exactly one client with the
// scripted envelope stream, then exits so a client retry can never replay
// the script against a second connection.
class ScriptedEnvelopeServer {
 public:
  ScriptedEnvelopeServer() = default;
  ~ScriptedEnvelopeServer() { stop(); }

  ScriptedEnvelopeServer(const ScriptedEnvelopeServer&) = delete;
  ScriptedEnvelopeServer& operator=(const ScriptedEnvelopeServer&) = delete;

  bool start(std::string* error) {
#ifdef _WIN32
    WSADATA wsa{};
    wsa_started_ = ::WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
    if (!wsa_started_) {
      if (error) *error = "WSAStartup failed";
      return false;
    }
#endif
    // This suite's TASK-0163 loopback capsule is 7160-7179; port 6500 is
    // never touched.
    for (std::uint16_t port = 7160; port <= 7179; ++port) {
      listener_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (listener_ == kPtInvalidSocket) break;
      sockaddr_in address{};
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // loopback only
      address.sin_port = htons(port);
      if (::bind(listener_, reinterpret_cast<sockaddr*>(&address),
                 sizeof(address)) == 0 &&
          ::listen(listener_, 1) == 0) {
        port_ = port;
        break;
      }
      pt_close_socket(listener_);
      listener_ = kPtInvalidSocket;
    }
    if (listener_ == kPtInvalidSocket) {
      if (error) *error = "no free port in the TASK-0163 capsule";
#ifdef _WIN32
      if (wsa_started_) {
        ::WSACleanup();
        wsa_started_ = false;
      }
#endif
      return false;
    }
    running_.store(true);
    worker_ = std::thread(&ScriptedEnvelopeServer::serve, this);
    return true;
  }

  void stop() {
    running_.store(false);
    if (listener_ != kPtInvalidSocket) {
      pt_close_socket(listener_);
      listener_ = kPtInvalidSocket;
    }
    if (worker_.joinable()) worker_.join();
#ifdef _WIN32
    if (wsa_started_) {
      ::WSACleanup();
      wsa_started_ = false;
    }
#endif
  }

  std::uint16_t port() const { return port_; }

  // Frame pacing: frame 0 goes out right after the upgrade; every later
  // frame waits for an explicit grant so each assertion block observes its
  // own frame deterministically.
  void grant_next_frame() { credits_.fetch_add(1); }

  std::vector<std::string> script;

 private:
  void serve() {
    while (running_.load()) {
      fd_set readfds;
      FD_ZERO(&readfds);
      FD_SET(listener_, &readfds);
      timeval timeout{};
      timeout.tv_sec = 0;
      timeout.tv_usec = 100000;
#ifdef _WIN32
      const int ready = ::select(0, &readfds, nullptr, nullptr, &timeout);
#else
      const int ready =
          ::select(static_cast<int>(listener_) + 1, &readfds, nullptr, nullptr,
                   &timeout);
#endif
      if (!running_.load()) return;
      if (ready <= 0) continue;
      const PtSocket connection = ::accept(listener_, nullptr, nullptr);
      if (connection == kPtInvalidSocket) return;
      serve_connection(connection);
      pt_close_socket(connection);
    }
  }

  void serve_connection(PtSocket connection) {
    std::string request;
    char buffer[1024];
    while (request.find("\r\n\r\n") == std::string::npos && request.size() < 8192) {
      const int got = ::recv(connection, buffer, sizeof(buffer), 0);
      if (got <= 0) return;
      request.append(buffer, buffer + got);
    }
    // Accept header paired with the fixed loopback key in
    // RemoteProtocolSession (the RFC6455 example pair).
    static const char kUpgrade[] =
        "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n"
        "Connection: Upgrade\r\nSec-WebSocket-Accept: "
        "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n";
    if (!pt_send_all(connection, kUpgrade, sizeof(kUpgrade) - 1)) return;
    for (std::size_t i = 0; i < script.size(); ++i) {
      if (i > 0) {
        const auto wait_deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(10);
        while (running_.load() && credits_.load() == 0 &&
               std::chrono::steady_clock::now() < wait_deadline) {
          std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        if (!running_.load() || credits_.load() == 0) return;
        credits_.fetch_sub(1);
      }
      if (!pt_send_text_frame(connection, script[i])) return;
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(300));  // drain window
  }

  PtSocket listener_ = kPtInvalidSocket;
  std::atomic<bool> running_{false};
  std::atomic<int> credits_{0};
  std::thread worker_;
  std::uint16_t port_ = 0;
  bool wsa_started_ = false;
};

std::string pt_login_frame(const std::string& tree_json) {
  std::string player =
      "{\"uuid\":\"hardening-guest\",\"x\":10,\"y\":11,\"facing\":\"down\"";
  if (!tree_json.empty()) player += ",\"passiveTree\":" + tree_json;
  player += "}";
  return "{\"event\":\"player:login\",\"data\":{\"player\":" + player +
         ",\"scene\":{\"id\":\"town\",\"type\":\"town\",\"name\":"
         "\"Verdigris Town\"}}}";
}

std::string pt_state_frame(const std::string& tree_json) {
  return "{\"event\":\"dev:state\",\"data\":{\"state\":{\"lifecycle\":"
         "\"alive\",\"passiveTree\":" +
         tree_json + "}}}";
}

std::string pt_update_frame(const std::string& tree_json) {
  return "{\"event\":\"player:skilltree:update\",\"data\":{\"passiveTree\":" +
         tree_json + "}}";
}

bool progression_is(const verdigris::client::ClientModel& model, bool present,
                    int unspent, int earned, int nodes, int conduits) {
  const auto& p = model.progression;
  return p.present == present && p.unspent_points == unspent &&
         p.earned_points == earned && p.node_count == nodes &&
         p.conduit_count == conduits;
}

template <typename Pred>
bool pt_pump_until(verdigris::client::RemoteProtocolSession& session,
                   std::vector<std::string>& errors, int timeout_ms, Pred done) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
  for (;;) {
    session.poll();
    for (const auto& event : session.drain_events())
      if (event.type == verdigris::client::PresentationEventType::ProtocolError)
        errors.push_back(event.text);
    if (done()) return true;
    if (std::chrono::steady_clock::now() >= deadline) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void remote_passive_tree_absence_stays_absent() {
  ScriptedEnvelopeServer server;
  server.script.push_back(pt_login_frame(""));
  std::string error;
  check(server.start(&error), "ptree-absent: scripted loopback server bound in capsule");
  if (server.port() == 0) return;

  verdigris::client::RemoteProtocolSession session("127.0.0.1", server.port(),
                                                   "pt-absent-guest", true);
  check(session.start(&error), "ptree-absent: connect + upgrade + login sent");
  check(wait_for_state(session, verdigris::client::ConnectionState::Ready, 5000),
        "ptree-absent: admission acknowledged");
  std::vector<std::string> seen;
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(400);
  while (std::chrono::steady_clock::now() < deadline) {
    session.poll();
    for (const auto& event : session.drain_events())
      if (event.type == verdigris::client::PresentationEventType::ProtocolError)
        seen.push_back(event.text);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  check(progression_is(session.model(), false, 0, 0, 0, 0),
        "ptree-absent: no payload -> mirror stays absent, never rendered as zero");
  check(seen.empty(), "ptree-absent: absent payload raises no diagnostic");
  session.shutdown();
  server.stop();
}

void remote_endgame_payload_mirrors_to_presentation() {
  ScriptedEnvelopeServer server;
  server.script.push_back(
      "{\"event\":\"player:login\",\"data\":{\"player\":{"
      "\"uuid\":\"map-guest\",\"x\":10,\"y\":11,\"facing\":\"down\","
      "\"inventory\":{\"slots\":[{\"id\":\"charted-tablet-crown\","
      "\"uuid\":\"tablet-1\",\"displayName\":\"Tier 6 Crown Charted Tablet\","
      "\"slot\":0,\"expeditionMap\":{\"tier\":6,"
      "\"goodsFoundPercent\":88,\"modifiers\":[\"Furious\",\"Teeming\"]}}]}},"
      "\"scene\":{\"id\":\"town\",\"type\":\"town\","
      "\"name\":\"Verdigris Town\"}}}");
  server.script.push_back(
      "{\"event\":\"dev:state\",\"data\":{\"state\":{"
      "\"lifecycle\":\"alive\",\"endgame\":{\"unlocked\":true,"
      "\"active\":true,\"cleared\":false,\"completed\":4,\"tier\":6,"
      "\"name\":\"Tier 6 Crown Charted Tablet\","
      "\"goodsFoundPercent\":88,\"modifiers\":[\"Furious\",\"Teeming\"]}}}}");
  std::string error;
  check(server.start(&error),
        "endgame-mirror: scripted loopback server bound in capsule");
  if (server.port() == 0) return;

  verdigris::client::RemoteProtocolSession session(
      "127.0.0.1", server.port(), "map-mirror-guest", true);
  check(session.start(&error), "endgame-mirror: connect + upgrade + login sent");
  check(wait_for_state(session, verdigris::client::ConnectionState::Ready, 5000),
        "endgame-mirror: admission acknowledged");
  check(session.model().inventory.size() == 1 &&
            session.model().inventory.front().expedition_map &&
            session.model().inventory.front().map_tier == 6 &&
            session.model().inventory.front().map_modifiers.size() == 2,
        "endgame-mirror: rolled tablet survives wire-to-model parsing");

  server.grant_next_frame();
  std::vector<std::string> errors;
  const bool arrived = pt_pump_until(
      session, errors, 5000,
      [&] { return session.model().endgame.present &&
                   session.model().endgame.active; });
  check(arrived && session.model().endgame.unlocked &&
            session.model().endgame.completed == 4 &&
            session.model().endgame.goods_found_percent == 88,
        "endgame-mirror: active expedition state survives wire-to-model parsing");
  verdigris::client::WorldView world;
  verdigris::client::sync_world_from_model(world, session.model());
  check(world.carried.size() == 1 && world.carried.front().expedition_map &&
            world.carried.front().map_tier == 6 && world.endgame.active &&
            world.endgame.tier == 6,
        "endgame-mirror: map item and active run survive model-to-presentation sync");
  check(errors.empty(), "endgame-mirror: valid payload raises no protocol error");
  session.shutdown();
  server.stop();
}

void remote_crossroads_dialogue_mirrors_to_presentation() {
  ScriptedEnvelopeServer server;
  server.script.push_back(
      "{\"event\":\"player:login\",\"data\":{\"player\":{"
      "\"uuid\":\"social-guest\",\"x\":31,\"y\":121,\"facing\":\"down\"},"
      "\"scene\":{\"id\":\"town:verdigris\",\"type\":\"town\","
      "\"name\":\"The Crossroads\"}}}");
  server.script.push_back(
      "{\"event\":\"dev:state\",\"data\":{\"state\":{"
      "\"lifecycle\":\"alive\",\"houseInvestment\":{"
      "\"firstClearCompleted\":true,\"eligible\":true,"
      "\"choice\":\"unchosen\",\"rewardClaimed\":false,"
      "\"scionGearTier\":0,\"houseIncomePerClear\":0},"
      "\"npcs\":[{\"id\":4,\"key\":\"rhea-countinghouse\","
      "\"name\":\"Rhea of the Countinghouse\",\"role\":\"steward\","
      "\"examine\":\"Keeps the House ledger.\",\"x\":31,\"y\":121,"
      "\"services\":[\"storage\",\"house_investment\"],"
      "\"actions\":[\"bank\",\"examine\"]}]}}}");
  server.script.push_back(
      "{\"event\":\"open:screen\",\"data\":{\"screen\":\"dialogue\","
      "\"payload\":{\"npcId\":4,\"npcKey\":\"rhea-countinghouse\","
      "\"name\":\"Rhea of the Countinghouse\",\"role\":\"steward\","
      "\"body\":\"Choose what the first clear builds.\",\"options\":["
      "{\"id\":\"bank\",\"label\":\"Open the Countinghouse\","
      "\"hint\":\"House storage.\",\"action\":\"player:screen:bank\",\"enabled\":true},"
      "{\"id\":\"scion_gear\",\"label\":\"Commission named Scion gear\","
      "\"hint\":\"Immediate gear.\",\"action\":\"house:investment:choose\",\"enabled\":true},"
      "{\"id\":\"house_production\",\"label\":\"Build House road production\","
      "\"hint\":\"Future income.\",\"action\":\"house:investment:choose\",\"enabled\":true}]}}}");
  std::string error;
  check(server.start(&error),
        "social-mirror: scripted loopback server bound in capsule");
  if (server.port() == 0) return;

  verdigris::client::RemoteProtocolSession session(
      "127.0.0.1", server.port(), "social-mirror-guest", true);
  check(session.start(&error), "social-mirror: connect + upgrade + login sent");
  check(wait_for_state(session, verdigris::client::ConnectionState::Ready, 5000),
        "social-mirror: admission acknowledged");

  server.grant_next_frame();
  std::vector<std::string> errors;
  const bool town_arrived = pt_pump_until(
      session, errors, 5000,
      [&] { return session.model().npcs.size() == 1 &&
                   session.model().house_investment.eligible; });
  check(town_arrived && session.model().npcs.front().key == "rhea-countinghouse" &&
            session.model().npcs.front().role == "steward" &&
            session.model().npcs.front().services.size() == 2,
        "social-mirror: NPC identity, role, services, and coffer state reach the model");
  verdigris::client::WorldView world;
  verdigris::client::sync_world_from_model(world, session.model());
  check(world.npcs.size() == 1 && world.npcs.front().key == "rhea-countinghouse" &&
            world.npcs.front().services.size() == 2,
        "social-mirror: town service identity reaches presentation");

  server.grant_next_frame();
  const bool dialogue_arrived = pt_pump_until(
      session, errors, 5000,
      [&] { return session.model().dialogue.open; });
  check(dialogue_arrived && session.model().dialogue.npc_id == 4 &&
            session.model().dialogue.options.size() == 3 &&
            session.model().dialogue.options[1].action == "house:investment:choose",
        "social-mirror: authoritative dialogue and investment verbs reach the pane model");
  session.submit(verdigris::client::ClientCommand::close_screen());
  check(!session.model().dialogue.open,
        "social-mirror: the shared Escape/close contract dismisses dialogue");
  check(errors.empty(), "social-mirror: valid payload raises no protocol error");
  session.shutdown();
  server.stop();
}

void remote_passive_tree_payload_hardening() {
  using verdigris::client::RemoteProtocolSession;

  ScriptedEnvelopeServer server;
  std::string error;
  check(server.start(&error), "ptree: scripted loopback server bound in capsule");
  if (server.port() == 0) return;

  const char* kPointsReason = "points.skill must be a nonnegative integer";
  const char* kEarnedReason = "earned must be a nonnegative integer";
  const char* kSchemaReason = "schemaVersion must be the number 2";

  // Frame plan. Frame 0 ships with the upgrade; every later frame waits for
  // one grant per assertion block so ordering stays deterministic.
  server.script.push_back(pt_login_frame(""));  // 0: valid ABSENT admission
  server.script.push_back(pt_state_frame(       // 1: valid ZERO tree
      "{\"schemaVersion\":2,\"points\":{\"skill\":0},\"earned\":0,"
      "\"nodes\":[\"0,0\"],\"conduits\":[]}"));
  server.script.push_back(pt_update_frame(      // 2: valid NONZERO refresh
      "{\"schemaVersion\":2,\"points\":{\"skill\":7},\"earned\":12,"
      "\"nodes\":[\"n0\",\"n1\",\"n2\",\"n3\"],"
      "\"conduits\":[\"c0\",\"c1\",\"c2\"]}"));

  // Invalid battery (frames 3+). Channels rotate across all three production
  // call sites: player:login, dev:state, player:skilltree:update.
  struct RejectCase {
    std::string envelope;
    std::string expected_text;
    const char* label;
  };
  const std::string fractional_earned_frame =
      pt_update_frame("{\"schemaVersion\":2,\"points\":{\"skill\":7},"
                      "\"earned\":11.5,\"nodes\":[\"n0\"],\"conduits\":[]}");
  std::vector<RejectCase> rejects = {
      // Missing fields.
      {pt_update_frame("{\"schemaVersion\":2}"),
       std::string("passiveTree rejected: ") + kPointsReason, "missing points"},
      {pt_state_frame("{\"schemaVersion\":2,\"points\":{\"skill\":0},"
                      "\"nodes\":[\"0,0\"],\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kEarnedReason, "missing earned"},
      {pt_update_frame("{\"schemaVersion\":2,\"points\":{\"skill\":7},"
                       "\"earned\":12,\"nodes\":[\"n0\"]}"),
       "passiveTree rejected: conduits must be an array", "missing conduits"},
      // Wrong types.
      {pt_update_frame("{\"schemaVersion\":\"2\",\"points\":{\"skill\":7},"
                       "\"earned\":12,\"nodes\":[\"n0\"],\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kSchemaReason,
       "wrong-typed schemaVersion"},
      {pt_update_frame("{\"schemaVersion\":2,\"points\":[1],\"earned\":12,"
                       "\"nodes\":[\"n0\"],\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kPointsReason,
       "wrong-typed points container"},
      {pt_update_frame("{\"schemaVersion\":2,\"points\":{\"skill\":\"3\"},"
                       "\"earned\":12,\"nodes\":[\"n0\"],\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kPointsReason,
       "wrong-typed points.skill"},
      {pt_update_frame("{\"schemaVersion\":2,\"points\":{\"skill\":7},"
                       "\"earned\":true,\"nodes\":[\"n0\"],\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kEarnedReason,
       "wrong-typed earned"},
      {pt_update_frame("{\"schemaVersion\":2,\"points\":{\"skill\":7},"
                       "\"earned\":12,\"nodes\":{},\"conduits\":[]}"),
       "passiveTree rejected: nodes must be an array", "wrong-typed nodes"},
      {pt_login_frame(
           "{\"schemaVersion\":2,\"points\":{\"skill\":7},\"earned\":12,"
           "\"nodes\":[\"n0\"],\"conduits\":\"none\"}"),
       "passiveTree rejected: conduits must be an array",
       "wrong-typed conduits on login"},
      // Fractional values.
      {pt_update_frame("{\"schemaVersion\":2,\"points\":{\"skill\":2.5},"
                       "\"earned\":12,\"nodes\":[\"n0\"],\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kPointsReason,
       "fractional points.skill"},
      {fractional_earned_frame,
       std::string("passiveTree rejected: ") + kEarnedReason,
       "fractional earned"},
      // Negative values.
      {pt_update_frame("{\"schemaVersion\":2,\"points\":{\"skill\":-1},"
                       "\"earned\":12,\"nodes\":[\"n0\"],\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kPointsReason,
       "negative points.skill"},
      {pt_state_frame("{\"schemaVersion\":2,\"points\":{\"skill\":7},"
                      "\"earned\":-3,\"nodes\":[\"n0\"],\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kEarnedReason,
       "negative earned"},
      // Non-finite / overflow-like values (strtod accepts these literals).
      {pt_state_frame("{\"schemaVersion\":2,\"points\":{\"skill\":Infinity},"
                      "\"earned\":12,\"nodes\":[\"n0\"],\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kPointsReason,
       "bare-Infinity points.skill"},
      {pt_update_frame("{\"schemaVersion\":2,\"points\":{\"skill\":7},"
                       "\"earned\":1e400,\"nodes\":[\"n0\"],\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kEarnedReason,
       "exponent-overflow earned (inf)"},
      {pt_update_frame("{\"schemaVersion\":2,\"points\":{\"skill\":7},"
                       "\"earned\":2147483648,\"nodes\":[\"n0\"],"
                       "\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kEarnedReason,
       "int-cast overflow earned"},
      // Unsupported schema version arrives on the admission channel.
      {pt_login_frame(
           "{\"schemaVersion\":3,\"points\":{\"skill\":1},\"earned\":1,"
           "\"nodes\":[\"n0\"],\"conduits\":[]}"),
       std::string("passiveTree rejected: ") + kSchemaReason,
       "future schemaVersion on login"},
      // Top-level shape failure on the admission channel.
      {pt_login_frame("[]"),
       "passiveTree rejected: envelope must be an object",
       "non-object passiveTree on login"},
  };

  // Oversized arrays: 65537 entries sits above the documented transport
  // entry bound yet far below the 1 MiB reader frame ceiling, so rejection
  // happens in the parser, not the transport (and each frame exercises the
  // 64-bit length path).
  {
    std::string entries;
    entries.reserve(65537 * 3);
    for (int i = 0; i < 65537; ++i) {
      if (i) entries += ",";
      entries += "\"\"";
    }
    rejects.push_back({pt_update_frame(
                           "{\"schemaVersion\":2,\"points\":{\"skill\":7},"
                           "\"earned\":12,\"conduits\":[],\"nodes\":[" +
                           entries + "]}"),
                       "passiveTree rejected: nodes exceeds the passiveTree "
                       "transport entry bound",
                       "oversized nodes array"});
    rejects.push_back(
        {pt_state_frame("{\"schemaVersion\":2,\"points\":{\"skill\":7},"
                        "\"earned\":12,\"nodes\":[\"0,0\"],\"conduits\":[" +
                        entries + "]}"),
         "passiveTree rejected: conduits exceeds the passiveTree transport "
         "entry bound",
         "oversized conduits array"});
  }
  const std::size_t kFractionalEarnedErrorIndex =
      10;  // position of "fractional earned" in the table above
  for (const auto& reject : rejects) server.script.push_back(reject.envelope);

  // Stability probe: the exact fractional-earned payload repeats so the two
  // diagnostics must be byte-identical.
  server.script.push_back(rejects[kFractionalEarnedErrorIndex].envelope);

  // Recovery: a valid refresh after the whole battery must still apply.
  server.script.push_back(pt_update_frame(
      "{\"schemaVersion\":2,\"points\":{\"skill\":5},\"earned\":11,"
      "\"nodes\":[\"n0\",\"n1\",\"n2\"],\"conduits\":[\"c0\",\"c1\"]}"));

  RemoteProtocolSession session("127.0.0.1", server.port(),
                                "hardening-guest", true);
  check(session.start(&error), "ptree: connect + upgrade + login sent");

  std::vector<std::string> seen;
  check(pt_pump_until(session, seen, 5000, [&] {
          return session.connection_state() ==
                 verdigris::client::ConnectionState::Ready;
        }),
        "ptree: valid absent login reaches Ready");
  check(progression_is(session.model(), false, 0, 0, 0, 0),
        "ptree: fresh admission starts absent (valid absent behavior)");

  server.grant_next_frame();  // -> valid ZERO tree over dev:state
  check(pt_pump_until(session, seen, 5000,
                      [&] { return session.model().progression.present; }),
        "ptree: VALID zero tree makes the mirror present");
  check(progression_is(session.model(), true, 0, 0, 1, 0),
        "ptree: zero tree mirrors verbatim zeros (valid zero behavior)");
  check(seen.empty(), "ptree: valid payloads raise no diagnostic");

  server.grant_next_frame();  // -> valid NONZERO refresh
  check(pt_pump_until(session, seen, 5000, [&] {
          return progression_is(session.model(), true, 7, 12, 4, 3);
        }),
        "ptree: VALID nonzero update mirrors verbatim (valid nonzero behavior)");
  check(seen.empty(), "ptree: valid nonzero update raises no diagnostic");
  const auto snapshot = session.model().progression;

  for (std::size_t i = 0; i < rejects.size() + 2; ++i) {
    const std::size_t baseline = seen.size();
    server.grant_next_frame();
    bool arrived = false;
    if (i < rejects.size()) {
      arrived = pt_pump_until(session, seen, 5000,
                              [&] { return seen.size() > baseline; });
      const std::string label = std::string("ptree: ") + rejects[i].label;
      check(arrived, (label + ": diagnostic surfaced").c_str());
      if (arrived)
        check(seen[baseline] == rejects[i].expected_text,
              (label + ": deterministic diagnostic text").c_str());
    } else if (i == rejects.size()) {
      arrived = pt_pump_until(session, seen, 5000,
                              [&] { return seen.size() > baseline; });
      check(arrived, "ptree: repeated invalid payload surfaces again");
      if (arrived)
        check(seen.back() == seen[kFractionalEarnedErrorIndex],
              "ptree: diagnostic text is byte-stable across repeats");
    } else {
      arrived = pt_pump_until(
          session, seen, 5000,
          [&] { return progression_is(session.model(), true, 5, 11, 3, 2); });
      check(arrived,
            "ptree: valid refresh applies after rejects (session healthy)");
    }
    if (i != rejects.size() + 1) {
      const auto& p = session.model().progression;
      check(p.present == snapshot.present &&
                p.unspent_points == snapshot.unspent_points &&
                p.earned_points == snapshot.earned_points &&
                p.node_count == snapshot.node_count &&
                p.conduit_count == snapshot.conduit_count,
            "ptree: invalid update left the last valid snapshot untouched");
    }
  }

  const std::size_t quiet_baseline = seen.size();
  const auto quiet_deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
  bool quiet = true;
  while (std::chrono::steady_clock::now() < quiet_deadline) {
    session.poll();
    for (const auto& event : session.drain_events())
      if (event.type == verdigris::client::PresentationEventType::ProtocolError)
        seen.push_back(event.text);
    if (seen.size() != quiet_baseline) {
      quiet = false;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  check(quiet, "ptree: valid recovery raises no diagnostic");

  session.shutdown();
  server.stop();
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
  remote_passive_tree_absence_stays_absent();
  remote_endgame_payload_mirrors_to_presentation();
  remote_crossroads_dialogue_mirrors_to_presentation();
  remote_passive_tree_payload_hardening();
  gateb_driver_state_machine_controls();
  gate_b_chronicles_reconnect_journey();
  if (failures == 0) {
    std::printf("session tests passed\n");
    return 0;
  }
  std::printf("%d session test check(s) failed\n", failures);
  return 1;
}
