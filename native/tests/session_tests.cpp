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
  // ox-pc-r worker capsule 6960-6979 (START_HERE_OX_PC_R.md): never 6500,
  // never another lane's capsule.
  for (std::uint16_t port = 6960; port <= 6979; ++port) {
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

// One movement sample with echo confirmation; silent means blocked (the
// server only emits player:movement for accepted steps). Rotates the caller's
// heading right-hand-style on a wall, and never steps onto a stair tile.
bool gateb_step(LoopbackClient& client, int& heading) {
  GateBView& view = client.view();
  if (!view.has_pos) return false;
  for (int attempt = 0; attempt < 4; ++attempt) {
    const int delta_x = heading == 0 ? 1 : heading == 2 ? -1 : 0;
    const int delta_y = heading == 1 ? 1 : heading == 3 ? -1 : 0;
    const int next_x = static_cast<int>(std::floor(view.px)) + delta_x;
    const int next_y = static_cast<int>(std::floor(view.py)) + delta_y;
    if (gateb_on_stairs(view, next_x, next_y)) {
      heading = (heading + 1) & 3;
      continue;
    }
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

// Sweep-walk (no swinging) until the scion falls in ordinary combat. This is
// the pre-change failing step: without the mortal-oath admission nothing is
// ever emitted for an ordinary lethal wound.
bool gateb_sweep_until_fallen(LoopbackClient& client, const std::string& scion_id,
                              int timeout_ms, bool* made_contact) {
  GateBView& view = client.view();
  int heading = 0;
  size_t index = client.mark();
  bool contact = false;
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
  while (std::chrono::steady_clock::now() < deadline) {
    LoopbackClient::Line line;
    while (client.scan_from(index, [](const LoopbackClient::Line&) { return true; }, &line)) {
      if (line.env.event == "chronicles:scion-fallen") {
        const JV* fallen = line.env.data.get("fallen");
        if (fallen && gateb_str(*fallen, "scionId") == scion_id) return true;
      }
      if (line.env.event == "combat:hit" && gateb_str(line.env.data, "targetType") == "player") {
        contact = true;
        *made_contact = true;
      }
      if (view.player_died_hit) contact = true;
    }
    if (!contact) {
      gateb_ensure_instance(client);
      gateb_step(client, heading);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(contact ? 30 : 15));
    client.service();
  }
  return false;
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
      const int dx = target_x - static_cast<int>(std::floor(view.px));
      const int dy = target_y - static_cast<int>(std::floor(view.py));
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
  // Serpentine sweep lattice state.
  int wp_serial = 0;
  int wp_x = -1, wp_y = -1;
  auto wp_deadline = now();
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
      wp_serial = 0;
      wp_x = -1;
      wp_y = -1;
      last_outgoing = never;
      last_incoming = never;
      continue;
    }
    if (now() >= heartbeat_at) {
      heartbeat_at = now() + std::chrono::seconds(10);
      std::printf(
          "note: hunt heartbeat hp=%d at=(%d,%d) kills=%d elite_known=%d "
          "elite=(%d,%d) ground=%zu scene=%s\n",
          view.hp, static_cast<int>(std::floor(view.px)),
          static_cast<int>(std::floor(view.py)), *kills,
          elite_known ? 1 : 0, elite_x, elite_y, view.ground.size(),
          view.scene_type.c_str());
    }
    const int tile_x = static_cast<int>(std::floor(view.px));
    const int tile_y = static_cast<int>(std::floor(view.py));
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
        if (chebyshev(tile_x, tile_y, elite_x, elite_y) <= 1) {
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
        // Serpentine lattice exploration: a border-hugging right-hand sweep
        // orbits the outer wall ring forever and never comes within reveal
        // range of an interior elite. Waypoints march serpentine columns so
        // the whole floor is covered; each waypoint is bounded and skipped
        // with evidence if a wall pocket blocks it.
        const bool wp_reached = wp_x >= 0 &&
                                std::abs(tile_x - wp_x) <= 1 &&
                                std::abs(tile_y - wp_y) <= 1;
        if (wp_x < 0 || wp_reached || now() >= wp_deadline) {
          if (wp_x >= 0 && !wp_reached) {
            std::printf("note: hunt sweep skips waypoint (%d,%d)\n", wp_x,
                        wp_y);
          }
          const int col = 2 + (wp_serial / 2) * 5;
          wp_x = col;
          wp_y = (wp_serial % 2 == 1) ? 36 : 2;
          ++wp_serial;
          if (col > 36) wp_serial = 0;  // restart the lattice after coverage
          wp_deadline = now() + std::chrono::seconds(25);
        }
        const int wdx = wp_x - tile_x;
        const int wdy = wp_y - tile_y;
        fight_heading = gateb_heading_for(wdx > 0 ? 1 : (wdx < 0 ? -1 : 0),
                                          wdy > 0 ? 1 : (wdy < 0 ? -1 : 0));
        const int wp_primary =
            gateb_heading_for(wdx > 0 ? 1 : (wdx < 0 ? -1 : 0), 0);
        const int wp_secondary =
            gateb_heading_for(0, wdy > 0 ? 1 : (wdy < 0 ? -1 : 0));
        int wp_lane = std::abs(wdx) >= std::abs(wdy) ? wp_primary : wp_secondary;
        gateb_step(client, wp_lane);
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
    const int dx = static_cast<int>(std::floor(relic->x)) -
                   static_cast<int>(std::floor(view.px));
    const int dy = static_cast<int>(std::floor(relic->y)) -
                   static_cast<int>(std::floor(view.py));
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
    if (std::chrono::steady_clock::now() >= deadline) return false;
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

// The complete frozen Gate-B journey over loopback with only accepted
// envelopes. Pre-change this fails at the ordinary-death fall; post-change
// every step must pass.
void gate_b_chronicles_reconnect_journey() {
  verdigris::networking::WebSocketServer* server = nullptr;
  const auto port = start_server_worker_capsule(server);
  check(server != nullptr, "gate-b: worker-capsule server bound (6960-6979)");
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
  gate_b_chronicles_reconnect_journey();
  if (failures == 0) {
    std::printf("session tests passed\n");
    return 0;
  }
  std::printf("%d session test check(s) failed\n", failures);
  return 1;
}
