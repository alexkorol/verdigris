#include "verdigris/networking.hpp"

#include <algorithm>
#include <chrono>
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

  ProtocolSession roles("guest-n3-roles", "socket-roles", 0xA11CE, false);
  roles.handle(Envelope{"instance:enterSolo", JsonValue::Object{
      {"template", "marsh"}, {"layout", "clearings"}}}, [](const Envelope&) {});
  const auto role_state = request_state(roles, "n3-roles");
  const auto* role_monsters = role_state["state"]["monsters"].array();
  const JsonValue* ranged = nullptr;
  for (const auto& value : *role_monsters)
    if (value["behaviour"]["type"].string() &&
        *value["behaviour"]["type"].string() == "ranged" &&
        value["rarity"].string() && *value["rarity"].string() != "elite") {
      ranged = &value;
      break;
    }
  check(ranged != nullptr, "roles wire: marsh snapshot identifies a ranged foe");
  if (ranged) {
    const int rx = static_cast<int>((*ranged)["x"].number().value_or(0));
    const int ry = static_cast<int>((*ranged)["y"].number().value_or(0));
    const std::string ranged_id = (*ranged)["uuid"].string()
        ? *(*ranged)["uuid"].string() : std::string();
    const int px = rx + 1 < 39 ? rx + 1 : rx - 1;
    bool volley_wire = false;
    bool movement_wire = false;
    auto collect_role = [&](const Envelope& event) {
      if (event.event == "monster:moved" &&
          event.data["monsterId"].string() &&
          *event.data["monsterId"].string() == ranged_id) {
        movement_wire = event.data["durationMs"].number().value_or(0) == 400 &&
                        event.data["behaviour"].string() &&
                        *event.data["behaviour"].string() == "ranged" &&
                        (event.data["x"].number().value_or(rx) != rx ||
                         event.data["y"].number().value_or(ry) != ry);
      }
      if (event.event == "monster:telegraph" &&
          event.data["skillId"].string() &&
          *event.data["skillId"].string() == "ranged:volley") {
        volley_wire = event.data["x"].number().value_or(-1) == px &&
                      event.data["y"].number().value_or(-1) == ry &&
                      event.data["radius"].number().value_or(0) == 1 &&
                      event.data["durationMs"].number().value_or(0) == 800;
      }
    };
    roles.set_direct_emit(collect_role);
    roles.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", px}, {"y", ry}}},
                 collect_role);
    for (const auto at : {5000000000000LL, 5000000000400LL,
                          5000000000800LL, 5000000001200LL})
      roles.tick(at);
    check(movement_wire,
          "roles wire: accepted ranged spacing emits an exact movement fact");
    check(volley_wire,
          "roles wire: ranged volley emits exact target, radius, and windup");
  }
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

  // Primary attacks are a server-owned three-beat cadence. The held attack
  // loop must advance it without fresh client guesses, and every resolved
  // hit carries the exact presentation contract.
  ProtocolSession combo("guest-skills-combo","socket-skills-combo",219,false);
  combo.handle(Envelope{"instance:enterSolo",JsonValue::Object{{"template","dungeon"},{"layout","clearings"}}},[](const Envelope&){});
  const auto combo_state=request_state(combo,"skills-combo-target");
  const auto& combo_target=combo_state["state"]["monsters"].array()->front();
  const int combo_tx=static_cast<int>(combo_target["x"].number().value_or(0));
  const int combo_ty=static_cast<int>(combo_target["y"].number().value_or(0));
  const int combo_px=combo_tx+1<39?combo_tx+1:combo_tx-1;
  const std::string combo_aim=direction_toward(combo_px,combo_ty,combo_tx,combo_ty);
  const std::string combo_uuid=combo_target["uuid"].string()?*combo_target["uuid"].string():std::string();
  combo.handle(Envelope{"dev:teleport",JsonValue::Object{{"x",combo_px},{"y",combo_ty}}},[](const Envelope&){});
  combo.handle(Envelope{"dev:monster:reset",JsonValue::Object{{"monsterUuid",combo_uuid},{"maxHealth",1000}}},[](const Envelope&){});
  std::vector<int> combo_steps;
  std::vector<int> combo_damage;
  int combo_stagger=0;
  int combat_state_step=0;
  int combat_state_window=0;
  auto collect_combo=[&](const Envelope& event){
    if(event.event=="combat:hit"&&event.data["targetType"].string()&&
       *event.data["targetType"].string()=="monster") {
      combo_steps.push_back(static_cast<int>(event.data["comboStep"].number().value_or(0)));
      combo_damage.push_back(static_cast<int>(event.data["amount"].number().value_or(0)));
      combo_stagger=static_cast<int>(event.data["staggerMs"].number().value_or(0));
    }
    if(event.event=="player:combat-state") {
      combat_state_step=static_cast<int>(event.data["comboStep"].number().value_or(0));
      combat_state_window=static_cast<int>(event.data["comboWindowTicks"].number().value_or(0));
    }
  };
  combo.handle(Envelope{"player:skill:trigger",JsonValue::Object{{"skill","melee"},{"direction",combo_aim}}},collect_combo);
  combo.set_direct_emit(collect_combo);
  const auto combo_clock = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  combo.tick(combo_clock+350);
  combo.tick(combo_clock+700);
  check(combo_steps==std::vector<int>({1,2,3})&&combo_damage.size()==3&&
            combo_damage[1]==combo_damage[0]*115/100&&
            combo_damage[2]==combo_damage[0]*160/100,
        "skills: wire carries the server's exact three-beat damage cadence");
  check(combo_stagger==700&&combat_state_step==3&&combat_state_window==18,
        "skills: finisher stagger and active cadence reach combat-state");
  const auto combo_after=request_state(combo,"skills-combo-after");
  check(combo_after["state"]["combatCadence"]["step"].number().value_or(0)==3&&
            combo_after["state"]["combatCadence"]["windowTicks"].number().value_or(0)==18,
        "skills: snapshots preserve authoritative cadence for reconnects");

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

