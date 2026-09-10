#pragma once

#include <cmath>
#include <string_view>

namespace raster_scenery {

// Select art for existing dwelling landmarks only. This function does not
// create scenery or alter its position, scale, solidity or collision radius.
inline const char* dwelling_asset(std::string_view route_id, int x, int y,
                                  double tile_units) {
  if (route_id.rfind("town:", 0) == 0) {
    // Mara's general stall, matching generate_scenery's rounded world point.
    const int stall_x = static_cast<int>(std::lround(49.0 * tile_units));
    const int stall_y = static_cast<int>(std::lround(102.0 * tile_units));
    return x == stall_x && y == stall_y ? "storehut" : "hut";
  }
  // The existing foreground dwelling gives each village one timber store
  // among its two mudbrick homes. Fields and later landmarks retain hut art.
  if (route_id.find(":1:") != std::string_view::npos && x == -420 && y == 180)
    return "storehut";
  return "hut";
}

}  // namespace raster_scenery
