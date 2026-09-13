#include "remote_session.hpp"
#include "presentation_state.hpp"
#include "input/preserve-diagonal-remote-input.hpp"
#include "input/make-aim-independent-of-motion.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
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

// Eight-way wire direction name for a quantized (dx, dy) input; matches the
// server's direction table exactly. Empty for the zero vector.
std::string direction_name(int dx, int dy) {
  return move::encode_eight_way(dx, dy);
}

double json_number(const JsonValue* value, double fallback = 0.0) {
  if (!value || !value->number()) return fallback;
  return *value->number();
}

bool json_bool(const JsonValue* value, bool fallback = false) {
  if (!value || !value->boolean()) return fallback;
  return *value->boolean();
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
  slot.quantity = (std::max)(1, static_cast<int>(json_number(entry.get("qty"), 1.0)));
  if (const auto* size = entry.get("size"); size && size->object()) {
    slot.width = std::clamp(static_cast<int>(json_number(size->get("width"), 1.0)), 1, 12);
    slot.height = std::clamp(static_cast<int>(json_number(size->get("height"), 1.0)), 1, 7);
  }
  if (const auto* equip_slot = json_string(entry.get("equipSlot")))
    slot.equip_slot = *equip_slot;
  slot.two_handed = json_bool(entry.get("twoHanded"), false);
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

void apply_wear_details(const JsonValue& source, ClientModel& model) {
  if (!source.object()) return;
  static constexpr const char* seats[] = {
      "right_hand", "left_hand", "armor", "head", "back", "belt",
      "gloves", "feet", "ring", "ring2", "necklace", "warhorn",
      "quick_rig", "attendant"};
  model.worn.clear();
  model.equipped = {};
  for (const char* seat : seats) {
    const auto* entry = source.get(seat);
    if (!entry || !entry->object()) continue;
    ClientWornItem worn;
    worn.seat = seat;
    worn.item = parse_item_slot(*entry);
    if (worn.item.uuid.empty() && worn.item.id.empty()) continue;
    if (worn.item.name.empty()) worn.item.name = worn.item.id;
    if (worn.seat == "right_hand") model.equipped = worn.item;
    model.worn.push_back(std::move(worn));
  }
}

void apply_combat_fields(const JsonValue& combat, ClientPlayer& player) {
  auto valid = [](const JsonValue* value) {
    if (!value || !value->number()) return false;
    const double n = *value->number();
    return std::isfinite(n) && n >= 0 && n <= 1000000 && std::floor(n) == n;
  };
  const auto* attack = combat.get("baseAttack");
  const auto* defense = combat.get("baseDefense");
  const auto* gear = combat.get("gearAttack");
  if (!valid(attack) || !valid(defense) || !valid(gear)) return;
  const auto* gear_defense = combat.get("gearDefense");
  if (gear_defense && !valid(gear_defense)) return;
  player.attack = static_cast<int>(*attack->number());
  player.defense = static_cast<int>(*defense->number());
  if (gear_defense) player.defense += static_cast<int>(*gear_defense->number());
  player.gear_attack = static_cast<int>(*gear->number());
  player.combat_stats_present = true;
}

// TASK-0156: mirror the authoritative `passiveTree` envelope (schemaVersion
// 2: nodes / conduits / points.skill / earned) into plain model fields. Only
// payload-borne values are copied; the client derives no rules, costs, or
// effects.
//
// TASK-0162 hardening: the mirror is fail-closed. It may only update when the
// schema version, points.skill, earned, nodes, and conduits all carry their
// expected wire types with sane nonnegative integral values; anything else
// leaves the last valid snapshot untouched and surfaces one deterministic
// ProtocolError diagnostic. Invalid payloads never silently become zero and
// never become absurd counts through unchecked casts.
//
// The single cap below is a TRANSPORT BOUND, not a product rule. It exists
// only so a hostile or corrupting frame cannot overflow an int cast or force
// pathological parse/memory behavior; it encodes no tree design, cost,
// budget, or balance opinion, and any well-typed value under it is mirrored
// verbatim. 65536 sits orders of magnitude above any authored tree while
// staying safely inside the 1 MiB reader frame ceiling in reader_loop().
constexpr std::size_t kPassiveTreeTransportBound = 65536;

bool sane_passive_tree_integer(const JsonValue* value) {
  if (!value || !value->number()) return false;
  const double raw = *value->number();
  if (!(raw >= 0.0)) return false;           // rejects NaN and negatives alike
  if (std::floor(raw) != raw) return false;  // fractional counts are malformed
  return raw <= static_cast<double>(kPassiveTreeTransportBound);
}

void apply_passive_tree(const JsonValue& tree, ClientModel& model,
                        std::vector<PresentationEvent>& events) {
  const char* reason = nullptr;
  const JsonValue* nodes = nullptr;
  const JsonValue* conduits = nullptr;
  if (!tree.object()) {
    reason = "envelope must be an object";
  } else {
    const auto* schema = tree.get("schemaVersion");
    const std::optional<double> schema_value =
        schema ? schema->number() : std::nullopt;
    if (!schema_value || std::floor(*schema_value) != *schema_value ||
        *schema_value != 2.0) {
      reason = "schemaVersion must be the number 2";
    }
    if (!reason) {
      const auto* points = tree.get("points");
      const auto* skill = points ? points->get("skill") : nullptr;
      if (!sane_passive_tree_integer(skill))
        reason = "points.skill must be a nonnegative integer";
    }
    if (!reason && !sane_passive_tree_integer(tree.get("earned")))
      reason = "earned must be a nonnegative integer";
    if (!reason) {
      nodes = tree.get("nodes");
      conduits = tree.get("conduits");
      if (!nodes || !nodes->array()) reason = "nodes must be an array";
      else if (!conduits || !conduits->array()) reason = "conduits must be an array";
      else if (nodes->array()->size() > kPassiveTreeTransportBound)
        reason = "nodes exceeds the passiveTree transport entry bound";
      else if (conduits->array()->size() > kPassiveTreeTransportBound)
        reason = "conduits exceeds the passiveTree transport entry bound";
    }
  }
  if (reason != nullptr) {
    events.push_back({PresentationEventType::ProtocolError, "", "",
                      std::string("passiveTree rejected: ") + reason, 0});
    return;
  }
  model.progression = ClientPassiveProgression{};
  model.progression.present = true;
  model.progression.unspent_points =
      static_cast<int>(*tree.get("points")->get("skill")->number());
  model.progression.earned_points =
      static_cast<int>(*tree.get("earned")->number());
  model.progression.node_count = static_cast<int>(nodes->array()->size());
  model.progression.conduit_count = static_cast<int>(conduits->array()->size());
  for (const auto& node : *nodes->array())
    if (node.string()) model.progression.nodes.push_back(*node.string());
  for (const auto& conduit : *conduits->array())
    if (conduit.string()) model.progression.conduits.push_back(*conduit.string());
  if (const auto* selected = tree.get("selectedNodeId");
      selected && selected->string())
    model.progression.selected_node = *selected->string();
}

void apply_player_level(ClientPlayer& player, const JsonValue& source) {
  // Login, scene admission and state snapshots all carry the actor's level.
  // Missing fields in movement deltas must retain the last authoritative value.
  const double level = json_number(source.get("level"), player.level);
  if (std::isfinite(level) && level >= 1 && level <= 2147483647.0 &&
      std::floor(level) == level)
    player.level = static_cast<int>(level);
}

void apply_player_fields(ClientPlayer& player, const JsonValue& source) {
  apply_player_level(player, source);
  if (const auto* appearance = source.get("appearance"))
    player.appearance = verdigris::player_appearance_id(appearance->string() ? *appearance->string() : "");
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
  if (!source.object()) return;
  // Login/transition scene objects replace the previous scene. Omitted town
  // stairs must retire the dungeon exit and its derived expedition phase.
  scene = ClientScene{};
  if (const auto* id = json_string(source.get("id"))) scene.id = *id;
  if (const auto* type = json_string(source.get("type"))) scene.type = *type;
  if (const auto* name = json_string(source.get("name"))) scene.name = *name;
  if (scene.type != "instance") return;
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

// ── TASK-0145: accepted Gate-B chronicle payload parsing ────────────────
// Shapes come verbatim from the frozen wire contract (TASK-0081 capture):
// chronicle {version, houses[]{id,name,scions[]{id,name,level,mortal},
// crypt[]{... relic{status,count}}, activeHouseId, activeScionId}.

void apply_chronicle_object(ClientChronicle& chronicle, const JsonValue& source) {
  const auto* houses = source.get("houses");
  if (!houses || !houses->array()) return;
  std::vector<ClientHouseEntry> parsed;
  for (const auto& entry : *houses->array()) {
    ClientHouseEntry parsed_house;
    if (const auto* id = json_string(entry.get("id"))) parsed_house.id = *id;
    if (const auto* name = json_string(entry.get("name"))) parsed_house.name = *name;
    if (const auto* scions = entry.get("scions"); scions && scions->array()) {
      for (const auto& scion_entry : *scions->array()) {
        ClientScionEntry parsed_scion;
        if (const auto* appearance = json_string(scion_entry.get("appearance")))
          parsed_scion.appearance = verdigris::player_appearance_id(*appearance);
        if (const auto* id = json_string(scion_entry.get("id"))) parsed_scion.id = *id;
        if (const auto* name = json_string(scion_entry.get("name"))) parsed_scion.name = *name;
        parsed_scion.level = static_cast<int>(json_number(scion_entry.get("level"), 1));
        if (const auto* mortal = scion_entry.get("mortal"))
          parsed_scion.mortal = mortal->boolean() && *mortal->boolean();
        parsed_house.scions.push_back(std::move(parsed_scion));
      }
    }
    if (const auto* crypt = entry.get("crypt"); crypt && crypt->array()) {
      for (const auto& crypt_entry : *crypt->array()) {
        ClientCryptEntry parsed_crypt;
        if (const auto* appearance = json_string(crypt_entry.get("appearance")))
          parsed_crypt.appearance = verdigris::player_appearance_id(*appearance);
        if (const auto* id = json_string(crypt_entry.get("id"))) parsed_crypt.id = *id;
        if (const auto* name = json_string(crypt_entry.get("name"))) parsed_crypt.name = *name;
        parsed_crypt.level = static_cast<int>(json_number(crypt_entry.get("level"), 1));
        if (const auto* relic = crypt_entry.get("relic"); relic && relic->object()) {
          if (const auto* status = json_string(relic->get("status")))
            parsed_crypt.relic_status = *status;
          parsed_crypt.relic_count = static_cast<int>(json_number(relic->get("count"), 0));
        }
        parsed_house.crypt.push_back(std::move(parsed_crypt));
      }
    }
    parsed.push_back(std::move(parsed_house));
  }
  chronicle.houses = std::move(parsed);
  chronicle.present = true;
  if (const auto* active_house = json_string(source.get("activeHouseId")))
    chronicle.active_house_id = *active_house;
  if (const auto* active_scion = json_string(source.get("activeScionId")))
    chronicle.active_scion_id = *active_scion;
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

  Envelope login{"player:login", JsonValue::Object{}};
  if (quick_guest_) {
    // Quick guests keep the historical fast path: straight into the world.
    login.data = JsonValue::Object{
        {"guestId", JsonValue(guest_id_)},
        {"quickGuest", JsonValue(quick_guest_)}};
  } else {
    // TASK-0145 owner path: await the Chronicles admission flow. The server
    // answers player:chronicles:ready (frozen contract) with the account's
    // chronicle payload instead of dropping a nameless guest into town.
    login.data = JsonValue::Object{
        {"guestId", JsonValue(guest_id_)},
        {"awaitChronicles", JsonValue(true)}};
  }
  if (!send_envelope(login)) {
    last_error_ = "login send failed";
    if (error) *error = last_error_;
    close_transport();
    return false;
  }
  return true;
}

void RemoteProtocolSession::close_transport() {
  has_player_sequence_ = false;
  clear_monster_display();
  clear_player_display();
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
  // VG-MOVE-008: encoding a command onto the wire is not input-to-photon.
  // Present markers live in the client paint path.
  Envelope envelope{"", JsonValue::Object{}};
  switch (command.type) {
    case ClientCommand::Type::Login:
      envelope.event = "player:login";
      envelope.data = JsonValue::Object{{"guestId", JsonValue(command.target)},
                                        {"quickGuest", JsonValue(command.value != 0)}};
      break;
    case ClientCommand::Type::Move: {
      // Full eight-way serialization: the server's direction table accepts
      // the compound names ("up-left", ...), so diagonals go on the wire
      // instead of being collapsed to their vertical component.
      const std::string direction = direction_name(command.dx, command.dy);
      if (direction.empty()) return;
      last_move_dir_ = direction;
      envelope.event = "player:move";
      envelope.data = JsonValue::Object{{"direction", JsonValue(direction)}};
      break;
    }
    case ClientCommand::Type::Aim: {
      const std::string direction = direction_name(command.dx, command.dy);
      if (direction.empty()) return;
      last_facing_ = direction;
      aim_held_ = true;
      model_.player.facing = last_facing_;
      return;
    }
    case ClientCommand::Type::UseAction: {
      envelope.event = "player:skill:trigger";
      // The native server reads the authoritative skill id from `skillId`.
      // Sending the old `skill` spelling silently downgraded every remote
      // input to primary-attack, which made War Cry (and future bindings)
      // look dead even though the key path fired.
      envelope.data = JsonValue::Object{{"skillId", JsonValue(command.target)},
                                        {"direction", JsonValue(last_facing_)}};
      break;
    }
    case ClientCommand::Type::PickUp:
      if (command.target.empty()) envelope.event = "player:take:underfoot";
      else {
        envelope.event = "player:context-menu:action";
        envelope.data = JsonValue::Object{{"queueItem", JsonValue::Object{
            {"action", JsonValue::Object{{"actionId", "player:take"}}},
            {"item", JsonValue::Object{{"uuid", command.target}}}}}};
      }
      break;
    case ClientCommand::Type::Equip:
      pending_equip_uuid_ = command.target;
      envelope.event = "item:equip";
      envelope.data = JsonValue::Object{
          {"item", JsonValue::Object{{"uuid", JsonValue(command.target)}}}};
      if (!command.extra.empty())
        (*(*envelope.data.object())["item"].object())["targetSlot"] = JsonValue(command.extra);
      break;
    case ClientCommand::Type::Unequip:
      envelope.event = "item:unequip";
      envelope.data = JsonValue::Object{{"seat", command.target}};
      break;
    case ClientCommand::Type::EnterZone:
      model_.chart.open = false;
      envelope.event = "world:zone:enter";
      envelope.data = JsonValue::Object{{"nodeId", JsonValue(command.target)}};
      break;
    case ClientCommand::Type::Extract: {
      const auto& scene = model_.scene;
      if (!model_.player.alive || scene.type != "instance" || !scene.has_stairs_up ||
          (std::max)(std::abs(std::round(model_.player.x) - scene.stairs_up_x),
                   std::abs(std::round(model_.player.y) - scene.stairs_up_y)) > 1.0) {
        pending_events_.push_back({PresentationEventType::Message, "", "",
                                   "Reach the exit stairs to return to the surface.", 0});
        return;
      }
      envelope.event = "player:extract";
      break;
    }
    case ClientCommand::Type::FoundHouse:
      envelope.event = "chronicles:house:found";
      envelope.data = JsonValue::Object{{"name", JsonValue(command.target)}};
      break;
    case ClientCommand::Type::CreateScion: {
      std::string house_id = command.house_id.empty() ? model_.chronicle.active_house_id : command.house_id;
      if (find_chronicle_house(model_.chronicle, house_id) == nullptr &&
          !model_.chronicle.houses.empty())
        house_id = model_.chronicle.houses.front().id;
      envelope.event = "chronicles:scion:create";
      envelope.data = JsonValue::Object{{"houseId", JsonValue(house_id)},
                                        {"name", JsonValue(command.target)},
                                        {"appearance", verdigris::player_appearance_id(command.extra)}};
      break;
    }
    case ClientCommand::Type::SelectScion: {
      // Resolve the scion's House from the authoritative chronicle roster.
      std::string house_id = model_.chronicle.active_house_id;
      for (const auto& house : model_.chronicle.houses) {
        bool found = false;
        for (const auto& scion : house.scions)
          if (scion.id == command.target) found = true;
        if (found) {
          house_id = house.id;
          break;
        }
      }
      const ClientScionEntry* scion =
          find_chronicle_scion(model_.chronicle, command.target);
      const std::string scion_name = scion ? scion->name : std::string{};
      envelope.event = "player:chronicles:select";
      envelope.data = JsonValue::Object{
          {"scionId", JsonValue(command.target)},
          {"houseId", JsonValue(house_id)},
          {"scionName", JsonValue(scion_name)},
          {"mortal", JsonValue(command.value != 0)}};
      break;
    }
    case ClientCommand::Type::SetOut:
      envelope.event = "chronicles:scion:set-out";
      envelope.data = JsonValue::Object{{"scionId", JsonValue(command.target)}};
      break;
    case ClientCommand::Type::NpcAction: {
      // The server dispatches NPC verbs through the context-menu action
      // surface: queueItem carries the actionId and the NPC item reference.
      envelope.event = "player:context-menu:action";
      envelope.data = JsonValue::Object{
          {"queueItem",
           JsonValue::Object{
               {"action", JsonValue::Object{{"actionId", JsonValue(command.target)}}},
               {"item", JsonValue::Object{{"id", JsonValue(command.value)}}}}}};
      break;
    }
    case ClientCommand::Type::MenuAction: {
      // Generic context-menu action with an item reference. The item object
      // carries the ref under both keys the server reads ("id" for shop buy,
      // "uuid" for sell/withdraw/deposit) plus the numeric field under both
      // of its spellings; handlers pick the fields they own.
      envelope.event = "player:context-menu:action";
      envelope.data = JsonValue::Object{
          {"queueItem",
           JsonValue::Object{
               {"action", JsonValue::Object{{"actionId", JsonValue(command.target)}}},
               {"item", JsonValue::Object{{"id", JsonValue(command.extra)},
                                          {"uuid", JsonValue(command.extra)},
                                          {"price", JsonValue(command.value)},
                                          {"qty", JsonValue(command.value)}}}}}};
      break;
    }
    case ClientCommand::Type::CloseScreen:
      // Pane dismissal is presentation-local; the server keeps no modal.
      model_.shop.open = false;
      model_.bank.open = false;
      model_.chart.open = false;
      return;
    case ClientCommand::Type::AllocateNode: {
      // Extend the authoritative allocation by one node and save the whole
      // snapshot (the wire's unit of tree persistence). The server owns the
      // point budget; the client only proposes.
      if (!model_.progression.present) return;
      JsonValue::Array nodes;
      bool already = false;
      for (const auto& node : model_.progression.nodes) {
        if (node == command.target) already = true;
        nodes.emplace_back(node);
      }
      if (already) return;
      nodes.emplace_back(command.target);
      JsonValue::Array conduits;
      for (const auto& conduit : model_.progression.conduits)
        conduits.emplace_back(conduit);
      JsonValue::Object snapshot;
      snapshot.emplace("schemaVersion", JsonValue(2));
      snapshot.emplace("nodes", JsonValue(std::move(nodes)));
      snapshot.emplace("conduits", JsonValue(std::move(conduits)));
      snapshot.emplace(
          "selectedNodeId",
          JsonValue(model_.progression.selected_node.empty()
                        ? std::string("0,0")
                        : model_.progression.selected_node));
      envelope.event = "player:skilltree:save";
      envelope.data = JsonValue::Object{{"snapshot", JsonValue(std::move(snapshot))}};
      break;
    }
  }
  if (!envelope.event.empty()) send_envelope(envelope);
}

void RemoteProtocolSession::poll() {
  // Authoritative monster/ground sync: the server's dev:state snapshot is
  // the source of truth (browser parity) — inference from combat envelopes
  // alone can miss fast kills entirely. Throttled to ~4Hz while Ready.
  if (state_.load() == ConnectionState::Ready) {
    const auto now = std::chrono::steady_clock::now();
    if (now - last_state_request_ > std::chrono::milliseconds(250)) {
      last_state_request_ = now;
      // Ask for the walkable grid whenever the scene we hold a map for is
      // not the scene the player is in (including the empty initial state).
      const bool need_map =
          model_.map_scene_id.empty() ||
          model_.map_scene_id != model_.player.scene_id;
      Envelope request{"dev:state",
                       JsonValue::Object{{"requestId", JsonValue("model-sync")},
                                         {"includeMap", JsonValue(need_map)}}};
      send_envelope(request);
    }
  }
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
  sample_monster_display();
  sample_player_display();
}

std::vector<PresentationEvent> RemoteProtocolSession::drain_events() {
  std::vector<PresentationEvent> drained;
  drained.swap(pending_events_);
  return drained;
}

bool RemoteProtocolSession::send_raw(const std::string& event, verdigris::networking::JsonValue data) {
  Envelope envelope{event, std::move(data)};
  return send_envelope(envelope);
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

void RemoteProtocolSession::clear_player_display() {
  has_player_movement_ = false;
  model_.player.has_display_position = false;
}

void RemoteProtocolSession::apply_player_movement(const Envelope& envelope) {
  const auto* id = json_string(envelope.data.get("uuid"));
  if (!id || *id != model_.player.uuid) return;
  const auto* x = envelope.data.get("x");
  const auto* y = envelope.data.get("y");
  if (!x || !x->number() || !y || !y->number() ||
      !std::isfinite(*x->number()) || !std::isfinite(*y->number())) return;
  const auto* scene = json_string(envelope.data.get("sceneId"));
  const bool changed_scene = scene && *scene != model_.player.scene_id;
  const auto* meta = envelope.meta ? &*envelope.meta : nullptr;
  const double sequence = meta ? json_number(meta->get("sequence"), -1.0) : -1.0;
  const double duration = meta ? json_number(meta->get("duration"), 0.0) : 0.0;
  const bool valid = std::isfinite(sequence) && sequence >= 0.0 &&
      sequence <= 9007199254740991.0 && std::floor(sequence) == sequence &&
      std::isfinite(duration) && duration >= 0.0;
  if (has_player_sequence_ && (!valid || sequence <= last_player_sequence_)) return;
  if (changed_scene) clear_player_display();
  const double from_x = model_.player.x, from_y = model_.player.y;
  apply_player_fields(model_.player, envelope.data);
  if (aim_held_) model_.player.facing = last_facing_;
  else if (!model_.player.facing.empty()) last_facing_ = model_.player.facing;
  if (!valid) { clear_player_display(); return; }
  last_player_sequence_ = static_cast<std::uint64_t>(sequence);
  has_player_sequence_ = true;
  player_movement_.sequence = last_player_sequence_;
  const double wire_from_x = json_number(meta->get("fromX"), from_x);
  const double wire_from_y = json_number(meta->get("fromY"), from_y);
  const bool valid_from = std::isfinite(wire_from_x) && std::isfinite(wire_from_y) &&
      std::hypot(model_.player.x - wire_from_x, model_.player.y - wire_from_y) <= 4.0;
  player_movement_.from_x = valid_from ? wire_from_x : from_x;
  player_movement_.from_y = valid_from ? wire_from_y : from_y;
  player_movement_.to_x = model_.player.x; player_movement_.to_y = model_.player.y;
  player_movement_.duration_ms = static_cast<int>(std::clamp(duration, 0.0, tile_movement::kSampleMs));
  if (changed_scene || json_bool(meta->get("blocked")) || !scene ||
      std::hypot(model_.player.x - from_x, model_.player.y - from_y) > 4.0)
    player_movement_.duration_ms = 0;
  player_movement_.received_at = std::chrono::steady_clock::now();
  has_player_movement_ = true;
  const auto* action = json_string(meta->get("action"));
  if (action && *action == "dash" && !changed_scene && valid_from &&
      !json_bool(meta->get("blocked")) && meta->get("fromX") && meta->get("fromY") &&
      std::hypot(model_.player.x - wire_from_x, model_.player.y - wire_from_y) > 1e-6) {
    PresentationEvent event{PresentationEventType::PlayerDashed, model_.player.uuid, "", "dash", 0};
    event.from_x = static_cast<int>(std::lround(protocol_to_world(wire_from_x)));
    event.from_y = static_cast<int>(std::lround(protocol_to_world(wire_from_y)));
    event.to_x = static_cast<int>(std::lround(protocol_to_world(model_.player.x)));
    event.to_y = static_cast<int>(std::lround(protocol_to_world(model_.player.y)));
    event.has_actor_pose = true; event.actor_x = event.to_x; event.actor_y = event.to_y;
    pending_events_.push_back(std::move(event));
  }
}

void RemoteProtocolSession::sample_player_display() {
  if (!has_player_movement_ || !model_.player.alive) {
    model_.player.has_display_position = false;
    return;
  }
  const double elapsed = std::chrono::duration<double, std::milli>(
      std::chrono::steady_clock::now() - player_movement_.received_at).count();
  const double t = player_movement_.duration_ms > 0
      ? std::clamp(elapsed / player_movement_.duration_ms, 0.0, 1.0) : 1.0;
  model_.player.display_x = player_movement_.from_x +
      (player_movement_.to_x - player_movement_.from_x) * t;
  model_.player.display_y = player_movement_.from_y +
      (player_movement_.to_y - player_movement_.from_y) * t;
  model_.player.has_display_position = true;
}

void RemoteProtocolSession::apply_ground_items(const JsonValue& items, bool announce) {
  const auto* list = items.array();
  if (!list || list->size() > 1024) return;
  std::vector<ClientGroundItem> ground;
  for (const auto& entry : *list) {
    const auto* uuid = json_string(entry.get("uuid"));
    const auto* x = entry.get("x"); const auto* y = entry.get("y");
    if (!uuid || uuid->empty() || !x || !x->number() || !y || !y->number() ||
        !std::isfinite(*x->number()) || !std::isfinite(*y->number())) continue;
    ClientGroundItem item;
    item.uuid = *uuid; item.x = *x->number(); item.y = *y->number();
    if (const auto* name = json_string(entry.get("name"))) item.name = *name;
    if (const auto* relic = entry.get("chroniclesRelic"); relic && relic->object()) {
      item.relic = true;
      if (const auto* name = json_string(relic->get("scionName"))) item.relic_of = *name;
    }
    if (std::any_of(ground.begin(), ground.end(), [&](const ClientGroundItem& old) { return old.uuid == item.uuid; })) continue;
    const bool known = std::any_of(model_.ground.begin(), model_.ground.end(),
        [&](const ClientGroundItem& old) { return old.uuid == item.uuid; });
    if (announce && !known) {
      PresentationEvent event{PresentationEventType::ItemDropped, "", item.uuid, item.name, 0};
      event.has_actor_pose = true;
      event.actor_x = static_cast<int>(std::lround(protocol_to_world(item.x)));
      event.actor_y = static_cast<int>(std::lround(protocol_to_world(item.y)));
      pending_events_.push_back(std::move(event));
    }
    ground.push_back(std::move(item));
  }
  model_.ground = std::move(ground);
}

void RemoteProtocolSession::clear_monster_display() {
  monster_movement_.clear();
  for (auto& monster : model_.monsters) monster.has_display_position = false;
}

bool RemoteProtocolSession::apply_monster_movement(ClientMonster& monster,
                                                   const JsonValue& source) {
  const auto* step = source.get("movementStep");
  const auto prior = monster_movement_.find(monster.id);
  const double sequence = step ? json_number(step->get("sequence"), -1.0) : -1.0;
  const bool valid = std::isfinite(sequence) && sequence >= 0.0 &&
      sequence <= 9007199254740991.0 && std::floor(sequence) == sequence;
  const double facing_x = step ? json_number(step->get("facingX"), 0.0) : 0.0;
  const double facing_y = step ? json_number(step->get("facingY"), 0.0) : 0.0;
  const bool valid_facing = std::isfinite(facing_x) && std::isfinite(facing_y) &&
      std::abs(facing_x) <= 1.0 && std::abs(facing_y) <= 1.0 &&
      std::floor(facing_x) == facing_x && std::floor(facing_y) == facing_y &&
      (facing_x != 0.0 || facing_y != 0.0);
  const auto copy_facing = [&](const MonsterMovement& motion) {
    monster.has_facing = motion.has_facing;
    monster.facing_x = motion.facing_x; monster.facing_y = motion.facing_y;
  };
  if (prior != monster_movement_.end() &&
      (!valid || sequence <= static_cast<double>(prior->second.sequence))) {
    // dev:state remains a lifecycle/stat reconciliation. A same/older
    // sequence cannot restart interpolation or rewind the motion endpoint.
    monster.x = prior->second.to_x;
    monster.y = prior->second.to_y;
    if (valid && sequence == static_cast<double>(prior->second.sequence) && valid_facing) {
      prior->second.has_facing = true;
      prior->second.facing_x = static_cast<int>(facing_x);
      prior->second.facing_y = static_cast<int>(facing_y);
    }
    copy_facing(prior->second);
    return valid && sequence == static_cast<double>(prior->second.sequence);
  }
  if (!valid || !monster.alive || !std::isfinite(monster.x) ||
      !std::isfinite(monster.y)) return false;
  if (prior == monster_movement_.end() && monster_movement_.size() >= 256) return false;
  MonsterMovement motion;
  if (prior != monster_movement_.end()) {
    motion.has_facing = prior->second.has_facing;
    motion.facing_x = prior->second.facing_x; motion.facing_y = prior->second.facing_y;
  }
  if (valid_facing) {
    motion.has_facing = true;
    motion.facing_x = static_cast<int>(facing_x);
    motion.facing_y = static_cast<int>(facing_y);
  }
  motion.sequence = static_cast<std::uint64_t>(sequence);
  motion.to_x = monster.x;
  motion.to_y = monster.y;
  motion.from_x = json_number(step->get("fromX"), monster.x);
  motion.from_y = json_number(step->get("fromY"), monster.y);
  const double duration = json_number(step->get("duration"), 0.0);
  if (!std::isfinite(motion.from_x) || !std::isfinite(motion.from_y) ||
      !std::isfinite(duration)) return false;
  motion.duration_ms = static_cast<int>(std::clamp(duration, 0.0, 250.0));
  // A teleport or newly admitted actor has no inferred travel path.
  if (sequence == 0.0 || std::hypot(motion.to_x - motion.from_x,
                                   motion.to_y - motion.from_y) > 4.0)
    motion.duration_ms = 0;
  const auto received_at = std::chrono::steady_clock::now();
  // A server stop packet can arrive in the same client poll as the movement
  // packet that reaches that endpoint.  Replacing the active segment with a
  // zero-duration stop would make the actor visibly snap before a single
  // presentation frame, even though the authoritative path was valid.  Keep
  // the active segment's clock until it naturally completes; the newer
  // sequence and endpoint still remain authoritative for collision/combat.
  const bool prior_segment_active = prior != monster_movement_.end() &&
      prior->second.duration_ms > 0 &&
      std::chrono::duration_cast<std::chrono::milliseconds>(
          received_at - prior->second.received_at).count() < prior->second.duration_ms;
  if (prior_segment_active && motion.duration_ms == 0 &&
      std::hypot(motion.to_x - prior->second.to_x,
                 motion.to_y - prior->second.to_y) <= 1e-6) {
    prior->second.sequence = motion.sequence;
    prior->second.to_x = motion.to_x;
    prior->second.to_y = motion.to_y;
    prior->second.has_facing = motion.has_facing;
    prior->second.facing_x = motion.facing_x;
    prior->second.facing_y = motion.facing_y;
    copy_facing(prior->second);
    return true;
  }
  motion.received_at = received_at;
  monster_movement_[monster.id] = motion;
  copy_facing(motion);
  return true;
}

void RemoteProtocolSession::sample_monster_display() {
  const auto now = std::chrono::steady_clock::now();
  for (auto it = monster_movement_.begin(); it != monster_movement_.end();) {
    ClientMonster* monster = find_monster(model_, it->first);
    if (!monster || !monster->alive) {
      if (monster) monster->has_display_position = false;
      it = monster_movement_.erase(it);
      continue;
    }
    const auto& motion = it->second;
    const double elapsed = std::chrono::duration<double, std::milli>(
        now - motion.received_at).count();
    const double phase = motion.duration_ms > 0
        ? std::clamp(elapsed / motion.duration_ms, 0.0, 1.0) : 1.0;
    monster->display_x = motion.from_x + (motion.to_x - motion.from_x) * phase;
    monster->display_y = motion.from_y + (motion.to_y - motion.from_y) * phase;
    monster->has_display_position = true;
    ++it;
  }
}

void RemoteProtocolSession::apply_envelope(const Envelope& envelope) {
  if (envelope.event == "item:equip:rejected") {
    const std::string id = json_string(envelope.data.get("uuid")) ? *json_string(envelope.data.get("uuid")) : "";
    const std::string reason = json_string(envelope.data.get("reason")) ? *json_string(envelope.data.get("reason")) : "Equip rejected";
    if (pending_equip_uuid_ == id) pending_equip_uuid_.clear();
    pending_events_.push_back({PresentationEventType::EquipRejected, model_.player.uuid, id, reason, 0});
    return;
  }
  if (envelope.event == "player:equippedAnItem") {
    const auto* actor = json_string(envelope.data.get("uuid"));
    if (!actor || *actor != model_.player.uuid) return;
    if (const auto* wear = envelope.data.get("wearDetails")) apply_wear_details(*wear, model_);
    if (const auto* combat = envelope.data.get("combat")) apply_combat_fields(*combat, model_.player);
    // A disappearing backpack item is not an equip acknowledgement.
    for (const auto& worn : model_.worn) {
      if (pending_equip_uuid_.empty() || worn.item.uuid != pending_equip_uuid_) continue;
      pending_events_.push_back({PresentationEventType::ItemEquipped, model_.player.uuid,
                                worn.item.uuid, worn.item.name, worn.item.attack_rating});
      pending_equip_uuid_.clear();
      break;
    }
    return;
  }
  if (envelope.event == "player:login") {
    model_.player.appearance = "male";
    model_.player.combat_stats_present = false;
    has_player_sequence_ = false;
    clear_monster_display();
    clear_player_display();
    if (const auto* player = envelope.data.get("player")) {
      apply_player_fields(model_.player, *player);
      if (const auto* username = json_string(player->get("username")))
        model_.player.display_name = *username;
      if (const auto* chronicles = player->get("chronicles")) {
        // Admission payload: the active scion/house ids and oath are
        // authoritative here (player_payload puts them at :590).
        if (const auto* scion_id = json_string(chronicles->get("scionId")))
          model_.chronicle.active_scion_id = *scion_id;
        if (const auto* house_id = json_string(chronicles->get("houseId")))
          model_.chronicle.active_house_id = *house_id;
      }
      if (const auto* wear = player->get("wearDetails"))
        apply_wear_details(*wear, model_);
      if (const auto* combat = player->get("combat"))
        apply_combat_fields(*combat, model_.player);
      last_facing_ = model_.player.facing.empty() ? last_facing_ : model_.player.facing;
      model_.inventory.clear();
      if (const auto* inventory = player->get("inventory")) {
        if (const auto* slots = inventory->get("slots"); slots && slots->array()) {
          for (const auto& entry : *slots->array()) {
            model_.inventory.push_back(parse_item_slot(entry));
          }
        }
      }
      // TASK-0156: the admission payload carries the authoritative
      // passiveTree envelope (player_payload puts it beside quests).
      if (const auto* tree = player->get("passiveTree"))
        apply_passive_tree(*tree, model_, pending_events_);
    }
    if (const auto* scene = envelope.data.get("scene")) apply_scene_fields(model_.scene, *scene);
    if (const auto* ground = envelope.data.get("droppedItems")) apply_ground_items(*ground, false);
    // A full player:login is a world admission on the Gate-B journey: the
    // owner has left the front door with a living Scion.
    model_.chronicles_pending = false;
    model_.player.alive = true;
    state_.store(ConnectionState::Ready);
    ever_ready_ = true;
    retry_attempt_ = 0;
    pending_events_.push_back(
        {PresentationEventType::SessionReady, model_.player.uuid, "", "", 0});
    return;
  }
  if (envelope.event == "chronicles:state" || envelope.event == "player:chronicles:ready" ||
      envelope.event == "player:chronicles:update") {
    const JsonValue* chronicle = envelope.data.get("chronicle");
    if (!chronicle) chronicle = envelope.data.get("chronicles");
    if (chronicle) apply_chronicle_object(model_.chronicle, *chronicle);
    // The account payload itself opens the door even before anything is
    // founded (a fresh chronicle carries no houses yet).
    model_.chronicle.present = true;
    if (const auto* account = json_string(envelope.data.get("accountName")))
      model_.chronicle.account_name = *account;
    if (envelope.event != "player:chronicles:update") {
      // state/ready mean the socket sits at the pre-game front door; update
      // is a roster refresh while already admitted.
      model_.chronicles_pending = true;
    }
    if (const auto* fallen = envelope.data.get("fallen")) {
      if (const auto* scion_id = json_string(fallen->get("scionId")))
        model_.chronicle.fallen.scion_id = *scion_id;
      if (const auto* name = json_string(fallen->get("scionName")))
        model_.chronicle.fallen.name = *name;
    }
    return;
  }
  if (envelope.event == "chronicles:scion-fallen") {
    if (const auto* fallen = envelope.data.get("fallen")) {
      model_.chronicle.fallen = ClientFallenScion{};
      if (const auto* scion_id = json_string(fallen->get("scionId")))
        model_.chronicle.fallen.scion_id = *scion_id;
      if (const auto* name = json_string(fallen->get("name")))
        model_.chronicle.fallen.name = *name;
      model_.chronicle.fallen.level = static_cast<int>(json_number(fallen->get("level"), 1));
    }
    model_.chronicle.fallen.relic_count =
        static_cast<int>(json_number(envelope.data.get("relicCount"), 0));
    if (const auto* chronicle = envelope.data.get("chronicle"))
      apply_chronicle_object(model_.chronicle, *chronicle);
    model_.player.alive = false;
    pending_events_.push_back(
        {PresentationEventType::ScionDied, model_.chronicle.fallen.scion_id, "",
         model_.chronicle.fallen.name, 0});
    pending_events_.push_back(
        {PresentationEventType::Message, "", "",
         "The chronicle records the fall of " + model_.chronicle.fallen.name + ".", 0});
    return;
  }
  if (envelope.event == "open:screen") {
    // Authoritative trader/countinghouse screens: mirrored into the model
    // verbatim for the pane painters. `open` clears only via CloseScreen.
    // The server emits {player, screen, payload} at the envelope's top
    // level; tolerate a nested data wrapper for forward compatibility.
    const auto* data = envelope.data.get("screen") ? &envelope.data
                                                   : envelope.data.get("data");
    const auto* screen = json_string(data ? data->get("screen") : nullptr);
    const auto* payload = data ? data->get("payload") : nullptr;
    if (screen && payload) {
      if (*screen == "shop") {
        ClientShopScreen shop;
        shop.open = true;
        if (const auto* name = json_string(payload->get("name"))) shop.name = *name;
        shop.carried_coins =
            static_cast<int>(json_number(payload->get("carriedCoins"), 0.0));
        if (const auto* items = payload->get("items"); items && items->array()) {
          for (const auto& row : *items->array()) {
            ClientShopRow entry;
            if (const auto* id = json_string(row.get("id"))) entry.id = *id;
            if (const auto* row_name = json_string(row.get("name")))
              entry.name = *row_name;
            entry.price = static_cast<int>(json_number(row.get("price"), 0.0));
            entry.qty = static_cast<int>(json_number(row.get("qty"), 0.0));
            shop.rows.push_back(std::move(entry));
          }
        }
        model_.shop = std::move(shop);
        model_.bank.open = false;
      } else if (*screen == "chart") {
        ClientChartScreen chart;
        chart.open = true;
        if (const auto* road = json_string(payload->get("roadId")))
          chart.road_id = *road;
        if (const auto* name = json_string(payload->get("roadName")))
          chart.road_name = *name;
        if (const auto* blurb = json_string(payload->get("blurb")))
          chart.blurb = *blurb;
        if (const auto* nodes = payload->get("nodes"); nodes && nodes->array()) {
          for (const auto& row : *nodes->array()) {
            ClientChartNode node;
            if (const auto* id = json_string(row.get("id"))) node.id = *id;
            if (const auto* node_name = json_string(row.get("name")))
              node.name = *node_name;
            if (const auto* warden = json_string(row.get("wardenName")))
              node.warden = *warden;
            if (const auto* status = json_string(row.get("status")))
              node.status = *status;
            node.tier = static_cast<int>(json_number(row.get("tier"), 1.0));
            chart.nodes.push_back(std::move(node));
          }
        }
        model_.chart = std::move(chart);
        model_.shop.open = false;
        model_.bank.open = false;
      } else if (*screen == "bank") {
        ClientBankScreen bank;
        bank.open = true;
        bank.carried_coins =
            static_cast<int>(json_number(payload->get("carriedCoins"), 0.0));
        if (const auto* house = payload->get("house"))
          bank.treasury =
              static_cast<int>(json_number(house->get("treasury"), 0.0));
        if (const auto* items = payload->get("items"); items && items->array()) {
          for (const auto& row : *items->array()) {
            ClientBankItem entry;
            if (const auto* uuid = json_string(row.get("uuid"))) entry.uuid = *uuid;
            if (const auto* row_name = json_string(row.get("name")))
              entry.name = *row_name;
            if (entry.name.empty())
              if (const auto* id = json_string(row.get("id"))) entry.name = *id;
            entry.qty = static_cast<int>(json_number(row.get("qty"), 0.0));
            bank.items.push_back(std::move(entry));
          }
        }
        model_.bank = std::move(bank);
        model_.shop.open = false;
      }
    }
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
  if (envelope.event == "player:extract") {
    // Full authority lists are totals, never deltas; duplicate summaries and
    // later dev:state reconciliation must not count the same item twice.
    if (const auto* items = envelope.data.get("storedItems"); items && items->array())
      model_.stored_items = static_cast<int>(items->array()->size());
    if (const auto* trophies = envelope.data.get("storedTrophies"); trophies && trophies->array())
      model_.stored_trophies = static_cast<int>(trophies->array()->size());
    return;
  }
  if (envelope.event == "player:movement") {
    apply_player_movement(envelope);
    return;
  }
  if (envelope.event == "world:scene:transition" ||
      envelope.event == "party:scene:transition") {
    if (const auto* scene = envelope.data.get("scene")) apply_scene_fields(model_.scene, *scene);
    if (const auto* player_state = envelope.data.get("playerState")) {
      apply_player_fields(model_.player, *player_state);
    }
    if (!model_.scene.id.empty()) model_.player.scene_id = model_.scene.id;
    clear_monster_display();
    clear_player_display();
    model_.monsters.clear();
    model_.npcs.clear();
    model_.ground.clear();
    if (const auto* scene = envelope.data.get("scene"))
      if (const auto* items = scene->get("droppedItems")) apply_ground_items(*items, false);
    return;
  }
  if (envelope.event == "world:itemDropped" || envelope.event == "item:change") {
    const auto* scene = envelope.meta ? json_string(envelope.meta->get("sceneId")) : nullptr;
    if (!scene || model_.player.scene_id.empty() || *scene != model_.player.scene_id) return;
    if (const auto* items = envelope.data.get("data")) apply_ground_items(*items, true);
    return;
  }
  if (envelope.event == "monster:state") {
    const auto* actors = envelope.data.array();
    const auto* scene = envelope.meta ? json_string(envelope.meta->get("sceneId")) : nullptr;
    const std::string& current_scene = model_.player.scene_id;
    // Scene-scoped deltas never admit unknown actors or populate a room.
    // The ordinary full snapshot remains authoritative for membership.
    if (!actors || actors->size() > 256 || !scene || current_scene.empty() ||
        *scene != current_scene) return;
    for (const auto& entry : *actors) {
      const auto* id = json_string(entry.get("uuid"));
      if (!id) continue;
      ClientMonster* existing = find_monster(model_, *id);
      if (!existing || !existing->alive) continue;
      const auto* x = entry.get("x");
      const auto* y = entry.get("y");
      if (!x || !x->number() || !y || !y->number() ||
          !std::isfinite(*x->number()) || !std::isfinite(*y->number())) continue;
      ClientMonster update = *existing;
      update.x = *x->number(); update.y = *y->number();
      if (!apply_monster_movement(update, entry)) continue;
      if (const auto* hp = entry.get("hp")) {
        update.life = static_cast<int>(json_number(hp->get("current"), update.life));
        update.life_max = static_cast<int>(json_number(hp->get("max"), update.life_max));
        update.alive = update.life > 0;
      }
      *existing = std::move(update);
    }
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
      // A confirmed hit does not change the authored actor's rarity/scale.
      if (attacker && !find_monster(model_, *attacker))
        upsert_monster(model_, *attacker, "", true);
      model_.last_incoming_hit = amount;
      const auto* skill = json_string(envelope.data.get("skillId"));
      if (attacker && !attacker->empty() && *attacker != model_.player.uuid && skill) {
        const auto* foe = find_monster(model_, *attacker);
        // monster:attack is also used by ranged foes. Only an explicit melee
        // action or the server's melee behaviour snapshot confirms a swing;
        // attackStyle describes damage and cannot identify the action.
        const bool explicit_melee = *skill == "melee" || *skill == "thrust" ||
                                    *skill == "sweep";
        const bool known_melee = *skill == "monster:attack" && foe &&
                                 foe->behaviour == "melee";
        if (explicit_melee || known_melee) {
          pending_events_.push_back({PresentationEventType::AttackStarted,
                                     *attacker, "", known_melee ? "melee" : *skill,
                                     amount});
        }
      }
      pending_events_.push_back({PresentationEventType::DamageApplied,
                                 attacker ? *attacker : "", "", "incoming", amount});
      if (died) {
        pending_events_.push_back(
            {PresentationEventType::ScionDied, model_.player.uuid, "", "", 0});
      }
    } else {
      model_.last_outgoing_hit = amount;
      // TASK-0122 Phase A: consume the already-shipped combat:hit parity
      // fields (server networking.cpp emits critical/attackStyle). Copied
      // verbatim into the presentation event; the client never computes them
      // and the envelope stays untouched.
      bool critical = false;
      if (const auto* crit = envelope.data.get("critical"))
        critical = crit->boolean() && *crit->boolean();
      std::string style;
      if (const auto* style_value = json_string(envelope.data.get("attackStyle")))
        style = *style_value;
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
      // skillId names the confirmed action; attackStyle is a damage channel,
      // and last_facing_ is only a direction. Neither can preserve Sweep.
      const auto* skill = json_string(envelope.data.get("skillId"));
      pending_events_.push_back({PresentationEventType::AttackStarted,
                                 attacker ? *attacker : model_.player.uuid, "",
                                 skill ? *skill : "primary-attack", amount});
      PresentationEvent outgoing;
      outgoing.type = PresentationEventType::DamageApplied;
      outgoing.actor_id = target ? *target : "";
      outgoing.text = "outgoing";
      outgoing.value = amount;
      outgoing.critical = critical;
      outgoing.style = style;
      pending_events_.push_back(std::move(outgoing));
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

      }
    }
    return;
  }
  if (envelope.event == "dev:state") {
    const auto* state = envelope.data.get("state");
    if (!state) return;
    if (const auto* combat = state->get("combat")) apply_combat_fields(*combat, model_.player);
    apply_player_level(model_.player, *state);
    if (const auto* appearance = json_string(state->get("appearance")))
      model_.player.appearance = verdigris::player_appearance_id(*appearance);
    if (const auto* items = state->get("houseStoredItems"); items && items->array())
      model_.stored_items = static_cast<int>(items->array()->size());
    // Authoritative lifecycle + oath visibility (snapshot puts these at the
    // top of dev:state). Keeps death/successor states honest between
    // chronicle payloads.
    if (const auto* lifecycle = json_string(state->get("lifecycle")))
      model_.lifecycle = *lifecycle;
    // TASK-0156: the dev:state snapshot carries the same authoritative
    // passiveTree envelope; keep the mirror current between logins. TASK-0162:
    // a malformed snapshot fails closed and surfaces its diagnostic.
    if (const auto* tree = state->get("passiveTree"))
      apply_passive_tree(*tree, model_, pending_events_);
    if (const auto* hp = state->get("hp")) {
      // Authoritative life keeps alive honest between combat envelopes.
      model_.player.life = static_cast<int>(json_number(hp->get("current"), model_.player.life));
      model_.player.life_max =
          static_cast<int>(json_number(hp->get("max"), model_.player.life_max));
      model_.player.alive = model_.player.life > 0;
    }
    if (const auto* attributes = state->get("attributes")) {
      model_.attr_strength = static_cast<int>(
          json_number(attributes->get("strength"), model_.attr_strength));
      model_.attr_dexterity = static_cast<int>(
          json_number(attributes->get("dexterity"), model_.attr_dexterity));
      model_.attr_intelligence = static_cast<int>(
          json_number(attributes->get("intelligence"), model_.attr_intelligence));
    }
    if (const auto* record = state->get("chroniclesRecord")) {
      if (json_number(record->get("revision"), 0) > 0.0) {
        if (const auto* chronicle = record->get("state"))
          apply_chronicle_object(model_.chronicle, *chronicle);
      }
    }
    if (const auto* theme = json_string(state->get("theme")))
      model_.theme = *theme;
    if (const auto* xp = state->get("xp")) {
      model_.xp_current = json_number(xp->get("current"), model_.xp_current);
      model_.xp_floor = json_number(xp->get("floor"), model_.xp_floor);
      model_.xp_next = json_number(xp->get("next"), model_.xp_next);
    }
    if (const auto* wear = state->get("wearDetails"))
      apply_wear_details(*wear, model_);
    if (const auto* monsters = state->get("monsters"); monsters && monsters->array()) {
      model_.monsters.clear();
      for (const auto& entry : *monsters->array()) {
        ClientMonster monster;
        if (const auto* uuid = json_string(entry.get("uuid"))) monster.id = *uuid;
        if (const auto* name = json_string(entry.get("name"))) monster.name = *name;
        if (const auto* kind = json_string(entry.get("id"))) monster.kind = *kind;
        if (const auto* behaviour = entry.get("behaviour"))
          if (const auto* type = json_string(behaviour->get("type")))
            monster.behaviour = *type;
        monster.x = json_number(entry.get("x"), 0.0);
        monster.y = json_number(entry.get("y"), 0.0);
        if (const auto* hp = entry.get("hp")) {
          monster.life = static_cast<int>(json_number(hp->get("current"), monster.life));
          monster.life_max = static_cast<int>(json_number(hp->get("max"), monster.life_max));
        }
        if (const auto* rarity = json_string(entry.get("rarity"))) {
          monster.elite = (*rarity != "normal" && *rarity != "common" && !rarity->empty());
        }
        monster.alive = monster.life > 0;
        apply_monster_movement(monster, entry);
        model_.monsters.push_back(std::move(monster));
      }
    }
    if (const auto* map = state->get("map"); map && map->object()) {
      const int width = static_cast<int>(json_number(map->get("width"), 0.0));
      const int height = static_cast<int>(json_number(map->get("height"), 0.0));
      const auto* rows = map->get("rows");
      if (width > 0 && height > 0 && rows && rows->array() &&
          static_cast<int>(rows->array()->size()) == height) {
        model_.map_width = width;
        model_.map_height = height;
        if (const auto* scene = json_string(map->get("sceneId")))
          model_.map_scene_id = *scene;
        model_.map_walkable.assign(
            static_cast<std::size_t>(width) * static_cast<std::size_t>(height),
            1);
        for (int y = 0; y < height; ++y) {
          const auto* row = (*rows->array())[static_cast<std::size_t>(y)].string();
          if (!row || static_cast<int>(row->size()) != width) continue;
          for (int x = 0; x < width; ++x)
            if ((*row)[static_cast<std::size_t>(x)] == '0')
              model_.map_walkable[static_cast<std::size_t>(y) * width + x] = 0;
        }
      }
    }
    if (const auto* npcs = state->get("npcs"); npcs && npcs->array()) {
      model_.npcs.clear();
      for (const auto& entry : *npcs->array()) {
        ClientNpc npc;
        npc.id = static_cast<int>(json_number(entry.get("id"), 0.0));
        if (const auto* name = json_string(entry.get("name"))) npc.name = *name;
        npc.x = json_number(entry.get("x"), 0.0);
        npc.y = json_number(entry.get("y"), 0.0);
        if (const auto* actions = entry.get("actions"); actions && actions->array()) {
          for (const auto& action : *actions->array())
            if (action.string()) npc.actions.push_back(*action.string());
        }
        model_.npcs.push_back(std::move(npc));
      }
    }
    if (const auto* ground = state->get("groundItems")) apply_ground_items(*ground, false);

    return;
  }
  if (envelope.event == "player:skilltree:update") {
    // TASK-0156: the server's reply to a committed tree snapshot carries the
    // refreshed authoritative passiveTree envelope. TASK-0162: malformed
    // refreshes fail closed with a diagnostic instead of zeroing the pane.
    if (const auto* tree = envelope.data.get("passiveTree"))
      apply_passive_tree(*tree, model_, pending_events_);
    return;
  }
  if (envelope.event == "core:refresh:inventory") {
    const auto* slots = envelope.data.get("data");
    std::vector<std::string> before;
    before.reserve(model_.inventory.size());
    for (const auto& item : model_.inventory) before.push_back(item.uuid);
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
    return;
  }
}

void RemoteProtocolSession::fail(ConnectionState state, const std::string& error) {
  last_error_ = error;
  state_.store(state);
  pending_events_.push_back({PresentationEventType::ConnectionLost, "", "", error, 0});
}

}  // namespace verdigris::client
