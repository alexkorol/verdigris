#include "remote_session.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
static constexpr socket_t kInvalidSocket = INVALID_SOCKET;
static void close_socket(socket_t socket) { ::closesocket(socket); }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
static constexpr socket_t kInvalidSocket = -1;
static void close_socket(socket_t socket) { ::close(socket); }
#endif

namespace verdigris::client {

namespace {

using verdigris::networking::Envelope;
using verdigris::networking::JsonValue;

bool send_all(socket_t socket, const void* data, std::size_t size) {
  const char* cursor = static_cast<const char*>(data);
  std::size_t remaining = size;
  while (remaining > 0) {
    const auto sent = ::send(socket, cursor, static_cast<int>(remaining), 0);
    if (sent <= 0) return false;
    cursor += sent;
    remaining -= static_cast<std::size_t>(sent);
  }
  return true;
}

bool recv_all(socket_t socket, void* data, std::size_t size) {
  char* cursor = static_cast<char*>(data);
  std::size_t remaining = size;
  while (remaining > 0) {
    const auto got = ::recv(socket, cursor, static_cast<int>(remaining), 0);
    if (got <= 0) return false;
    cursor += got;
    remaining -= static_cast<std::size_t>(got);
  }
  return true;
}

// The server accepts any syntactically valid Sec-WebSocket-Key; loopback
// development transport does not need nonce randomness (recorded coupling —
// revisit if the transport ever leaves loopback).
constexpr const char* kWebSocketKey = "dGhlIHNhbXBsZSBub25jZQ==";

const std::string* json_string(const JsonValue* value) {
  return value && value->string() ? value->string() : nullptr;
}

double json_number(const JsonValue* value, double fallback = 0.0) {
  if (!value || !value->number()) return fallback;
  return *value->number();
}

ClientItemSlot parse_item_slot(const JsonValue& entry) {
  ClientItemSlot slot;
  if (const auto* id = json_string(entry.get("id"))) slot.id = *id;
  if (const auto* uuid = json_string(entry.get("uuid"))) slot.uuid = *uuid;
  if (const auto* display = json_string(entry.get("displayName"))) slot.name = *display;
  else if (const auto* plain = json_string(entry.get("name"))) slot.name = *plain;
  if (const auto* index = entry.get("slot"); index && index->number()) {
    slot.slot = static_cast<int>(*index->number());
  }
  if (const auto* health = entry.get("resourceBonuses")) {
    slot.bonus_health = static_cast<int>(json_number(health->get("health")));
  }
  if (const auto* combat = entry.get("combatBonuses")) {
    slot.critical_chance = static_cast<int>(json_number(combat->get("criticalChance")));
  }
  if (const auto* stats = entry.get("stats")) {
    if (const auto* attack = stats->get("attack")) {
      const int slash = static_cast<int>(json_number(attack->get("slash")));
      const int stab = static_cast<int>(json_number(attack->get("stab")));
      const int crush = static_cast<int>(json_number(attack->get("crush")));
      const int range = static_cast<int>(json_number(attack->get("range")));
      slot.critical_chance = (std::max)(slot.critical_chance,
                                        (std::max)(slash, (std::max)(stab, (std::max)(crush, range))));
      slot.attack_rating = (std::max)(slash, (std::max)(stab, (std::max)(crush, range)));
    }
  }
  return slot;
}

void apply_player_fields(ClientPlayer& player, const JsonValue& source) {
  if (const auto* uuid = json_string(source.get("uuid"))) player.uuid = *uuid;
  if (const auto* scene = json_string(source.get("sceneId"))) player.scene_id = *scene;
  if (source.get("x") && source.get("x")->number()) player.x = *source.get("x")->number();
  if (source.get("y") && source.get("y")->number()) player.y = *source.get("y")->number();
  if (const auto* facing = json_string(source.get("facing"))) player.facing = *facing;
}

void facing_delta(const std::string& facing, double& dx, double& dy) {
  dx = 0.0;
  dy = 1.0;
  if (facing == "left" || facing == "west") {
    dx = -1.0;
    dy = 0.0;
  } else if (facing == "right" || facing == "east") {
    dx = 1.0;
    dy = 0.0;
  } else if (facing == "up" || facing == "north") {
    dx = 0.0;
    dy = -1.0;
  }
}

void place_in_front(const ClientPlayer& player, double& x, double& y) {
  double dx = 0.0;
  double dy = 1.0;
  facing_delta(player.facing, dx, dy);
  x = player.x + dx;
  y = player.y + dy;
}

ClientMonster* find_monster(ClientModel& model, const std::string& id) {
  for (auto& monster : model.monsters)
    if (monster.id == id) return &monster;
  return nullptr;
}

ClientMonster& upsert_monster(ClientModel& model, const std::string& id, const std::string& name,
                              bool elite) {
  if (ClientMonster* existing = find_monster(model, id.empty() ? name : id)) {
    if (!name.empty()) existing->name = name;
    if (elite) existing->elite = true;
    existing->alive = true;
    return *existing;
  }
  ClientMonster monster;
  monster.id = id.empty() ? ("foe-" + std::to_string(model.monsters.size() + 1)) : id;
  monster.name = name.empty() ? "monster" : name;
  place_in_front(model.player, monster.x, monster.y);
  monster.elite = elite;
  monster.life = elite ? 80 : 40;
  monster.life_max = monster.life;
  model.monsters.push_back(std::move(monster));
  return model.monsters.back();
}

void apply_scene_fields(ClientScene& scene, const JsonValue& source) {
  if (const auto* id = json_string(source.get("id"))) scene.id = *id;
  if (const auto* type = json_string(source.get("type"))) scene.type = *type;
  if (const auto* name = json_string(source.get("name"))) scene.name = *name;
  if (const auto* metadata = source.get("metadata")) {
    if (const auto* stairs = metadata->get("stairsUp")) {
      if (stairs->get("x") && stairs->get("x")->number() && stairs->get("y") &&
          stairs->get("y")->number()) {
        scene.stairs_up_x = *stairs->get("x")->number();
        scene.stairs_up_y = *stairs->get("y")->number();
        scene.has_stairs_up = true;
      }
    }
  }
}

}  // namespace

RemoteProtocolSession::RemoteProtocolSession(std::string host, std::uint16_t port,
                                             std::string guest_id, bool quick_guest)
    : host_(std::move(host)), port_(port), guest_id_(std::move(guest_id)),
      quick_guest_(quick_guest) {}

RemoteProtocolSession::~RemoteProtocolSession() { shutdown(); }

bool RemoteProtocolSession::connect_transport(std::string* error) {
  const auto socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket == kInvalidSocket) {
    last_error_ = "socket() failed";
    if (error) *error = last_error_;
    return false;
  }
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(host_.c_str());
  address.sin_port = htons(port_);
  if (::connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
    close_socket(socket);
    last_error_ = "connection refused at " + host_ + ":" + std::to_string(port_);
    if (error) *error = last_error_;
    return false;
  }

