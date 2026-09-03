#include "presentation_state.hpp"

#include <algorithm>
#include <cmath>

namespace verdigris::client {

verdigris::Vec2 facing_vector(const std::string& facing) {
  // Compound eight-way names ("up-left", ...) resolve component-wise so the
  // rendered facing matches the diagonal the wire actually carried.
  verdigris::Vec2 result{0, 0};
  if (facing.find("left") != std::string::npos || facing == "west") result.x = -1;
  else if (facing.find("right") != std::string::npos || facing == "east") result.x = 1;
  if (facing.find("up") != std::string::npos || facing == "north") result.y = -1;
  else if (facing.find("down") != std::string::npos || facing == "south") result.y = 1;
  if (result.x == 0 && result.y == 0) result.y = 1;
  return result;
}

double protocol_to_world(double protocol_units) {
  return protocol_units *
         (static_cast<double>(verdigris::world_scale::kArenaHalfExtent) / 8.0);
}

const char* extraction_action_hint(bool remote_session) {
  // TASK-0153: exactly one truthful action per session mode (F-3 in accepted
  // TASK-0119). Remote Extract is a deliberate no-op on this wire; walking
  // onto the stairs is the contract.
  return remote_session ? "walk onto it" : "press F there";
}

void sync_world_from_simulation(WorldView& world, const verdigris::Simulation& sim) {
  world.endgame = EndgameView{};
  world = WorldView{};
  world.house_name = sim.house().name;
  world.scion_name = sim.scion().name;
  world.tick = sim.tick();
  world.route_id = sim.instance().route_id;
  // TASK-0153: the core's authoritative expedition phase (SlayWardens ->
  // ExtractCarriedValue, emitted as ExpeditionPhaseChanged) is read directly;
  // the client never re-derives it from actor scans in local play.
  world.expedition_phase =
      sim.instance().phase == verdigris::ExpeditionPhase::ExtractCarriedValue
          ? ExpeditionPhaseView::ExtractCarriedValue
          : ExpeditionPhaseView::SlayWardens;
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
  // TASK-0145: prefer the authoritative chronicle identity when present.
  std::string house_name = model.house_name;
  std::string scion_name;
  if (!model.chronicle.active_house_id.empty()) {
    if (const auto* house =
            find_chronicle_house(model.chronicle, model.chronicle.active_house_id)) {
      house_name = house->name;
      for (const auto& scion : house->scions)
        if (scion.id == model.chronicle.active_scion_id) scion_name = scion.name;
    }
  }
  if (scion_name.empty()) {
    const auto* scion =
        find_chronicle_scion(model.chronicle, model.chronicle.active_scion_id);
    if (scion) scion_name = scion->name;
  }
  world.house_name = house_name.empty() ? "House Verdigris" : house_name;
  world.scion_name = !scion_name.empty() ? scion_name
                                         : (!model.player.display_name.empty()
                                                ? model.player.display_name
                                                : model.player.uuid);
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
  world.player.cooldown_ticks = model.player.cooldown_ticks;
  world.player.combo_step = model.player.combo_step;
  world.player.combo_window_ticks = model.player.combo_window_ticks;
  world.player.war_cry_ticks_remaining = model.player.war_cry_ticks_remaining;
  world.player.alive = model.player.alive;
  world.has_extraction = model.scene.has_stairs_up;
  // TASK-0153 remote phase view: this wire carries no dedicated phase event,
  // so the strip mirrors the equivalent already-authoritative session state —
  // the living-foe snapshot the server publishes. No state is invented: with
  // wardens alive the objective is the slay objective; once the snapshot
  // shows none remaining, only the carry-to-exit leg remains. Outside an
  // authoritative instance scene there is no phase at all (Unknown).
  if (!model.scene.has_stairs_up) {
    world.expedition_phase = ExpeditionPhaseView::Unknown;
  } else {
    bool foes_remain = false;
    for (const auto& monster : model.monsters)
      if (monster.alive) {
        foes_remain = true;
        break;
      }
    world.expedition_phase = foes_remain ? ExpeditionPhaseView::SlayWardens
                                         : ExpeditionPhaseView::ExtractCarriedValue;
  }
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
    monster.name = source.name;
    monster.position = {static_cast<int>(std::lround(protocol_to_world(source.x))),
                        static_cast<int>(std::lround(protocol_to_world(source.y)))};
    // TASK-0122 Phase A: the wire snapshot carries no monster facing field,
    // so the proved client-only fabrication (inverting the player's facing)
    // is removed. Monsters keep the neutral default until the wire ships an
    // authoritative facing; the presentation never invents one from the
    // player's aim.
    monster.kind = source.kind;
    monster.behaviour = source.behaviour;
    monster.life = source.life;
    monster.life_max = source.life_max;
    monster.alive = source.alive;
    monster.elite = source.elite;
    world.monsters.push_back(std::move(monster));
  }
  world.theme = model.theme;
  const double xp_span = model.xp_next - model.xp_floor;
  world.xp_present = model.xp_present && xp_span > 0.0;
  world.xp_fraction = world.xp_present
                          ? std::clamp((model.xp_current - model.xp_floor) /
                                           xp_span,
                                       0.0, 1.0)
                          : 0.0;
  world.map_width = model.map_width;
  world.map_height = model.map_height;
  world.map_walkable = model.map_walkable;
  world.npcs.clear();
  for (const auto& source : model.npcs) {
    WorldNpc npc;
    npc.id = source.id;
    npc.key = source.key;
    npc.name = source.name;
    npc.role = source.role;
    npc.examine = source.examine;
    npc.position = {static_cast<int>(std::lround(protocol_to_world(source.x))),
                    static_cast<int>(std::lround(protocol_to_world(source.y)))};
    npc.services = source.services;
    npc.actions = source.actions;
    world.npcs.push_back(std::move(npc));
  }
  world.carried.clear();
  for (const auto& item : model.inventory) {
    const std::string label = item.name.empty() ? item.id : item.name;
    WorldCarriedItem carried{item.uuid, label, item.attack_rating, false};
    carried.expedition_map = item.expedition_map;
    carried.map_tier = item.map_tier;
    carried.map_goods_found_percent = item.map_goods_found_percent;
    carried.map_family = item.map_family;
    carried.map_objective_key = item.map_objective_key;
    carried.map_modifiers = item.map_modifiers;
    world.carried.push_back(std::move(carried));
  }
  if (!model.equipped.uuid.empty()) {
    const std::string label =
        model.equipped.name.empty() ? model.equipped.id : model.equipped.name;
    world.carried.push_back(
        {model.equipped.uuid, label, model.equipped.attack_rating, true});
  }
  world.endgame.present = model.endgame.present;
  world.endgame.unlocked = model.endgame.unlocked;
  world.endgame.active = model.endgame.active;
  world.endgame.cleared = model.endgame.cleared;
  world.endgame.completed = model.endgame.completed;
  world.endgame.mastered = model.endgame.mastered;
  world.endgame.mastery_total = model.endgame.mastery_total;
  world.endgame.highest_tier = model.endgame.highest_tier;
  world.endgame.ascent_chance_percent =
      model.endgame.ascent_chance_percent;
  world.endgame.tier = model.endgame.tier;
  world.endgame.goods_found_percent = model.endgame.goods_found_percent;
  world.endgame.first_clear = model.endgame.first_clear;
  world.endgame.name = model.endgame.name;
  world.endgame.family = model.endgame.family;
  world.endgame.objective_key = model.endgame.objective_key;
  world.endgame.mastery_keys = model.endgame.mastery_keys;
  world.endgame.modifiers = model.endgame.modifiers;
  world.loot_names.clear();
  for (const auto& item : model.ground)
    world.loot_names[item.uuid] = item.name.empty() ? item.uuid : item.name;
  // TASK-0156: the mirrored passive-tree progression travels with the model
  // untouched; sync_world_from_simulation leaves it absent because the local
  // core carries no tree authority on this seam.
  world.progression = model.progression;
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
  const bool to_player = event.text == "incoming" ||
                         event.type == PresentationEventType::ScionDied ||
                         event.type == PresentationEventType::BuffApplied ||
                         event.type == PresentationEventType::BuffExpired;
  const verdigris::Vec2 at = event_anchor(world, fx, event, to_player);
  const double ex = static_cast<double>(at.x);
  const double ey = static_cast<double>(at.y);
  switch (event.type) {
    case PresentationEventType::AttackStarted: {
      fx.telegraphs.erase(event.actor_id);
      // Orient the confirmed swing along the player's authoritative facing
      // instead of a hardcoded eastward arc.
      const double swing_angle =
          std::atan2(static_cast<double>(world.player.facing.y),
                     static_cast<double>(world.player.facing.x));
      fx.effects.push_back({event.text == "sweep" ? EffectFx::Kind::SweepArc
                                                   : EffectFx::Kind::Swing,
                            static_cast<double>(world.player.position.x),
                            static_cast<double>(world.player.position.y), swing_angle, 0,
                            event.text == "sweep" ? 8 : 6});
      break;
    }
    case PresentationEventType::DamageApplied: {
      fx.effects.push_back({EffectFx::Kind::Impact, ex, ey, 0.0, 0, 4});
      if (!to_player && event.combo_step == 3) {
        EffectFx finisher;
        finisher.kind = EffectFx::Kind::ComboFinisher;
        finisher.wx = ex;
        finisher.wy = ey;
        finisher.ttl = phase_a::kComboFinisherTtlTicks;
        finisher.finisher = true;
        fx.effects.push_back(finisher);
      }
      EffectFx flash;
      flash.kind = EffectFx::Kind::TargetFlash;
      flash.wx = ex;
      flash.wy = ey;
      flash.ttl = event.critical ? phase_a::kCriticalFlashTtlTicks : 4;
      flash.damage_to_player = to_player;
      flash.critical = event.critical;
      flash.finisher = event.combo_step == 3;
      flash.style = event.style;
      fx.effects.push_back(flash);
      EffectFx number;
      number.kind = EffectFx::Kind::DamageNumber;
      number.wx = ex;
      number.wy = ey;
      number.ttl = event.critical ? phase_a::kCriticalNumberTtlTicks : 12;
      number.value = event.value;
      number.damage_to_player = to_player;
      number.critical = event.critical;
      number.finisher = event.combo_step == 3;
      number.style = event.style;
      fx.effects.push_back(number);
      if (to_player) {
        fx.screen_pulse_ticks = 3;
        if (!event.actor_id.empty())
          fx.monster_strikes[event.actor_id] = now_tick;
      }
      break;
    }
    case PresentationEventType::HealingApplied: {
      EffectFx pulse;
      pulse.kind = EffectFx::Kind::SupportMend;
      pulse.wx = ex;
      pulse.wy = ey;
      pulse.ttl = phase_a::kSupportMendTtlTicks;
      pulse.value = event.value;
      pulse.healing = true;
      fx.effects.push_back(pulse);
      EffectFx number = pulse;
      number.kind = EffectFx::Kind::DamageNumber;
      fx.effects.push_back(number);
      break;
    }
    case PresentationEventType::Telegraph: {
      ActiveTelegraph telegraph;
      telegraph.actor_id = event.actor_id;
      telegraph.action = event.text.find("volley") != std::string::npos
                             ? "volley"
                             : event.text.find("sweep") != std::string::npos
                                   ? "sweep" : "thrust";
      telegraph.position = event.has_position
          ? verdigris::Vec2{static_cast<int>(std::lround(protocol_to_world(event.x))),
                            static_cast<int>(std::lround(protocol_to_world(event.y)))}
          : event_anchor(world, fx, event, false);
      // TASK-0122 Phase A: same inversion removal as monster sync. Without an
      // authoritative telegraph facing on the wire (radius/position wire work
      // is deferred), the warning keeps its neutral default instead of a
      // client-only inverted copy of the player's aim.
      telegraph.start_tick = now_tick;
      telegraph.windup_ticks = std::max(1, event.value > 20 ? event.value / 50 : event.value);
      telegraph.radius_tiles = std::max(1, event.radius);
      fx.telegraphs[event.actor_id.empty() ? "foe" : event.actor_id] = std::move(telegraph);
      break;
    }
    case PresentationEventType::ActorDied:
    case PresentationEventType::ScionDied:
      fx.telegraphs.erase(event.actor_id);
      fx.monster_strikes.erase(event.actor_id);
      if (event.type == PresentationEventType::ScionDied) fx.telegraphs.clear();
      fx.last_death_pos = at;
      fx.effects.push_back({EffectFx::Kind::DeathRing, ex, ey, 0.0, 0, 12});
      fx.effects.push_back({EffectFx::Kind::Dust, ex, ey, 0.7, 0, 10});
      break;
    case PresentationEventType::ScionLost:
      // TASK-0122 Phase A: a long, distinct loss beat anchored on the player.
      // Distinct from death rings in color, shape, and lifetime; it decorates
      // the already-resolved loss and mutates nothing authoritative.
      fx.telegraphs.clear();
      fx.effects.push_back(
          {EffectFx::Kind::ScionLostBeat, static_cast<double>(world.player.position.x),
           static_cast<double>(world.player.position.y), 0.0, 0,
           phase_a::kScionLostRingTtlTicks});
      fx.screen_pulse_ticks = phase_a::kScionLostPulseTicks;
      break;
    case PresentationEventType::BuffApplied:
      if (event.text.empty() || event.text == "war-cry")
        fx.effects.push_back({EffectFx::Kind::WarCryAura, ex, ey, 0.0, 0, 14});
      break;
    case PresentationEventType::BuffExpired:
      // TASK-0122 Phase A: the war-cry end contract beat — an imploding,
      // dimmed-gold ring at its anchor (the empowered self), clearly unlike
      // the expanding bright aura shown on BuffApplied.
      if (event.text.empty() || event.text == "war-cry")
        fx.effects.push_back({EffectFx::Kind::WarCryFade, ex, ey, 0.0, 0,
                              phase_a::kWarcryFadeTtlTicks});
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
      // Server messages carry the story: quest dialogue, trade receipts,
      // extraction flavor. Surface them as a HUD toast — longer lines get
      // longer to read — instead of dropping them on the floor.
      if (!event.text.empty()) {
        fx.hint = event.text;
        fx.hint_ticks = std::min<int>(400, 100 + static_cast<int>(event.text.size()) * 2);
      }
      break;
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
      case PresentationEventType::HealingApplied:
        line = "Mended " + std::to_string(event.value);
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
      // TASK-0122 Phase A readable beat lines.
      case PresentationEventType::ScionLost:
        line = "scion lost";
        break;
      case PresentationEventType::BuffExpired:
        if (event.text.empty() || event.text == "war-cry") line = "war cry faded";
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

void detect_monster_spawns(PresentationFx& fx, const WorldView& world,
                           std::uint64_t now_tick) {
  (void)now_tick;
  // TASK-0122 Phase A: one materialization beat per never-before-seen living
  // foe, in the deterministic snapshot order. This reads the authoritative
  // snapshot only — it never creates, moves, or damages an actor.
  for (const auto& monster : world.monsters) {
    if (!monster.alive) continue;
    if (!fx.known_monsters.insert(monster.id).second) continue;
    EffectFx spawn;
    spawn.kind = EffectFx::Kind::Materialize;
    spawn.wx = static_cast<double>(monster.position.x);
    spawn.wy = static_cast<double>(monster.position.y);
    spawn.ttl = phase_a::kMaterializeTtlTicks;
    fx.effects.push_back(std::move(spawn));
  }
}

void record_world_ops(render::List& rl, const WorldView& world, const PresentationFx& fx,
                      const camera2d::Camera& camera, int width, int height) {
  const camera2d::Screen screen{width, height};
  auto at = [&](double wx, double wy) { return camera2d::project(camera, screen, wx, wy); };
  if (world.has_extraction) {
    const auto pad = at(world.extraction.x, world.extraction.y);
    rl.push_back({render::Op::Extraction, static_cast<double>(pad.x),
                  static_cast<double>(pad.y), 0.0, 0, "stairs-up"});
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
  for (const auto& npc : world.npcs) {
    const auto base = at(npc.position.x, npc.position.y);
    rl.push_back({render::Op::Npc, static_cast<double>(base.x),
                  static_cast<double>(base.y), 0.0, npc.id, npc.name});
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
      case EffectFx::Kind::DamageNumber: {
        std::string damage_label =
            effect.healing ? "healing" :
                (effect.damage_to_player ? "player" : "monster");
        if (effect.finisher)
          damage_label = std::string(effect.critical ? "critical-finisher:" : "finisher:") +
                         (effect.style.empty() ? "slash" : effect.style);
        else if (effect.critical)
          damage_label = std::string(phase_a::kCriticalDamageLabel) + ":" +
                         (effect.style.empty() ? "slash" : effect.style);
        rl.push_back({render::Op::Damage, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, effect.value, damage_label});
        break;
      }
      case EffectFx::Kind::SupportMend:
        rl.push_back({render::Op::WarCry, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, effect.value,
                      phase_a::kSupportMendLabel});
        break;
      case EffectFx::Kind::DeathRing:
        rl.push_back({render::Op::Death, static_cast<double>(base.x),
                      static_cast<double>(base.y)});
        break;
      case EffectFx::Kind::TargetFlash:
        rl.push_back({render::Op::TargetFlash, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, 0,
                      effect.finisher ? "finisher" :
                          (effect.damage_to_player ? "player" : "monster")});
        break;
      case EffectFx::Kind::ComboFinisher:
        rl.push_back({render::Op::Impact, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, 0,
                      phase_a::kComboFinisherLabel});
        break;
      case EffectFx::Kind::Materialize:
        // TASK-0122 Phase A: recorded on the existing vocabulary with a
        // distinct label so the render list stays honest without new ops.
        rl.push_back({render::Op::TargetFlash, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, 0, phase_a::kSpawnRenderLabel});
        break;
      case EffectFx::Kind::WarCryFade:
        rl.push_back({render::Op::WarCry, static_cast<double>(base.x),
                      static_cast<double>(base.y), 0.0, 0, phase_a::kWarcryFadeLabel});
        break;
      case EffectFx::Kind::ScionLostBeat:
        rl.push_back({render::Op::ScreenPulse, 0.0, 0.0, 0.0, 0,
                      phase_a::kScionLostLabel});
        break;
      default:
        break;
    }
  }
  if (fx.screen_pulse_ticks > 0)
    rl.push_back({render::Op::ScreenPulse, 0.0, 0.0, 0.0, 0, "player-damage"});
}

}  // namespace verdigris::client
