#include "verdigris/networking.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
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
}  // namespace

int main() {
  try {
    test_envelope_round_trip();
    test_session_lifecycle();
    test_continuous_movement();
    test_instance_entry_and_stairs();
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
