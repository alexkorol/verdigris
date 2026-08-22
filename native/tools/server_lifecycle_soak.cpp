// TASK-0129: native WebSocket server lifecycle soak. Regression guard for
// the PR #46 reader-thread lifetime fix: drives the REAL
// verdigris::networking::WebSocketServer through 100 sequential
// start/connect/login/close/stop cycles plus an eight-client burst before a
// final stop, all on loopback inside this lane's port capsule. Emits
// machine-readable JSON (--out) and exits non-zero on any failed cycle,
// timeout, or hang. No server or client behavior changes live here.

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "../client/remote_session.hpp"
#include "verdigris/networking.hpp"

namespace {

using verdigris::client::ConnectionState;
using verdigris::client::RemoteProtocolSession;
using verdigris::networking::WebSocketServer;

constexpr std::uint16_t kCapsuleMin = 6680;
constexpr std::uint16_t kCapsuleMax = 6699;
constexpr int kCycleCount = 100;
constexpr int kBurstClients = 8;
constexpr int kLoginTimeoutMs = 5000;
constexpr int kBurstTimeoutMs = 15000;

std::atomic<bool> g_watchdog_armed{true};
std::atomic<std::int64_t> g_hard_deadline_ms{0};

std::int64_t steady_now_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

// Hard-process backstop: the soak must never hang silently; a watchdog breach
// is reported on stderr and becomes a non-zero exit.
void watchdog_loop() {
  while (g_watchdog_armed.load()) {
    if (steady_now_ms() > g_hard_deadline_ms.load()) {
      std::fprintf(stderr,
                   "SOAK WATCHDOG: hard wall-clock deadline exceeded; "
                   "aborting with exit code 3\n");
      std::fflush(stderr);
      std::_Exit(3);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
}

std::string json_escape(const std::string& text) {
  std::string out;
  out.reserve(text.size());
  for (const char c : text) {
    switch (c) {
      case '"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          char slot[8];
          std::snprintf(slot, sizeof(slot), "\\u%04x",
                        static_cast<unsigned>(static_cast<unsigned char>(c)));
          out += slot;
        } else {
          out += c;
        }
    }
  }
  return out;
}

std::string iso_stamp(std::int64_t epoch_ms) {
  const std::time_t secs = static_cast<std::time_t>(epoch_ms / 1000);
  std::tm local{};
  localtime_s(&local, &secs);
  char buffer[40];
  std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
                local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                local.tm_hour, local.tm_min, local.tm_sec,
                static_cast<int>(epoch_ms % 1000));
  return buffer;
}

std::string ms_string(double value) {
  char buffer[32];
  std::snprintf(buffer, sizeof(buffer), "%.3f", value);
  return buffer;
}

// Capsule-bound port allocator: rotate through 6680-6699 and prove each bind
// against the real listener so parallel suites cannot collide with us.
class PortBinder {
 public:
  std::unique_ptr<WebSocketServer> bind(std::uint16_t& chosen,
                                        std::string& error) {
    constexpr int span = static_cast<int>(kCapsuleMax - kCapsuleMin) + 1;
    std::string last_error;
    for (int offset = 0; offset < span; ++offset) {
      const int raw =
          kCapsuleMin + (next_ - kCapsuleMin + offset) % span;
      const auto port = static_cast<std::uint16_t>(raw);
      auto server = std::make_unique<WebSocketServer>(port);
      std::string candidate_error;
      if (server->start(&candidate_error)) {
        next_ = raw + 1;
        chosen = port;
        return server;
      }
      if (!candidate_error.empty()) last_error = candidate_error;
    }
    error = "no free port in capsule 6680-6699 (" +
            (last_error.empty() ? std::string("bind/listen failed")
                                : last_error) +
            ")";
    return nullptr;
  }

