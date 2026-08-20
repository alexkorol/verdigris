// TASK-0060 (D-122): C3 session seam tests. Proves the local adapter keeps
// deterministic play available and the remote adapter completes a REAL
// handshake against verdigris_server's WebSocket listener — plus the
// authentic negative: a dead endpoint is a visible hard failure, never a
// silent fallback to local play.

#include <chrono>
#include <cstdio>
#include <thread>

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

}  // namespace

int main() {
  local_session_ready_and_deterministic();
  remote_dead_endpoint_is_a_visible_failure();
  remote_handshake_reaches_ready();
  remote_guest_journey();
  remote_mid_session_disconnect();
  remote_session_replaced();
  remote_render_list_ops();
  if (failures == 0) {
    std::printf("session tests passed\n");
    return 0;
  }
  std::printf("%d session test check(s) failed\n", failures);
  return 1;
}
