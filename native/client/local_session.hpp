#pragma once

// LocalCoreSession (D-122): the ONLY client code allowed to own a
// verdigris::Simulation. Exists so deterministic client scenarios, negative
// controls, and replay fixtures survive the move to networked play.

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "session.hpp"
#include "verdigris/core.hpp"

namespace verdigris::client {

class LocalCoreSession final : public IClientSession {
 public:
  explicit LocalCoreSession(std::uint64_t seed, std::string house_name = "House Verdigris");
  ~LocalCoreSession() override;

  bool start(std::string* error = nullptr) override;
  void shutdown() override;
  void submit(const ClientCommand& command) override;
  void poll() override;
  ConnectionState connection_state() const override { return state_; }
  const ClientModel& model() const override { return model_; }
  std::vector<PresentationEvent> drain_events() override;
  const std::string& last_error() const override { return last_error_; }

  // Deterministic-test escape hatch. Scenario drivers may use this to reach
  // the simulation; production presentation code must not.
  verdigris::Simulation* simulation_for_scenarios() { return simulation_.get(); }

 private:
  void refresh_model();
  void translate_new_events();

  std::uint64_t seed_;
  std::string house_name_;
  std::unique_ptr<verdigris::Simulation> simulation_;
  ConnectionState state_ = ConnectionState::Idle;
  ClientModel model_;
  std::vector<PresentationEvent> pending_events_;
  std::size_t processed_events_ = 0;
  std::string last_error_;
};

}  // namespace verdigris::client
