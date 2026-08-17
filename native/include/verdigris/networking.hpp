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

 private:
  std::string player_payload() const;
  JsonValue snapshot() const;
  void emit_login(const std::function<void(const Envelope&)>& emit) const;
  void emit_transition(const std::function<void(const Envelope&)>& emit) const;
  void grant_item(const std::string& item_id, int quantity);

  std::string identity_;
  std::string socket_id_;
  bool quick_start_ = false;
  std::string scene_type_ = "town";
  std::string scene_id_ = "town:verdigris";
  std::vector<Item> inventory_;
  std::unique_ptr<Simulation> simulation_;
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

  std::uint16_t port_;
  std::intptr_t listen_socket_ = -1;
  bool running_ = false;
  std::mutex mutex_;
  std::vector<std::shared_ptr<Connection>> connections_;
  std::unordered_map<std::string, std::shared_ptr<ProtocolSession>> sessions_;
  std::unique_ptr<std::thread> accept_thread_;
};

}  // namespace verdigris::networking
