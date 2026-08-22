#include "local_session.hpp"

namespace verdigris::client {

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

LocalCoreSession::LocalCoreSession(std::uint64_t seed, std::string house_name)
    : seed_(seed), house_name_(std::move(house_name)) {}

LocalCoreSession::~LocalCoreSession() { shutdown(); }

bool LocalCoreSession::start(std::string*) {
  simulation_ = std::make_unique<verdigris::Simulation>(seed_, house_name_);
  processed_events_ = 0;
  state_ = ConnectionState::Ready;  // local play needs no handshake
  pending_events_.push_back({PresentationEventType::SessionReady, "", "", "local", 0});
  refresh_model();
  return true;
}

void LocalCoreSession::shutdown() {
  simulation_.reset();
  state_ = ConnectionState::Disconnected;
}

void LocalCoreSession::submit(const ClientCommand& command) {
  if (!simulation_) return;
  switch (command.type) {
    case ClientCommand::Type::Login:
      break;  // local sessions are implicitly logged in
    case ClientCommand::Type::Move:
      simulation_->dispatch(verdigris::Command::move(command.dx, command.dy));
      break;
    case ClientCommand::Type::Aim:
      simulation_->dispatch(verdigris::Command::aim(command.dx, command.dy));
      break;
    case ClientCommand::Type::UseAction:
      simulation_->dispatch(verdigris::Command::action_use(verdigris::ActionType::Melee));
      break;
    case ClientCommand::Type::PickUp:
      simulation_->dispatch(verdigris::Command::pick_up(command.target));
      break;
    case ClientCommand::Type::Equip:
      simulation_->dispatch(verdigris::Command::equip(command.target));
      break;
    case ClientCommand::Type::EnterZone:
      simulation_->dispatch(verdigris::Command::enter(command.target));
      break;
    case ClientCommand::Type::Extract:
      simulation_->dispatch(verdigris::Command::extract());
      break;
    case ClientCommand::Type::FoundHouse:
      // Local play always has its seeded House; the front door never shows.
      break;
    case ClientCommand::Type::CreateScion:
    case ClientCommand::Type::SelectScion:
    case ClientCommand::Type::SetOut:
      // The local simulation admits its single Scion at construction; these
      // intents have no additional local authority to invoke.
      break;
  }
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
  model_.player.uuid = scion.id;
  model_.player.display_name = scion.name;
  model_.player.level = scion.level;
  model_.player.alive = scion.alive;
  if (const auto* actor = simulation_->actor(scion.actor_id)) {
    model_.player.x = actor->position.x;
    model_.player.y = actor->position.y;
    model_.player.life = actor->stats.life;
    model_.player.life_max = actor->stats.life_max;
    model_.player.resource = actor->stats.resource;
    model_.player.resource_max = actor->stats.resource_max;
    model_.player.attack = actor->stats.attack;
    if (actor->facing.x < 0) model_.player.facing = "left";
    else if (actor->facing.x > 0) model_.player.facing = "right";
    else if (actor->facing.y < 0) model_.player.facing = "up";
    else model_.player.facing = "down";
  }
  model_.scene.id = simulation_->instance().active ? simulation_->instance().route_id : "surface";
  model_.inventory.clear();
  model_.equipped = {};
  for (const auto& item : scion.carried_items) {
    ClientItemSlot slot{item.id, item.id, item.name, -1, 0, 0, item.attack_bonus};
    model_.inventory.push_back(slot);
    if (item.equipped) model_.equipped = slot;
  }
  model_.ground.clear();
  for (const auto& item : simulation_->ground_items())
    model_.ground.push_back({item.id, item.name, 0.0, 0.0});
  model_.monsters.clear();
  for (const auto& actor : simulation_->actors()) {
    if (actor.kind != verdigris::ActorKind::Monster || !actor.alive) continue;
    model_.monsters.push_back({actor.id, actor.elite ? "elite" : "monster",
                               static_cast<double>(actor.position.x),
                               static_cast<double>(actor.position.y), actor.stats.life,
                               actor.stats.life_max, actor.elite, actor.alive});
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
    switch (event.type) {
      case verdigris::EventType::AttackStarted: out.type = PresentationEventType::AttackStarted; break;
      case verdigris::EventType::DamageApplied: out.type = PresentationEventType::DamageApplied; break;
      case verdigris::EventType::ActorDied: out.type = PresentationEventType::ActorDied; break;
      case verdigris::EventType::ItemDropped: out.type = PresentationEventType::ItemDropped; break;
      case verdigris::EventType::ItemPickedUp: out.type = PresentationEventType::ItemPickedUp; break;
      case verdigris::EventType::ItemEquipped: out.type = PresentationEventType::ItemEquipped; break;
      case verdigris::EventType::ItemExtracted: out.type = PresentationEventType::ExtractionCompleted; break;
      default: continue;  // remaining core events gain mappings with 0061+
    }
    pending_events_.push_back(std::move(out));
  }
  processed_events_ = events.size();
}

}  // namespace verdigris::client
