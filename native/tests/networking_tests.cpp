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

  session.handle(Envelope{"world:zone:enter", JsonValue::Object{{"nodeId", "tin:1:0"}}}, [&](const Envelope& event) {
    check(event.event == "world:scene:transition", "zone enter emits scene transition");
  });
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
  check(response.data["state"]["inventory"].array()->size() == 1, "dev give appears in inventory");

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
                 [&](const Envelope& event) { transition = event; });
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
}  // namespace

int main() {
  try {
    test_envelope_round_trip();
    test_session_lifecycle();
    test_continuous_movement();
    test_instance_entry_and_stairs();
    std::cout << "verdigris networking tests: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "verdigris networking tests: FAIL: " << error.what() << "\n";
    return 1;
  }
}