void test_active_forge_properties_cross_the_protocol() {
  ProtocolSession session("guest-forge-active", "socket-forge-active", 0xB1EED,
                          false);
  session.handle(Envelope{"dev:give", JsonValue::Object{
      {"itemId", "vessel-macuahuitl"}, {"qty", 1},
      {"itemLevel", 40}, {"seed", 17}}}, [](const Envelope&) {});
  const auto granted = request_state(session, "forge-granted");
  const std::string uuid = inventory_uuid_for(granted, "vessel-macuahuitl");
  check(!uuid.empty(), "forge wire: granted Macuahuitl has a stable uuid");
  const JsonValue* details = nullptr;
  if (const auto* inventory = granted["state"]["inventoryDetails"].array())
    for (const auto& item : *inventory)
      if (item["uuid"].string() && *item["uuid"].string() == uuid)
        details = &item;
  check(details &&
            (*details)["combatBonuses"]["bleedChance"].number().value_or(0) == 100,
        "forge wire: item projection carries the active bleeding implicit");
  check(details && (*details)["size"]["width"].number().value_or(0) > 0 &&
            (*details)["size"]["height"].number().value_or(0) > 0 &&
            (*details)["equipSlot"].string() &&
            *(*details)["equipSlot"].string() == "right_hand" &&
            (*details)["twoHanded"].boolean().has_value(),
        "forge wire: item identity publishes footprint and equip metadata");
  bool active_bleed_line = false;
  if (details) {
    if (const auto* lines = (*details)["vessel"]["lines"].array())
      for (const auto& line : *lines)
        if (line["text"].string() &&
            *line["text"].string() == "Hits cause Bleeding" &&
            line["section"].string() && *line["section"].string() == "implicit")
          active_bleed_line = true;
  }
  check(active_bleed_line,
        "forge wire: active bleed tooltip is not mislabeled Dormant");

  std::optional<Envelope> equipped;
  session.handle(Envelope{"item:equip", JsonValue::Object{{
                     "item", JsonValue::Object{{"uuid", uuid}}}}},
                 [&](const Envelope& event) {
                   if (event.event == "player:equippedAnItem") equipped = event;
                 });
  check(equipped &&
            equipped->data["combat"]["bleedChance"].number().value_or(0) == 100,
        "forge wire: equip response publishes worn bleed chance");
  const auto worn = request_state(session, "forge-worn");
  check(worn["state"]["combat"]["bleedChance"].number().value_or(0) == 100 &&
            worn["state"]["combat"]["reachPercent"].number().has_value() &&
            worn["state"]["combat"]["movementSpeedPercent"].number().has_value() &&
            worn["state"]["combat"]["emberResistance"].number().has_value() &&
            worn["state"]["combat"]["riverResistance"].number().has_value(),
        "forge wire: snapshots publish the complete active-property totals contract");

  session.handle(Envelope{"instance:enterSolo", JsonValue::Object{
      {"template", "dungeon"}, {"layout", "clearings"}}}, [](const Envelope&) {});
  const auto in_zone = request_state(session, "forge-target");
  const auto* monsters = in_zone["state"]["monsters"].array();
  check(monsters && !monsters->empty(), "forge wire: bleed trial has a target");
  if (!monsters || monsters->empty()) return;
  const auto& target = monsters->front();
  const std::string target_uuid = target["uuid"].string()
      ? *target["uuid"].string() : std::string();
  const int tx = static_cast<int>(target["x"].number().value_or(0));
  const int ty = static_cast<int>(target["y"].number().value_or(0));
  const int px = tx + 1 < 39 ? tx + 1 : tx - 1;
  const std::string aim = px < tx ? "right" : "left";
  session.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                 [](const Envelope&) {});
  session.handle(Envelope{"dev:monster:reset", JsonValue::Object{
      {"monsterUuid", target_uuid}, {"maxHealth", 1000}}}, [](const Envelope&) {});
  session.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", px}, {"y", ty}}},
                 [](const Envelope&) {});

  bool status_wire = false;
  const auto action_clock = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  auto collect = [&](const Envelope& event) {
    if (event.event == "monster:status" &&
        event.data["statusId"].string() && *event.data["statusId"].string() == "bleed" &&
        event.data["active"].boolean().value_or(false))
      status_wire = event.data["damagePerTick"].number().value_or(0) > 0 &&
                    event.data["durationMs"].number().value_or(0) == 3000;
  };
  session.handle(Envelope{"player:skill:trigger", JsonValue::Object{
      {"skill", "thrust"}, {"direction", aim}}}, collect);
  check(status_wire, "forge wire: guaranteed bleed emits a typed status packet");
  const auto bleeding = request_state(session, "forge-bleeding");
  bool effect_snapshot = false;
  if (const auto* current = bleeding["state"]["monsters"].array())
    for (const auto& monster : *current)
      if (monster["uuid"].string() && *monster["uuid"].string() == target_uuid &&
          monster["state"]["effects"]["bleed"].object())
        effect_snapshot = monster["state"]["effects"]["bleed"]["remainingMs"]
                              .number().value_or(0) > 0;
  check(effect_snapshot,
        "forge wire: reconnect snapshot carries the live bleed effect");

  bool tick_wire = false;
  session.set_direct_emit([&](const Envelope& event) {
    collect(event);
    if (event.event == "combat:hit" && event.data["skillId"].string() &&
        *event.data["skillId"].string() == "status:bleed")
      tick_wire = event.data["amount"].number().value_or(0) > 0 &&
                  event.data["damageChannel"].string() &&
                  *event.data["damageChannel"].string() == "physical";
  });
  session.tick(action_clock + 1200);
  check(tick_wire,
        "forge wire: periodic bleed damage remains a named physical hit");

  ProtocolSession ranged("guest-forge-ranged", "socket-forge-ranged",
                         0xA71A7, false);
  for (const char* item_id : {"vessel-atlatl", "vessel-grips"})
    ranged.handle(Envelope{"dev:give", JsonValue::Object{
        {"itemId", item_id}, {"qty", 1}, {"itemLevel", 40}, {"seed", 23}}},
        [](const Envelope&) {});
  auto ranged_state = request_state(ranged, "forge-ranged-granted");
  const std::string atlatl_uuid =
      inventory_uuid_for(ranged_state, "vessel-atlatl");
  const std::string grips_uuid =
      inventory_uuid_for(ranged_state, "vessel-grips");
  bool atlatl_line_active = false;
  bool grips_line_active = false;
  if (const auto* inventory = ranged_state["state"]["inventoryDetails"].array())
    for (const auto& item : *inventory) {
      if (!item["uuid"].string() || !item["vessel"]["lines"].array()) continue;
      for (const auto& line : *item["vessel"]["lines"].array()) {
        if (!line["text"].string() || !line["section"].string()) continue;
        const bool active = *line["section"].string() == "implicit";
        if (*item["uuid"].string() == atlatl_uuid &&
            *line["text"].string() == "+20% Projectile Range")
          atlatl_line_active = active;
        if (*item["uuid"].string() == grips_uuid &&
            *line["text"].string() == "+8% Attack Speed")
          grips_line_active = active;
      }
    }
  check(atlatl_line_active && grips_line_active,
        "forge wire: Atlatl range and Grips speed ship as active implicits");
  for (const auto& uuid : {atlatl_uuid, grips_uuid})
    ranged.handle(Envelope{"item:equip", JsonValue::Object{{
        "item", JsonValue::Object{{"uuid", uuid}}}}}, [](const Envelope&) {});
  ranged_state = request_state(ranged, "forge-ranged-worn");
  check(ranged_state["state"]["combat"]["projectileRangePercent"]
                .number().value_or(0) == 20 &&
            ranged_state["state"]["combat"]["attackSpeedPercent"]
                .number().value_or(0) == 8,
        "forge wire: equipped Atlatl and Grips publish their worn totals");
  ranged.handle(Envelope{"instance:enterSolo", JsonValue::Object{
      {"template", "dungeon"}, {"layout", "clearings"}}},
      [](const Envelope&) {});
  ranged_state = request_state(ranged, "forge-ranged-target");
  const auto& ranged_target = ranged_state["state"]["monsters"].array()->front();
  const std::string ranged_target_uuid = *ranged_target["uuid"].string();
  const int ranged_tx = static_cast<int>(ranged_target["x"].number().value_or(0));
  const int ranged_ty = static_cast<int>(ranged_target["y"].number().value_or(0));
  const int ranged_px = ranged_tx + 6 < 39 ? ranged_tx + 6 : ranged_tx - 6;
  const std::string ranged_aim = ranged_px < ranged_tx ? "right" : "left";
  ranged.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                [](const Envelope&) {});
  ranged.handle(Envelope{"dev:monster:reset", JsonValue::Object{
      {"monsterUuid", ranged_target_uuid}, {"maxHealth", 1000}}},
      [](const Envelope&) {});
  ranged.handle(Envelope{"dev:teleport", JsonValue::Object{
      {"x", ranged_px}, {"y", ranged_ty}}}, [](const Envelope&) {});
  bool sixth_tile_hit = false;
  int fast_cooldown_ticks = 0;
  ranged.handle(Envelope{"player:skill:trigger", JsonValue::Object{
      {"skill", "melee"}, {"direction", ranged_aim}}},
      [&](const Envelope& event) {
        if (event.event == "combat:hit" && event.data["targetId"].string() &&
            *event.data["targetId"].string() == ranged_target_uuid)
          sixth_tile_hit = true;
        if (event.event == "player:combat-state")
          fast_cooldown_ticks = static_cast<int>(
              event.data["cooldownTicks"].number().value_or(0));
      });
  check(sixth_tile_hit && fast_cooldown_ticks == 7,
        "forge wire: Atlatl reaches six tiles while Grips shorten recovery");

  ProtocolSession piercing("guest-forge-piercing", "socket-forge-piercing",
                           0x511A6, false);
  piercing.handle(Envelope{"dev:give", JsonValue::Object{
      {"itemId", "vessel-sling"}, {"qty", 1},
      {"itemLevel", 40}, {"seed", 29}}}, [](const Envelope&) {});
  auto piercing_state = request_state(piercing, "forge-piercing-granted");
  const std::string sling_uuid =
      inventory_uuid_for(piercing_state, "vessel-sling");
  bool sling_line_active = false;
  if (const auto* inventory = piercing_state["state"]["inventoryDetails"].array())
    for (const auto& item : *inventory)
      if (item["uuid"].string() && *item["uuid"].string() == sling_uuid &&
          item["vessel"]["lines"].array())
        for (const auto& line : *item["vessel"]["lines"].array())
          if (line["text"].string() &&
              *line["text"].string() == "Ignores half of Armour" &&
              line["section"].string() &&
              *line["section"].string() == "implicit")
            sling_line_active = true;
  check(sling_line_active,
        "forge wire: Sling armour bypass ships as an active implicit");
  piercing.handle(Envelope{"item:equip", JsonValue::Object{{
      "item", JsonValue::Object{{"uuid", sling_uuid}}}}},
      [](const Envelope&) {});
  piercing.handle(Envelope{"instance:enterSolo", JsonValue::Object{
      {"template", "dungeon"}, {"layout", "clearings"}}},
      [](const Envelope&) {});
  piercing_state = request_state(piercing, "forge-piercing-target");
  const auto& piercing_target =
      piercing_state["state"]["monsters"].array()->front();
  const std::string piercing_target_uuid = *piercing_target["uuid"].string();
  const int piercing_tx =
      static_cast<int>(piercing_target["x"].number().value_or(0));
  const int piercing_ty =
      static_cast<int>(piercing_target["y"].number().value_or(0));
  const int piercing_px = piercing_tx + 1 < 39 ? piercing_tx + 1
                                               : piercing_tx - 1;
  piercing.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                  [](const Envelope&) {});
  piercing.handle(Envelope{"dev:monster:reset", JsonValue::Object{
      {"monsterUuid", piercing_target_uuid}, {"maxHealth", 1000},
      {"armour", 100}}}, [](const Envelope&) {});
  piercing.handle(Envelope{"dev:teleport", JsonValue::Object{
      {"x", piercing_px}, {"y", piercing_ty}}}, [](const Envelope&) {});
  bool penetration_wire = false;
  piercing.handle(Envelope{"player:skill:trigger", JsonValue::Object{
      {"skill", "melee"},
      {"direction", piercing_px < piercing_tx ? "right" : "left"}}},
      [&](const Envelope& event) {
        if (event.event != "combat:hit" || !event.data["targetId"].string() ||
            *event.data["targetId"].string() != piercing_target_uuid)
          return;
        penetration_wire =
            event.data["attackStyle"].string() &&
            *event.data["attackStyle"].string() == "range" &&
            event.data["armourRating"].number().value_or(0) == 100 &&
            event.data["armourPenetrationPercent"].number().value_or(0) == 50 &&
            event.data["armourPrevented"].number().value_or(0) > 0;
      });
  check(penetration_wire,
        "forge wire: Sling hit publishes exact Armour and penetration facts");
}

