#include "verdigris/networking.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

using verdigris::networking::Envelope;
using verdigris::networking::JsonValue;
using verdigris::networking::ProtocolSession;
using verdigris::networking::emit_envelope;
using verdigris::networking::parse_envelope;

namespace {
void check(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_envelope_round_trip() {
  Envelope source{"player:login", JsonValue::Object{{"useGuestAccount", true}, {"guestId", "roundtrip-guest"}}};
  const auto wire = emit_envelope(source);
  Envelope decoded;
  std::string error;
  check(parse_envelope(wire, decoded, &error), error.c_str());
  check(decoded.event == source.event, "event survives envelope round-trip");
  check(decoded.data["useGuestAccount"].boolean().value_or(false), "boolean payload survives round-trip");
  check(decoded.data["guestId"].string() && *decoded.data["guestId"].string() == "roundtrip-guest", "string payload survives round-trip");
  check(!parse_envelope("{\"event\":\"dev:state\",\"data\":[]}", decoded, &error), "array payload is rejected");
}

void test_session_lifecycle() {
  ProtocolSession session("guest-lifecycle", "socket-a", 7, true);
  Envelope login{"player:login", JsonValue::Object{{"useGuestAccount", true}, {"quickGuest", true}}};
  std::string login_wire;
  session.handle(login, [&](const Envelope& response) { login_wire = emit_envelope(response); });
  Envelope response;
  check(parse_envelope(login_wire, response), "session emits login envelope");
  check(response.event == "player:login", "session emits player:login");
  check(response.data["quickStart"].boolean().value_or(false), "quick guest marks quickStart");

  bool saw_zone_transition = false;
  session.handle(Envelope{"world:zone:enter", JsonValue::Object{{"nodeId", "tin:1:0"}}}, [&](const Envelope& event) {
    if (event.event == "world:scene:transition") saw_zone_transition = true;
  });
  check(saw_zone_transition, "zone enter emits scene transition");
  std::string state_wire;
  session.handle(Envelope{"dev:state", JsonValue::Object{{"requestId", "state-1"}}}, [&](const Envelope& event) {
    state_wire = emit_envelope(event);
  });
  check(parse_envelope(state_wire, response), "session emits state envelope");
  check(response.data["requestId"].string() && *response.data["requestId"].string() == "state-1", "state request id is echoed");
  check(response.data["state"]["sceneType"].string() && *response.data["state"]["sceneType"].string() == "instance", "state reports active instance");
  check(response.data["state"]["monsters"].array() && !response.data["state"]["monsters"].array()->empty(), "instance state has a monster");
  check(response.data["state"]["xp"].object() != nullptr,
        "state publishes authoritative combat XP");
  check(response.data["state"]["xp"]["next"].number().value_or(0.0) >
            response.data["state"]["xp"]["floor"].number().value_or(0.0),
        "combat XP publishes a valid current-level span");

  session.handle(Envelope{"dev:give", JsonValue::Object{{"itemId", "garnet-amulet"}, {"qty", 1}}}, [](const Envelope&) {});
  session.handle(Envelope{"dev:state", JsonValue::Object{{"requestId", "state-2"}}}, [&](const Envelope& event) { state_wire = emit_envelope(event); });
  check(parse_envelope(state_wire, response), "state after grant parses");
  bool granted = false;
  if (const auto* inventory = response.data["state"]["inventory"].array()) {
    for (const auto& entry : *inventory) {
      if (entry["id"].string() && *entry["id"].string() == "garnet-amulet") granted = true;
    }
  }
  check(granted, "dev give appears in inventory");

  session.replace_socket("socket-b");
  check(session.state_payload("state-3").find("socket-b") != std::string::npos, "replacement binds the new socket");
}

double state_axis(const JsonValue& state, const char* axis) {
  return state["state"][axis].number().value_or(0.0);
}

JsonValue request_state(ProtocolSession& session, const std::string& request_id) {
  std::string wire;
  session.handle(Envelope{"dev:state", JsonValue::Object{{"requestId", request_id}}},
                 [&](const Envelope& event) { wire = emit_envelope(event); });
  Envelope response;
  std::string error;
  check(parse_envelope(wire, response, &error), error.c_str());
  return response.data;
}

void test_continuous_movement() {
  ProtocolSession session("guest-movement", "socket-m", 11, false);
  const auto start = request_state(session, "m-0");
  const double start_y = state_axis(start, "y");
  const double start_x = state_axis(start, "x");

  // One held-key sample advances exactly 1/3 tile and stays fractional.
  session.handle(Envelope{"player:move", JsonValue::Object{{"direction", "down"}}}, [](const Envelope&) {});
  const auto after_one = request_state(session, "m-1");
  const double one_y = state_axis(after_one, "y");
  check(one_y > start_y, "one sample moves down");
  check(std::abs(one_y - std::round(one_y)) > 1e-9, "position stays fractional mid-tile");
  check(std::abs(one_y - (start_y + 1.0 / 3.0)) < 0.01, "sample distance is one third tile");

  // Eight more samples complete three tiles of travel.
  for (int i = 0; i < 8; ++i) {
    session.handle(Envelope{"player:move", JsonValue::Object{{"direction", "down"}}}, [](const Envelope&) {});
  }
  const auto after_nine = request_state(session, "m-2");
  check(std::abs(state_axis(after_nine, "y") - (start_y + 3.0)) < 0.01, "nine samples travel three tiles");

  // A movement broadcast carries the player payload plus the step metadata.
  std::optional<Envelope> movement;
  session.handle(Envelope{"player:move", JsonValue::Object{{"direction", "right"}}},
                 [&](const Envelope& event) { movement = event; });
  check(movement && movement->event == "player:movement", "applied sample broadcasts player:movement");
  check(movement && movement->meta && (*movement->meta)["sequence"].number().value_or(0) > 0,
        "movement step metadata sequences");
  check(state_axis(request_state(session, "m-3"), "x") > start_x, "right sample moves east");

  // Unknown directions are ignored without moving.
  const auto before_bad = request_state(session, "m-4");
  session.handle(Envelope{"player:move", JsonValue::Object{{"direction", "sideways"}}}, [](const Envelope&) {});
  const auto after_bad = request_state(session, "m-5");
  check(state_axis(after_bad, "x") == state_axis(before_bad, "x")
        && state_axis(after_bad, "y") == state_axis(before_bad, "y"), "unknown direction is a no-op");
}

void test_instance_entry_and_stairs() {
  ProtocolSession session("guest-zones", "socket-z", 13, false);
  const auto town = request_state(session, "z-0");
  check(town["state"]["sceneType"].string() && *town["state"]["sceneType"].string() == "town", "starts in town");

  std::optional<Envelope> transition;
  session.handle(Envelope{"instance:enterSolo", JsonValue::Object{{"template", "dungeon"}, {"layout", "warren"}}},
                 [&](const Envelope& event) {
                   if (event.event == "party:scene:transition") transition = event;
                 });
  check(transition && transition->event == "party:scene:transition", "solo entry emits a scene transition");
  const auto& scene = (*transition).data["scene"];
  check(scene["name"].string() && *scene["name"].string() == "The Old Barrow", "zone display name comes from the adventure table");
  check((*transition).data["playerState"]["uuid"].string() != nullptr, "transition carries playerState");

  const auto in_zone = request_state(session, "z-1");
  check(in_zone["state"]["sceneType"].string() && *in_zone["state"]["sceneType"].string() == "instance", "state reports instance");
  check(in_zone["state"]["sceneMetadata"]["layout"].string()
        && *in_zone["state"]["sceneMetadata"]["layout"].string() == "warren", "layout applied to metadata");
  check(in_zone["state"]["sceneMetadata"]["stairsUp"].is_object()
        && in_zone["state"]["sceneMetadata"]["stairsDown"].is_object(), "both stairs exist");
  check(in_zone["state"]["monsters"].array() && in_zone["state"]["monsters"].array()->size() >= 15,
        "instance is populated");

  // Mid-walk entry must not bounce: step first, then enter, then confirm the
  // session stays in the instance with no return-to-surface message.
  ProtocolSession walker("guest-midwalk", "socket-w", 17, false);
  std::vector<std::string> messages;
  auto capture = [&](const Envelope& event) {
    if (event.event == "game:send:message") {
      if (const auto* text = event.data["text"].string()) messages.push_back(*text);
    }
  };
  walker.handle(Envelope{"player:move", JsonValue::Object{{"direction", "right"}}}, capture);
  walker.handle(Envelope{"player:move", JsonValue::Object{{"direction", "right"}}}, capture);
  walker.handle(Envelope{"instance:enterSolo", JsonValue::Object{{"template", "crypt"}, {"layout", "gauntlet"}}}, capture);
  const auto walked = request_state(walker, "w-1");
  check(walked["state"]["sceneType"].string() && *walked["state"]["sceneType"].string() == "instance",
        "mid-walk entry stays in the instance");
  check(messages.empty(), "no bounce message on entry");

  // Teleporting onto the entry stairs returns to the pre-entry town tile.
  const double stairs_x = in_zone["state"]["sceneMetadata"]["stairsUp"]["x"].number().value_or(0);
  const double stairs_y = in_zone["state"]["sceneMetadata"]["stairsUp"]["y"].number().value_or(0);
  session.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", stairs_x}, {"y", stairs_y}}}, capture);
  const auto back = request_state(session, "z-2");
  check(back["state"]["sceneType"].string() && *back["state"]["sceneType"].string() == "town",
        "entry stairs return to town");
  check(state_axis(back, "x") == state_axis(town, "x") && state_axis(back, "y") == state_axis(town, "y"),
        "pre-entry position restored");
  check(std::find(messages.begin(), messages.end(), "The party returns to the surface.") != messages.end(),
        "stair return announces the surface");
}

void test_n3_combat_rules_and_wire_events() {
  ProtocolSession session("guest-n3-rules", "socket-n3", 101, false);
  session.handle(Envelope{"instance:enterSolo", JsonValue::Object{{"template", "marsh"}, {"layout", "clearings"}}}, [](const Envelope&) {});
  const auto marsh = request_state(session, "n3-marsh");
  const auto* monsters = marsh["state"]["monsters"].array();
  check(monsters && monsters->size() >= 20, "N3 marsh has the authored pack population");
  bool rare = false;
  bool empowered = false;
  for (const auto& value : *monsters) {
    if (value["rarity"].string() && *value["rarity"].string() == "rare"
        && value["modifiers"].array() && value["modifiers"].array()->size() == 1) rare = true;
    if (value["state"]["effects"]["aura"].is_object()) empowered = true;
  }
  check(rare, "N3 rare exposes one named modifier");
  check(empowered, "N3 buffer aura exposes Empowered state");

  ProtocolSession boss("guest-n3-boss", "socket-boss", 103, false);
  boss.handle(Envelope{"instance:enterSolo", JsonValue::Object{{"template", "dungeon"}, {"layout", "warren"}}}, [](const Envelope&) {});
  const auto state = request_state(boss, "n3-boss");
  const auto* actors = state["state"]["monsters"].array();
  check(actors != nullptr, "N3 boss snapshot has monsters");
  const JsonValue* elite = nullptr;
  for (const auto& value : *actors) if (value["rarity"].string() && *value["rarity"].string() == "elite") elite = &value;
  check(elite && elite->operator[]("name").string() && *elite->operator[]("name").string() == "Warden of the Deep", "N3 names the Old Barrow boss");
  const int x = static_cast<int>(elite->operator[]("x").number().value_or(0));
  const int y = static_cast<int>(elite->operator[]("y").number().value_or(0));
  bool telegraphed = false;
  boss.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", x + 1}, {"y", y}}}, [&](const Envelope& event) {
    if (event.event == "monster:telegraph" && event.data["skillId"].string()
        && *event.data["skillId"].string() == "boss:ground-slam") {
      telegraphed = event.data["radius"].number().value_or(0) >= 2
        && event.data["durationMs"].number().value_or(0) >= 800;
    }
  });
  check(telegraphed, "N3 boss emits a readable ground-slam telegraph");
}

