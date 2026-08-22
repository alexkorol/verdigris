// TASK-0152 native density benchmark evidence (supersedes the TASK-0065 form).
//
// Purpose: turn the entity-density measurement into reproducible evidence for
// the owner-visible encounter and presentation waves. Measurement only — the
// simulation rules, gameplay, and presentation are untouched.
//
// Evidence contract (schema "verdigris-density-evidence/2"):
//   - Fixed scenario id "density-melee-contact": enter route tin:1:0, pack N
//     monsters into the melee contact band through Simulation::spawn_monster,
//     then drive T scripted Melee frames at the fixed 20 Hz cadence budget.
//   - Every invocation executes the seeded scenario TWICE in fresh Simulation
//     instances and requires both runs to agree on counts and the FNV-1a64
//     state checksum. Any disagreement fails the run (exit 1).
//   - Frame timing = one dispatch(Melee) plus the read-only presentation
//     state pass (actors/instance/counters scan feeding the rolling state
//     checksum). Update timing = the dispatch alone. Both are reported as
//     p50/p90/p99/max/mean milliseconds (nearest-rank percentiles).
//   - Complete hardware/build provenance is mandatory.
//
// Threshold contract "verdigris-density-threshold/1" (enforced on every run
// and re-enforced by --validate):
//   monsters_spawned            >= n          (spawn seam delivered density)
//   samples_complete            == ticks      (no dropped samples)
//   reproducible                == 1          (double-run counts+checksum)
//   frame_p99_within_budget     <= 50 ms      (one fixed-step frame budget)
//   frame_mean_within_budget    <= 50 ms
//   update_p99_within_budget    <= 50 ms
//   ticks_per_sec_floor         >= 20         (real-time simulation floor)
//
// Exit codes: 0 = evidence complete and passing; 1 = invalid, incomplete,
// irreproducible, threshold-failing, or unwritable evidence; 2 = usage error.
// `--validate <file>` re-checks any stored capture against the full schema,
// provenance completeness, percentile sanity, and threshold contract.
#include "verdigris/core.hpp"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

using verdigris::ActionType;
using verdigris::ActorKind;
using verdigris::Command;
using verdigris::Simulation;
using verdigris::Vec2;
using verdigris::world_scale::kMeleeRange;

namespace {

constexpr const char* kSchema = "verdigris-density-evidence/2";
constexpr const char* kTask = "TASK-0152";
constexpr const char* kToolVersion = "2.0.0";
constexpr const char* kScenarioId = "density-melee-contact";
constexpr const char* kRouteId = "route:tin:1:0";
constexpr const char* kThresholdContract = "verdigris-density-threshold/1";
constexpr const char* kRequiredCheckIds[] = {
    "monsters_spawned",
    "samples_complete",
    "reproducible",
    "frame_p99_within_budget",
    "frame_mean_within_budget",
    "update_p99_within_budget",
    "ticks_per_sec_floor",
};
constexpr std::size_t kRequiredCheckCount =
    sizeof(kRequiredCheckIds) / sizeof(kRequiredCheckIds[0]);
constexpr double kTickBudgetMs = 50.0;
constexpr double kTicksPerSecFloor = 20.0;

struct Options {
  int n = 50;
  int run = 1;
  int ticks = 1000;
  std::uint64_t seed = 0;
  std::string out_path;
  std::string scenario_id = kScenarioId;
  std::string git_ref_arg;
  std::string validate_path;
};

void usage() {
  std::cerr << "usage:\n"
            << "  entity_density_bench --n N --run R [--ticks T] [--seed S]"
            << " [--scenario " << kScenarioId << "] [--git-ref REF] --out path.json\n"
            << "  entity_density_bench --validate path.json\n";
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
  bool have_out = false;
  for (int i = 1; i < argc; ++i) {
    const std::string flag = argv[i];
    if (flag == "--validate") {
      if (i + 1 >= argc) return false;
      options->validate_path = argv[++i];
      return true;
    }
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
    } else if (flag == "--scenario") {
      options->scenario_id = value;
    } else if (flag == "--git-ref") {
      options->git_ref_arg = value;
    } else if (flag == "--out") {
      options->out_path = value;
      have_out = true;
    } else {
      return false;
    }
  }
  return have_out;
}

std::uint64_t resolve_seed(const Options& options) {
  if (options.seed != 0) return options.seed;
  return 0xD1160000ULL + static_cast<std::uint64_t>(options.n) * 16ULL +
         static_cast<std::uint64_t>(options.run);
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

// ──────────────────────────────────────────────────────────────────────────
// Deterministic state checksum (FNV-1a 64) over the live instance surface:
// tick, instance state, every actor's identity/stats/position/effects, the
// scion, and the event/ground/legend counter surfaces.
// ──────────────────────────────────────────────────────────────────────────
class StateChecksum {
 public:
  void mix_u64(std::uint64_t value) {
    for (int shift = 56; shift >= 0; shift -= 8) {
      mix_byte(static_cast<std::uint8_t>((value >> shift) & 0xFFU));
    }
  }
  void mix_i64(std::int64_t value) { mix_u64(static_cast<std::uint64_t>(value)); }
  void mix_bool(bool value) { mix_byte(value ? 1U : 0U); }
  void mix_text(std::string_view text) {
    mix_u64(static_cast<std::uint64_t>(text.size()));
    for (char c : text) mix_byte(static_cast<std::uint8_t>(c));
  }

  std::string hex() const {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "fnv1a64:%016llx",
                  static_cast<unsigned long long>(hash_));
    return buf;
  }

 private:
  void mix_byte(std::uint8_t byte) {
    hash_ ^= byte;
    hash_ *= 0x100000001b3ULL;
  }
  std::uint64_t hash_ = 0xcbf29ce484222325ULL;
};

