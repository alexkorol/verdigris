#include "presentation_state.hpp"

#include <algorithm>
#include <cmath>

namespace verdigris::client {

verdigris::Vec2 facing_vector(const std::string& facing) {
  if (facing == "left" || facing == "west") return {-1, 0};
  if (facing == "right" || facing == "east") return {1, 0};
  if (facing == "up" || facing == "north") return {0, -1};
  return {0, 1};
}

double protocol_to_world(double protocol_units) {
  return protocol_units *
         (static_cast<double>(verdigris::world_scale::kArenaHalfExtent) / 8.0);
}

void sync_world_from_simulation(WorldView& world, const verdigris::Simulation& sim) {
  world = WorldView{};
  world.house_name = sim.house().name;
  world.scion_name = sim.scion().name;
  world.tick = sim.tick();
  world.route_id = sim.instance().route_id;
  world.extraction = sim.instance().extraction_point;
  world.has_extraction = true;
  world.stored_items = sim.house().stored_items.size();
  world.stored_trophies = sim.house().stored_trophies.size();
  world.carried_trophies = sim.scion().carried_trophies.size();
  if (const auto* player = sim.actor(sim.scion().actor_id)) {
    world.player.id = player->id;
    world.player.position = player->position;
    world.player.facing = player->facing;
    world.player.life = player->stats.life;
    world.player.life_max = player->stats.life_max;
    world.player.resource = player->stats.resource;
    world.player.resource_max = player->stats.resource_max;
    world.player.attack = player->stats.attack;
    world.player.defense = player->stats.defense;
    world.player.level = player->stats.level;
    world.player.cooldown_ticks = player->cooldown_ticks;
    world.player.war_cry_ticks_remaining = player->war_cry_ticks_remaining;
    world.player.alive = player->alive;
  }
  for (const auto& actor : sim.actors()) {
    if (actor.kind != verdigris::ActorKind::Monster || !actor.alive) continue;
    WorldActor monster;
    monster.id = actor.id;
    monster.position = actor.position;
    monster.facing = actor.facing;
    monster.life = actor.stats.life;
    monster.life_max = actor.stats.life_max;
    monster.alive = actor.alive;
    monster.elite = actor.elite;
    world.monsters.push_back(std::move(monster));
  }
  for (const auto& item : sim.scion().carried_items) {
    world.carried.push_back({item.id, item.name, item.attack_bonus, item.equipped});
  }
  for (const auto& item : sim.ground_items()) world.loot_names[item.id] = item.name;
  for (const auto& trophy : sim.ground_trophies())
    world.loot_names[trophy.id] = trophy.name;
}

void sync_world_from_model(WorldView& world, const ClientModel& model) {
  world.house_name = model.house_name.empty() ? "House Verdigris" : model.house_name;
  world.scion_name = model.player.uuid;
  world.route_id = model.scene.id;
  world.player.id = model.player.uuid;
  world.player.position = {static_cast<int>(std::lround(protocol_to_world(model.player.x))),
                           static_cast<int>(std::lround(protocol_to_world(model.player.y)))};
  world.player.facing = facing_vector(model.player.facing);
  world.player.life = model.player.life;
  world.player.life_max = model.player.life_max;
  world.player.resource = model.player.resource;
  world.player.resource_max = model.player.resource_max;
  world.player.attack = model.player.attack;
  world.player.level = model.player.level;
  world.player.alive = model.player.alive;
  world.has_extraction = model.scene.has_stairs_up;
  world.extraction = {
      static_cast<int>(std::lround(protocol_to_world(model.scene.stairs_up_x))),
      static_cast<int>(std::lround(protocol_to_world(model.scene.stairs_up_y)))};
  world.stored_items = static_cast<std::size_t>(std::max(0, model.stored_items));
  world.stored_trophies = static_cast<std::size_t>(std::max(0, model.stored_trophies));
  world.monsters.clear();
  for (const auto& source : model.monsters) {
    if (!source.alive) continue;
    WorldActor monster;
    monster.id = source.id;
    monster.position = {static_cast<int>(std::lround(protocol_to_world(source.x))),
                        static_cast<int>(std::lround(protocol_to_world(source.y)))};
    monster.facing = facing_vector(model.player.facing);
    monster.facing.x = -monster.facing.x;
    monster.facing.y = -monster.facing.y;
    monster.life = source.life;
    monster.life_max = source.life_max;
    monster.alive = source.alive;
    monster.elite = source.elite;
    world.monsters.push_back(std::move(monster));
  }
  world.carried.clear();
  for (const auto& item : model.inventory) {
    const std::string label = item.name.empty() ? item.id : item.name;
    world.carried.push_back({item.uuid, label, item.attack_rating, false});
  }
  if (!model.equipped.uuid.empty()) {
    const std::string label =
        model.equipped.name.empty() ? model.equipped.id : model.equipped.name;
    world.carried.push_back(
        {model.equipped.uuid, label, model.equipped.attack_rating, true});
  }
  world.loot_names.clear();
  for (const auto& item : model.ground)
    world.loot_names[item.uuid] = item.name.empty() ? item.uuid : item.name;
}

const WorldActor* find_monster(const WorldView& world, const std::string& id) {
  for (const auto& monster : world.monsters)
    if (monster.id == id) return &monster;
  return nullptr;
}

verdigris::Vec2 event_anchor(const WorldView& world, const PresentationFx& fx,
                             const PresentationEvent& event, bool prefer_player) {
  if (prefer_player) return world.player.position;
  if (const auto* monster = find_monster(world, event.actor_id)) return monster->position;
  if (fx.last_death_pos.x != 0 || fx.last_death_pos.y != 0) return fx.last_death_pos;
  verdigris::Vec2 at = world.player.position;
  at.x += world.player.facing.x * verdigris::world_scale::kMeleeRange;
  at.y += world.player.facing.y * verdigris::world_scale::kMeleeRange;
  return at;
}

void apply_presentation_event(PresentationFx& fx, const WorldView& world,
                              const PresentationEvent& event, std::uint64_t now_tick) {
  const bool to_player = event.text == "incoming" || event.type == PresentationEventType::ScionDied;
  const verdigris::Vec2 at = event_anchor(world, fx, event, to_player);
  const double ex = static_cast<double>(at.x);
  const double ey = static_cast<double>(at.y);
  switch (event.type) {
    case PresentationEventType::AttackStarted:
      fx.telegraphs.erase(event.actor_id);
      fx.effects.push_back({EffectFx::Kind::Swing, static_cast<double>(world.player.position.x),
                            static_cast<double>(world.player.position.y), 0.0, 0, 6});
      break;
    case PresentationEventType::DamageApplied: {
      fx.effects.push_back({EffectFx::Kind::Impact, ex, ey, 0.0, 0, 4});
      EffectFx flash;
      flash.kind = EffectFx::Kind::TargetFlash;
      flash.wx = ex;
      flash.wy = ey;
      flash.ttl = 4;
      flash.damage_to_player = to_player;
      fx.effects.push_back(flash);
      EffectFx number;
      number.kind = EffectFx::Kind::DamageNumber;
      number.wx = ex;
      number.wy = ey;
      number.ttl = 12;
      number.value = event.value;
      number.damage_to_player = to_player;
      fx.effects.push_back(number);
      if (to_player) fx.screen_pulse_ticks = 3;
      break;
    }
    case PresentationEventType::Telegraph: {
      ActiveTelegraph telegraph;
      telegraph.actor_id = event.actor_id;
      telegraph.action = event.text.find("sweep") != std::string::npos ? "sweep" : "thrust";
      telegraph.position = event_anchor(world, fx, event, false);
      telegraph.facing = world.player.facing;
      telegraph.facing.x = -telegraph.facing.x;
      telegraph.facing.y = -telegraph.facing.y;
      telegraph.start_tick = now_tick;
      telegraph.windup_ticks = std::max(1, event.value > 20 ? event.value / 50 : event.value);
      fx.telegraphs[event.actor_id.empty() ? "foe" : event.actor_id] = std::move(telegraph);
      break;
    }
    case PresentationEventType::ActorDied:
    case PresentationEventType::ScionDied:
      fx.telegraphs.erase(event.actor_id);
      if (event.type == PresentationEventType::ScionDied) fx.telegraphs.clear();
      fx.last_death_pos = at;
      fx.effects.push_back({EffectFx::Kind::DeathRing, ex, ey, 0.0, 0, 12});
      fx.effects.push_back({EffectFx::Kind::Dust, ex, ey, 0.7, 0, 10});
      break;
    case PresentationEventType::ItemDropped: {
      verdigris::Vec2 drop = fx.last_death_pos;
      if (drop.x == 0 && drop.y == 0) drop = at;
      drop.x += (fx.loot_scatter % 3 - 1) * 40;
      drop.y += ((fx.loot_scatter / 3) % 3 - 1) * 40 + 30;
      ++fx.loot_scatter;
      const std::string id = event.item_id.empty() ? ("drop-" + std::to_string(fx.loot_scatter))
                                                   : event.item_id;
      fx.loot_positions[id] = drop;
      fx.effects.push_back({EffectFx::Kind::Sparkle, static_cast<double>(drop.x),
                            static_cast<double>(drop.y), 0.0, 0, 24});
      break;
    }
    case PresentationEventType::ItemPickedUp:
      if (!event.item_id.empty()) fx.loot_positions.erase(event.item_id);
      else if (!fx.loot_positions.empty()) fx.loot_positions.erase(fx.loot_positions.begin());
      break;
    case PresentationEventType::ExtractionCompleted:
      fx.hint = "Returned to the surface";
      fx.hint_ticks = 80;
      break;
    case PresentationEventType::ConnectionLost:
      fx.hint = event.text.empty() ? "CONNECTION LOST — not playing offline" : event.text;
      fx.hint_ticks = 200;
      fx.screen_pulse_ticks = 8;
      break;
    case PresentationEventType::Message:
    case PresentationEventType::SessionReady:
    case PresentationEventType::ItemEquipped:
    case PresentationEventType::ConnectionEstablished:
    case PresentationEventType::ProtocolError:
      break;
  }
  if (event.type != PresentationEventType::ConnectionEstablished) {
    std::string line;
    switch (event.type) {
      case PresentationEventType::DamageApplied:
        line = (to_player ? "Taken " : "Hit ") + std::to_string(event.value);
        break;
      case PresentationEventType::ActorDied:
        line = "Kill " + event.text;
        break;
      case PresentationEventType::Telegraph:
        line = "Telegraph " + event.text;
        break;
      case PresentationEventType::ItemDropped:
        line = "Loot";
        break;
      case PresentationEventType::ItemPickedUp:
        line = "Picked up " + event.text;
        break;
      case PresentationEventType::ConnectionLost:
        line = "DISCONNECTED " + event.text;
        break;
      default:
        break;
    }
    if (!line.empty()) {
      fx.event_log.push_back(line);
      if (fx.event_log.size() > 6) fx.event_log.erase(fx.event_log.begin());
    }
  }
}

void age_presentation_fx(PresentationFx& fx) {
  for (auto& effect : fx.effects) ++effect.age;
  fx.effects.erase(std::remove_if(fx.effects.begin(), fx.effects.end(),
                                  [](const EffectFx& effect) { return effect.age >= effect.ttl; }),
                   fx.effects.end());
  if (fx.hint_ticks > 0) --fx.hint_ticks;
  if (fx.screen_pulse_ticks > 0) --fx.screen_pulse_ticks;
}

void record_world_ops(render::List& rl, const WorldView& world, const PresentationFx& fx,
                      const camera2d::Camera& camera, int width, int height) {
  const camera2d::Screen screen{width, height};
  auto at = [&](double wx, double wy) { return camera2d::project(camera, screen, wx, wy); };
  if (world.has_extraction) {
    const auto pad = at(world.extraction.x, world.extraction.y);
    rl.push_back({render::Op::Extraction, static_cast<double>(pad.x),
                  static_cast<double>(pad.y)});
  }
  if (world.player.alive) {
    const auto base = at(world.player.position.x, world.player.position.y);
    rl.push_back({render::Op::Player, static_cast<double>(base.x), static_cast<double>(base.y)});
  }
  for (const auto& monster : world.monsters) {
    const auto base = at(monster.position.x, monster.position.y);
    rl.push_back({render::Op::Monster, static_cast<double>(base.x),
                  static_cast<double>(base.y), 0.0, monster.life,
                  monster.elite ? "elite" : "monster"});
  }
  for (const auto& loot : fx.loot_positions) {
    const auto base = at(loot.second.x, loot.second.y);
    rl.push_back({render::Op::Drop, static_cast<double>(base.x), static_cast<double>(base.y), 0.0,
                  0, loot.first});
  }
  for (const auto& entry : fx.telegraphs) {
    const auto base = at(entry.second.position.x, entry.second.position.y);
    rl.push_back({render::Op::Telegraph, static_cast<double>(base.x),
                  static_cast<double>(base.y), 0.0, 0, entry.second.action});
  }
  for (const auto& effect : fx.effects) {
    const auto base = at(effect.wx, effect.wy);
    switch (effect.kind) {
      case EffectFx::Kind::Swing:
        rl.push_back({render::Op::Swing, static_cast<double>(base.x),
                      static_cast<double>(base.y)});
        break;
      case EffectFx::Kind::SweepArc:
        rl.push_back({render::Op::Sweep, static_cast<double>(base.x),
                      static_cast<double>(base.y)});
        break;
      case EffectFx::Kind::DamageNumber:
        rl.push_back({render::Op::Damage, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, effect.value,
                      effect.damage_to_player ? "player" : "monster"});
        break;
      case EffectFx::Kind::DeathRing:
        rl.push_back({render::Op::Death, static_cast<double>(base.x),
                      static_cast<double>(base.y)});
        break;
      case EffectFx::Kind::TargetFlash:
        rl.push_back({render::Op::TargetFlash, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, 0,
                      effect.damage_to_player ? "player" : "monster"});
        break;
      default:
        break;
    }
  }
  if (fx.screen_pulse_ticks > 0)
    rl.push_back({render::Op::ScreenPulse, 0.0, 0.0, 0.0, 0, "player-damage"});
}

}  // namespace verdigris::client
