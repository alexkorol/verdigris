#include "remote_session.hpp"

#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
static constexpr socket_t kInvalidSocket = INVALID_SOCKET;
static void close_socket(socket_t socket) { ::closesocket(socket); }
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_t = int;
static constexpr socket_t kInvalidSocket = -1;
static void close_socket(socket_t socket) { ::close(socket); }
#endif

namespace verdigris::client {

namespace {

using verdigris::networking::Envelope;
using verdigris::networking::JsonValue;

bool send_all(socket_t socket, const void* data, std::size_t size) {
  const char* cursor = static_cast<const char*>(data);
  std::size_t remaining = size;
  while (remaining > 0) {
    const auto sent = ::send(socket, cursor, static_cast<int>(remaining), 0);
    if (sent <= 0) return false;
    cursor += sent;
    remaining -= static_cast<std::size_t>(sent);
  }
  return true;
}

bool recv_all(socket_t socket, void* data, std::size_t size) {
  char* cursor = static_cast<char*>(data);
  std::size_t remaining = size;
  while (remaining > 0) {
    const auto got = ::recv(socket, cursor, static_cast<int>(remaining), 0);
    if (got <= 0) return false;
    cursor += got;
    remaining -= static_cast<std::size_t>(got);
  }
  return true;
}

// The server accepts any syntactically valid Sec-WebSocket-Key; loopback
// development transport does not need nonce randomness (recorded coupling —
// revisit if the transport ever leaves loopback).
constexpr const char* kWebSocketKey = "dGhlIHNhbXBsZSBub25jZQ==";

}  // namespace

RemoteProtocolSession::RemoteProtocolSession(std::string host, std::uint16_t port,
                                             std::string guest_id, bool quick_guest)
    : host_(std::move(host)), port_(port), guest_id_(std::move(guest_id)),
      quick_guest_(quick_guest) {}

RemoteProtocolSession::~RemoteProtocolSession() { shutdown(); }

bool RemoteProtocolSession::start(std::string* error) {
  state_.store(ConnectionState::Connecting);
#ifdef _WIN32
  WSADATA data{};
  if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
    fail(ConnectionState::Rejected, "WSAStartup failed");
    if (error) *error = last_error_;
    return false;
  }
#endif
  const auto socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket == kInvalidSocket) {
    fail(ConnectionState::Rejected, "socket() failed");
    if (error) *error = last_error_;
    return false;
  }
  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(host_.c_str());
  address.sin_port = htons(port_);
  if (::connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
    close_socket(socket);
    fail(ConnectionState::Rejected,
         "connection refused at " + host_ + ":" + std::to_string(port_));
    if (error) *error = last_error_;
    return false;
  }

  const std::string request =
      "GET / HTTP/1.1\r\nHost: " + host_ + ":" + std::to_string(port_) +
      "\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: " +
      kWebSocketKey + "\r\nSec-WebSocket-Version: 13\r\n\r\n";
  if (!send_all(socket, request.data(), request.size())) {
    close_socket(socket);
    fail(ConnectionState::Rejected, "upgrade request send failed");
    if (error) *error = last_error_;
    return false;
  }

  std::string response;
  char buffer[1024];
  while (response.find("\r\n\r\n") == std::string::npos && response.size() < 8192) {
    const auto got = ::recv(socket, buffer, sizeof(buffer), 0);
    if (got <= 0) break;
    response.append(buffer, buffer + got);
  }
  if (response.find(" 101 ") == std::string::npos ||
      response.find("\r\n\r\n") == std::string::npos) {
    close_socket(socket);
    fail(ConnectionState::ProtocolMismatch,
         "endpoint did not complete a websocket upgrade");
    if (error) *error = last_error_;
    return false;
  }

  socket_ = static_cast<std::intptr_t>(socket);
  state_.store(ConnectionState::Connected);
  pending_events_.push_back(
      {PresentationEventType::ConnectionEstablished, "", "", host_, port_});
  running_.store(true);
  reader_ = std::make_unique<std::thread>(&RemoteProtocolSession::reader_loop, this);

  Envelope login{"player:login", JsonValue::Object{
      {"guestId", JsonValue(guest_id_)},
      {"quickGuest", JsonValue(quick_guest_)}}};
  if (!send_envelope(login)) {
    fail(ConnectionState::Disconnected, "login send failed");
    if (error) *error = last_error_;
    return false;
  }
  return true;
}