std::string direction_toward(int player_x, int player_y, int target_x, int target_y) {
  if (std::abs(target_x-player_x)>=std::abs(target_y-player_y))
    return target_x<player_x?"left":"right";
  return target_y<player_y?"up":"down";
}

void test_authoritative_remote_skill_actions() {
  // The native client sends `skill` (not legacy `skillId`). A melee whiff at
  // thrust-only range must not spend cooldown; the following thrust resolves
  // once, spends its resource once, and retains its exact wire identity.
  ProtocolSession thrust("guest-skills-thrust","socket-skills-thrust",211,false);
  thrust.handle(Envelope{"instance:enterSolo",JsonValue::Object{{"template","dungeon"},{"layout","clearings"}}},[](const Envelope&){});
  const auto thrust_state=request_state(thrust,"skills-thrust-0");
  const auto* thrust_monsters=thrust_state["state"]["monsters"].array();
  check(thrust_monsters&&!thrust_monsters->empty(),"skills: thrust trial has a target");
  const auto& thrust_target=thrust_monsters->front();
  const int tx=static_cast<int>(thrust_target["x"].number().value_or(0));
  const int ty=static_cast<int>(thrust_target["y"].number().value_or(0));
  const int px=tx+4<39?tx+4:tx-4;
  const std::string aim=direction_toward(px,ty,tx,ty);
  const std::string target_uuid=thrust_target["uuid"].string()?*thrust_target["uuid"].string():std::string();
  thrust.handle(Envelope{"dev:teleport",JsonValue::Object{{"x",px},{"y",ty}}},[](const Envelope&){});
  thrust.handle(Envelope{"dev:monster:reset",JsonValue::Object{{"monsterUuid",target_uuid},{"maxHealth",1000}}},[](const Envelope&){});
  int melee_hits=0;
  thrust.handle(Envelope{"player:skill:trigger",JsonValue::Object{{"skill","melee"},{"direction",aim}}},[&](const Envelope& event){
    if(event.event=="combat:hit"&&event.data["targetType"].string()&&*event.data["targetType"].string()=="monster") ++melee_hits;
  });
  check(melee_hits==0,"skills: melee cannot hit from thrust-only range");
  int thrust_hits=0;
  bool thrust_named=false;
  int resource_after_thrust=-1;
  auto collect_thrust=[&](const Envelope& event){
    if(event.event=="combat:hit"&&event.data["targetType"].string()&&*event.data["targetType"].string()=="monster") {
      ++thrust_hits;
      thrust_named=event.data["skillId"].string()&&*event.data["skillId"].string()=="thrust";
    }
    if(event.event=="player:combat-state")
      resource_after_thrust=static_cast<int>(event.data["resource"].number().value_or(-1));
  };
  thrust.handle(Envelope{"player:skill:trigger",JsonValue::Object{{"skill","thrust"},{"direction",aim}}},collect_thrust);
  check(thrust_hits==1&&thrust_named&&resource_after_thrust==40,
        "skills: thrust resolves once with exact identity and cost");
  thrust.handle(Envelope{"player:skill:trigger",JsonValue::Object{{"skill","thrust"},{"direction",aim}}},collect_thrust);
  check(thrust_hits==1&&resource_after_thrust==40,
        "skills: input spam cannot reset cooldown or double-spend resource");

  // Find a deterministic generated pack with two bodies in one radius and
  // prove Sweep produces separate authoritative hit events for both.
  std::unique_ptr<ProtocolSession> sweep;
  int sweep_x=0,sweep_y=0;
  std::vector<std::string> sweep_targets;
  for(std::uint64_t seed=300;seed<340&&!sweep;++seed) {
    auto candidate=std::make_unique<ProtocolSession>(
        "guest-skills-sweep-"+std::to_string(seed),"socket-skills-sweep",seed,false);
    candidate->handle(Envelope{"instance:enterSolo",JsonValue::Object{{"template","grove"},{"layout","clearings"}}},[](const Envelope&){});
    const auto state=request_state(*candidate,"skills-sweep-scan");
    const auto* monsters=state["state"]["monsters"].array();
    if(!monsters) continue;
    for(int y=1;y<39&&!sweep;++y) for(int x=1;x<39&&!sweep;++x) {
      std::vector<std::string> nearby;
      for(const auto& monster:*monsters) {
        const int mx=static_cast<int>(monster["x"].number().value_or(0));
        const int my=static_cast<int>(monster["y"].number().value_or(0));
        if(std::abs(mx-x)+std::abs(my-y)<=3&&monster["uuid"].string())
          nearby.push_back(*monster["uuid"].string());
      }
      if(nearby.size()>=2) {
        sweep=std::move(candidate); sweep_x=x; sweep_y=y;
        sweep_targets=std::move(nearby);
      }
    }
  }
  check(sweep!=nullptr,"skills: generated pack exposes a sweep radius");
  sweep->handle(Envelope{"dev:teleport",JsonValue::Object{{"x",sweep_x},{"y",sweep_y}}},[](const Envelope&){});
  for(const auto& uuid:sweep_targets)
    sweep->handle(Envelope{"dev:monster:reset",JsonValue::Object{{"monsterUuid",uuid},{"maxHealth",1000}}},[](const Envelope&){});
  std::set<std::string> swept;
  int sweep_resource=-1;
  sweep->handle(Envelope{"player:skill:trigger",JsonValue::Object{{"skill","sweep"},{"direction","right"}}},[&](const Envelope& event){
    if(event.event=="combat:hit"&&event.data["skillId"].string()&&*event.data["skillId"].string()=="sweep"&&event.data["targetId"].string())
      swept.insert(*event.data["targetId"].string());
    if(event.event=="player:combat-state") sweep_resource=static_cast<int>(event.data["resource"].number().value_or(-1));
  });
  check(swept.size()>=2&&sweep_resource==35,
        "skills: sweep hits every nearby body and spends one cast cost");

  // War Cry is a timed self buff, not a disguised attack. Its immediate
  // combat-state packet makes the HUD truthful before the next state poll.
  ProtocolSession cry("guest-skills-cry","socket-skills-cry",401,false);
  cry.handle(Envelope{"instance:enterSolo",JsonValue::Object{{"template","dungeon"},{"layout","clearings"}}},[](const Envelope&){});
  bool cry_active=false;
  bool cry_dealt_damage=false;
  int cry_resource=-1;
  cry.handle(Envelope{"player:skill:trigger",JsonValue::Object{{"skill","war-cry"},{"direction","right"}}},[&](const Envelope& event){
    if(event.event=="player:skill:effect") cry_active=event.data["active"].boolean().value_or(false);
    if(event.event=="combat:hit"&&event.data["targetType"].string()&&*event.data["targetType"].string()=="monster") cry_dealt_damage=true;
    if(event.event=="player:combat-state") cry_resource=static_cast<int>(event.data["resource"].number().value_or(-1));
  });
  check(cry_active&&!cry_dealt_damage&&cry_resource==30,
        "skills: War Cry buffs without dealing damage and spends its cost");
  const auto cry_state=request_state(cry,"skills-cry-target");
  const auto* cry_monsters=cry_state["state"]["monsters"].array();
  check(cry_monsters&&!cry_monsters->empty(),"skills: War Cry trial has a target");
  const auto& cry_target=cry_monsters->front();
  const int cx=static_cast<int>(cry_target["x"].number().value_or(0));
  const int cy=static_cast<int>(cry_target["y"].number().value_or(0));
  const int cpx=cx+1<39?cx+1:cx-1;
  const std::string caim=direction_toward(cpx,cy,cx,cy);
  const std::string cry_uuid=cry_target["uuid"].string()?*cry_target["uuid"].string():std::string();
  cry.handle(Envelope{"dev:teleport",JsonValue::Object{{"x",cpx},{"y",cy}}},[](const Envelope&){});
  cry.handle(Envelope{"dev:monster:reset",JsonValue::Object{{"monsterUuid",cry_uuid},{"maxHealth",1000}}},[](const Envelope&){});
  int empowered_damage=0;
  cry.handle(Envelope{"player:skill:trigger",JsonValue::Object{{"skill","melee"},{"direction",caim}}},[&](const Envelope& event){
    if(event.event=="combat:hit"&&event.data["targetType"].string()&&*event.data["targetType"].string()=="monster")
      empowered_damage=static_cast<int>(event.data["amount"].number().value_or(0));
  });

  ProtocolSession baseline("guest-skills-baseline","socket-skills-baseline",401,false);
  baseline.handle(Envelope{"instance:enterSolo",JsonValue::Object{{"template","dungeon"},{"layout","clearings"}}},[](const Envelope&){});
  const auto baseline_state=request_state(baseline,"skills-baseline-target");
  const auto& baseline_target=baseline_state["state"]["monsters"].array()->front();
  const std::string baseline_uuid=baseline_target["uuid"].string()?*baseline_target["uuid"].string():std::string();
  baseline.handle(Envelope{"dev:teleport",JsonValue::Object{{"x",cpx},{"y",cy}}},[](const Envelope&){});
  baseline.handle(Envelope{"dev:monster:reset",JsonValue::Object{{"monsterUuid",baseline_uuid},{"maxHealth",1000}}},[](const Envelope&){});
  int baseline_damage=0;
  baseline.handle(Envelope{"player:skill:trigger",JsonValue::Object{{"skill","melee"},{"direction",caim}}},[&](const Envelope& event){
    if(event.event=="combat:hit"&&event.data["targetType"].string()&&*event.data["targetType"].string()=="monster")
      baseline_damage=static_cast<int>(event.data["amount"].number().value_or(0));
  });
  check(empowered_damage==baseline_damage+
            verdigris::presentation_constants::kWarCryAttackBonus,
        "skills: War Cry's authoritative bonus changes subsequent damage");

  // Dash must be authoritative movement and can never inherit the attack
  // handler merely because it shares the use-action wire verb.
  ProtocolSession dash("guest-skills-dash","socket-skills-dash",501,false);
  dash.handle(Envelope{"instance:enterSolo",JsonValue::Object{{"template","dungeon"},{"layout","warren"}}},[](const Envelope&){});
  const auto dash_before=request_state(dash,"skills-dash-before");
  bool dash_moved=false;
  bool dash_hit=false;
  dash.handle(Envelope{"player:skill:trigger",JsonValue::Object{{"skill","dash"},{"direction","right"}}},[&](const Envelope& event){
    if(event.event=="player:movement") dash_moved=true;
    if(event.event=="combat:hit"&&event.data["targetType"].string()&&*event.data["targetType"].string()=="monster") dash_hit=true;
  });
  const auto dash_after=request_state(dash,"skills-dash-after");
  check(dash_moved&&!dash_hit&&state_axis(dash_after,"x")-state_axis(dash_before,"x")>1.0,
        "skills: dash moves through collision rules and never attacks");
}

