#include "verdigris/networking.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
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
  check(parse_envelope("{\"event\":\"monster:state\",\"data\":[],\"meta\":{\"sceneId\":\"crypt:test\"}}", decoded, &error),
        "existing monster:state delta arrays round-trip without broadening requests");
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

void test_town_services_share_real_chronicles_arrival() {
  ProtocolSession session("town-service-arrival", "socket-court", 47, false);
  std::string house, scion;
  session.handle(Envelope{"chronicles:house:found", JsonValue::Object{{"name", "House of the Court"}}}, [&](const Envelope& e) {
    if (const auto* houses = e.data["chronicle"]["houses"].array(); houses && !houses->empty())
      if (const auto* id = houses->front()["id"].string()) house = *id;
  });
  check(!house.empty(), "town cluster fixture founds a real House");
  session.handle(Envelope{"chronicles:scion:create", JsonValue::Object{{"houseId", house}, {"name", "Court Walker"}}}, [&](const Envelope& e) {
    if (const auto* id = e.data["createdScionId"].string()) scion = *id;
  });
  check(!scion.empty(), "town cluster fixture creates a real Scion");
  auto set_out = [&]() {
    bool login = false;
    session.handle(Envelope{"chronicles:scion:set-out", JsonValue::Object{{"scionId", scion}}}, [&](const Envelope& e) {
      if (e.event != "player:login") return;
      login = e.data["player"]["x"].number().value_or(-1) == 38 &&
              e.data["player"]["y"].number().value_or(-1) == 116 &&
              e.data["player"]["chronicles"]["mortal"].boolean().value_or(false);
    });
    check(login, "real mortal set-out admits beside the fountain, independent of House wagon pitch");
  };
  set_out();
  const auto initial = request_state(session, "court-initial");
  const auto* npcs = initial["state"]["npcs"].array();
  check(npcs && npcs->size() == 4, "court retains the four existing services");
  const auto carried_before = initial["state"]["inventory"].stringify();
  set_out();
  check(request_state(session, "court-repeat")["state"]["inventory"].stringify() == carried_before,
        "central re-admission does not duplicate the Scion's kit or purse");
  session.handle(Envelope{"instance:enterSolo", JsonValue::Object{{"template", "dungeon"}, {"layout", "warren"}}}, [](const Envelope&) {});
  session.handle(Envelope{"player:extract", JsonValue::Object{}}, [](const Envelope&) {});
  check(!session.shared_world()->in_instance() && session.shared_world()->position().x == 38 &&
        session.shared_world()->position().y == 116, "entry-stairs extraction returns to the actual central admission origin");

  bool far_talk = false;
  const Envelope talk{"player:npc:talk", JsonValue::Object{{"item", JsonValue::Object{{"id", 1}}}}};
  session.handle(talk, [&](const Envelope& e) { if (e.event == "quest:update") far_talk = true; });
  check(!far_talk, "guide still requires contact; visibility is not interaction range");
  auto walk_to = [&](int x, int y) {
    const auto at = verdigris::tile_movement::occupied_tile(session.shared_world()->position());
    const int dx = x - at.x, dy = y - at.y;
    for (int step = 0; step < std::abs(dx) * 3; ++step)
      session.handle(Envelope{"player:move", JsonValue::Object{{"direction", dx < 0 ? "left" : "right"}}}, [](const Envelope&) {});
    for (int step = 0; step < std::abs(dy) * 3; ++step)
      session.handle(Envelope{"player:move", JsonValue::Object{{"direction", dy < 0 ? "up" : "down"}}}, [](const Envelope&) {});
    const auto arrived = verdigris::tile_movement::occupied_tile(session.shared_world()->position());
    check(arrived.x == x && arrived.y == y && !session.shared_world()->in_instance(),
          "existing movement rules reach each compact service on walkable town ground");
  };
  for (const auto& npc : *npcs) {
    const int id = static_cast<int>(npc["id"].number().value_or(0));
    const int x = static_cast<int>(npc["x"].number().value_or(0));
    const int y = static_cast<int>(npc["y"].number().value_or(0));
    const int distance = (std::max)(std::abs(x - 38), std::abs(y - 116));
    check(distance >= 3 && distance <= 6 && y <= 114 && session.shared_world()->grid().walkable_at(x, y),
          "services are north/side biased, walkable and three-to-six tiles from the open fountain-side arrival");
    walk_to(x, y + 1);
    JsonValue action;
    session.handle(Envelope{"player:context-menu:build", JsonValue::Object{
        {"miscData", JsonValue::Object{{"clickedOn", JsonValue::Object{{"0", "gameMap"}}}}},
        {"tile", JsonValue::Object{{"world", JsonValue::Object{{"x", x}, {"y", y}}}}}}}, [&](const Envelope& e) {
      if (const auto* entries = e.data["data"].array()) for (const auto& entry : *entries) {
        const auto* verb = entry["action"]["actionId"].string();
        if (entry["item"]["id"].number().value_or(-1) == id && verb && *verb != "player:npc:examine") action = entry;
      }
    });
    check(action.is_object(), "authoritative context menu uses the new service tile and original NPC ID");
    bool service_open = false;
    session.handle(Envelope{"player:context-menu:action", JsonValue::Object{{"queueItem", action}}}, [&](const Envelope& e) {
      if (id == 1) service_open |= e.event == "quest:update";
      else if (e.event == "open:screen") {
        const auto* screen = e.data["screen"].string();
        service_open |= screen && *screen == (id == 4 ? "bank" : "shop");
      }
    });
    check(service_open, "guide progression, both traders and storage retain their actual service behavior");
  }
}