  const std::string request =
      "GET / HTTP/1.1\r\nHost: " + host_ + ":" + std::to_string(port_) +
      "\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: " +
      kWebSocketKey + "\r\nSec-WebSocket-Version: 13\r\n\r\n";
  if (!send_all(socket, request.data(), request.size())) {
    close_socket(socket);
    last_error_ = "upgrade request send failed";
    if (error) *error = last_error_;
    return false;
  }

  std::string response;
  char buffer[1024];
  while (response.find("\r\n\r\n") == std::string::npos && response.size() < 8192) {
    const auto got = ::recv(socket, buffer, sizeof(buffer), 0);
    if (got <= 0) break;
    response.append(buffer, buffer + got);
  }
  if (response.find(" 101 ") == std::string::npos ||
      response.find("\r\n\r\n") == std::string::npos) {
    close_socket(socket);
    last_error_ = "endpoint did not complete a websocket upgrade";
    if (error) *error = last_error_;
    return false;
  }

  socket_ = static_cast<std::intptr_t>(socket);
  state_.store(ConnectionState::Connected);
  pending_events_.push_back(
      {PresentationEventType::ConnectionEstablished, "", "", host_, port_});
  running_.store(true);
  reader_ = std::make_unique<std::thread>(&RemoteProtocolSession::reader_loop, this);

  Envelope login{"player:login", JsonValue::Object{
      {"guestId", JsonValue(guest_id_)},
      {"quickGuest", JsonValue(quick_guest_)}}};
  if (!send_envelope(login)) {
    last_error_ = "login send failed";
    if (error) *error = last_error_;
    close_transport();
    return false;
  }
  return true;
}

void RemoteProtocolSession::close_transport() {
  running_.store(false);
  if (socket_ != -1) {
    send_frame(0x8, "");
    close_socket(static_cast<socket_t>(socket_));
    socket_ = -1;
  }
  if (reader_ && reader_->joinable()) reader_->join();
  reader_.reset();
}