bool ground_item_has_fields(const JsonValue& item) {
  return item["uuid"].string() && !item["uuid"].string()->empty()
      && item["id"].string() && !item["id"].string()->empty()
      && item["name"].string() && !item["name"].string()->empty()
      && item["x"].number().has_value() && item["y"].number().has_value();
}

std::string inventory_uuid_for(const JsonValue& state, const char* item_id) {
  if (const auto* inventory = state["state"]["inventory"].array()) {
    for (const auto& entry : *inventory) {
      if (entry["id"].string() && *entry["id"].string() == item_id && entry["uuid"].string()) {
        return *entry["uuid"].string();
      }
    }
  }
  return {};
}

std::string inventory_map_uuid(const JsonValue& state) {
  if (const auto* inventory = state["state"]["inventory"].array()) {
    for (const auto& entry : *inventory)
      if (entry["expeditionMap"].object() && entry["uuid"].string())
        return *entry["uuid"].string();
  }
  return {};
}

int inventory_count(const JsonValue& state) {
  if (const auto* inventory = state["state"]["inventory"].array()) {
    return static_cast<int>(inventory->size());
  }
  return 0;
}

const JsonValue::Array* ground_list_from_change(const Envelope& event) {
  return event.data["data"].array();
}

void test_gate_a_ground_login_and_kill_loot() {
  ProtocolSession session("guest-0063-ground", "socket-g", 19, false);
  std::string login_wire;
  session.handle(Envelope{"player:login", JsonValue::Object{{"useGuestAccount", true}}},
                 [&](const Envelope& event) {
                   if (event.event == "player:login") login_wire = emit_envelope(event);
                 });
  Envelope login;
  check(parse_envelope(login_wire, login), "login parses");
  check(login.data["droppedItems"].is_array(), "login includes droppedItems");

  std::optional<Envelope> change;
  std::optional<Envelope> dropped;
  session.handle(Envelope{"instance:enterSolo", JsonValue::Object{{"template", "dungeon"}, {"layout", "warren"}}},
                 [&](const Envelope& event) {
                   if (event.event == "item:change") change = event;
                   if (event.event == "world:itemDropped") dropped = event;
                 });
  check(change.has_value() && dropped.has_value(),
        "floor treasure emits item:change and world:itemDropped");
  const auto* floor = ground_list_from_change(*change);
  check(floor && !floor->empty(), "item:change carries the floor ground list");
  check(ground_item_has_fields((*floor)[0]), "ground envelope has uuid, id, name, x, y");

  session.handle(Envelope{"player:login", JsonValue::Object{{"useGuestAccount", true}}},
                 [&](const Envelope& event) {
                   if (event.event == "player:login") login_wire = emit_envelope(event);
                 });
  check(parse_envelope(login_wire, login), "instance re-login parses");
  const auto* login_ground = login.data["droppedItems"].array();
  check(login_ground && !login_ground->empty(), "login snapshot includes instance ground items");
  check(ground_item_has_fields((*login_ground)[0]), "login ground items have uuid, id, name, x, y");
  check(login.data["scene"]["droppedItems"].is_array()
            && login.data["scene"]["droppedItems"].array()->size() == login_ground->size(),
        "scene.droppedItems matches login droppedItems");

  const auto state = request_state(session, "g-1");
  check(state["state"]["groundItems"].array()
            && state["state"]["groundItems"].array()->size() == login_ground->size(),
        "dev:state groundItems matches login");

  bool drop_change = false;
  session.handle(Envelope{"dev:drop", JsonValue::Object{{"itemId", "coins"}}}, [&](const Envelope& event) {
    if (event.event != "item:change") return;
    if (const auto* items = ground_list_from_change(event)) {
      for (const auto& item : *items) {
        if (item["id"].string() && *item["id"].string() == "coins" && ground_item_has_fields(item)) {
          drop_change = true;
        }
      }
    }
  });
  check(drop_change, "dev:drop emits item:change with uuid, id, name, x, y");

  session.handle(Envelope{"dev:setlevel", JsonValue::Object{{"level", 40}}}, [](const Envelope&) {});
  const auto pack = request_state(session, "g-kill");
  const JsonValue* target = nullptr;
  if (const auto* monsters = pack["state"]["monsters"].array()) {
    for (const auto& monster : *monsters) {
      if (monster["rarity"].string() && *monster["rarity"].string() != "elite") {
        target = &monster;
        break;
      }
    }
  }
  check(target != nullptr, "found a non-elite for kill-loot");
  const int mx = static_cast<int>(target->operator[]("x").number().value_or(0));
  const int my = static_cast<int>(target->operator[]("y").number().value_or(0));
  const std::string target_uuid = target->operator[]("uuid").string()
                                      ? *target->operator[]("uuid").string()
                                      : std::string();
  session.handle(Envelope{"dev:monster:reset",
                          JsonValue::Object{{"monsterUuid", target_uuid},
                                            {"maxHealth", 1}}},
                 [](const Envelope&) {});
  session.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", mx + 1.0}, {"y", static_cast<double>(my)}}},
                 [](const Envelope&) {});
  bool kill_loot = false;
  for (int swing = 0; swing < 40 && !kill_loot; ++swing) {
    session.handle(Envelope{"dev:forcecritical", JsonValue::Object{}}, [](const Envelope&) {});
    session.handle(Envelope{"player:skill:trigger", JsonValue::Object{{"direction", "left"}}},
                   [&](const Envelope& event) {
                     if (event.event != "item:change") return;
                     if (const auto* items = ground_list_from_change(event)) {
                       for (const auto& item : *items) {
                         if (item["id"].string() && *item["id"].string() == "coins"
                             && ground_item_has_fields(item)) {
                           kill_loot = true;
                         }
                       }
                     }
                   });
  }
  check(kill_loot, "kill loot emits item:change with coin drop fields");
}