StateChecksum checksum_state(const Simulation& sim) {
  StateChecksum checksum;
  checksum.mix_u64(sim.tick());
  const auto& instance = sim.instance();
  checksum.mix_bool(instance.active);
  checksum.mix_text(instance.route_id);
  checksum.mix_i64(static_cast<std::int64_t>(instance.phase));
  checksum.mix_i64(instance.extraction_point.x);
  checksum.mix_i64(instance.extraction_point.y);
  for (const auto& actor : sim.actors()) {
    checksum.mix_text(actor.id);
    checksum.mix_i64(static_cast<std::int64_t>(actor.kind));
    checksum.mix_bool(actor.alive);
    checksum.mix_i64(actor.position.x);
    checksum.mix_i64(actor.position.y);
    checksum.mix_i64(actor.stats.level);
    checksum.mix_i64(actor.stats.life);
    checksum.mix_i64(actor.stats.resource);
    checksum.mix_i64(actor.stats.attack);
    checksum.mix_i64(actor.stats.defense);
    checksum.mix_i64(actor.cooldown_ticks);
    checksum.mix_bool(actor.elite);
    checksum.mix_i64(actor.facing.x);
    checksum.mix_i64(actor.facing.y);
    checksum.mix_i64(static_cast<std::int64_t>(actor.pending_action));
    checksum.mix_i64(actor.pending_action_ticks);
  }
  const auto& scion = sim.scion();
  checksum.mix_text(scion.id);
  checksum.mix_bool(scion.alive);
  checksum.mix_i64(scion.level);
  checksum.mix_u64(sim.events().size());
  checksum.mix_u64(sim.ground_items().size());
  checksum.mix_u64(sim.ground_trophies().size());
  checksum.mix_u64(sim.legends().size());
  return checksum;
}

// ──────────────────────────────────────────────────────────────────────────
// Seeded scenario execution.
// ──────────────────────────────────────────────────────────────────────────
struct Counts {
  int monsters_start = 0;
  int monsters_end = 0;
  int spawned_via_seam = 0;
  std::uint64_t events = 0;
  std::uint64_t final_tick = 0;

  bool operator==(const Counts& other) const {
    return monsters_start == other.monsters_start &&
           monsters_end == other.monsters_end &&
           spawned_via_seam == other.spawned_via_seam && events == other.events &&
           final_tick == other.final_tick;
  }
};

struct RunOutcome {
  Counts counts;
  std::string state_checksum;
  std::vector<double> update_ms;
  std::vector<double> frame_ms;
  double elapsed_s = 0.0;
};

using SteadyClock = std::chrono::steady_clock;

double millis_between(SteadyClock::time_point begin, SteadyClock::time_point end) {
  return std::chrono::duration<double, std::milli>(end - begin).count();
}

RunOutcome execute_scenario(const Options& options, std::uint64_t seed) {
  RunOutcome outcome;
  Simulation sim(seed, "House Density");
  sim.dispatch(Command::enter(kRouteId));
  const auto* player = sim.actor(sim.scion().actor_id);
  if (!player || !sim.instance().active) {
    std::cerr << "entity_density_bench: failed to enter tin:1:0\n";
    std::exit(1);
  }

  int spawned = 0;
  while (count_alive_monsters(sim) < options.n) {
    sim.spawn_monster(melee_slot(spawned), 1, false);
    ++spawned;
    if (spawned > options.n + 8) break;
  }
  outcome.counts.monsters_start = count_alive_monsters(sim);
  outcome.counts.spawned_via_seam = spawned;

  outcome.update_ms.reserve(static_cast<std::size_t>(options.ticks));
  outcome.frame_ms.reserve(static_cast<std::size_t>(options.ticks));
  StateChecksum checksum;
  const auto wall_begin = SteadyClock::now();
  for (int i = 0; i < options.ticks; ++i) {
    const auto frame_begin = SteadyClock::now();
    sim.dispatch(Command::action_use(ActionType::Melee));
    const auto update_end = SteadyClock::now();
    checksum = checksum_state(sim);
    const auto frame_end = SteadyClock::now();
    outcome.update_ms.push_back(millis_between(frame_begin, update_end));
    outcome.frame_ms.push_back(millis_between(frame_begin, frame_end));
  }
  const auto wall_end = SteadyClock::now();

  outcome.elapsed_s = std::chrono::duration<double>(wall_end - wall_begin).count();
  outcome.counts.monsters_end = count_alive_monsters(sim);
  outcome.counts.events = sim.events().size();
  outcome.counts.final_tick = sim.tick();
  outcome.state_checksum = checksum.hex();
  return outcome;
}

// ──────────────────────────────────────────────────────────────────────────
// Percentiles: nearest-rank over ascending-sorted samples.
// ──────────────────────────────────────────────────────────────────────────
struct Percentiles {
  double p50 = 0.0;
  double p90 = 0.0;
  double p99 = 0.0;
  double max = 0.0;
  double mean = 0.0;
};

Percentiles summarize(const std::vector<double>& samples) {
  Percentiles result;
  if (samples.empty()) return result;
  std::vector<double> sorted = samples;
  std::sort(sorted.begin(), sorted.end());
  auto nearest_rank = [&sorted](double pct) {
    const std::size_t rank = static_cast<std::size_t>(
        std::ceil(pct / 100.0 * static_cast<double>(sorted.size())));
    const std::size_t index =
        rank == 0 ? 0 : (rank > sorted.size() ? sorted.size() - 1 : rank - 1);
    return sorted[index];
  };
  result.p50 = nearest_rank(50.0);
  result.p90 = nearest_rank(90.0);
  result.p99 = nearest_rank(99.0);
  result.max = sorted.back();
  double total = 0.0;
  for (double value : sorted) total += value;
  result.mean = total / static_cast<double>(sorted.size());
  return result;
}

// ──────────────────────────────────────────────────────────────────────────
// Hardware/build provenance.
// ──────────────────────────────────────────────────────────────────────────
std::string trim(const std::string& value) {
  const auto begin = value.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) return {};
  const auto end = value.find_last_not_of(" \t\r\n");
  return value.substr(begin, end - begin + 1);
}

const char* safe_getenv(const char* name) {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
  const char* value = std::getenv(name);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
  return value;
}

std::string compiler_name() {
#if defined(_MSC_VER)
  return "msvc-" + std::to_string(_MSC_FULL_VER);
#elif defined(__clang__)
  return std::string("clang-") + __clang_version__;
#elif defined(__GNUC__)
  return std::string("gcc-") + __VERSION__;
#else
  return "unknown-cpp-compiler";
#endif
}