void RemoteProtocolSession::begin_retry(const std::string& reason) {
  close_transport();
  if (suppress_retry_ || !ever_ready_) {
    fail(ConnectionState::Disconnected, reason);
    return;
  }
  last_error_ = reason;
  if (state_.load() != ConnectionState::Retrying) {
    pending_events_.push_back(
        {PresentationEventType::ConnectionLost, "", "", reason, 0});
  }
  if (retry_attempt_ >= 3) {
    fail(ConnectionState::Disconnected, "reconnect failed after 3 attempts");
    return;
  }
  static constexpr int kBackoffMs[3] = {1000, 2000, 4000};
  state_.store(ConnectionState::Retrying);
  retry_at_ = std::chrono::steady_clock::now() +
              std::chrono::milliseconds(kBackoffMs[retry_attempt_]);
}

void RemoteProtocolSession::pump_retry() {
  if (state_.load() != ConnectionState::Retrying) return;
  if (std::chrono::steady_clock::now() < retry_at_) return;
  if (retry_attempt_ >= 3) {
    fail(ConnectionState::Disconnected, "reconnect failed after 3 attempts");
    return;
  }
  std::string error;
  if (connect_transport(&error)) return;
  ++retry_attempt_;
  if (retry_attempt_ >= 3) {
    fail(ConnectionState::Disconnected, "reconnect failed after 3 attempts");
    return;
  }
  static constexpr int kBackoffMs[3] = {1000, 2000, 4000};
  retry_at_ = std::chrono::steady_clock::now() +
              std::chrono::milliseconds(kBackoffMs[retry_attempt_]);
}

bool RemoteProtocolSession::start(std::string* error) {
  state_.store(ConnectionState::Connecting);
#ifdef _WIN32
  WSADATA data{};
  if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
    fail(ConnectionState::Rejected, "WSAStartup failed");
    if (error) *error = last_error_;
    return false;
  }
  wsa_started_ = true;
#endif
  if (!connect_transport(error)) {
    const bool protocol = last_error_.find("websocket") != std::string::npos;
    fail(protocol ? ConnectionState::ProtocolMismatch : ConnectionState::Rejected,
         last_error_);
    if (error) *error = last_error_;
    return false;
  }
  return true;
}

void RemoteProtocolSession::shutdown() {
  suppress_retry_ = true;
  close_transport();
  const auto state = state_.load();
  if (state == ConnectionState::Connecting || state == ConnectionState::Connected ||
      state == ConnectionState::Ready || state == ConnectionState::Retrying) {
    state_.store(ConnectionState::Disconnected);
  }
#ifdef _WIN32
  if (wsa_started_) {
    WSACleanup();
    wsa_started_ = false;
  }
#endif
}

void RemoteProtocolSession::submit(const ClientCommand& command) {
  Envelope envelope{"", JsonValue::Object{}};
  switch (command.type) {
    case ClientCommand::Type::Login:
      envelope.event = "player:login";
      envelope.data = JsonValue::Object{{"guestId", JsonValue(command.target)},
                                        {"quickGuest", JsonValue(command.value != 0)}};
      break;
    case ClientCommand::Type::Move: {
      const char* direction = "down";
      if (command.dy < 0) direction = "up";
      else if (command.dy > 0) direction = "down";
      else if (command.dx < 0) direction = "left";
      else if (command.dx > 0) direction = "right";
      last_facing_ = direction;
      model_.player.facing = direction;
      envelope.event = "player:move";
      envelope.data = JsonValue::Object{{"direction", JsonValue(direction)}};
      break;
    }
    case ClientCommand::Type::Aim: {
      // Aim is presentation-local on this protocol: no envelope, facing
      // updates the model so the next skill trigger carries direction.
      if (command.dy < 0) last_facing_ = "up";
      else if (command.dy > 0) last_facing_ = "down";
      else if (command.dx < 0) last_facing_ = "left";
      else if (command.dx > 0) last_facing_ = "right";
      model_.player.facing = last_facing_;
      return;
    }
    case ClientCommand::Type::UseAction: {
      envelope.event = "player:skill:trigger";
      envelope.data = JsonValue::Object{{"skill", JsonValue(command.target)},
                                        {"direction", JsonValue(last_facing_)}};
      break;
    }
    case ClientCommand::Type::PickUp:
      envelope.event = "player:take:underfoot";
      break;
    case ClientCommand::Type::Equip:
      pending_equip_uuid_ = command.target;
      envelope.event = "item:equip";
      envelope.data = JsonValue::Object{
          {"item", JsonValue::Object{{"uuid", JsonValue(command.target)}}}};
      break;
    case ClientCommand::Type::EnterZone:
      envelope.event = "world:zone:enter";
      envelope.data = JsonValue::Object{{"nodeId", JsonValue(command.target)}};
      break;
    case ClientCommand::Type::Extract:
      // No player:extract handler exists on the native server. The owner
      // extracts by walking onto stairs-up (existing player:move surface).
      pending_events_.push_back({PresentationEventType::Message, "", "",
                                 "Reach the exit stairs to return to the surface.", 0});
      return;
  }
  if (!envelope.event.empty()) send_envelope(envelope);
}

