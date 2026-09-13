#include "local_session.hpp"
#include "input/make-aim-independent-of-motion.hpp"

#include <algorithm>

namespace verdigris::client {

namespace {
// ClientModel coordinates use protocol tiles. This is the inverse of the
// shared protocol_to_world conversion; the deterministic core uses world units.
double local_model_coordinate(int world_units) {
  return static_cast<double>(world_units) * 8.0 / verdigris::world_scale::kArenaHalfExtent;
}

std::string local_model_facing(verdigris::Vec2 facing) {
  if (facing.y < 0) return facing.x < 0 ? "up-left" : facing.x > 0 ? "up-right" : "up";
  if (facing.y > 0) return facing.x < 0 ? "down-left" : facing.x > 0 ? "down-right" : "down";
  return facing.x < 0 ? "left" : "right";
}
}  // namespace

const char* connection_state_label(ConnectionState state) {
  switch (state) {
    case ConnectionState::Idle: return "idle";
    case ConnectionState::Connecting: return "connecting";
    case ConnectionState::Connected: return "connected";
    case ConnectionState::Ready: return "ready";
    case ConnectionState::Disconnected: return "disconnected";
    case ConnectionState::Retrying: return "retrying";
    case ConnectionState::Rejected: return "rejected";
    case ConnectionState::ProtocolMismatch: return "protocol-mismatch";
  }
  return "unknown";
}

ClientCommand ClientCommand::login(std::string guest_id, bool quick_guest) {
  ClientCommand command;
  command.type = Type::Login;
  command.target = std::move(guest_id);
  command.value = quick_guest ? 1 : 0;
  return command;
}
ClientCommand ClientCommand::move(int dx, int dy) {
  ClientCommand command;
  command.type = Type::Move;
  command.dx = dx;
  command.dy = dy;
  return command;
}
ClientCommand ClientCommand::aim(int dx, int dy) {
  ClientCommand command;
  command.type = Type::Aim;
  command.dx = dx;
  command.dy = dy;
  return command;
}
ClientCommand ClientCommand::use_action(std::string action) {
  ClientCommand command;
  command.type = Type::UseAction;
  command.target = std::move(action);
  return command;
}
ClientCommand ClientCommand::pick_up(std::string item_uuid) {
  ClientCommand command;
  command.type = Type::PickUp;
  command.target = std::move(item_uuid);
  return command;
}
ClientCommand ClientCommand::equip(std::string item_uuid) {
  ClientCommand command;
  command.type = Type::Equip;
  command.target = std::move(item_uuid);
  return command;
}
ClientCommand ClientCommand::enter_zone(std::string node_id) {
  ClientCommand command;
  command.type = Type::EnterZone;
  command.target = std::move(node_id);
  return command;
}
ClientCommand ClientCommand::extract() {
  ClientCommand command;
  command.type = Type::Extract;
  return command;
}
ClientCommand ClientCommand::found_house(std::string house_name) {
  ClientCommand command;
  command.type = Type::FoundHouse;
  command.target = std::move(house_name);
  return command;
}
ClientCommand ClientCommand::create_scion(std::string scion_name) {
  ClientCommand command;
  command.type = Type::CreateScion;
  command.target = std::move(scion_name);
  return command;
}
ClientCommand ClientCommand::select_scion(std::string scion_id, bool mortal_oath) {
  ClientCommand command;
  command.type = Type::SelectScion;
  command.target = std::move(scion_id);
  command.value = mortal_oath ? 1 : 0;
  return command;
}
ClientCommand ClientCommand::set_out(std::string scion_id) {
  ClientCommand command;
  command.type = Type::SetOut;
  command.target = std::move(scion_id);
  return command;
}
ClientCommand ClientCommand::npc_action(int npc_id, std::string action_id) {
  ClientCommand command;
  command.type = Type::NpcAction;
  command.value = npc_id;
  command.target = std::move(action_id);
  return command;
}
ClientCommand ClientCommand::menu_action(std::string action_id,
                                         std::string item_ref, int value) {
  ClientCommand command;
  command.type = Type::MenuAction;
  command.target = std::move(action_id);
  command.extra = std::move(item_ref);
  command.value = value;
  return command;
}
ClientCommand ClientCommand::close_screen() {
  ClientCommand command;
  command.type = Type::CloseScreen;
  return command;
}
ClientCommand ClientCommand::allocate_node(std::string node_id) {
  ClientCommand command;
  command.type = Type::AllocateNode;
  command.target = std::move(node_id);
  return command;
}

LocalCoreSession::LocalCoreSession(std::uint64_t seed, std::string house_name)
    : seed_(seed), house_name_(std::move(house_name)) {}

LocalCoreSession::~LocalCoreSession() { shutdown(); }

bool LocalCoreSession::start(std::string*) {
  simulation_ = std::make_unique<verdigris::Simulation>(seed_, house_name_);
  pending_commands_.clear();
  pending_events_.clear();
  ground_positions_.clear();
  aim_hold_ = {};
  processed_events_ = 0;
  state_ = ConnectionState::Ready;  // local play needs no handshake
  pending_events_.push_back({PresentationEventType::SessionReady, "", "", "local", 0});
  refresh_model();
  return true;
}

void LocalCoreSession::shutdown() {
  pending_commands_.clear();
  ground_positions_.clear();
  aim_hold_ = {};
  simulation_.reset();
  state_ = ConnectionState::Disconnected;
}

void LocalCoreSession::submit(const ClientCommand& command) {
  if (!simulation_) return;
  switch (command.type) {
    case ClientCommand::Type::Login:
      break;  // local sessions are implicitly logged in
    case ClientCommand::Type::Move:
      queue_command(verdigris::Command::move(command.dx, command.dy));
      if (aim_hold_.held)
        queue_command(
            verdigris::Command::aim(aim_hold_.dx, aim_hold_.dy));
      break;
    case ClientCommand::Type::Aim:
      move::remember_aim(aim_hold_, command.dx, command.dy);
      queue_command(verdigris::Command::aim(command.dx, command.dy));
      break;
    case ClientCommand::Type::UseAction: {
      // TASK-0122 Phase A: the seam now carries the named action through
      // instead of flattening every skill to Melee, so lifecycle beats such
      // as BuffExpired(war-cry) are reachable by seam consumers.
      verdigris::ActionType action = verdigris::ActionType::Melee;
      if (command.target == "war-cry") action = verdigris::ActionType::WarCry;
      else if (command.target == "thrust") action = verdigris::ActionType::Thrust;
      else if (command.target == "sweep") action = verdigris::ActionType::Sweep;
      else if (command.target == "wait") action = verdigris::ActionType::Wait;
      else if (command.target == "dash") action = verdigris::ActionType::Dash;
      queue_command(verdigris::Command::action_use(action));
      break;
    }
    case ClientCommand::Type::PickUp:
      queue_command(verdigris::Command::pick_up(command.target));
      break;
    case ClientCommand::Type::Equip:
      queue_command(verdigris::Command::equip(command.target));
      break;
    case ClientCommand::Type::EnterZone:
      queue_command(verdigris::Command::enter(command.target));
      break;
    case ClientCommand::Type::Extract:
      queue_command(verdigris::Command::extract());
      break;
    case ClientCommand::Type::FoundHouse:
      // Local play always has its seeded House; the front door never shows.
      break;
    case ClientCommand::Type::CreateScion:
    case ClientCommand::Type::SelectScion:
    case ClientCommand::Type::SetOut:
    case ClientCommand::Type::NpcAction:
    case ClientCommand::Type::MenuAction:
    case ClientCommand::Type::CloseScreen:
    case ClientCommand::Type::AllocateNode:
      // The local simulation admits its single Scion at construction and
      // carries no town NPCs or screens; these intents have no local
      // authority to invoke.
      break;
  }
}

void LocalCoreSession::queue_command(const verdigris::Command& command) {
  // Coalesce sampled intent only after the latest discrete command. An
  // AimEast / Attack / AimWest batch must still attack toward the east.
  if (command.type == verdigris::CommandType::MoveIntent ||
      command.type == verdigris::CommandType::AimIntent) {
    auto suffix = pending_commands_.end();
    while (suffix != pending_commands_.begin()) {
      const auto type = (suffix - 1)->type;
      if (type != verdigris::CommandType::MoveIntent &&
          type != verdigris::CommandType::AimIntent) break;
      --suffix;
    }
    pending_commands_.erase(std::remove_if(suffix, pending_commands_.end(),
        [&](const verdigris::Command& pending) { return pending.type == command.type; }),
        pending_commands_.end());
  }
  constexpr std::size_t kMaxPendingCommands = 64;
  if (pending_commands_.size() < kMaxPendingCommands) pending_commands_.push_back(command);
}

void LocalCoreSession::advance_fixed_tick() {
  if (!simulation_) return;
  simulation_->dispatch_tick(pending_commands_);
  pending_commands_.clear();
  poll();
}

void LocalCoreSession::set_navigation_obstacles(
    std::vector<verdigris::NavigationObstacle> obstacles) {
  if (simulation_)
    simulation_->set_navigation_obstacles(simulation_->instance().active
        ? std::move(obstacles) : std::vector<verdigris::NavigationObstacle>{});
}

std::vector<verdigris::Vec2> LocalCoreSession::navigation_anchors() const {
  return simulation_ ? simulation_->navigation_anchors() : std::vector<verdigris::Vec2>{};
}

void LocalCoreSession::poll() {
  if (!simulation_) return;
  translate_new_events();
  refresh_model();
}

std::vector<PresentationEvent> LocalCoreSession::drain_events() {
  std::vector<PresentationEvent> drained;
  drained.swap(pending_events_);
  return drained;
}

void LocalCoreSession::refresh_model() {
  const auto& scion = simulation_->scion();
  model_.house_name = simulation_->house().name;
  // Rendering/event ownership follows the live actor. The persistent Scion
  // identity remains in the chronicle roster and active_scion_id below.
  model_.player.uuid = scion.actor_id;
  model_.player.display_name = scion.name;
  model_.player.level = scion.level;
  model_.player.alive = scion.alive;
  if (const auto* actor = simulation_->actor(scion.actor_id)) {
    model_.player.x = local_model_coordinate(actor->position.x);
    model_.player.y = local_model_coordinate(actor->position.y);
    model_.player.life = actor->stats.life;
    model_.player.life_max = actor->stats.life_max;
    model_.player.resource = actor->stats.resource;
    model_.player.resource_max = actor->stats.resource_max;
    model_.player.attack = actor->stats.attack;
    model_.player.facing = local_model_facing(actor->facing);
  }
  const auto& instance = simulation_->instance();
  model_.player.scene_id = instance.active ? instance.route_id : std::string{};
  model_.scene.id = instance.active ? instance.route_id : "surface";
  model_.scene.type = instance.active ? "instance" : "surface";
  model_.scene.has_stairs_up = instance.active;
  model_.scene.stairs_up_x = instance.active ? local_model_coordinate(instance.extraction_point.x) : 0.0;
  model_.scene.stairs_up_y = instance.active ? local_model_coordinate(instance.extraction_point.y) : 0.0;
  model_.inventory.clear();
  model_.equipped = {};
  for (const auto& item : scion.carried_items) {
    ClientItemSlot slot{item.id, item.id, item.name, -1, 0, 0, item.attack_bonus};
    model_.inventory.push_back(slot);
    if (item.equipped) model_.equipped = slot;
  }
  model_.ground.clear();
  std::unordered_map<std::string, verdigris::Vec2> retained_positions;
  const auto append_ground = [&](const auto& item) {
    const auto found = ground_positions_.find(item.id);
    if (found == ground_positions_.end()) {
      // Pending recovery can reattach to a new floor without a position
      // event. Preserve its legacy zero-coordinate representation; this is
      // explicitly not an authored anchor or an exact-position guarantee.
      model_.ground.push_back({item.id, item.name, 0.0, 0.0});
      return;
    }
    const auto position = found->second;
    model_.ground.push_back({item.id, item.name, local_model_coordinate(position.x),
                             local_model_coordinate(position.y)});
    retained_positions.emplace(item.id, position);
  };
  for (const auto& item : simulation_->ground_items()) append_ground(item);
  for (const auto& trophy : simulation_->ground_trophies()) append_ground(trophy);
  ground_positions_.swap(retained_positions);
  model_.monsters.clear();
  for (const auto& actor : simulation_->actors()) {
    if (actor.kind != verdigris::ActorKind::Monster || !actor.alive) continue;
    ClientMonster monster;
    monster.id = actor.id;
    monster.name = actor.elite ? "elite" : "monster";
    monster.x = local_model_coordinate(actor.position.x);
    monster.y = local_model_coordinate(actor.position.y);
    monster.behaviour = "melee";
    monster.life = actor.stats.life;
    monster.life_max = actor.stats.life_max;
    monster.elite = actor.elite;
    monster.alive = actor.alive;
    model_.monsters.push_back(std::move(monster));
  }
  model_.stored_items = static_cast<int>(simulation_->house().stored_items.size());
  model_.stored_trophies = static_cast<int>(simulation_->house().stored_trophies.size());
  // Local play mirrors the same chronicle view remote sessions parse: one
  // House, its living Scion, no front-door pending state.
  model_.chronicle.present = true;
  model_.chronicle.account_name = scion.name;
  if (model_.chronicle.houses.empty()) model_.chronicle.houses.emplace_back();
  ClientHouseEntry& local_house = model_.chronicle.houses.front();
  local_house.id = simulation_->house().id.empty() ? "local" : simulation_->house().id;
  local_house.name = model_.house_name;
  local_house.scions.clear();
  local_house.scions.push_back({scion.id, scion.name, scion.level, false});
  local_house.crypt.clear();
  model_.chronicle.active_house_id = local_house.id;
  model_.chronicle.active_scion_id = scion.alive ? scion.id : std::string{};
  model_.chronicles_pending = false;
}

void LocalCoreSession::translate_new_events() {
  const auto& events = simulation_->events();
  for (std::size_t i = processed_events_; i < events.size(); ++i) {
    const auto& event = events[i];
    PresentationEvent out;
    out.actor_id = event.actor_id;
    out.item_id = event.item_id;
    out.text = event.text;
    out.value = event.value;
    // Event poses already use world units. Only ClientModel positions use
    // protocol tiles; converting these would relocate/rotate confirmed hits.
    out.has_actor_pose = event.has_actor_pose;
    out.actor_x = event.actor_position.x;
    out.actor_y = event.actor_position.y;
    out.facing_x = event.actor_facing.x;
    out.facing_y = event.actor_facing.y;
    if (event.has_actor_pose &&
        (event.type == verdigris::EventType::ItemDropped ||
         event.type == verdigris::EventType::TrophyDropped ||
         event.type == verdigris::EventType::RelicResurfaced ||
         event.type == verdigris::EventType::TrophyResurfaced)) {
      const auto& id = event.item_id.empty() ? event.trophy_id : event.item_id;
      if (!id.empty()) ground_positions_[id] = event.actor_position;
    }
    switch (event.type) {
      case verdigris::EventType::AttackStarted: out.type = PresentationEventType::AttackStarted; break;
      case verdigris::EventType::DamageApplied: out.type = PresentationEventType::DamageApplied; break;
      case verdigris::EventType::ActorDied: out.type = PresentationEventType::ActorDied; break;
      case verdigris::EventType::ItemDropped: out.type = PresentationEventType::ItemDropped; break;
      case verdigris::EventType::ItemPickedUp: out.type = PresentationEventType::ItemPickedUp; break;
      case verdigris::EventType::ItemEquipped: out.type = PresentationEventType::ItemEquipped; break;
      case verdigris::EventType::ItemExtracted: out.type = PresentationEventType::ExtractionCompleted; break;
      // TASK-0122 Phase A: the previously-dropped lifecycle events now cross
      // the presentation seam so every renderer consumes PresentationEvents.
      case verdigris::EventType::ScionLost: out.type = PresentationEventType::ScionLost; break;
      case verdigris::EventType::BuffExpired: out.type = PresentationEventType::BuffExpired; break;
      case verdigris::EventType::AttackTelegraphed:
        out.type = PresentationEventType::Telegraph;
        break;
      default: continue;  // remaining core events gain mappings with 0061+
    }
    pending_events_.push_back(std::move(out));
  }
  processed_events_ = events.size();
}

}  // namespace verdigris::client