std::string cxx_standard_name() {
#if defined(_MSVC_LANG)
#if _MSVC_LANG >= 202302L
  return "c++23";
#elif _MSVC_LANG >= 202002L
  return "c++20";
#elif _MSVC_LANG >= 201703L
  return "c++17";
#else
  return "pre-c++17";
#endif
#elif defined(__cplusplus)
#if __cplusplus >= 202302L
  return "c++23";
#elif __cplusplus >= 202002L
  return "c++20";
#elif __cplusplus >= 201703L
  return "c++17";
#else
  return "pre-c++17";
#endif
#else
  return "pre-c++17";
#endif
}

std::string build_config_name() {
#ifdef NDEBUG
  return "release";
#else
  return "debug-no-ndebug";
#endif
}

std::string os_name() {
#if defined(_WIN32)
  const char* env = safe_getenv("OS");
  return env && *env ? std::string(env) : std::string("Windows");
#elif defined(__APPLE__)
  return "macos";
#elif defined(__linux__)
  return "linux";
#else
  return "unknown-os";
#endif
}

std::string arch_name() {
#if defined(_M_X64) || defined(__x86_64__)
  return "x86_64";
#elif defined(_M_ARM64) || defined(__aarch64__)
  return "arm64";
#elif defined(_M_ARM64EC)
  return "arm64ec";
#elif defined(_M_IX86) || defined(__i386__)
  return "x86";
#else
  return "unknown-arch";
#endif
}

#if defined(_MSC_VER)
std::string cpu_brand_msvc() {
  std::int32_t info[4] = {0, 0, 0, 0};
  __cpuid(info, 0x80000000);
  if (static_cast<std::uint32_t>(info[0]) < 0x80000004U) return {};
  char brand[49] = {0};
  for (std::uint32_t extended = 0x80000002U; extended <= 0x80000004U; ++extended) {
    __cpuid(info, static_cast<std::int32_t>(extended));
    std::memcpy(brand + (extended - 0x80000002U) * 16, info, 16);
  }
  brand[48] = '\0';
  return trim(brand);
}
#endif

std::string cpu_brand() {
#if defined(_MSC_VER)
  std::string brand = cpu_brand_msvc();
  if (!brand.empty()) return brand;
#endif
  if (const char* env = safe_getenv("PROCESSOR_IDENTIFIER"); env && *env) return env;
  return "unknown-cpu";
}

long cpu_core_count() {
  if (const char* env = safe_getenv("NUMBER_OF_PROCESSORS"); env && *env) {
    const long parsed = std::strtol(env, nullptr, 10);
    if (parsed >= 1) return parsed;
  }
  return std::max(1U, std::thread::hardware_concurrency());
}

std::string iso8601_utc_now() {
  std::time_t now = std::time(nullptr);
  std::tm utc{};
#if defined(_WIN32)
  gmtime_s(&utc, &now);
#else
  gmtime_r(&now, &utc);
#endif
  char buf[24];
  std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &utc);
  return buf;
}

double timer_resolution_ns() {
  double best = 0.0;
  bool found = false;
  for (int i = 0; i < 1000; ++i) {
    const auto begin = SteadyClock::now();
    const auto end = SteadyClock::now();
    const double nanos =
        std::chrono::duration<double, std::nano>(end - begin).count();
    if (nanos > 0.0 && (!found || nanos < best)) {
      best = nanos;
      found = true;
    }
  }
  return found ? best : 0.0;
}

std::string resolve_git_ref(const std::string& explicit_ref) {
  if (!explicit_ref.empty()) return explicit_ref;
  if (const char* env = safe_getenv("VERDIGRIS_GIT_REF"); env && *env) return env;
  std::string candidate;
#if defined(_WIN32)
  FILE* pipe = _popen("git rev-parse HEAD 2>nul", "r");
#else
  FILE* pipe = ::popen("git rev-parse HEAD 2>/dev/null", "r");
#endif
  if (pipe) {
    char buffer[128] = {0};
    if (std::fgets(buffer, sizeof(buffer), pipe)) candidate = trim(buffer);
#if defined(_WIN32)
    _pclose(pipe);
#else
    ::pclose(pipe);
#endif
  }
  bool looks_like_sha = candidate.size() >= 7 && candidate.size() <= 40;
  for (char c : candidate) {
    const bool hex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
    looks_like_sha = looks_like_sha && hex;
  }
  return looks_like_sha ? candidate : std::string("unknown");
}

// ──────────────────────────────────────────────────────────────────────────
// Evidence emission.
// ──────────────────────────────────────────────────────────────────────────
struct ThresholdCheck {
  std::string id;
  double value = 0.0;
  const char* op = "min";
  double bound = 0.0;
  bool pass = false;
};

void append_percentiles(std::ostringstream& json, const Percentiles& stats) {
  json << "{\"p50\": " << stats.p50 << ", \"p90\": " << stats.p90
       << ", \"p99\": " << stats.p99 << ", \"max\": " << stats.max
       << ", \"mean\": " << stats.mean << "}";
}

void append_counts(std::ostringstream& json, const Counts& counts) {
  json << "{\"monsters_start\": " << counts.monsters_start
       << ", \"monsters_end\": " << counts.monsters_end
       << ", \"spawned_via_seam\": " << counts.spawned_via_seam
       << ", \"events\": " << counts.events
       << ", \"final_tick\": " << counts.final_tick << "}";
}