void test_gate_a_extract_and_stairs() {
  ProtocolSession extract_session("guest-0063-extract", "socket-ex", 23, false);
  extract_session.handle(Envelope{"dev:give", JsonValue::Object{{"itemId", "garnet-amulet"}, {"qty", 1}}},
                         [](const Envelope&) {});
  extract_session.handle(Envelope{"instance:enterSolo",
                                  JsonValue::Object{{"template", "dungeon"}, {"layout", "warren"}}},
                         [](const Envelope&) {});
  check(!inventory_uuid_for(request_state(extract_session, "ex-0"), "garnet-amulet").empty(),
        "amulet is carried before extract");

  std::optional<Envelope> summary;
  extract_session.handle(Envelope{"player:extract", JsonValue::Object{}}, [&](const Envelope& event) {
    if (event.event == "player:extract") summary = event;
  });
  check(summary.has_value(), "player:extract emits a bank summary");
  check(summary->data["items"].number().value_or(0) >= 1, "extract banks at least the amulet");
  const auto after = request_state(extract_session, "ex-1");
  check(after["state"]["sceneType"].string() && *after["state"]["sceneType"].string() == "town",
        "extract returns to town");
  check(inventory_uuid_for(after, "garnet-amulet").empty(), "extract clears the amulet from the backpack");
  bool stored = false;
  if (const auto* bank = after["state"]["houseStoredItems"].array()) {
    for (const auto& item : *bank) {
      if (item["id"].string() && *item["id"].string() == "garnet-amulet") stored = true;
    }
  }
  check(stored, "extract places the amulet in the House store");

  ProtocolSession stairs("guest-0063-stairs", "socket-st", 29, false);
  stairs.handle(Envelope{"dev:give", JsonValue::Object{{"itemId", "bronze-sword"}, {"qty", 1}}},
                [](const Envelope&) {});
  stairs.handle(Envelope{"instance:enterSolo", JsonValue::Object{{"template", "dungeon"}, {"layout", "warren"}}},
                [](const Envelope&) {});
  const auto in_zone = request_state(stairs, "st-1");
  const double stairs_x = in_zone["state"]["sceneMetadata"]["stairsUp"]["x"].number().value_or(0);
  const double stairs_y = in_zone["state"]["sceneMetadata"]["stairsUp"]["y"].number().value_or(0);
  std::optional<Envelope> stairs_summary;
  stairs.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", stairs_x}, {"y", stairs_y}}},
                [&](const Envelope& event) {
                  if (event.event == "player:extract") stairs_summary = event;
                });
  check(stairs_summary.has_value(), "stairs-up emits the same player:extract bank summary");
  const auto back = request_state(stairs, "st-2");
  check(back["state"]["sceneType"].string() && *back["state"]["sceneType"].string() == "town",
        "stairs-up still returns to town");
  check(inventory_uuid_for(back, "bronze-sword").empty(), "stairs-up banks carried items");
  bool sword_stored = false;
  if (const auto* bank = back["state"]["houseStoredItems"].array()) {
    for (const auto& item : *bank) {
      if (item["id"].string() && *item["id"].string() == "bronze-sword") sword_stored = true;
    }
  }
  check(sword_stored, "stairs-up and player:extract converge on the House store");
}

void test_gate_a_equip_totals_and_unknown_uuid() {
  ProtocolSession session("guest-0063-equip", "socket-eq", 31, false);
  session.handle(Envelope{"dev:give", JsonValue::Object{{"itemId", "garnet-amulet"}, {"qty", 1}}},
                 [](const Envelope&) {});
  const auto before = request_state(session, "eq-0");
  const std::string uuid = inventory_uuid_for(before, "garnet-amulet");
  check(!uuid.empty(), "granted amulet has a uuid");
  const double before_stab = before["state"]["combat"]["attack"]["stab"].number().value_or(0);

  std::optional<Envelope> equipped;
  session.handle(Envelope{"item:equip", JsonValue::Object{{"item", JsonValue::Object{{"uuid", uuid}}}}},
                 [&](const Envelope& event) {
                   if (event.event == "player:equippedAnItem") equipped = event;
                 });
  check(equipped.has_value(), "item:equip emits player:equippedAnItem");
  check(equipped->data["wear"]["necklace"].string()
            && *equipped->data["wear"]["necklace"].string() == "garnet-amulet",
        "equip response includes wear-slot state");
  check(equipped->data["combat"]["attack"]["stab"].number().value_or(0) > before_stab,
        "equip response includes derived combat totals");

  const auto worn = request_state(session, "eq-1");
  check(worn["state"]["wear"]["necklace"].string()
            && *worn["state"]["wear"]["necklace"].string() == "garnet-amulet",
        "snapshot wear matches the equip response");

  bool error = false;
  bool refresh = false;
  session.handle(Envelope{"item:equip", JsonValue::Object{{"item", JsonValue::Object{{"uuid", "missing-uuid"}}}}},
                 [&](const Envelope& event) {
                   if (event.event == "game:send:message" && event.data["text"].string()
                       && *event.data["text"].string() == "That item is no longer in your inventory.") {
                     error = true;
                   }
                   if (event.event == "core:refresh:inventory") refresh = true;
                   if (event.event == "player:equippedAnItem") equipped.reset();
                 });
  check(error && refresh, "unknown uuid emits the JS inventory error envelope");
  const auto after = request_state(session, "eq-2");
  check(after["state"]["wear"]["necklace"].string()
            && *after["state"]["wear"]["necklace"].string() == "garnet-amulet",
        "unknown uuid does not change wear");
  check(inventory_count(after) == inventory_count(worn), "unknown uuid does not change inventory");
}