void test_scion_appearance_survives_selection_and_account_save() {
  const auto file = std::filesystem::temp_directory_path() /
      ("verdigris-appearance-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".json");
  ProtocolSession session("appearance-account", "appearance-socket", 19, false);
  session.attach_persistence(file);
  std::string house;
  session.handle(Envelope{"chronicles:house:found", JsonValue::Object{{"name", "Appearance House"}}}, [&](const Envelope& e) {
    if (const auto* list = e.data["chronicle"]["houses"].array(); list && !list->empty()) house = *list->front()["id"].string();
  });
  auto create = [&](const char* name, const char* appearance) {
    std::string id;
    JsonValue::Object payload{{"houseId", house}, {"name", name}};
    if (appearance) payload["appearance"] = JsonValue(appearance);
    session.handle(Envelope{"chronicles:scion:create", std::move(payload)}, [&](const Envelope& e) {
      if (const auto* value = e.data["createdScionId"].string()) id = *value;
    });
    return id;
  };
  const auto female = create("Female scion", "female");
  const auto male = create("Original default", nullptr);
  check(!female.empty() && !male.empty(), "appearance creation uses real unique Scion records");
  auto select = [&](const std::string& id, const char* expected) {
    bool seen = false;
    session.handle(Envelope{"player:chronicles:select", JsonValue::Object{{"houseId", house}, {"scionId", id},
        {"scionName", "Selected"}, {"mortal", true}, {"appearance", "malicious-override"}}}, [&](const Envelope& e) {
      if (e.event == "player:login") seen = e.data["player"]["appearance"].string() && *e.data["player"]["appearance"].string() == expected;
    });
    check(seen, "selection publishes the saved appearance and ignores a new creation override");
  };
  select(male, "male");
  const auto baseline = request_state(session, "male");
  select(female, "female");
  const auto chosen = request_state(session, "female");
  check(chosen["state"]["appearance"].string() && *chosen["state"]["appearance"].string() == "female" &&
        chosen["state"]["hp"].stringify() == baseline["state"]["hp"].stringify() &&
        chosen["state"]["combat"].stringify() == baseline["state"]["combat"].stringify(),
        "authority snapshot exposes appearance without changing health/combat stats");
  bool moved = false, transitioned = false;
  session.handle(Envelope{"player:move", JsonValue::Object{{"direction", "down"}}}, [&](const Envelope& e) {
    if (e.event == "player:movement") moved = e.data["appearance"].string() && *e.data["appearance"].string() == "female";
  });
  session.handle(Envelope{"instance:enterSolo", JsonValue::Object{{"template", "dungeon"}, {"layout", "warren"}}}, [&](const Envelope& e) {
    if (e.event == "party:scene:transition") transitioned = e.data["playerState"]["appearance"].string() && *e.data["playerState"]["appearance"].string() == "female";
  });
  check(moved && transitioned, "movement and scene transition preserve selected appearance");
  session.persist();
  ProtocolSession loaded("appearance-account", "new-socket", 19, false);
  loaded.attach_persistence(file);
  JsonValue login;
  check(verdigris::networking::parse_json(loaded.login_payload(), login) &&
        login["player"]["appearance"].string() && *login["player"]["appearance"].string() == "female",
        "appearance survives a real durable account file and fresh ProtocolSession");
  std::ifstream in(file); const std::string text((std::istreambuf_iterator<char>(in)), {}); in.close();
  JsonValue legacy; check(verdigris::networking::parse_json(text, legacy), "saved appearance account is valid JSON");
  if (auto* chronicle = legacy.get("chronicle")) if (auto* houses_value = chronicle->get("houses"))
    if (auto* houses = houses_value->array()) for (auto& entry : *houses)
      if (auto* value = entry.get("scions")) if (auto* scions = value->array())
        for (auto& scion : *scions) if (scion.object()) scion.object()->erase("appearance");
  { std::ofstream old(file, std::ios::trunc); old << legacy.stringify(); }
  ProtocolSession old("appearance-account", "legacy-socket", 19, false); old.attach_persistence(file);
  check(verdigris::networking::parse_json(old.login_payload(), login) && *login["player"]["appearance"].string() == "male",
        "legacy persisted Scions without appearance default male");
  std::filesystem::remove(file);
}

void test_scion_creation_skips_persisted_living_and_crypt_ids() {
  const auto file = std::filesystem::temp_directory_path() /
      ("verdigris-scion-ids-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".json");
  ProtocolSession seed("saved-lineage", "seed-socket", 23, false);
  seed.attach_persistence(file);
  std::string house, probe;
  seed.handle(Envelope{"chronicles:house:found", JsonValue::Object{{"name", "Saved House"}}}, [&](const Envelope& e) {
    if (const auto* list = e.data["chronicle"]["houses"].array(); list && !list->empty()) house = *list->front()["id"].string();
  });
  seed.handle(Envelope{"chronicles:scion:create", JsonValue::Object{{"houseId", house}, {"name", "Counter probe"}}}, [&](const Envelope& e) {
    if (const auto* id = e.data["createdScionId"].string()) probe = *id;
  });
  check(probe.rfind("scion-", 0) == 0, "ID fixture obtains a real generated Scion ID");
  const auto next = std::stoull(probe.substr(6)) + 1;
  const std::string living_id = "scion-" + std::to_string(next);
  const std::string fallen_id = "scion-" + std::to_string(next + 1);
  const JsonValue living = JsonValue::Object{{"id", living_id}, {"name", "Existing male"}, {"appearance", "male"}};
  const JsonValue fallen = JsonValue::Object{{"id", fallen_id}, {"name", "Remembered female"}, {"appearance", "female"}};
  const JsonValue saved = JsonValue::Object{{"version", 3}, {"houses", JsonValue::Array{
      JsonValue::Object{{"id", house}, {"name", "Saved House"}, {"scions", JsonValue::Array{living}}, {"crypt", JsonValue::Array{}}},
      JsonValue::Object{{"id", "other-saved-house"}, {"name", "Other House"}, {"scions", JsonValue::Array{}}, {"crypt", JsonValue::Array{fallen}}}}}};
  seed.handle(Envelope{"player:chronicles:save", JsonValue::Object{{"state", saved}}}, [](const Envelope&) {});
  seed.persist();

  // Loading IDs ahead of the live counter reproduces the restart collision
  // without relying on this test's position or resetting production globals.
  ProtocolSession loaded("saved-lineage", "loaded-socket", 23, false);
  loaded.attach_persistence(file);
  std::string created;
  JsonValue updated;
  loaded.handle(Envelope{"chronicles:scion:create", JsonValue::Object{
      {"houseId", house}, {"name", "New female"}, {"appearance", "female"}}}, [&](const Envelope& e) {
    if (const auto* id = e.data["createdScionId"].string()) { created = *id; updated = e.data["chronicle"]; }
  });
  check(!created.empty() && created != living_id && created != fallen_id,
        "creation skips IDs in every persisted living and crypt roster");
  const auto* houses = updated["houses"].array();
  check(houses && houses->size() == 2, "creation preserves both saved Houses");
  const auto* scions = (*houses)[0]["scions"].array();
  const auto* crypt = (*houses)[1]["crypt"].array();
  check(scions && scions->size() == 2 && scions->front().stringify() == living.stringify() &&
        crypt && crypt->size() == 1 && crypt->front().stringify() == fallen.stringify(),
        "new creation neither overwrites a living identity nor reuses a fallen identity");
  check(scions->back()["id"].string() && *scions->back()["id"].string() == created &&
        scions->back()["appearance"].string() && *scions->back()["appearance"].string() == "female",
        "the returned ID identifies the newly requested female Scion");
  loaded.persist();
  std::filesystem::remove(file);
}

void test_set_out_resolves_the_saved_living_house() {
  ProtocolSession session("multi-house", "house-socket", 29, false);
  const JsonValue chronicle = JsonValue::Object{{"version", 3}, {"houses", JsonValue::Array{
      JsonValue::Object{{"id", "house-first"}, {"name", "First House"},
          {"scions", JsonValue::Array{JsonValue::Object{{"id", "first-female"}, {"name", "Iria"}, {"appearance", "female"}}}},
          {"crypt", JsonValue::Array{JsonValue::Object{{"id", "fallen-female"}, {"name", "Ancestor"}, {"appearance", "female"}}}}},
      JsonValue::Object{{"id", "house-second"}, {"name", "Second House"},
          {"scions", JsonValue::Array{JsonValue::Object{{"id", "second-male"}, {"name", "Taran"}, {"appearance", "male"}}}},
          {"crypt", JsonValue::Array{}}}}}};
  session.handle(Envelope{"player:chronicles:save", JsonValue::Object{{"state", chronicle}}}, [](const Envelope&) {});
  auto set_out = [&](const char* id, const char* expected_house, const char* appearance) {
    bool admitted = false;
    session.handle(Envelope{"chronicles:scion:set-out", JsonValue::Object{
        {"scionId", id}, {"houseId", "untrusted-house"}, {"appearance", "untrusted-appearance"}}}, [&](const Envelope& e) {
      if (e.event != "player:login") return;
      const auto& player = e.data["player"];
      admitted = player["chronicles"]["houseId"].string() && *player["chronicles"]["houseId"].string() == expected_house &&
          player["chronicles"]["scionId"].string() && *player["chronicles"]["scionId"].string() == id &&
          player["appearance"].string() && *player["appearance"].string() == appearance;
    });
    check(admitted, "set-out resolves the living Scion's saved House and appearance on the server");
  };
  set_out("second-male", "house-second", "male");
  set_out("first-female", "house-first", "female");
  const auto before = request_state(session, "before-invalid-admission");
  for (const char* id : {"fallen-female", "missing-scion", ""}) {
    bool admitted = false, explained = false;
    session.handle(Envelope{"chronicles:scion:set-out", JsonValue::Object{{"scionId", id}}}, [&](const Envelope& e) {
      admitted |= e.event == "player:login";
      explained |= e.event == "game:send:message";
    });
    const auto after = request_state(session, "after-invalid-admission");
    check(!admitted && explained && after["state"]["chronicles"].stringify() == before["state"]["chronicles"].stringify() &&
          after["state"]["appearance"].stringify() == before["state"]["appearance"].stringify() &&
          after["state"]["inventory"].stringify() == before["state"]["inventory"].stringify(),
          "fallen, missing and empty IDs cannot change admission, appearance or grant a new kit");
  }
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

void test_authoritative_dash_and_remote_controls() {
  using namespace verdigris;
  WorldSimulation world(42, "dash-core");
  world.set_spawn_suppressed(true);
  world.enter_solo_instance("forest", "clearings");
  world.teleport(7, 7, 900);
  const auto from = world.position();
  check(world.dash("right", 1000), "dash travels an open authority route");
  const auto first = world.position();
  check(std::abs(first.x - from.x - kDashMovementTicks * tile_movement::kMoveDistance) < 0.00001 &&
        first.y == from.y, "dash distance is exactly ten rounded normal samples");
  check(world.last_step().action == "dash" && world.last_step().from.x == from.x &&
        world.last_step().duration_ms == 50, "dash publishes its real origin and one-sample display duration");
  const auto sequence = world.last_step().sequence;
  check(!world.dash("right", 1499) && world.position().x == first.x &&
        world.last_step().sequence == sequence, "dash cooldown rejection does not move or publish an action");
  check(world.dash("down", 1500), "dash cooldown opens at the exact shared 500ms boundary");
  world.teleport(7, 7, 2000);
  check(world.dash("down-right", 2000) &&
        std::abs(std::hypot(world.position().x - 7, world.position().y - 7) -
                 kDashMovementTicks * tile_movement::kMoveDistance) < 0.00001,
        "diagonal dash preserves normalized distance");
  WorldSimulation walls(43, "dash-walls");
  walls.set_spawn_suppressed(true);
  walls.enter_solo_instance("dungeon", "warren");
  walls.teleport(10, 14, 0); // rib x12 is between two open endpoints
  check(walls.grid().walkable_at(10, 14) && walls.grid().walkable_at(13, 14) &&
        !walls.grid().walkable_at(12, 14), "wall fixture has an interior blocker with open endpoints");
  const auto before = walls.last_step().sequence;
  check(!walls.dash("right", 1000) && walls.position().x == 10 &&
        walls.last_step().sequence == before, "swept dash cannot skip an interior wall");
  check(walls.dash("down", 1000), "blocked dash consumes neither cooldown nor distance");

  ProtocolSession session("dash-wire", "socket-dash", 42, false);
  session.shared_world()->set_spawn_suppressed(true);
  session.handle(Envelope{"instance:enterSolo", JsonValue::Object{{"template", "forest"}, {"layout", "clearings"}}}, [](const Envelope&) {});
  session.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", 7}, {"y", 7}}}, [](const Envelope&) {});
  const auto stats_before = request_state(session, "dash-before");
  int moves = 0, hits = 0;
  Envelope movement;
  auto capture = [&](const Envelope& event) {
    if (event.event == "player:movement") { ++moves; movement = event; }
    if (event.event == "combat:hit") ++hits;
  };
  const Envelope dash{"player:skill:trigger", JsonValue::Object{{"skillId", "dash"}, {"direction", "right"}}};
  session.handle(dash, capture);
  check(moves == 1 && hits == 0 && movement.meta &&
        (*movement.meta)["action"].string() && *(*movement.meta)["action"].string() == "dash" &&
        (*movement.meta)["fromX"].number().value_or(-1) == 7 &&
        movement.data["x"].number().value_or(0) > 10.3,
        "skill dash emits accepted travel metadata and never an attack hit");
  session.handle(dash, capture);
  check(moves == 1 && hits == 0, "repeat wire dash in cooldown emits no accepted action");
  const auto stats_after = request_state(session, "dash-after");
  check(stats_before["state"]["hp"]["current"].number() == stats_after["state"]["hp"]["current"].number(),
        "dash adds no life or invulnerability substitution");
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

void test_crypt_pursuit_publishes_exact_authority() {
  ProtocolSession session("guest-crypt-pursuit", "socket-crypt", 79, false);
  std::vector<Envelope> published;
  session.set_direct_emit([&](const Envelope& envelope) { published.push_back(envelope); });
  session.handle(Envelope{"instance:enterSolo", JsonValue::Object{{"template", "crypt"}, {"layout", "warren"}}},
                 [](const Envelope&) {});
  auto world = session.shared_world();
  check(world->metadata().theme == "crypt", "pursuit transport enters authored crypt");
  const auto& grid = world->grid();
  verdigris::Vec2 player_tile{};
  bool corridor = false;
  for (int y = 4; y < grid.height - 3 && !corridor; ++y) {
    for (int x = 3; x < grid.width - 4 && !corridor; ++x) {
      bool open = true;
      for (int oy = -2; oy <= 0; ++oy)
        for (int ox = 0; ox <= 2; ++ox) open = open && grid.walkable_at(x + ox, y + oy);
      const auto up = world->metadata().stairs_up;
      const auto down = world->metadata().stairs_down;
      if (open && (x != up.x || y != up.y) && (x != down.x || y != down.y)) {
        player_tile = {x, y}; corridor = true;
      }
    }
  }
  check(corridor, "pursuit transport finds real open crypt corridor");
  auto& monsters = const_cast<std::vector<verdigris::WorldMonster>&>(world->monsters());
  std::string id;
  for (auto& monster : monsters) {
    if (id.empty() && !monster.boss && monster.behaviour_type == "melee") id = monster.uuid;
    const bool chosen = monster.uuid == id;
    monster.continuous_position = chosen
        ? verdigris::WorldPosition{double(player_tile.x + 2), double(player_tile.y - 2)}
        : verdigris::WorldPosition{-1000.0 - double(&monster - monsters.data()) * 2.0, -1000.0};
    monster.has_continuous_position = true;
    monster.x = int(monster.continuous_position.x); monster.y = int(monster.continuous_position.y);
    monster.movement_from = monster.continuous_position;
    monster.pursuit_home = monster.continuous_position;
  }
  check(!id.empty(), "pursuit transport uses authored melee wight without stat changes");
  session.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", player_tile.x}, {"y", player_tile.y}}},
                 [](const Envelope&) {});
  const auto base = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  bool moved = false, fractional = false, hit = false;
  std::uint64_t previous_sequence = 0;
  int movement_packets = 0;
  for (int tick = 0; tick < 40 && !hit; ++tick) {
    published.clear(); session.tick(base + tick * 150);
    const verdigris::WorldMonster* authority = nullptr;
    for (const auto& monster : world->monsters()) if (monster.uuid == id) authority = &monster;
    check(authority != nullptr, "pursuit transport retains the authority actor");
    for (const auto& envelope : published) {
      check(envelope.event != "dev:state", "ordinary tick sends no full snapshot");
      if (envelope.event == "monster:state") {
        check(envelope.meta && (*envelope.meta)["sceneId"].string() &&
              *(*envelope.meta)["sceneId"].string() == world->scene_id(),
              "monster deltas are scoped to the actual scene");
        for (const auto& actor : *envelope.data.array()) {
          if (!actor["uuid"].string() || *actor["uuid"].string() != id) continue;
          const auto precise = authority->world_position();
          const double x = actor["x"].number().value_or(-10000);
          const double y = actor["y"].number().value_or(-10000);
          check(std::abs(x - precise.x) < 1e-9 && std::abs(y - precise.y) < 1e-9,
                "monster packet carries precise authority coordinates");
          const auto seq = std::uint64_t(actor["movementStep"]["sequence"].number().value_or(0));
          check(seq >= previous_sequence, "monster sequence is monotonic");
          if (seq > previous_sequence) {
            previous_sequence = seq; ++movement_packets;
            moved = moved || x < player_tile.x + 2;
            fractional = fractional || std::abs(x - std::round(x)) > 1e-6;
          }
        }
      }
      if (envelope.event == "combat:hit" && envelope.data["attackerId"].string() &&
          *envelope.data["attackerId"].string() == id &&
          envelope.data["targetId"].string() && *envelope.data["targetId"].string() == session.identity() &&
          envelope.data["amount"].number().value_or(0) > 0) hit = true;
    }
  }
  check(moved && fractional && movement_packets >= 2,
        "ordinary server ticks publish multiple fractional pursuit segments");
  check(hit, "authored wight arrives and deals real incoming damage");
  JsonValue snapshot;
  check(verdigris::networking::parse_json(session.state_payload("after-pursuit"), snapshot),
        "pursuit snapshot can be reconciled");
  bool exact_snapshot = false;
  for (const auto& monster : *snapshot["state"]["monsters"].array()) {
    if (monster["uuid"].string() && *monster["uuid"].string() == id)
      exact_snapshot = monster["movementStep"]["sequence"].number().value_or(-1) == previous_sequence;
  }
  check(exact_snapshot, "snapshot and delta use the same movement sequence");
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
  session.handle(Envelope{"dev:teleport", JsonValue::Object{{"x", mx + 1.0}, {"y", static_cast<double>(my)}}},
                 [](const Envelope&) {});
  bool kill_loot = false;
  const auto observe_loot = [&](const Envelope& event) {
    if (event.event != "item:change") return;
    if (const auto* items = ground_list_from_change(event)) {
      for (const auto& item : *items) {
        if (item["id"].string() && *item["id"].string() == "coins"
            && ground_item_has_fields(item)) {
          kill_loot = true;
        }
      }
    }
  };
  session.set_direct_emit(observe_loot);
  session.handle(Envelope{"dev:forcecritical", JsonValue::Object{}}, [](const Envelope&) {});
  session.handle(Envelope{"player:skill:trigger", JsonValue::Object{{"direction", "left"}}},
                 observe_loot);
  // Drive the server's existing clock seam at the real attack cadence.
  // Forty commands in one instant must not substitute for elapsed recovery.
  const auto started_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  for (int swing = 1; swing < 40 && !kill_loot; ++swing) {
    session.handle(Envelope{"dev:forcecritical", JsonValue::Object{}}, [](const Envelope&) {});
    session.tick(started_ms + swing * 350);
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

  const auto exit = extract_session.shared_world()->metadata().stairs_up;
  extract_session.shared_world()->teleport(20, 20, 0);
  bool far_summary = false;
  extract_session.handle(Envelope{"player:extract", JsonValue::Object{}}, [&](const Envelope& e) {
    if (e.event == "player:extract") far_summary = true;
  });
  check(!far_summary && extract_session.shared_world()->in_instance(),
        "server rejects extraction away from exit stairs");
  extract_session.shared_world()->teleport(exit.x + 1, exit.y, 0);
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
}  // namespace

int main() {
  try {
    test_envelope_round_trip();
    test_session_lifecycle();
    test_town_services_share_real_chronicles_arrival();
    test_scion_appearance_survives_selection_and_account_save();
    test_scion_creation_skips_persisted_living_and_crypt_ids();
    test_set_out_resolves_the_saved_living_house();
    test_continuous_movement();
    test_authoritative_dash_and_remote_controls();
    test_instance_entry_and_stairs();
    test_crypt_pursuit_publishes_exact_authority();
    test_n3_combat_rules_and_wire_events();
    test_gate_a_ground_login_and_kill_loot();
    test_gate_a_extract_and_stairs();
    test_gate_a_equip_totals_and_unknown_uuid();
    std::cout << "verdigris networking tests: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "verdigris networking tests: FAIL: " << error.what() << "\n";
    return 1;
  }
}