std::string build_evidence_json(const Options& options, std::uint64_t seed,
                                const RunOutcome& run_a, const RunOutcome& run_b,
                                bool counts_match, bool checksum_match,
                                const Percentiles& update_stats,
                                const Percentiles& frame_stats,
                                double timer_ns,
                                const std::vector<ThresholdCheck>& checks) {
  const bool reproducible = counts_match && checksum_match;
  const double ticks_per_sec =
      run_a.elapsed_s > 0.0
          ? static_cast<double>(options.ticks) / run_a.elapsed_s
          : 0.0;
  const std::size_t sample_count =
      std::min(run_a.update_ms.size(), run_a.frame_ms.size());

  std::ostringstream json;
  json.setf(std::ios::fixed);
  json.precision(9);
  json << "{\n"
       << "  \"schema\": \"" << kSchema << "\",\n"
       << "  \"task\": \"" << kTask << "\",\n"
       << "  \"scenario\": {\n"
       << "    \"id\": \"" << options.scenario_id << "\",\n"
       << "    \"route\": \"" << kRouteId << "\",\n"
       << "    \"action\": \"Melee\",\n"
       << "    \"n\": " << options.n << ",\n"
       << "    \"ticks\": " << options.ticks << ",\n"
       << "    \"seed\": " << seed << ",\n"
       << "    \"run\": " << options.run << "\n"
       << "  },\n"
       << "  \"provenance\": {\n"
       << "    \"generated_at_utc\": \"" << iso8601_utc_now() << "\",\n"
       << "    \"tool_version\": \"" << kToolVersion << "\",\n"
       << "    \"git_ref\": \"" << resolve_git_ref(options.git_ref_arg) << "\",\n"
       << "    \"compiler\": \"" << compiler_name() << "\",\n"
       << "    \"cxx_standard\": \"" << cxx_standard_name() << "\",\n"
       << "    \"build_config\": \"" << build_config_name() << "\",\n"
       << "    \"os\": \"" << os_name() << "\",\n"
       << "    \"arch\": \"" << arch_name() << "\",\n"
       << "    \"cpu\": \"" << cpu_brand() << "\",\n"
       << "    \"cpu_cores\": " << cpu_core_count() << ",\n"
       << "    \"timer\": \"std::chrono::steady_clock\",\n"
       << "    \"timer_resolution_ns\": " << timer_ns << ",\n"
       << "    \"host\": \"" << os_name() << "\"\n"
       << "  },\n"
       << "  \"determinism\": {\n"
       << "    \"runs\": 2,\n"
       << "    \"reproducible\": " << (reproducible ? "true" : "false") << ",\n"
       << "    \"counts_match\": " << (counts_match ? "true" : "false") << ",\n"
       << "    \"checksum_match\": " << (checksum_match ? "true" : "false") << ",\n"
       << "    \"state_checksum\": \"" << run_a.state_checksum << "\",\n"
       << "    \"state_checksum_repeat\": \"" << run_b.state_checksum << "\",\n"
       << "    \"counts\": ";
  append_counts(json, run_a.counts);
  json << ",\n    \"counts_repeat\": ";
  append_counts(json, run_b.counts);
  json << "\n"
       << "  },\n"
       << "  \"timings\": {\n"
       << "    \"tick_budget_ms\": " << kTickBudgetMs << ",\n"
       << "    \"elapsed_s\": " << run_a.elapsed_s << ",\n"
       << "    \"ticks_per_sec\": " << ticks_per_sec << ",\n"
       << "    \"samples\": " << sample_count << ",\n"
       << "    \"percentile_method\": \"nearest-rank\",\n"
       << "    \"update_ms\": ";
  append_percentiles(json, update_stats);
  json << ",\n    \"frame_ms\": ";
  append_percentiles(json, frame_stats);
  json << "\n"
       << "  },\n"
       << "  \"thresholds\": {\n"
       << "    \"contract\": \"" << kThresholdContract << "\",\n"
       << "    \"all_pass\": "
       << (reproducible && std::all_of(checks.begin(), checks.end(),
                                       [](const ThresholdCheck& c) { return c.pass; })
               ? "true"
               : "false")
       << ",\n"
       << "    \"checks\": [\n";
  for (std::size_t i = 0; i < checks.size(); ++i) {
    json << "      {\"id\": \"" << checks[i].id << "\", \"value\": " << checks[i].value
         << ", \"op\": \"" << checks[i].op << "\", \"bound\": " << checks[i].bound
         << ", \"pass\": " << (checks[i].pass ? "true" : "false") << "}"
         << (i + 1 < checks.size() ? "," : "") << "\n";
  }
  json << "    ]\n"
       << "  }\n"
       << "}\n";
  return json.str();
}

std::vector<ThresholdCheck> evaluate_thresholds(const Options& options,
                                                const RunOutcome& run_a,
                                                const Percentiles& update_stats,
                                                const Percentiles& frame_stats,
                                                double ticks_per_sec,
                                                bool reproducible,
                                                std::size_t samples) {
  std::vector<ThresholdCheck> checks;
  auto add = [&checks](const char* id, double value, const char* op, double bound,
                       bool pass) {
    checks.push_back({id, value, op, bound, pass});
  };
  add("monsters_spawned", run_a.counts.monsters_start, "min",
      static_cast<double>(options.n),
      run_a.counts.monsters_start >= options.n);
  add("samples_complete", static_cast<double>(samples), "min",
      static_cast<double>(options.ticks),
      samples == static_cast<std::size_t>(options.ticks));
  add("reproducible", reproducible ? 1.0 : 0.0, "min", 1.0, reproducible);
  add("frame_p99_within_budget", frame_stats.p99, "max", kTickBudgetMs,
      frame_stats.p99 <= kTickBudgetMs);
  add("frame_mean_within_budget", frame_stats.mean, "max", kTickBudgetMs,
      frame_stats.mean <= kTickBudgetMs);
  add("update_p99_within_budget", update_stats.p99, "max", kTickBudgetMs,
      update_stats.p99 <= kTickBudgetMs);
  add("ticks_per_sec_floor", ticks_per_sec, "min", kTicksPerSecFloor,
      ticks_per_sec >= kTicksPerSecFloor);
  return checks;
}

int write_evidence(const std::string& path, const std::string& content) {
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  if (!out) {
    std::cerr << "entity_density_bench: cannot write " << path << "\n";
    return 1;
  }
  out << content;
  out.close();
  if (!out) {
    std::cerr << "entity_density_bench: failed while writing " << path << "\n";
    return 1;
  }
  std::cout << content;
  return 0;
}

