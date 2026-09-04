#pragma once

#include "verdigris/networking.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace verdigris::networking::account_archive {
// Explicit, versioned field encoding; never write compiler layouts or pointers.
// Lengths are bounded before allocation. The outer checksum detects torn/corrupt
// saves before any session state is changed.
class Archive {
 public:
  bool reading;
  std::vector<std::uint8_t> bytes;
  std::size_t cursor = 0;
  Archive() : reading(false) {}
  explicit Archive(std::vector<std::uint8_t> source)
      : reading(true), bytes(std::move(source)) {}
  template<class... T> void operator()(T&... values) { (field(values), ...); }
  void finish() const {
    if (reading && cursor != bytes.size()) throw std::runtime_error("save has trailing data");
  }
 private:
  std::uint8_t byte(std::uint8_t value = 0) {
    if (!reading) { bytes.push_back(value); return value; }
    if (cursor >= bytes.size()) throw std::runtime_error("truncated account save");
    return bytes[cursor++];
  }
  template<class T> requires(std::is_integral_v<T> || std::is_enum_v<T>)
  void field(T& value) {
    std::uint64_t bits = reading ? 0 : static_cast<std::uint64_t>(value);
    for (unsigned i = 0; i < 8; ++i) {
      if (reading) bits |= static_cast<std::uint64_t>(byte()) << (i * 8);
      else byte(static_cast<std::uint8_t>(bits >> (i * 8)));
    }
    if (reading) value = static_cast<T>(bits);
  }
  void field(double& value) {
    std::uint64_t bits = 0;
    if (!reading) std::memcpy(&bits, &value, sizeof(bits));
    field(bits);
    if (reading) std::memcpy(&value, &bits, sizeof(bits));
  }
  std::uint64_t count(std::size_t size, std::uint64_t maximum) {
    std::uint64_t length = size; field(length);
    if (length > maximum || (reading && length > bytes.size() - cursor))
      throw std::runtime_error("invalid account save length");
    return length;
  }
  void field(std::string& value) {
    const auto length = count(value.size(), 4 * 1024 * 1024);
    if (reading) value.resize(static_cast<std::size_t>(length));
    for (auto& ch : value) ch = static_cast<char>(byte(static_cast<std::uint8_t>(ch)));
  }
  void field(JsonValue& value) {
    std::string text = reading ? std::string() : value.stringify(); field(text);
    if (reading && !parse_json(text, value)) throw std::runtime_error("invalid saved JSON");
  }
  template<class T> void field(std::vector<T>& values) {
    const auto length = count(values.size(), 4 * 1024 * 1024);
    if (reading) values.resize(static_cast<std::size_t>(length));
    for (auto& value : values) field(value);
  }
  void field(std::vector<std::uint8_t>& values) {
    const auto length = count(values.size(), 16 * 1024 * 1024);
    if (reading) values.resize(static_cast<std::size_t>(length));
    for (auto& value : values) value = byte(value);
  }
  template<class T> void field(std::optional<T>& value) {
    bool exists = value.has_value(); field(exists);
    if (reading) { if (exists) value.emplace(); else value.reset(); }
    if (exists) field(*value);
  }
  template<class T> void field(std::set<T>& values) {
    const auto length = count(values.size(), 100000);
    if (reading) {
      values.clear();
      for (std::uint64_t i = 0; i < length; ++i) { T v{}; field(v); values.insert(std::move(v)); }
    } else for (auto value : values) field(value);
  }
  template<class K, class V> void field(std::map<K,V>& values) {
    const auto length = count(values.size(), 100000);
    if (reading) {
      values.clear();
      for (std::uint64_t i = 0; i < length; ++i) {
        K key{}; V value{}; field(key); field(value);
        if (!values.emplace(std::move(key), std::move(value)).second)
          throw std::runtime_error("duplicate save key");
      }
    } else for (auto& [key, value] : values) { auto copy = key; field(copy); field(value); }
  }
  void field(ChannelRatings& v) { (*this)(v.stab,v.slash,v.crush,v.range); }
  void field(CombatModifiers& v) {
    (*this)(v.block_chance,v.critical_chance,v.attack_speed_percent,v.goods_found,
      v.damage_against_beasts,v.bleed_chance,v.reach_percent,v.projectile_range_percent,
      v.armour_penetration_percent,v.movement_speed_percent,v.ember_resistance,v.river_resistance,
      v.health_on_kill_percent,v.attack_speed_on_kill_percent,v.critical_against_bleeding_percent,
      v.health_on_block,v.stationary_block_chance,v.armour_on_hit_percent,
      v.ability_power_high_resource_percent,v.resource_on_kill_percent,v.curse_avoid_percent,
      v.movement_speed_on_kill_percent,v.thrown_avoid_while_moving_percent,v.health_regen_while_moving,
      v.awakened_echoing_kill,v.awakened_last_stand,v.awakened_twinned_voice,v.awakened_untraceable);
  }
  void field(VesselBrand& v) { (*this)(v.id,v.mod_id,v.tier,v.value); }
  void field(VesselBond& v) { (*this)(v.id,v.mod_id,v.theme_id,v.base,v.tier); }
  void field(VesselTrophy& v) { (*this)(v.id,v.trophy_id); }
  void field(VesselAttunement& v) { (*this)(v.xp,v.next,v.theme_counts); }
  void field(VesselAwakened& v) { (*this)(v.name,v.theme_id,v.power,v.flavor); }
  void field(VesselItem& v) {
    (*this)(v.id,v.form_id,v.material_id,v.kind,v.w,v.h,v.ilvl,v.vessel,v.scars,
      v.patience,v.patience_max,v.brands,v.bonds,v.trophies,v.attunement,
      v.evolutions,v.epithet_name,v.awakened);
  }
  void field(TooltipLine& v) { (*this)(v.section,v.text,v.tone); }
  void field(VesselCombat& v) {
    (*this)(v.attack,v.defense,v.modifiers,v.has_attributes,v.attributes,v.resource_health,
      v.resource_mana,v.has_damage,v.damage_min,v.damage_max,v.attacks_per_second,
      v.dps,v.rating,v.channel,v.ward);
  }
  void field(VesselBlock& v) {
    (*this)(v.item,v.pack_id,v.material,v.material_tier,v.form,v.display_name,v.lines,v.combat);
  }
  void field(ItemSize& v) { (*this)(v.width,v.height); }
  void field(ExpeditionMapBlock& v) {
    (*this)(v.tier,v.family,v.objective_key,v.theme,v.layout,v.monster_level_bonus,
      v.monster_life_percent,v.monster_damage_percent,v.extra_monsters,v.goods_found_percent,v.modifiers);
  }
  void field(GameItem& v) {
    (*this)(v.id,v.uuid,v.name,v.display_name,v.qty,v.slot,v.size,v.stackable,v.two_handed,
      v.equip_slot,v.attack,v.defense,v.combat_bonuses,v.bonus_health,v.bonus_mana,
      v.bonus_attributes,v.vessel,v.expedition_map,v.bound_to);
  }
  void field(ActorStats& v) {
    (*this)(v.level,v.strength,v.dexterity,v.intelligence,v.life_max,v.life,
      v.resource_max,v.resource,v.attack,v.defense,v.move_speed,v.attack_speed_ticks,v.resistances);
  }
  void field(HouseProgressionState& v) {
    (*this)(v.choice,v.first_clear_completed,v.reward_claimed,v.scion_gear_tier,v.house_income_per_tick);
  }
};

inline std::uint64_t checksum(const std::vector<std::uint8_t>& bytes) {
  std::uint64_t hash = 1469598103934665603ULL;
  for (auto byte : bytes) hash = (hash ^ byte) * 1099511628211ULL;
  return hash;
}
inline std::vector<std::uint8_t> wrap(std::vector<std::uint8_t> payload) {
  Archive out;
  std::string magic = "Verdigris account";
  int version = 1;
  auto hash = checksum(payload);
  out(magic,version,hash,payload);
  return std::move(out.bytes);
}
inline std::vector<std::uint8_t> unwrap(std::vector<std::uint8_t> bytes) {
  Archive in(std::move(bytes));
  std::string magic; int version = 0; std::uint64_t hash = 0;
  std::vector<std::uint8_t> payload;
  in(magic,version,hash,payload); in.finish();
  if (magic != "Verdigris account" || version != 1 || hash != checksum(payload))
    throw std::runtime_error("unsupported or damaged account save; original preserved");
  return payload;
}
}  // namespace verdigris::networking::account_archive