void test_crossroads_social_hub_and_house_investment() {
  ProtocolSession session("guest-crossroads", "socket-crossroads", 0xc055u, false);

  JsonValue::Object house;
  house["id"] = JsonValue("house-crossroads");
  house["name"] = JsonValue("House Ashwake");
  house["treasury"] = JsonValue(0);
  house["scions"] = JsonValue(JsonValue::Array{
      JsonValue(JsonValue::Object{{"id", "scion-roadborn"},
                                  {"name", "Edda"}, {"level", 1},
                                  {"mortal", false}})});
  house["crypt"] = JsonValue(JsonValue::Array{});
  JsonValue::Object chronicle;
  chronicle["version"] = JsonValue(3);
  chronicle["houses"] = JsonValue(JsonValue::Array{JsonValue(house)});
  chronicle["activeHouseId"] = JsonValue("house-crossroads");
  chronicle["activeScionId"] = JsonValue("scion-roadborn");
  session.handle(Envelope{"player:chronicles:save",
                          JsonValue::Object{{"state", JsonValue(chronicle)}}},
                 [](const Envelope&) {});
  session.handle(Envelope{"player:chronicles:select",
                          JsonValue::Object{{"scionId", "scion-roadborn"},
                                            {"houseId", "house-crossroads"},
                                            {"scionName", "Edda"},
                                            {"mortal", false}}},
                 [](const Envelope&) {});

  auto state = request_state(session, "town-roster");
  const auto* npcs = state["state"]["npcs"].array();
  check(npcs && npcs->size() == 4,
        "crossroads: accepted owner-demo roster has exactly four townsfolk");
  bool saw_ludovicus = false;
  bool saw_selene = false;
  bool saw_rhea_services = false;
  bool saw_retired_mara = false;
  if (npcs) {
    for (const auto& npc : *npcs) {
      const std::string key = npc["key"].string() ? *npc["key"].string() : "";
      if (key == "ludovicus-weapons" && npc["role"].string() &&
          *npc["role"].string() == "weapons_tools_trainer")
        saw_ludovicus = true;
      if (key == "selene-rite" && npc["x"].number().value_or(0) == 45 &&
          npc["y"].number().value_or(0) == 108)
        saw_selene = true;
      if (key == "rhea-countinghouse" && npc["services"].array() &&
          npc["services"].array()->size() == 2)
        saw_rhea_services = true;
      if (npc["name"].string() && npc["name"].string()->find("Mara") != std::string::npos)
        saw_retired_mara = true;
    }
  }
  check(saw_ludovicus && saw_selene && saw_rhea_services && !saw_retired_mara,
        "crossroads: stable ids, roles, services, and seed positions reach the wire");
  check(!state["state"]["houseInvestment"]["eligible"].boolean().value_or(true),
        "investment: coffer is locked before the first clear");

  bool forged_chart = false;
  session.handle(
      Envelope{"player:context-menu:action",
               JsonValue::Object{{"queueItem",
                  JsonValue::Object{{"action", JsonValue::Object{{"actionId", "world:road:chart"}}},
                                    {"item", JsonValue::Object{{"id", "tin"}}}}}}},
      [&](const Envelope& event) {
        if (event.event == "open:screen") forged_chart = true;
      });
  check(!forged_chart,
        "crossroads: a forged road-chart option cannot bypass Aldwyn's reach");

  // A forged action cannot choose from across town.
  session.handle(Envelope{"house:investment:choose",
                          JsonValue::Object{{"choice", "house_production"}}},
                 [](const Envelope&) {});
  state = request_state(session, "investment-far");
  check(*state["state"]["houseInvestment"]["choice"].string() == "unchosen",
        "investment: choice requires authoritative proximity to Rhea");

  session.handle(Envelope{"world:zone:enter",
                          JsonValue::Object{{"nodeId", "tin:1:0"}}},
                 [](const Envelope&) {});
  session.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                 [](const Envelope&) {});
  state = request_state(session, "investment-earned");
  check(state["state"]["houseInvestment"]["firstClearCompleted"].boolean().value_or(false) &&
            state["state"]["houseInvestment"]["eligible"].boolean().value_or(false),
        "investment: first authoritative floor clear opens one House choice");

  session.handle(Envelope{"party:returnToTown", JsonValue::Object{}},
                 [](const Envelope&) {});
  session.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", 31}, {"y", 121}}},
                 [](const Envelope&) {});
  std::optional<Envelope> dialogue;
  session.handle(
      Envelope{"player:context-menu:action",
               JsonValue::Object{{"queueItem",
                  JsonValue::Object{{"action", JsonValue::Object{{"actionId", "player:npc:examine"}}},
                                    {"item", JsonValue::Object{{"id", 4}}}}}}},
      [&](const Envelope& event) {
        if (event.event == "open:screen" && event.data["screen"].string() &&
            *event.data["screen"].string() == "dialogue") dialogue = event;
      });
  check(dialogue && dialogue->data["payload"]["npcKey"].string() &&
            *dialogue->data["payload"]["npcKey"].string() == "rhea-countinghouse" &&
            dialogue->data["payload"]["options"].array() &&
            dialogue->data["payload"]["options"].array()->size() == 3,
        "crossroads: Rhea opens a state-aware dialogue with bank and two investments");

  session.handle(
      Envelope{"player:context-menu:action",
               JsonValue::Object{{"queueItem",
                  JsonValue::Object{{"action", JsonValue::Object{{"actionId", "house:investment:choose"}}},
                                    {"item", JsonValue::Object{{"id", "house_production"}}}}}}},
      [](const Envelope&) {});
  state = request_state(session, "investment-chosen");
  check(*state["state"]["houseInvestment"]["choice"].string() == "house_production" &&
            state["state"]["houseInvestment"]["rewardClaimed"].boolean().value_or(false) &&
            state["state"]["houseInvestment"]["houseIncomePerClear"].number().value_or(0) == 5,
        "investment: production choice seals once with its exact yield");

  session.handle(Envelope{"player:chronicles:select",
                          JsonValue::Object{{"scionId", "scion-roadborn"},
                                            {"houseId", "house-crossroads"},
                                            {"scionName", "Edda"},
                                            {"mortal", false}}},
                 [](const Envelope&) {});
  state = request_state(session, "investment-restored");
  check(*state["state"]["houseInvestment"]["choice"].string() == "house_production",
        "investment: the sealed choice restores on a later Scion admission");
  session.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", 31}, {"y", 121}}},
                 [](const Envelope&) {});
  session.handle(Envelope{"house:investment:choose",
                          JsonValue::Object{{"choice", "scion_gear"}}},
                 [](const Envelope&) {});
  state = request_state(session, "investment-immutable");
  check(*state["state"]["houseInvestment"]["choice"].string() == "house_production",
        "investment: a later Scion cannot replace the House's founding choice");

  session.handle(Envelope{"world:zone:enter",
                          JsonValue::Object{{"nodeId", "salt:1:0"}}},
                 [](const Envelope&) {});
  session.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                 [](const Envelope&) {});
  state = request_state(session, "investment-yield");
  check(state["state"]["houseInvestment"]["choice"].string() &&
            *state["state"]["houseInvestment"]["choice"].string() == "house_production" &&
            state["state"]["houseInvestment"]["houseIncomePerClear"].number().value_or(0) == 5,
        "investment: House choice survives the next expedition");
  const JsonValue* saved_house = nullptr;
  if (const auto* houses = state["state"]["chroniclesRecord"]["state"]["houses"].array())
    if (!houses->empty()) saved_house = &houses->front();
  check(saved_house && (*saved_house)["treasury"].number().value_or(0) == 5 &&
            (*saved_house)["firstInvestment"]["choice"].string() &&
            *(*saved_house)["firstInvestment"]["choice"].string() == "house_production",
        "investment: subsequent clears persist yield and choice in the Chronicle House");

  ProtocolSession gear("guest-coffer-gear", "socket-coffer-gear", 0x9ea1u, true);
  gear.handle(Envelope{"world:zone:enter", JsonValue::Object{{"nodeId", "tin:1:0"}}},
              [](const Envelope&) {});
  gear.handle(Envelope{"dev:clear-floor", JsonValue::Object{}}, [](const Envelope&) {});
  gear.handle(Envelope{"party:returnToTown", JsonValue::Object{}}, [](const Envelope&) {});
  gear.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", 31}, {"y", 121}}},
              [](const Envelope&) {});
  gear.handle(Envelope{"house:investment:choose",
                       JsonValue::Object{{"choice", "scion_gear"}}},
              [](const Envelope&) {});
  const auto gear_state = request_state(gear, "investment-gear");
  bool named_vessel = false;
  if (const auto* items = gear_state["state"]["inventory"].array()) {
    for (const auto& item : *items)
      if (item["id"].string() && *item["id"].string() == "vessel-handaxe" &&
          item["vessel"].object()) named_vessel = true;
  }
  check(named_vessel &&
            *gear_state["state"]["houseInvestment"]["choice"].string() == "scion_gear",
        "investment: Scion path grants real Vesselforge-native named gear");
}