int run_mode(const Options& options) {
  if (options.scenario_id != kScenarioId) {
    std::cerr << "entity_density_bench: unknown scenario '" << options.scenario_id
              << "' (fixed set: " << kScenarioId << ")\n";
    return 2;
  }
  const std::uint64_t seed = resolve_seed(options);
  const double timer_ns = timer_resolution_ns();

  const RunOutcome run_a = execute_scenario(options, seed);
  const RunOutcome run_b = execute_scenario(options, seed);
  const bool counts_match = run_a.counts == run_b.counts;
  const bool checksum_match = run_a.state_checksum == run_b.state_checksum;
  const bool reproducible = counts_match && checksum_match;
  if (!reproducible) {
    std::cerr << "entity_density_bench: SEEDED RUNS DISAGREE (counts_match="
              << counts_match << " checksum_match=" << checksum_match << ")\n";
  }

  const Percentiles update_stats = summarize(run_a.update_ms);
  const Percentiles frame_stats = summarize(run_a.frame_ms);
  const double ticks_per_sec =
      run_a.elapsed_s > 0.0
          ? static_cast<double>(options.ticks) / run_a.elapsed_s
          : 0.0;
  const std::size_t samples =
      std::min(run_a.update_ms.size(), run_a.frame_ms.size());
  const std::vector<ThresholdCheck> checks =
      evaluate_thresholds(options, run_a, update_stats, frame_stats,
                          ticks_per_sec, reproducible, samples);
  const bool all_pass = std::all_of(checks.begin(), checks.end(),
                                    [](const ThresholdCheck& c) { return c.pass; });
  for (const ThresholdCheck& check : checks) {
    if (!check.pass) {
      std::cerr << "entity_density_bench: threshold FAIL " << check.id << " (value "
                << check.value << " " << check.op << " " << check.bound << ")\n";
    }
  }

  const std::string json =
      build_evidence_json(options, seed, run_a, run_b, counts_match,
                          checksum_match, update_stats, frame_stats, timer_ns,
                          checks);
  const int write_result = write_evidence(options.out_path, json);
  if (write_result != 0) return write_result;
  return all_pass && reproducible ? 0 : 1;
}

// ──────────────────────────────────────────────────────────────────────────
// Strict minimal JSON reader for --validate (RFC 8259 subset).
// ──────────────────────────────────────────────────────────────────────────
struct JValue {
  enum class Type { Null, Bool, Number, String, Array, Object };
  Type type = Type::Null;
  bool boolean = false;
  double number = 0.0;
  std::string text;
  std::vector<JValue> items;
  std::vector<std::pair<std::string, JValue>> members;

  const JValue* find(std::string_view key) const {
    for (const auto& member : members) {
      if (member.first == key) return &member.second;
    }
    return nullptr;
  }
};

class JsonParser {
 public:
  explicit JsonParser(std::string_view input) : input_(input) {}

  bool parse(JValue* out, std::string* error) {
    skip_ws();
    if (!parse_value(out, 0)) {
      *error = error_;
      return false;
    }
    skip_ws();
    if (pos_ != input_.size()) {
      *error = error_at("trailing characters after JSON value");
      return false;
    }
    return true;
  }