void RemoteProtocolSession::poll() {
  std::deque<std::string> batch;
  {
    std::lock_guard lock(inbox_mutex_);
    batch.swap(inbox_);
  }
  for (const auto& text : batch) {
    Envelope envelope;
    std::string error;
    if (!verdigris::networking::parse_envelope(text, envelope, &error)) {
      pending_events_.push_back({PresentationEventType::ProtocolError, "", "", error, 0});
      continue;
    }
    apply_envelope(envelope);
  }
  if (peer_dropped_.exchange(false)) {
    if (suppress_retry_ || state_.load() == ConnectionState::Disconnected ||
        state_.load() == ConnectionState::Rejected) {
      close_transport();
    } else {
      begin_retry("server closed the connection");
    }
  }
  pump_retry();
}

std::vector<PresentationEvent> RemoteProtocolSession::drain_events() {
  std::vector<PresentationEvent> drained;
  drained.swap(pending_events_);
  return drained;
}

bool RemoteProtocolSession::send_envelope(const Envelope& envelope) {
  return send_frame(0x1, verdigris::networking::emit_envelope(envelope));
}

bool RemoteProtocolSession::send_frame(std::uint8_t opcode, const std::string& payload) {
  if (socket_ == -1) return false;
  std::lock_guard lock(send_mutex_);
  std::vector<std::uint8_t> frame;
  frame.push_back(static_cast<std::uint8_t>(0x80 | opcode));
  const auto size = payload.size();
  // Clients MUST mask (RFC6455 5.3); the server enforces this.
  if (size < 126) {
    frame.push_back(static_cast<std::uint8_t>(0x80 | size));
  } else if (size <= 65535) {
    frame.push_back(0x80 | 126);
    frame.push_back(static_cast<std::uint8_t>(size >> 8));
    frame.push_back(static_cast<std::uint8_t>(size));
  } else {
    frame.push_back(0x80 | 127);
    for (int i = 7; i >= 0; --i) {
      frame.push_back(static_cast<std::uint8_t>((size >> (i * 8)) & 0xff));
    }
  }
  // Deterministic mask: loopback development transport (see kWebSocketKey).
  const std::uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
  frame.insert(frame.end(), mask, mask + 4);
  const auto offset = frame.size();
  frame.resize(offset + size);
  for (std::size_t i = 0; i < size; ++i) {
    frame[offset + i] = static_cast<std::uint8_t>(payload[i]) ^ mask[i % 4];
  }
  return send_all(static_cast<socket_t>(socket_), frame.data(), frame.size());
}

void RemoteProtocolSession::reader_loop() {
  const auto socket = static_cast<socket_t>(socket_);
  while (running_.load()) {
    std::uint8_t header[2];
    if (!recv_all(socket, header, 2)) break;
    const auto opcode = static_cast<std::uint8_t>(header[0] & 0x0f);
    const bool masked = (header[1] & 0x80) != 0;
    std::uint64_t length = header[1] & 0x7f;
    if (length == 126) {
      std::uint8_t ext[2];
      if (!recv_all(socket, ext, 2)) break;
      length = static_cast<std::uint64_t>((ext[0] << 8) | ext[1]);
    } else if (length == 127) {
      std::uint8_t ext[8];
      if (!recv_all(socket, ext, 8)) break;
      length = 0;
      for (auto byte : ext) length = (length << 8) | byte;
    }
    if (length > (1u << 20)) break;  // cap: no single game envelope is 1MB
    std::uint8_t mask[4] = {0, 0, 0, 0};
    if (masked && !recv_all(socket, mask, 4)) break;
    std::string payload(static_cast<std::size_t>(length), '\0');
    if (length > 0 && !recv_all(socket, payload.data(), payload.size())) break;
    if (masked) {
      for (std::size_t i = 0; i < payload.size(); ++i) payload[i] ^= mask[i % 4];
    }
    if (opcode == 0x8) break;  // server close
    if (opcode == 0x9) {       // ping -> masked pong
      send_frame(0xA, payload);
      continue;
    }
    if (opcode == 0x1) {
      std::lock_guard lock(inbox_mutex_);
      inbox_.push_back(std::move(payload));
    }
  }
  if (running_.load()) {
    // The peer dropped us; surface it on the next poll (never fall back
    // to local play — fail() emits ConnectionLost on the session thread).
    peer_dropped_.store(true);
    running_.store(false);
  }
}