void RemoteProtocolSession::shutdown() {
  running_.store(false);
  if (socket_ != -1) {
    send_frame(0x8, "");  // close frame; best effort
    close_socket(static_cast<socket_t>(socket_));
    socket_ = -1;
  }
  if (reader_ && reader_->joinable()) reader_->join();
  reader_.reset();
  const auto state = state_.load();
  if (state == ConnectionState::Connected || state == ConnectionState::Ready) {
    state_.store(ConnectionState::Disconnected);
  }
#ifdef _WIN32
  WSACleanup();
#endif
}

void RemoteProtocolSession::submit(const ClientCommand& command) {
  Envelope envelope{"", JsonValue::Object{}};
  switch (command.type) {
    case ClientCommand::Type::Login:
      envelope.event = "player:login";
      envelope.data = JsonValue::Object{{"guestId", JsonValue(command.target)},
                                        {"quickGuest", JsonValue(command.value != 0)}};
      break;
    case ClientCommand::Type::Move: {
      const char* direction = "down";
      if (command.dy < 0) direction = "up";
      else if (command.dy > 0) direction = "down";
      else if (command.dx < 0) direction = "left";
      else if (command.dx > 0) direction = "right";
      envelope.event = "player:move";
      envelope.data = JsonValue::Object{{"direction", JsonValue(direction)}};
      break;
    }
    case ClientCommand::Type::Aim:
      return;  // aim intent is presentation-local until 0061 maps it
    case ClientCommand::Type::UseAction: {
      const char* direction = "down";
      envelope.event = "player:skill:trigger";
      envelope.data = JsonValue::Object{{"skill", JsonValue(command.target)},
                                        {"direction", JsonValue(direction)}};
      break;
    }
    case ClientCommand::Type::PickUp:
      envelope.event = "player:take:underfoot";
      break;
    case ClientCommand::Type::Equip:
      envelope.event = "item:equip";
      envelope.data = JsonValue::Object{{"uuid", JsonValue(command.target)}};
      break;
    case ClientCommand::Type::EnterZone:
      envelope.event = "world:zone:enter";
      envelope.data = JsonValue::Object{{"nodeId", JsonValue(command.target)}};
      break;
    case ClientCommand::Type::Extract:
      return;  // extraction envelope lands with 0061 (server surface pending)
  }
  if (!envelope.event.empty()) send_envelope(envelope);
}

void RemoteProtocolSession::poll() {
  std::deque<std::string> batch;
  {
    std::lock_guard lock(inbox_mutex_);
    batch.swap(inbox_);
  }
  for (const auto& text : batch) {
    Envelope envelope;
    std::string error;
    if (!verdigris::networking::parse_envelope(text, envelope, &error)) {
      pending_events_.push_back({PresentationEventType::ProtocolError, "", "", error, 0});
      continue;
    }
    apply_envelope(envelope);
  }
}

std::vector<PresentationEvent> RemoteProtocolSession::drain_events() {
  std::vector<PresentationEvent> drained;
  drained.swap(pending_events_);
  return drained;
}

bool RemoteProtocolSession::send_envelope(const Envelope& envelope) {
  return send_frame(0x1, verdigris::networking::emit_envelope(envelope));
}

bool RemoteProtocolSession::send_frame(std::uint8_t opcode, const std::string& payload) {
  if (socket_ == -1) return false;
  std::lock_guard lock(send_mutex_);
  std::vector<std::uint8_t> frame;
  frame.push_back(static_cast<std::uint8_t>(0x80 | opcode));
  const auto size = payload.size();
  // Clients MUST mask (RFC6455 5.3); the server enforces this.
  if (size < 126) {
    frame.push_back(static_cast<std::uint8_t>(0x80 | size));
  } else if (size <= 65535) {
    frame.push_back(0x80 | 126);
    frame.push_back(static_cast<std::uint8_t>(size >> 8));
    frame.push_back(static_cast<std::uint8_t>(size));
  } else {
    frame.push_back(0x80 | 127);
    for (int i = 7; i >= 0; --i) {
      frame.push_back(static_cast<std::uint8_t>((size >> (i * 8)) & 0xff));
    }
  }
  // Deterministic mask: loopback development transport (see kWebSocketKey).
  const std::uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
  frame.insert(frame.end(), mask, mask + 4);
  const auto offset = frame.size();
  frame.resize(offset + size);
  for (std::size_t i = 0; i < size; ++i) {
    frame[offset + i] = static_cast<std::uint8_t>(payload[i]) ^ mask[i % 4];
  }
  return send_all(static_cast<socket_t>(socket_), frame.data(), frame.size());
}