 private:
  void skip_ws() {
    while (pos_ < input_.size()) {
      const char c = input_[pos_];
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        ++pos_;
      } else {
        break;
      }
    }
  }

  std::string error_at(const std::string& message) {
    std::ostringstream formatted;
    formatted << "offset " << pos_ << ": " << message;
    return formatted.str();
  }

  bool fail(const std::string& message) {
    if (error_.empty()) error_ = error_at(message);
    return false;
  }

  bool parse_value(JValue* out, int depth) {
    if (depth > 64) return fail("nesting depth exceeds 64");
    if (pos_ >= input_.size()) return fail("unexpected end of input");
    const char c = input_[pos_];
    switch (c) {
      case '{':
        return parse_object(out, depth);
      case '[':
        return parse_array(out, depth);
      case '"':
        out->type = JValue::Type::String;
        return parse_string(&out->text);
      case 't':
        return parse_literal(out, "true", true);
      case 'f':
        return parse_literal(out, "false", false);
      case 'n':
        return parse_null(out);
      default:
        return parse_number(out);
    }
  }

  bool parse_literal(JValue* out, const char* literal, bool value) {
    const std::size_t length = std::strlen(literal);
    if (input_.substr(pos_, length) != literal) {
      return fail(std::string("invalid literal near '") +
                  std::string(input_.substr(pos_, length)) + "'");
    }
    pos_ += length;
    out->type = JValue::Type::Bool;
    out->boolean = value;
    return true;
  }

  bool parse_null(JValue* out) {
    if (input_.substr(pos_, 4) != "null") return fail("invalid literal");
    pos_ += 4;
    out->type = JValue::Type::Null;
    return true;
  }

  bool parse_number(JValue* out) {
    const std::size_t begin = pos_;
    if (pos_ < input_.size() && input_[pos_] == '-') ++pos_;
    if (pos_ >= input_.size()) return fail("truncated number");
    if (input_[pos_] == '0') {
      ++pos_;
    } else if (input_[pos_] >= '1' && input_[pos_] <= '9') {
      while (pos_ < input_.size() && input_[pos_] >= '0' && input_[pos_] <= '9') {
        ++pos_;
      }
    } else {
      return fail("invalid number");
    }
    if (pos_ < input_.size() && input_[pos_] == '.') {
      ++pos_;
      if (pos_ >= input_.size() || input_[pos_] < '0' || input_[pos_] > '9') {
        return fail("invalid fraction");
      }
      while (pos_ < input_.size() && input_[pos_] >= '0' && input_[pos_] <= '9') {
        ++pos_;
      }
    }
    if (pos_ < input_.size() && (input_[pos_] == 'e' || input_[pos_] == 'E')) {
      ++pos_;
      if (pos_ < input_.size() && (input_[pos_] == '+' || input_[pos_] == '-')) {
        ++pos_;
      }
      if (pos_ >= input_.size() || input_[pos_] < '0' || input_[pos_] > '9') {
        return fail("invalid exponent");
      }
      while (pos_ < input_.size() && input_[pos_] >= '0' && input_[pos_] <= '9') {
        ++pos_;
      }
    }
    const std::string token(input_.substr(begin, pos_ - begin));
    char* end = nullptr;
    out->number = std::strtod(token.c_str(), &end);
    if (end != token.c_str() + token.size()) return fail("invalid number");
    out->type = JValue::Type::Number;
    return true;
  }

  bool parse_string(std::string* out) {
    if (input_[pos_] != '"') return fail("expected string");
    ++pos_;
    out->clear();
    while (true) {
      if (pos_ >= input_.size()) return fail("unterminated string");
      const unsigned char c = static_cast<unsigned char>(input_[pos_]);
      if (c == '"') {
        ++pos_;
        return true;
      }
      if (c < 0x20U) return fail("raw control character in string");
      if (c == '\\') {
        ++pos_;
        if (pos_ >= input_.size()) return fail("unterminated escape");
        const char escape = input_[pos_++];
        switch (escape) {
          case '"': out->push_back('"'); break;
          case '\\': out->push_back('\\'); break;
          case '/': out->push_back('/'); break;
          case 'b': out->push_back('\b'); break;
          case 'f': out->push_back('\f'); break;
          case 'n': out->push_back('\n'); break;
          case 'r': out->push_back('\r'); break;
          case 't': out->push_back('\t'); break;
          case 'u': {
            std::uint32_t code_point = 0;
            if (!parse_hex4(&code_point)) return false;
            if (code_point >= 0xD800U && code_point <= 0xDBFFU) {
              if (pos_ + 1 >= input_.size() || input_[pos_] != '\\' ||
                  input_[pos_ + 1] != 'u') {
                return fail("unpaired high surrogate");
              }
              pos_ += 2;
              std::uint32_t low = 0;
              if (!parse_hex4(&low)) return false;
              if (low < 0xDC00U || low > 0xDFFFU) {
                return fail("invalid low surrogate");
              }
              code_point = 0x10000U + ((code_point - 0xD800U) << 10) +
                           (low - 0xDC00U);
            } else if (code_point >= 0xDC00U && code_point <= 0xDFFFU) {
              return fail("unpaired low surrogate");
            }
            append_utf8(out, code_point);
            break;
          }
          default:
            return fail("invalid escape");
        }
        continue;
      }
      out->push_back(static_cast<char>(c));
      ++pos_;
    }
  }

  bool parse_hex4(std::uint32_t* out) {
    if (pos_ + 4 > input_.size()) return fail("truncated \\u escape");
    *out = 0;
    for (int i = 0; i < 4; ++i) {
      const char c = input_[pos_++];
      *out <<= 4;
      if (c >= '0' && c <= '9') {
        *out |= static_cast<std::uint32_t>(c - '0');
      } else if (c >= 'a' && c <= 'f') {
        *out |= static_cast<std::uint32_t>(c - 'a' + 10);
      } else if (c >= 'A' && c <= 'F') {
        *out |= static_cast<std::uint32_t>(c - 'A' + 10);
      } else {
        return fail("invalid \\u escape digit");
      }
    }
    return true;
  }

  static void append_utf8(std::string* out, std::uint32_t code_point) {
    if (code_point < 0x80U) {
      out->push_back(static_cast<char>(code_point));
    } else if (code_point < 0x800U) {
      out->push_back(static_cast<char>(0xC0U | (code_point >> 6)));
      out->push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    } else if (code_point < 0x10000U) {
      out->push_back(static_cast<char>(0xE0U | (code_point >> 12)));
      out->push_back(static_cast<char>(0x80U | ((code_point >> 6) & 0x3FU)));
      out->push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    } else {
      out->push_back(static_cast<char>(0xF0U | (code_point >> 18)));
      out->push_back(static_cast<char>(0x80U | ((code_point >> 12) & 0x3FU)));
      out->push_back(static_cast<char>(0x80U | ((code_point >> 6) & 0x3FU)));
      out->push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    }
  }

  bool parse_array(JValue* out, int depth) {
    ++pos_;  // '['
    out->type = JValue::Type::Array;
    skip_ws();
    if (pos_ < input_.size() && input_[pos_] == ']') {
      ++pos_;
      return true;
    }
    while (true) {
      skip_ws();
      JValue element;
      if (!parse_value(&element, depth + 1)) return false;
      out->items.push_back(std::move(element));
      skip_ws();
      if (pos_ >= input_.size()) return fail("unterminated array");
      if (input_[pos_] == ',') {
        ++pos_;
        continue;
      }
      if (input_[pos_] == ']') {
        ++pos_;
        return true;
      }
      return fail("expected ',' or ']' in array");
    }
  }

  bool parse_object(JValue* out, int depth) {
    ++pos_;  // '{'
    out->type = JValue::Type::Object;
    skip_ws();
    if (pos_ < input_.size() && input_[pos_] == '}') {
      ++pos_;
      return true;
    }
    while (true) {
      skip_ws();
      if (pos_ >= input_.size() || input_[pos_] != '"') {
        return fail("expected object key");
      }
      std::string key;
      if (!parse_string(&key)) return false;
      skip_ws();
      if (pos_ >= input_.size() || input_[pos_] != ':') {
        return fail("expected ':' after object key");
      }
      ++pos_;
      skip_ws();
      JValue value;
      if (!parse_value(&value, depth + 1)) return false;
      out->members.emplace_back(std::move(key), std::move(value));
      skip_ws();
      if (pos_ >= input_.size()) return fail("unterminated object");
      if (input_[pos_] == ',') {
        ++pos_;
        continue;
      }
      if (input_[pos_] == '}') {
        ++pos_;
        return true;
      }
      return fail("expected ',' or '}' in object");
    }
  }

  std::string_view input_;
  std::size_t pos_ = 0;
  std::string error_;
};

// ──────────────────────────────────────────────────────────────────────────
// Validation mode.
// ──────────────────────────────────────────────────────────────────────────
bool read_file(const std::string& path, std::string* out, std::string* error) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    *error = "cannot open " + path;
    return false;
  }
  std::ostringstream buffer;
  buffer << in.rdbuf();
  *out = buffer.str();
  if (in.bad()) {
    *error = "read failure on " + path;
    return false;
  }
  return true;
}

bool is_integral(double value) {
  return std::isfinite(value) && value == std::floor(value);
}

bool field_string(const JValue& object, const char* key,
                  std::vector<std::string>* failures, std::string* out) {
  const JValue* field = object.find(key);
  if (!field || field->type != JValue::Type::String || field->text.empty()) {
    failures->push_back(std::string("missing or empty string field '") + key + "'");
    return false;
  }
  *out = field->text;
  return true;
}

bool field_number(const JValue& object, const char* key,
                  std::vector<std::string>* failures, double* out) {
  const JValue* field = object.find(key);
  if (!field || field->type != JValue::Type::Number ||
      !std::isfinite(field->number)) {
    failures->push_back(std::string("missing or non-numeric field '") + key + "'");
    return false;
  }
  *out = field->number;
  return true;
}

bool field_bool_true(const JValue& object, const char* key,
                     std::vector<std::string>* failures) {
  const JValue* field = object.find(key);
  if (!field || field->type != JValue::Type::Bool || !field->boolean) {
    failures->push_back(std::string("field '") + key + "' must be true");
    return false;
  }
  return true;
}

