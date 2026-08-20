// N7 entity-density benchmark (TASK-0065). Measurement only — does not change
// simulation rules. Spawns N monsters through Simulation::spawn_monster after
// entering tin:1:0, then dispatches 1000 Melee ticks and reports ticks/sec
// plus per-tick p99.
#include "verdigris/core.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using verdigris::ActionType;
using verdigris::ActorKind;
using verdigris::Command;
using verdigris::Simulation;
using verdigris::Vec2;
using verdigris::world_scale::kMeleeRange;

namespace {

struct Options {
  int n = 50;
  int run = 1;
  int ticks = 1000;
  std::uint64_t seed = 0;
  std::string out_path;
};

void usage() {
  std::cerr << "entity_density_bench --n N --run R [--ticks T] [--seed S] --out path.json\n";
}

bool parse_int(const char* text, int* out) {
  if (!text || !out) return false;
  char* end = nullptr;
  const long value = std::strtol(text, &end, 10);
  if (end == text || *end != '\0') return false;
  *out = static_cast<int>(value);
  return true;
}

bool parse_u64(const char* text, std::uint64_t* out) {
  if (!text || !out) return false;
  char* end = nullptr;
  const unsigned long long value = std::strtoull(text, &end, 10);
  if (end == text || *end != '\0') return false;
  *out = static_cast<std::uint64_t>(value);
  return true;
}

bool parse_args(int argc, char** argv, Options* options) {
  for (int i = 1; i < argc; ++i) {
    const std::string flag = argv[i];
    if (i + 1 >= argc) return false;
    const char* value = argv[++i];
    if (flag == "--n") {
      if (!parse_int(value, &options->n) || options->n < 1) return false;
    } else if (flag == "--run") {
      if (!parse_int(value, &options->run) || options->run < 1) return false;
    } else if (flag == "--ticks") {
      if (!parse_int(value, &options->ticks) || options->ticks < 1) return false;
    } else if (flag == "--seed") {
      if (!parse_u64(value, &options->seed)) return false;
    } else if (flag == "--out") {
      options->out_path = value;
    } else {
      return false;
    }
  }
  return !options->out_path.empty();
}

int count_alive_monsters(const Simulation& sim) {
  int count = 0;
  for (const auto& actor : sim.actors()) {
    if (actor.kind == ActorKind::Monster && actor.alive) ++count;
  }
  return count;
}

Vec2 melee_slot(int index) {
  // Pack into the melee contact band so the scripted Melee loop actually
  // resolves combat instead of swinging at empty air.
  const int ring = kMeleeRange - 1;
  const int x = (index % 3) - 1;
  const int y = ((index / 3) % 3) - 1;
  return {ring + x, y};
}

std::string json_escape(const std::string& value) {
  std::string out;
  for (char c : value) {
    if (c == '\\' || c == '"') {
      out.push_back('\\');
      out.push_back(c);
    } else {
      out.push_back(c);
    }
  }
  return out;
}

}  // namespace

int main(int argc, char** argv) {
  Options options;
  if (!parse_args(argc, argv, &options)) {
    usage();
    return 2;
  }
  if (options.seed == 0) {
    options.seed = 0xD1160000ULL + static_cast<std::uint64_t>(options.n) * 16ULL +
                   static_cast<std::uint64_t>(options.run);
  }

  Simulation sim(options.seed, "House Density");
  sim.dispatch(Command::enter("route:tin:1:0"));
  const auto* player = sim.actor(sim.scion().actor_id);
  if (!player || !sim.instance().active) {
    std::cerr << "entity_density_bench: failed to enter tin:1:0\n";
    return 1;
  }

  int spawned = 0;
  while (count_alive_monsters(sim) < options.n) {
    sim.spawn_monster(melee_slot(spawned), 1, false);
    ++spawned;
    if (spawned > options.n + 8) break;
  }
  const int monsters_start = count_alive_monsters(sim);

  std::vector<double> tick_ms;
  tick_ms.reserve(static_cast<std::size_t>(options.ticks));
  const auto wall_begin = std::chrono::steady_clock::now();
  for (int i = 0; i < options.ticks; ++i) {
    const auto tick_begin = std::chrono::steady_clock::now();
    sim.dispatch(Command::action_use(ActionType::Melee));
    const auto tick_end = std::chrono::steady_clock::now();
    tick_ms.push_back(std::chrono::duration<double, std::milli>(tick_end - tick_begin).count());
  }
  const auto wall_end = std::chrono::steady_clock::now();
  const double elapsed_s =
      std::chrono::duration<double>(wall_end - wall_begin).count();
  const double ticks_per_sec = elapsed_s > 0.0 ? options.ticks / elapsed_s : 0.0;

  std::vector<double> sorted = tick_ms;
  std::sort(sorted.begin(), sorted.end());
  const std::size_t p99_index =
      sorted.empty() ? 0
                     : (std::min)(sorted.size() - 1,
                                  static_cast<std::size_t>((sorted.size() * 99 + 99) / 100) - 1);
  const double p99_ms = sorted.empty() ? 0.0 : sorted[p99_index];
  const int monsters_end = count_alive_monsters(sim);

  std::ostringstream json;
  json.setf(std::ios::fixed);
  json.precision(6);
  json << "{\n"
       << "  \"task\": \"TASK-0065\",\n"
       << "  \"n\": " << options.n << ",\n"
       << "  \"run\": " << options.run << ",\n"
       << "  \"ticks\": " << options.ticks << ",\n"
       << "  \"seed\": " << options.seed << ",\n"
       << "  \"monsters_start\": " << monsters_start << ",\n"
       << "  \"monsters_end\": " << monsters_end << ",\n"
       << "  \"spawned_via_seam\": " << spawned << ",\n"
       << "  \"elapsed_s\": " << elapsed_s << ",\n"
       << "  \"ticks_per_sec\": " << ticks_per_sec << ",\n"
       << "  \"p99_tick_ms\": " << p99_ms << ",\n"
       << "  \"tick_ms\": " << 50 << ",\n"
       << "  \"host\": \"windows\",\n"
       << "  \"note\": \""
       << json_escape("core Simulation spawn_monster + Melee dispatch; JS has no matching spawn seam")
       << "\"\n"
       << "}\n";

  std::ofstream out(options.out_path);
  if (!out) {
    std::cerr << "entity_density_bench: cannot write " << options.out_path << "\n";
    return 1;
  }
  out << json.str();
  std::cout << json.str();
  if (monsters_start < options.n) return 1;
  return 0;
}
