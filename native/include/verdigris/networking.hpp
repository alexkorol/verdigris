#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <map>
#include <unordered_map>
#include <variant>
#include <vector>

#include "verdigris/core.hpp"

namespace verdigris::networking {

// Small JSON value used at the protocol boundary.  The simulation never sees
// this type: transport parses an envelope, the session translates commands,
// and snapshots are serialized back out at the edge.
class JsonValue {
 public:
  using Object = std::map<std::string, JsonValue>;
  using Array = std::vector<JsonValue>;
  using Storage = std::variant<std::nullptr_t, bool, double, std::string, Array, Object>;

  JsonValue();
  JsonValue(std::nullptr_t);
  JsonValue(bool value);
  JsonValue(double value);
  JsonValue(int value);
  JsonValue(std::string value);
  JsonValue(const char* value);
  JsonValue(Array value);
  JsonValue(Object value);

  bool is_null() const;
  bool is_object() const;
  bool is_array() const;
  bool is_string() const;
  bool is_number() const;
  bool is_bool() const;
  const Object* object() const;
  Object* object();
  const Array* array() const;
  Array* array();
  const std::string* string() const;
  std::optional<double> number() const;
  std::optional<bool> boolean() const;
  const JsonValue* get(const std::string& key) const;
  JsonValue* get(const std::string& key);
  const JsonValue& operator[](const std::string& key) const;

  std::string stringify() const;

 private:
  Storage value_;
};

struct Envelope {
  std::string event;
  JsonValue data;
  std::optional<JsonValue> meta;
};

bool parse_json(const std::string& text, JsonValue& out, std::string* error = nullptr);
bool parse_envelope(const std::string& text, Envelope& out, std::string* error = nullptr);
std::string emit_envelope(const Envelope& envelope);

class ProtocolSession {
 public:
  ProtocolSession(std::string identity, std::string socket_id, std::uint64_t seed,
                  bool quick_start);

  const std::string& identity() const { return identity_; }
  const std::string& socket_id() const { return socket_id_; }
  std::string login_payload() const;
  std::string state_payload(const std::string& request_id) const;
  void handle(const Envelope& envelope, const std::function<void(const Envelope&)>& emit);
  void replace_socket(std::string socket_id);
  void reset_world_for_new_socket();
  // World events (movement, scene transitions) are broadcast to every live
  // connection, mirroring the JS server's room broadcast.  Unit tests leave
  // this unset and receive the same envelopes through the requester's emit.
  void set_broadcast(std::function<void(const Envelope&)> broadcast);

 private:
  std::string player_payload() const;
  JsonValue snapshot() const;
  JsonValue scene_payload() const;
  JsonValue movement_step_payload() const;
  void emit_login(const std::function<void(const Envelope&)>& emit) const;
  void emit_transition(const std::function<void(const Envelope&)>& emit, const char* event) const;
  void emit_movement(const std::function<void(const Envelope&)>& emit) const;
  void emit_message(const std::function<void(const Envelope&)>& emit, const std::string& text) const;
  void emit_world(const Envelope& envelope, const std::function<void(const Envelope&)>& emit) const;
  void process_combat(std::int64_t now_ms, const std::function<void(const Envelope&)>& emit);
  void emit_combat_event(const WorldCombatEvent& event,
                         const std::function<void(const Envelope&)>& emit);
  // N4 item pipeline (dev.js / inventory.js / wear-slots.js / registry.js).
  void emit_inventory_refresh(const std::function<void(const Envelope&)>& emit) const;
  void emit_ground_change(const std::function<void(const Envelope&)>& emit) const;
  void emit_equip_state(const std::function<void(const Envelope&)>& emit) const;
  JsonValue wear_json() const;
  JsonValue wear_details_json() const;
  JsonValue combat_totals_json() const;
  JsonValue dropped_items_json() const;
  void sync_combat_mods();
  void finish_extraction(const std::function<void(const Envelope&)>& emit);
  void handle_give(const JsonValue& payload, const std::function<void(const Envelope&)>& emit);
  void handle_drop(const JsonValue& payload, const std::function<void(const Envelope&)>& emit);
  void handle_equip(const JsonValue& payload, const std::function<void(const Envelope&)>& emit);
  void handle_extract(const std::function<void(const Envelope&)>& emit);
  void mark_relic_recovered(const std::string& scion_id);
  void handle_take_ground(const std::string& uuid, const std::function<void(const Envelope&)>& emit);
  void handle_take_underfoot(const std::function<void(const Envelope&)>& emit);
  void handle_menu_build(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) const;
  void handle_menu_action(const JsonValue& payload, const std::function<void(const Envelope&)>& emit);
  void handle_inventory_commit(const JsonValue& payload, const std::function<void(const Envelope&)>& emit);
  static std::int64_t now_ms();

