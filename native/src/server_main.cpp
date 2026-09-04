#include "verdigris/networking.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <random>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace {
// Two server processes must not overwrite the same account directory.
struct SaveDirectoryLock {
#ifdef _WIN32
  HANDLE handle = INVALID_HANDLE_VALUE;
  ~SaveDirectoryLock() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
#else
  int handle = -1;
  ~SaveDirectoryLock() { if (handle >= 0) close(handle); }
#endif
  bool acquire(const std::filesystem::path& directory) {
    if (directory.empty()) return true;
    std::filesystem::create_directories(directory);
    const auto path = directory / "server.lock";
#ifdef _WIN32
    handle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                         nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    return handle != INVALID_HANDLE_VALUE;
#else
    handle = open(path.c_str(), O_CREAT | O_RDWR, 0600);
    return handle >= 0 && flock(handle, LOCK_EX | LOCK_NB) == 0;
#endif
  }
};
}

int main(int argc, char** argv) {
  std::uint16_t port = 6500;
  if (argc > 1) {
    const auto value = std::strtol(argv[1], nullptr, 10);
    if (value > 0 && value <= 65535) port = static_cast<std::uint16_t>(value);
  }
  if (const char* configured = std::getenv("VERDIGRIS_PORT")) {
    const auto value = std::strtol(configured, nullptr, 10);
    if (value > 0 && value <= 65535 && argc == 1) port = static_cast<std::uint16_t>(value);
  }

  std::filesystem::path saves;
  if (const char* configured = std::getenv("VERDIGRIS_SAVE_DIR")) saves = configured;
  else if (const char* local = std::getenv("LOCALAPPDATA")) saves = std::filesystem::path(local) / "Verdigris" / "Saves";
  else saves = std::filesystem::absolute("verdigris-saves");
  for (int i = 2; i < argc; ++i) {
    const std::string option = argv[i];
    if (option == "--ephemeral") saves.clear();
    else if (option == "--save-dir" && i + 1 < argc) saves = argv[++i];
    else { std::cerr << "Unknown server option: " << option << "\n"; return 1; }
  }
  SaveDirectoryLock save_lock;
  try {
    if (!save_lock.acquire(saves)) {
      std::cerr << "Save directory already in use or unavailable: " << saves << "\n";
      return 1;
    }
  } catch (const std::exception& error) {
    std::cerr << "Cannot open save directory: " << error.what() << "\n";
    return 1;
  }
  std::random_device random;
  verdigris::set_item_identity_namespace(
      (static_cast<std::uint64_t>(random()) << 32) ^ random());
  verdigris::networking::WebSocketServer server(port, saves);
  std::string error;
  if (!server.start(&error)) {
    std::cerr << "verdigris_server: " << error << "\n";
    return 1;
  }
  std::cout << "verdigris_server listening on ws://127.0.0.1:" << server.port() << "\n";
  std::cout << "Account saves: " << (saves.empty() ? std::string("disabled (test server)") : saves.string()) << "\n";
  std::cout.flush();
  std::string line;
  while (std::getline(std::cin, line)) {
    if (line == "quit" || line == "stop") break;
  }
  // A detached child commonly inherits a closed stdin. EOF is not a server
  // shutdown request: keep the loopback service alive until its owner sends
  // the explicit stop command or terminates the process.
  if (std::cin.eof()) {
    std::cin.clear();
    for (;;) std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  server.stop();
  return 0;
}
