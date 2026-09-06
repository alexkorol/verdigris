#pragma once

// RemoteProtocolSession (D-122): the ONLY client code allowed to own a
// socket. Speaks the existing `{event, data}` envelope to verdigris_server
// over RFC6455. Runs NO authoritative simulation; a failed connection is a
// visible failure, never a silent fallback to local play.

#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "session.hpp"
#include "verdigris/networking.hpp"

namespace verdigris::client {

class RemoteProtocolSession final : public IClientSession {
 public:
  RemoteProtocolSession(std::string host, std::uint16_t port,
                        std::string guest_id, bool quick_guest = true);
  ~RemoteProtocolSession() override;

  RemoteProtocolSession(const RemoteProtocolSession&) = delete;
  RemoteProtocolSession& operator=(const RemoteProtocolSession&) = delete;

  bool start(std::string* error = nullptr) override;
  void shutdown() override;
  void submit(const ClientCommand& command) override;
  void poll() override;
  ConnectionState connection_state() const override { return state_.load(); }
  const ClientModel& model() const override { return model_; }
  std::vector<PresentationEvent> drain_events() override;
  // Test-harness escape hatch: send a raw protocol envelope (dev:* control
  // surface). Production presentation code never calls this.
  bool send_raw(const std::string& event, verdigris::networking::JsonValue data);
  const std::string& last_error() const override { return last_error_; }

 private:
  bool send_envelope(const verdigris::networking::Envelope& envelope);
  bool send_frame(std::uint8_t opcode, const std::string& payload);
  bool connect_transport(std::string* error);
  void close_transport();
  void begin_retry(const std::string& reason);
  void pump_retry();
  void reader_loop();
  void apply_envelope(const verdigris::networking::Envelope& envelope);
  void fail(ConnectionState state, const std::string& error);

  std::string host_;
  std::uint16_t port_;
  std::string guest_id_;
  bool quick_guest_;

  std::intptr_t socket_ = -1;
  std::atomic<ConnectionState> state_{ConnectionState::Idle};
  std::unique_ptr<std::thread> reader_;
  std::atomic<bool> running_{false};
  bool wsa_started_ = false;
  bool ever_ready_ = false;
  bool suppress_retry_ = false;
  int retry_attempt_ = 0;
  std::chrono::steady_clock::time_point retry_at_{};
  std::chrono::steady_clock::time_point last_state_request_{};

  std::mutex send_mutex_;
  std::mutex inbox_mutex_;
  std::deque<std::string> inbox_;
  std::atomic<bool> peer_dropped_{false};

  ClientModel model_;
  std::vector<PresentationEvent> pending_events_;
  std::string last_error_;
  std::string last_facing_{"down"};
  std::string last_move_dir_;
  bool aim_held_ = false;
  std::string pending_equip_uuid_;
};

}  // namespace verdigris::client
