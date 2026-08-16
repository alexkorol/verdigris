#include "verdigris/seasonal.hpp"

namespace verdigris {

void EmberHunt::on_instance_enter(Simulation& simulation, InstanceState&) {
  simulation.add_seasonal_objective("ignite the Warden's ember");
}

void EmberHunt::on_event(Simulation& simulation, const Event& event) {
  if (event.type == EventType::ActorDied && event.text == "monster" && !reward_granted_) {
    reward_granted_ = true;
    simulation.grant_seasonal_reward("seasonal:ember-mark");
  }
}

}  // namespace verdigris
