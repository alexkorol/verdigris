// inventory_grid.hpp — TASK-0171 pure Diablo-style backpack grid model.
//
// Header-only deterministic grid for multi-cell item footprints, placement,
// move/swap rejection, stack metadata, hover hit-testing, and stable ids.
// Matches WIZARD rpg_inventory main backpack geometry (12x6). No heap, no
// rotation, no Win32/GDI, and zero main.cpp integration in this packet.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace inventory_grid {

// WIZARD Brands & Bonds main backpack: horizontal 12x6.
inline constexpr std::uint8_t kDefaultWidth = 12;
inline constexpr std::uint8_t kDefaultHeight = 6;
inline constexpr std::uint8_t kMaxGridWidth = 12;
inline constexpr std::uint8_t kMaxGridHeight = 6;
inline constexpr std::size_t kMaxCells =
    static_cast<std::size_t>(kMaxGridWidth) * kMaxGridHeight;
inline constexpr std::size_t kMaxItems = 48;

struct Item {
  std::uint32_t id = 0;
  std::uint8_t width = 0;
  std::uint8_t height = 0;
  std::uint16_t stack_count = 0;
  std::uint16_t stack_max = 0;
  std::uint8_t x = 0;
  std::uint8_t y = 0;

  [[nodiscard]] constexpr bool valid_shape() const {
    return id != 0 && width > 0 && height > 0 &&
           width <= kMaxGridWidth && height <= kMaxGridHeight &&
           stack_max > 0 && stack_count > 0 && stack_count <= stack_max;
  }

  [[nodiscard]] constexpr bool operator==(const Item&) const = default;
};

struct State {
  std::uint8_t width = kDefaultWidth;
  std::uint8_t height = kDefaultHeight;
  std::uint8_t count = 0;
  std::array<Item, kMaxItems> items{};
  std::array<std::uint32_t, kMaxCells> occupancy{};
  std::int8_t hover_x = -1;
  std::int8_t hover_y = -1;

  [[nodiscard]] constexpr bool valid() const {
    if (width == 0 || height == 0 || width > kMaxGridWidth ||
        height > kMaxGridHeight) {
      return false;
    }
    if (count > kMaxItems) return false;
    for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
      if (!items[i].valid_shape()) return false;
      if (items[i].x + items[i].width > width ||
          items[i].y + items[i].height > height) {
        return false;
      }
    }
    for (std::size_t i = static_cast<std::size_t>(count); i < kMaxItems; ++i) {
      if (items[i].id != 0) return false;
    }
    return occupancy_consistent();
  }

  [[nodiscard]] constexpr bool occupancy_consistent() const {
    std::array<bool, kMaxCells> seen{};
    for (std::uint8_t cy = 0; cy < height; ++cy) {
      for (std::uint8_t cx = 0; cx < width; ++cx) {
        const std::size_t idx = cell_index(cx, cy);
        const std::uint32_t occupant = occupancy[idx];
        if (occupant == 0) continue;
        bool found = false;
        for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
          if (items[i].id != occupant) continue;
          if (cx >= items[i].x && cx < items[i].x + items[i].width &&
              cy >= items[i].y && cy < items[i].y + items[i].height) {
            found = true;
            break;
          }
        }
        if (!found) return false;
        if (seen[idx]) return false;
        seen[idx] = true;
      }
    }
    for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
      for (std::uint8_t dy = 0; dy < items[i].height; ++dy) {
        for (std::uint8_t dx = 0; dx < items[i].width; ++dx) {
          const std::size_t idx =
              cell_index(items[i].x + dx, items[i].y + dy);
          if (occupancy[idx] != items[i].id) return false;
        }
      }
    }
    return true;
  }

  [[nodiscard]] static constexpr std::size_t cell_index(std::uint8_t x,
                                                        std::uint8_t y) {
    return static_cast<std::size_t>(y) * kMaxGridWidth + x;
  }

  [[nodiscard]] constexpr bool operator==(const State&) const = default;
};

enum class Status : std::uint8_t {
  Ok,
  OutOfBounds,
  Overlap,
  NotFound,
  Full,
  InvalidItem,
  InvalidState,
};

