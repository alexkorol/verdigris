#pragma once

// VG-GPU-004: software present of the live session's GPU packets.
// Actors, world, effects, and HUD must arrive through packets_from_render_list
// after a simulation present. A standalone textured-quad demo is not this scene.

#include <cstdint>
#include <cstring>
#include <vector>

#include "cook-shaders-and-resource-bindings.hpp"
#include "packets.hpp"

namespace verdigris::gpu {

struct ReferenceCensus {
  int actors = 0;
  int world = 0;
  int effects = 0;
  int hud = 0;
  bool target_sheet = false;
};

inline ReferenceCensus census_packets(const std::vector<Packet>& packets) {
  ReferenceCensus c{};
  for (const auto& packet : packets) {
    const auto op = static_cast<render::Op>(packet.op);
    if (op == render::Op::Player || op == render::Op::Monster ||
        op == render::Op::Npc)
      ++c.actors;
    else if (op == render::Op::Floor || op == render::Op::Tile ||
             op == render::Op::Scenery)
      ++c.world;
    else if (op == render::Op::Telegraph || op == render::Op::Swing ||
             op == render::Op::Sweep || op == render::Op::WarCry ||
             op == render::Op::Impact || op == render::Op::Drop)
      ++c.effects;
    else if (op == render::Op::Hud || op == render::Op::Orb ||
             op == render::Op::Quickbar || op == render::Op::Minimap)
      ++c.hud;
    if (op == render::Op::Hud && packet.label == "target:camera:top-down")
      c.target_sheet = true;
  }
  return c;
}

inline bool session_scene_complete(const ReferenceCensus& c) {
  return c.actors > 0 && c.world > 0 && c.hud > 0 && c.target_sheet;
}

inline bool present_reference_scene(Sample& sample, const std::vector<Packet>& packets,
                                    int src_w, int src_h, bool session_live) {
  if (!session_live || !sample.alive || src_w <= 0 || src_h <= 0) return false;
  if (packets.empty() || !snapshot_valid(packets)) return false;
  const ReferenceCensus census = census_packets(packets);
  if (!session_scene_complete(census)) return false;

  Bindings bindings{};
  if (!load_bindings(Backend::Software, kBindingLayoutVersion, &bindings))
    return false;

  const int w = sample.width;
  const int h = sample.height;
  for (const auto& packet : packets) {
    int px = static_cast<int>(packet.x * static_cast<float>(w) /
                              static_cast<float>(src_w));
    int py = static_cast<int>(packet.y * static_cast<float>(h) /
                              static_cast<float>(src_h));
    if (px < 0) px = 0;
    if (py < 0) py = 0;
    if (px >= w) px = w - 1;
    if (py >= h) py = h - 1;
    const auto op = static_cast<render::Op>(packet.op);
    std::uint32_t color = shade_texel(bindings, px % bindings.map_size,
                                      py % bindings.map_size);
    if (op == render::Op::Player) color = 0x00E8C878u;
    else if (op == render::Op::Monster) color = 0x00C45A48u;
    else if (op == render::Op::Hud || op == render::Op::Orb)
      color = 0x00F2E6D4u;
    else if (op == render::Op::Telegraph || op == render::Op::Impact)
      color = 0x00E8C878u;
    sample.pixels[static_cast<std::size_t>(py * w + px)] = color;
    if (px + 1 < w)
      sample.pixels[static_cast<std::size_t>(py * w + px + 1)] = color;
    if (py + 1 < h)
      sample.pixels[static_cast<std::size_t>((py + 1) * w + px)] = color;
  }
  return true;
}

}  // namespace verdigris::gpu
