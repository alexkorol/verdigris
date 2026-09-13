// instance_refresh.hpp — TASK-0176 persistent instance refresh policy.
//
// Deterministic reuse vs fresh-instance requests, expiry, and town/non-
// refreshable rejection. Complements gate_interaction FreshInstance intent.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace instance_refresh {

inline constexpr std::size_t kMaxInstances = 16;

enum class ZoneKind : std::uint8_t {
  Town = 0,
  Combat,
};

enum class Request : std::uint8_t {
  Reuse = 0,
  FreshInstance,
};

enum class Outcome : std::uint8_t {
  ReuseExisting = 0,
  CreateFresh,
  RejectedTownFresh,
  RejectedExpired,
  RejectedNotRefreshable,
};

enum class Status : std::uint8_t {
  Ok,
  Invalid,
};

enum class MessageCode : std::uint8_t {
  None = 0,
  Reused,
  FreshCreated,
  TownNoFresh,
  Expired,
  NotRefreshable,
};

struct ZonePolicy {
  std::uint32_t zone_id = 0;
  ZoneKind kind = ZoneKind::Combat;
  bool allow_fresh = true;
  std::uint32_t lifetime_ticks = 0;  // 0 = no expiry

  [[nodiscard]] constexpr bool valid() const { return zone_id != 0; }
};

struct InstanceRecord {
  std::uint32_t instance_id = 0;
  std::uint32_t zone_id = 0;
  std::uint32_t created_tick = 0;
  std::uint32_t expires_tick = 0;  // 0 = never
  bool active = false;

  [[nodiscard]] constexpr bool expired_at(std::uint32_t tick) const {
    return active && expires_tick != 0 && tick >= expires_tick;
  }

  [[nodiscard]] constexpr bool past_expiry(std::uint32_t tick) const {
    return expires_tick != 0 && tick >= expires_tick;
  }
};

struct Registry {
  std::array<InstanceRecord, kMaxInstances> instances{};
  std::uint8_t count = 0;
  std::uint32_t next_instance_id = 1;
  std::uint32_t tick = 0;
};

struct Decision {
  Status status = Status::Ok;
  Outcome outcome = Outcome::ReuseExisting;
  MessageCode message = MessageCode::None;
  std::uint32_t instance_id = 0;

  [[nodiscard]] constexpr bool operator==(const Decision&) const = default;
};

[[nodiscard]] constexpr const char* message_text(MessageCode code) {
  switch (code) {
    case MessageCode::None:
      return "";
    case MessageCode::Reused:
      return "Reusing persistent instance.";
    case MessageCode::FreshCreated:
      return "Fresh instance created.";
    case MessageCode::TownNoFresh:
      return "Town areas cannot be refreshed.";
    case MessageCode::Expired:
      return "Instance expired; create a fresh expedition.";
    case MessageCode::NotRefreshable:
      return "This zone cannot be force-refreshed.";
  }
  return "";
}

[[nodiscard]] constexpr std::int8_t find_active_instance(const Registry& reg,
                                                         std::uint32_t zone_id,
                                                         std::uint32_t tick) {
  for (std::uint8_t i = 0; i < reg.count; ++i) {
    const InstanceRecord& inst =
        reg.instances[static_cast<std::size_t>(i)];
    if (!inst.active || inst.zone_id != zone_id) continue;
    if (inst.expired_at(tick)) continue;
    return static_cast<std::int8_t>(i);
  }
  return -1;
}

[[nodiscard]] constexpr Status push_instance(Registry& reg,
                                           std::uint32_t zone_id,
                                           const ZonePolicy& zone) {
  if (reg.count >= kMaxInstances) return Status::Invalid;
  InstanceRecord inst;
  inst.instance_id = reg.next_instance_id++;
  inst.zone_id = zone_id;
  inst.created_tick = reg.tick;
  inst.expires_tick =
      zone.lifetime_ticks == 0 ? 0 : reg.tick + zone.lifetime_ticks;
  inst.active = true;
  reg.instances[static_cast<std::size_t>(reg.count)] = inst;
  ++reg.count;
  return Status::Ok;
}

constexpr void expire_stale(Registry& reg) {
  for (std::uint8_t i = 0; i < reg.count; ++i) {
    InstanceRecord& inst = reg.instances[static_cast<std::size_t>(i)];
    if (inst.active && inst.expired_at(reg.tick)) {
      inst.active = false;
    }
  }
}

[[nodiscard]] constexpr Decision evaluate(Registry& reg, const ZonePolicy& zone,
                                          Request request) {
  Decision out;
  if (!zone.valid()) {
    out.status = Status::Invalid;
    return out;
  }

  expire_stale(reg);

  if (request == Request::FreshInstance) {
    if (zone.kind == ZoneKind::Town) {
      out.outcome = Outcome::RejectedTownFresh;
      out.message = MessageCode::TownNoFresh;
      return out;
    }
    if (!zone.allow_fresh) {
      out.outcome = Outcome::RejectedNotRefreshable;
      out.message = MessageCode::NotRefreshable;
      return out;
    }
    if (push_instance(reg, zone.zone_id, zone) != Status::Ok) {
      out.status = Status::Invalid;
      return out;
    }
    out.outcome = Outcome::CreateFresh;
    out.message = MessageCode::FreshCreated;
    out.instance_id = reg.instances[static_cast<std::size_t>(reg.count - 1)]
                          .instance_id;
    return out;
  }

  const std::int8_t idx = find_active_instance(reg, zone.zone_id, reg.tick);
  if (idx >= 0) {
    out.outcome = Outcome::ReuseExisting;
    out.message = MessageCode::Reused;
    out.instance_id =
        reg.instances[static_cast<std::size_t>(idx)].instance_id;
    return out;
  }

  for (std::uint8_t i = 0; i < reg.count; ++i) {
    const InstanceRecord& inst = reg.instances[static_cast<std::size_t>(i)];
    if (inst.zone_id == zone.zone_id && inst.past_expiry(reg.tick)) {
      out.outcome = Outcome::RejectedExpired;
      out.message = MessageCode::Expired;
      return out;
    }
  }

  if (push_instance(reg, zone.zone_id, zone) != Status::Ok) {
    out.status = Status::Invalid;
    return out;
  }
  out.outcome = Outcome::CreateFresh;
  out.message = MessageCode::FreshCreated;
  out.instance_id = reg.instances[static_cast<std::size_t>(reg.count - 1)]
                        .instance_id;
  return out;
}

[[nodiscard]] constexpr void advance_tick(Registry& reg) {
  ++reg.tick;
  expire_stale(reg);
}

}  // namespace instance_refresh