  std::string identity_;
  std::string socket_id_;
  bool quick_start_ = false;
  // N5: mortal-oath lifecycle + soft respawn ward (server/core/lifecycle.js).
  std::string lifecycle_ = "alive";        // alive | awaiting-respawn | dead | permadead
  int lifecycle_deaths_ = 0;
  std::int64_t respawn_at_ms_ = 0;
  std::int64_t respawn_protection_until_ms_ = 0;
  void maybe_respawn(std::int64_t now_ms);
  void handle_final_death(const std::function<void(const Envelope&)>& emit);
  // N5: Chronicles auth (server/core/services/chronicles.js + chronicles store).
  JsonValue chronicle_;               // { version, houses:[...], activeHouseId, activeScionId }
  int chronicles_revision_ = 0;
  bool pending_chronicles_ = false;   // login admitted to the pending state
  std::string lifecycle_mode_ = "soft";
  bool mortal_oath_ = false;
  std::string active_scion_id_;
  std::string active_house_id_;
  std::string active_scion_name_;
  bool prepare_final_death_ = false;
  int best_depth_ = 0;
  int pending_relic_count_ = 0;
  std::vector<GameItem> pending_relic_items_;
  std::string relic_source_scion_name_;
  std::string relic_source_scion_id_;
  JsonValue chronicles_payload() const;
  JsonValue chronicles_state_payload(const std::string& created_scion_id) const;
  void ensure_chronicle_house(const std::string& id, const std::string& name);
  void ensure_chronicle_scion(const std::string& house_id, const std::string& id,
                              const std::string& name, bool mortal);
  // N4: the real item pipeline state (12x7 backpack + wear seats); the forge
  // itself lives on the world (JS module singleton).
  PlayerInventory inventory_;
  WearSet wear_;
  // Protocol House bank (JS has no player:extract; core Simulation::house is
  // const from this layer). Extraction and stairs-up both drain here.
  std::vector<GameItem> house_store_;
  Mulberry32 session_rng_;
  std::unique_ptr<Simulation> simulation_;
  std::unique_ptr<WorldSimulation> world_;
  std::function<void(const Envelope&)> broadcast_;
  mutable std::mutex mutex_;
};

class WebSocketServer {
 public:
  explicit WebSocketServer(std::uint16_t port = 6500);
  ~WebSocketServer();

  WebSocketServer(const WebSocketServer&) = delete;
  WebSocketServer& operator=(const WebSocketServer&) = delete;

  bool start(std::string* error = nullptr);
  void stop();
  std::uint16_t port() const { return port_; }

 private:
  struct Connection;
  void accept_loop();
  void handle_connection(std::shared_ptr<Connection> connection);
  void handle_message(const std::shared_ptr<Connection>& connection, const std::string& text);
  void remove_connection(const std::shared_ptr<Connection>& connection);
  void broadcast(const Envelope& envelope);

  std::uint16_t port_;
  std::intptr_t listen_socket_ = -1;
  bool running_ = false;
  std::mutex mutex_;
  std::vector<std::shared_ptr<Connection>> connections_;
  std::unordered_map<std::string, std::shared_ptr<ProtocolSession>> sessions_;
  std::unique_ptr<std::thread> accept_thread_;
};

}  // namespace verdigris::networking