[[nodiscard]] constexpr const char* name(Status status) {
  switch (status) {
    case Status::Ok:
      return "ok";
    case Status::OutOfBounds:
      return "out-of-bounds";
    case Status::Overlap:
      return "overlap";
    case Status::NotFound:
      return "not-found";
    case Status::Full:
      return "full";
    case Status::InvalidItem:
      return "invalid-item";
    case Status::InvalidState:
      return "invalid-state";
  }
  return "unknown-status";
}

[[nodiscard]] inline State make_default() {
  State s{};
  s.width = kDefaultWidth;
  s.height = kDefaultHeight;
  return s;
}

[[nodiscard]] constexpr std::size_t find_index(const State& state,
                                               std::uint32_t id) {
  for (std::size_t i = 0; i < static_cast<std::size_t>(state.count); ++i) {
    if (state.items[i].id == id) return i;
  }
  return kMaxItems;
}

[[nodiscard]] constexpr bool region_in_bounds(const State& state, std::uint8_t x,
                                            std::uint8_t y, std::uint8_t w,
                                            std::uint8_t h) {
  return w > 0 && h > 0 && x + w <= state.width && y + h <= state.height;
}

[[nodiscard]] constexpr bool region_overlaps_occupancy(
    const State& state, std::uint8_t x, std::uint8_t y, std::uint8_t w,
    std::uint8_t h, std::uint32_t ignore_id) {
  for (std::uint8_t dy = 0; dy < h; ++dy) {
    for (std::uint8_t dx = 0; dx < w; ++dx) {
      const std::size_t idx = State::cell_index(x + dx, y + dy);
      const std::uint32_t occupant = state.occupancy[idx];
      if (occupant != 0 && occupant != ignore_id) return true;
    }
  }
  return false;
}

[[nodiscard]] constexpr bool can_place(const State& state, std::uint8_t x,
                                       std::uint8_t y, std::uint8_t w,
                                       std::uint8_t h,
                                       std::uint32_t ignore_id = 0) {
  if (!state.valid()) return false;
  if (!region_in_bounds(state, x, y, w, h)) return false;
  return !region_overlaps_occupancy(state, x, y, w, h, ignore_id);
}

[[nodiscard]] constexpr Status rebuild_occupancy(State& state) {
  for (auto& cell : state.occupancy) cell = 0;
  for (std::size_t i = 0; i < static_cast<std::size_t>(state.count); ++i) {
    const Item& item = state.items[i];
    if (!item.valid_shape() ||
        !region_in_bounds(state, item.x, item.y, item.width, item.height)) {
      return Status::InvalidState;
    }
    for (std::uint8_t dy = 0; dy < item.height; ++dy) {
      for (std::uint8_t dx = 0; dx < item.width; ++dx) {
        const std::size_t idx = State::cell_index(item.x + dx, item.y + dy);
        if (state.occupancy[idx] != 0) return Status::Overlap;
        state.occupancy[idx] = item.id;
      }
    }
  }
  return Status::Ok;
}

[[nodiscard]] constexpr Status place(State& state, Item item) {
  if (!state.valid()) return Status::InvalidState;
  if (!item.valid_shape()) return Status::InvalidItem;
  if (state.count >= kMaxItems) return Status::Full;
  if (find_index(state, item.id) != kMaxItems) return Status::Overlap;
  if (!can_place(state, item.x, item.y, item.width, item.height)) {
    if (!region_in_bounds(state, item.x, item.y, item.width, item.height)) {
      return Status::OutOfBounds;
    }
    return Status::Overlap;
  }
  state.items[static_cast<std::size_t>(state.count)] = item;
  ++state.count;
  return rebuild_occupancy(state);
}

[[nodiscard]] constexpr Status remove(State& state, std::uint32_t id) {
  if (!state.valid()) return Status::InvalidState;
  const std::size_t idx = find_index(state, id);
  if (idx == kMaxItems) return Status::NotFound;
  for (std::size_t i = idx; i + 1 < static_cast<std::size_t>(state.count);
       ++i) {
    state.items[i] = state.items[i + 1];
  }
  state.items[static_cast<std::size_t>(state.count) - 1] = Item{};
  --state.count;
  return rebuild_occupancy(state);
}