void test_tamar_vesselforge_service() {
  ProtocolSession session("guest-tamar-forge", "socket-tamar-forge", 0x5ea2u,
                          false);
  session.handle(Envelope{"dev:give", JsonValue::Object{
      {"itemId", "vessel-handaxe"}, {"qty", 1},
      {"itemLevel", 40}, {"seed", 37}}}, [](const Envelope&) {});
  auto before = request_state(session, "tamar-before");
  const std::string uuid = inventory_uuid_for(before, "vessel-handaxe");
  check(!uuid.empty(), "vesselforge service: carried vessel has an exact uuid");

  const auto find_detail = [&](const JsonValue& state) -> const JsonValue* {
    if (const auto* items = state["state"]["inventoryDetails"].array())
      for (const auto& item : *items)
        if (item["uuid"].string() && *item["uuid"].string() == uuid)
          return &item;
    return nullptr;
  };
  const JsonValue* before_item = find_detail(before);
  check(before_item && (*before_item)["vessel"]["item"]["brands"].array(),
        "vesselforge service: vessel identity is published before searing");
  const std::size_t brands_before =
      (*before_item)["vessel"]["item"]["brands"].array()->size();
  const int patience_before = static_cast<int>(
      (*before_item)["vessel"]["item"]["patience"].number().value_or(0));

  const auto action = [&](const std::string& action_id,
                          const std::string& ref,
                          const std::function<void(const Envelope&)>& emit) {
    session.handle(
        Envelope{"player:context-menu:action",
                 JsonValue::Object{{"queueItem",
                    JsonValue::Object{{"action", JsonValue::Object{{"actionId", action_id}}},
                                      {"item", JsonValue::Object{{"id", ref}, {"uuid", ref},
                                                                  {"price", 100}}}}}}},
        emit);
  };

  bool forged_from_afar = false;
  action("player:vesselforge:add-brand", uuid,
         [&](const Envelope&) { forged_from_afar = true; });
  auto after_far = request_state(session, "tamar-far");
  const JsonValue* far_item = find_detail(after_far);
  check(!forged_from_afar && far_item &&
            (*far_item)["vessel"]["item"]["brands"].array()->size() == brands_before &&
            (*far_item)["vessel"]["item"]["patience"].number().value_or(0) == patience_before,
        "vesselforge service: forged remote actions cannot mutate a vessel");

  session.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", 42}, {"y", 121}}},
                 [](const Envelope&) {});
  std::optional<Envelope> dialogue;
  session.handle(
      Envelope{"player:context-menu:action",
               JsonValue::Object{{"queueItem",
                  JsonValue::Object{{"action", JsonValue::Object{{"actionId", "player:npc:talk"}}},
                                    {"item", JsonValue::Object{{"id", 5}}}}}}},
      [&](const Envelope& event) {
        if (event.event == "open:screen") dialogue = event;
      });
  check(dialogue && dialogue->data["screen"].string() &&
            *dialogue->data["screen"].string() == "dialogue" &&
            dialogue->data["payload"]["npcKey"].string() &&
            *dialogue->data["payload"]["npcKey"].string() == "tamar-vesselwright" &&
            dialogue->data["payload"]["options"].array() &&
            dialogue->data["payload"]["options"].array()->size() == 1,
        "vesselforge service: Tamar opens a dedicated service conversation");

  std::optional<Envelope> opened;
  action("player:screen:vesselforge", "vesselforge",
         [&](const Envelope& event) {
           if (event.event == "open:screen") opened = event;
         });
  const JsonValue* open_row = nullptr;
  if (opened && opened->data["payload"]["items"].array())
    for (const auto& row : *opened->data["payload"]["items"].array())
      if (row["uuid"].string() && *row["uuid"].string() == uuid)
        open_row = &row;
  const int coins_before = opened
      ? static_cast<int>(opened->data["payload"]["carriedCoins"].number().value_or(0))
      : 0;
  check(opened && opened->data["screen"].string() &&
            *opened->data["screen"].string() == "vesselforge" && open_row &&
            (*open_row)["eligible"].boolean().value_or(false) &&
            (*open_row)["cost"].number().value_or(0) == 100 && coins_before >= 100 &&
            (*open_row)["bondCount"].number().value_or(-1) == 0 &&
            (*open_row)["attunement"].number().value_or(-1) == 0 &&
            (*open_row)["attunementNext"].number().value_or(0) == 80 &&
            (*open_row)["evolutions"].number().value_or(-1) == 0 &&
            !(*open_row)["awakened"].boolean().value_or(true),
        "vesselforge service: screen publishes capacity, purse, and living-item progress");

  bool saw_inventory = false;
  bool saw_message = false;
  std::optional<Envelope> refreshed_screen;
  action("player:vesselforge:add-brand", uuid,
         [&](const Envelope& event) {
           if (event.event == "core:refresh:inventory") saw_inventory = true;
           if (event.event == "game:send:message") saw_message = true;
           if (event.event == "open:screen" && event.data["screen"].string() &&
               *event.data["screen"].string() == "vesselforge")
             refreshed_screen = event;
         });
  const auto after = request_state(session, "tamar-after");
  const JsonValue* after_item = find_detail(after);
  check(after_item && saw_inventory && saw_message && refreshed_screen &&
            (*after_item)["vessel"]["item"]["brands"].array()->size() == brands_before + 1 &&
            (*after_item)["vessel"]["item"]["patience"].number().value_or(0) == patience_before - 1 &&
            refreshed_screen->data["payload"]["carriedCoins"].number().value_or(0) == coins_before - 100,
        "vesselforge service: one sear spends exact resources and refreshes the open service");
  check(after_item && (*after_item)["displayName"].string() &&
            (*after_item)["vessel"]["displayName"].string() &&
            *(*after_item)["displayName"].string() ==
                *(*after_item)["vessel"]["displayName"].string() &&
            (*after_item)["stats"]["attack"]["slash"].number().value_or(-1) ==
                (*after_item)["vessel"]["combat"]["ratings"]["attack"]["slash"].number().value_or(-2),
        "vesselforge service: refreshed identity and combat projection agree");
}

