#include "verdigris/networking.hpp"

#include <iostream>
#include <stdexcept>

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
  check(decoded.data["guestId"].string().value_or("") == "roundtrip-guest", "string payload survives round-trip");
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
  check(response.data["requestId"].string().value_or("") == "state-1", "state request id is echoed");
  check(response.data["state"]["sceneType"].string().value_or("") == "instance", "state reports active instance");
  check(response.data["state"]["monsters"].array() && !response.data["state"]["monsters"].array()->empty(), "instance state has a monster");

  session.handle(Envelope{"dev:give", JsonValue::Object{{"itemId", "garnet-amulet"}, {"qty", 1}}}, [](const Envelope&) {});
  session.handle(Envelope{"dev:state", JsonValue::Object{{"requestId", "state-2"}}}, [&](const Envelope& event) { state_wire = emit_envelope(event); });
  check(parse_envelope(state_wire, response), "state after grant parses");
  check(response.data["state"]["inventory"].array()->size() == 1, "dev give appears in inventory");

  session.replace_socket("socket-b");
  check(session.state_payload("state-3").find("socket-b") != std::string::npos, "replacement binds the new socket");
}
}  // namespace

int main() {
  try {
    test_envelope_round_trip();
    test_session_lifecycle();
    std::cout << "verdigris networking tests: PASS\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "verdigris networking tests: FAIL: " << error.what() << "\n";
    return 1;
  }
}