void RemoteProtocolSession::reader_loop() {
  const auto socket = static_cast<socket_t>(socket_);
  while (running_.load()) {
    std::uint8_t header[2];
    if (!recv_all(socket, header, 2)) break;
    const auto opcode = static_cast<std::uint8_t>(header[0] & 0x0f);
    const bool masked = (header[1] & 0x80) != 0;
    std::uint64_t length = header[1] & 0x7f;
    if (length == 126) {
      std::uint8_t ext[2];
      if (!recv_all(socket, ext, 2)) break;
      length = static_cast<std::uint64_t>((ext[0] << 8) | ext[1]);
    } else if (length == 127) {
      std::uint8_t ext[8];
      if (!recv_all(socket, ext, 8)) break;
      length = 0;
      for (auto byte : ext) length = (length << 8) | byte;
    }
    if (length > (1u << 20)) break;  // cap: no single game envelope is 1MB
    std::uint8_t mask[4] = {0, 0, 0, 0};
    if (masked && !recv_all(socket, mask, 4)) break;
    std::string payload(static_cast<std::size_t>(length), '\0');
    if (length > 0 && !recv_all(socket, payload.data(), payload.size())) break;
    if (masked) {
      for (std::size_t i = 0; i < payload.size(); ++i) payload[i] ^= mask[i % 4];
    }
    if (opcode == 0x8) break;  // server close
    if (opcode == 0x9) {       // ping -> masked pong
      send_frame(0xA, payload);
      continue;
    }
    if (opcode == 0x1) {
      std::lock_guard lock(inbox_mutex_);
      inbox_.push_back(std::move(payload));
    }
  }
  if (running_.load()) {
    // The peer dropped us; surface it (never fall back to local play).
    state_.store(ConnectionState::Disconnected);
    running_.store(false);
  }
}

void RemoteProtocolSession::apply_envelope(const Envelope& envelope) {
  if (envelope.event == "player:login") {
    if (const auto* player = envelope.data.get("player")) {
      if (const auto* uuid = player->get("uuid"); uuid && uuid->string()) {
        model_.player.uuid = *uuid->string();
      }
      if (const auto* scene = player->get("sceneId"); scene && scene->string()) {
        model_.player.scene_id = *scene->string();
      }
      if (const auto* x = player->get("x"); x && x->number()) model_.player.x = *x->number();
      if (const auto* y = player->get("y"); y && y->number()) model_.player.y = *y->number();
      if (const auto* facing = player->get("facing"); facing && facing->string()) {
        model_.player.facing = *facing->string();
      }
      model_.inventory.clear();
      if (const auto* inventory = player->get("inventory")) {
        if (const auto* slots = inventory->get("slots"); slots && slots->array()) {
          for (const auto& entry : *slots->array()) {
            ClientItemSlot slot;
            if (const auto* id = entry.get("id"); id && id->string()) slot.id = *id->string();
            if (const auto* uuid = entry.get("uuid"); uuid && uuid->string()) slot.uuid = *uuid->string();
            if (const auto* name = entry.get("name"); name && name->string()) slot.name = *name->string();
            if (const auto* index = entry.get("slot"); index && index->number()) {
              slot.slot = static_cast<int>(*index->number());
            }
            model_.inventory.push_back(std::move(slot));
          }
        }
      }
    }
    if (const auto* scene = envelope.data.get("scene")) {
      if (const auto* id = scene->get("id"); id && id->string()) model_.scene.id = *id->string();
      if (const auto* type = scene->get("type"); type && type->string()) model_.scene.type = *type->string();
      if (const auto* name = scene->get("name"); name && name->string()) model_.scene.name = *name->string();
    }
    state_.store(ConnectionState::Ready);
    pending_events_.push_back({PresentationEventType::SessionReady, model_.player.uuid, "", "", 0});
    return;
  }
  if (envelope.event == "player:session-replaced") {
    fail(ConnectionState::Disconnected, "session replaced by a newer connection");
    return;
  }
  if (envelope.event == "game:send:message") {
    if (const auto* text = envelope.data.get("text"); text && text->string()) {
      model_.last_message = *text->string();
      pending_events_.push_back({PresentationEventType::Message, "", "", model_.last_message, 0});
    }
    return;
  }
  // Remaining protocol surfaces (movement echo, combat, inventory refresh)
  // gain typed handling in 0061; unknown envelopes are not errors.
}

void RemoteProtocolSession::fail(ConnectionState state, const std::string& error) {
  last_error_ = error;
  state_.store(state);
  pending_events_.push_back({PresentationEventType::ConnectionLost, "", "", error, 0});
}

}  // namespace verdigris::client
