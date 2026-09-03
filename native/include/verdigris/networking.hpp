#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
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
  const std::string& username() const { return username_; }
  void set_username(const std::string& name) { username_ = name; }
  std::string display_name() const {
    return !username_.empty() ? username_ : (!active_scion_name_.empty() ? active_scion_name_ : identity_);
  }
  bool matches_name(const std::string& name) const {
    return name == username_ || name == active_scion_name_ || name == identity_;
  }
  const std::string& socket_id() const { return socket_id_; }
  std::string login_payload() const;
  std::string state_payload(const std::string& request_id,
                            bool include_map = false) const;
  void handle(const Envelope& envelope, const std::function<void(const Envelope&)>& emit);
  void replace_socket(std::string socket_id);
  void reset_world_for_new_socket();
  // World events (movement, scene transitions) are broadcast to every live
  // connection, mirroring the JS server's room broadcast.  Unit tests leave
  // this unset and receive the same envelopes through the requester's emit.
  void set_broadcast(std::function<void(const Envelope&)> broadcast);
  // JS parity: the server runs its own game loop; combat and respawns
  // advance on the tick, not only on inbound envelopes.
  void set_direct_emit(std::function<void(const Envelope&)> emit);
  void tick(std::int64_t now_ms);
  void enter_shared_instance(const std::string& scene_id, const std::function<void(const Envelope&)>& emit);
  void leave_to_town(const std::function<void(const Envelope&)>& emit);
  std::shared_ptr<WorldSimulation> shared_world() { return world_; }
  void adopt_world(std::shared_ptr<WorldSimulation> world, const std::string& scene_id, const std::function<void(const Envelope&)>& emit);

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
  void handle_npc_talk(const JsonValue& payload, const std::function<void(const Envelope&)>& emit);
  void emit_quest_update(const std::function<void(const Envelope&)>& emit) const;
  void maybe_complete_first_goal(const std::function<void(const Envelope&)>& emit);
  JsonValue quests_json() const;
  JsonValue passive_tree_json() const;
  void tree_attributes(int* strength, int* dexterity, int* intelligence) const;
  void handle_skilltree_save(const JsonValue& payload, const std::function<void(const Envelope&)>& emit);
  void emit_bank_screen(const std::function<void(const Envelope&)>& emit) const;
  void handle_house_deposit(const JsonValue& payload, const std::function<void(const Envelope&)>& emit);
  int carried_gold() const;
  void maybe_floor_cleared(const std::function<void(const Envelope&)>& emit);
  void auto_pickup_gold(const std::function<void(const Envelope&)>& emit);
  void emit_wagon_screen(const std::function<void(const Envelope&)>& emit) const;
  void emit_shop_screen(const std::function<void(const Envelope&)>& emit) const;
  JsonValue bank_items_json() const;
  void emit_chart_screen(const std::string& road_id, const std::function<void(const Envelope&)>& emit) const;
  void enter_road_node(const std::string& node_id, const std::function<void(const Envelope&)>& emit);
  void open_expedition_map(const std::string& uuid,
                           const std::function<void(const Envelope&)>& emit);
  void award_expedition_map(int tier, double x, double y,
                            const std::function<void(const Envelope&)>& emit,
                            bool to_backpack);

  void check_road_gates(const std::function<void(const Envelope&)>& emit);
  void quest_trigger(const char* trigger, const std::function<void(const Envelope&)>& emit,
                     const std::string& detail_a = std::string(), const std::string& detail_b = std::string(), int depth = 0);
  void handle_take_ground(const std::string& uuid, const std::function<void(const Envelope&)>& emit);
  void handle_take_underfoot(const std::function<void(const Envelope&)>& emit);
  void handle_menu_build(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) const;
  void handle_menu_action(const JsonValue& payload, const std::function<void(const Envelope&)>& emit);
  void handle_inventory_commit(const JsonValue& payload, const std::function<void(const Envelope&)>& emit);
  static std::int64_t now_ms();

  std::string identity_;
  std::string socket_id_;
  std::string username_;
  bool quick_start_ = false;
  // N5: mortal-oath lifecycle + soft respawn ward (server/core/lifecycle.js).
  std::string lifecycle_ = "alive";        // alive | awaiting-respawn | dead | permadead
  int lifecycle_deaths_ = 0;
  std::int64_t respawn_at_ms_ = 0;
  std::int64_t respawn_protection_until_ms_ = 0;
  // N6 first-goal quest machine (server/core/first-goal.js).
  std::string first_goal_stage_ = "available";
  std::int64_t first_goal_started_ms_ = 0;
  std::int64_t first_goal_completed_ms_ = 0;
  int quest_points_ = 0;
  // JS Player.questPoints: the LIVE session tree budget. Unlike the chain
  // record above it does not survive a plain re-login (skilltree relog).
  int tree_quest_points_ = 0;
  // N6 passive tree (verdigris-authority.js) - stored allocation + budget.
  JsonValue passive_tree_;
  bool passive_tree_saved_ = false;
  // N6 house treasury + floor-clear tracking.
  int house_treasury_ = 0;
  std::uint64_t last_cleared_floor_key_ = 0;
  bool daily_purse_claimed_ = false;
  int home_pitch_index_ = 0;
  // N6 ordered quest chain (server/shared/quests.js).
  int active_quest_ = 0;      // index into kQuestChain; >= chain size = done
  int quest_objective_ = 0;   // objectiveIndex within the active quest
  std::vector<std::string> quests_completed_;
  bool campaign_complete_ = false;  // House-wide; inherited by successor Scions
  int house_renown_ = 0;
  std::string last_instance_theme_;
  std::string last_instance_layout_;
  // N6 economy: personal bank + shop session state.
  std::vector<GameItem> bank_;
  bool shop_open_ = false;
  bool bank_open_ = false;
  // N6 world-web (server/core/world-web.js): per-house deterministic road
  // chart; cleared wardens persist for the session (dead stays dead).
  std::set<std::string> cleared_nodes_;
  std::string current_node_id_;
  int current_node_tier_ = 0;
  std::string current_node_name_;
  std::string current_child_id_;
  std::string current_child_name_;
  bool node_warden_dead_on_entry_ = false;
  // Consumable charted-tablet endgame. Unlock is the completed campaign
  // commission chain; the active roll is copied before its item is consumed.
  bool endgame_active_ = false;
  bool endgame_completed_ = false;
  int endgame_maps_completed_ = 0;
  int endgame_map_tier_ = 0;
  int endgame_goods_found_percent_ = 0;
  std::string endgame_map_name_;
  std::vector<std::string> endgame_map_modifiers_;
  std::set<std::string> kitted_scions_;
  std::string active_skill_id_ = "primary-attack";
  // N6 combat experience (experience.js / shared/ui.js curve).
  long long combat_xp_ = 0;
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
  std::string active_house_name_;
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
  // shared_ptr: party instances share ONE authoritative world between
  // member sessions (build-divergence: both builds hit the same monster).
  std::shared_ptr<WorldSimulation> world_;
  std::function<void(const Envelope&)> broadcast_;
  std::function<void(const Envelope&)> direct_emit_;
  mutable std::recursive_mutex mutex_;
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
  std::unique_ptr<std::thread> tick_thread_;
  // Per-connection reader threads hold a raw `this`; they must be JOINED in
  // stop() - a detached thread that wakes after `delete server` dereferences
  // a freed WebSocketServer (session_tests reconnect segfault under load).
  std::vector<std::thread> connection_threads_;
  // party.js registry: parties are server state shared across sessions.
  struct ServerParty {
    std::string id;
    std::string leader_uuid;
    std::vector<std::string> member_uuids;
    std::map<std::string, bool> ready;
    std::string state = "lobby";
  };
  std::map<std::string, ServerParty> parties_;
  std::map<std::string, std::string> party_by_uuid_;
  bool handle_party_event(const std::shared_ptr<Connection>& connection, const Envelope& envelope);
  void send_party_update(const ServerParty& party);
  void send_to_identity(const std::string& identity, const Envelope& envelope);
  std::shared_ptr<ProtocolSession> session_by_username(const std::string& username);
};

}  // namespace verdigris::networking
