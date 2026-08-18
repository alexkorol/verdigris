#include "verdigris/networking.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

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

  verdigris::networking::WebSocketServer server(port);
  std::string error;
  if (!server.start(&error)) {
    std::cerr << "verdigris_server: " << error << "\n";
    return 1;
  }
  std::cout << "verdigris_server listening on ws://127.0.0.1:" << server.port() << "\n";
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
