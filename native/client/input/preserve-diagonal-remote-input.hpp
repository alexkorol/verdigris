#pragma once

#include <string>
#include <utility>

// VG-MOVE-001: eight-way wire names. A diagonal must keep both axes —
// collapsing to the vertical component is the known defect.

namespace verdigris::client::move {

inline constexpr int kEightWay[8][2] = {
    {1, 0},  {1, 1},  {0, 1},  {-1, 1},
    {-1, 0}, {-1, -1}, {0, -1}, {1, -1},
};

inline std::string encode_eight_way(int dx, int dy) {
  std::string name;
  if (dy < 0) name = "up";
  else if (dy > 0) name = "down";
  if (dx < 0) name += name.empty() ? "left" : "-left";
  else if (dx > 0) name += name.empty() ? "right" : "-right";
  return name;
}

inline std::string collapse_to_vertical(int dx, int dy) {
  if (dy < 0) return "up";
  if (dy > 0) return "down";
  if (dx < 0) return "left";
  if (dx > 0) return "right";
  return {};
}

inline bool diagonal_keeps_both_axes(int dx, int dy, const std::string& name) {
  if (dx == 0 || dy == 0) return true;
  const bool has_x = name.find("left") != std::string::npos ||
                     name.find("right") != std::string::npos;
  const bool has_y = name.find("up") != std::string::npos ||
                     name.find("down") != std::string::npos;
  return has_x && has_y;
}

inline bool all_eight_encode() {
  for (const auto& vec : kEightWay) {
    const std::string name = encode_eight_way(vec[0], vec[1]);
    if (name.empty()) return false;
    if (!diagonal_keeps_both_axes(vec[0], vec[1], name)) return false;
    if (vec[0] != 0 && vec[1] != 0 &&
        collapse_to_vertical(vec[0], vec[1]) == name)
      return false;
  }
  return true;
}

inline const char* owner_eight_way_label() { return "Eight-way"; }
inline const char* owner_up_left_label() { return "Up-left"; }

}  // namespace verdigris::client::move
