#pragma once
// Included after ClientState. Presentation anchors follow the rendered actor;
// detached particles retain the point at which the event occurred.
namespace particle_view {
using namespace verdigris::client::vfx;
inline const WorldActor* actor(const WorldView& world,const std::string& id) {
  if(id==world.player.id)return &world.player;
  for(const auto& a:world.monsters)if(a.id==id)return &a;
  return nullptr;
}
inline std::optional<Vec3> resolve(const ClientState& state,const Attachment& attachment) {
  if(attachment.socket==Anchor::World)return attachment.point;
  const auto* a=actor(state.world,attachment.entity);
  if(!a || !a->alive)return {};
  const auto& p=a->displayed_position();
  Vec3 point{float(p.x),float(p.y),0};
  const float length=std::max(1.f,std::hypot(float(a->facing.x),float(a->facing.y)));
  const float dx=a->facing.x/length,dy=a->facing.y/length;
  // Authored actor canvas anchors, in world units, shared across all poses.
  if(attachment.socket==Anchor::Head)point.z=110;
  if(attachment.socket==Anchor::MainHand || attachment.socket==Anchor::OffHand) {
    const float side=attachment.socket==Anchor::MainHand?1.f:-1.f;
    point.x+=dx*18-dy*18*side;point.y+=dy*18+dx*18*side;point.z=55;
  }
  return point+attachment.point;
}
}
void ensure_particle_assets(ClientState& state) {
  if(state.particle_assets_attempted)return;
  state.particle_assets_attempted=true;
  const std::string root=executable_directory()+"/../client/assets/effects";
  if(verdigris::client::vfx::load_assets(root,state.particles.assets))return;
  for(const char* root:{"native/client/assets/effects","client/assets/effects","../client/assets/effects","../../native/client/assets/effects"})
    if(verdigris::client::vfx::load_assets(root,state.particles.assets))return;
}
void clear_particles(ClientState& state) {
  state.particles.clear();state.bowl_emitter=0;state.particle_step_known=false;state.particle_step_distance=0;
  state.particle_scene=state.world.route_id;
}
void particle_event(ClientState& state,const verdigris::client::PresentationEvent& event,const WorldView& world) {
  using namespace verdigris::client::vfx;
  using E=verdigris::client::PresentationEventType;
  ensure_particle_assets(state);
  if(state.particle_scene!=state.world.route_id)clear_particles(state);
  if(event.type==E::SessionReady || event.type==E::ConnectionLost || event.type==E::ScionLost) {clear_particles(state);return;}
  if(event.type==E::LevelUp) {
    state.particles.play("level_up",{event.actor_id,Anchor::Root},++state.particle_seed);return;
  }
  const auto* actor=particle_view::actor(world,event.text=="incoming"?world.player.id:event.actor_id);
  if(!actor)return;
  const auto p=actor->displayed_position();
  Attachment at{"",Anchor::World,{float(p.x),float(p.y),event.type==E::ActorDied?8.f:52.f}};
  if(event.type==E::DamageApplied && event.value>0)
    state.particles.play(event.item_id=="burning-touch"?"burning_touch_contact":"melee_hit_small",at,++state.particle_seed);
  if(event.type==E::ActorDied)state.particles.play("simple_death_puff",at,++state.particle_seed);
}
void tick_particles(ClientState& state) {
  using namespace verdigris::client::vfx;
  ensure_particle_assets(state);
  if(state.particle_scene!=state.world.route_id)clear_particles(state);
  const auto& p=state.world.player.position;
  if(state.world.player.alive && state.frontend==Frontend::None) {
    if(state.particle_step_known) {
      const float distance=std::hypot(float(p.x-state.particle_last_step.x),float(p.y-state.particle_last_step.y));
      // Teleports and scene admission do not leave a trail across the map.
      if(distance<100)state.particle_step_distance+=distance;else state.particle_step_distance=0;
      if(state.particle_step_distance>=38) {
        state.particle_step_distance=std::fmod(state.particle_step_distance,38.f);
        state.particles.play("foot_dust",{"",Anchor::World,{float(p.x),float(p.y),0}},++state.particle_seed);
      }
    }
    state.particle_last_step=p;state.particle_step_known=true;
  } else state.particle_step_known=false;
  bool bowl=false;Anchor hand=Anchor::MainHand;
  for(const auto& item:state.world.carried)if(item.equipped) {
    std::string name=item.name;std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return char(std::tolower(c));});
    if(name.find("bowl")!=std::string::npos || name.find("fire vessel")!=std::string::npos || name.find("censer")!=std::string::npos) {
      bowl=state.world.player.alive;hand=item.equip_seat=="left_hand"?Anchor::OffHand:Anchor::MainHand;break;
    }
  }
  if(!bowl && state.bowl_emitter){state.particles.stop(state.bowl_emitter);state.bowl_emitter=0;}
  if(bowl && !state.particles.playing(state.bowl_emitter))
    state.bowl_emitter=state.particles.play("bowl_ember_idle",{state.world.player.id,hand},++state.particle_seed);
  if(bowl && state.bowl_emitter)state.particles.attach(state.bowl_emitter,{state.world.player.id,hand});
  state.particles.tick([&](const Attachment& a){return particle_view::resolve(state,a);});
}