void test_worn_vessel_learns_from_cleared_expeditions() {
  ProtocolSession session("guest-living-vessel", "socket-living-vessel",
                          0xB04Du, false);
  session.handle(Envelope{"dev:give", JsonValue::Object{
      {"itemId", "vessel-handaxe"}, {"qty", 1},
      {"itemLevel", 40}, {"seed", 37}}}, [](const Envelope&) {});
  const auto granted = request_state(session, "living-vessel-granted");
  const std::string uuid = inventory_uuid_for(granted, "vessel-handaxe");
  check(!uuid.empty(), "living vessel wire: expedition gear has an exact uuid");
  session.handle(Envelope{"item:equip", JsonValue::Object{{
                     "item", JsonValue::Object{{"uuid", uuid}}}}},
                 [](const Envelope&) {});

  bool remembered = false;
  bool refreshed_wear = false;
  session.handle(Envelope{"instance:enterSolo", JsonValue::Object{
      {"template", "dungeon"}, {"layout", "warren"}}},
      [](const Envelope&) {});
  for (int clear = 0; clear < 5; ++clear) {
    session.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                   [&](const Envelope& event) {
                     if (event.event == "game:send:message" &&
                         event.data["text"].string() &&
                         event.data["text"].string()->find("remembers this road") !=
                             std::string::npos)
                       remembered = true;
                     if (event.event == "player:equippedAnItem")
                       refreshed_wear = true;
                   });
    if (clear != 4) {
      const auto floor = request_state(
          session, "living-vessel-floor-" + std::to_string(clear));
      const double down_x = floor["state"]["sceneMetadata"]["stairsDown"]["x"]
                                .number().value_or(0);
      const double down_y = floor["state"]["sceneMetadata"]["stairsDown"]["y"]
                                .number().value_or(0);
      session.handle(Envelope{"dev:teleport", JsonValue::Object{
                         {"x", down_x}, {"y", down_y}}},
                     [](const Envelope&) {});
    }
  }

  auto learned = request_state(session, "living-vessel-learned");
  const JsonValue& worn = learned["state"]["wearDetails"]["right_hand"];
  const auto* bonds = worn["vessel"]["item"]["bonds"].array();
  check(remembered && refreshed_wear && worn["uuid"].string() &&
            *worn["uuid"].string() == uuid,
        "living vessel wire: a floor clear refreshes the exact worn item");
  check(bonds && !bonds->empty() &&
            (*bonds)[0]["themeId"].string() &&
            *(*bonds)[0]["themeId"].string() == "warding" &&
            worn["vessel"]["item"]["evolutions"].number().value_or(0) >= 1,
        "living vessel wire: five real dungeon clears form a Warding Bond");
  check(worn["vessel"]["item"]["att"]["tc"]["warding"].number().value_or(0) == 10 &&
            worn["vessel"]["item"]["att"]["tc"]["slaughter"].number().value_or(0) == 5,
        "living vessel wire: classless road themes accumulate on the authoritative item");
  bool active_bond = false;
  if (const auto* lines = worn["vessel"]["lines"].array())
    for (const auto& line : *lines)
      if (line["section"].string() && *line["section"].string() == "bond" &&
          line["text"].string() &&
          line["text"].string()->find("BOND:") == 0)
        active_bond = true;
  check(active_bond,
        "living vessel wire: the protocol presents learned combat Bonds as active");

  session.replace_socket("socket-living-vessel-returned");
  learned = request_state(session, "living-vessel-reconnected");
  const JsonValue& returned = learned["state"]["wearDetails"]["right_hand"];
  check(returned["vessel"]["item"]["bonds"].array() &&
            !returned["vessel"]["item"]["bonds"].array()->empty(),
        "living vessel wire: Bond identity survives a reconnect to the same session");
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
  check(npcs && npcs->size() == 5,
        "crossroads: the accepted roster includes five authored townsfolk");
  bool saw_ludovicus = false;
  bool saw_selene = false;
  bool saw_rhea_services = false;
  bool saw_tamar_services = false;
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
      if (key == "tamar-vesselwright" && npc["services"].array() &&
          npc["services"].array()->size() == 2 &&
          npc["x"].number().value_or(0) == 42 &&
          npc["y"].number().value_or(0) == 121)
        saw_tamar_services = true;
      if (npc["name"].string() && npc["name"].string()->find("Mara") != std::string::npos)
        saw_retired_mara = true;
    }
  }
  check(saw_ludovicus && saw_selene && saw_rhea_services && saw_tamar_services &&
            !saw_retired_mara,
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
  {
    ProtocolSession guard("guest-road-depth-guard", "socket-road-depth-guard",
                          0x4a0eu, false);
    JsonValue::Object guarded_campaign;
    guarded_campaign["activeQuestIndex"] = JsonValue(4);
    guarded_campaign["objectiveIndex"] = JsonValue(0);
    guarded_campaign["questPoints"] = JsonValue(4);
    guarded_campaign["completed"] = JsonValue(JsonValue::Array{
        JsonValue("aldwyns-charge"), JsonValue("proof-of-temper"),
        JsonValue("the-pale-crown"), JsonValue("rot-in-the-reeds")});
    JsonValue::Object guarded_scion{
        {"id", "scion-road-depth-guard"}, {"name", "Tressa"},
        {"level", 1}, {"mortal", false},
        {"campaignQuests", JsonValue(guarded_campaign)}};
    JsonValue::Object guarded_house;
    guarded_house["id"] = JsonValue("house-road-depth-guard");
    guarded_house["name"] = JsonValue("House Rimegate");
    guarded_house["campaignComplete"] = JsonValue(false);
    guarded_house["clearedRoadNodes"] = JsonValue(JsonValue::Array{
        JsonValue("tin:1:0"), JsonValue("tin:2:0")});
    guarded_house["scions"] =
        JsonValue(JsonValue::Array{JsonValue(guarded_scion)});
    guarded_house["crypt"] = JsonValue(JsonValue::Array{});
    JsonValue::Object guarded_chronicle;
    guarded_chronicle["version"] = JsonValue(3);
    guarded_chronicle["houses"] =
        JsonValue(JsonValue::Array{JsonValue(guarded_house)});
    guard.handle(Envelope{"player:chronicles:save",
                          JsonValue::Object{{"state", JsonValue(guarded_chronicle)}}},
                 [](const Envelope&) {});
    guard.handle(Envelope{"player:chronicles:select",
                          JsonValue::Object{{"scionId", "scion-road-depth-guard"},
                                            {"houseId", "house-road-depth-guard"},
                                            {"scionName", "Tressa"},
                                            {"mortal", false}}},
                 [](const Envelope&) {});
    guard.handle(Envelope{"world:zone:enter",
                          JsonValue::Object{{"nodeId", "tin:3:0"}}},
                 [](const Envelope&) {});
    auto guarded_state = request_state(guard, "road-depth-guard-enter");
    check(guarded_state["state"]["sceneType"].string() &&
              *guarded_state["state"]["sceneType"].string() == "instance" &&
              guarded_state["state"]["quests"]["objectiveIndex"].number().value_or(-1) == 0,
          "campaign roads: an unlocked deeper holding cannot satisfy a shallower rite");
    guard.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                 [](const Envelope&) {});
    guard.handle(Envelope{"party:returnToTown", JsonValue::Object{}},
                 [](const Envelope&) {});
    guarded_state = request_state(guard, "road-depth-guard-return");
    check(guarded_state["state"]["quests"]["objectiveIndex"].number().value_or(-1) == 0,
          "campaign roads: deeper clear and return cannot forge exact-tier progress");
  }

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

  const auto check_warden_warning = [&](const JsonValue& snapshot,
                                         const char* skill,
                                         const char* shape,
                                         const char* channel, int radius,
                                         int inner_radius,
                                         bool targets_player) {
    const JsonValue* warden = nullptr;
    if (const auto* monsters = snapshot["state"]["monsters"].array()) {
      for (const auto& monster : *monsters)
        if (monster["boss"].boolean().value_or(false)) {
          warden = &monster;
          break;
        }
    }
    check(warden != nullptr,
          "warden mechanics: road instance publishes its Warden");
    if (!warden) return;
    const int boss_x = static_cast<int>((*warden)["x"].number().value_or(0));
    const int boss_y = static_cast<int>((*warden)["y"].number().value_or(0));
    const int player_x = boss_x + 1 < 39 ? boss_x + 1 : boss_x - 1;
    bool exact_warning = false;
    session.handle(
        Envelope{"dev:teleport",
                 JsonValue::Object{{"x", player_x}, {"y", boss_y}}},
        [&](const Envelope& event) {
          if (event.event != "monster:telegraph" ||
              !event.data["skillId"].string() ||
              *event.data["skillId"].string() != skill)
            return;
          const int expected_x = targets_player ? player_x : boss_x;
          exact_warning = event.data["shape"].string() &&
              *event.data["shape"].string() == shape &&
              event.data["damageChannel"].string() &&
              *event.data["damageChannel"].string() == channel &&
              event.data["radius"].number().value_or(-1) == radius &&
              event.data["innerRadius"].number().value_or(-1) == inner_radius &&
              event.data["x"].number().value_or(-1) == expected_x &&
              event.data["y"].number().value_or(-1) == boss_y;
        });
    check(exact_warning,
          "warden mechanics: road family emits exact authored warning geometry");
  };

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
  check_warden_warning(state, "boss:stonefall", "circle", "physical", 1,
                       0, true);
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

  struct RoadActStep {
    const char* road;
    const char* next_quest;
    const char* skill;
    const char* shape;
    const char* channel;
    int radius;
    int inner_radius;
    bool targets_player;
  };
  const RoadActStep remaining[] = {
      {"salt", "chalk-vigil", "boss:tidal-mark", "circle", "river", 2, 0,
       true},
      {"chalk", "copper-testament", "boss:grave-ring", "ring", "physical",
       4, 2, false},
      {"copper", nullptr, "boss:ember-crucible", "circle", "ember", 2, 0,
       false},
  };
  for (const auto& step : remaining) {
    session.handle(
        Envelope{"world:zone:enter",
                 JsonValue::Object{{"nodeId", std::string(step.road) + ":1:0"}}},
        [](const Envelope&) {});
    state = request_state(session, std::string("roads-") + step.road + "-entered");
    check(state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 1,
          "four-roads: each named road advances only on authoritative entry");
    check_warden_warning(state, step.skill, step.shape, step.channel,
                         step.radius, step.inner_radius, step.targets_player);
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
  check(!state["state"]["quests"]["campaignComplete"].boolean().value_or(true) &&
            state["state"]["quests"]["activeQuest"]["id"].string() &&
            *state["state"]["quests"]["activeQuest"]["id"].string() ==
                "quarry-saints-canon" &&
            state["state"]["quests"]["questPoints"].number().value_or(0) == 8 &&
            state["state"]["quests"]["houseRenown"].number().value_or(0) == 180 &&
            !state["state"]["endgame"]["unlocked"].boolean().value_or(true) &&
            state["state"]["quests"]["campaignQuestTotal"].number().value_or(0) == 23 &&
            state["state"]["quests"]["act"]["number"].number().value_or(0) == 3,
        "four-roads: Copper return opens Act III instead of prematurely sealing the campaign");

  struct CampaignRoadStep {
    const char* road;
    int tier;
    const char* holding;
    const char* warden;
    const char* next_quest;
  };
  const auto complete_road_commission =
      [&](const CampaignRoadStep& step, const std::string& prefix) {
    session.handle(
        Envelope{"world:zone:enter",
                 JsonValue::Object{{"nodeId", std::string(step.road) + ":" +
                                                  std::to_string(step.tier) +
                                                  ":0"}}},
        [](const Envelope&) {});
    state = request_state(session, prefix + "-entered");
    bool named_warden = false;
    int warden_level = 0;
    if (const auto* monsters = state["state"]["monsters"].array()) {
      for (const auto& monster : *monsters) {
        if (monster["boss"].boolean().value_or(false) &&
            monster["name"].string() &&
            *monster["name"].string() == step.warden) {
          named_warden = true;
          warden_level = static_cast<int>(
              monster["level"].number().value_or(0));
          break;
        }
      }
    }
    check(named_warden && state["state"]["sceneName"].string() &&
              *state["state"]["sceneName"].string() == step.holding,
          "campaign roads: exact tier entry reveals its named holding and Warden");
    check(state["state"]["sceneMetadata"]["depth"].number().value_or(0) ==
              step.tier,
          "campaign roads: road tier becomes authoritative instance depth");
    check(warden_level >= step.tier + 2,
          "campaign roads: deeper named Wardens inherit depth scaling");
    check(state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 1,
          "campaign roads: exact road and tier advance the entry rite");
    session.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                   [](const Envelope&) {});
    state = request_state(session, prefix + "-cleared");
    check(state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 2,
          "campaign roads: canonical Warden clear advances the commission");
    session.handle(Envelope{"party:returnToTown", JsonValue::Object{}},
                   [](const Envelope&) {});
    state = request_state(session, prefix + "-returned");
    if (step.next_quest) {
      check(state["state"]["quests"]["activeQuest"]["id"].string() &&
                *state["state"]["quests"]["activeQuest"]["id"].string() ==
                    step.next_quest,
            "campaign roads: living return opens the next named commission");
    }
  };

  const CampaignRoadStep deep_roads[] = {
      {"tin", 2, "Saint's Quarry", "The Quarry Saint", "brine-widows-tithe"},
      {"salt", 2, "Widow's Tithe", "The Brine Widow", "bell-beneath-chalk"},
      {"chalk", 2, "The Ossuary", "The Ossuary Bell", "cinder-judgment"},
      {"copper", 2, "Cinder Court", "The Cinder Judge", "iron-abbots-rule"},
  };
  for (const auto& step : deep_roads) {
    complete_road_commission(
        step, std::string("deep-roads-") + step.road);
    admit();
    state = request_state(
        session, std::string("deep-roads-") + step.road + "-restored");
    check(state["state"]["quests"]["activeQuest"]["id"].string() &&
              *state["state"]["quests"]["activeQuest"]["id"].string() ==
                  step.next_quest,
          "deep-roads: the completed commission persists across Scion admission");
  }
  check(!state["state"]["quests"]["campaignComplete"].boolean().value_or(true) &&
            state["state"]["quests"]["questPoints"].number().value_or(0) == 12 &&
            state["state"]["quests"]["houseRenown"].number().value_or(0) == 390 &&
            !state["state"]["endgame"]["unlocked"].boolean().value_or(true) &&
            state["state"]["quests"]["act"]["number"].number().value_or(0) == 4,
        "deep-roads: Cinder return opens the Crownless Marches without minting an endgame tablet");

  const CampaignRoadStep crownless_marches[] = {
      {"tin", 3, "The Iron Cloister", "The Iron Abbot", "drowned-factors-toll"},
      {"salt", 3, "The Drowned Ledger", "The Drowned Factor", "white-harrow"},
      {"chalk", 3, "Harrowfield", "The White Harrow", "ash-castellan"},
      {"copper", 3, "The Ashen Gate", "The Ash Castellan", "chain-regent"},
  };
  for (const auto& step : crownless_marches)
    complete_road_commission(
        step, std::string("crownless-") + step.road);
  check(state["state"]["quests"]["questPoints"].number().value_or(0) == 16 &&
            state["state"]["quests"]["houseRenown"].number().value_or(0) == 700 &&
            state["state"]["quests"]["act"]["number"].number().value_or(0) == 5,
        "crownless marches: four tier-three victories open the War of Claimants");

  const CampaignRoadStep war_of_claimants[] = {
      {"tin", 4, "Chainhold", "The Chain Regent", "mire-leviathan"},
      {"salt", 4, "Leviathan Mere", "The Mire Leviathan", "nameless-bishop"},
      {"chalk", 4, "The Nameless See", "The Nameless Bishop", "furnace-king"},
      {"copper", 4, "The Furnace Crown", "The Furnace King", "claim-of-iron"},
  };
  for (const auto& step : war_of_claimants)
    complete_road_commission(
        step, std::string("claimants-") + step.road);
  check(state["state"]["quests"]["questPoints"].number().value_or(0) == 20 &&
            state["state"]["quests"]["houseRenown"].number().value_or(0) == 1090 &&
            state["state"]["quests"]["act"]["number"].number().value_or(0) == 6,
        "war of claimants: tier-four victories open the three-part crown act");

  const CampaignRoadStep crown_claims[] = {
      {"tin", 5, "The Last Waystone", "The Last Mason", "claim-of-salt"},
      {"salt", 5, "The Queen's Ford", "The Flood-Tithe Queen", "crown-without-king"},
  };
  for (const auto& step : crown_claims)
    complete_road_commission(step, std::string("crown-") + step.road);

  session.handle(Envelope{"world:zone:enter",
                          JsonValue::Object{{"nodeId", "chalk:5:0"}}},
                 [](const Envelope&) {});
  state = request_state(session, "crown-chalk-entered");
  bool choir_present = false;
  if (const auto* monsters = state["state"]["monsters"].array())
    for (const auto& monster : *monsters)
      if (monster["boss"].boolean().value_or(false) &&
          monster["name"].string() &&
          *monster["name"].string() == "The Sepulchral Choir")
        choir_present = true;
  check(choir_present && state["state"]["sceneName"].string() &&
            *state["state"]["sceneName"].string() == "The Sepulchral Sanctum" &&
            state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 1,
        "verdigris crown: final commission begins at the Sepulchral Choir");
  session.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                 [](const Envelope&) {});
  session.handle(Envelope{"party:returnToTown", JsonValue::Object{}},
                 [](const Envelope&) {});
  state = request_state(session, "crown-chalk-returned");
  check(state["state"]["quests"]["activeQuest"]["id"].string() &&
            *state["state"]["quests"]["activeQuest"]["id"].string() ==
                "crown-without-king" &&
            state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 3,
        "verdigris crown: the Chalk seal persists inside the six-part finale");
  session.handle(Envelope{"world:zone:enter",
                          JsonValue::Object{{"nodeId", "copper:5:0"}}},
                 [](const Envelope&) {});
  state = request_state(session, "crown-copper-entered");
  bool usurper_present = false;
  if (const auto* monsters = state["state"]["monsters"].array())
    for (const auto& monster : *monsters)
      if (monster["boss"].boolean().value_or(false) &&
          monster["name"].string() &&
          *monster["name"].string() == "The Verdigris Usurper")
        usurper_present = true;
  check(usurper_present && state["state"]["sceneName"].string() &&
            *state["state"]["sceneName"].string() == "The Empty Throne" &&
            state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 4,
        "verdigris crown: the western throne reveals its canonical Usurper");
  session.handle(Envelope{"dev:clear-floor", JsonValue::Object{}},
                 [](const Envelope&) {});
  state = request_state(session, "crown-usurper-fallen");
  check(state["state"]["quests"]["objectiveIndex"].number().value_or(0) == 5,
        "verdigris crown: the Usurper must fall before the final living return");
  session.handle(Envelope{"party:returnToTown", JsonValue::Object{}},
                 [](const Envelope&) {});
  state = request_state(session, "campaign-sealed");
  check(state["state"]["quests"]["campaignComplete"].boolean().value_or(false) &&
            state["state"]["quests"]["activeQuest"].is_null() &&
            state["state"]["quests"]["questPoints"].number().value_or(0) == 23 &&
            state["state"]["quests"]["houseRenown"].number().value_or(0) == 1480 &&
            state["state"]["endgame"]["unlocked"].boolean().value_or(false) &&
            !inventory_map_uuid(state).empty(),
        "verdigris crown: the twenty-third return seals the campaign and awards the first tablet");

  std::string heir_id;
  session.handle(
      Envelope{"chronicles:scion:create",
               JsonValue::Object{{"houseId", "house-four-roads"},
                                 {"name", "Orla"}}},
      [&](const Envelope& event) {
        if (event.event == "chronicles:state" &&
            event.data["createdScionId"].string())
          heir_id = *event.data["createdScionId"].string();
      });
  check(!heir_id.empty(),
        "deep-roads: the sealed House can name a successor Scion");
  session.handle(
      Envelope{"player:chronicles:select",
               JsonValue::Object{{"scionId", heir_id},
                                 {"houseId", "house-four-roads"},
                                 {"scionName", "Orla"},
                                 {"mortal", false}}},
      [](const Envelope&) {});
  const auto heir_state = request_state(session, "deep-roads-heir");
  check(heir_state["state"]["quests"]["campaignComplete"].boolean().value_or(false) &&
            heir_state["state"]["quests"]["questPoints"].number().value_or(0) == 23 &&
            heir_state["state"]["quests"]["completed"].array() &&
            heir_state["state"]["quests"]["completed"].array()->size() == 23 &&
            heir_state["state"]["endgame"]["unlocked"].boolean().value_or(false),
        "verdigris crown: a new Scion inherits all campaign points and endgame access");
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
  check(inherited["state"]["quests"]["questPoints"].number().value_or(0) == 23 &&
            inherited["state"]["quests"]["completed"].array() &&
            inherited["state"]["quests"]["completed"].array()->size() == 23,
        "endgame: a legacy House seal inherits the expanded campaign budget");

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
  bool crown_warning = false;
  session.handle(Envelope{"dev:teleport",
                          JsonValue::Object{{"x", boss ? boss->operator[]("x").number().value_or(0) + 1 : 0},
                                            {"y", boss ? boss->operator[]("y").number().value_or(0) : 0}}},
                 [&](const Envelope& event) {
                   if (event.event == "monster:telegraph" &&
                       event.data["skillId"].string() &&
                       *event.data["skillId"].string() == "boss:grave-ring")
                     crown_warning = event.data["shape"].string() &&
                         *event.data["shape"].string() == "ring" &&
                         event.data["radius"].number().value_or(0) == 4 &&
                         event.data["innerRadius"].number().value_or(0) == 2 &&
                         event.data["x"].number().value_or(-1) ==
                             (boss ? (*boss)["x"].number().value_or(-2) : -2) &&
                         event.data["y"].number().value_or(-1) ==
                             (boss ? (*boss)["y"].number().value_or(-2) : -2);
                 });
  check(crown_warning,
        "endgame: Crown tablets carry the learned Grave Ring discipline");
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
    test_active_forge_properties_cross_the_protocol();
    test_tamar_vesselforge_service();
    test_worn_vessel_learns_from_cleared_expeditions();
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
