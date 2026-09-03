#include "remote_session.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <limits>

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
  std::string name;
  if (dy < 0) name = "up";
  else if (dy > 0) name = "down";
  if (dx < 0) name += name.empty() ? "left" : "-left";
  else if (dx > 0) name += name.empty() ? "right" : "-right";
  return name;
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
  if (const auto* map = entry.get("expeditionMap"); map && map->object()) {
    slot.expedition_map = true;
    slot.map_tier = static_cast<int>(json_number(map->get("tier"), 1.0));
    slot.map_goods_found_percent =
        static_cast<int>(json_number(map->get("goodsFoundPercent"), 0.0));
    if (const auto* family = json_string(map->get("family")))
      slot.map_family = *family;
    if (const auto* key = json_string(map->get("objectiveKey")))
      slot.map_objective_key = *key;
    if (const auto* modifiers = map->get("modifiers");
        modifiers && modifiers->array()) {
      for (const auto& modifier : *modifiers->array())
        if (modifier.string()) slot.map_modifiers.push_back(*modifier.string());
    }
  }
  if (const auto* vessel = entry.get("vessel"); vessel && vessel->object()) {
    if (const auto* lines = vessel->get("lines"); lines && lines->array()) {
      for (const auto& line : *lines->array()) {
        const auto* section = json_string(line.get("section"));
        const auto* copy = json_string(line.get("text"));
        if (!section || !copy || copy->empty()) continue;
        if (*section == "implicit" || *section == "brand" ||
            *section == "scar" || *section == "vessel")
          slot.forge_lines.push_back(*copy);
      }
    }
  }
  return slot;
}

void apply_combat_totals(ClientModel& model, const JsonValue& combat) {
  model.bleed_chance = std::clamp(
      static_cast<int>(json_number(combat.get("bleedChance"), 0.0)), 0, 100);
  model.attack_speed_percent = std::clamp(
      static_cast<int>(json_number(combat.get("attackSpeedPercent"), 0.0)), 0, 100);
  model.reach_percent = std::clamp(
      static_cast<int>(json_number(combat.get("reachPercent"), 0.0)), 0, 100);
  model.projectile_range_percent = std::clamp(
      static_cast<int>(json_number(combat.get("projectileRangePercent"), 0.0)), 0, 100);
  model.armour_penetration_percent = std::clamp(
      static_cast<int>(json_number(combat.get("armourPenetrationPercent"), 0.0)), 0, 100);
  model.movement_speed_percent = std::clamp(
      static_cast<int>(json_number(combat.get("movementSpeedPercent"), 0.0)), 0, 100);
  model.ember_resistance = std::clamp(
      static_cast<int>(json_number(combat.get("emberResistance"), 0.0)), 0, 75);
  model.river_resistance = std::clamp(
      static_cast<int>(json_number(combat.get("riverResistance"), 0.0)), 0, 75);
}

bool quest_integer(const JsonValue* value, int maximum) {
  if (!value || !value->number()) return false;
  const double raw = *value->number();
  return raw >= 0.0 && raw <= maximum && std::floor(raw) == raw;
}

bool valid_endgame_mastery_key(const std::string& key) {
  static constexpr const char* kFamilies[] = {
      "barrow", "reeds", "crown", "thorns"};
  for (const char* family : kFamilies)
    for (int tier = 1; tier <= 16; ++tier)
      if (key == std::string(family) + ":" + std::to_string(tier))
        return true;
  return false;
}

