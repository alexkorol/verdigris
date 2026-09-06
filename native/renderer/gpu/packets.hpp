#pragma once

// VG-GPU-002: backend-neutral packets copied from the semantic render list.
// Headless scenarios stay deterministic with no GPU/D3D/GL object in the
// recorded snapshot. A non-zero backend_handle is a defect.

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "../../client/render_list.hpp"

namespace verdigris::gpu {

struct Packet {
  std::uint16_t op = 0;
  float x = 0.0f;
  float y = 0.0f;
  float radius = 0.0f;
  std::int32_t value = 0;
  std::string label;
  // Isolated GPU resources (textures, HDC, device pointers) must never be
  // smuggled through this field into a snapshot.
  std::uint64_t backend_handle = 0;
};

inline std::vector<Packet> packets_from_render_list(const render::List& list) {
  std::vector<Packet> out;
  out.reserve(list.size());
  for (const auto& item : list) {
    Packet packet;
    packet.op = static_cast<std::uint16_t>(item.op);
    packet.x = static_cast<float>(item.x);
    packet.y = static_cast<float>(item.y);
    packet.radius = static_cast<float>(item.radius);
    packet.value = item.value;
    packet.label = item.label;
    packet.backend_handle = 0;
    out.push_back(std::move(packet));
  }
  return out;
}

inline bool snapshot_valid(const std::vector<Packet>& packets) {
  for (const auto& packet : packets)
    if (packet.backend_handle != 0) return false;
  return true;
}

inline std::string snapshot_text(const std::vector<Packet>& packets) {
  std::string text;
  text.reserve(packets.size() * 48);
  for (const auto& packet : packets) {
    char line[256];
    std::snprintf(line, sizeof(line), "%u %.3f %.3f %.3f %d %s\n",
                  static_cast<unsigned>(packet.op),
                  static_cast<double>(packet.x), static_cast<double>(packet.y),
                  static_cast<double>(packet.radius), packet.value,
                  packet.label.c_str());
    text += line;
  }
  return text;
}

inline bool snapshot_mentions_backend(const std::string& text) {
  return text.find("hdc") != std::string::npos ||
         text.find("HDC") != std::string::npos ||
         text.find("d3d") != std::string::npos ||
         text.find("0x") != std::string::npos;
}

inline const char* owner_handle_free_label() { return "Handle-free"; }
inline const char* owner_telegraph_class_label() { return "Telegraph class"; }

}  // namespace verdigris::gpu