 private:
  int next_ = kCapsuleMin;
};

bool reached_ready(RemoteProtocolSession& session, int timeout_ms) {
  const std::int64_t deadline = steady_now_ms() + timeout_ms;
  while (steady_now_ms() < deadline) {
    session.poll();
    const auto state = session.connection_state();
    if (state == ConnectionState::Ready) return true;
    if (state == ConnectionState::Disconnected ||
        state == ConnectionState::Rejected ||
        state == ConnectionState::ProtocolMismatch) {
      return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  session.poll();
  return session.connection_state() == ConnectionState::Ready;
}

struct Attempt {
  bool ok = false;
  bool server_started = false;
  bool upgrade = false;
  bool login = false;
  bool clean_close = false;
  std::uint16_t port = 0;
  double stop_duration_ms = 0.0;
  std::string phase;
  std::string error;
};

// Stops the bound server (timed), releases it, and stamps the attempt with
// the stop duration. Uniform across success and failure paths so every cycle
// contributes its stop measurement.
Attempt finish_attempt(Attempt attempt,
                       std::unique_ptr<WebSocketServer>& server) {
  if (server) {
    const std::int64_t begin = steady_now_ms();
    server->stop();
    attempt.stop_duration_ms =
        static_cast<double>(steady_now_ms() - begin);
    server.reset();
  }
  return attempt;
}

std::string connect_failure(const RemoteProtocolSession& session,
                            const std::string& error) {
  if (!error.empty()) return error;
  if (!session.last_error().empty()) return session.last_error();
  return "connect/upgrade failed";
}

class Soak {
 public:
  // One full lifecycle: start -> connect -> login -> close -> stop, all
  // against the real WebSocketServer on a fresh loopback port.
  Attempt run_cycle(int index) {
    Attempt attempt;
    std::string error;
    std::uint16_t port = 0;
    auto server = binder_.bind(port, error);
    if (!server) {
      attempt.phase = "server-start";
      attempt.error = error;
      return finish_attempt(std::move(attempt), server);
    }
    attempt.server_started = true;
    attempt.port = port;

    RemoteProtocolSession session("127.0.0.1", port,
                                  "soak-cycle-" + std::to_string(index), true);
    if (!session.start(&error)) {
      attempt.phase = "client-upgrade";
      attempt.error = connect_failure(session, error);
      return finish_attempt(std::move(attempt), server);
    }
    attempt.upgrade = true;

    if (!reached_ready(session, kLoginTimeoutMs)) {
      attempt.phase = "client-login";
      attempt.error = std::string("login not acknowledged within ") +
                      std::to_string(kLoginTimeoutMs) + " ms (state " +
                      verdigris::client::connection_state_label(
                          session.connection_state()) +
                      ")" +
                      (session.last_error().empty()
                           ? std::string()
                           : ": " + session.last_error());
      return finish_attempt(std::move(attempt), server);
    }
    attempt.login = true;

    session.shutdown();
    if (session.connection_state() != ConnectionState::Disconnected) {
      attempt.phase = "client-close";
      attempt.error = std::string("clean close did not reach disconnected (state ") +
                      verdigris::client::connection_state_label(
                          session.connection_state()) +
                      ")";
      return finish_attempt(std::move(attempt), server);
    }
    attempt.clean_close = true;

    attempt.ok = true;
    return finish_attempt(std::move(attempt), server);
  }

  // Eight concurrent clients on ONE server instance: connect + login burst,
  // then all closes, then the final stop of that server.
  Attempt run_burst(int clients_requested) {
    Attempt attempt;
    std::vector<std::unique_ptr<RemoteProtocolSession>> sessions;
    sessions.reserve(static_cast<std::size_t>(clients_requested));

    std::string error;
    std::uint16_t port = 0;
    auto server = binder_.bind(port, error);
    if (!server) {
      attempt.phase = "burst-server-start";
      attempt.error = error;
      return finish_attempt(std::move(attempt), server);
    }
    attempt.server_started = true;
    attempt.port = port;

    int upgrades = 0;
    for (int i = 0; i < clients_requested; ++i) {
      auto session = std::make_unique<RemoteProtocolSession>(
          "127.0.0.1", port, "soak-burst-" + std::to_string(i), true);
      std::string connect_error;
      if (session->start(&connect_error)) {
        ++upgrades;
        sessions.push_back(std::move(session));
      } else {
        record_burst_failure(attempt, i, "burst-connect",
                             connect_failure(*session, connect_error));
        session->shutdown();
      }
    }
    attempt.upgrade = upgrades == clients_requested;

    int logins = 0;
    const std::int64_t burst_deadline = steady_now_ms() + kBurstTimeoutMs;
    for (const auto& session : sessions) {
      const std::int64_t budget = burst_deadline - steady_now_ms();
      if (budget <= 0 ||
          !reached_ready(*session, static_cast<int>(budget))) {
        record_burst_failure(attempt, logins, "burst-login",
                             "login not acknowledged within the burst budget");
        continue;
      }
      ++logins;
    }
    attempt.login = logins == clients_requested;

    int clean_closes = 0;
    for (const auto& session : sessions) {
      session->shutdown();
      if (session->connection_state() == ConnectionState::Disconnected) {
        ++clean_closes;
      } else {
        record_burst_failure(attempt, clean_closes, "burst-close",
                             "clean close did not reach disconnected");
      }
    }
    attempt.clean_close = clean_closes == clients_requested;

    attempt.ok = attempt.upgrade && attempt.login && attempt.clean_close &&
                 static_cast<int>(burst_failures_.size()) == 0;
    return finish_attempt(std::move(attempt), server);
  }

  const std::vector<Attempt>& burst_failures() const {
    return burst_failures_;
  }

 private:
  void record_burst_failure(Attempt& attempt, int client, const char* phase,
                            const std::string& detail) {
    Attempt marker;
    marker.port = attempt.port;
    marker.phase = std::string(phase) + "#client-" + std::to_string(client);
    marker.error = detail;
    burst_failures_.push_back(std::move(marker));
    if (attempt.error.empty()) {
      attempt.phase = marker.phase;
      attempt.error = detail;
    }
  }

  PortBinder binder_;
  std::vector<Attempt> burst_failures_;
};

}  // namespace

int main(int argc, char** argv) {
  std::string out_path = "server_lifecycle_soak.json";
  for (int i = 1; i + 1 < argc; i += 2) {
    if (std::strcmp(argv[i], "--out") == 0) out_path = argv[i + 1];
  }

  const std::int64_t started_epoch_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  g_hard_deadline_ms.store(steady_now_ms() + 15 * 60 * 1000);
  std::thread watchdog(watchdog_loop);
  watchdog.detach();

  Soak soak;

  std::vector<Attempt> cycles;
  cycles.reserve(static_cast<std::size_t>(kCycleCount));
  for (int index = 1; index <= kCycleCount; ++index) {
    Attempt attempt = soak.run_cycle(index);
    if (attempt.ok) {
      std::printf("[soak] cycle %3d/%d port=%u ok (stop %.1f ms)\n", index,
                  kCycleCount, attempt.port, attempt.stop_duration_ms);
    } else {
      std::printf("[soak] cycle %3d/%d port=%u FAIL at %s: %s\n", index,
                  kCycleCount, attempt.port, attempt.phase.c_str(),
                  attempt.error.c_str());
    }
    std::fflush(stdout);
    cycles.push_back(std::move(attempt));
  }

  Attempt burst = soak.run_burst(kBurstClients);
  std::printf("[soak] burst port=%u %s (upgrades %d/%d, logins %d/%d, "
              "clean closes %d/%d, stop %.1f ms)\n",
              burst.port, burst.ok ? "ok" : "FAIL",
              burst.upgrade ? kBurstClients : 0, kBurstClients,
              burst.login ? kBurstClients : 0, kBurstClients,
              burst.clean_close ? kBurstClients : 0, kBurstClients,
              burst.stop_duration_ms);
  std::fflush(stdout);

  int completed = 0;
  int upgrades = 0;
  int logins = 0;
  int clean_closes = 0;
  double stop_total = 0.0;
  double stop_max = 0.0;
  std::string stops_csv;
  std::string failures_json;
  int failure_count = 0;
  for (std::size_t i = 0; i < cycles.size(); ++i) {
    const Attempt& attempt = cycles[i];
    if (attempt.ok) ++completed;
    if (attempt.upgrade) ++upgrades;
    if (attempt.login) ++logins;
    if (attempt.clean_close) ++clean_closes;
    stop_total += attempt.stop_duration_ms;
    if (attempt.stop_duration_ms > stop_max) {
      stop_max = attempt.stop_duration_ms;
    }
    if (!stops_csv.empty()) stops_csv += ", ";
    stops_csv += ms_string(attempt.stop_duration_ms);
    if (!attempt.ok) {
      if (failure_count > 0) failures_json += ",\n    ";
      failures_json +=
          "{\"cycle\": " + std::to_string(static_cast<int>(i) + 1) +
          ", \"phase\": \"" + json_escape(attempt.phase) +
          "\", \"port\": " + std::to_string(attempt.port) +
          ", \"error\": \"" + json_escape(attempt.error) + "\"}";
      ++failure_count;
    }
  }
  for (const Attempt& marker : soak.burst_failures()) {
    if (failure_count > 0) failures_json += ",\n    ";
    failures_json +=
        std::string("{\"cycle\": -1") +
        ", \"phase\": \"" + json_escape(marker.phase) +
        "\", \"port\": " + std::to_string(marker.port) +
        ", \"error\": \"" + json_escape(marker.error) + "\"}";
    ++failure_count;
  }

  const bool burst_passed = burst.ok;
  const bool passed = completed == kCycleCount && failure_count == 0 &&
                      burst_passed;

  const std::int64_t finished_epoch_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();

  std::string json;
  json.reserve(8192 + stops_csv.size() * 2);
  json += "{\n";
  json += "  \"tool\": \"server_lifecycle_soak\",\n";
  json += "  \"task\": \"TASK-0129\",\n";
  json += "  \"host\": \"127.0.0.1\",\n";
  json += "  \"portCapsule\": [" + std::to_string(kCapsuleMin) + ", " +
          std::to_string(kCapsuleMax) + "],\n";
  json += "  \"startedAt\": \"" + iso_stamp(started_epoch_ms) + "\",\n";
  json += "  \"finishedAt\": \"" + iso_stamp(finished_epoch_ms) + "\",\n";
  json += "  \"cyclesRequested\": " + std::to_string(kCycleCount) + ",\n";
  json += "  \"cyclesCompleted\": " + std::to_string(completed) + ",\n";
  json += "  \"upgradesSucceeded\": " + std::to_string(upgrades) + ",\n";
  json += "  \"loginsSucceeded\": " + std::to_string(logins) + ",\n";
  json += "  \"cleanCloses\": " + std::to_string(clean_closes) + ",\n";
  json += "  \"stopDurationsMs\": [" + stops_csv + "],\n";
  json += "  \"stopDurationTotalMs\": " + ms_string(stop_total) + ",\n";
  json += "  \"stopDurationMaxMs\": " + ms_string(stop_max) + ",\n";
  json += "  \"burstClientsRequested\": " + std::to_string(kBurstClients) +
          ",\n";
  json += "  \"burstServerStartOk\": " +
          std::string(burst.server_started ? "true" : "false") + ",\n";
  json += "  \"burstUpgradesSucceeded\": " +
          std::to_string(burst.upgrade ? kBurstClients : 0) + ",\n";
  json += "  \"burstLoginsSucceeded\": " +
          std::to_string(burst.login ? kBurstClients : 0) + ",\n";
  json += "  \"burstCleanCloses\": " +
          std::to_string(burst.clean_close ? kBurstClients : 0) + ",\n";
  json += "  \"burstStopDurationMs\": " + ms_string(burst.stop_duration_ms) +
          ",\n";
  json += "  \"burstPassed\": " +
          std::string(burst_passed ? "true" : "false") + ",\n";
  json += "  \"totalDurationMs\": " +
          ms_string(static_cast<double>(finished_epoch_ms -
                                        started_epoch_ms)) +
          ",\n";
  json += "  \"passed\": " + std::string(passed ? "true" : "false") + ",\n";
  json += "  \"failures\": [" +
          (failures_json.empty()
               ? std::string()
               : "\n    " + failures_json + "\n  ") +
          "]\n";
  json += "}\n";

  FILE* out = nullptr;
  if (fopen_s(&out, out_path.c_str(), "wb") != 0 || !out) {
    std::fprintf(stderr, "SOAK: cannot open output path %s\n",
                 out_path.c_str());
    return 2;
  }
  std::fwrite(json.data(), 1, json.size(), out);
  std::fclose(out);

  std::printf(
      "SOAK RESULT: %s — cycles %d/%d, upgrades %d, logins %d, clean closes "
      "%d, burst upgrades %d/%d, burst logins %d/%d, burst closes %d/%d, "
      "total %.0f ms\n",
      passed ? "PASS" : "FAIL", completed, kCycleCount, upgrades, logins,
      clean_closes, burst.upgrade ? kBurstClients : 0, kBurstClients,
      burst.login ? kBurstClients : 0, kBurstClients,
      burst.clean_close ? kBurstClients : 0, kBurstClients,
      static_cast<double>(finished_epoch_ms - started_epoch_ms));
  std::printf("SOAK JSON: %s\n", out_path.c_str());
  std::fflush(stdout);
  g_watchdog_armed.store(false);
  return passed ? 0 : 1;
}