void test_campaign_contract_and_scion_checkpoint() {
  ProtocolSession session("guest-campaign", "socket-campaign", 0xca11u, false);
  JsonValue::Object house;
  house["id"] = JsonValue("house-campaign");
  house["name"] = JsonValue("House Emberwake");
  house["renown"] = JsonValue(17);
  house["campaignComplete"] = JsonValue(false);
  house["scions"] = JsonValue(JsonValue::Array{
      JsonValue(JsonValue::Object{{"id", "scion-campaign"},
                                  {"name", "Ilyra"}, {"level", 1},
                                  {"mortal", false}})});
  house["crypt"] = JsonValue(JsonValue::Array{});
  JsonValue::Object chronicle;
  chronicle["version"] = JsonValue(3);
  chronicle["houses"] = JsonValue(JsonValue::Array{JsonValue(house)});
  session.handle(Envelope{"player:chronicles:save",
                          JsonValue::Object{{"state", JsonValue(chronicle)}}},
                 [](const Envelope&) {});
  const auto admit = [&] {
    session.handle(Envelope{"player:chronicles:select",
                            JsonValue::Object{{"scionId", "scion-campaign"},
                                              {"houseId", "house-campaign"},
                                              {"scionName", "Ilyra"},
                                              {"mortal", false}}},
                   [](const Envelope&) {});
  };
  admit();

  auto state = request_state(session, "campaign-contract");
  const JsonValue& quests = state["state"]["quests"];
  check(quests["activeQuest"]["title"].string() &&
            *quests["activeQuest"]["title"].string() == "Aldwyn's Charge" &&
            quests["activeQuest"]["objective"]["text"].string() &&
            quests["activeQuest"]["reward"].string(),
        "campaign: snapshot publishes title, objective copy, and reward");
  check(quests["houseRenown"].number().value_or(0) == 17 &&
            !quests["campaignComplete"].boolean().value_or(true),
        "campaign: House renown and completion are authoritative journal fields");

  session.handle(Envelope{"player:move",
                          JsonValue::Object{{"direction", "right"}}},
                 [](const Envelope&) {});
  state = request_state(session, "campaign-after-move");
  check(state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 1,
        "campaign: accepted movement advances the first objective");

  admit();
  state = request_state(session, "campaign-restored");
  check(state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 1 &&
            state["state"]["quests"]["activeQuest"]["objective"]["text"].string() &&
            *state["state"]["quests"]["activeQuest"]["objective"]["text"].string() ==
                "Strike with your equipped weapon.",
        "campaign: selecting the same Scion restores the exact objective checkpoint");
}

void test_four_roads_campaign_act_and_persistence() {
  ProtocolSession session("guest-four-roads", "socket-four-roads", 0x4a0du,
                          false);
  JsonValue::Object campaign;
  campaign["activeQuestIndex"] = JsonValue(4);
  campaign["objectiveIndex"] = JsonValue(0);
  campaign["questPoints"] = JsonValue(4);
  campaign["completed"] = JsonValue(JsonValue::Array{
      JsonValue("aldwyns-charge"), JsonValue("proof-of-temper"),
      JsonValue("the-pale-crown"), JsonValue("rot-in-the-reeds")});
  JsonValue::Object scion{{"id", "scion-four-roads"},
                          {"name", "Maelin"},
                          {"level", 1},
                          {"mortal", false},
                          {"campaignQuests", JsonValue(campaign)}};
  JsonValue::Object house;
  house["id"] = JsonValue("house-four-roads");
  house["name"] = JsonValue("House Greyfen");
  house["renown"] = JsonValue(50);
  house["campaignComplete"] = JsonValue(false);
  house["clearedRoadNodes"] = JsonValue(JsonValue::Array{});
  house["scions"] = JsonValue(JsonValue::Array{JsonValue(scion)});
  house["crypt"] = JsonValue(JsonValue::Array{});
  JsonValue::Object chronicle;
  chronicle["version"] = JsonValue(3);
  chronicle["houses"] = JsonValue(JsonValue::Array{JsonValue(house)});
  session.handle(Envelope{"player:chronicles:save",
                          JsonValue::Object{{"state", JsonValue(chronicle)}}},
                 [](const Envelope&) {});
  const auto admit = [&] {
    session.handle(Envelope{"player:chronicles:select",
                            JsonValue::Object{{"scionId", "scion-four-roads"},
                                              {"houseId", "house-four-roads"},
                                              {"scionName", "Maelin"},
                                              {"mortal", false}}},
                   [](const Envelope&) {});
  };
  admit();
  auto state = request_state(session, "roads-act-start");
  check(state["state"]["quests"]["activeQuest"]["id"].string() &&
            *state["state"]["quests"]["activeQuest"]["id"].string() ==
                "oath-of-tin",
        "four-roads: second campaign act begins with the Oath of Tin");

  bool barred_message = false;
  session.handle(Envelope{"world:zone:enter",
                          JsonValue::Object{{"nodeId", "tin:2:0"}}},
                 [&](const Envelope& event) {
                   if (event.event == "game:send:message" &&
                       event.data["text"].string() &&
                       event.data["text"].string()->find("barred") !=
                           std::string::npos)
                     barred_message = true;
                 });
  state = request_state(session, "roads-barred");
  check(barred_message && state["state"]["sceneType"].string() &&
            *state["state"]["sceneType"].string() == "town",
        "four-roads: a forged deeper-node request cannot bypass its parent Warden");

  session.handle(Envelope{"world:zone:enter",
                          JsonValue::Object{{"nodeId", "tin:1:0"}}},
                 [](const Envelope&) {});
  state = request_state(session, "roads-entered");
  check(state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 1,
        "four-roads: entering the exact road advances its commission");
  session.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                 [](const Envelope&) {});
  state = request_state(session, "roads-cleared");
  check(state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 2,
        "four-roads: the authoritative Warden clear advances the rite");
  session.handle(Envelope{"party:returnToTown", JsonValue::Object{}},
                 [](const Envelope&) {});
  state = request_state(session, "roads-returned");
  check(state["state"]["quests"]["activeQuest"]["id"].string() &&
            *state["state"]["quests"]["activeQuest"]["id"].string() ==
                "salt-reckoning" &&
            state["state"]["quests"]["questPoints"].number().value_or(0) == 5 &&
            state["state"]["quests"]["houseRenown"].number().value_or(0) == 75,
        "four-roads: returning completes Tin and opens the Salt Reckoning");

  admit();
  std::optional<Envelope> chart;
  session.handle(Envelope{"world:road:chart",
                          JsonValue::Object{{"roadId", "tin"}}},
                 [&](const Envelope& event) {
                   if (event.event == "open:screen") chart = event;
                 });
  bool tier_one_cleared = false;
  bool tier_two_open = false;
  if (chart) {
    const auto* nodes = chart->data["payload"]["nodes"].array();
    if (nodes) {
      for (const auto& node : *nodes) {
        const int tier = static_cast<int>(node["tier"].number().value_or(0));
        const std::string status =
            node["status"].string() ? *node["status"].string() : "";
        if (tier == 1 && status == "cleared") tier_one_cleared = true;
        if (tier == 2 && status == "open") tier_two_open = true;
      }
    }
  }
  check(tier_one_cleared && tier_two_open,
        "four-roads: House road history restores and authorizes the next tier");
  session.handle(Envelope{"world:zone:enter",
                          JsonValue::Object{{"nodeId", "tin:2:0"}}},
                 [](const Envelope&) {});
  state = request_state(session, "roads-tier-two");
  check(state["state"]["sceneType"].string() &&
            *state["state"]["sceneType"].string() == "instance",
        "four-roads: the restored parent clear permits the deeper holding");
  session.handle(Envelope{"party:returnToTown", JsonValue::Object{}},
                 [](const Envelope&) {});

  struct RoadActStep { const char* road; const char* next_quest; };
  const RoadActStep remaining[] = {
      {"salt", "chalk-vigil"},
      {"chalk", "copper-testament"},
      {"copper", nullptr},
  };
  for (const auto& step : remaining) {
    session.handle(
        Envelope{"world:zone:enter",
                 JsonValue::Object{{"nodeId", std::string(step.road) + ":1:0"}}},
        [](const Envelope&) {});
    state = request_state(session, std::string("roads-") + step.road + "-entered");
    check(state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 1,
          "four-roads: each named road advances only on authoritative entry");
    session.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                   [](const Envelope&) {});
    state = request_state(session, std::string("roads-") + step.road + "-cleared");
    check(state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 2,
          "four-roads: each named road requires its Warden clear");
    session.handle(Envelope{"party:returnToTown", JsonValue::Object{}},
                   [](const Envelope&) {});
    state = request_state(session, std::string("roads-") + step.road + "-returned");
    if (step.next_quest) {
      check(state["state"]["quests"]["activeQuest"]["id"].string() &&
                *state["state"]["quests"]["activeQuest"]["id"].string() ==
                    step.next_quest,
            "four-roads: returning opens the next named road commission");
    }
  }
  check(state["state"]["quests"]["campaignComplete"].boolean().value_or(false) &&
            state["state"]["quests"]["activeQuest"].is_null() &&
            state["state"]["quests"]["questPoints"].number().value_or(0) == 8 &&
            state["state"]["quests"]["houseRenown"].number().value_or(0) == 180 &&
            state["state"]["endgame"]["unlocked"].boolean().value_or(false) &&
            !inventory_map_uuid(state).empty(),
        "four-roads: Copper return seals Act II and awards the first endgame tablet");
}

