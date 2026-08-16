#pragma once

#include <string>

#include "verdigris/core.hpp"

namespace verdigris {

class SeasonalMechanic {
 public:
  virtual ~SeasonalMechanic() = default;
  virtual void on_instance_enter(Simulation& simulation, InstanceState& instance) = 0;
  virtual void on_event(Simulation& simulation, const Event& event) = 0;
};

class EmberHunt final : public SeasonalMechanic {
 public:
  void on_instance_enter(Simulation& simulation, InstanceState& instance) override;
  void on_event(Simulation& simulation, const Event& event) override;
  bool reward_granted() const { return reward_granted_; }

 private:
  bool reward_granted_ = false;
};

}  // namespace verdigris