const JValue* field_object(const JValue& object, const char* key,
                           std::vector<std::string>* failures) {
  const JValue* field = object.find(key);
  if (!field || field->type != JValue::Type::Object) {
    failures->push_back(std::string("missing object field '") + key + "'");
    return nullptr;
  }
  return field;
}

bool valid_timestamp(const std::string& value) {
  if (value.size() != 20) return false;
  if (value[4] != '-' || value[7] != '-' || value[10] != 'T' ||
      value[13] != ':' || value[16] != ':' || value[19] != 'Z') {
    return false;
  }
  for (std::size_t i = 0; i < value.size(); ++i) {
    const bool digit = value[i] >= '0' && value[i] <= '9';
    const bool structural = i == 4 || i == 7 || i == 10 || i == 13 || i == 16 || i == 19;
    if (!digit && !structural) return false;
  }
  return true;
}

bool valid_checksum_format(const std::string& value) {
  if (value.size() != 24 || value.compare(0, 8, "fnv1a64:") != 0) return false;
  for (std::size_t i = 8; i < value.size(); ++i) {
    const bool hex = (value[i] >= '0' && value[i] <= '9') ||
                     (value[i] >= 'a' && value[i] <= 'f');
    if (!hex) return false;
  }
  return true;
}

void validate_counts(const JValue& counts, const JValue& counts_repeat, long n,
                     long ticks, std::vector<std::string>* failures) {
  const char* fields[] = {"monsters_start", "monsters_end", "spawned_via_seam",
                          "events", "final_tick"};
  for (const char* name : fields) {
    double value = 0.0;
    double repeat = 0.0;
    const bool have_value = field_number(counts, name, failures, &value);
    field_number(counts_repeat, name, failures, &repeat);
    if (!have_value) continue;
    if (!is_integral(value)) {
      failures->push_back(std::string("determinism.counts.") + name +
                          " must be an integer");
      continue;
    }
    if (value != repeat) {
      failures->push_back(std::string("determinism.counts_repeat.") + name +
                          " disagrees with determinism.counts." + name);
    }
  }
  double start = 0.0;
  double end = 0.0;
  double seam_spawns = 0.0;
  double final_tick = 0.0;
  if (field_number(counts, "monsters_start", failures, &start) &&
      is_integral(start) && static_cast<long>(start) != n) {
    failures->push_back("determinism.counts.monsters_start != scenario.n");
  }
  if (field_number(counts, "monsters_end", failures, &end) && is_integral(end) &&
      end < 0) {
    failures->push_back("determinism.counts.monsters_end is negative");
  }
  if (field_number(counts, "spawned_via_seam", failures, &seam_spawns) &&
      is_integral(seam_spawns) && seam_spawns < 1) {
    failures->push_back("determinism.counts.spawned_via_seam < 1");
  }
  if (field_number(counts, "final_tick", failures, &final_tick) &&
      is_integral(final_tick) && final_tick < static_cast<double>(ticks)) {
    failures->push_back("determinism.counts.final_tick < scenario.ticks");
  }
}

void validate_percentile_block(const JValue& parent, const char* key,
                               std::vector<std::string>* failures) {
  const JValue* block = field_object(parent, key, failures);
  if (!block) return;
  double p50 = 0.0;
  double p90 = 0.0;
  double p99 = 0.0;
  double max = 0.0;
  double mean = 0.0;
  const bool have_all =
      field_number(*block, "p50", failures, &p50) &&
      field_number(*block, "p90", failures, &p90) &&
      field_number(*block, "p99", failures, &p99) &&
      field_number(*block, "max", failures, &max) &&
      field_number(*block, "mean", failures, &mean);
  if (!have_all) return;
  const std::string prefix = std::string("timings.") + key + ".";
  if (!(p50 > 0.0)) failures->push_back(prefix + "p50 must be > 0");
  if (!(mean > 0.0)) failures->push_back(prefix + "mean must be > 0");
  if (!(p50 <= p90 && p90 <= p99 && p99 <= max)) {
    failures->push_back(prefix + "percentiles are not monotonic (p50<=p90<=p99<=max)");
  }
}