void apply_quests(const JsonValue& source, ClientModel& model,
                  std::vector<PresentationEvent>& events) {
  const char* reason = nullptr;
  const JsonValue* active = nullptr;
  const JsonValue* completed = nullptr;
  const JsonValue* act = nullptr;
  if (!source.object()) {
    reason = "envelope must be an object";
  } else if (!quest_integer(source.get("questPoints"), 23)) {
    reason = "questPoints must be an integer from 0 to 23";
  } else if (!quest_integer(source.get("houseRenown"), 1000000)) {
    reason = "houseRenown must be a nonnegative integer";
  } else if (!source.get("campaignComplete") ||
             !source.get("campaignComplete")->boolean()) {
    reason = "campaignComplete must be a boolean";
  } else {
    const auto* campaign_total = source.get("campaignQuestTotal");
    act = source.get("act");
    if (campaign_total &&
        (!quest_integer(campaign_total, 23) ||
         *campaign_total->number() < 1.0)) {
      reason = "campaignQuestTotal must be an integer from 1 to 23";
    } else if (act &&
               (!act->object() || !quest_integer(act->get("number"), 10) ||
                *act->get("number")->number() < 1.0 ||
                !act->get("title") || !act->get("title")->string() ||
                !quest_integer(act->get("completed"), 23) ||
                !quest_integer(act->get("total"), 23) ||
                *act->get("total")->number() < 1.0 ||
                *act->get("completed")->number() >
                    *act->get("total")->number())) {
      reason = "act has an invalid campaign presentation contract";
    }
    completed = source.get("completed");
    if (!reason && (!completed || !completed->array())) {
      reason = "completed must be an array";
    } else if (!reason) {
      active = source.get("activeQuest");
      if (!active) {
        reason = "activeQuest is required";
      } else if (!active->is_null()) {
        const auto text = [&](const char* key) {
          return active->get(key) && active->get(key)->string();
        };
        const auto* objective = active->get("objective");
        if (!active->object() || !text("id") || !text("title") ||
            !text("giver") || !text("summary") || !text("reward") ||
            !quest_integer(active->get("objectiveIndex"), 100) ||
            !quest_integer(active->get("objectiveCount"), 100) ||
            !objective || !objective->object() ||
            !objective->get("text") || !objective->get("text")->string())
          reason = "activeQuest has an invalid presentation contract";
        else if (*active->get("objectiveCount")->number() < 1.0 ||
                 *active->get("objectiveIndex")->number() >=
                     *active->get("objectiveCount")->number())
          reason = "activeQuest objective cursor is out of range";
      } else if (!*source.get("campaignComplete")->boolean()) {
        reason = "an incomplete campaign requires activeQuest";
      }
      if (!reason) {
        for (const auto& entry : *completed->array()) {
          if (!entry.object() || !entry.get("id") || !entry.get("id")->string() ||
              !entry.get("title") || !entry.get("title")->string() ||
              !entry.get("deed") || !entry.get("deed")->string()) {
            reason = "completed contains an invalid quest record";
            break;
          }
        }
      }
    }
  }
  if (reason) {
    events.push_back({PresentationEventType::ProtocolError, "", "",
                      std::string("quests rejected: ") + reason, 0});
    return;
  }

  ClientQuestState parsed;
  parsed.present = true;
  parsed.quest_points = static_cast<int>(*source.get("questPoints")->number());
  if (const auto* total = source.get("campaignQuestTotal"))
    parsed.campaign_quest_total = static_cast<int>(*total->number());
  parsed.house_renown = static_cast<int>(*source.get("houseRenown")->number());
  parsed.campaign_complete = *source.get("campaignComplete")->boolean();
  if (act) {
    parsed.act_number = static_cast<int>(*act->get("number")->number());
    parsed.act_title = *act->get("title")->string();
    parsed.act_completed = static_cast<int>(*act->get("completed")->number());
    parsed.act_total = static_cast<int>(*act->get("total")->number());
  }
  if (!active->is_null()) {
    parsed.active_id = *active->get("id")->string();
    parsed.title = *active->get("title")->string();
    parsed.giver = *active->get("giver")->string();
    parsed.summary = *active->get("summary")->string();
    parsed.reward = *active->get("reward")->string();
    parsed.objective_index =
        static_cast<int>(*active->get("objectiveIndex")->number());
    parsed.objective_count =
        static_cast<int>(*active->get("objectiveCount")->number());
    parsed.objective = *active->get("objective")->get("text")->string();
  }
  for (const auto& entry : *completed->array()) {
    if (!entry.object()) continue;
    ClientCompletedQuest done;
    if (const auto* id = json_string(entry.get("id"))) done.id = *id;
    if (const auto* title = json_string(entry.get("title"))) done.title = *title;
    if (const auto* deed = json_string(entry.get("deed"))) done.deed = *deed;
    if (!done.id.empty()) parsed.completed.push_back(std::move(done));
  }
  model.quests = std::move(parsed);
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

void apply_player_fields(ClientPlayer& player, const JsonValue& source) {
  if (const auto* uuid = json_string(source.get("uuid"))) player.uuid = *uuid;
  if (const auto* scene = json_string(source.get("sceneId"))) player.scene_id = *scene;
  if (source.get("x") && source.get("x")->number()) player.x = *source.get("x")->number();
  if (source.get("y") && source.get("y")->number()) player.y = *source.get("y")->number();
  if (const auto* facing = json_string(source.get("facing"))) player.facing = *facing;
  if (source.get("level") && source.get("level")->number())
    player.level = static_cast<int>(*source.get("level")->number());
  if (source.get("life") && source.get("life")->number())
    player.life = static_cast<int>(*source.get("life")->number());
  if (source.get("lifeMax") && source.get("lifeMax")->number())
    player.life_max = static_cast<int>(*source.get("lifeMax")->number());
  if (source.get("resource") && source.get("resource")->number())
    player.resource = static_cast<int>(*source.get("resource")->number());
  if (source.get("resourceMax") && source.get("resourceMax")->number())
    player.resource_max = static_cast<int>(*source.get("resourceMax")->number());
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
    if (const auto* complete = entry.get("campaignComplete"))
      parsed_house.campaign_complete =
          complete->boolean() && *complete->boolean();
    parsed_house.endgame_maps_completed = static_cast<int>(
        json_number(entry.get("endgameMapsCompleted"), 0.0));
    if (const auto* masteries = entry.get("endgameMasteries");
        masteries && masteries->array()) {
      std::vector<std::string> unique;
      for (const auto& mastery : *masteries->array()) {
        if (!mastery.string() ||
            !valid_endgame_mastery_key(*mastery.string()) ||
            std::find(unique.begin(), unique.end(), *mastery.string()) !=
                unique.end())
          continue;
        unique.push_back(*mastery.string());
        if (unique.size() == 64) break;
      }
      parsed_house.endgame_masteries = static_cast<int>(unique.size());
    }
    if (const auto* scions = entry.get("scions"); scions && scions->array()) {
      for (const auto& scion_entry : *scions->array()) {
        ClientScionEntry parsed_scion;
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
      // Full eight-way serialization: the server's direction table accepts
      // the compound names ("up-left", ...), so diagonals go on the wire
      // instead of being collapsed to their vertical component.
      const std::string direction = direction_name(command.dx, command.dy);
      if (direction.empty()) return;
      last_facing_ = direction;
      model_.player.facing = direction;
      envelope.event = "player:move";
      envelope.data = JsonValue::Object{{"direction", JsonValue(direction)}};
      break;
    }
    case ClientCommand::Type::Aim: {
      // Aim is presentation-local on this protocol: no envelope, facing
      // updates the model so the next skill trigger carries direction.
      const std::string direction = direction_name(command.dx, command.dy);
      if (direction.empty()) return;
      last_facing_ = direction;
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
      model_.chart.open = false;
      envelope.event = "world:zone:enter";
      envelope.data = JsonValue::Object{{"nodeId", JsonValue(command.target)}};
      break;
    case ClientCommand::Type::Extract:
      // No player:extract handler exists on the native server. The owner
      // extracts by walking onto stairs-up (existing player:move surface).
      pending_events_.push_back({PresentationEventType::Message, "", "",
                                 "Reach the exit stairs to return to the surface.", 0});
      return;
    case ClientCommand::Type::FoundHouse:
      envelope.event = "chronicles:house:found";
      envelope.data = JsonValue::Object{{"name", JsonValue(command.target)}};
      break;
    case ClientCommand::Type::CreateScion: {
      std::string house_id = model_.chronicle.active_house_id;
      if (find_chronicle_house(model_.chronicle, house_id) == nullptr &&
          !model_.chronicle.houses.empty())
        house_id = model_.chronicle.houses.front().id;
      envelope.event = "chronicles:scion:create";
      envelope.data = JsonValue::Object{{"houseId", JsonValue(house_id)},
                                        {"name", JsonValue(command.target)}};
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
      model_.dialogue.open = false;
      model_.forge.open = false;
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

void RemoteProtocolSession::apply_envelope(const Envelope& envelope) {
  if (envelope.event == "player:login") {
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
      if (const auto* quests = player->get("quests"))
        apply_quests(*quests, model_, pending_events_);
    }
    if (const auto* scene = envelope.data.get("scene")) apply_scene_fields(model_.scene, *scene);
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
        model_.chart.open = false;
        model_.dialogue.open = false;
        model_.forge.open = false;
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
        model_.dialogue.open = false;
        model_.forge.open = false;
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
        model_.chart.open = false;
        model_.dialogue.open = false;
        model_.forge.open = false;
      } else if (*screen == "dialogue") {
        ClientDialogueScreen dialogue;
        dialogue.open = true;
        dialogue.npc_id = static_cast<int>(
            json_number(payload->get("npcId"), 0.0));
        if (const auto* key = json_string(payload->get("npcKey")))
          dialogue.npc_key = *key;
        if (const auto* name = json_string(payload->get("name")))
          dialogue.name = *name;
        if (const auto* role = json_string(payload->get("role")))
          dialogue.role = *role;
        if (const auto* body = json_string(payload->get("body")))
          dialogue.body = *body;
        if (const auto* options = payload->get("options");
            options && options->array()) {
          for (const auto& row : *options->array()) {
            ClientDialogueOption option;
            if (const auto* id = json_string(row.get("id"))) option.id = *id;
            if (const auto* label = json_string(row.get("label")))
              option.label = *label;
            if (const auto* hint = json_string(row.get("hint")))
              option.hint = *hint;
            if (const auto* action = json_string(row.get("action")))
              option.action = *action;
            if (const auto enabled = row.get("enabled"); enabled && enabled->boolean())
              option.enabled = *enabled->boolean();
            dialogue.options.push_back(std::move(option));
          }
        }
        model_.dialogue = std::move(dialogue);
        model_.shop.open = false;
        model_.bank.open = false;
        model_.chart.open = false;
        model_.forge.open = false;
      } else if (*screen == "vesselforge") {
        ClientForgeScreen forge;
        forge.open = true;
        forge.npc_id = static_cast<int>(json_number(payload->get("npcId"), 0.0));
        forge.carried_coins =
            static_cast<int>(json_number(payload->get("carriedCoins"), 0.0));
        if (const auto* name = json_string(payload->get("name")))
          forge.name = *name;
        if (const auto* items = payload->get("items"); items && items->array()) {
          for (const auto& item : *items->array()) {
            ClientForgeRow row;
            if (const auto* value = json_string(item.get("uuid"))) row.uuid = *value;
            if (const auto* value = json_string(item.get("name"))) row.name = *value;
            if (const auto* value = json_string(item.get("material"))) row.material = *value;
            if (const auto* value = json_string(item.get("form"))) row.form = *value;
            if (const auto* value = json_string(item.get("reason"))) row.reason = *value;
            row.item_level = static_cast<int>(json_number(item.get("itemLevel"), 0.0));
            row.vessel = static_cast<int>(json_number(item.get("vessel"), 0.0));
            row.used = static_cast<int>(json_number(item.get("used"), 0.0));
            row.free_slots = static_cast<int>(json_number(item.get("freeSlots"), 0.0));
            row.patience = static_cast<int>(json_number(item.get("patience"), 0.0));
            row.patience_max = static_cast<int>(json_number(item.get("patienceMax"), 0.0));
            row.brand_count = static_cast<int>(json_number(item.get("brandCount"), 0.0));
            row.cost = static_cast<int>(json_number(item.get("cost"), 100.0));
            if (const auto* eligible = item.get("eligible");
                eligible && eligible->boolean())
              row.eligible = *eligible->boolean();
            if (const auto* lines = item.get("lines"); lines && lines->array()) {
              for (const auto& line : *lines->array()) {
                ClientForgeLine parsed;
                if (const auto* value = json_string(line.get("section")))
                  parsed.section = *value;
                if (const auto* value = json_string(line.get("text")))
                  parsed.text = *value;
                if (const auto* value = json_string(line.get("tone")))
                  parsed.tone = *value;
                row.lines.push_back(std::move(parsed));
              }
            }
            forge.rows.push_back(std::move(row));
          }
        }
        model_.forge = std::move(forge);
        model_.shop.open = false;
        model_.bank.open = false;
        model_.chart.open = false;
        model_.dialogue.open = false;
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
  if (envelope.event == "player:movement") {
    apply_player_fields(model_.player, envelope.data);
    if (!model_.player.facing.empty()) last_facing_ = model_.player.facing;
    return;
  }
  if (envelope.event == "player:combat-state") {
    model_.player.resource = static_cast<int>(
        json_number(envelope.data.get("resource"), model_.player.resource));
    model_.player.resource_max = static_cast<int>(
        json_number(envelope.data.get("resourceMax"), model_.player.resource_max));
    model_.player.cooldown_ticks = static_cast<int>(
        json_number(envelope.data.get("cooldownTicks"), model_.player.cooldown_ticks));
    model_.player.combo_step = std::clamp(
        static_cast<int>(json_number(envelope.data.get("comboStep"),
                                     model_.player.combo_step)), 0, 3);
    model_.player.combo_window_ticks = (std::max)(0, static_cast<int>(
        json_number(envelope.data.get("comboWindowTicks"),
                    model_.player.combo_window_ticks)));
    model_.player.war_cry_ticks_remaining = static_cast<int>(json_number(
        envelope.data.get("warCryTicksRemaining"),
        model_.player.war_cry_ticks_remaining));
    return;
  }
  if (envelope.event == "player:skill:effect") {
    const auto* skill = json_string(envelope.data.get("skillId"));
    const bool active = envelope.data.get("active") &&
                        envelope.data.get("active")->boolean() &&
                        *envelope.data.get("active")->boolean();
    if (skill && *skill == "war-cry") {
      model_.player.war_cry_ticks_remaining = active
          ? (std::max)(1, static_cast<int>(json_number(
                envelope.data.get("durationMs"), 50.0) / 50.0))
          : 0;
      pending_events_.push_back({active ? PresentationEventType::BuffApplied
                                        : PresentationEventType::BuffExpired,
                                 model_.player.uuid, "", "war-cry", 0});
    }
    return;
  }
  if (envelope.event == "world:scene:transition" ||
      envelope.event == "party:scene:transition") {
    if (const auto* scene = envelope.data.get("scene")) apply_scene_fields(model_.scene, *scene);
    if (const auto* player_state = envelope.data.get("playerState")) {
      apply_player_fields(model_.player, *player_state);
    }
    if (!model_.scene.id.empty()) model_.player.scene_id = model_.scene.id;
    model_.player.combo_step = 0;
    model_.player.combo_window_ticks = 0;
    model_.monsters.clear();
    model_.npcs.clear();
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
    PresentationEvent warning;
    warning.type = PresentationEventType::Telegraph;
    warning.actor_id = attacker ? *attacker : "";
    warning.text = std::string(name ? *name : "") + " " + skill_id;
    warning.value = static_cast<int>(json_number(envelope.data.get("durationMs")));
    if (envelope.data.get("x") && envelope.data.get("x")->number() &&
        envelope.data.get("y") && envelope.data.get("y")->number()) {
      warning.has_position = true;
      warning.x = *envelope.data.get("x")->number();
      warning.y = *envelope.data.get("y")->number();
    }
    warning.radius = (std::max)(1, static_cast<int>(
        json_number(envelope.data.get("radius"), 1.0)));
    pending_events_.push_back(std::move(warning));
    return;
  }
  if (envelope.event == "monster:moved") {
    const auto* id = json_string(envelope.data.get("monsterId"));
    const auto* name = json_string(envelope.data.get("monsterName"));
    ClientMonster& foe = upsert_monster(model_, id ? *id : "",
                                        name ? *name : "", false);
    foe.x = json_number(envelope.data.get("x"), foe.x);
    foe.y = json_number(envelope.data.get("y"), foe.y);
    foe.move_duration_ms = std::clamp(
        static_cast<int>(json_number(envelope.data.get("durationMs"), 400.0)),
        50, 1000);
    if (const auto* behaviour = json_string(envelope.data.get("behaviour")))
      foe.behaviour = *behaviour;
    return;
  }
  if (envelope.event == "monster:interrupted") {
    PresentationEvent interrupted;
    interrupted.type = PresentationEventType::TelegraphCancelled;
    if (const auto* id = json_string(envelope.data.get("monsterId")))
      interrupted.actor_id = *id;
    if (const auto* skill = json_string(envelope.data.get("skillId")))
      interrupted.text = *skill;
    interrupted.value = (std::max)(0, static_cast<int>(
        json_number(envelope.data.get("staggerMs"), 0.0)));
    pending_events_.push_back(std::move(interrupted));
    return;
  }
  if (envelope.event == "monster:healed") {
    const auto* target = json_string(envelope.data.get("targetId"));
    const auto* target_name = json_string(envelope.data.get("targetName"));
    const int amount = (std::max)(0, static_cast<int>(
        json_number(envelope.data.get("amount"), 0.0)));
    ClientMonster& foe = upsert_monster(model_, target ? *target : "",
                                        target_name ? *target_name : "", false);
    if (const auto* health = envelope.data.get("health")) {
      foe.life = static_cast<int>(json_number(health->get("current"), foe.life));
      foe.life_max = static_cast<int>(json_number(health->get("max"), foe.life_max));
    } else {
      foe.life = (std::min)(foe.life_max, foe.life + amount);
    }
    PresentationEvent mend;
    mend.type = PresentationEventType::HealingApplied;
    mend.actor_id = foe.id;
    mend.text = json_string(envelope.data.get("sourceName"))
        ? *json_string(envelope.data.get("sourceName")) : "support";
    mend.value = amount;
    pending_events_.push_back(std::move(mend));
    return;
  }
  if (envelope.event == "monster:status") {
    const auto* target = json_string(envelope.data.get("targetId"));
    const auto* target_name = json_string(envelope.data.get("targetName"));
    const auto* status = json_string(envelope.data.get("statusId"));
    const bool active = json_bool(envelope.data.get("active"));
    ClientMonster& foe = upsert_monster(model_, target ? *target : "",
                                        target_name ? *target_name : "", false);
    if (status && *status == "bleed") {
      foe.bleeding = active;
      if (active) {
        PresentationEvent applied;
        applied.type = PresentationEventType::DebuffApplied;
        applied.actor_id = foe.id;
        applied.text = "bleed";
        applied.value = (std::max)(0, static_cast<int>(
            json_number(envelope.data.get("damagePerTick"), 0.0)));
        applied.duration_ms = (std::max)(0, static_cast<int>(
            json_number(envelope.data.get("durationMs"), 0.0)));
        pending_events_.push_back(std::move(applied));
      }
    }
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
      // A hit does not imply elite rarity. Snapshot/telegraph facts already
      // carry that distinction; ranged pressure must not promote common foes.
      if (attacker) upsert_monster(model_, *attacker, "", false);
      model_.last_incoming_hit = amount;
      PresentationEvent incoming;
      incoming.type = PresentationEventType::DamageApplied;
      incoming.actor_id = attacker ? *attacker : "";
      incoming.text = "incoming";
      incoming.value = amount;
      if (const auto* channel = json_string(envelope.data.get("damageChannel")))
        incoming.damage_channel = *channel;
      incoming.style = incoming.damage_channel;
      incoming.base_amount = static_cast<int>(
          json_number(envelope.data.get("baseAmount"), amount));
      incoming.resistance_percent = std::clamp(
          static_cast<int>(json_number(envelope.data.get("resistancePercent"), 0.0)),
          0, 75);
      pending_events_.push_back(std::move(incoming));
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
      std::string skill_id = "melee";
      if (const auto* skill_value = json_string(envelope.data.get("skillId")))
        skill_id = *skill_value;
      const int combo_step = std::clamp(
          static_cast<int>(json_number(envelope.data.get("comboStep"), 0.0)), 0, 3);
      const int combo_window_ms = (std::max)(0, static_cast<int>(
          json_number(envelope.data.get("comboWindowMs"), 0.0)));
      const int stagger_ms = (std::max)(0, static_cast<int>(
          json_number(envelope.data.get("staggerMs"), 0.0)));
      if (combo_step > 0) {
        model_.player.combo_step = combo_step;
        model_.player.combo_window_ticks = (combo_window_ms + 49) / 50;
      }
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
      if (skill_id != "status:bleed")
        pending_events_.push_back({PresentationEventType::AttackStarted,
                                   attacker ? *attacker : model_.player.uuid, "",
                                   skill_id, amount});
      PresentationEvent outgoing;
      outgoing.type = PresentationEventType::DamageApplied;
      outgoing.actor_id = target ? *target : "";
      outgoing.text = "outgoing";
      outgoing.value = amount;
      outgoing.critical = critical;
      outgoing.style = style;
      outgoing.combo_step = combo_step;
      outgoing.combo_window_ms = combo_window_ms;
      outgoing.stagger_ms = stagger_ms;
      if (const auto* channel = json_string(envelope.data.get("damageChannel")))
        outgoing.damage_channel = *channel;
      outgoing.base_amount = static_cast<int>(
          json_number(envelope.data.get("baseAmount"), amount));
      outgoing.resistance_percent = std::clamp(
          static_cast<int>(json_number(envelope.data.get("resistancePercent"), 0.0)),
          0, 75);
      outgoing.armour_rating = (std::max)(0, static_cast<int>(
          json_number(envelope.data.get("armourRating"), 0.0)));
      outgoing.armour_prevented = (std::max)(0, static_cast<int>(
          json_number(envelope.data.get("armourPrevented"), 0.0)));
      outgoing.armour_penetration_percent = std::clamp(
          static_cast<int>(json_number(
              envelope.data.get("armourPenetrationPercent"), 0.0)), 0, 100);
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
  if (envelope.event == "dev:state") {
    const auto* state = envelope.data.get("state");
    if (!state) return;
    // Authoritative lifecycle + oath visibility (snapshot puts these at the
    // top of dev:state). Keeps death/successor states honest between
    // chronicle payloads.
    if (const auto* lifecycle = json_string(state->get("lifecycle")))
      model_.lifecycle = *lifecycle;
    model_.player.level = static_cast<int>(
        json_number(state->get("level"), model_.player.level));
    // TASK-0156: the dev:state snapshot carries the same authoritative
    // passiveTree envelope; keep the mirror current between logins. TASK-0162:
    // a malformed snapshot fails closed and surfaces its diagnostic.
    if (const auto* tree = state->get("passiveTree"))
      apply_passive_tree(*tree, model_, pending_events_);
    if (const auto* quests = state->get("quests"))
      apply_quests(*quests, model_, pending_events_);
    if (const auto* hp = state->get("hp")) {
      // Authoritative life keeps alive honest between combat envelopes.
      model_.player.life = static_cast<int>(json_number(hp->get("current"), model_.player.life));
      model_.player.life_max =
          static_cast<int>(json_number(hp->get("max"), model_.player.life_max));
      model_.player.alive = model_.player.life > 0;
    }
    if (const auto* resource = state->get("resource")) {
      model_.player.resource = static_cast<int>(
          json_number(resource->get("current"), model_.player.resource));
      model_.player.resource_max = static_cast<int>(
          json_number(resource->get("max"), model_.player.resource_max));
    }
    if (const auto* combat = state->get("combat"))
      apply_combat_totals(model_, *combat);
    model_.player.cooldown_ticks = static_cast<int>(json_number(
        state->get("cooldownTicks"), model_.player.cooldown_ticks));
    model_.player.war_cry_ticks_remaining = static_cast<int>(json_number(
        state->get("warCryTicksRemaining"),
        model_.player.war_cry_ticks_remaining));
    if (const auto* cadence = state->get("combatCadence");
        cadence && cadence->object()) {
      model_.player.combo_step = std::clamp(
          static_cast<int>(json_number(cadence->get("step"), 0.0)), 0, 3);
      model_.player.combo_window_ticks = (std::max)(0, static_cast<int>(
          json_number(cadence->get("windowTicks"), 0.0)));
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
    if (const auto* xp = state->get("xp"); xp && xp->object()) {
      const double current =
          json_number(xp->get("current"), std::numeric_limits<double>::quiet_NaN());
      const double floor =
          json_number(xp->get("floor"), std::numeric_limits<double>::quiet_NaN());
      const double next =
          json_number(xp->get("next"), std::numeric_limits<double>::quiet_NaN());
      if (std::isfinite(current) && std::isfinite(floor) &&
          std::isfinite(next) && current >= 0.0 && floor >= 0.0 &&
          next > floor) {
        model_.xp_present = true;
        model_.xp_current = current;
        model_.xp_floor = floor;
        model_.xp_next = next;
      }
    }
    if (const auto* endgame = state->get("endgame");
        endgame && endgame->object()) {
      ClientEndgameState parsed;
      parsed.present = true;
      if (const auto* unlocked = endgame->get("unlocked"))
        parsed.unlocked = unlocked->boolean() && *unlocked->boolean();
      if (const auto* active = endgame->get("active"))
        parsed.active = active->boolean() && *active->boolean();
      if (const auto* cleared = endgame->get("cleared"))
        parsed.cleared = cleared->boolean() && *cleared->boolean();
      parsed.completed =
          static_cast<int>(json_number(endgame->get("completed"), 0.0));
      parsed.mastered = std::clamp(
          static_cast<int>(json_number(endgame->get("mastered"), 0.0)), 0, 64);
      parsed.mastery_total = std::clamp(
          static_cast<int>(json_number(endgame->get("masteryTotal"), 64.0)),
          1, 64);
      parsed.highest_tier = std::clamp(
          static_cast<int>(json_number(endgame->get("highestTier"), 0.0)),
          0, 16);
      parsed.ascent_chance_percent = std::clamp(
          static_cast<int>(
              json_number(endgame->get("ascentChancePercent"), 35.0)),
          0, 100);
      parsed.tier = static_cast<int>(json_number(endgame->get("tier"), 0.0));
      parsed.goods_found_percent = static_cast<int>(
          json_number(endgame->get("goodsFoundPercent"), 0.0));
      if (const auto* name = json_string(endgame->get("name")))
        parsed.name = *name;
      if (const auto* family = json_string(endgame->get("family")))
        parsed.family = *family;
      if (const auto* key = json_string(endgame->get("objectiveKey")))
        parsed.objective_key = *key;
      if (const auto* keys = endgame->get("masteryKeys");
          keys && keys->array()) {
        for (const auto& key : *keys->array()) {
          if (parsed.mastery_keys.size() >= 64) break;
          if (key.string() && valid_endgame_mastery_key(*key.string()) &&
              std::find(parsed.mastery_keys.begin(),
                        parsed.mastery_keys.end(), *key.string()) ==
                  parsed.mastery_keys.end())
            parsed.mastery_keys.push_back(*key.string());
        }
      }
      if (const auto* first_clear = endgame->get("firstClear"))
        parsed.first_clear =
            first_clear->boolean() && *first_clear->boolean();
      if (const auto* modifiers = endgame->get("modifiers");
          modifiers && modifiers->array()) {
        for (const auto& modifier : *modifiers->array())
          if (modifier.string()) parsed.modifiers.push_back(*modifier.string());
      }
      model_.endgame = std::move(parsed);
    }
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
        if (const auto* channel = json_string(entry.get("damageChannel")))
          monster.damage_channel = *channel;
        monster.x = json_number(entry.get("x"), 0.0);
        monster.y = json_number(entry.get("y"), 0.0);
        monster.armour = (std::max)(0, static_cast<int>(
            json_number(entry.get("armour"), 0.0)));
        if (const auto* hp = entry.get("hp")) {
          monster.life = static_cast<int>(json_number(hp->get("current"), monster.life));
          monster.life_max = static_cast<int>(json_number(hp->get("max"), monster.life_max));
        }
        if (const auto* rarity = json_string(entry.get("rarity"))) {
          monster.elite = (*rarity != "normal" && !rarity->empty());
        }
        monster.alive = monster.life > 0;
        if (const auto* monster_state = entry.get("state")) {
          if (const auto* effects = monster_state->get("effects"))
            monster.bleeding = effects->get("bleed") &&
                               effects->get("bleed")->object();
        }
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
    if (const auto* investment = state->get("houseInvestment");
        investment && investment->object()) {
      model_.house_investment.first_clear_completed =
          json_bool(investment->get("firstClearCompleted"), false);
      model_.house_investment.eligible =
          json_bool(investment->get("eligible"), false);
      if (const auto* choice = json_string(investment->get("choice")))
        model_.house_investment.choice = *choice;
      model_.house_investment.reward_claimed =
          json_bool(investment->get("rewardClaimed"), false);
      model_.house_investment.scion_gear_tier = static_cast<int>(
          json_number(investment->get("scionGearTier"), 0.0));
      model_.house_investment.house_income_per_clear = static_cast<int>(
          json_number(investment->get("houseIncomePerClear"), 0.0));
    }
    if (const auto* npcs = state->get("npcs"); npcs && npcs->array()) {
      model_.npcs.clear();
      for (const auto& entry : *npcs->array()) {
        ClientNpc npc;
        npc.id = static_cast<int>(json_number(entry.get("id"), 0.0));
        if (const auto* key = json_string(entry.get("key"))) npc.key = *key;
        if (const auto* name = json_string(entry.get("name"))) npc.name = *name;
        if (const auto* role = json_string(entry.get("role"))) npc.role = *role;
        if (const auto* examine = json_string(entry.get("examine")))
          npc.examine = *examine;
        npc.x = json_number(entry.get("x"), 0.0);
        npc.y = json_number(entry.get("y"), 0.0);
        if (const auto* services = entry.get("services");
            services && services->array()) {
          for (const auto& service : *services->array())
            if (service.string()) npc.services.push_back(*service.string());
        }
        if (const auto* actions = entry.get("actions"); actions && actions->array()) {
          for (const auto& action : *actions->array())
            if (action.string()) npc.actions.push_back(*action.string());
        }
        model_.npcs.push_back(std::move(npc));
      }
    }
    if (const auto* ground = state->get("groundItems"); ground && ground->array()) {
      model_.ground.clear();
      for (const auto& entry : *ground->array()) {
        ClientGroundItem item;
        if (const auto* uuid = json_string(entry.get("uuid"))) item.uuid = *uuid;
        if (const auto* name = json_string(entry.get("name"))) item.name = *name;
        item.x = json_number(entry.get("x"), 0.0);
        item.y = json_number(entry.get("y"), 0.0);
        if (const auto* relic = entry.get("chroniclesRelic"); relic && relic->object()) {
          item.relic = true;
          if (const auto* scion_name = json_string(relic->get("scionName")))
            item.relic_of = *scion_name;
        }
        model_.ground.push_back(std::move(item));
      }
    }
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
  if (envelope.event == "quest:update") {
    if (const auto* quests = envelope.data.get("quests"))
      apply_quests(*quests, model_, pending_events_);
    if (const auto* tree = envelope.data.get("passiveTree"))
      apply_passive_tree(*tree, model_, pending_events_);
    return;
  }
  if (envelope.event == "player:equippedAnItem") {
    if (const auto* combat = envelope.data.get("combat"))
      apply_combat_totals(model_, *combat);
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