void test_consumable_endgame_tablet_loop() {
  ProtocolSession session("guest-endgame", "socket-endgame", 0x51ea1u, false);
  auto open_tablet = [&](const std::string& uuid,
                         const std::function<void(const Envelope&)>& emit) {
    session.handle(
        Envelope{"player:context-menu:action",
                 JsonValue::Object{{
                     "queueItem",
                     JsonValue::Object{
                         {"action", JsonValue::Object{{
                                        "actionId", "player:endgame:open-map"}}},
                         {"item", JsonValue::Object{{"uuid", uuid}}}}}}},
        emit);
  };

  session.handle(Envelope{"dev:give", JsonValue::Object{
                                          {"itemId", "charted-tablet-crown"},
                                          {"qty", 1}, {"itemLevel", 5},
                                          {"seed", 77}}},
                 [](const Envelope&) {});
  auto locked = request_state(session, "map-locked");
  const std::string locked_uuid =
      inventory_uuid_for(locked, "charted-tablet-crown");
  check(!locked_uuid.empty() &&
            !locked["state"]["endgame"]["unlocked"].boolean().value_or(true),
        "endgame: a tablet is an inventory item before the campaign unlock");
  open_tablet(locked_uuid, [](const Envelope&) {});
  locked = request_state(session, "map-still-locked");
  check(*locked["state"]["sceneType"].string() == "town" &&
            !inventory_uuid_for(locked, "charted-tablet-crown").empty(),
        "endgame: a locked or out-of-place use consumes nothing");

  JsonValue::Object house;
  house["id"] = JsonValue("house-endgame");
  house["name"] = JsonValue("House Emberwake");
  house["campaignComplete"] = JsonValue(true);
  house["endgameMapsCompleted"] = JsonValue(2);
  house["endgameMasteries"] = JsonValue(JsonValue::Array{
      JsonValue("barrow:1"), JsonValue("barrow:1"),
      JsonValue("crown:4"), JsonValue("counterfeit:99")});
  house["scions"] = JsonValue(JsonValue::Array{
      JsonValue(JsonValue::Object{{"id", "scion-cartographer"},
                                  {"name", "Ilyra"}, {"level", 12},
                                  {"mortal", false}})});
  house["crypt"] = JsonValue(JsonValue::Array{});
  JsonValue::Object chronicle;
  chronicle["version"] = JsonValue(3);
  chronicle["houses"] = JsonValue(JsonValue::Array{JsonValue(house)});
  chronicle["activeHouseId"] = JsonValue("house-endgame");
  chronicle["activeScionId"] = JsonValue("scion-cartographer");
  session.handle(Envelope{"player:chronicles:save",
                          JsonValue::Object{{"state", JsonValue(chronicle)}}},
                 [](const Envelope&) {});
  session.handle(Envelope{"player:chronicles:select",
                          JsonValue::Object{{"scionId", "scion-cartographer"},
                                            {"houseId", "house-endgame"},
                                            {"scionName", "Ilyra"},
                                            {"mortal", false}}},
                 [](const Envelope&) {});
  auto inherited = request_state(session, "map-inherited");
  check(inherited["state"]["endgame"]["unlocked"].boolean().value_or(false) &&
            inherited["state"]["endgame"]["completed"].number().value_or(0) == 2 &&
            inherited["state"]["endgame"]["mastered"].number().value_or(0) == 2 &&
            inherited["state"]["endgame"]["highestTier"].number().value_or(0) == 4,
        "endgame: valid unique mastery objectives belong to the House, not one Scion");

  session.handle(Envelope{"dev:give", JsonValue::Object{
                                          {"itemId", "charted-tablet-crown"},
                                          {"qty", 1}, {"itemLevel", 5},
                                          {"seed", 77}}},
                 [](const Envelope&) {});
  const auto carried = request_state(session, "map-carried");
  const std::string uuid = inventory_uuid_for(carried, "charted-tablet-crown");
  const JsonValue* tablet = nullptr;
  for (const auto& entry : *carried["state"]["inventory"].array())
    if (entry["uuid"].string() && *entry["uuid"].string() == uuid)
      tablet = &entry;
  check(tablet && (*tablet)["expeditionMap"]["tier"].number().value_or(0) == 5 &&
            (*tablet)["expeditionMap"]["family"].string() &&
            *(*tablet)["expeditionMap"]["family"].string() == "Crown" &&
            (*tablet)["expeditionMap"]["objectiveKey"].string() &&
            *(*tablet)["expeditionMap"]["objectiveKey"].string() == "crown:5" &&
            (*tablet)["expeditionMap"]["modifiers"].array() &&
            (*tablet)["expeditionMap"]["modifiers"].array()->size() == 2,
        "endgame: tier and rolled clauses serialize with the exact tablet");
  JsonValue reconnect_player;
  check(parse_json(session.login_payload(), reconnect_player) &&
            reconnect_player["player"]["inventory"]["slots"].array(),
        "endgame: reconnect player payload remains valid JSON");
  const JsonValue* reconnect_tablet = nullptr;
  for (const auto& entry :
       *reconnect_player["player"]["inventory"]["slots"].array())
    if (entry["uuid"].string() && *entry["uuid"].string() == uuid)
      reconnect_tablet = &entry;
  check(reconnect_tablet &&
            (*reconnect_tablet)["expeditionMap"]["tier"].number().value_or(0) == 5 &&
            (*reconnect_tablet)["expeditionMap"]["modifiers"].array() &&
            (*reconnect_tablet)["expeditionMap"]["modifiers"].array()->size() == 2,
        "endgame: reconnect retains the tablet tier and rolled clauses");

  bool transitioned = false;
  open_tablet(uuid, [&](const Envelope& event) {
    if (event.event == "world:scene:transition") transitioned = true;
  });
  const auto opened = request_state(session, "map-opened");
  check(transitioned && *opened["state"]["sceneType"].string() == "instance" &&
            opened["state"]["endgame"]["active"].boolean().value_or(false) &&
            opened["state"]["endgame"]["tier"].number().value_or(0) == 5 &&
            opened["state"]["endgame"]["firstClear"].boolean().value_or(false) &&
            opened["state"]["endgame"]["objectiveKey"].string() &&
            *opened["state"]["endgame"]["objectiveKey"].string() == "crown:5",
        "endgame: consuming the tablet opens its one authoritative expedition");
  check(inventory_uuid_for(opened, "charted-tablet-crown").empty(),
        "endgame: opening consumes the exact tablet once");

  const auto* monsters = opened["state"]["monsters"].array();
  const JsonValue* boss = nullptr;
  if (monsters) {
    for (const auto& monster : *monsters)
      if (monster["name"].string() &&
          *monster["name"].string() == "The Seal-Bound Warden")
        boss = &monster;
  }
  check(boss != nullptr, "endgame: the rolled expedition has a named terminal Warden");
  session.handle(Envelope{"dev:setlevel", JsonValue::Object{{"level", 50}}},
                 [](const Envelope&) {});
  session.handle(Envelope{"dev:teleport",
                          JsonValue::Object{{"x", boss ? boss->operator[]("x").number().value_or(0) + 1 : 0},
                                            {"y", boss ? boss->operator[]("y").number().value_or(0) : 0}}},
                 [](const Envelope&) {});
  session.handle(Envelope{"dev:monster:reset",
                          JsonValue::Object{{"monsterUuid", boss && boss->operator[]("uuid").string()
                                                ? *boss->operator[]("uuid").string()
                                                : std::string()},
                                            {"maxHealth", 1}}},
                 [](const Envelope&) {});
  session.set_direct_emit([](const Envelope&) {});
  session.handle(Envelope{"player:skill:trigger",
                          JsonValue::Object{{"skillId", "primary-attack"},
                                            {"direction", "left"}}},
                 [](const Envelope&) {});
  session.tick(5000000000000LL);
  const auto cleared = request_state(session, "map-cleared");
  check(cleared["state"]["endgame"]["cleared"].boolean().value_or(false) &&
            cleared["state"]["endgame"]["completed"].number().value_or(0) == 3 &&
            cleared["state"]["endgame"]["mastered"].number().value_or(0) == 3 &&
            cleared["state"]["endgame"]["highestTier"].number().value_or(0) == 5 &&
            cleared["state"]["endgame"]["ascentChancePercent"].number().value_or(0) == 36 &&
            cleared["state"]["quests"]["houseRenown"].number().value_or(0) == 15,
        "endgame: first Warden clear advances mastery, renown, and ascent sustain once");
  bool next_tablet = false;
  double next_tablet_x = 0;
  double next_tablet_y = 0;
  for (const auto& ground : *cleared["state"]["groundItems"].array())
    if (ground["expeditionMap"].object()) {
      next_tablet = true;
      next_tablet_x = ground["x"].number().value_or(0);
      next_tablet_y = ground["y"].number().value_or(0);
    }
  check(next_tablet,
        "endgame: a cleared expedition drops the next rolled tablet for sustain");
  session.handle(Envelope{"dev:teleport", JsonValue::Object{
                                                {"x", next_tablet_x},
                                                {"y", next_tablet_y}}},
                 [](const Envelope&) {});
  JsonValue claimed;
  for (int pickup = 0; pickup < 6; ++pickup) {
    session.handle(Envelope{"player:take:underfoot", JsonValue::Object{}},
                   [](const Envelope&) {});
    claimed = request_state(session, "map-claimed-" + std::to_string(pickup));
    if (!inventory_map_uuid(claimed).empty()) break;
  }
  const std::string next_uuid = inventory_map_uuid(claimed);
  check(!next_uuid.empty(),
        "endgame: the next tablet can be claimed through the ordinary pickup path");
  const double stairs_x =
      claimed["state"]["sceneMetadata"]["stairsUp"]["x"].number().value_or(0);
  const double stairs_y =
      claimed["state"]["sceneMetadata"]["stairsUp"]["y"].number().value_or(0);
  session.handle(Envelope{"dev:teleport",
                          JsonValue::Object{{"x", stairs_x}, {"y", stairs_y}}},
                 [](const Envelope&) {});
  const auto returned = request_state(session, "map-returned");
  check(*returned["state"]["sceneType"].string() == "town" &&
            !returned["state"]["endgame"]["active"].boolean().value_or(true),
        "endgame: the entry waymark closes the one-use expedition");
  check(inventory_map_uuid(returned) == next_uuid,
        "endgame: extraction banks loot but keeps the next tablet usable in town");

  session.handle(Envelope{"player:chronicles:select",
                          JsonValue::Object{{"scionId", "scion-cartographer"},
                                            {"houseId", "house-endgame"},
                                            {"scionName", "Ilyra"},
                                            {"mortal", false}}},
                 [](const Envelope&) {});
  const auto restored = request_state(session, "map-mastery-restored");
  check(restored["state"]["endgame"]["completed"].number().value_or(0) == 3 &&
            restored["state"]["endgame"]["mastered"].number().value_or(0) == 3 &&
            restored["state"]["endgame"]["highestTier"].number().value_or(0) == 5 &&
            restored["state"]["quests"]["houseRenown"].number().value_or(0) == 15,
        "endgame: mastery board and first-clear renown survive House re-admission");

  session.handle(Envelope{"dev:give", JsonValue::Object{
                                          {"itemId", "charted-tablet-crown"},
                                          {"qty", 1}, {"itemLevel", 5},
                                          {"seed", 91}}},
                 [](const Envelope&) {});
  auto repeat_ready = request_state(session, "map-repeat-ready");
  open_tablet(inventory_uuid_for(repeat_ready, "charted-tablet-crown"),
              [](const Envelope&) {});
  auto repeat_opened = request_state(session, "map-repeat-opened");
  check(!repeat_opened["state"]["endgame"]["firstClear"].boolean().value_or(true),
        "endgame: an already-mastered family and tier is marked as a repeat");
  const JsonValue* repeat_boss = nullptr;
  for (const auto& monster : *repeat_opened["state"]["monsters"].array())
    if (monster["name"].string() &&
        *monster["name"].string() == "The Seal-Bound Warden")
      repeat_boss = &monster;
  session.handle(Envelope{"dev:teleport",
                          JsonValue::Object{{"x", repeat_boss ? repeat_boss->operator[]("x").number().value_or(0) + 1 : 0},
                                            {"y", repeat_boss ? repeat_boss->operator[]("y").number().value_or(0) : 0}}},
                 [](const Envelope&) {});
  session.handle(Envelope{"dev:monster:reset",
                          JsonValue::Object{{"monsterUuid", repeat_boss && repeat_boss->operator[]("uuid").string()
                                                ? *repeat_boss->operator[]("uuid").string()
                                                : std::string()},
                                            {"maxHealth", 1}}},
                 [](const Envelope&) {});
  session.handle(Envelope{"player:skill:trigger",
                          JsonValue::Object{{"skillId", "primary-attack"},
                                            {"direction", "left"}}},
                 [](const Envelope&) {});
  session.tick(5000000100000LL);
  const auto repeated = request_state(session, "map-repeat-cleared");
  check(repeated["state"]["endgame"]["completed"].number().value_or(0) == 4 &&
            repeated["state"]["endgame"]["mastered"].number().value_or(0) == 3 &&
            repeated["state"]["quests"]["houseRenown"].number().value_or(0) == 15,
        "endgame: repeat clears count as runs without duplicating mastery or renown");
}
}  // namespace

int main() {
  try {
    test_envelope_round_trip();
    test_session_lifecycle();
    test_continuous_movement();
    test_instance_entry_and_stairs();
    test_n3_combat_rules_and_wire_events();
    test_authoritative_remote_skill_actions();
    test_gate_a_ground_login_and_kill_loot();
    test_gate_a_extract_and_stairs();
    test_gate_a_equip_totals_and_unknown_uuid();
    test_crossroads_social_hub_and_house_investment();
    test_campaign_contract_and_scion_checkpoint();
    test_four_roads_campaign_act_and_persistence();
    test_consumable_endgame_tablet_loop();
    std::cout << "verdigris networking tests: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "verdigris networking tests: FAIL: " << error.what() << "\n";
    return 1;
  }
}