void RemoteProtocolSession::apply_envelope(const Envelope& envelope) {
  if (envelope.event == "player:login") {
    if (const auto* player = envelope.data.get("player")) {
      apply_player_fields(model_.player, *player);
      last_facing_ = model_.player.facing.empty() ? last_facing_ : model_.player.facing;
      model_.inventory.clear();
      if (const auto* inventory = player->get("inventory")) {
        if (const auto* slots = inventory->get("slots"); slots && slots->array()) {
          for (const auto& entry : *slots->array()) {
            model_.inventory.push_back(parse_item_slot(entry));
          }
        }
      }
    }
    if (const auto* scene = envelope.data.get("scene")) apply_scene_fields(model_.scene, *scene);
    state_.store(ConnectionState::Ready);
    ever_ready_ = true;
    retry_attempt_ = 0;
    pending_events_.push_back(
        {PresentationEventType::SessionReady, model_.player.uuid, "", "", 0});
    return;
  }
  if (envelope.event == "player:session-replaced") {
    suppress_retry_ = true;
    fail(ConnectionState::Disconnected, "session replaced by a newer connection");
    close_transport();
    return;
  }
  if (envelope.event == "game:send:message") {
    if (const auto* text = json_string(envelope.data.get("text"))) {
      model_.last_message = *text;
      pending_events_.push_back({PresentationEventType::Message, "", "", model_.last_message, 0});
      if (model_.last_message.find("returns to the surface") != std::string::npos) {
        model_.extracted = true;
        pending_events_.push_back(
            {PresentationEventType::ExtractionCompleted, model_.player.uuid, "",
             model_.last_message, 0});
      }
    }
    return;
  }
  if (envelope.event == "player:movement") {
    apply_player_fields(model_.player, envelope.data);
    if (!model_.player.facing.empty()) last_facing_ = model_.player.facing;
    return;
  }
  if (envelope.event == "world:scene:transition" ||
      envelope.event == "party:scene:transition") {
    if (const auto* scene = envelope.data.get("scene")) apply_scene_fields(model_.scene, *scene);
    if (const auto* player_state = envelope.data.get("playerState")) {
      apply_player_fields(model_.player, *player_state);
    }
    if (!model_.scene.id.empty()) model_.player.scene_id = model_.scene.id;
    model_.monsters.clear();
    model_.ground.clear();
    return;
  }
  if (envelope.event == "monster:telegraph") {
    const auto* attacker = json_string(envelope.data.get("attackerId"));
    const auto* name = json_string(envelope.data.get("attackerName"));
    const auto* skill = json_string(envelope.data.get("skillId"));
    const std::string skill_id = skill ? *skill : "telegraph";
    const bool elite = skill_id.find("sweep") != std::string::npos ||
                       skill_id.find("boss") != std::string::npos;
    upsert_monster(model_, attacker ? *attacker : "", name ? *name : "", elite);
    pending_events_.push_back({PresentationEventType::Telegraph,
                               attacker ? *attacker : "", "",
                               std::string(name ? *name : "") + " " + skill_id,
                               static_cast<int>(json_number(envelope.data.get("durationMs")))});
    return;
  }
  if (envelope.event == "combat:hit") {
    const auto* attacker = json_string(envelope.data.get("attackerId"));
    const auto* target = json_string(envelope.data.get("targetId"));
    const auto* target_type = json_string(envelope.data.get("targetType"));
    const int amount = static_cast<int>(json_number(envelope.data.get("amount")));
    const bool died = envelope.data.get("died") && envelope.data.get("died")->boolean() &&
                      *envelope.data.get("died")->boolean();
    const bool hits_player =
        (target && *target == model_.player.uuid) ||
        (target_type && *target_type == "player");
    if (const auto* health = envelope.data.get("health")) {
      if (hits_player) {
        model_.player.life = static_cast<int>(json_number(health->get("current"), model_.player.life));
        model_.player.life_max =
            static_cast<int>(json_number(health->get("max"), model_.player.life_max));
        if (died) model_.player.alive = false;
      }
    }
    if (hits_player) {
      if (attacker) upsert_monster(model_, *attacker, "", true);
      model_.last_incoming_hit = amount;
      pending_events_.push_back({PresentationEventType::DamageApplied,
                                 attacker ? *attacker : "", "", "incoming", amount});
      if (died) {
        pending_events_.push_back(
            {PresentationEventType::ScionDied, model_.player.uuid, "", "", 0});
      }
    } else {
      model_.last_outgoing_hit = amount;
      ClientMonster& foe = upsert_monster(model_, target ? *target : "",
                                          json_string(envelope.data.get("targetName"))
                                              ? *json_string(envelope.data.get("targetName"))
                                              : "",
                                          false);
      if (const auto* health = envelope.data.get("health")) {
        foe.life = static_cast<int>(json_number(health->get("current"), foe.life));
        foe.life_max = static_cast<int>(json_number(health->get("max"), foe.life_max));
      } else {
        foe.life = (std::max)(0, foe.life - amount);
      }
      pending_events_.push_back({PresentationEventType::AttackStarted,
                                 attacker ? *attacker : model_.player.uuid, "",
                                 last_facing_, amount});
      pending_events_.push_back({PresentationEventType::DamageApplied,
                                 target ? *target : "", "", "outgoing", amount});
      if (died) {
        ++model_.kills;
        foe.alive = false;
        foe.life = 0;
        pending_events_.push_back({PresentationEventType::ActorDied,
                                   target ? *target : "", "",
                                   json_string(envelope.data.get("targetName"))
                                       ? *json_string(envelope.data.get("targetName"))
                                       : "",
                                   amount});
        // The native server drops loot in-world but does not emit a ground
        // envelope. Sparkle at the last known player tile so the kill is a
        // visible reward beat until pickup names the item.
        const std::string drop_id = "drop-" + std::to_string(model_.kills);
        model_.ground.push_back({drop_id, "kill reward", foe.x, foe.y});
        pending_events_.push_back({PresentationEventType::ItemDropped,
                                   target ? *target : "", drop_id, "kill reward", 0});
      }
    }
    return;
  }
  if (envelope.event == "core:refresh:inventory") {
    const auto* slots = envelope.data.get("data");
    std::vector<std::string> before;
    before.reserve(model_.inventory.size());
    ClientItemSlot equipped_snapshot;
    for (const auto& item : model_.inventory) {
      before.push_back(item.uuid);
      if (!pending_equip_uuid_.empty() && item.uuid == pending_equip_uuid_) {
        equipped_snapshot = item;
      }
    }
    model_.inventory.clear();
    if (slots && slots->array()) {
      for (const auto& entry : *slots->array()) {
        model_.inventory.push_back(parse_item_slot(entry));
      }
    }
    for (const auto& item : model_.inventory) {
      bool known = false;
      for (const auto& uuid : before) {
        if (uuid == item.uuid) {
          known = true;
          break;
        }
      }
      if (!known && !item.uuid.empty()) {
        pending_events_.push_back({PresentationEventType::ItemPickedUp, model_.player.uuid,
                                   item.uuid, item.name, 0});
      }
    }
    if (!pending_equip_uuid_.empty()) {
      bool still_carried = false;
      for (const auto& item : model_.inventory) {
        if (item.uuid == pending_equip_uuid_) {
          still_carried = true;
          break;
        }
      }
      if (!still_carried) {
        if (!equipped_snapshot.uuid.empty()) model_.equipped = equipped_snapshot;
        else model_.equipped.uuid = pending_equip_uuid_;
        pending_events_.push_back({PresentationEventType::ItemEquipped, model_.player.uuid,
                                   model_.equipped.uuid, model_.equipped.name,
                                   model_.equipped.attack_rating});
        pending_equip_uuid_.clear();
      }
    }
    return;
  }
}

void RemoteProtocolSession::fail(ConnectionState state, const std::string& error) {
  last_error_ = error;
  state_.store(state);
  pending_events_.push_back({PresentationEventType::ConnectionLost, "", "", error, 0});
}

}  // namespace verdigris::client