int validate_evidence(const std::string& path) {
  std::string content;
  std::string io_error;
  if (!read_file(path, &content, &io_error)) {
    std::cerr << "entity_density_bench --validate: " << io_error << "\n";
    return 1;
  }

  JsonParser parser(content);
  JValue root;
  std::string parse_error;
  if (!parser.parse(&root, &parse_error)) {
    std::cerr << "entity_density_bench --validate: malformed JSON (" << parse_error
              << ")\n";
    return 1;
  }
  if (root.type != JValue::Type::Object) {
    std::cerr << "entity_density_bench --validate: evidence root must be an object\n";
    return 1;
  }

  std::vector<std::string> failures;
  std::string schema;
  if (field_string(root, "schema", &failures, &schema) && schema != kSchema) {
    failures.push_back(std::string("unsupported schema '") + schema + "'");
  }
  std::string task;
  if (field_string(root, "task", &failures, &task) && task != kTask) {
    failures.push_back(std::string("evidence task '") + task + "' is not " +
                       kTask);
  }

  const JValue* scenario = field_object(root, "scenario", &failures);
  double n = 0.0;
  double ticks = 0.0;
  double seed = 0.0;
  if (scenario) {
    std::string scenario_id;
    if (field_string(*scenario, "id", &failures, &scenario_id) &&
        scenario_id != kScenarioId) {
      failures.push_back(std::string("unknown scenario id '") + scenario_id + "'");
    }
    std::string route;
    std::string action;
    field_string(*scenario, "route", &failures, &route);
    field_string(*scenario, "action", &failures, &action);
    if (field_number(*scenario, "n", &failures, &n) &&
        (!is_integral(n) || n < 1)) {
      failures.push_back("scenario.n must be a positive integer");
    }
    if (field_number(*scenario, "ticks", &failures, &ticks) &&
        (!is_integral(ticks) || ticks < 1)) {
      failures.push_back("scenario.ticks must be a positive integer");
    }
    if (field_number(*scenario, "seed", &failures, &seed) && !(seed > 0.0)) {
      failures.push_back("scenario.seed must be > 0");
    }
    double run = 0.0;
    if (field_number(*scenario, "run", &failures, &run) &&
        (!is_integral(run) || run < 1)) {
      failures.push_back("scenario.run must be a positive integer");
    }
  }

  const JValue* provenance = field_object(root, "provenance", &failures);
  if (provenance) {
    const char* strings[] = {"generated_at_utc", "tool_version", "git_ref",
                             "compiler", "cxx_standard", "build_config",
                             "os", "arch", "cpu", "timer", "host"};
    for (const char* name : strings) {
      std::string value;
      if (field_string(*provenance, name, &failures, &value) &&
          std::string(name) == "generated_at_utc" && !valid_timestamp(value)) {
        failures.push_back("provenance.generated_at_utc is not ISO-8601 UTC");
      }
    }
    double cores = 0.0;
    if (field_number(*provenance, "cpu_cores", &failures, &cores) &&
        (!is_integral(cores) || cores < 1)) {
      failures.push_back("provenance.cpu_cores must be a positive integer");
    }
    double resolution = 0.0;
    if (field_number(*provenance, "timer_resolution_ns", &failures, &resolution) &&
        resolution < 0.0) {
      failures.push_back("provenance.timer_resolution_ns must be >= 0");
    }
  }

  const JValue* determinism = field_object(root, "determinism", &failures);
  if (determinism) {
    double runs = 0.0;
    if (field_number(*determinism, "runs", &failures, &runs) &&
        (!is_integral(runs) || runs != 2)) {
      failures.push_back("determinism.runs must equal 2");
    }
    field_bool_true(*determinism, "reproducible", &failures);
    field_bool_true(*determinism, "counts_match", &failures);
    field_bool_true(*determinism, "checksum_match", &failures);
    std::string checksum;
    std::string checksum_repeat;
    if (field_string(*determinism, "state_checksum", &failures, &checksum) &&
        !valid_checksum_format(checksum)) {
      failures.push_back("determinism.state_checksum has invalid fnv1a64 format");
    }
    if (field_string(*determinism, "state_checksum_repeat", &failures,
                     &checksum_repeat) &&
        !valid_checksum_format(checksum_repeat)) {
      failures.push_back(
          "determinism.state_checksum_repeat has invalid fnv1a64 format");
    }
    if (!checksum.empty() && !checksum_repeat.empty() && checksum != checksum_repeat) {
      failures.push_back("state_checksum disagrees between seeded runs");
    }
    const JValue* counts = field_object(*determinism, "counts", &failures);
    const JValue* counts_repeat =
        field_object(*determinism, "counts_repeat", &failures);
    if (counts && counts_repeat && is_integral(n) && is_integral(ticks)) {
      validate_counts(*counts, *counts_repeat, static_cast<long>(n),
                      static_cast<long>(ticks), &failures);
    }
  }

  const JValue* timings = field_object(root, "timings", &failures);
  if (timings) {
    double budget = 0.0;
    if (field_number(*timings, "tick_budget_ms", &failures, &budget) &&
        budget != kTickBudgetMs) {
      failures.push_back("timings.tick_budget_ms must equal the 50 ms budget");
    }
    double elapsed = 0.0;
    if (field_number(*timings, "elapsed_s", &failures, &elapsed) &&
        !(elapsed > 0.0)) {
      failures.push_back("timings.elapsed_s must be > 0");
    }
    double per_sec = 0.0;
    if (field_number(*timings, "ticks_per_sec", &failures, &per_sec) &&
        !(per_sec > 0.0)) {
      failures.push_back("timings.ticks_per_sec must be > 0");
    }
    double samples = 0.0;
    if (field_number(*timings, "samples", &failures, &samples) &&
        (!is_integral(samples) || samples != ticks)) {
      failures.push_back("timings.samples != scenario.ticks (incomplete capture)");
    }
    std::string method;
    if (field_string(*timings, "percentile_method", &failures, &method) &&
        method != "nearest-rank") {
      failures.push_back("timings.percentile_method must be nearest-rank");
    }
    validate_percentile_block(*timings, "update_ms", &failures);
    validate_percentile_block(*timings, "frame_ms", &failures);
  }

  const JValue* thresholds = field_object(root, "thresholds", &failures);
  if (thresholds) {
    std::string contract;
    if (field_string(*thresholds, "contract", &failures, &contract) &&
        contract != kThresholdContract) {
      failures.push_back(std::string("unknown threshold contract '") + contract +
                         "'");
    }
    field_bool_true(*thresholds, "all_pass", &failures);
    const JValue* checks = thresholds->find("checks");
    if (!checks || checks->type != JValue::Type::Array) {
      failures.push_back("thresholds.checks must be an array");
    } else {
      std::vector<std::string> seen_ids;
      for (const JValue& check : checks->items) {
        if (check.type != JValue::Type::Object) {
          failures.push_back("thresholds.checks entries must be objects");
          continue;
        }
        std::string id;
        if (!field_string(check, "id", &failures, &id)) continue;
        seen_ids.push_back(id);
        field_bool_true(check, "pass", &failures);
        double value = 0.0;
        double bound = 0.0;
        field_number(check, "value", &failures, &value);
        field_number(check, "bound", &failures, &bound);
        std::string op;
        if (field_string(check, "op", &failures, &op) && op != "min" && op != "max") {
          failures.push_back("threshold check '" + id + "' has unknown op '" + op +
                             "'");
        }
      }
      if (seen_ids.size() != kRequiredCheckCount) {
        failures.push_back("thresholds.checks does not cover the documented "
                           "check set exactly");
      } else {
        for (const char* required : kRequiredCheckIds) {
          bool found = false;
          for (const std::string& seen : seen_ids) found = found || seen == required;
          if (!found) {
            failures.push_back(std::string("thresholds.checks is missing '") +
                               required + "'");
          }
        }
      }
    }
  }

  if (!failures.empty()) {
    std::cerr << "entity_density_bench --validate: INVALID EVIDENCE (" << path
              << ")\n";
    for (const std::string& failure : failures) {
      std::cerr << "  - " << failure << "\n";
    }
    return 1;
  }
  std::cout << "entity_density_bench --validate: OK (" << path << ")\n";
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  Options options;
  if (!parse_args(argc, argv, &options)) {
    usage();
    return 2;
  }
  if (!options.validate_path.empty()) return validate_evidence(options.validate_path);
  return run_mode(options);
}