[[nodiscard]] constexpr Status move(State& state, std::uint32_t id,
                                  std::uint8_t new_x, std::uint8_t new_y) {
  if (!state.valid()) return Status::InvalidState;
  const std::size_t idx = find_index(state, id);
  if (idx == kMaxItems) return Status::NotFound;
  Item& item = state.items[idx];
  if (!can_place(state, new_x, new_y, item.width, item.height, id)) {
    if (!region_in_bounds(state, new_x, new_y, item.width, item.height)) {
      return Status::OutOfBounds;
    }
    return Status::Overlap;
  }
  item.x = new_x;
  item.y = new_y;
  return rebuild_occupancy(state);
}

[[nodiscard]] constexpr Status swap(State& state, std::uint32_t id_a,
                                    std::uint32_t id_b) {
  if (!state.valid()) return Status::InvalidState;
  if (id_a == id_b) return Status::Ok;
  const std::size_t ia = find_index(state, id_a);
  const std::size_t ib = find_index(state, id_b);
  if (ia == kMaxItems || ib == kMaxItems) return Status::NotFound;

  const Item a = state.items[ia];
  const Item b = state.items[ib];

  State scratch = state;
  scratch.items[ia].x = b.x;
  scratch.items[ia].y = b.y;
  scratch.items[ib].x = a.x;
  scratch.items[ib].y = a.y;
  if (rebuild_occupancy(scratch) != Status::Ok) return Status::Overlap;

  state.items[ia].x = b.x;
  state.items[ia].y = b.y;
  state.items[ib].x = a.x;
  state.items[ib].y = a.y;
  return rebuild_occupancy(state);
}

[[nodiscard]] constexpr Status set_hover(State& state, std::int8_t x,
                                         std::int8_t y) {
  if (!state.valid()) return Status::InvalidState;
  if (x < 0 || y < 0 || x >= static_cast<std::int8_t>(state.width) ||
      y >= static_cast<std::int8_t>(state.height)) {
    state.hover_x = -1;
    state.hover_y = -1;
    return Status::OutOfBounds;
  }
  state.hover_x = x;
  state.hover_y = y;
  return Status::Ok;
}

[[nodiscard]] constexpr std::uint32_t item_at(const State& state,
                                              std::uint8_t x, std::uint8_t y) {
  if (!state.valid() || x >= state.width || y >= state.height) return 0;
  return state.occupancy[State::cell_index(x, y)];
}

[[nodiscard]] constexpr std::uint32_t hovered_item(const State& state) {
  if (state.hover_x < 0 || state.hover_y < 0) return 0;
  return item_at(state, static_cast<std::uint8_t>(state.hover_x),
                 static_cast<std::uint8_t>(state.hover_y));
}

[[nodiscard]] constexpr bool has_space_for(const State& state, std::uint8_t w,
                                           std::uint8_t h) {
  if (!state.valid() || w == 0 || h == 0) return false;
  for (std::uint8_t y = 0; y + h <= state.height; ++y) {
    for (std::uint8_t x = 0; x + w <= state.width; ++x) {
      if (can_place(state, x, y, w, h)) return true;
    }
  }
  return false;
}

[[nodiscard]] constexpr bool is_full(const State& state) {
  return !has_space_for(state, 1, 1);
}

// Deterministic serialization order: ascending stable item id.
[[nodiscard]] constexpr std::array<std::uint8_t, kMaxItems>
serialization_order(const State& state) {
  std::array<std::uint8_t, kMaxItems> order{};
  std::uint8_t n = 0;
  for (std::uint8_t i = 0; i < state.count; ++i) order[n++] = i;

  for (std::uint8_t a = 0; a + 1 < n; ++a) {
    for (std::uint8_t b = a + 1; b < n; ++b) {
      const Item& left = state.items[order[a]];
      const Item& right = state.items[order[b]];
      if (right.id < left.id) {
        const std::uint8_t tmp = order[a];
        order[a] = order[b];
        order[b] = tmp;
      }
    }
  }
  return order;
}

}  // namespace inventory_grid
