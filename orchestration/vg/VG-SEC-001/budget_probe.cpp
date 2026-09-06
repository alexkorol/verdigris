// One-off VG-SEC-001 measurement probe (not committed): prints the real byte
// size and container depth of the largest legitimate parsed payload class —
// a dev:state town snapshot with the walkable map — plus a deep-instance
// variant, to ground the budget values in REPORT.md.
#include "verdigris/networking.hpp"

#include <iostream>
#include <string>

using verdigris::networking::Envelope;
using verdigris::networking::JsonValue;
using verdigris::networking::ProtocolSession;

int max_depth(const JsonValue& value, int depth) {
  if (value.object()) {
    int best = depth;
    for (const auto& [key, child] : *value.object()) best = std::max(best, max_depth(child, depth + 1));
    return best;
  }
  if (value.array()) {
    int best = depth;
    for (const auto& child : *value.array()) best = std::max(best, max_depth(child, depth + 1));
    return best;
  }
  return depth;
}

std::size_t count_tokens(const JsonValue& value) {
  std::size_t tokens = 1;
  if (value.object())
    for (const auto& [key, child] : *value.object()) tokens += 1 + count_tokens(child);
  if (value.array())
    for (const auto& child : *value.array()) tokens += count_tokens(child);
  return tokens;
}

int main() {
  ProtocolSession session("guest-probe", "socket-probe", 17, true);
  session.handle(Envelope{"player:login", JsonValue::Object{{"useGuestAccount", true}, {"quickGuest", true}}},
                 [](const Envelope&) {});
  const std::string wire = session.state_payload("probe", /*include_map=*/true);
  JsonValue parsed;
  std::string error;
  if (!verdigris::networking::parse_json(wire, parsed, &error)) {
    std::cerr << "parse failed: " << error << "\n";
    return 1;
  }
  std::cout << "town snapshot with map: bytes=" << wire.size()
            << " depth=" << max_depth(parsed, 1)
            << " tokens=" << count_tokens(parsed) << "\n";
  session.handle(Envelope{"instance:enterSolo", JsonValue::Object{{"template", "dungeon"}, {"layout", "warren"}}},
                 [](const Envelope&) {});
  const std::string inst_wire = session.state_payload("probe-2", /*include_map=*/true);
  JsonValue inst_parsed;
  if (!verdigris::networking::parse_json(inst_wire, inst_parsed, &error)) {
    std::cerr << "parse failed: " << error << "\n";
    return 1;
  }
  std::cout << "instance snapshot with map: bytes=" << inst_wire.size()
            << " depth=" << max_depth(inst_parsed, 1)
            << " tokens=" << count_tokens(inst_parsed) << "\n";
  return 0;
}
