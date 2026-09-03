#include "verdigris/networking.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_length_t = int;
using socket_t = SOCKET;
constexpr socket_t invalid_socket = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using socket_length_t = socklen_t;
using socket_t = int;
constexpr socket_t invalid_socket = -1;
#endif

namespace verdigris::networking {
namespace {

void close_socket(socket_t socket) {
  if (socket == invalid_socket) return;
#ifdef _WIN32
  closesocket(socket);
#else
  ::close(socket);
#endif
}

bool send_all(socket_t socket, const void* data, std::size_t size) {
  const auto* bytes = static_cast<const char*>(data);
  while (size) {
#ifdef _WIN32
    const int sent = ::send(socket, bytes, static_cast<int>(size), 0);
#else
    const auto sent = ::send(socket, bytes, size, MSG_NOSIGNAL);
#endif
    if (sent <= 0) return false;
    bytes += sent;
    size -= static_cast<std::size_t>(sent);
  }
  return true;
}

bool recv_all(socket_t socket, void* data, std::size_t size) {
  auto* bytes = static_cast<char*>(data);
  while (size) {
#ifdef _WIN32
    const int got = ::recv(socket, bytes, static_cast<int>(size), 0);
#else
    const auto got = ::recv(socket, bytes, size, 0);
#endif
    if (got <= 0) return false;
    bytes += got;
    size -= static_cast<std::size_t>(got);
  }
  return true;
}

std::string json_escape(const std::string& input) {
  std::ostringstream output;
  for (unsigned char ch : input) {
    switch (ch) {
      case '"': output << "\\\""; break;
      case '\\': output << "\\\\"; break;
      case '\b': output << "\\b"; break;
      case '\f': output << "\\f"; break;
      case '\n': output << "\\n"; break;
      case '\r': output << "\\r"; break;
      case '\t': output << "\\t"; break;
      default:
        if (ch < 0x20) {
          output << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                 << static_cast<int>(ch) << std::dec;
        } else output << static_cast<char>(ch);
    }
  }
  return output.str();
}

class JsonParser {
 public:
  explicit JsonParser(const std::string& text) : text_(text) {}

  bool parse(JsonValue& result, std::string* error) {
    skip();
    if (!value(result)) return fail("invalid JSON value", error);
    skip();
    if (position_ != text_.size()) return fail("trailing JSON data", error);
    return true;
  }

 private:
  bool fail(const char* message, std::string* error) {
    if (error) *error = std::string(message) + " at byte " + std::to_string(position_);
    return false;
  }
  void skip() { while (position_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[position_]))) ++position_; }
  bool consume(char expected) {
    skip();
    if (position_ >= text_.size() || text_[position_] != expected) return false;
    ++position_;
    return true;
  }
  bool value(JsonValue& result) {
    skip();
    if (position_ >= text_.size()) return false;
    const char ch = text_[position_];
    if (ch == '{') return object(result);
    if (ch == '[') return array(result);
    if (ch == '"') { std::string value; if (!string(value)) return false; result = JsonValue(std::move(value)); return true; }
    if (text_.compare(position_, 4, "true") == 0) { position_ += 4; result = JsonValue(true); return true; }
    if (text_.compare(position_, 5, "false") == 0) { position_ += 5; result = JsonValue(false); return true; }
    if (text_.compare(position_, 4, "null") == 0) { position_ += 4; result = JsonValue(nullptr); return true; }
    char* end = nullptr;
    const auto* start = text_.c_str() + position_;
    const double number = std::strtod(start, &end);
    if (end == start) return false;
    position_ += static_cast<std::size_t>(end - start);
    result = JsonValue(number);
    return true;
  }
  bool string(std::string& result) {
    if (!consume('"')) return false;
    while (position_ < text_.size()) {
      const char ch = text_[position_++];
      if (ch == '"') return true;
      if (ch != '\\') { result += ch; continue; }
      if (position_ >= text_.size()) return false;
      const char escaped = text_[position_++];
      switch (escaped) {
        case '"': result += '"'; break; case '\\': result += '\\'; break;
        case '/': result += '/'; break; case 'b': result += '\b'; break;
        case 'f': result += '\f'; break; case 'n': result += '\n'; break;
        case 'r': result += '\r'; break; case 't': result += '\t'; break;
        case 'u': {
          if (position_ + 4 > text_.size()) return false;
          unsigned value = 0;
          for (int i = 0; i < 4; ++i) {
            const char hex = text_[position_++];
            value <<= 4;
            if (hex >= '0' && hex <= '9') value += hex - '0';
            else if (hex >= 'a' && hex <= 'f') value += hex - 'a' + 10;
            else if (hex >= 'A' && hex <= 'F') value += hex - 'A' + 10;
            else return false;
          }
          if (value <= 0x7f) result += static_cast<char>(value);
          else if (value <= 0x7ff) { result += static_cast<char>(0xc0 | (value >> 6)); result += static_cast<char>(0x80 | (value & 0x3f)); }
          else { result += static_cast<char>(0xe0 | (value >> 12)); result += static_cast<char>(0x80 | ((value >> 6) & 0x3f)); result += static_cast<char>(0x80 | (value & 0x3f)); }
          break;
        }
        default: return false;
      }
    }
    return false;
  }
  bool object(JsonValue& result) {
    if (!consume('{')) return false;
    JsonValue::Object object;
    skip();
    if (consume('}')) { result = JsonValue(std::move(object)); return true; }
    for (;;) {
      std::string key;
      if (!string(key) || !consume(':')) return false;
      JsonValue value;
      if (!value_(value)) return false;
      object.emplace(std::move(key), std::move(value));
      if (consume('}')) { result = JsonValue(std::move(object)); return true; }
      if (!consume(',')) return false;
    }
  }
  bool array(JsonValue& result) {
    if (!consume('[')) return false;
    JsonValue::Array array;
    skip();
    if (consume(']')) { result = JsonValue(std::move(array)); return true; }
    for (;;) {
      JsonValue value;
      if (!value_(value)) return false;
      array.push_back(std::move(value));
      if (consume(']')) { result = JsonValue(std::move(array)); return true; }
      if (!consume(',')) return false;
    }
  }
  bool value_(JsonValue& result) { return value(result); }
  const std::string& text_;
  std::size_t position_ = 0;
};

void put(JsonValue::Object& object, const std::string& key, JsonValue value) { object.emplace(key, std::move(value)); }
int as_int(const JsonValue* value, int fallback = 0) {
  return value && value->number() ? static_cast<int>(*value->number()) : fallback;
}
std::string as_string(const JsonValue* value, const std::string& fallback = {}) {
  return value && value->string() ? *value->string() : fallback;
}
bool as_bool(const JsonValue* value, bool fallback = false) {
  return value && value->boolean() ? *value->boolean() : fallback;
}

// ── N5 chronicle roster helpers (mortal-oath persistence) ──────────────────
// The living roster is the only durable home for a scion's sworn oath in the
// native server (no database seam yet). Admissions persist it so a re-login
// restores the hard lifecycle exactly like the JS beginScionSession rebuild
// (server/core/services/chronicles.js forces lifecycle.mode 'hard' on every
// set-out).
JsonValue::Object* find_chronicle_house_object(JsonValue& chronicle, const std::string& house_id) {
  if (auto* root = chronicle.object()) {
    auto houses_it = root->find("houses");
    if (houses_it != root->end() && houses_it->second.array()) {
      for (auto& entry : *houses_it->second.array()) {
        auto* house = entry.object();
        if (!house) continue;
        auto id_it = house->find("id");
        if (id_it != house->end() && id_it->second.string()
            && *id_it->second.string() == house_id) return house;
      }
    }
  }
  return nullptr;
}

JsonValue::Object* find_chronicle_scion_object(JsonValue& chronicle,
                                                const std::string& house_id,
                                                const std::string& scion_id) {
  JsonValue::Object* house = find_chronicle_house_object(chronicle, house_id);
  if (!house) return nullptr;
  auto scions_it = house->find("scions");
  if (scions_it == house->end() || !scions_it->second.array()) return nullptr;
  for (auto& entry : *scions_it->second.array()) {
    auto* scion = entry.object();
    if (!scion) continue;
    auto id_it = scion->find("id");
    if (id_it != scion->end() && id_it->second.string() &&
        *id_it->second.string() == scion_id)
      return scion;
  }
  return nullptr;
}

void set_scion_record_mortal(JsonValue& chronicle, const std::string& house_id,
                             const std::string& scion_id, bool mortal) {
  JsonValue::Object* house = find_chronicle_house_object(chronicle, house_id);
  if (!house) return;
  auto scions_it = house->find("scions");
  if (scions_it == house->end() || !scions_it->second.array()) return;
  for (auto& entry : *scions_it->second.array()) {
    auto* scion = entry.object();
    if (!scion) continue;
    auto id_it = scion->find("id");
    if (id_it == scion->end() || !id_it->second.string()
        || *id_it->second.string() != scion_id) continue;
    (*scion)["mortal"] = JsonValue(mortal);
    return;
  }
}

bool scion_record_mortal(const JsonValue& chronicle, const std::string& house_id,
                         const std::string& scion_id) {
  const auto* root = chronicle.object();
  if (!root) return false;
  auto houses_it = root->find("houses");
  if (houses_it == root->end() || !houses_it->second.array()) return false;
  for (const auto& house_entry : *houses_it->second.array()) {
    const auto* house = house_entry.object();
    if (!house) continue;
    auto id_it = house->find("id");
    if (id_it == house->end() || !id_it->second.string()
        || *id_it->second.string() != house_id) continue;
    auto scions_it = house->find("scions");
    if (scions_it == house->end() || !scions_it->second.array()) return false;
    for (const auto& scion_entry : *scions_it->second.array()) {
      const auto* scion = scion_entry.object();
      if (!scion) continue;
      auto sid_it = scion->find("id");
      if (sid_it == scion->end() || !sid_it->second.string()
          || *sid_it->second.string() != scion_id) continue;
      auto mortal_it = scion->find("mortal");
      return mortal_it != scion->end() && as_bool(&mortal_it->second);
    }
  }
  return false;
}

// ── N4 item wire shapes (server/player/handlers/dev.js buildStateSnapshot) ──

JsonValue ratings_json(const ChannelRatings& ratings) {
  JsonValue::Object out;
  put(out, "stab", ratings.stab);
  put(out, "slash", ratings.slash);
  put(out, "crush", ratings.crush);
  put(out, "range", ratings.range);
  return JsonValue(std::move(out));
}

JsonValue stats_json(const GameItem& item) {
  JsonValue::Object out;
  put(out, "attack", ratings_json(item.attack));
  put(out, "defense", ratings_json(item.defense));
  return JsonValue(std::move(out));
}

// adapter.js deriveVesselCombat modifiers: zero-valued keys are omitted and
// an all-zero block is null.
JsonValue modifiers_json(const CombatModifiers& mods) {
  JsonValue::Object out;
  if (mods.block_chance > 0) put(out, "blockChance", mods.block_chance);
  if (mods.critical_chance > 0) put(out, "criticalChance", mods.critical_chance);
  if (mods.goods_found > 0) put(out, "goodsFound", mods.goods_found);
  if (mods.damage_against_beasts > 0) put(out, "damageAgainstBeasts", mods.damage_against_beasts);
  if (out.empty()) return JsonValue(nullptr);
  return JsonValue(std::move(out));
}

JsonValue vessel_json(const VesselBlock& block) {
  JsonValue::Object item;
  put(item, "v", 1);
  put(item, "id", block.item.id);
  put(item, "formId", block.item.form_id);
  put(item, "materialId", block.item.material_id);
  put(item, "kind", block.item.kind);
  put(item, "w", block.item.w);
  put(item, "h", block.item.h);
  put(item, "ilvl", block.item.ilvl);
  put(item, "vessel", block.item.vessel);
  put(item, "scars", block.item.scars);
  put(item, "patienceMax", block.item.patience_max);
  put(item, "patience", block.item.patience);
  JsonValue::Array brands;
  for (const auto& brand : block.item.brands) {
    JsonValue::Object entry;
    put(entry, "id", brand.id);
    put(entry, "modId", brand.mod_id);
    put(entry, "tier", brand.tier);
    put(entry, "value", brand.value);
    brands.emplace_back(std::move(entry));
  }
  put(item, "brands", std::move(brands));
  put(item, "bonds", JsonValue::Array{});
  put(item, "trophies", JsonValue::Array{});
  put(item, "att", JsonValue::Object{{"xp", 0}, {"next", 80}, {"tc", JsonValue::Object{}}});
  put(item, "evolutions", 0);
  put(item, "fired", 0);
  if (block.item.epithet_name.empty()) put(item, "epithetName", nullptr);
  else put(item, "epithetName", block.item.epithet_name);
  put(item, "awakened", nullptr);

  JsonValue::Array lines;
  for (const auto& line : block.lines) {
    JsonValue::Object entry;
    put(entry, "section", line.section);
    put(entry, "text", line.text);
    put(entry, "tone", line.tone);
    lines.emplace_back(std::move(entry));
  }

  JsonValue::Object combat;
  if (block.combat.has_damage) {
    JsonValue::Object damage;
    put(damage, "minimum", block.combat.damage_min);
    put(damage, "maximum", block.combat.damage_max);
    put(damage, "attacksPerSecond", block.combat.attacks_per_second);
    put(damage, "dps", block.combat.dps);
    put(damage, "channel", block.combat.channel);
    put(damage, "rating", block.combat.rating);
    put(combat, "damage", std::move(damage));
  } else {
    put(combat, "damage", nullptr);
  }
  put(combat, "ward", block.combat.ward);
  if (block.combat.has_attributes) {
    put(combat, "attributes", JsonValue::Object{{"strength", block.combat.attributes},
                                                {"dexterity", block.combat.attributes},
                                                {"intelligence", block.combat.attributes}});
  } else {
    put(combat, "attributes", nullptr);
  }
  if (block.combat.resource_health > 0 || block.combat.resource_mana > 0) {
    put(combat, "resources", JsonValue::Object{{"health", block.combat.resource_health},
                                               {"mana", block.combat.resource_mana}});
  } else {
    put(combat, "resources", nullptr);
  }
  put(combat, "modifiers", modifiers_json(block.combat.modifiers));
  put(combat, "ratings", JsonValue::Object{{"attack", ratings_json(block.combat.attack)},
                                           {"defense", ratings_json(block.combat.defense)}});

  JsonValue::Object out;
  put(out, "packId", block.pack_id);
  put(out, "item", std::move(item));
  put(out, "material", block.material);
  put(out, "materialTier", block.material_tier);
  put(out, "form", block.form);
  put(out, "displayName", block.display_name);
  put(out, "lines", std::move(lines));
  put(out, "combat", std::move(combat));
  return JsonValue(std::move(out));
}

JsonValue vessel_or_null(const GameItem& item) {
  if (!item.vessel) return JsonValue(nullptr);
  return vessel_json(*item.vessel);
}

JsonValue expedition_map_or_null(const GameItem& item) {
  if (!item.expedition_map) return JsonValue(nullptr);
  const ExpeditionMapBlock& map = *item.expedition_map;
  JsonValue::Array modifiers;
  for (const auto& modifier : map.modifiers) modifiers.emplace_back(modifier);
  JsonValue::Object out;
  put(out, "tier", map.tier);
  put(out, "theme", map.theme);
  put(out, "layout", map.layout);
  put(out, "monsterLevelBonus", map.monster_level_bonus);
  put(out, "monsterLifePercent", map.monster_life_percent);
  put(out, "monsterDamagePercent", map.monster_damage_percent);
  put(out, "extraMonsters", map.extra_monsters);
  put(out, "goodsFoundPercent", map.goods_found_percent);
  put(out, "modifiers", std::move(modifiers));
  return JsonValue(std::move(out));
}

int item_level_of(const GameItem& item) { return item.item_level(); }

// dev.js snapshotItem.
JsonValue snapshot_item_json(const GameItem& item) {
  JsonValue::Object out;
  put(out, "id", item.id);
  put(out, "uuid", item.uuid);
  put(out, "name", item.name);
  put(out, "qty", item.qty);
  if (item.slot >= 0) put(out, "slot", item.slot);
  else put(out, "slot", nullptr);
  put(out, "size", JsonValue::Object{{"width", item.size.width}, {"height", item.size.height}});
  if (item.vessel) put(out, "itemLevel", item_level_of(item));
  else put(out, "itemLevel", nullptr);
  put(out, "stats", stats_json(item));
  put(out, "vessel", vessel_or_null(item));
  put(out, "expeditionMap", expedition_map_or_null(item));
  return JsonValue(std::move(out));
}

// dev.js itemIdentity (server/shared item identity projection).
JsonValue item_identity_json(const GameItem& item) {
  JsonValue::Object out;
  put(out, "slot", item.slot >= 0 ? JsonValue(item.slot) : JsonValue(nullptr));
  put(out, "id", item.id);
  put(out, "uuid", item.uuid);
  put(out, "name", item.name);
  put(out, "displayName", item.display_name);
  put(out, "qty", item.qty);
  if (item.slot >= 0) {
    put(out, "slot", item.slot);
    put(out, "position", JsonValue::Object{{"x", item.slot % 12}, {"y", item.slot / 12}});
  } else {
    put(out, "slot", nullptr);
    put(out, "position", nullptr);
  }
  if (item.bound_to.empty()) put(out, "boundTo", nullptr);
  else put(out, "boundTo", item.bound_to);
  put(out, "affixes", JsonValue::Object{{"brand", nullptr}, {"bond", nullptr}});
  put(out, "vessel", vessel_or_null(item));
  put(out, "expeditionMap", expedition_map_or_null(item));
  put(out, "stats", stats_json(item));
  if (item.bonus_attributes > 0) {
    put(out, "attributes", JsonValue::Object{{"strength", item.bonus_attributes},
                                             {"dexterity", item.bonus_attributes},
                                             {"intelligence", item.bonus_attributes}});
  } else {
    put(out, "attributes", nullptr);
  }
  if (item.bonus_health > 0 || item.bonus_mana > 0) {
    put(out, "resourceBonuses", JsonValue::Object{{"health", item.bonus_health},
                                                  {"mana", item.bonus_mana}});
  } else {
    put(out, "resourceBonuses", nullptr);
  }
  put(out, "combatBonuses", modifiers_json(item.combat_bonuses));
  put(out, "size", JsonValue::Object{{"width", item.size.width}, {"height", item.size.height}});
  return JsonValue(std::move(out));
}

// dev.js groundItems entry.
JsonValue ground_item_json(const GroundItem& ground) {
  JsonValue::Object out;
  put(out, "id", ground.item.id);
  put(out, "uuid", ground.item.uuid);
  put(out, "name", ground.item.name);
  put(out, "displayName", ground.item.display_name);
  if (ground.item.bound_to.empty()) put(out, "boundTo", nullptr);
  else put(out, "boundTo", ground.item.bound_to);
  put(out, "x", ground.x);
  put(out, "y", ground.y);
  put(out, "qty", ground.item.qty);
  if (ground.item.vessel) put(out, "itemLevel", ground.item.item_level());
  else put(out, "itemLevel", nullptr);
  put(out, "stats", stats_json(ground.item));
  put(out, "vessel", vessel_or_null(ground.item));
  put(out, "expeditionMap", expedition_map_or_null(ground.item));
  if (!ground.relic_record_id.empty()) {
    JsonValue::Object relic;
    put(relic, "relicId", ground.relic_record_id);
    put(relic, "scionId", ground.relic_source_scion_id_field);
    put(relic, "scionName", ground.relic_source_scion_name_field);
    put(out, "chroniclesRelic", std::move(relic));
  }
  if (!ground.relic_record_id.empty()) {
    put(out, "legacyRelicId", ground.relic_record_id);
    JsonValue::Object legacy;
    if (!ground.relic_source_scion_id_field.empty()) put(legacy, "sourceScionId", ground.relic_source_scion_id_field);
    if (!ground.relic_source_scion_name_field.empty()) put(legacy, "sourceScionName", ground.relic_source_scion_name_field);
    put(out, "legacy", std::move(legacy));
  }
  return JsonValue(std::move(out));
}

// Tiny SHA-1 implementation solely for the RFC6455 challenge response.
std::array<std::uint8_t, 20> sha1(const std::string& input) {
  std::vector<std::uint8_t> message(input.begin(), input.end());
  const std::uint64_t bit_length = static_cast<std::uint64_t>(message.size()) * 8;
  message.push_back(0x80);
  while (message.size() % 64 != 56) message.push_back(0);
  for (int shift = 56; shift >= 0; shift -= 8) message.push_back(static_cast<std::uint8_t>((bit_length >> shift) & 0xff));
  std::uint32_t h0 = 0x67452301, h1 = 0xefcdab89, h2 = 0x98badcfe, h3 = 0x10325476, h4 = 0xc3d2e1f0;
  for (std::size_t offset = 0; offset < message.size(); offset += 64) {
    std::array<std::uint32_t, 80> w{};
    for (int i = 0; i < 16; ++i) w[i] = (message[offset + i*4] << 24) | (message[offset + i*4+1] << 16) | (message[offset + i*4+2] << 8) | message[offset + i*4+3];
    for (int i = 16; i < 80; ++i) { const auto v = w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16]; w[i] = (v << 1) | (v >> 31); }
    std::uint32_t a=h0,b=h1,c=h2,d=h3,e=h4;
    for (int i = 0; i < 80; ++i) {
      std::uint32_t f, k;
      if (i < 20) { f=(b&c)|((~b)&d); k=0x5a827999; }
      else if (i < 40) { f=b^c^d; k=0x6ed9eba1; }
      else if (i < 60) { f=(b&c)|(b&d)|(c&d); k=0x8f1bbcdc; }
      else { f=b^c^d; k=0xca62c1d6; }
      const auto temp = ((a << 5) | (a >> 27)) + f + e + k + w[i]; e=d; d=c; c=(b << 30) | (b >> 2); b=a; a=temp;
    }
    h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
  }
  std::array<std::uint8_t,20> digest{}; const std::uint32_t hs[] = {h0,h1,h2,h3,h4};
  for (int i=0;i<5;++i) for (int j=0;j<4;++j) digest[i*4+j] = static_cast<std::uint8_t>((hs[i] >> (24-j*8)) & 0xff);
  return digest;
}
std::string base64(const std::uint8_t* bytes, std::size_t size) {
  static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string result;
  for (std::size_t i=0;i<size;i+=3) { const std::uint32_t a=bytes[i], b=i+1<size?bytes[i+1]:0, c=i+2<size?bytes[i+2]:0; const auto n=(a<<16)|(b<<8)|c; result += alphabet[(n>>18)&63]; result += alphabet[(n>>12)&63]; result += i+1<size?alphabet[(n>>6)&63]:'='; result += i+2<size?alphabet[n&63]:'='; }
  return result;
}

std::string ws_accept_key(const std::string& key) {
  const auto digest = sha1(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
  return base64(digest.data(), digest.size());
}

}  // namespace

JsonValue::JsonValue() : value_(nullptr) {}
JsonValue::JsonValue(std::nullptr_t) : value_(nullptr) {}
JsonValue::JsonValue(bool value) : value_(value) {}
JsonValue::JsonValue(double value) : value_(value) {}
JsonValue::JsonValue(int value) : value_(static_cast<double>(value)) {}
JsonValue::JsonValue(std::string value) : value_(std::move(value)) {}
JsonValue::JsonValue(const char* value) : value_(std::string(value)) {}
JsonValue::JsonValue(Array value) : value_(std::move(value)) {}
JsonValue::JsonValue(Object value) : value_(std::move(value)) {}
bool JsonValue::is_null() const { return std::holds_alternative<std::nullptr_t>(value_); }
bool JsonValue::is_object() const { return std::holds_alternative<Object>(value_); }
bool JsonValue::is_array() const { return std::holds_alternative<Array>(value_); }
bool JsonValue::is_string() const { return std::holds_alternative<std::string>(value_); }
bool JsonValue::is_number() const { return std::holds_alternative<double>(value_); }
bool JsonValue::is_bool() const { return std::holds_alternative<bool>(value_); }
const JsonValue::Object* JsonValue::object() const { return std::get_if<Object>(&value_); }
JsonValue::Object* JsonValue::object() { return std::get_if<Object>(&value_); }
const JsonValue::Array* JsonValue::array() const { return std::get_if<Array>(&value_); }
JsonValue::Array* JsonValue::array() { return std::get_if<Array>(&value_); }
const std::string* JsonValue::string() const { return std::get_if<std::string>(&value_); }
std::optional<double> JsonValue::number() const { if (const auto* value=std::get_if<double>(&value_)) return *value; return std::nullopt; }
std::optional<bool> JsonValue::boolean() const { if (const auto* value=std::get_if<bool>(&value_)) return *value; return std::nullopt; }
const JsonValue* JsonValue::get(const std::string& key) const { const auto* values=object(); if (!values) return nullptr; const auto it=values->find(key); return it==values->end()?nullptr:&it->second; }
JsonValue* JsonValue::get(const std::string& key) { auto* values=object(); if (!values) return nullptr; const auto it=values->find(key); return it==values->end()?nullptr:&it->second; }
const JsonValue& JsonValue::operator[](const std::string& key) const { static const JsonValue null; const auto* value=get(key); return value?*value:null; }
std::string JsonValue::stringify() const {
  if (is_null()) return "null";
  if (is_bool()) return *boolean() ? "true" : "false";
  if (is_number()) { std::ostringstream out; out << std::setprecision(15) << *number(); return out.str(); }
  if (is_string()) return "\"" + json_escape(*string()) + "\"";
  if (is_array()) { std::string out="["; bool first=true; for (const auto& value:*array()) { if (!first) out+=','; first=false; out+=value.stringify(); } return out+"]"; }
  std::string out="{"; bool first=true; for (const auto& [key,value]:*object()) { if (!first) out+=','; first=false; out+="\""+json_escape(key)+"\":"+value.stringify(); } return out+"}";
}

bool parse_json(const std::string& text, JsonValue& out, std::string* error) { return JsonParser(text).parse(out, error); }
bool parse_envelope(const std::string& text, Envelope& out, std::string* error) {
  JsonValue root; if (!parse_json(text, root, error) || !root.is_object()) { if (error && error->empty()) *error="envelope must be an object"; return false; }
  const auto* event=root.get("event"); if (!event || !event->string() || event->string()->empty()) { if (error) *error="envelope event must be a non-empty string"; return false; }
  const auto* data=root.get("data"); if (!data || !data->is_object()) { if (error) *error="envelope data must be an object"; return false; }
  out.event=*event->string(); out.data=*data; out.meta.reset(); if (const auto* meta=root.get("meta")) out.meta=*meta; return true;
}
std::string emit_envelope(const Envelope& envelope) {
  JsonValue::Object root; put(root,"event",envelope.event); put(root,"data",envelope.data); if (envelope.meta) put(root,"meta",*envelope.meta); return JsonValue(std::move(root)).stringify();
}

ProtocolSession::ProtocolSession(std::string identity, std::string socket_id, std::uint64_t seed, bool quick_start)
    : identity_(std::move(identity)), socket_id_(std::move(socket_id)), quick_start_(quick_start),
      session_rng_(static_cast<std::uint32_t>(seed ^ (seed >> 32))),
      simulation_(std::make_unique<Simulation>(seed, "House Verdigris")), world_(std::make_shared<WorldSimulation>(seed, identity_)) {
  // Fresh-scion admission (server/core treasuries/fresh profile): the purse
  // is the only starting inventory; the legacy starter blade is retired
  // vocabulary and intentionally absent (see the N4 report).
  CreateItemOptions purse;
  purse.quantity = 100;
  auto coins = create_game_item("coins", purse);
  if (coins) inventory_.add(std::move(*coins));
  sync_combat_mods();
}
void ProtocolSession::replace_socket(std::string socket_id) { std::lock_guard<std::recursive_mutex> lock(mutex_); socket_id_=std::move(socket_id); }

void ProtocolSession::set_direct_emit(std::function<void(const Envelope&)> emit) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  direct_emit_ = std::move(emit);
}

void ProtocolSession::tick(std::int64_t now) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  if (!direct_emit_) return;
  maybe_respawn(now);
  if (world_->in_instance()) process_combat(now, direct_emit_);
}

void ProtocolSession::reset_world_for_new_socket() {
  // JS parity: a NEW socket gets a fresh Player position (town) while the
  // account state (stats, inventory, quests, chronicle) persists - guests
  // "survive relogins" via the saved snapshot on JS. The commission chain
  // resets only on scion admission (player:chronicles:select), never here.
  // Same-socket re-logins never pass through here, so hot dev re-logins
  // keep the instance (networking_tests: instance re-login snapshot).
  //
  // JS builds a brand-new Player object per login and merges ONLY the saved
  // snapshot fields (loot, levels, bank, skill tree, quest record) over the
  // template. Everything transient must therefore reset here, or state from
  // one closed session bleeds into the account's next login (a dead
  // lifecycle blocked respawn.mjs; a leftover Chronicle draft broke
  // mortality.mjs's seeded revision).
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  tree_quest_points_ = 0;  // JS: a rebuilt Player starts with questPoints 0
  if (lifecycle_ != "permadead") {
    // A soft death clears on re-login (fresh Player), but a mortal Scion's
    // final death is Chronicle history - reconnecting must NOT resurrect
    // them; only selecting an heir through chronicles admission does.
    lifecycle_ = "alive";
    lifecycle_deaths_ = 0;
    // JS parity: beginScionSession rebuilds mode:'hard' whenever an admitted
    // scion resumes, so a sworn oath survives relogins; a socket with no
    // admitted scion keeps the legacy soft-guest profile.
    const bool sworn = active_scion_id_.empty()
        ? false
        : scion_record_mortal(chronicle_, active_house_id_, active_scion_id_);
    mortal_oath_ = sworn;
    lifecycle_mode_ = sworn ? "hard" : "soft";
  }
  respawn_at_ms_ = 0;
  respawn_protection_until_ms_ = 0;
  prepare_final_death_ = false;
  first_goal_stage_ = "available";
  first_goal_started_ms_ = 0;
  first_goal_completed_ms_ = 0;
  shop_open_ = false;
  bank_open_ = false;
  active_skill_id_ = "primary-attack";
  combat_clock_ms_ = 0;
  resource_regen_at_ms_ = 0;
  war_cry_until_ms_ = 0;
  war_cry_attack_bonus_ = 0;
  pending_chronicles_ = false;
  current_node_id_.clear();
  current_child_id_.clear();
  endgame_active_ = false;
  endgame_completed_ = false;
  endgame_map_tier_ = 0;
  endgame_goods_found_percent_ = 0;
  endgame_map_name_.clear();
  endgame_map_modifiers_.clear();
  if (auto* actor = simulation_->actor(simulation_->scion().actor_id)) {
    actor->stats.life = actor->stats.life_max;  // fresh Player logs in healthy
  }
  world_->reset_to_town();
}
void ProtocolSession::set_broadcast(std::function<void(const Envelope&)> broadcast) { std::lock_guard<std::recursive_mutex> lock(mutex_); broadcast_=std::move(broadcast); }
std::int64_t ProtocolSession::now_ms() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
std::string ProtocolSession::player_payload() const {
  JsonValue::Object player; const auto position=world_->position();
  put(player,"uuid",identity_); put(player,"username",!username_.empty()?username_:(active_scion_name_.empty()?identity_:active_scion_name_)); put(player,"socket_id",socket_id_); put(player,"sceneId",world_->scene_id()); put(player,"x",position.x); put(player,"y",position.y); put(player,"facing",world_->facing());
  { const auto* actor=simulation_->actor(simulation_->scion().actor_id);
    put(player,"level",actor?actor->stats.level:1);
    put(player,"life",actor?actor->stats.life:0);
    put(player,"lifeMax",actor?actor->stats.life_max:0);
    put(player,"resource",actor?actor->stats.resource:0);
    put(player,"resourceMax",actor?actor->stats.resource_max:0); }
  put(player,"passiveTree",passive_tree_json());
  put(player,"quests",quests_json());
  JsonValue::Array slots;
  for (const auto& item : inventory_.items()) slots.emplace_back(item_identity_json(item));
  JsonValue::Object inventory; put(inventory,"slots",std::move(slots)); put(player,"inventory",std::move(inventory));
  JsonValue::Object chronicles; put(chronicles,"mortal",mortal_oath_); put(chronicles,"scionId",active_scion_id_.empty()?JsonValue(nullptr):JsonValue(active_scion_id_)); put(chronicles,"houseId",active_house_id_.empty()?JsonValue(nullptr):JsonValue(active_house_id_)); put(player,"chronicles",std::move(chronicles));
  return JsonValue(std::move(player)).stringify();
}
std::string ProtocolSession::login_payload() const {
  std::lock_guard<std::recursive_mutex> lock(mutex_); JsonValue::Object data; JsonValue player; parse_json(player_payload(),player); put(data,"player",std::move(player)); put(data,"scene",scene_payload()); put(data,"droppedItems",dropped_items_json()); if (quick_start_) put(data,"quickStart",true); return JsonValue(std::move(data)).stringify();
}
JsonValue ProtocolSession::dropped_items_json() const {
  JsonValue::Array ground;
  for (const auto& entry : world_->ground_items()) ground.emplace_back(ground_item_json(entry));
  return JsonValue(std::move(ground));
}
JsonValue ProtocolSession::wear_json() const {
  JsonValue::Object wear;
  for (const auto& seat : WearSet::physical_slots()) {
    const GameItem* worn = wear_.in_seat(seat);
    if (worn) put(wear, seat, worn->id);
    else put(wear, seat, nullptr);
  }
  return JsonValue(std::move(wear));
}
JsonValue ProtocolSession::wear_details_json() const {
  JsonValue::Object wear_details;
  for (const auto& seat : WearSet::physical_slots()) {
    const GameItem* worn = wear_.in_seat(seat);
    if (worn) put(wear_details, seat, item_identity_json(*worn));
    else put(wear_details, seat, nullptr);
  }
  return JsonValue(std::move(wear_details));
}
namespace {
// server/shared/quests.js QUEST_DEFINITIONS - ordered objective chain.
struct QuestObjective {
  const char* trigger;
  const char* text;
  const char* crit_a;
  const char* crit_b;
  int min_depth;
};
struct QuestDef {
  const char* id;
  const char* title;
  const char* giver;
  const char* summary;
  const char* deed;
  const char* reward;
  int renown;
  int objective_count;
  QuestObjective objectives[5];
};
const QuestDef kQuestChain[] = {
    {"aldwyns-charge", "Aldwyn's Charge", "Aldwyn the Guide",
     "Learn the road rites every Verdigris Scion must survive.",
     "Answered Aldwyn's Charge", "+1 quest point / +5 House renown", 5, 5, {
        {"move", "Take your first steps beyond the wagon circle.", "", "", 0},
        {"attack", "Strike with your equipped weapon.", "", "", 0},
        {"slay", "Slay a creature of the old realms.", "", "", 0},
        {"loot", "Claim an item from the fallen.", "", "", 0},
        {"delve", "Enter your first expedition.", "", "", 0}}},
    {"proof-of-temper", "Proof of Temper", "Ludovicus",
     "Forge a weapon worthy of a named Scion.",
     "Proved their temper in the old realms", "+1 quest point / +10 House renown", 10, 3, {
        {"slay-elite", "Bring down an elite foe.", "", "", 0},
        {"loot-vessel", "Recover the vessel it guarded.", "", "", 0},
        {"equip-vessel", "Equip the vessel and bind it to your hand.", "", "", 0}, {}, {}}},
    {"the-pale-crown", "The Pale Crown", "Selene of the Rite",
     "Break the sovereign seal beneath the Weir Crypt.",
     "Broke the Pale Sovereign's seal", "+1 quest point / +15 House renown", 15, 3, {
        {"delve", "Enter the Weir Crypt.", "weir-crypt", "", 1},
        {"slay-elite", "Defeat the Pale Sovereign.", "The Pale Sovereign", "crypt", 0},
        {"delve", "Descend to the crypt's second floor.", "", "crypt", 2}, {}, {}}},
    {"rot-in-the-reeds", "Rot in the Reeds", "Aldwyn the Guide",
     "Cut the marsh blight out at its buried root.",
     "Ended the rot beneath the reeds", "+1 quest point / +20 House renown", 20, 3, {
        {"delve", "Enter the Marsh of Reeds.", "marsh-of-reeds", "", 1},
        {"slay-elite", "Defeat the Rotfather.", "The Rotfather", "marsh", 0},
        {"return-surface", "Return alive to the surface.", "marsh-of-reeds", "", 0}, {}, {}}},
};
const int kQuestChainSize = 4;
std::string zone_id_for_instance(const std::string& theme, const std::string& layout) {
  // party.js ADVENTURE_ZONES: identity is theme+layout, not theme alone.
  for (const auto& zone : adventure_zones()) {
    if (zone.template_id == theme && zone.layout == layout) return zone.id;
  }
  for (const auto& zone : adventure_zones()) {
    if (zone.template_id == theme) return zone.id;
  }
  return "old-barrow";
}
// server/core/world-web.js - deterministic per-house road chart. The hash
// need not match JS bit-for-bit: node identity only has to be stable and
// self-consistent within this server (house ids are per-session anyway).
struct RoadDef { const char* id; const char* name; const char* direction; const char* blurb; const char* pairs[4][2]; };
const RoadDef kRoads[4] = {
    {"tin", "The Tin Road", "north", "North into the old quarry country.",
     {{"dungeon", "warren"}, {"dungeon", "gauntlet"}, {"wilds", "clearings"}, {"dungeon", "clearings"}}},
    {"salt", "The Salt Road", "east", "East through the fens.",
     {{"marsh", "clearings"}, {"grove", "clearings"}, {"marsh", "gauntlet"}, {"grove", "warren"}}},
    {"chalk", "The Chalk Road", "south", "South over the downs and their graves.",
     {{"crypt", "warren"}, {"crypt", "gauntlet"}, {"wilds", "clearings"}, {"crypt", "clearings"}}},
    {"copper", "The Copper Road", "west", "West into the burnt hills.",
     {{"crypt", "warren"}, {"wilds", "clearings"}, {"crypt", "gauntlet"}, {"wilds", "warren"}}},
};
const char* kRoadFirsts[4][6] = {
    {"Hoar", "Grey", "Whet", "Stone", "Cold", "Scree"},
    {"Eel", "Sedge", "Rush", "Weir", "Mere", "Fen"},
    {"Barrow", "Chalk", "Bone", "Lych", "Grave", "Dust"},
    {"Ash", "Cinder", "Ember", "Slag", "Copper", "Forge"},
};
const char* kRoadSeconds[4][6] = {
    {"fell", "moor", "delf", "gate", "cleft", "howe"},
    {"fen", "mere", "carr", "weir", "holm", "hythe"},
    {"down", "barrow", "field", "kirk", "vault", "howe"},
    {"hill", "works", "kiln", "heath", "brink", "reach"},
};
int road_index(const std::string& road_id) {
  for (int i = 0; i < 4; ++i) if (road_id == kRoads[i].id) return i;
  return -1;
}
std::uint32_t web_hash(const std::string& text) {
  std::uint32_t h = 2166136261u;
  for (unsigned char c : text) { h ^= c; h *= 16777619u; }
  return h;
}
struct RoadNode {
  std::string id, name, template_id, layout, parent_id, warden_name;
  int tier = 1, index = 0;
  std::vector<std::string> child_ids;
};
int web_tier_width(const std::string& house, const std::string& road, int tier) {
  if (tier <= 1) return 1;
  const int previous = web_tier_width(house, road, tier - 1);
  const int step_pick = static_cast<int>(web_hash(house + "|" + road + "|" + std::to_string(tier) + "|width") % 4);
  const int step = step_pick == 0 ? -1 : (step_pick == 3 ? 1 : 0);
  return (std::max)(1, (std::min)(3, previous + step));
}
std::vector<RoadNode> web_road_nodes(const std::string& house, const std::string& road_id, int max_tier) {
  std::vector<RoadNode> nodes;
  const int ri = road_index(road_id);
  if (ri < 0) return nodes;
  std::vector<int> previous_tier;  // indices into nodes
  std::set<std::string> used;
  for (int tier = 1; tier <= max_tier; ++tier) {
    const int width = web_tier_width(house, road_id, tier);
    std::vector<int> current_tier;
    for (int index = 0; index < width; ++index) {
      const std::uint32_t h = web_hash(house + "|" + road_id + "|" + std::to_string(tier) + "|" + std::to_string(index));
      RoadNode node;
      node.id = road_id + ":" + std::to_string(tier) + ":" + std::to_string(index);
      node.tier = tier;
      node.index = index;
      const auto& pair = kRoads[ri].pairs[h % 4];
      node.template_id = pair[0];
      node.layout = pair[1];
      std::string name = std::string(kRoadFirsts[ri][(h >> 4) % 6]) + kRoadSeconds[ri][(h >> 8) % 6];
      while (used.count(name)) name += " Deep";
      used.insert(name);
      node.name = name;
      node.warden_name = "Warden of " + name;
      if (!previous_tier.empty()) {
        const int parent_pick = (std::min)(static_cast<int>(previous_tier.size()) - 1,
                                           (index * static_cast<int>(previous_tier.size())) / width);
        node.parent_id = nodes[previous_tier[static_cast<std::size_t>(parent_pick)]].id;
      }
      current_tier.push_back(static_cast<int>(nodes.size()));
      nodes.push_back(std::move(node));
    }
    for (int node_index : current_tier) {
      if (!nodes[node_index].parent_id.empty()) {
        for (auto& candidate : nodes) {
          if (candidate.id == nodes[node_index].parent_id) { candidate.child_ids.push_back(nodes[node_index].id); break; }
        }
      }
    }
    previous_tier = current_tier;
  }
  return nodes;
}
bool parse_node_id(const std::string& id, std::string* road, int* tier, int* index) {
  const auto first = id.find(':');
  const auto second = id.find(':', first == std::string::npos ? first : first + 1);
  if (first == std::string::npos || second == std::string::npos) return false;
  *road = id.substr(0, first);
  if (road_index(*road) < 0) return false;
  try {
    *tier = std::stoi(id.substr(first + 1, second - first - 1));
    *index = std::stoi(id.substr(second + 1));
  } catch (...) { return false; }
  return *tier >= 1 && *index >= 0;
}
// chronicles repository parity: relic circulation is WORLD state - a fallen
// scion gear can surface for any survivor (party-stories), preferring the
// fallen own account (mortality/chronicles).
struct CirculatingRelic { GameItem item; std::string scion_id; std::string scion_name; std::string account; };
std::vector<CirculatingRelic>& circulation_pool() { static std::vector<CirculatingRelic> pool; return pool; }
std::mutex& circulation_mutex() { static std::mutex m; return m; }
struct RoadGateTile { int x; int y; const char* road; };
const RoadGateTile kRoadGates[4] = {{37, 94, "tin"}, {64, 114, "salt"}, {37, 138, "chalk"}, {12, 115, "copper"}};

// world-layout.js WAGON_PITCHES - the plaza ring.
const int kWagonPitches[8][2] = {{47,112},{42,109},{34,109},{29,112},{29,118},{34,121},{42,121},{47,118}};
}  // namespace
namespace {
struct TownNpc {
  int id;
  const char* key;
  const char* name;
  const char* role;
  const char* examine;
  int x;
  int y;
  const char* services[2];
  int service_count;
  const char* actions[2];
  int action_count;
};
// content/seeds/owner_demo_town.json - the accepted Crossroads owner roster.
const TownNpc kTownNpcs[] = {
    {1, "aldwyn-guide", "Aldwyn the Guide", "elder",
     "A weathered wayfinder watching the ash banners gather beyond Thornward.",
     34, 116, {"guidance", "expedition_access"}, 2, {"talk", "examine"}, 2},
    {2, "ludovicus-weapons", "Ludovicus, Weapons Trader",
     "weapons_tools_trainer",
     "Road iron, whetstones, and hard lessons hang from Ludovicus' boards.",
     19, 113, {"shop", ""}, 1, {"trade", "examine"}, 2},
    {3, "selene-rite", "Selene of the Rite", "armor_ritual_merchant",
     "Wax-sealed armor and ritual fittings line Selene's quiet vault.",
     45, 108, {"shop", ""}, 1, {"trade", "examine"}, 2},
    {4, "rhea-countinghouse", "Rhea of the Countinghouse", "steward",
     "The House ledgers, shared stores, and first investments pass through Rhea's hands.",
     31, 121, {"storage", "house_investment"}, 2, {"bank", "examine"}, 2},
};

const TownNpc* town_npc(int id) {
  for (const auto& npc : kTownNpcs)
    if (npc.id == id) return &npc;
  return nullptr;
}
}  // namespace

JsonValue ProtocolSession::combat_totals_json() const {
  const auto totals = wear_.totals();
  JsonValue::Object combat;
  put(combat, "attack", ratings_json(totals.attack));
  put(combat, "defense", ratings_json(totals.defense));
  put(combat, "blockChance", totals.modifiers.block_chance);
  put(combat, "criticalChance", totals.modifiers.critical_chance);
  put(combat, "goodsFound", totals.modifiers.goods_found);
  put(combat, "damageAgainstBeasts", totals.modifiers.damage_against_beasts);
  put(combat, "respawnProtectionUntil", static_cast<double>(respawn_protection_until_ms_));
  return JsonValue(std::move(combat));
}
JsonValue ProtocolSession::scene_payload() const {
  JsonValue::Object scene; put(scene,"id",world_->scene_id()); put(scene,"type",world_->scene_type()); put(scene,"name",world_->scene_name());
  put(scene,"droppedItems",dropped_items_json());
  if (world_->in_instance()) { const auto& meta=world_->metadata(); JsonValue::Object metadata; put(metadata,"seed",static_cast<double>(meta.seed)); put(metadata,"theme",meta.theme); if(meta.layout.empty()) put(metadata,"layout",nullptr); else put(metadata,"layout",meta.layout); put(metadata,"depth",meta.depth);
    JsonValue::Object up; put(up,"x",meta.stairs_up.x); put(up,"y",meta.stairs_up.y); put(metadata,"stairsUp",std::move(up));
    JsonValue::Object down; put(down,"x",meta.stairs_down.x); put(down,"y",meta.stairs_down.y); put(metadata,"stairsDown",std::move(down));
    JsonValue::Array spawns; for (const auto& spawn:meta.spawn_points) { JsonValue::Object value; put(value,"x",spawn.x); put(value,"y",spawn.y); spawns.emplace_back(std::move(value)); } put(metadata,"spawnPoints",std::move(spawns));
    if (!current_node_id_.empty()) {
      put(metadata, "nodeId", current_node_id_);
      put(metadata, "tier", current_node_tier_);
      JsonValue::Object entry_gate; put(entry_gate, "x", meta.stairs_up.x); put(entry_gate, "y", meta.stairs_up.y);
      put(metadata, "entryGate", std::move(entry_gate));
      JsonValue::Array zone_gates;
      if (!current_child_id_.empty()) {
        JsonValue::Object gate; put(gate, "x", meta.stairs_down.x); put(gate, "y", meta.stairs_down.y);
        put(gate, "nodeId", current_child_id_); put(gate, "name", current_child_name_);
        zone_gates.emplace_back(std::move(gate));
      }
      put(metadata, "zoneGates", std::move(zone_gates));
      put(metadata, "wardenDead", cleared_nodes_.count(current_node_id_) > 0);
    }
    put(scene,"metadata",std::move(metadata)); }
  return JsonValue(std::move(scene));
}
JsonValue ProtocolSession::movement_step_payload() const {
  const auto& step=world_->last_step(); JsonValue::Object value; put(value,"sequence",static_cast<double>(step.sequence)); put(value,"startedAt",static_cast<double>(step.started_at_ms)); put(value,"duration",step.duration_ms); if(step.direction.empty()) put(value,"direction",nullptr); else put(value,"direction",step.direction); put(value,"blocked",step.blocked); return JsonValue(std::move(value));
}
namespace {
long long xp_for_level(int level);
int level_from_xp(long long exp);
}  // namespace

JsonValue ProtocolSession::snapshot() const {
  JsonValue::Object state; const auto& scion=simulation_->scion(); const auto* actor=simulation_->actor(scion.actor_id); const auto position=world_->position();
  put(state,"uuid",identity_); put(state,"x",position.x); put(state,"y",position.y); put(state,"sceneId",world_->scene_id()); put(state,"sceneType",world_->scene_type()); put(state,"sceneName",world_->scene_name());
  put(state,"lifecycle",lifecycle_);
  put(state,"lifecycleMode",lifecycle_mode_);
  put(state,"theme",world_->in_instance()?world_->metadata().theme:std::string("town"));
  {
    // The client renders only this authoritative current-level span; it never
    // reimplements the experience curve or guesses progress from player level.
    const int xp_level = level_from_xp(combat_xp_);
    JsonValue::Object xp;
    put(xp, "current", static_cast<double>(combat_xp_));
    put(xp, "level", xp_level);
    put(xp, "floor", static_cast<double>(xp_for_level(xp_level)));
    put(xp, "next", static_cast<double>(xp_for_level(xp_level + 1)));
    put(state, "xp", std::move(xp));
  }
  {
    JsonValue::Array modifiers;
    for (const auto& modifier : endgame_map_modifiers_)
      modifiers.emplace_back(modifier);
    JsonValue::Object endgame;
    put(endgame, "unlocked", campaign_complete_);
    put(endgame, "active", endgame_active_);
    put(endgame, "cleared", endgame_completed_);
    put(endgame, "completed", endgame_maps_completed_);
    if (endgame_active_) {
      put(endgame, "name", endgame_map_name_);
      put(endgame, "tier", endgame_map_tier_);
      put(endgame, "goodsFoundPercent", endgame_goods_found_percent_);
      put(endgame, "modifiers", std::move(modifiers));
    } else {
      put(endgame, "name", nullptr);
      put(endgame, "tier", nullptr);
      put(endgame, "goodsFoundPercent", nullptr);
      put(endgame, "modifiers", JsonValue::Array{});
    }
    put(state, "endgame", std::move(endgame));
  }
  JsonValue::Object chronicles; put(chronicles,"mortal",mortal_oath_); put(chronicles,"scionId",active_scion_id_.empty()?JsonValue(nullptr):JsonValue(active_scion_id_)); put(chronicles,"houseId",active_house_id_.empty()?JsonValue(nullptr):JsonValue(active_house_id_)); put(state,"chronicles",std::move(chronicles));
  put(state,"bestDepth",best_depth_);
  put(state,"quests",quests_json());
  put(state,"questPoints",tree_quest_points_);
  put(state,"bank",bank_items_json());
  put(state,"passiveTree",passive_tree_json());
  {
    JsonValue::Object investment;
    put(investment, "firstClearCompleted", house_progression_.first_clear_completed);
    put(investment, "eligible", first_clear_eligible(house_progression_));
    put(investment, "choice", choice_name(house_progression_.choice));
    put(investment, "rewardClaimed", house_progression_.reward_claimed);
    put(investment, "scionGearTier", house_progression_.scion_gear_tier);
    // The underlying accepted model calls this per-tick income. Production
    // awards it once per cleared floor so a 20 Hz simulation tick cannot
    // inflate the House economy.
    put(investment, "houseIncomePerClear",
        house_progression_.house_income_per_tick);
    put(state, "houseInvestment", std::move(investment));
  }
  { // stats-manager attributes: base 10s plus the tree path. STUB NOTE:
    // per-node attribute identity from the 271-node graph is approximated
    // as +2/attr per allocated node beyond the root until the geometric
    // tree engine is ported (successor task).
    int str_attr = 10, dex_attr = 10, int_attr = 10;
    tree_attributes(&str_attr, &dex_attr, &int_attr);
    JsonValue::Object attributes;
    put(attributes, "strength", str_attr);
    put(attributes, "dexterity", dex_attr);
    put(attributes, "intelligence", int_attr);
    put(state, "attributes", std::move(attributes));
  }
  JsonValue::Array npcs;
  if (!world_->in_instance()) {
    for (const auto& npc : kTownNpcs) {
      JsonValue::Object entry;
      put(entry, "id", npc.id); put(entry, "key", npc.key);
      put(entry, "name", npc.name); put(entry, "role", npc.role);
      put(entry, "examine", npc.examine);
      put(entry, "x", npc.x); put(entry, "y", npc.y);
      put(entry, "tileX", npc.x); put(entry, "tileY", npc.y);
      JsonValue::Array services;
      for (int i = 0; i < npc.service_count; ++i)
        services.emplace_back(npc.services[i]);
      put(entry, "services", std::move(services));
      JsonValue::Array actions;
      for (int i = 0; i < npc.action_count; ++i) actions.emplace_back(npc.actions[i]);
      put(entry, "actions", std::move(actions));
      npcs.emplace_back(std::move(entry));
    }
  }
  put(state,"npcs",std::move(npcs));
  { // dev.js: chroniclesRecord mirrors chroniclesStore.snapshot(uuid).
    JsonValue::Object record;
    put(record,"exists",chronicles_revision_>0);
    put(record,"revision",chronicles_revision_);
    put(record,"state",chronicle_);
    put(state,"chroniclesRecord",std::move(record));
  }
  JsonValue::Object lifecycle_details; put(lifecycle_details,"deaths",lifecycle_deaths_); put(lifecycle_details,"respawn",JsonValue::Object{{"at",static_cast<double>(respawn_at_ms_)}}); put(state,"lifecycleDetails",std::move(lifecycle_details));
  JsonValue::Object hp; put(hp,"current",actor?actor->stats.life:0); put(hp,"max",actor?actor->stats.life_max:0); put(state,"hp",std::move(hp));
  JsonValue::Object resource;
  put(resource,"current",actor?actor->stats.resource:0);
  put(resource,"max",actor?actor->stats.resource_max:0);
  put(state,"resource",std::move(resource));
  const int cooldown_ms = world_->player_cooldown_remaining_ms(combat_clock_ms_);
  put(state,"cooldownTicks",(cooldown_ms+kSimulationTickMs-1)/kSimulationTickMs);
  put(state,"warCryTicksRemaining",war_cry_until_ms_>combat_clock_ms_
      ? static_cast<int>((war_cry_until_ms_-combat_clock_ms_+kSimulationTickMs-1)/kSimulationTickMs)
      : 0);
  JsonValue::Array monsters; for (const auto& candidate:world_->monsters()) if (candidate.alive) {
    JsonValue::Object monster; put(monster,"uuid",candidate.uuid); put(monster,"id",candidate.id); put(monster,"name",candidate.name);
    put(monster,"x",candidate.x); put(monster,"y",candidate.y); put(monster,"level",candidate.level); put(monster,"rarity",candidate.rarity);
    JsonValue::Array tags; for (const auto& tag:candidate.tags) tags.emplace_back(tag); put(monster,"tags",std::move(tags));
    put(monster,"coins",candidate.coins);
    JsonValue::Object behaviour; put(behaviour,"type",candidate.behaviour_type); put(monster,"behaviour",std::move(behaviour));
    JsonValue::Object mhp; put(mhp,"current",candidate.life); put(mhp,"max",candidate.life_max); put(monster,"hp",std::move(mhp));
    JsonValue::Array modifiers; for (const auto& modifier:candidate.modifiers) { JsonValue::Object value; put(value,"id",modifier); put(value,"label",modifier=="empowered"?"Empowered":modifier); modifiers.emplace_back(std::move(value)); } put(monster,"modifiers",std::move(modifiers));
    JsonValue::Object effects; if (candidate.empowered) { JsonValue::Object effect; put(effect,"label","Empowered"); put(effect,"id","aura:damage"); put(effects,"aura",std::move(effect)); } put(monster,"state",JsonValue::Object{{"effects",std::move(effects)}});
    monsters.emplace_back(std::move(monster));
  } put(state,"monsters",std::move(monsters));
  if (world_->in_instance()) { const auto& meta=world_->metadata(); JsonValue::Object metadata; put(metadata,"seed",static_cast<double>(meta.seed)); put(metadata,"theme",meta.theme); if(meta.layout.empty()) put(metadata,"layout",nullptr); else put(metadata,"layout",meta.layout); put(metadata,"depth",meta.depth);
    JsonValue::Object up; put(up,"x",meta.stairs_up.x); put(up,"y",meta.stairs_up.y); put(metadata,"stairsUp",std::move(up));
    JsonValue::Object down; put(down,"x",meta.stairs_down.x); put(down,"y",meta.stairs_down.y); put(metadata,"stairsDown",std::move(down));
    JsonValue::Array spawns; for (const auto& spawn:meta.spawn_points) { JsonValue::Object value; put(value,"x",spawn.x); put(value,"y",spawn.y); spawns.emplace_back(std::move(value)); } put(metadata,"spawnPoints",std::move(spawns));
    if (!current_node_id_.empty()) {
      put(metadata, "nodeId", current_node_id_);
      put(metadata, "tier", current_node_tier_);
      JsonValue::Object entry_gate; put(entry_gate, "x", meta.stairs_up.x); put(entry_gate, "y", meta.stairs_up.y);
      put(metadata, "entryGate", std::move(entry_gate));
      JsonValue::Array zone_gates;
      if (!current_child_id_.empty()) {
        JsonValue::Object gate; put(gate, "x", meta.stairs_down.x); put(gate, "y", meta.stairs_down.y);
        put(gate, "nodeId", current_child_id_); put(gate, "name", current_child_name_);
        zone_gates.emplace_back(std::move(gate));
      }
      put(metadata, "zoneGates", std::move(zone_gates));
      put(metadata, "wardenDead", cleared_nodes_.count(current_node_id_) > 0);
    }
    put(state,"sceneMetadata",std::move(metadata)); }
  else {
    JsonValue::Object town_meta;
    JsonValue::Array pitches;
    for (const auto& pitch : kWagonPitches) {
      JsonValue::Object entry; put(entry, "x", pitch[0]); put(entry, "y", pitch[1]);
      pitches.emplace_back(std::move(entry));
    }
    put(town_meta, "wagonPitches", std::move(pitches));
    put(state,"sceneMetadata",std::move(town_meta));
  }
  // N4: the real item pipeline snapshot (dev.js buildStateSnapshot).
  put(state,"level",actor?actor->stats.level:1);
  JsonValue::Array items; for (const auto& item:inventory_.items()) items.emplace_back(snapshot_item_json(item)); put(state,"inventory",std::move(items));
  JsonValue::Array details; for (const auto& item:inventory_.items()) details.emplace_back(item_identity_json(item)); put(state,"inventoryDetails",std::move(details));
  put(state,"wear",wear_json()); put(state,"wearDetails",wear_details_json());
  { // dev.js wornItems: per-seat worn item identity (uuid-bearing).
    JsonValue::Object worn;
    for (const auto& [seat, item] : wear_.slots()) {
      JsonValue::Object entry;
      put(entry, "uuid", item.uuid); put(entry, "id", item.id); put(entry, "name", item.name);
      put(worn, seat, std::move(entry));
    }
    put(state, "wornItems", std::move(worn));
  }
  put(state,"combat",combat_totals_json());
  put(state,"groundItems",dropped_items_json());
  put(state,"droppedItems",dropped_items_json());
  JsonValue::Array stored; for (const auto& item:house_store_) stored.emplace_back(item_identity_json(item)); put(state,"houseStoredItems",std::move(stored));
  put(state,"groundTrophies",JsonValue::Array{}); return JsonValue(std::move(state));
}
std::string ProtocolSession::state_payload(const std::string& request_id, bool include_map) const {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  JsonValue::Object data;
  put(data,"player",JsonValue::Object{{"socket_id",socket_id_}});
  JsonValue state_value = snapshot();
  if (include_map) {
    // The walkable grid the world already resolves movement against. Sent
    // only on request (the client asks once per scene) so the 4 Hz snapshot
    // stays light; without it walls are invisible and read as ghost
    // collisions on the client.
    const auto& grid = world_->grid();
    JsonValue::Object map;
    put(map, "sceneId", world_->scene_id());
    put(map, "width", grid.width);
    put(map, "height", grid.height);
    JsonValue::Array rows;
    for (int y = 0; y < grid.height; ++y) {
      std::string row(static_cast<std::size_t>(grid.width), '1');
      for (int x = 0; x < grid.width; ++x)
        if (!grid.walkable_at(x, y)) row[static_cast<std::size_t>(x)] = '0';
      rows.emplace_back(std::move(row));
    }
    put(map, "rows", std::move(rows));
    if (auto* state_object = state_value.object())
      (*state_object)["map"] = JsonValue(std::move(map));
  }
  put(data,"state",std::move(state_value));
  put(data,"requestId",request_id);
  return JsonValue(std::move(data)).stringify();
}
void ProtocolSession::emit_inventory_refresh(const std::function<void(const Envelope&)>& emit) const {
  // dev.js: core:refresh:inventory carries the full slot list.
  JsonValue::Array slots; for (const auto& item:inventory_.items()) slots.emplace_back(item_identity_json(item));
  emit(Envelope{"core:refresh:inventory",JsonValue::Object{{"player",JsonValue::Object{{"socket_id",socket_id_}}},{"data",std::move(slots)}}});
}
void ProtocolSession::emit_ground_change(const std::function<void(const Envelope&)>& emit) const {
  // JS server/player/handlers/actions/index.js broadcasts scene.items as both
  // world:itemDropped and item:change. Native parse_envelope requires object
  // `data`, so wrap the array at data.data like core:refresh:inventory.
  const JsonValue items = dropped_items_json();
  JsonValue::Object body; put(body, "data", items);
  emit_world(Envelope{"world:itemDropped", JsonValue(body)}, emit);
  emit_world(Envelope{"item:change", JsonValue(std::move(body))}, emit);
}
void ProtocolSession::emit_equip_state(const std::function<void(const Envelope&)>& emit) const {
  // JS player:equippedAnItem carries the public projection (wear). Native
  // adds wearDetails + combat totals so the client can show the derived line
  // without waiting for the next dev:state.
  JsonValue::Object data;
  put(data, "uuid", identity_);
  put(data, "wear", wear_json());
  put(data, "wearDetails", wear_details_json());
  put(data, "combat", combat_totals_json());
  emit(Envelope{"player:equippedAnItem", JsonValue(std::move(data))});
}
void ProtocolSession::finish_extraction(const std::function<void(const Envelope&)>& emit) {
  // Drain backpack + wear into the House store. JS has no player:extract;
  // the response envelope reuses that name (see REPORT).
  int banked_items = 0;
  int retained_maps = 0;
  std::vector<std::string> uuids;
  uuids.reserve(inventory_.items().size());
  for (const auto& item : inventory_.items()) {
    if (item.expedition_map) {
      ++retained_maps;
      continue;
    }
    uuids.push_back(item.uuid);
  }
  for (const auto& uuid : uuids) {
    GameItem item;
    if (inventory_.remove_by_uuid(uuid, &item)) {
      house_store_.push_back(std::move(item));
      ++banked_items;
    }
  }
  for (const auto& seat : WearSet::physical_slots()) {
    auto worn = wear_.unequip(seat);
    if (worn) {
      house_store_.push_back(std::move(*worn));
      ++banked_items;
    }
  }
  sync_combat_mods();
  JsonValue::Array stored;
  for (const auto& item : house_store_) stored.emplace_back(item_identity_json(item));
  JsonValue::Object summary;
  put(summary, "items", banked_items);
  put(summary, "trophies", 0);
  put(summary, "storedItems", std::move(stored));
  put(summary, "storedTrophies", JsonValue::Array{});
  emit(Envelope{"player:extract", JsonValue(std::move(summary))});
  emit_message(emit, "Banked " + std::to_string(banked_items) + " items into the House store.");
  if (retained_maps > 0)
    emit_message(emit, std::to_string(retained_maps) +
                           " charted tablet" +
                           (retained_maps == 1 ? " remains" : "s remain") +
                           " in the House map case.");
  emit_inventory_refresh(emit);
  emit_equip_state(emit);
  endgame_active_ = false;
  endgame_completed_ = false;
  endgame_map_tier_ = 0;
  endgame_goods_found_percent_ = 0;
  endgame_map_name_.clear();
  endgame_map_modifiers_.clear();
}
void ProtocolSession::sync_combat_mods() {
  const auto totals=wear_.totals();
  PlayerCombatMods mods=world_->player_combat_mods();  // preserves force_critical
  mods.critical_chance=totals.modifiers.critical_chance;
  mods.goods_found=totals.modifiers.goods_found;
  mods.damage_against_beasts=totals.modifiers.damage_against_beasts;
  // combat/index.js attackStyle: the dominant trained channel, stab first on ties.
  const ChannelRatings& a=totals.attack;
  int best=a.stab; std::string style="stab";
  if (a.slash>best) { best=a.slash; style="slash"; }
  if (a.crush>best) { best=a.crush; style="crush"; }
  if (a.range>best) { best=a.range; style="range"; }
  mods.attack_style=best>0?style:"slash";
  world_->set_player_combat_mods(mods);
}
void ProtocolSession::handle_give(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) {
  // dev.js dev:give: real inventory pipeline, overflow 'drop' at the feet.
  const std::string item_id=as_string(payload.get("itemId"));
  const int quantity=(std::max)(1,as_int(payload.get("qty"),1));
  Mulberry32 seeded;
  Mulberry32* rng=&session_rng_;  // unseeded grants draw from the session stream (JS: Math.random)
  if (const auto* seed_value=payload.get("seed")) {
    if (seed_value->number()) { seeded=Mulberry32(static_cast<std::uint32_t>(std::floor(*seed_value->number()))); rng=&seeded; }
  }
  const auto* level_value=payload.get("itemLevel");
  const int item_level=level_value&&level_value->number()?static_cast<int>(*level_value->number()):0;
  const ItemDef* def=item_def(item_id);
  if (!def) { emit_message(emit,"Unknown item "+item_id+"."); return; }
  int dropped=0;
  if (def->stackable) {
    CreateItemOptions opts; opts.quantity=quantity;
    auto item=create_game_item(item_id,opts);
    if (item) inventory_.add(std::move(*item));
  } else {
    for (int i=0;i<quantity;++i) {
      CreateItemOptions opts; opts.rng=rng; opts.item_level=item_level; opts.bind_to=identity_; opts.forge=&world_->forge();
      auto item=create_game_item(item_id,opts);
      if (!item) break;
      auto result=inventory_.add(std::move(*item));
      const auto position=world_->position();
      for (auto& spill:result.overflow) { world_->add_ground_item(std::move(spill),position.x,position.y); ++dropped; }
    }
  }
  if (dropped>0) { emit_message(emit,"Your backpack is full. "+std::to_string(dropped)+" item"+(dropped==1?"":"s")+" fell at your feet."); emit_ground_change(emit); }
  emit_inventory_refresh(emit);
  emit_message(emit,"Granted "+std::to_string(quantity)+"x "+item_id+".");
}
void ProtocolSession::handle_drop(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) {
  // dev.js dev:drop: unbound deterministic gear on the active floor.
  const std::string item_id=as_string(payload.get("itemId"));
  Mulberry32 seeded;
  Mulberry32* rng=&session_rng_;
  if (const auto* seed_value=payload.get("seed")) {
    if (seed_value->number()) { seeded=Mulberry32(static_cast<std::uint32_t>(std::floor(*seed_value->number()))); rng=&seeded; }
  }
  const auto* level_value=payload.get("itemLevel");
  CreateItemOptions opts; opts.rng=rng; opts.forge=&world_->forge();
  if (level_value&&level_value->number()) opts.item_level=static_cast<int>(*level_value->number());
  auto item=create_game_item(item_id,opts);
  if (!item) return;
  const auto position=world_->position();
  world_->add_ground_item(std::move(*item),position.x,position.y);
  emit_ground_change(emit);
  emit_message(emit,"Dropped "+item_id+" on the active floor.");
}
void ProtocolSession::handle_equip(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) {
  // item:equip (wear-slots.js resolveEquipSlot + wear.js totals refresh).
  const auto* item_data=payload.get("item");
  const std::string uuid=as_string(item_data?item_data->get("uuid"):nullptr);
  const std::string target=as_string(item_data?item_data->get("targetSlot"):nullptr);
  GameItem item;
  if (uuid.empty()||!inventory_.remove_by_uuid(uuid,&item)) {
    // JS sendInventoryError: game:send:message + inventory refresh, no mutation.
    emit_message(emit,"That item is no longer in your inventory.");
    emit_inventory_refresh(emit);
    return;
  }
  const ItemDef* def=item_def(item.id);
  const std::string base=!item.equip_slot.empty()?item.equip_slot:(def?def->slot:"");
  if (base.empty()) { inventory_.add(std::move(item)); emit_inventory_refresh(emit); return; }
  const std::string seat=wear_.resolve_seat(base,target);
  auto displaced=wear_.equip(std::move(item),seat);
  bool spilled=false;
  if (displaced) {
    displaced->slot=-1;
    auto result=inventory_.add(std::move(*displaced));
    // Full backpack mid-swap: the displaced piece spills bound at the feet
    // (JS aborts the equip instead; documented N4 simplification — the
    // scenario set never swaps onto a full grid).
    const auto position=world_->position();
    for (auto& spill:result.overflow) { world_->add_ground_item(std::move(spill),position.x,position.y); spilled=true; }
  }
  sync_combat_mods();
  emit_inventory_refresh(emit);
  emit_equip_state(emit);
  quest_trigger("equip-vessel", emit);
  if (spilled) emit_ground_change(emit);
}
JsonValue ProtocolSession::quests_json() const {
  JsonValue::Object first_goal;
  put(first_goal, "stage", first_goal_stage_);
  if (first_goal_started_ms_ > 0) put(first_goal, "startedAt", static_cast<double>(first_goal_started_ms_));
  else put(first_goal, "startedAt", nullptr);
  if (first_goal_completed_ms_ > 0) put(first_goal, "completedAt", static_cast<double>(first_goal_completed_ms_));
  else put(first_goal, "completedAt", nullptr);
  JsonValue::Object quests;
  put(quests, "firstGoal", std::move(first_goal));
  put(quests, "activeQuestId", active_quest_ < kQuestChainSize ? JsonValue(kQuestChain[active_quest_].id) : JsonValue(nullptr));
  put(quests, "objectiveIndex", quest_objective_);
  put(quests, "questPoints", quest_points_);
  put(quests, "houseRenown", house_renown_);
  put(quests, "campaignComplete", campaign_complete_);
  if (active_quest_ < kQuestChainSize) {
    const QuestDef& active = kQuestChain[active_quest_];
    JsonValue::Object quest;
    put(quest, "id", active.id);
    put(quest, "title", active.title);
    put(quest, "giver", active.giver);
    put(quest, "summary", active.summary);
    put(quest, "objectiveIndex", quest_objective_);
    put(quest, "objectiveCount", active.objective_count);
    put(quest, "reward", active.reward);
    JsonValue::Object objective;
    if (quest_objective_ >= 0 && quest_objective_ < active.objective_count)
      put(objective, "text", active.objectives[quest_objective_].text);
    else
      put(objective, "text", "Commission complete.");
    put(quest, "objective", std::move(objective));
    put(quests, "activeQuest", std::move(quest));
  } else {
    put(quests, "activeQuest", nullptr);
  }
  JsonValue::Array completed;
  for (const auto& done : quests_completed_) {
    JsonValue::Object entry; put(entry, "id", done);
    for (const auto& definition : kQuestChain) {
      if (done != definition.id) continue;
      put(entry, "title", definition.title);
      put(entry, "deed", definition.deed);
      break;
    }
    completed.emplace_back(std::move(entry));
  }
  put(quests, "completed", std::move(completed));
  return JsonValue(std::move(quests));
}

void ProtocolSession::tree_attributes(int* strength, int* dexterity, int* intelligence) const {
  // verdigris-geometric-tree.js: the tree is GEOMETRIC - node "q,r" ids sit
  // on hex axes (STR {q:-1,r:1}, DEX {q:0,r:-1}, INT {q:1,r:0}). Each
  // allocated node feeds the attribute whose axis it best aligns with.
  int str_total = 10, dex_total = 10, int_total = 10;
  if (passive_tree_saved_) {
    if (const auto* nodes = passive_tree_.get("nodes"); nodes && nodes->array()) {
      for (const auto& entry : *nodes->array()) {
        if (!entry.string()) continue;
        const std::string& id = *entry.string();
        const auto comma = id.find(',');
        if (comma == std::string::npos) continue;
        int q = 0, r = 0;
        try { q = std::stoi(id.substr(0, comma)); r = std::stoi(id.substr(comma + 1)); } catch (...) { continue; }
        if (q == 0 && r == 0) continue;  // root grants nothing
        const int str_score = -q + r;
        const int dex_score = -r;
        const int int_score = q;
        if (str_score >= dex_score && str_score >= int_score) str_total += 2;
        else if (dex_score >= int_score) dex_total += 2;
        else int_total += 2;
      }
    }
  }
  if (strength) *strength = str_total;
  if (dexterity) *dexterity = dex_total;
  if (intelligence) *intelligence = int_total;
}
JsonValue ProtocolSession::passive_tree_json() const {
  // verdigris-authority.js: server owns the budget. earned =
  // min(140, min(max(2, level), 117) + min(questPoints, 23)).
  const auto* actor = simulation_->actor(simulation_->scion().actor_id);
  const int level = actor ? actor->stats.level : 1;
  const int earned = (std::min)(140, (std::min)((std::max)(2, level), 117) +
                                     (std::min)((std::max)(0, tree_quest_points_), 23));
  JsonValue::Array nodes;
  JsonValue::Array conduits;
  std::string selected = "0,0";
  JsonValue::Array class_order;
  int spent = 1;
  if (passive_tree_saved_) {
    if (const auto* saved_nodes = passive_tree_.get("nodes"); saved_nodes && saved_nodes->array()) {
      nodes = *saved_nodes->array();
      spent = (std::max)(0, static_cast<int>(nodes.size()) - 1);  // root is free
    }
    if (const auto* saved_conduits = passive_tree_.get("conduits"); saved_conduits && saved_conduits->array()) {
      conduits = *saved_conduits->array();
      spent += static_cast<int>(conduits.size());  // each conduit choice costs a point
    }
    if (const auto* sel = passive_tree_.get("selectedNodeId"); sel && sel->string()) selected = *sel->string();
    if (const auto* order = passive_tree_.get("classOrder"); order && order->array()) class_order = *order->array();
  } else {
    nodes.emplace_back("0,0");
  }
  JsonValue::Object tree;
  put(tree, "schemaVersion", 2);
  put(tree, "nodes", std::move(nodes));
  put(tree, "conduits", std::move(conduits));
  put(tree, "points", JsonValue::Object{{"skill", JsonValue((std::max)(0, earned - spent))}});
  put(tree, "earned", earned);
  put(tree, "selectedNodeId", selected);
  put(tree, "classOrder", std::move(class_order));
  return JsonValue(std::move(tree));
}

void ProtocolSession::handle_skilltree_save(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) {
  const auto* snapshot = payload.get("snapshot");
  if (!snapshot || !snapshot->object()) return;
  passive_tree_ = *snapshot;
  passive_tree_saved_ = true;
  JsonValue::Object data;
  put(data, "player", JsonValue::Object{{"socket_id", socket_id_}});
  put(data, "passiveTree", passive_tree_json());
  emit(Envelope{"player:skilltree:update", JsonValue(std::move(data))});
}

namespace {
// shared/ui.js getExperience/getLevel (RS-style curve).
long long xp_for_level(int level) {
  long long a = 0;
  for (int x = 1; x < level; ++x) a += static_cast<long long>(std::floor(x + 265.0 * std::pow(2.0, x / 7.0)));
  return a / 4;
}
int level_from_xp(long long exp) {
  if (exp <= 0) return 1;
  int level = 1;
  long long calc = 0;
  while (exp > calc) {
    calc = xp_for_level(level);
    if (calc > exp) break;
    level += 1;
  }
  return (std::max)(1, level - 1);
}
}  // namespace
int ProtocolSession::carried_gold() const {
  int total = 0;
  for (const auto& item : inventory_.items()) if (item.id == "coins") total += item.qty;
  return total;
}

void ProtocolSession::emit_shop_screen(const std::function<void(const Envelope&)>& emit) const {
  // The two accepted owner-demo merchants have distinct identities and
  // stock. Selection is captured when the authoritative nearby action opens
  // the pane; purchases continue to refresh that same merchant.
  struct StockRow { const char* id; const char* name; int price; };
  const StockRow weapons[3] = {{"knife", "Knife", 5},
                               {"bronze-sword", "Bronze Sword", 15},
                               {"bronze-dagger", "Bronze Dagger", 10}};
  const StockRow rites[3] = {{"wooden-shield", "Wooden Shield", 8},
                             {"bronze-shield", "Bronze Shield", 45},
                             {"garnet-amulet", "Garnet Amulet", 60}};
  const StockRow* rows = shop_npc_id_ == 3 ? rites : weapons;
  JsonValue::Array stock;
  for (int i = 0; i < 3; ++i) {
    JsonValue::Object row;
    put(row, "id", rows[i].id); put(row, "name", rows[i].name);
    put(row, "price", rows[i].price); put(row, "qty", 10); put(row, "slot", i);
    stock.emplace_back(std::move(row));
  }
  JsonValue::Object payload;
  put(payload, "name", shop_npc_id_ == 3 ? "Selene's Rite Vault"
                                          : "Road Iron Yard");
  put(payload, "npcId", shop_npc_id_);
  { JsonValue::Array copy = stock; put(payload, "items", std::move(copy)); }
  put(payload, "inventory", std::move(stock));
  put(payload, "carriedCoins", carried_gold());
  JsonValue::Object data;
  put(data, "player", JsonValue::Object{{"socket_id", socket_id_}});
  put(data, "screen", "shop");
  put(data, "payload", std::move(payload));
  emit(Envelope{"open:screen", JsonValue(std::move(data))});
}

JsonValue ProtocolSession::bank_items_json() const {
  JsonValue::Array items;
  int index = 0;
  for (const auto& item : bank_) {
    JsonValue row = snapshot_item_json(item);
    if (auto* obj = row.object()) (*obj)["slot"] = JsonValue(index);
    items.emplace_back(std::move(row));
    ++index;
  }
  return JsonValue(std::move(items));
}
void ProtocolSession::emit_bank_screen(const std::function<void(const Envelope&)>& emit) const {
  // chronicles.js sendBankState: open:screen bank with House treasury.
  JsonValue::Object house;
  put(house, "id", active_house_id_.empty() ? JsonValue(nullptr) : JsonValue(active_house_id_));
  put(house, "name", active_house_name_.empty() ? JsonValue("House Verdigris") : JsonValue(active_house_name_));
  put(house, "treasury", house_treasury_);
  JsonValue::Object payload;
  put(payload, "items", bank_items_json());
  put(payload, "carriedCoins", carried_gold());
  put(payload, "house", std::move(house));
  JsonValue::Object data;
  put(data, "player", JsonValue::Object{{"socket_id", socket_id_}});
  put(data, "screen", "bank");
  put(data, "payload", std::move(payload));
  emit(Envelope{"open:screen", JsonValue(std::move(data))});
}

void ProtocolSession::handle_house_deposit(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) {
  const int amount = as_int(payload.get("amount"), 0);
  if (amount <= 0 || carried_gold() < amount) return;
  int remaining = amount;
  auto slots = inventory_.items();
  for (const auto& item : slots) {
    if (remaining <= 0) break;
    if (item.id != "coins") continue;
    GameItem taken;
    if (!inventory_.remove_by_uuid(item.uuid, &taken)) continue;
    if (taken.qty > remaining) {
      GameItem back = taken;
      back.qty = taken.qty - remaining;
      inventory_.add(std::move(back));
      remaining = 0;
    } else {
      remaining -= taken.qty;
    }
  }
  house_treasury_ += amount;
  if (auto* root = chronicle_.object()) {
    auto houses_it = root->find("houses");
    if (houses_it != root->end() && houses_it->second.array()) {
      for (auto& house_entry : *houses_it->second.array()) {
        auto* house = house_entry.object();
        if (!house) continue;
        auto id_it = house->find("id");
        if (id_it == house->end() || !id_it->second.string() || *id_it->second.string() != active_house_id_) continue;
        (*house)["treasury"] = JsonValue(house_treasury_);
        chronicles_revision_ += 1;
        break;
      }
    }
  }
  emit_message(emit, std::to_string(amount) + " gold nailed under the boards of House " +
                     (active_house_name_.empty() ? std::string("Verdigris") : active_house_name_) + ".");
  emit_inventory_refresh(emit);
  emit_bank_screen(emit);
  emit_wagon_screen(emit);
}

void ProtocolSession::maybe_floor_cleared(const std::function<void(const Envelope&)>& emit) {
  // party.js completeInstanceFloor: fires once per floor when the pack dies.
  if (!world_->in_instance()) return;
  bool any_alive = false;
  for (const auto& monster : world_->monsters()) if (monster.alive) { any_alive = true; break; }
  if (any_alive) return;
  const auto& meta = world_->metadata();
  const std::uint64_t key = meta.seed * 131u + static_cast<std::uint64_t>(meta.depth);
  if (key == last_cleared_floor_key_) return;
  last_cleared_floor_key_ = key;
  {
    JsonValue::Object data;
    put(data, "player", JsonValue::Object{{"socket_id", socket_id_}});
    put(data, "depth", meta.depth);
    put(data, "rewards", JsonValue::Object{{"coins", JsonValue(0)}});
    emit(Envelope{"party:instance:complete", JsonValue(std::move(data))});
  }
  emit_message(emit, "Floor " + std::to_string(meta.depth) +
      " cleared! Rewards distributed - find the stairs to descend, or take the entry stairs to leave.");
  if (!house_progression_.first_clear_completed) {
    (void)mark_first_clear(house_progression_);
    persist_house_progression();
    emit_message(emit, "Your first clear has opened a founding choice. Rhea is waiting at the House Coffer in the Crossroads.");
  } else if (grants_house_income(house_progression_)) {
    const int income = house_progression_.house_income_per_tick;
    house_treasury_ += income;
    persist_house_progression();
    emit_message(emit, "House production returns " + std::to_string(income) +
                           " gold to the treasury from this clear.");
  }
  if (first_goal_stage_ == "clear-floor" && meta.theme == "dungeon" && meta.layout == "warren" && meta.depth == 1) {
    first_goal_stage_ = "return-to-town";
    emit_message(emit, "The floor is cleared. Return to Aldwyn at the Crossroads for your reward.");
    emit_quest_update(emit);
  }
}
void ProtocolSession::emit_wagon_screen(const std::function<void(const Envelope&)>& emit) const {
  // wagon-service.js buildStock + payload: tiered outfitting from the ledger.
  struct TierRow { int tier; const char* label; bool unlocked; const char* requirement; };
  const TierRow tiers[3] = {
    {1, "Road kit", true, ""},
    {2, "Proven iron", false, "renown 500 or a level-1 House Forge"},
    {3, "Named steel", false, "renown 1500 or a level-2 House Forge"},
  };
  const char* tier_items[3][3] = {
    {"bronze-sword", "bronze-dagger", "wooden-shield"},
    {"iron-sword", "bronze-shield", "shortbow"},
    {"steel-battleaxe", "longbow", "gold-ring"},
  };
  const int tier_prices[3][3] = {{15, 10, 8}, {60, 40, 45}, {649, 320, 210}};
  JsonValue::Array stock;
  for (int t = 0; t < 3; ++t) {
    JsonValue::Object tier;
    put(tier, "tier", tiers[t].tier);
    put(tier, "label", tiers[t].label);
    put(tier, "unlocked", tiers[t].unlocked);
    if (tiers[t].requirement[0]) put(tier, "requirement", tiers[t].requirement);
    else put(tier, "requirement", nullptr);
    JsonValue::Array items;
    for (int i = 0; i < 3; ++i) {
      JsonValue::Object item;
      put(item, "id", tier_items[t][i]);
      put(item, "name", tier_items[t][i]);
      put(item, "price", tier_prices[t][i]);
      items.emplace_back(std::move(item));
    }
    put(tier, "items", std::move(items));
    stock.emplace_back(std::move(tier));
  }
  JsonValue::Object house;
  put(house, "id", active_house_id_.empty() ? JsonValue(nullptr) : JsonValue(active_house_id_));
  put(house, "name", active_house_name_.empty() ? JsonValue("House Verdigris") : JsonValue(active_house_name_));
  put(house, "treasury", house_treasury_);
  JsonValue::Object payload;
  put(payload, "house", std::move(house));
  put(payload, "stock", std::move(stock));
  put(payload, "carriedCoins", carried_gold());
  JsonValue::Object data;
  put(data, "player", JsonValue::Object{{"socket_id", socket_id_}});
  put(data, "screen", "wagon");
  put(data, "payload", std::move(payload));
  emit(Envelope{"open:screen", JsonValue(std::move(data))});
}
void ProtocolSession::quest_trigger(const char* trigger, const std::function<void(const Envelope&)>& emit,
                                    const std::string& detail_a, const std::string& detail_b, int depth) {
  if (active_quest_ >= kQuestChainSize) return;
  const QuestDef& quest = kQuestChain[active_quest_];
  if (quest_objective_ >= quest.objective_count) return;
  const QuestObjective& objective = quest.objectives[quest_objective_];
  if (std::string(objective.trigger) != trigger) return;
  // criteria: crit_a matches zoneId or monsterName; crit_b matches theme/template.
  if (objective.crit_a[0] && detail_a != objective.crit_a) return;
  if (objective.crit_b[0] && detail_b != objective.crit_b) return;
  if (objective.min_depth > 0 && depth < objective.min_depth) return;
  if (objective.min_depth == 0 && std::string(trigger) == "delve" && objective.crit_b[0] && depth < 2) return;
  quest_objective_ += 1;
  if (quest_objective_ >= quest.objective_count) {
    quests_completed_.push_back(quest.id);
    quest_points_ = (std::min)(quest_points_ + 1, 23);
    tree_quest_points_ = (std::min)(tree_quest_points_ + 1, 23);
    house_renown_ += quest.renown;
    // chronicle: renown on the house, deed on the active scion.
    if (auto* root = chronicle_.object()) {
      auto houses_it = root->find("houses");
      if (houses_it != root->end() && houses_it->second.array()) {
        for (auto& house_entry : *houses_it->second.array()) {
          auto* house = house_entry.object();
          if (!house) continue;
          auto id_it = house->find("id");
          if (id_it == house->end() || !id_it->second.string() || *id_it->second.string() != active_house_id_) continue;
          (*house)["renown"] = JsonValue(house_renown_);
          auto scions_it = house->find("scions");
          if (scions_it != house->end() && scions_it->second.array()) {
            for (auto& scion_entry : *scions_it->second.array()) {
              auto* scion = scion_entry.object();
              if (!scion) continue;
              auto sid = scion->find("id");
              if (sid == scion->end() || !sid->second.string() || *sid->second.string() != active_scion_id_) continue;
              auto deeds_it = scion->find("deeds");
              if (deeds_it == scion->end() || !deeds_it->second.array()) (*scion)["deeds"] = JsonValue(JsonValue::Array{});
              scion->find("deeds")->second.array()->emplace_back(quest.deed);
              break;
            }
          }
          chronicles_revision_ += 1;
          break;
        }
      }
    }
    active_quest_ += 1;
    quest_objective_ = 0;
    emit_message(emit, std::string("Commission complete: ") + quest.id + ".");
    if (active_quest_ == kQuestChainSize && !campaign_complete_) {
      campaign_complete_ = true;
      if (JsonValue::Object* house =
              find_chronicle_house_object(chronicle_, active_house_id_)) {
        (*house)["campaignComplete"] = JsonValue(true);
        (*house)["endgameMapsCompleted"] = JsonValue(endgame_maps_completed_);
        chronicles_revision_ += 1;
      }
      const auto position = world_->position();
      award_expedition_map(1, position.x, position.y, emit, true);
      emit_message(emit,
                   "Campaign complete. Charted tablets now open one-use "
                   "endgame expeditions from the Crossroads.");
    }
  }
  persist_quest_progression();
  emit_quest_update(emit);
}
void ProtocolSession::emit_chart_screen(const std::string& road_id, const std::function<void(const Envelope&)>& emit) const {
  const int ri = road_index(road_id);
  if (ri < 0) return;
  const std::string house = active_house_id_.empty() ? identity_ : active_house_id_;
  int frontier = 1;
  for (const auto& cleared : cleared_nodes_) {
    std::string road; int tier = 0; int index = 0;
    if (parse_node_id(cleared, &road, &tier, &index) && road == road_id) frontier = (std::max)(frontier, tier + 1);
  }
  const auto nodes = web_road_nodes(house, road_id, frontier);
  JsonValue::Array node_rows;
  for (const auto& node : nodes) {
    const bool is_cleared = cleared_nodes_.count(node.id) > 0;
    const bool unlocked = node.tier == 1 || (!node.parent_id.empty() && cleared_nodes_.count(node.parent_id) > 0);
    JsonValue::Object row;
    put(row, "id", node.id); put(row, "name", node.name);
    put(row, "tier", node.tier); put(row, "index", node.index);
    put(row, "roadId", road_id); put(row, "roadName", kRoads[ri].name);
    put(row, "template", node.template_id); put(row, "layout", node.layout);
    put(row, "wardenName", node.warden_name);
    put(row, "parentId", node.parent_id.empty() ? JsonValue(nullptr) : JsonValue(node.parent_id));
    put(row, "status", is_cleared ? "cleared" : (unlocked ? "open" : "barred"));
    node_rows.emplace_back(std::move(row));
  }
  JsonValue::Object payload;
  put(payload, "roadId", road_id); put(payload, "roadName", kRoads[ri].name);
  put(payload, "direction", kRoads[ri].direction); put(payload, "blurb", kRoads[ri].blurb);
  put(payload, "nodes", std::move(node_rows));
  JsonValue::Object data;
  put(data, "player", JsonValue::Object{{"socket_id", socket_id_}});
  put(data, "screen", "chart");
  put(data, "payload", std::move(payload));
  emit(Envelope{"open:screen", JsonValue(std::move(data))});
}

void ProtocolSession::enter_road_node(const std::string& node_id, const std::function<void(const Envelope&)>& emit) {
  std::string road; int tier = 0; int index = 0;
  if (!parse_node_id(node_id, &road, &tier, &index)) return;
  const std::string house = active_house_id_.empty() ? identity_ : active_house_id_;
  const auto nodes = web_road_nodes(house, road, tier + 1);
  const RoadNode* node = nullptr;
  for (const auto& candidate : nodes) if (candidate.id == node_id) { node = &candidate; break; }
  if (!node) return;
  endgame_active_ = false;
  endgame_completed_ = false;
  world_->clear_expedition_tuning();
  current_node_id_ = node->id;
  current_node_tier_ = node->tier;
  current_node_name_ = node->name;
  current_child_id_ = node->child_ids.empty() ? std::string() : node->child_ids.front();
  current_child_name_.clear();
  if (!current_child_id_.empty()) {
    for (const auto& candidate : nodes) if (candidate.id == current_child_id_) { current_child_name_ = candidate.name; break; }
  }
  node_warden_dead_on_entry_ = cleared_nodes_.count(node->id) > 0;
  world_->set_boss_name_override(node->warden_name);
  world_->set_spawn_suppressed(node_warden_dead_on_entry_);
  world_->enter_solo_instance(node->template_id, node->layout);
  world_->set_spawn_suppressed(false);
  world_->set_boss_name_override(std::string());
  world_->set_block_stairs_down(!node_warden_dead_on_entry_);
  world_->set_scene_name(current_node_name_);
  world_->set_stairs_up_returns_to_town(true);
  last_instance_theme_ = world_->metadata().theme;
  last_instance_layout_ = world_->metadata().layout;
  emit_transition(emit, "world:scene:transition");
  emit_ground_change(emit);
  quest_trigger("delve", emit, zone_id_for_instance(world_->metadata().theme, world_->metadata().layout), world_->metadata().theme, world_->metadata().depth);
}

void ProtocolSession::award_expedition_map(
    int tier, double x, double y,
    const std::function<void(const Envelope&)>& emit, bool to_backpack) {
  static constexpr const char* kTabletIds[] = {
      "charted-tablet-barrow", "charted-tablet-reeds",
      "charted-tablet-crown", "charted-tablet-thorns"};
  const int bounded_tier = std::clamp(tier, 1, 16);
  const int pick = static_cast<int>(std::floor(session_rng_.next() * 4.0)) % 4;
  CreateItemOptions options;
  options.rng = &session_rng_;
  options.item_level = bounded_tier;
  auto tablet = create_game_item(kTabletIds[pick], options);
  if (!tablet) return;
  const std::string name = tablet->display_name;
  if (to_backpack) {
    auto result = inventory_.add(std::move(*tablet));
    for (auto& spill : result.overflow)
      world_->add_ground_item(std::move(spill), x, y);
    emit_inventory_refresh(emit);
    if (!result.overflow.empty()) emit_ground_change(emit);
  } else {
    world_->add_ground_item(std::move(*tablet), x, y);
    emit_ground_change(emit);
  }
  emit_message(emit, name + (to_backpack ? " was entered in your ledger."
                                             : " fell from the Warden."));
}

void ProtocolSession::open_expedition_map(
    const std::string& uuid,
    const std::function<void(const Envelope&)>& emit) {
  if (!campaign_complete_) {
    emit_message(emit, "The charted roads open after the campaign commissions.");
    return;
  }
  if (world_->in_instance()) {
    emit_message(emit, "A charted tablet can only be broken at the Crossroads.");
    return;
  }
  const GameItem* carried = inventory_.find_by_uuid(uuid);
  if (!carried || !carried->expedition_map) {
    emit_message(emit, "That charted tablet is no longer in your backpack.");
    emit_inventory_refresh(emit);
    return;
  }

  GameItem consumed;
  if (!inventory_.remove_by_uuid(uuid, &consumed) || !consumed.expedition_map)
    return;
  const ExpeditionMapBlock map = *consumed.expedition_map;
  current_node_id_.clear();
  current_child_id_.clear();
  endgame_active_ = true;
  endgame_completed_ = false;
  endgame_map_tier_ = map.tier;
  endgame_goods_found_percent_ = map.goods_found_percent;
  endgame_map_name_ = consumed.display_name;
  endgame_map_modifiers_ = map.modifiers;
  world_->set_expedition_tuning(map.monster_level_bonus,
                                map.monster_life_percent,
                                map.monster_damage_percent,
                                map.extra_monsters);
  world_->set_boss_name_override("The Seal-Bound Warden");
  world_->enter_solo_instance(map.theme, map.layout);
  world_->set_boss_name_override(std::string());
  world_->set_block_stairs_down(true);
  world_->set_stairs_up_returns_to_town(true);
  world_->set_scene_name(consumed.display_name);
  last_instance_theme_ = world_->metadata().theme;
  last_instance_layout_ = world_->metadata().layout;
  emit_inventory_refresh(emit);
  emit_transition(emit, "world:scene:transition");
  emit_ground_change(emit);
  emit_message(emit, "The tablet breaks. " + consumed.display_name +
                         " opens for one expedition.");
}

void ProtocolSession::enter_shared_instance(const std::string& scene_id, const std::function<void(const Envelope&)>& emit) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  current_node_id_.clear();
  endgame_active_ = false;
  endgame_completed_ = false;
  world_->clear_expedition_tuning();
  world_->set_block_stairs_down(false);
  world_->set_stairs_up_returns_to_town(false);
  world_->enter_solo_instance("dungeon", "warren");
  world_->set_scene_id(scene_id);
  last_instance_theme_ = world_->metadata().theme;
  last_instance_layout_ = world_->metadata().layout;
  emit_transition(emit, "party:scene:transition");
  emit_ground_change(emit);
}

void ProtocolSession::adopt_world(std::shared_ptr<WorldSimulation> world, const std::string& scene_id, const std::function<void(const Envelope&)>& emit) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  world_ = std::move(world);
  (void)scene_id;
  emit_transition(emit, "party:scene:transition");
  emit_ground_change(emit);
}

void ProtocolSession::leave_to_town(const std::function<void(const Envelope&)>& emit) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  if (world_.use_count() > 1) {
    // leaving a SHARED party world: fall back to a personal town world so
    // the remaining members keep their live instance untouched.
    std::uint64_t seed = 1469598103934665603ULL;
    for (unsigned char c : identity_) seed = (seed ^ c) * 1099511628211ULL;
    world_ = std::make_shared<WorldSimulation>(seed, identity_);
    emit_movement(emit);
    emit_transition(emit, "party:scene:transition");
    return;
  }
  if (world_->in_instance()) {
    world_->return_to_surface();
    emit_movement(emit);
    emit_transition(emit, "party:scene:transition");
  }
}
void ProtocolSession::check_road_gates(const std::function<void(const Envelope&)>& emit) {
  // gates.mjs: standing on a road-gate tile in town opens that chart.
  if (world_->in_instance()) {
    // node instances: landing on the stairs-down gate while the Warden
    // lives holds the road (world-web.mjs section 3).
    if (!current_node_id_.empty()) {
      const Vec2 tile = tile_movement::occupied_tile(world_->position());
      const auto& meta = world_->metadata();
      if (tile.x == meta.stairs_down.x && tile.y == meta.stairs_down.y) {
        bool warden_alive = false;
        for (const auto& monster : world_->monsters()) if (monster.alive && monster.boss) { warden_alive = true; break; }
        if (warden_alive) emit_message(emit, "No road holds past a living Warden.");
      }
    }
    return;
  }
  const Vec2 tile = tile_movement::occupied_tile(world_->position());
  for (const auto& gate : kRoadGates) {
    if (gate.x == tile.x && gate.y == tile.y) { emit_chart_screen(gate.road, emit); break; }
  }
}
void ProtocolSession::emit_quest_update(const std::function<void(const Envelope&)>& emit) const {
  // first-goal.js pushQuestState -> quest:update.
  JsonValue::Object data;
  put(data, "player", JsonValue::Object{{"socket_id", socket_id_}});
  put(data, "quests", quests_json());
  put(data, "questPoints", tree_quest_points_);
  put(data, "passiveTree", passive_tree_json());
  emit(Envelope{"quest:update", JsonValue(std::move(data))});
}

void ProtocolSession::handle_npc_talk(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) {
  // Town conversation is server-authoritative: identity, reach, quest
  // mutation, available services, and investment eligibility all come from
  // this session. The client only paints the resulting dialogue payload.
  const auto* item = payload.get("item");
  const int npc_id = as_int(item ? item->get("id") : nullptr, -1);
  const TownNpc* npc = town_npc(npc_id);
  if (!npc || world_->in_instance()) return;
  const Vec2 tile = tile_movement::occupied_tile(world_->position());
  if ((std::max)(std::abs(tile.x - npc->x), std::abs(tile.y - npc->y)) > 1)
    return;
  if (npc_id == 1 && first_goal_stage_ == "available") {
    first_goal_stage_ = "clear-floor";
    first_goal_started_ms_ = now_ms();
    emit_quest_update(emit);
  }
  emit_npc_dialogue(npc_id, emit);
}

void ProtocolSession::emit_npc_dialogue(
    int npc_id, const std::function<void(const Envelope&)>& emit) const {
  const TownNpc* npc = town_npc(npc_id);
  if (!npc || world_->in_instance()) return;
  const Vec2 tile = tile_movement::occupied_tile(world_->position());
  if ((std::max)(std::abs(tile.x - npc->x), std::abs(tile.y - npc->y)) > 1)
    return;

  std::string body = npc->examine;
  JsonValue::Array options;
  const auto add_option = [&](const char* id, const char* label,
                              const char* hint, const char* action,
                              bool enabled = true) {
    JsonValue::Object option;
    put(option, "id", id); put(option, "label", label);
    put(option, "hint", hint); put(option, "action", action);
    put(option, "enabled", enabled);
    options.emplace_back(std::move(option));
  };
  if (npc_id == 1) {
    if (first_goal_stage_ == "clear-floor")
      body = "No road holds past a living Warden. Ash banners crowd the Thornward ridge. Take a first stretch, put its Warden down, and bring your House home.";
    else if (first_goal_stage_ == "return-to-town")
      body = "The road has carried word ahead of you. Return through the gate and I will mark the deed in Verdigris.";
    else if (first_goal_stage_ == "complete")
      body = "The chart remembers your first Warden. The deeper roads will ask more of every Scion who follows.";
    add_option("tin", "Review the Tin Road chart",
               "Choose an open stretch and set out.", "world:road:chart");
  } else if (npc_id == 2) {
    body = "An edge is a promise you keep with a stone. Choose road iron that can keep yours.";
    add_option("weapons", "Browse the Road Iron Yard",
               "Weapons and practical tools for the next patrol.",
               "player:npc:trade");
  } else if (npc_id == 3) {
    body = "Steel remembers the hand that consecrates it. My vault carries armor and fittings for those willing to be remembered in turn.";
    add_option("rite-vault", "Browse the Rite Vault",
               "Armor and ritual fittings selected by Selene.",
               "player:npc:trade");
  } else if (npc_id == 4) {
    if (first_clear_eligible(house_progression_)) {
      body = "Your first cleared road has earned one founding investment. Choose for this Scion now, or build a yield every future clear returns to the House.";
    } else if (house_progression_.choice == FirstInvestmentChoice::ScionGear) {
      body = "The first investment armed a Scion. The entry is sealed; what they make of that iron belongs to the Chronicle.";
    } else if (house_progression_.choice == FirstInvestmentChoice::HouseProduction) {
      body = "The first investment went into House production. Every cleared floor now returns five gold to the shared ledger.";
    } else {
      body = "The coffer opens its first true choice after your House clears a floor. Until then, I can keep what you cannot carry.";
    }
    add_option("bank", "Open the Countinghouse",
               "Move carried goods and gold into House keeping.",
               "player:screen:bank");
    if (first_clear_eligible(house_progression_)) {
      add_option("scion_gear", "Commission named Scion gear",
                 "Immediate tier-one Vesselforge gear, bound to this Scion.",
                 "house:investment:choose");
      add_option("house_production", "Build House road production",
                 "+5 House treasury after every future floor clear.",
                 "house:investment:choose");
    }
  }

  JsonValue::Object payload;
  put(payload, "npcId", npc->id); put(payload, "npcKey", npc->key);
  put(payload, "name", npc->name); put(payload, "role", npc->role);
  put(payload, "body", body); put(payload, "options", std::move(options));
  JsonValue::Object data;
  put(data, "player", JsonValue::Object{{"socket_id", socket_id_}});
  put(data, "screen", "dialogue"); put(data, "payload", std::move(payload));
  emit(Envelope{"open:screen", JsonValue(std::move(data))});
}

void ProtocolSession::persist_house_progression() {
  JsonValue::Object* house =
      find_chronicle_house_object(chronicle_, active_house_id_);
  if (!house) return;
  JsonValue::Object investment;
  put(investment, "firstClearCompleted", house_progression_.first_clear_completed);
  put(investment, "choice", choice_name(house_progression_.choice));
  put(investment, "rewardClaimed", house_progression_.reward_claimed);
  put(investment, "scionGearTier", house_progression_.scion_gear_tier);
  put(investment, "houseIncomePerClear", house_progression_.house_income_per_tick);
  (*house)["firstInvestment"] = JsonValue(std::move(investment));
  (*house)["treasury"] = JsonValue(house_treasury_);
  chronicles_revision_ += 1;
}

void ProtocolSession::restore_house_progression() {
  house_progression_ = {};
  JsonValue::Object* house =
      find_chronicle_house_object(chronicle_, active_house_id_);
  if (!house) return;
  house_treasury_ = as_int(house->find("treasury") == house->end()
                               ? nullptr : &house->find("treasury")->second,
                           0);
  const auto it = house->find("firstInvestment");
  if (it == house->end() || !it->second.object()) return;
  const JsonValue& investment = it->second;
  house_progression_.first_clear_completed =
      as_bool(investment.get("firstClearCompleted"), false);
  house_progression_.reward_claimed =
      as_bool(investment.get("rewardClaimed"), false);
  house_progression_.scion_gear_tier = static_cast<std::uint16_t>(
      (std::max)(0, as_int(investment.get("scionGearTier"), 0)));
  house_progression_.house_income_per_tick = static_cast<std::uint16_t>(
      (std::max)(0, as_int(investment.get("houseIncomePerClear"), 0)));
  const std::string choice = as_string(investment.get("choice"));
  if (choice == "scion_gear")
    house_progression_.choice = FirstInvestmentChoice::ScionGear;
  else if (choice == "house_production")
    house_progression_.choice = FirstInvestmentChoice::HouseProduction;
}

void ProtocolSession::persist_quest_progression() {
  JsonValue::Object* scion = find_chronicle_scion_object(
      chronicle_, active_house_id_, active_scion_id_);
  if (!scion) return;
  JsonValue::Object campaign;
  put(campaign, "activeQuestIndex", active_quest_);
  put(campaign, "objectiveIndex", quest_objective_);
  put(campaign, "questPoints", quest_points_);
  JsonValue::Array completed;
  for (const auto& id : quests_completed_) completed.emplace_back(id);
  put(campaign, "completed", std::move(completed));
  (*scion)["campaignQuests"] = JsonValue(std::move(campaign));
  chronicles_revision_ += 1;
}

void ProtocolSession::restore_quest_progression() {
  active_quest_ = campaign_complete_ ? kQuestChainSize : 0;
  quest_objective_ = 0;
  quests_completed_.clear();
  quest_points_ = 0;
  tree_quest_points_ = 0;
  JsonValue::Object* scion = find_chronicle_scion_object(
      chronicle_, active_house_id_, active_scion_id_);
  if (!scion) return;
  auto saved_it = scion->find("campaignQuests");
  if (saved_it == scion->end() || !saved_it->second.object()) return;
  const JsonValue& saved = saved_it->second;
  const int saved_active = std::clamp(
      as_int(saved.get("activeQuestIndex"), active_quest_), 0,
      kQuestChainSize);
  active_quest_ = campaign_complete_ ? kQuestChainSize : saved_active;
  const int objective_limit = active_quest_ < kQuestChainSize
                                  ? kQuestChain[active_quest_].objective_count - 1
                                  : 0;
  quest_objective_ = std::clamp(
      as_int(saved.get("objectiveIndex"), 0), 0,
      (std::max)(0, objective_limit));
  quest_points_ = std::clamp(as_int(saved.get("questPoints"), 0), 0, 23);
  tree_quest_points_ = quest_points_;
  if (const auto* completed = saved.get("completed");
      completed && completed->array()) {
    for (const auto& entry : *completed->array()) {
      if (!entry.string()) continue;
      for (const auto& definition : kQuestChain) {
        if (*entry.string() == definition.id &&
            std::find(quests_completed_.begin(), quests_completed_.end(),
                      definition.id) == quests_completed_.end()) {
          quests_completed_.push_back(definition.id);
          break;
        }
      }
    }
  }
}

void ProtocolSession::handle_house_investment(
    const JsonValue& payload, const std::function<void(const Envelope&)>& emit) {
  const Vec2 tile = tile_movement::occupied_tile(world_->position());
  const TownNpc* rhea = town_npc(4);
  if (!rhea || world_->in_instance() ||
      (std::max)(std::abs(tile.x - rhea->x), std::abs(tile.y - rhea->y)) > 1)
    return;
  const std::string requested = as_string(payload.get("choice"));
  FirstInvestmentChoice choice = FirstInvestmentChoice::Unchosen;
  if (requested == "scion_gear") choice = FirstInvestmentChoice::ScionGear;
  if (requested == "house_production")
    choice = FirstInvestmentChoice::HouseProduction;
  const InvestmentStatus status = apply_first_investment(house_progression_, choice);
  if (status != InvestmentStatus::Ok) {
    emit_message(emit, status == InvestmentStatus::NotEligible
                           ? "The House Coffer opens after your first cleared floor."
                           : status == InvestmentStatus::AlreadyChosen
                                 ? "The first House investment is already sealed."
                                 : "Rhea cannot enter that choice in the ledger.");
    emit_npc_dialogue(4, emit);
    return;
  }

  if (choice == FirstInvestmentChoice::ScionGear) {
    CreateItemOptions options;
    options.rng = &session_rng_;
    if (const auto* actor = simulation_->actor(simulation_->scion().actor_id))
      options.item_level = (std::max)(1, actor->stats.level);
    options.bind_to = identity_;
    options.forge = &world_->forge();
    auto reward = create_game_item(gear_drop_pool().front(), options);
    if (reward) {
      const std::string reward_name = reward->display_name;
      auto added = inventory_.add(std::move(*reward));
      const auto position = world_->position();
      for (auto& overflow : added.overflow)
        world_->add_ground_item(std::move(overflow), position.x, position.y);
      emit_message(emit, "Rhea breaks the coffer seal. " + reward_name +
                             " is entered against this Scion's name.");
      emit_inventory_refresh(emit);
      if (!added.overflow.empty()) emit_ground_change(emit);
    }
  } else {
    emit_message(emit, "Rhea seals the order: every future floor clear returns 5 gold to the House treasury.");
  }
  persist_house_progression();
  emit_npc_dialogue(4, emit);
}

void ProtocolSession::auto_pickup_gold(const std::function<void(const Envelope&)>& emit) {
  // gold.js: nearby coins enter the carried balance without a Take action.
  const Vec2 tile=tile_movement::occupied_tile(world_->position());
  std::vector<std::string> picked;
  for (const auto& ground:world_->ground_items()) {
    if (ground.item.id!="coins") continue;
    if ((std::max)(std::abs(static_cast<int>(std::floor(ground.x))-tile.x),
                   std::abs(static_cast<int>(std::floor(ground.y))-tile.y))>1) continue;
    picked.push_back(ground.item.uuid);
  }
  bool changed=false;
  for (const auto& uuid:picked) {
    GameItem item;
    if (!world_->take_ground_item(uuid,&item)) continue;
    item.slot=-1;
    inventory_.add(std::move(item));
    changed=true;
  }
  if (changed) { emit_inventory_refresh(emit); emit_ground_change(emit); }
}
void ProtocolSession::maybe_complete_first_goal(const std::function<void(const Envelope&)>& emit) {
  // first-goal.js notifyFirstGoalReturned - completes on returning to town.
  if (first_goal_stage_ != "return-to-town") return;
  first_goal_stage_ = "complete";
  first_goal_completed_ms_ = now_ms();
  tree_quest_points_ = (std::min)(tree_quest_points_ + 1, 12);
  emit_message(emit, "You kept your word. Take this Verdigris point; it opens another path in your skill tree.");
  emit_quest_update(emit);
}

void ProtocolSession::mark_relic_recovered(const std::string& scion_id) {
  auto* root = chronicle_.object();
  if (!root) return;
  auto houses_it = root->find("houses");
  if (houses_it == root->end() || !houses_it->second.array()) return;
  for (auto& house_entry : *houses_it->second.array()) {
    auto* house = house_entry.object();
    if (!house) continue;
    auto crypt_it = house->find("crypt");
    if (crypt_it == house->end() || !crypt_it->second.array()) continue;
    for (auto& crypt_entry : *crypt_it->second.array()) {
      auto* scion = crypt_entry.object();
      if (!scion) continue;
      auto id_it = scion->find("id");
      if (id_it == scion->end() || !id_it->second.string() || *id_it->second.string() != scion_id) continue;
      const double recovered_at = static_cast<double>(now_ms());
      auto relic_it = scion->find("relic");
      if (relic_it != scion->end() && relic_it->second.object()) {
        (*relic_it->second.object())["status"] = JsonValue("recovered");
        (*relic_it->second.object())["recoveredAt"] = JsonValue(recovered_at);
      } else {
        (*scion)["relic"] = JsonValue(JsonValue::Object{
            {"status", JsonValue("recovered")},
            {"recoveredAt", JsonValue(recovered_at)}});
      }
      chronicles_revision_ += 1;
      return;
    }
  }
}
void ProtocolSession::handle_take_ground(const std::string& uuid, const std::function<void(const Envelope&)>& emit) {
  // registry.js Take: chebyshev reach, bind check, real inventory admission.
  const GroundItem* found=nullptr;
  for (const auto& ground:world_->ground_items()) { if (ground.item.uuid==uuid) { found=&ground; break; } }
  if (!found) return;
  if (!found->item.bound_to.empty()&&found->item.bound_to!=identity_) return;
  const Vec2 player_tile=tile_movement::occupied_tile(world_->position());
  const int ix=static_cast<int>(std::floor(found->x));
  const int iy=static_cast<int>(std::floor(found->y));
  if ((std::max)(std::abs(ix-player_tile.x),std::abs(iy-player_tile.y))>1) return;
  const double gx=found->x; const double gy=found->y;
  const std::string relic_scion_id = found->relic_record_id.empty()
      ? std::string{} : found->relic_source_scion_id_field;
  GameItem item;
  if (!world_->take_ground_item(uuid,&item)) return;
  item.slot=-1;
  const bool took_vessel = static_cast<bool>(item.vessel);
  if (!relic_scion_id.empty()) mark_relic_recovered(relic_scion_id);
  quest_trigger("loot", emit);
  if (took_vessel) quest_trigger("loot-vessel", emit);
  auto result=inventory_.add(std::move(item));
  bool spilled=false;
  for (auto& spill:result.overflow) { world_->add_ground_item(std::move(spill),gx,gy); spilled=true; }  // no room: stays on the ground
  emit_inventory_refresh(emit);
  emit_ground_change(emit);
  (void)spilled;
}
void ProtocolSession::handle_take_underfoot(const std::function<void(const Envelope&)>& emit) {
  // player:take:underfoot: own tile, then +x/-x/+y/-y; takes ONE item.
  const Vec2 tile=tile_movement::occupied_tile(world_->position());
  const Vec2 candidates[]={{tile.x,tile.y},{tile.x+1,tile.y},{tile.x-1,tile.y},{tile.x,tile.y+1},{tile.x,tile.y-1}};
  for (const auto& candidate:candidates) {
    for (const auto& ground:world_->ground_items()) {
      if (static_cast<int>(std::floor(ground.x))!=candidate.x||static_cast<int>(std::floor(ground.y))!=candidate.y) continue;
      if (!ground.item.bound_to.empty()&&ground.item.bound_to!=identity_) continue;
      handle_take_ground(ground.item.uuid,emit);
      return;
    }
  }
}
void ProtocolSession::handle_menu_build(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) const {
  // registry.js: the server builds the menu; the client only renders it.
  const auto* misc=payload.get("miscData");
  const auto* clicked=misc?misc->get("clickedOn"):nullptr;
  auto clicked_has=[&](const char* marker) {
    if (!clicked||!clicked->object()) return false;
    for (const auto& [key,value]:*clicked->object()) { if (value.string()&&*value.string()==marker) return true; }
    return false;
  };
  JsonValue::Array entries;
  if (clicked_has("shopSlot")) {
    // shops.js pane menu: Buy-1 from the shop stock row.
    const int slot = as_int(misc ? misc->get("slot") : nullptr, -1);
    const char* ids[3] = {"knife", "bronze-sword", "wooden-shield"};
    const int prices[3] = {5, 15, 8};
    if (slot >= 0 && slot < 3) {
      JsonValue::Object entry;
      put(entry, "label", std::string("Buy-1 ") + ids[slot]);
      put(entry, "action", JsonValue::Object{{"name", JsonValue("Buy")}, {"actionId", JsonValue("player:shop:buy")},
          {"context", JsonValue::Array{JsonValue("shopSlot")}}, {"nearby", false}, {"weight", 1}});
      put(entry, "type", "shop");
      JsonValue::Object item_ref; put(item_ref, "id", ids[slot]); put(item_ref, "price", prices[slot]); put(item_ref, "slot", slot);
      put(entry, "item", std::move(item_ref));
      entries.emplace_back(std::move(entry));
    }
  }
  if (clicked_has("inventorySlot") && (shop_open_ || bank_open_)) {
    const int slot = as_int(misc ? misc->get("slot") : nullptr, -1);
    const GameItem* item = nullptr;
    if (slot >= 0) {
      for (const auto& candidate : inventory_.items()) { if (candidate.slot == slot) { item = &candidate; break; } }
    }
    if (item && shop_open_) {
      JsonValue::Object entry;
      put(entry, "label", std::string("Sell-1 ") + item->name);
      put(entry, "action", JsonValue::Object{{"name", JsonValue("Sell")}, {"actionId", JsonValue("player:shop:sell")},
          {"context", JsonValue::Array{JsonValue("inventorySlot")}}, {"nearby", false}, {"weight", 1}});
      put(entry, "type", "shop");
      JsonValue::Object item_ref; put(item_ref, "uuid", item->uuid); put(item_ref, "id", item->id); put(item_ref, "slot", slot);
      put(entry, "item", std::move(item_ref));
      entries.emplace_back(std::move(entry));
    }
    if (item && bank_open_) {
      const int quantities[3] = {1, 5, 10};
      for (int q : quantities) {
        JsonValue::Object entry;
        put(entry, "label", "Deposit-" + std::to_string(q));
        put(entry, "action", JsonValue::Object{{"name", JsonValue("Deposit")}, {"actionId", JsonValue("player:bank:deposit")},
            {"context", JsonValue::Array{JsonValue("inventorySlot")}}, {"nearby", false}, {"weight", 1}});
        put(entry, "type", "bank");
        JsonValue::Object item_ref; put(item_ref, "uuid", item->uuid); put(item_ref, "id", item->id); put(item_ref, "slot", slot); put(item_ref, "qty", q);
        put(entry, "item", std::move(item_ref));
        entries.emplace_back(std::move(entry));
      }
    }
  }
  if (clicked_has("bankSlot") && bank_open_) {
    const int slot = as_int(misc ? misc->get("slot") : nullptr, -1);
    if (slot >= 0 && slot < static_cast<int>(bank_.size())) {
      const GameItem& item = bank_[static_cast<std::size_t>(slot)];
      const int quantities[3] = {1, 5, 10};
      for (int q : quantities) {
        JsonValue::Object entry;
        put(entry, "label", "Withdraw-" + std::to_string(q));
        put(entry, "action", JsonValue::Object{{"name", JsonValue("Withdraw")}, {"actionId", JsonValue("player:bank:withdraw")},
            {"context", JsonValue::Array{JsonValue("bankSlot")}}, {"nearby", false}, {"weight", 1}});
        put(entry, "type", "bank");
        JsonValue::Object item_ref; put(item_ref, "uuid", item.uuid); put(item_ref, "id", item.id); put(item_ref, "slot", slot); put(item_ref, "qty", q);
        put(entry, "item", std::move(item_ref));
        entries.emplace_back(std::move(entry));
      }
    }
  }
  if (clicked_has("gameMap")) {
    // World variant: Take per ground item on the clicked tile, newest first.
    const auto* tile=payload.get("tile");
    const auto* world_pos=tile?tile->get("world"):nullptr;
    const int wx=as_int(world_pos?world_pos->get("x"):nullptr);
    const int wy=as_int(world_pos?world_pos->get("y"):nullptr);
    if (!world_->in_instance()) {
      for (const auto& pitch : kWagonPitches) {
        if (pitch[0] != wx || pitch[1] != wy) continue;
        JsonValue::Object entry;
        put(entry, "label", "Open the House wagon");
        put(entry, "action", JsonValue::Object{{"name", JsonValue("wagon")}, {"actionId", JsonValue("player:screen:wagon")},
            {"context", JsonValue::Array{JsonValue("gameMap")}}, {"nearby", true}, {"weight", 1}});
        put(entry, "type", "wagon");
        entries.emplace_back(std::move(entry));
        break;
      }
    }
    if (!world_->in_instance() && wx==45 && wy==101) {
      // General Store floor display (town-amenities/economy): browse, buy,
      // appraise - display stock can never be taken for free.
      const char* kDisplayActions[3][3] = {
        {"Browse the General Store", "browse", "player:screen:shop-display"},
        {"Buy Bronze Sword", "buy", "player:shop-display:buy"},
        {"Appraise Bronze Sword", "appraise", "player:shop-display:appraise"},
      };
      for (const auto& row : kDisplayActions) {
        JsonValue::Object entry;
        put(entry, "label", row[0]);
        put(entry, "action", JsonValue::Object{{"name", JsonValue(row[1])}, {"actionId", JsonValue(row[2])},
            {"context", JsonValue::Array{JsonValue("gameMap")}}, {"nearby", true}, {"weight", 1}});
        put(entry, "type", "shop-display");
        put(entry, "shopItemId", "bronze-sword");
        JsonValue::Object item_ref; put(item_ref, "id", 1); put(item_ref, "shopItemId", "bronze-sword");
        put(entry, "item", std::move(item_ref));
        entries.emplace_back(std::move(entry));
      }
    }
    if (!world_->in_instance() && wx==38 && wy==115) {
      // town fountain (town-amenities): Drink restores to full.
      JsonValue::Object entry;
      put(entry, "label", "Drink from the fountain");
      put(entry, "action", JsonValue::Object{{"name", JsonValue("drink")}, {"actionId", JsonValue("player:fountain:drink")},
          {"context", JsonValue::Array{JsonValue("gameMap")}}, {"nearby", true}, {"weight", 1}});
      put(entry, "type", "object");
      entries.emplace_back(std::move(entry));
    }
    if (!world_->in_instance()) {
      for (const auto& npc : kTownNpcs) {
        if (npc.x != wx || npc.y != wy) continue;
        for (int i = 0; i < npc.action_count; ++i) {
          const std::string action = npc.actions[i];
          JsonValue::Object entry;
          const char* action_id = action == "talk" ? "player:npc:talk"
                                : action == "trade" ? "player:npc:trade"
                                : action == "bank" ? "player:screen:bank"
                                : "player:npc:examine";
          put(entry, "label", (action == "talk" ? std::string("Talk to ") + npc.name
                              : action == "trade" ? std::string("Trade with ") + npc.name
                              : action == "bank" ? std::string("Bank with ") + npc.name
                              : std::string("Examine ") + npc.name));
          put(entry, "action", JsonValue::Object{{"name", JsonValue(action)}, {"actionId", JsonValue(action_id)},
              {"context", JsonValue::Array{JsonValue("gameMap")}}, {"nearby", true}, {"weight", 1}});
          put(entry, "type", "npc");
          JsonValue::Object item_ref; put(item_ref, "id", npc.id); put(item_ref, "name", npc.name);
          put(entry, "item", std::move(item_ref));
          put(entry, "id", npc.id);
          entries.emplace_back(std::move(entry));
        }
      }
    }
    std::vector<const GroundItem*> matches;
    for (const auto& ground:world_->ground_items()) {
      if (static_cast<int>(std::floor(ground.x))==wx&&static_cast<int>(std::floor(ground.y))==wy) matches.push_back(&ground);
    }
    std::sort(matches.begin(),matches.end(),[](const GroundItem* a,const GroundItem* b){return a->timestamp>b->timestamp;});
    for (const auto* ground:matches) {
      JsonValue::Object entry;
      put(entry,"label","Take <span style='color:#e8d8a0'>"+ground->item.display_name+"</span>");
      put(entry,"action",JsonValue::Object{{"name","Take"},{"actionId","player:take"},{"context",JsonValue::Array{JsonValue("gameMap")}},{"allow",JsonValue::Array{JsonValue("item")}},{"nearby","edge"},{"weight",1},{"queueable",true}});
      put(entry,"type","item");
      put(entry,"at",JsonValue::Object{{"x",wx},{"y",wy}});
      put(entry,"id",ground->item.id);
      put(entry,"uuid",ground->item.uuid);
      put(entry,"timestamp",static_cast<double>(ground->timestamp));
      entries.emplace_back(std::move(entry));
    }
    JsonValue::Object walk;
    put(walk,"label","Walk here");
    put(walk,"action",JsonValue::Object{{"name","Walk here"},{"actionId","player:walk"},{"context",JsonValue::Array{JsonValue("gameMap")}},{"weight",2},{"queueable",true}});
    put(walk,"type","tile");
    put(walk,"at",JsonValue::Object{{"x",wx},{"y",wy}});
    entries.emplace_back(std::move(walk));
  } else if (clicked_has("inventorySlot")) {
    // Inventory variant: charted tablets are one-use expedition keys; vessel
    // items retain the existing brand-service entry.
    const int slot=as_int(misc?misc->get("slot"):nullptr,-1);
    const GameItem* item=nullptr;
    for (const auto& candidate:inventory_.items()) { if (candidate.slot==slot) { item=&candidate; break; } }
    if (item && item->expedition_map) {
      JsonValue::Object entry;
      put(entry, "label", campaign_complete_
                              ? "Break tablet and open expedition"
                              : "Campaign completion required");
      put(entry, "action", JsonValue::Object{
          {"name", "Open expedition"},
          {"actionId", "player:endgame:open-map"},
          {"context", JsonValue::Array{JsonValue("inventorySlot")}},
          {"nearby", false}, {"weight", 1}});
      put(entry, "type", "map");
      put(entry, "uuid", item->uuid);
      put(entry, "id", item->id);
      entries.emplace_back(std::move(entry));
    }
    if (item&&item->vessel&&world_->scene_id()=="town:verdigris") {
      const VesselItem& vi=item->vessel->item;
      const bool room=vi.vessel-static_cast<int>(vi.brands.size())-vi.scars>0;
      if (room&&vi.patience>=1) {
        JsonValue::Object entry;
        put(entry,"label","Add a random brand (100 coins)");
        put(entry,"action",JsonValue::Object{{"name","Add brand"},{"actionId","player:vesselforge:add-brand"},{"context",JsonValue::Array{JsonValue("inventorySlot")}},{"disallowWhile",JsonValue::Array{JsonValue("bank"),JsonValue("shop")}},{"allow",JsonValue::Array{JsonValue("item")}},{"nearby",false},{"weight",2}});
        put(entry,"type","item");
        put(entry,"miscData",misc?*misc:JsonValue(nullptr));
        put(entry,"uuid",item->uuid);
        put(entry,"id",item->id);
        entries.emplace_back(std::move(entry));
      }
    }
  }
  JsonValue::Object cancel;
  put(cancel,"label","Cancel");
  put(cancel,"action",JsonValue::Object{{"name","Cancel"},{"actionId","cancel"},{"weight",99}});
  put(cancel,"type","cancel");
  entries.emplace_back(std::move(cancel));
  emit(Envelope{"game:context-menu:items",JsonValue::Object{{"player",JsonValue::Object{{"socket_id",socket_id_}}},{"data",std::move(entries)}}});
}
void ProtocolSession::handle_menu_action(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) {
  // actions/index.js: dispatch on the echoed action's actionId.
  const auto* queue_item=payload.get("queueItem");
  const auto* action=queue_item?queue_item->get("action"):nullptr;
  const std::string action_id=as_string(action?action->get("actionId"):nullptr);
  // choose() forwards only {uuid,id} in queueItem.item; the FULL menu entry
  // (with our item payload: price, slot, qty) rides in data.item.
  const auto* data_wrap=payload.get("data");
  const auto* full_entry=data_wrap?data_wrap->get("item"):nullptr;
  const auto* entry_item=full_entry?full_entry->get("item"):nullptr;
  const auto* item_ref=entry_item?entry_item:(queue_item?queue_item->get("item"):nullptr);
  const std::string uuid=as_string(item_ref?item_ref->get("uuid"):nullptr);
  if (action_id=="player:take") { handle_take_ground(uuid,emit); return; }
  if (action_id=="player:endgame:open-map") {
    open_expedition_map(uuid, emit);
    return;
  }
  if (action_id=="player:screen:bank") {
    int npc_id = as_int(item_ref ? item_ref->get("id") : nullptr, 0);
    if (as_string(item_ref ? item_ref->get("id") : nullptr) == "bank") npc_id = 4;
    const TownNpc* npc = town_npc(npc_id);
    const Vec2 tile = tile_movement::occupied_tile(world_->position());
    if (!npc || npc_id != 4 || world_->in_instance() ||
        (std::max)(std::abs(tile.x - npc->x), std::abs(tile.y - npc->y)) > 1)
      return;
    bank_open_ = true; shop_open_ = false; emit_bank_screen(emit); return;
  }
  if (action_id=="world:road:chart") {
    const TownNpc* guide = town_npc(1);
    const Vec2 tile = tile_movement::occupied_tile(world_->position());
    if (!guide || world_->in_instance() ||
        (std::max)(std::abs(tile.x - guide->x), std::abs(tile.y - guide->y)) > 1)
      return;
    emit_chart_screen(as_string(item_ref ? item_ref->get("id") : nullptr,
                                "tin"), emit);
    return;
  }
  if (action_id=="house:investment:choose") {
    JsonValue::Object choice;
    put(choice, "choice", as_string(item_ref ? item_ref->get("id") : nullptr));
    handle_house_investment(JsonValue(std::move(choice)), emit);
    return;
  }
  if (action_id=="player:shop:buy") {
    const std::string item_id = as_string(item_ref ? item_ref->get("id") : nullptr);
    const int price = as_int(item_ref ? item_ref->get("price") : nullptr, item_id == "knife" ? 5 : 15);
    if (carried_gold() >= price) {
      int remaining = price;
      auto slots = inventory_.items();
      for (const auto& coin : slots) {
        if (remaining <= 0) break;
        if (coin.id != "coins") continue;
        GameItem taken;
        if (!inventory_.remove_by_uuid(coin.uuid, &taken)) continue;
        if (taken.qty > remaining) { GameItem back = taken; back.qty = taken.qty - remaining; inventory_.add(std::move(back)); remaining = 0; }
        else remaining -= taken.qty;
      }
      CreateItemOptions o; auto bought = create_game_item(item_id, o);
      if (bought) inventory_.add(std::move(*bought));
      emit_inventory_refresh(emit);
      emit_shop_screen(emit);
    }
    return;
  }
  if (action_id=="player:shop:sell") {
    GameItem sold;
    if (inventory_.remove_by_uuid(uuid, &sold)) {
      const int value = sold.id == "knife" ? 5 : sold.id == "bronze-sword" ? 15 : 8;
      CreateItemOptions o; o.quantity = value;
      auto coins = create_game_item("coins", o);
      if (coins) inventory_.add(std::move(*coins));
      emit_inventory_refresh(emit);
      emit_shop_screen(emit);
    }
    return;
  }
  if (action_id=="player:bank:withdraw") {
    const int qty = as_int(item_ref ? item_ref->get("qty") : nullptr, 1);
    for (std::size_t i = 0; i < bank_.size(); ++i) {
      if (bank_[i].uuid != uuid) continue;
      GameItem out = bank_[i];
      if (out.stackable && out.qty > qty) {
        bank_[i].qty -= qty;
        out.qty = qty;
      } else {
        bank_.erase(bank_.begin() + static_cast<long long>(i));
      }
      inventory_.add(std::move(out));
      emit_inventory_refresh(emit);
      emit_bank_screen(emit);
      break;
    }
    return;
  }  if (action_id=="player:bank:deposit") {
    const int qty = as_int(item_ref ? item_ref->get("qty") : nullptr, 1);
    GameItem taken;
    if (inventory_.remove_by_uuid(uuid, &taken)) {
      if (taken.stackable && taken.qty > qty) {
        GameItem back = taken; back.qty = taken.qty - qty;
        inventory_.add(std::move(back));
        taken.qty = qty;
      }
      bool merged = false;
      for (auto& existing : bank_) {
        if (existing.id == taken.id && existing.stackable) { existing.qty += taken.qty; merged = true; break; }
      }
      if (!merged) bank_.push_back(std::move(taken));
      emit_inventory_refresh(emit);
      emit_bank_screen(emit);
    }
    return;
  }  if (action_id=="player:screen:wagon") { emit_wagon_screen(emit); return; }
  if (action_id=="player:npc:trade") {
    int npc_id = as_int(item_ref ? item_ref->get("id") : nullptr, 0);
    const std::string service = as_string(item_ref ? item_ref->get("id") : nullptr);
    if (service == "weapons") npc_id = 2;
    if (service == "rite-vault") npc_id = 3;
    const TownNpc* npc = town_npc(npc_id);
    const Vec2 tile = tile_movement::occupied_tile(world_->position());
    if (!npc || world_->in_instance() ||
        (std::max)(std::abs(tile.x - npc->x), std::abs(tile.y - npc->y)) > 1)
      return;
    shop_npc_id_ = npc_id;
    shop_open_ = true; bank_open_ = false;
    emit_shop_screen(emit);
    return;
  }
  if (action_id=="player:screen:shop-display" ||
      action_id=="player:shop-display:buy" || action_id=="player:shop-display:appraise") {
    Envelope forwarded{action_id, payload};
    handle(forwarded, emit);
    return;
  }
  if (action_id=="player:npc:talk") { if (queue_item) handle_npc_talk(*queue_item, emit); return; }
  if (action_id=="player:npc:examine") {
    emit_npc_dialogue(as_int(item_ref ? item_ref->get("id") : nullptr, -1), emit);
    return;
  }
  if (action_id=="player:vesselforge:add-brand") {
    // vesselforge-brand.js: town service, 100 coins, sear on the live item;
    // a failed roll spends nothing (engine rolls on a clone internally).
    if (world_->scene_id()!="town:verdigris"||uuid.empty()) return;
    GameItem* item=inventory_.find_by_uuid(uuid);
    if (!item||!item->vessel) return;
    VesselItem rolled=item->vessel->item;
    if (!world_->forge().sear(rolled)) return;
    if (!inventory_.spend_coins(100)) { emit_message(emit,"You need 100 coins for the brand service."); return; }
    // spend_coins may rebuild the items vector; re-resolve the pointer.
    item=inventory_.find_by_uuid(uuid);
    if (!item||!item->vessel) return;
    item->vessel->item=rolled;
    // The JS handler refreshes the raw tooltip lines only; combat stats and
    // the display name stay stale until the next full refresh.
    item->vessel->lines=world_->forge().tooltip(rolled);
    emit_inventory_refresh(emit);
    emit_message(emit,"The forge sears a new brand into "+item->display_name+".");
    return;
  }
}
void ProtocolSession::handle_inventory_commit(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) {
  // player:inventory:commit world-drop: the production inventory drop verb.
  const std::string action=as_string(payload.get("action"));
  if (action!="world-drop") return;
  const auto* item_ref=payload.get("item");
  const std::string uuid=as_string(item_ref?item_ref->get("uuid"):nullptr);
  GameItem item;
  if (uuid.empty()||!inventory_.remove_by_uuid(uuid,&item)) return;
  item.slot=-1;
  const auto position=world_->position();
  world_->add_ground_item(std::move(item),position.x,position.y);
  emit_inventory_refresh(emit);
  emit_ground_change(emit);
}
void ProtocolSession::emit_combat_event(const WorldCombatEvent& event, const std::function<void(const Envelope&)>& emit) {
  if (event.type == "telegraph") {
    JsonValue::Object data; put(data,"attackerId",event.attacker_id); put(data,"attackerName",event.attacker_name); put(data,"skillId",event.skill_id);
    put(data,"x",event.x); put(data,"y",event.y); put(data,"radius",event.radius); put(data,"durationMs",event.duration_ms);
    emit_world(Envelope{"monster:telegraph",JsonValue(std::move(data))},emit); return;
  }
  // N4: kill rewards go through world_->drop_monster_loot inside
  // advance_combat; the legacy synthetic 'drop' trophy event is retired.
  JsonValue::Object data; put(data,"attackerId",event.attacker_id); put(data,"attackerName",event.attacker_name); put(data,"targetId",event.target_id);
  put(data,"targetName",event.target_name); put(data,"targetType",event.target_id==identity_?"player":"monster"); put(data,"skillId",event.skill_id.empty()?"primary-attack":event.skill_id);
  put(data,"amount",event.amount); put(data,"died",event.died); put(data,"health",JsonValue::Object{{"current",event.health},{"max",event.health_max}});
  // combat/index.js hit parity fields.
  put(data,"baseAmount",event.base_amount); put(data,"beastbaneAmount",event.beastbane_amount);
  put(data,"beastbanePercent",event.beastbane_percent); put(data,"beastbane",event.beastbane);
  put(data,"critical",event.critical); put(data,"attackStyle",event.attack_style);
  emit_world(Envelope{"combat:hit",JsonValue(std::move(data))},emit);
}

void ProtocolSession::emit_combat_state(
    const std::function<void(const Envelope&)>& emit) const {
  const auto* actor = simulation_->actor(simulation_->scion().actor_id);
  const int cooldown_ms = world_->player_cooldown_remaining_ms(combat_clock_ms_);
  const int cooldown_ticks =
      (cooldown_ms + kSimulationTickMs - 1) / kSimulationTickMs;
  const int war_cry_ticks = war_cry_until_ms_ > combat_clock_ms_
      ? static_cast<int>((war_cry_until_ms_ - combat_clock_ms_ +
                          kSimulationTickMs - 1) / kSimulationTickMs)
      : 0;
  JsonValue::Object data;
  put(data, "resource", actor ? actor->stats.resource : 0);
  put(data, "resourceMax", actor ? actor->stats.resource_max : 0);
  put(data, "cooldownTicks", cooldown_ticks);
  put(data, "warCryTicksRemaining", war_cry_ticks);
  emit(Envelope{"player:combat-state", JsonValue(std::move(data))});
}

void ProtocolSession::refresh_combat_state(
    std::int64_t now_ms_value, const std::function<void(const Envelope&)>& emit) {
  combat_clock_ms_ = (std::max)(combat_clock_ms_, now_ms_value);
  auto* actor = simulation_->actor(simulation_->scion().actor_id);
  if (!actor) return;
  if (resource_regen_at_ms_ == 0) resource_regen_at_ms_ = combat_clock_ms_;
  const std::int64_t elapsed = combat_clock_ms_ - resource_regen_at_ms_;
  if (elapsed >= kSimulationTickMs) {
    const std::int64_t ticks = elapsed / kSimulationTickMs;
    const std::int64_t restored = ticks *
        presentation_constants::kResourceRegenPerTick;
    actor->stats.resource = (std::min)(
        actor->stats.resource_max,
        actor->stats.resource + static_cast<int>((std::min<std::int64_t>)(
            restored, actor->stats.resource_max)));
    resource_regen_at_ms_ += ticks * kSimulationTickMs;
  }
  if (war_cry_attack_bonus_ > 0 && combat_clock_ms_ >= war_cry_until_ms_) {
    war_cry_attack_bonus_ = 0;
    war_cry_until_ms_ = 0;
    emit(Envelope{"player:skill:effect",
                  JsonValue::Object{{"skillId", "war-cry"},
                                    {"active", false}}});
  }
}

void ProtocolSession::process_combat(std::int64_t now, const std::function<void(const Envelope&)>& emit) {
  auto* actor = simulation_->actor(simulation_->scion().actor_id); if (!actor) return;
  refresh_combat_state(now, emit);
  const int life_before = actor->stats.life;
  world_->set_guaranteed_elite_gear(active_quest_ == 1 && quest_objective_ == 0);
  const auto combat_totals = wear_.totals();
  {
    PlayerCombatMods active_mods = world_->player_combat_mods();
    active_mods.goods_found = combat_totals.modifiers.goods_found +
                              (endgame_active_ ? endgame_goods_found_percent_ : 0);
    world_->set_player_combat_mods(active_mods);
  }
  const int wear_attack = (std::max)(0, (std::max)((std::max)(combat_totals.attack.stab, combat_totals.attack.slash),
                                                   (std::max)(combat_totals.attack.crush, combat_totals.attack.range)));
  // combat/index.js rollPlayerDamage: melee = 2 + STR*0.45 + weapon*1.5;
  // mana skills = 4 + INT*0.5 (no weapon term). Tree attributes feed both.
  int str_attr = 10, dex_attr = 10, int_attr = 10;
  tree_attributes(&str_attr, &dex_attr, &int_attr);
  const bool mana_skill = active_skill_id_.rfind("ability", 0) == 0;
  const int player_power = (std::max)(
      1, static_cast<int>(std::lround(mana_skill
          ? 4.0 + int_attr * 0.5
          : 2.0 + str_attr * 0.45 + wear_attack * 1.5)) +
             war_cry_attack_bonus_);
  const bool engaged_here = world_->engaged_by().empty() || world_->engaged_by() == identity_;
  const auto events = world_->advance_combat(actor->stats.level, engaged_here ? player_power : 0, actor->stats.life, actor->stats.life_max, now);
  // N5 respawn ward: monsters cannot damage a freshly-respawned scion until
  // the scion acts. Absorb monster damage here (the player still lands hits);
  // the skill handler ends the ward.
  if (respawn_protection_until_ms_ > now && actor->stats.life < life_before) {
    actor->stats.life = life_before;
  }
  bool loot = false;
  for (const auto& event : events) {
    emit_combat_event(event, emit);
    if (event.died && event.target_id != identity_) loot = true;
    if (event.type == "hit" && event.target_id != identity_) quest_trigger("attack", emit);
    if (event.type == "death" && event.target_id != identity_) {
      quest_trigger("slay", emit);
      for (const auto& monster : world_->monsters()) {
        if (monster.uuid != event.target_id) continue;
        if (monster.rarity == "elite") quest_trigger("slay-elite", emit, monster.name, world_->metadata().theme);
        break;
      }
    }
    if (event.type == "death" && event.target_id != identity_) {
      emit_message(emit, "You have slain " + event.target_name + ".");
      // experience.js: kills grant combat XP; the character level derives
      // from the shared curve. Level-ups refresh and refill resources.
      {
        int monster_level = 1;
        for (const auto& monster : world_->monsters())
          if (monster.uuid == event.target_id) { monster_level = (std::max)(1, monster.level); break; }
        combat_xp_ += static_cast<long long>(monster_level) * 12;
        const int derived = level_from_xp(combat_xp_);
        if (derived > actor->stats.level) {
          actor->stats.level = derived;
          actor->stats.attack = 12 + derived * 3;
          actor->stats.life_max = 100 + derived * 10;
          actor->stats.life = actor->stats.life_max;
          world_->set_level(derived);
          emit_message(emit, "You are now level " + std::to_string(derived) + "!");
        }
      }
      // world-web: the node Warden falls - dead stays dead, the road opens.
      if (!current_node_id_.empty()) {
        for (const auto& monster : world_->monsters()) {
          if (monster.uuid != event.target_id || !monster.boss) continue;
          cleared_nodes_.insert(current_node_id_);
          world_->set_block_stairs_down(false);
          emit_message(emit, "The " + std::string("Warden of ") + current_node_name_ + " is down. The road runs on.");
          break;
        }
      }
      if (endgame_active_ && !endgame_completed_) {
        for (const auto& monster : world_->monsters()) {
          if (monster.uuid != event.target_id || !monster.boss) continue;
          endgame_completed_ = true;
          endgame_maps_completed_ += 1;
          if (JsonValue::Object* house =
                  find_chronicle_house_object(chronicle_, active_house_id_)) {
            (*house)["endgameMapsCompleted"] = JsonValue(endgame_maps_completed_);
            chronicles_revision_ += 1;
          }
          const int next_tier = (std::min)(
              16, endgame_map_tier_ + (session_rng_.next() < 0.35 ? 1 : 0));
          award_expedition_map(next_tier, monster.x, monster.y, emit, false);
          emit_message(emit,
                       "The Seal-Bound Warden is broken. Claim the next "
                       "tablet and return by the entry waymark.");
          break;
        }
      }
      // first-goal.js notifyFirstGoalWardenDown: any tier-1 (depth-1) boss.
      if (first_goal_stage_ == "clear-floor" && world_->metadata().depth <= 1) {
        for (const auto& monster : world_->monsters()) {
          if (monster.uuid == event.target_id && monster.boss) {
            first_goal_stage_ = "return-to-town";
            emit_message(emit, "Your first Warden is down. Return to Aldwyn at the Crossroads for your reward.");
            emit_quest_update(emit);
            break;
          }
        }
      }
      // Relic circulation (D-106): an elite slain by a living scion returns
      // one circulating House heirloom to the floor where it fell.
      {
        for (const auto& monster : world_->monsters()) {
          if (monster.uuid != event.target_id || monster.rarity != "elite") continue;
          CirculatingRelic relic; bool found = false;
          { std::lock_guard<std::mutex> pool_lock(circulation_mutex());
            auto& pool = circulation_pool();
            for (std::size_t i2 = 0; i2 < pool.size() && !found; ++i2)
              if (pool[i2].account == identity_) { relic = pool[i2]; pool.erase(pool.begin() + static_cast<long long>(i2)); found = true; }
          }
          if (found) {
            static std::atomic<std::uint64_t> kill_relic_serial{1};
            const std::string relic_id = "relic-" + std::to_string(kill_relic_serial++);
            // Own-account recovery keeps the House binding (mortality flow);
            // only a relic circulated to a stranger surfaces unbound.
            if (relic.item.bound_to != identity_) relic.item.bound_to.clear();
            world_->add_relic_ground_item(std::move(relic.item), monster.x, monster.y, relic_id,
                                          relic.scion_id, relic.scion_name);
            emit_message(emit, "A relic of the fallen has surfaced.");
          }
          break;
        }
      }
    }
  }
  if (loot) emit_ground_change(emit);
  maybe_floor_cleared(emit);
  // A mortal scion's lethal wound is final: commit to the crypt (D-106).
  if (actor->stats.life <= 0 && (prepare_final_death_ || mortal_oath_) && lifecycle_ != "permadead") {
    handle_final_death(emit);
  }
}
void ProtocolSession::handle_extract(const std::function<void(const Envelope&)>& emit) {
  if (!world_->in_instance()) {
    emit_message(emit, "There is no extraction here.");
    return;
  }
  world_->return_to_surface();
  emit_movement(emit);
  emit_message(emit, "The party returns to the surface.");
  finish_extraction(emit);
  maybe_complete_first_goal(emit); quest_trigger("return-surface", emit, zone_id_for_instance(last_instance_theme_, last_instance_layout_), last_instance_theme_);
  emit_transition(emit, "party:scene:transition");
}
void ProtocolSession::handle_final_death(const std::function<void(const Envelope&)>& emit) {
  auto* actor = simulation_->actor(simulation_->scion().actor_id);
  lifecycle_ = "permadead";
  lifecycle_mode_ = "hard";
  prepare_final_death_ = false;
  // D-106: carried value is never destroyed — capture it into circulation.
  pending_relic_items_.clear();
  // "Notable gear was committed to circulation" - the starter kit is not
  // notable; only earned gear circulates (chronicles scenario contract).
  for (const auto& item : inventory_.items()) if (item.id != "coins" && item.id != "bronze-dagger") pending_relic_items_.push_back(item);
  for (const auto& [seat, item] : wear_.slots()) (void)seat, pending_relic_items_.push_back(item);
  int relic_count = static_cast<int>(pending_relic_items_.size());
  { std::lock_guard<std::mutex> pool_lock(circulation_mutex());
    for (const auto& item : pending_relic_items_)
      circulation_pool().push_back({item, active_scion_id_, active_scion_name_, identity_});
  }
  pending_relic_count_ = relic_count;
  relic_source_scion_name_ = active_scion_name_;
  relic_source_scion_id_ = active_scion_id_;
  // Move the fallen scion from the living roster into the crypt.
  if (auto* root = chronicle_.object()) {
    auto houses_it = root->find("houses");
    if (houses_it != root->end() && houses_it->second.array()) {
      for (auto& house_entry : *houses_it->second.array()) {
        auto* house = house_entry.object();
        if (!house) continue;
        auto id_it = house->find("id");
        if (id_it == house->end() || !id_it->second.string() || *id_it->second.string() != active_house_id_) continue;
        auto scions_it = house->find("scions");
        if (scions_it != house->end() && scions_it->second.array()) {
          auto* scions = scions_it->second.array();
          JsonValue::Array kept;
          JsonValue::Object fallen_entry;
          for (auto& scion_entry : *scions) {
            auto* scion = scion_entry.object();
            const bool is_active = scion && scion->find("id") != scion->end()
              && scion->find("id")->second.string() && *scion->find("id")->second.string() == active_scion_id_;
            if (is_active) { fallen_entry = *scion; }
            else kept.emplace_back(std::move(scion_entry));
          }
          *scions = std::move(kept);
          // Relic record: the heirlooms circulate until a successor recovers
          // them (mortality scenario: crypt[].relic.status lost->recovered).
          fallen_entry["relic"] = JsonValue(JsonValue::Object{
              {"status", JsonValue("lost")},
              {"count", JsonValue(relic_count)}});
          auto crypt_it = house->find("crypt");
          if (crypt_it == house->end()) { (*house)["crypt"] = JsonValue(JsonValue::Array{}); crypt_it = house->find("crypt"); }
          crypt_it->second.array()->emplace_back(std::move(fallen_entry));
        }
        break;
      }
    }
  }
  JsonValue::Object fallen;
  put(fallen, "scionId", active_scion_id_.empty() ? JsonValue(nullptr) : JsonValue(active_scion_id_));
  put(fallen, "name", active_scion_name_);
  put(fallen, "level", actor ? actor->stats.level : 1);
  JsonValue::Object data;
  put(data, "fallen", JsonValue(std::move(fallen)));
  put(data, "relicCount", relic_count);
  put(data, "chronicle", chronicle_);
  emit(Envelope{"chronicles:scion-fallen", JsonValue(std::move(data))});
  if (broadcast_) {
    JsonValue::Object witnessed_fallen;
    put(witnessed_fallen, "name", active_scion_name_.empty() ? (username_.empty() ? identity_ : username_) : active_scion_name_);
    JsonValue::Object witnessed;
    put(witnessed, "fallen", std::move(witnessed_fallen));
    put(witnessed, "relicCount", relic_count);
    broadcast_(Envelope{"chronicles:scion-witnessed", JsonValue(std::move(witnessed))});
  }
  // lifecycle.js broadcastStats: the mortal death is authoritative state.
  JsonValue::Object stats;
  put(stats, "playerId", identity_);
  JsonValue::Object lc;
  put(lc, "state", "permadead");
  put(lc, "mode", "hard");
  put(stats, "lifecycle", std::move(lc));
  emit(Envelope{"player:stats:update", JsonValue(std::move(stats))});
}
void ProtocolSession::maybe_respawn(std::int64_t now_ms) {
  if (lifecycle_ != "awaiting-respawn" || now_ms < respawn_at_ms_) return;
  // Soft respawn (lifecycle.js): back to the instance entry spawn, full life,
  // with a short ward against the surrounding pack.
  auto* actor = simulation_->actor(simulation_->scion().actor_id);
  const auto& spawns = world_->metadata().spawn_points;
  const Vec2 spawn = spawns.empty() ? Vec2{38, 115} : spawns[0];
  world_->teleport(spawn.x, spawn.y, now_ms);
  if (actor) {
    actor->alive = true;
    actor->stats.life = actor->stats.life_max;
  }
  lifecycle_ = "alive";
  respawn_at_ms_ = 0;
  respawn_protection_until_ms_ = now_ms + 8000;
}
JsonValue ProtocolSession::chronicles_payload() const {
  JsonValue::Object out;
  put(out, "chroniclesAccountId", identity_);
  put(out, "accountName", identity_);
  auto* actor = simulation_->actor(simulation_->scion().actor_id);
  put(out, "level", actor ? actor->stats.level : 1);
  put(out, "chronicles", chronicle_);
  put(out, "chroniclesRevision", chronicles_revision_);
  put(out, "chroniclesExists", chronicles_revision_ > 0);
  return JsonValue(std::move(out));
}
JsonValue ProtocolSession::chronicles_state_payload(const std::string& created_scion_id) const {
  JsonValue::Object out;
  put(out, "player", JsonValue::Object{{"socket_id", socket_id_}});
  put(out, "chronicle", chronicle_);
  if (!created_scion_id.empty()) put(out, "createdScionId", created_scion_id);
  return JsonValue(std::move(out));
}
void ProtocolSession::ensure_chronicle_house(const std::string& id, const std::string& name) {
  JsonValue::Object* root = chronicle_.object();
  if (!root) {
    JsonValue::Object fresh;
    put(fresh, "version", 3);
    put(fresh, "houses", JsonValue::Array{});
    put(fresh, "activeHouseId", id);
    put(fresh, "activeScionId", JsonValue(nullptr));
    chronicle_ = JsonValue(std::move(fresh));
    root = chronicle_.object();
  }
  JsonValue::Array* houses = nullptr;
  auto houses_it = root->find("houses");
  if (houses_it == root->end()) {
    (*root)["houses"] = JsonValue(JsonValue::Array{});
    houses_it = root->find("houses");
  }
  houses = houses_it->second.array();
  for (const auto& entry : *houses) {
    const auto* obj = entry.object();
    if (obj && obj->find("id") != obj->end() && obj->find("id")->second.string()
        && *obj->find("id")->second.string() == id) return;
  }
  JsonValue::Object house;
  put(house, "id", id);
  put(house, "name", name);
  put(house, "scions", JsonValue::Array{});
  put(house, "crypt", JsonValue::Array{});
  put(house, "campaignComplete", false);
  put(house, "endgameMapsCompleted", 0);
  put(house, "treasury", 0);
  put(house, "renown", 0);
  JsonValue::Object investment;
  put(investment, "firstClearCompleted", false);
  put(investment, "choice", "unchosen");
  put(investment, "rewardClaimed", false);
  put(investment, "scionGearTier", 0);
  put(investment, "houseIncomePerClear", 0);
  put(house, "firstInvestment", std::move(investment));
  houses->emplace_back(std::move(house));
  (*root)["activeHouseId"] = JsonValue(id);
}
void ProtocolSession::ensure_chronicle_scion(const std::string& house_id, const std::string& id,
                                             const std::string& name, bool mortal) {
  JsonValue::Object* root = chronicle_.object();
  if (!root) return;
  auto houses_it = root->find("houses");
  if (houses_it == root->end() || !houses_it->second.array()) return;
  for (auto& entry : *houses_it->second.array()) {
    auto* obj = entry.object();
    if (!obj) continue;
    auto house_id_it = obj->find("id");
    if (house_id_it == obj->end() || !house_id_it->second.string()
        || *house_id_it->second.string() != house_id) continue;
    auto scions_it = obj->find("scions");
    if (scions_it == obj->end()) { (*obj)["scions"] = JsonValue(JsonValue::Array{}); scions_it = obj->find("scions"); }
    for (const auto& scion : *scions_it->second.array()) {
      const auto* so = scion.object();
      if (so && so->find("id") != so->end() && so->find("id")->second.string()
          && *so->find("id")->second.string() == id) return;
    }
    JsonValue::Object scion;
    put(scion, "id", id);
    put(scion, "name", name);
    put(scion, "level", 1);
    put(scion, "mortal", mortal);
    put(scion, "deeds", JsonValue::Array{});
    scions_it->second.array()->emplace_back(std::move(scion));
    return;
  }
}
void ProtocolSession::emit_login(const std::function<void(const Envelope&)>& emit) const { Envelope response{"player:login",JsonValue::Object{}}; parse_json(login_payload(),response.data); emit(response); }
void ProtocolSession::emit_world(const Envelope& envelope, const std::function<void(const Envelope&)>& emit) const { if (broadcast_) broadcast_(envelope); else emit(envelope); }
void ProtocolSession::emit_transition(const std::function<void(const Envelope&)>& emit, const char* event) const { JsonValue::Object data; put(data,"player",JsonValue::Object{{"socket_id",socket_id_}}); put(data,"scene",scene_payload()); JsonValue player_state; parse_json(player_payload(),player_state); JsonValue::Object state_fields; if (const auto* fields=player_state.object()) { for (const auto& key:{"uuid","x","y","sceneId"}) if (const auto* field=player_state.get(key)) put(state_fields,key,*field); } put(data,"playerState",std::move(state_fields)); emit_world(Envelope{event,JsonValue(std::move(data))},emit); }
void ProtocolSession::emit_movement(const std::function<void(const Envelope&)>& emit) const { JsonValue data; parse_json(player_payload(),data); Envelope movement{"player:movement",std::move(data)}; movement.meta=movement_step_payload(); emit_world(movement,emit); }
void ProtocolSession::emit_message(const std::function<void(const Envelope&)>& emit, const std::string& text) const { emit(Envelope{"game:send:message",JsonValue::Object{{"text",text}}}); }
void ProtocolSession::handle(const Envelope& envelope, const std::function<void(const Envelope&)>& emit) {
  // The server tick thread and the socket handler share the session; one
  // lock serialises world/inventory/chronicle access (mirrors the JS
  // single-threaded loop). Helper methods that also lock keep their guards
  // for direct-call paths; std::recursive_mutex makes both safe.
  std::lock_guard<std::recursive_mutex> handle_lock(mutex_);
  const auto* payload=envelope.data.object()?&envelope.data:nullptr;
  if (envelope.event=="world:zone:enter") { const auto node=as_string(payload?payload->get("nodeId"):nullptr,"tin:1:0"); simulation_->dispatch(Command::enter(node.rfind("route:",0)==0?node:"route:"+node)); { std::string web_road; int web_tier=0; int web_index=0; if (parse_node_id(node,&web_road,&web_tier,&web_index)) { enter_road_node(node, emit); return; } } current_node_id_.clear(); world_->set_block_stairs_down(false); world_->set_stairs_up_returns_to_town(false); world_->enter_solo_instance("dungeon",""); emit_transition(emit,"world:scene:transition"); emit_ground_change(emit); last_instance_theme_ = world_->metadata().theme; last_instance_layout_ = world_->metadata().layout; quest_trigger("delve", emit, zone_id_for_instance(world_->metadata().theme, world_->metadata().layout), world_->metadata().theme, world_->metadata().depth); return; }
  if (envelope.event=="instance:enterSolo") { current_node_id_.clear(); world_->set_block_stairs_down(false); world_->set_stairs_up_returns_to_town(false); world_->enter_solo_instance(as_string(payload?payload->get("template"):nullptr,"dungeon"),as_string(payload?payload->get("layout"):nullptr,"")); emit_transition(emit,"party:scene:transition"); emit_ground_change(emit); last_instance_theme_ = world_->metadata().theme; last_instance_layout_ = world_->metadata().layout; quest_trigger("delve", emit, zone_id_for_instance(world_->metadata().theme, world_->metadata().layout), world_->metadata().theme, world_->metadata().depth); return; }
  if (envelope.event=="player:move") { const auto direction=as_string(payload?payload->get("direction"):nullptr); const bool was_instance=world_->in_instance(); const int depth_before=world_->metadata().depth; const std::string scene_before=world_->scene_id(); if (world_->apply_movement_sample(direction,now_ms())) { emit_movement(emit); auto_pickup_gold(emit); quest_trigger("move", emit); check_road_gates(emit); const bool depth_changed=world_->in_instance()&&world_->metadata().depth!=depth_before; const bool scene_changed=world_->scene_id()!=scene_before; if (depth_changed||scene_changed) { if (was_instance&&!world_->in_instance()) { emit_message(emit,"The party returns to the surface."); finish_extraction(emit); maybe_complete_first_goal(emit); quest_trigger("return-surface", emit, zone_id_for_instance(last_instance_theme_, last_instance_layout_), last_instance_theme_); } if (depth_changed) quest_trigger("delve", emit, zone_id_for_instance(world_->metadata().theme, world_->metadata().layout), world_->metadata().theme, world_->metadata().depth); if (depth_changed && !current_node_id_.empty() && !current_child_id_.empty()) { current_node_id_ = current_child_id_; current_node_tier_ += 1; current_node_name_ = current_child_name_.empty() ? current_node_name_ : current_child_name_; current_child_id_.clear(); world_->set_block_stairs_down(true); } emit_transition(emit,"party:scene:transition"); if (world_->in_instance()) emit_ground_change(emit); } } return; }
  if (envelope.event=="dev:teleport") { if (!payload) return; const auto* x=payload->get("x"); const auto* y=payload->get("y"); if (!x||!x->number()||!y||!y->number()) return; const int tx=static_cast<int>(*x->number()); const int ty=static_cast<int>(*y->number()); const bool was_instance=world_->in_instance(); const int depth_before=world_->metadata().depth; const std::string scene_before=world_->scene_id(); world_->teleport(tx,ty,now_ms()); const bool returned=was_instance&&!world_->in_instance(); const bool depth_changed=world_->in_instance()&&world_->metadata().depth!=depth_before; const bool transitioned=returned||depth_changed||world_->scene_id()!=scene_before; emit_movement(emit); emit_message(emit,"Teleported to "+std::to_string(tx)+", "+std::to_string(ty)+(transitioned?" (portal followed).":".")); check_road_gates(emit); if (returned) { emit_message(emit,"The party returns to the surface."); finish_extraction(emit); maybe_complete_first_goal(emit); quest_trigger("return-surface", emit, zone_id_for_instance(last_instance_theme_, last_instance_layout_), last_instance_theme_); } if (transitioned) emit_transition(emit,"party:scene:transition"); if (world_->in_instance()) { if (depth_changed) { emit_ground_change(emit); quest_trigger("delve", emit, zone_id_for_instance(world_->metadata().theme, world_->metadata().layout), world_->metadata().theme, world_->metadata().depth); if (!current_node_id_.empty() && !current_child_id_.empty()) { current_node_id_ = current_child_id_; current_node_tier_ += 1; if (!current_child_name_.empty()) { current_node_name_ = current_child_name_; world_->set_scene_name(current_node_name_); } current_child_id_.clear(); world_->set_block_stairs_down(true); } } process_combat(now_ms(),emit); } return; }
  if (envelope.event=="dev:setlevel") { auto* actor=simulation_->actor(simulation_->scion().actor_id); const int level=as_int(payload?payload->get("level"):nullptr,1); if(actor){ actor->stats.level=(std::max)(1,level); actor->stats.attack=12+actor->stats.level*3; actor->stats.life_max=100+actor->stats.level*10; actor->stats.life=actor->stats.life_max; world_->set_level(actor->stats.level); } return; }
  if (envelope.event=="player:screen:wagon") { emit_wagon_screen(emit); return; }
  if (envelope.event=="world:road:chart") { emit_chart_screen(as_string(payload?payload->get("roadId"):nullptr,"tin"), emit); return; }
  if (envelope.event=="wagon:outfit:buy") {
    const std::string item_id = as_string(payload ? payload->get("itemId") : nullptr);
    int price = item_id == "bronze-sword" ? 15 : item_id == "bronze-dagger" ? 10 : item_id == "wooden-shield" ? 8 : -1;
    if (price > 0 && house_treasury_ >= price) {
      house_treasury_ -= price;
      persist_house_progression();
      CreateItemOptions o;
      auto bought = create_game_item(item_id, o);
      if (bought) inventory_.add(std::move(*bought));
      emit_message(emit, "The quartermaster hands down the " + item_id + "; the ledger pays " + std::to_string(price) + " gold.");
      emit_inventory_refresh(emit);
      emit_wagon_screen(emit);
    }
    return;
  }  if (envelope.event=="player:screen:shop-display") {
    shop_open_ = true; bank_open_ = false;
    emit_shop_screen(emit);
    return;
  }
  if (envelope.event=="player:npc:trade") {
    const auto* item = payload ? payload->get("item") : nullptr;
    const int npc_id = as_int(item ? item->get("id") : nullptr, -1);
    const TownNpc* npc = town_npc(npc_id);
    const Vec2 tile = tile_movement::occupied_tile(world_->position());
    if (npc && !world_->in_instance() &&
        (std::max)(std::abs(tile.x - npc->x), std::abs(tile.y - npc->y)) <= 1) {
      shop_npc_id_ = npc_id;
      shop_open_ = true; bank_open_ = false;
      emit_shop_screen(emit);
    }
    return;
  }
  if (envelope.event=="player:shop-display:appraise") {
    emit_message(emit,"Bronze Sword: 15 coins.");
    return;
  }
  if (envelope.event=="player:shop-display:buy") {
    if (carried_gold()>=15) {
      JsonValue::Object spend; 
      // deduct 15 coins then grant the sword.
      int remaining=15;
      auto slots=inventory_.items();
      for (const auto& item:slots) {
        if (remaining<=0) break;
        if (item.id!="coins") continue;
        GameItem taken;
        if (!inventory_.remove_by_uuid(item.uuid,&taken)) continue;
        if (taken.qty>remaining) { GameItem back=taken; back.qty=taken.qty-remaining; inventory_.add(std::move(back)); remaining=0; }
        else remaining-=taken.qty;
      }
      CreateItemOptions o; auto sword=create_game_item("bronze-sword",o);
      if (sword) inventory_.add(std::move(*sword));
      emit_message(emit,"You buy the Bronze Sword for 15 coins.");
      emit_inventory_refresh(emit);
    } else {
      emit_message(emit,"You cannot afford that.");
    }
    return;
  }
  if (envelope.event=="player:fountain:drink") {
    auto* actor=simulation_->actor(simulation_->scion().actor_id);
    const Vec2 tile=tile_movement::occupied_tile(world_->position());
    if (actor && !world_->in_instance() && (std::max)(std::abs(tile.x-38),std::abs(tile.y-115))<=1) {
      actor->stats.life=actor->stats.life_max;
      world_->heal_player(actor->stats.life,actor->stats.life_max);
      emit_message(emit,"Cool water. The road ahead feels lighter.");
    }
    return;
  }
  if (envelope.event=="dev:hurt") {
    auto* actor=simulation_->actor(simulation_->scion().actor_id);
    const int amount=as_int(payload?payload->get("amount"):nullptr,5);
    if (actor) actor->stats.life=(std::max)(1,actor->stats.life-amount);
    return;
  }
  if (envelope.event=="dev:heal") { auto* actor=simulation_->actor(simulation_->scion().actor_id); if(actor) world_->heal_player(actor->stats.life,actor->stats.life_max); return; }
  if (envelope.event=="dev:kill") {
    auto* actor=simulation_->actor(simulation_->scion().actor_id);
    if (actor) actor->stats.life = 0;
    if (mortal_oath_ || lifecycle_mode_ == "hard") {
      // Hard lifecycle: a mortal scion's lethal blow is final (permadead).
      if (lifecycle_ != "permadead") handle_final_death(emit);
      emit_message(emit,"Mortal lifecycle advanced to final death.");
    } else if (lifecycle_ == "alive") {
      // Soft death (lifecycle.js): enter awaiting-respawn once.
      lifecycle_ = "awaiting-respawn";
      lifecycle_deaths_ += 1;
      respawn_at_ms_ = now_ms() + 2000;
      respawn_protection_until_ms_ = 0;
      emit_message(emit,"You have been slain.");
    }
    return;
  }
  if (envelope.event=="dev:prepare-final-death") {
    // dev.js dev:prepare-final-death: one real monster hit away from the
    // crypt (hard lifecycle, hp -> 1).
    prepare_final_death_=true;
    mortal_oath_=true;
    lifecycle_mode_="hard";
    auto* pd_actor=simulation_->actor(simulation_->scion().actor_id);
    if (pd_actor) pd_actor->stats.life = 1;
    emit_message(emit,"Final death armed; the next damaging monster hit is fatal.");
    return;
  }
  if (envelope.event=="dev:release-relic") {
    // dev.js dev:release-relic: surface the next circulating heirloom - own
    // account first, then any House the world remembers.
    CirculatingRelic relic; bool found = false;
    { std::lock_guard<std::mutex> pool_lock(circulation_mutex());
      auto& pool = circulation_pool();
      for (std::size_t i2 = 0; i2 < pool.size() && !found; ++i2)
        if (pool[i2].account == identity_) { relic = pool[i2]; pool.erase(pool.begin() + static_cast<long long>(i2)); found = true; }
      if (!found && !pool.empty()) { relic = pool.front(); pool.erase(pool.begin()); found = true; }
    }
    if (found) {
      const auto position = world_->position();
      static std::atomic<std::uint64_t> relic_serial{1};
      const std::string relic_id = "relic-" + std::to_string(relic_serial++);
      // Own-account releases keep the binding; cross-account circulation
      // surfaces unbound so any finder can claim it.
      if (relic.item.bound_to != identity_) relic.item.bound_to.clear();
      world_->add_relic_ground_item(std::move(relic.item), position.x, position.y, relic_id,
                                    relic.scion_id, relic.scion_name);
      emit_message(emit, "A relic of the fallen has surfaced.");
      emit_ground_change(emit);
    }
    return;
  }
  if (envelope.event=="player:skill:trigger") {
    auto* actor=simulation_->actor(simulation_->scion().actor_id);
    if (actor&&world_->in_instance()) {
      const std::int64_t action_now=(std::max)(combat_clock_ms_,now_ms());
      refresh_combat_state(action_now,emit);
      // Native clients send `skill`; browser/legacy scenarios send
      // `skillId`. Both names resolve through one canonical vocabulary.
      active_skill_id_=as_string(payload?payload->get("skillId"):nullptr);
      if(active_skill_id_.empty())
        active_skill_id_=as_string(payload?payload->get("skill"):nullptr,"melee");
      if(active_skill_id_=="primary-attack") active_skill_id_="melee";
      const auto direction=as_string(payload?payload->get("direction"):nullptr,"down");

      if(active_skill_id_=="dash") {
        if(world_->apply_dash(direction,action_now)) {
          if(respawn_protection_until_ms_>0) respawn_protection_until_ms_=0;
          emit_movement(emit);
        }
        process_combat(action_now,emit);
        emit_combat_state(emit);
        return;
      }
      if(active_skill_id_=="war-cry") {
        if(actor->stats.resource>=presentation_constants::kWarCryResourceCost) {
          actor->stats.resource-=presentation_constants::kWarCryResourceCost;
          war_cry_attack_bonus_=presentation_constants::kWarCryAttackBonus;
          war_cry_until_ms_=action_now+
              presentation_constants::kWarCryDurationTicks*kSimulationTickMs;
          if(respawn_protection_until_ms_>0) respawn_protection_until_ms_=0;
          emit(Envelope{"player:skill:effect",
                        JsonValue::Object{
                            {"skillId","war-cry"},{"active",true},
                            {"attackBonus",war_cry_attack_bonus_},
                            {"durationMs",presentation_constants::kWarCryDurationTicks*
                                              kSimulationTickMs}}});
        }
        process_combat(action_now,emit);
        emit_combat_state(emit);
        return;
      }

      const int resource_cost=active_skill_id_=="thrust"
          ? presentation_constants::kThrustResourceCost
          : active_skill_id_=="sweep"
              ? presentation_constants::kSweepResourceCost : 0;
      const bool known=active_skill_id_=="melee"||active_skill_id_=="thrust"||
                       active_skill_id_=="sweep";
      const auto wear_totals=wear_.totals();
      const int wear_bonus=(std::max)(
          (std::max)(wear_totals.attack.stab,wear_totals.attack.slash),
          (std::max)(wear_totals.attack.crush,wear_totals.attack.range));
      const bool accepted=known&&actor->stats.resource>=resource_cost&&
          world_->start_player_attack(actor->stats.level,
              actor->stats.attack+(std::max)(0,wear_bonus),action_now,direction,
              active_skill_id_);
      if(accepted) {
        actor->stats.resource-=resource_cost;
        if(respawn_protection_until_ms_>0) respawn_protection_until_ms_=0;
        world_->set_engaged_by(identity_);
      }
      process_combat(action_now,emit);
      emit_combat_state(emit);
    }
    return;
  }
  if (envelope.event=="dev:give") { if (payload) handle_give(*payload,emit); return; }
  if (envelope.event=="dev:drop") { if (payload) handle_drop(*payload,emit); return; }
  if (envelope.event=="dev:forcecritical") { world_->player_combat_mods().force_critical=true; emit_message(emit,"Your next strike will be a critical hit."); return; }
  if (envelope.event=="party:create") {
    // party.js createParty: a fresh solo party with a unique id. The id is
    // per-creation (never reused), so a post-disconnect create proves the old
    // membership was dropped (persistence scenario).
    static std::atomic<std::uint64_t> party_serial{1};
    JsonValue::Object party;
    put(party,"id","native-party-"+std::to_string(party_serial++));
    put(party,"members",JsonValue::Array{});
    emit(Envelope{"party:update",JsonValue::Object{{"party",std::move(party)}}});
    return;
  }
  if (envelope.event=="item:equip") { if (payload) handle_equip(*payload,emit); return; }
  if (envelope.event=="player:extract") { handle_extract(emit); return; }
  if (envelope.event=="player:npc:talk") { if (payload) handle_npc_talk(*payload, emit); return; }
  if (envelope.event=="player:npc:examine") {
    const auto* item = payload ? payload->get("item") : nullptr;
    emit_npc_dialogue(as_int(item ? item->get("id") : nullptr, -1), emit);
    return;
  }
  if (envelope.event=="house:investment:choose") {
    if (payload) handle_house_investment(*payload, emit);
    return;
  }
  if (envelope.event=="player:skilltree:save") { if (payload) handle_skilltree_save(*payload, emit); return; }
  if (envelope.event=="chronicles:house:deposit") { if (payload) handle_house_deposit(*payload, emit); return; }
  if (envelope.event=="dev:monster:reset") {
    const std::string monster_uuid = as_string(payload ? payload->get("monsterUuid") : nullptr);
    const int max_health = as_int(payload ? payload->get("maxHealth") : nullptr, 0);
    if (world_->in_instance() && world_->reset_monster(monster_uuid, max_health)) {
      emit_message(emit, "Reset the monster for a comparison trial.");
    }
    return;
  }
  if (envelope.event=="dev:clear-floor") {
    // dev.js dev:clear-floor: kill every monster on the active floor.
    if (world_->in_instance()) {
      bool warden_was_alive = false;
      for (const auto& monster : world_->monsters()) if (monster.alive && monster.boss) { warden_was_alive = true; break; }
      world_->kill_all_monsters();
      if (warden_was_alive && !current_node_id_.empty()) {
        cleared_nodes_.insert(current_node_id_);
        world_->set_block_stairs_down(false);
        emit_message(emit, "The Warden of " + current_node_name_ + " is down. The road runs on.");
      }
      emit_message(emit, "Cleared the active floor for objective verification.");
      emit_ground_change(emit);
      maybe_floor_cleared(emit);
    }
    return;
  }
  if (envelope.event=="party:returnToTown") {
    // party.js party:returnToTown: leave the instance for the surface.
    if (world_->in_instance()) {
      world_->return_to_surface();
      emit_movement(emit);
      emit_message(emit, "The party returns to the surface.");
      finish_extraction(emit);
      maybe_complete_first_goal(emit); quest_trigger("return-surface", emit, zone_id_for_instance(last_instance_theme_, last_instance_layout_), last_instance_theme_);
      emit_transition(emit, "party:scene:transition");
    }
    return;
  }
  if (envelope.event=="player:take:underfoot") { handle_take_underfoot(emit); return; }
  if (envelope.event=="player:context-menu:build") { if (payload) handle_menu_build(*payload,emit); return; }
  if (envelope.event=="player:context-menu:action") { if (payload) handle_menu_action(*payload,emit); return; }
  if (envelope.event=="player:inventory:commit") { if (payload) handle_inventory_commit(*payload,emit); return; }
  if (envelope.event=="dev:state") { maybe_respawn(now_ms()); if (world_->in_instance()) best_depth_=(std::max)(best_depth_,world_->metadata().depth); process_combat(now_ms(),emit); const auto id=as_string(payload?payload->get("requestId"):nullptr); const auto* want_map=payload?payload->get("includeMap"):nullptr; const bool include_map=want_map&&((want_map->boolean()&&*want_map->boolean())||(want_map->number()&&*want_map->number()!=0.0)); JsonValue data; parse_json(state_payload(id,include_map),data); emit(Envelope{"dev:state",std::move(data)}); return; }
  // ── N5 Chronicles admission (server/player/handlers/chronicles.js) ──────
  if (envelope.event=="chronicles:house:found") {
    static std::atomic<std::uint64_t> house_serial{1};
    const std::string name=as_string(payload?payload->get("name"):nullptr,"House");
    const std::string house_id="house-"+std::to_string(house_serial++);
    ensure_chronicle_house(house_id,name);
    active_house_id_=house_id;
    active_house_name_=name;
    campaign_complete_=false;
    endgame_maps_completed_=0;
    house_treasury_=0;
    house_progression_={};
    chronicles_revision_+=1;
    emit(Envelope{"chronicles:state",chronicles_state_payload("")});
    return;
  }
  if (envelope.event=="chronicles:scion:create") {
    static std::atomic<std::uint64_t> scion_serial{1};
    const std::string house_id=as_string(payload?payload->get("houseId"):nullptr);
    const std::string name=as_string(payload?payload->get("name"):nullptr,"Scion");
    const std::string scion_id="scion-"+std::to_string(scion_serial++);
    ensure_chronicle_scion(house_id,scion_id,name,false);
    active_scion_name_=name;
    chronicles_revision_+=1;
    emit(Envelope{"chronicles:state",chronicles_state_payload(scion_id)});
    return;
  }
  if (envelope.event=="chronicles:scion:set-out") {
    active_scion_id_=as_string(payload?payload->get("scionId"):nullptr);
    pending_chronicles_=false;
    house_renown_=0;
    if (JsonValue::Object* house =
            find_chronicle_house_object(chronicle_, active_house_id_)) {
      auto campaign = house->find("campaignComplete");
      campaign_complete_ = campaign != house->end() &&
                           as_bool(&campaign->second, false);
      auto completed = house->find("endgameMapsCompleted");
      endgame_maps_completed_ = completed == house->end()
                                    ? 0
                                    : as_int(&completed->second, 0);
      auto renown = house->find("renown");
      house_renown_ = renown == house->end()
                          ? 0
                          : as_int(&renown->second, 0);
    }
    restore_house_progression();
    restore_quest_progression();
    // JS beginScionSession parity (server/core/services/chronicles.js:210-219):
    // EVERY Chronicles set-out admits the scion under the hard lifecycle -
    // the mortal oath is the Chronicles admission contract, not a dev-only
    // arming path. Without it process_combat's final-death gate can never
    // fire from an ordinary lethal wound, so a normal player can never reach
    // the crypt, circulation, or succession (the TASK-0148 Gate-B gap).
    mortal_oath_=true;
    lifecycle_mode_="hard";
    lifecycle_="alive";
    lifecycle_deaths_=0;
    respawn_at_ms_=0;
    respawn_protection_until_ms_=0;
    prepare_final_death_=false;
    set_scion_record_mortal(chronicle_, active_house_id_, active_scion_id_, true);
    // crossroads: the scion spawns beside their House wagon pitch, and the
    // first set-out of the day claims the road purse into the ledger.
    {
      std::uint32_t hash = 5381;
      for (unsigned char c : active_house_id_) hash = hash * 33 + c;
      home_pitch_index_ = static_cast<int>(hash % 8);
    }
    if (!daily_purse_claimed_) {
      daily_purse_claimed_ = true;
      house_treasury_ += 100;
      persist_house_progression();
      emit_message(emit, "Your House's wagon rolls in with the dawn market. The quartermaster counts 100 gold into the ledger - the day's road purse.");
    }
    // chronicles.js starter kit: granted once per scion - a re-set-out must
    // not mint gold back (house-treasury: deposits stay deposited).
    if (!kitted_scions_.count(active_scion_id_)) {
      kitted_scions_.insert(active_scion_id_);
      bool has_dagger=false; int coins=0;
      for (const auto& item:inventory_.items()) { if (item.id=="bronze-dagger") has_dagger=true; if (item.id=="coins") coins+=item.qty; }
      if (!has_dagger) { CreateItemOptions o; auto dagger=create_game_item("bronze-dagger",o); if (dagger) inventory_.add(std::move(*dagger)); }
      if (coins<100) { CreateItemOptions o; o.quantity=100-coins; auto purse=create_game_item("coins",o); if (purse) inventory_.add(std::move(*purse)); }
    }
    world_->reset_to_town();
    world_->teleport(kWagonPitches[home_pitch_index_][0], kWagonPitches[home_pitch_index_][1] + 1, now_ms());
    emit_login(emit);
    return;
  }
  if (envelope.event=="player:chronicles:mutate") {
    const std::string type=as_string(payload?payload->get("type"):nullptr);
    if (type=="found-house") {
      const auto* house=payload?payload->get("house"):nullptr;
      const std::string id=as_string(house?house->get("id"):nullptr);
      const std::string name=as_string(house?house->get("name"):nullptr,"House");
      ensure_chronicle_house(id,name);
      active_house_id_=id;
    } else if (type=="add-scion") {
      const std::string house_id=as_string(payload?payload->get("houseId"):nullptr);
      const auto* scion=payload?payload->get("scion"):nullptr;
      const std::string id=as_string(scion?scion->get("id"):nullptr);
      const std::string name=as_string(scion?scion->get("name"):nullptr,"Scion");
      const bool mortal=as_bool(scion?scion->get("mortal"):nullptr,false);
      ensure_chronicle_scion(house_id,id,name,mortal);
    }
    chronicles_revision_+=1;
    JsonValue::Object data;
    put(data,"player",JsonValue::Object{{"socket_id",socket_id_}});
    put(data,"chronicles",chronicle_);
    put(data,"chroniclesRevision",chronicles_revision_);
    put(data,"chroniclesExists",true);
    emit(Envelope{"player:chronicles:update",JsonValue(std::move(data))});
    return;
  }
  if (envelope.event=="player:chronicles:save") {
    const auto* state=payload?payload->get("state"):nullptr;
    if (state) chronicle_=*state;
    chronicles_revision_+=1;
    JsonValue::Object data;
    put(data,"player",JsonValue::Object{{"socket_id",socket_id_}});
    put(data,"chronicles",chronicle_);
    put(data,"chroniclesRevision",chronicles_revision_);
    put(data,"chroniclesExists",true);
    emit(Envelope{"player:chronicles:update",JsonValue(std::move(data))});
    return;
  }
  if (envelope.event=="player:chronicles:return") {
    // The scion was moved to the crypt at final death; attach the queued
    // heirloom (exact item identity) to its crypt record and return the
    // socket to the pending Chronicles state.
    if (auto* root = chronicle_.object()) {
      auto houses_it = root->find("houses");
      if (houses_it != root->end() && houses_it->second.array()) {
        for (auto& house_entry : *houses_it->second.array()) {
          auto* house = house_entry.object();
          if (!house) continue;
          auto id_it = house->find("id");
          if (id_it == house->end() || !id_it->second.string() || *id_it->second.string() != active_house_id_) continue;
          auto crypt_it = house->find("crypt");
          if (crypt_it != house->end() && crypt_it->second.array()) {
            for (auto& crypt_entry : *crypt_it->second.array()) {
              auto* scion = crypt_entry.object();
              if (!scion) continue;
              auto sid_it = scion->find("id");
              if (sid_it == scion->end() || !sid_it->second.string() || *sid_it->second.string() != active_scion_id_) continue;
              JsonValue::Object relic;
              put(relic, "status", "queued");
              if (!pending_relic_items_.empty()) {
                put(relic, "item", item_identity_json(pending_relic_items_.front()));
              } else {
                put(relic, "item", JsonValue(nullptr));
              }
              (*scion)["relic"] = JsonValue(std::move(relic));
              break;
            }
          }
          break;
        }
      }
    }
    JsonValue::Object data;
    put(data, "player", JsonValue::Object{{"socket_id", socket_id_}});
    JsonValue::Object fallen;
    put(fallen, "scionId", active_scion_id_.empty() ? JsonValue(nullptr) : JsonValue(active_scion_id_));
    put(fallen, "scionName", active_scion_name_);
    put(data, "fallen", JsonValue(std::move(fallen)));
    JsonValue cp = chronicles_payload();
    if (const auto* cp_obj = cp.object()) for (const auto& [k,v]:*cp_obj) data[k]=v;
    pending_chronicles_=true;
    emit(Envelope{"player:chronicles:ready", JsonValue(std::move(data))});
    return;
  }
  if (envelope.event=="player:chronicles:select") {
    active_scion_id_=as_string(payload?payload->get("scionId"):nullptr);
    active_house_id_=as_string(payload?payload->get("houseId"):nullptr);
    active_scion_name_=as_string(payload?payload->get("scionName"):nullptr);
    mortal_oath_=as_bool(payload?payload->get("mortal"):nullptr,false);
    lifecycle_mode_=mortal_oath_?"hard":"soft";
    lifecycle_="alive"; lifecycle_deaths_=0; respawn_at_ms_=0; respawn_protection_until_ms_=0; prepare_final_death_=false;
    pending_chronicles_=false;
    campaign_complete_=false;
    endgame_maps_completed_=0;
    house_renown_=0;
    if (JsonValue::Object* house =
            find_chronicle_house_object(chronicle_, active_house_id_)) {
      auto campaign = house->find("campaignComplete");
      if (campaign != house->end())
        campaign_complete_ = as_bool(&campaign->second, false);
      auto completed = house->find("endgameMapsCompleted");
      if (completed != house->end())
        endgame_maps_completed_ = as_int(&completed->second, 0);
      auto renown = house->find("renown");
      if (renown != house->end())
        house_renown_ = as_int(&renown->second, 0);
    }
    restore_house_progression();
    // Persist the sworn oath on the living roster so relogins restore the
    // same lifecycle (see reset_world_for_new_socket).
    set_scion_record_mortal(chronicle_, active_house_id_, active_scion_id_, mortal_oath_);
    // A first admission starts fresh; selecting an existing living Scion
    // restores that Scion's Chronicle-backed campaign checkpoint.
    first_goal_stage_="available"; first_goal_started_ms_=0; first_goal_completed_ms_=0;
    restore_quest_progression();
    // A new scion starts with the fresh-scion profile (purse only), never a
    // duplicate of the previous scion's equipment.
    wear_.clear(); inventory_.clear();
    CreateItemOptions purse; purse.quantity=100;
    auto coins=create_game_item("coins",purse); if (coins) inventory_.add(std::move(*coins));
    // JS parity (createScionSessionProfile): an admitted scion arrives with
    // full resources. The Simulation actor is reused across scions, so the
    // heir must not inherit the fallen scion's lethal wound - leaving life 0
    // makes the next process_combat tick commit a second fall with no combat
    // input at all.
    if (auto* fresh_actor = simulation_->actor(simulation_->scion().actor_id)) {
      fresh_actor->alive = true;
      fresh_actor->stats.life = fresh_actor->stats.life_max;
    }
    sync_combat_mods();
    world_->reset_to_town();
    emit_login(emit);
    return;
  }
  if (envelope.event=="player:login") {
    const bool await_chronicles=as_bool(payload?payload->get("awaitChronicles"):nullptr,false);
    const std::string scion_name=as_string(payload?payload->get("scionName"):nullptr);
    const std::string guest_id=as_string(payload?payload->get("guestId"):nullptr);
    if (await_chronicles && scion_name.empty()) {
      pending_chronicles_=true;
      JsonValue::Object data;
      put(data,"player",JsonValue::Object{{"socket_id",socket_id_}});
      JsonValue cp=chronicles_payload();
      if (const auto* cp_obj=cp.object()) for (const auto& [k,v]:*cp_obj) data[k]=v;
      emit(Envelope{"player:chronicles:ready",JsonValue(std::move(data))});
      return;
    }
    // guestId routes into the Chronicle-auth flow: emit chronicles:state and
    // let the harness auto-found a house / create a scion / set out. Quick
    // guests keep the JS server's fast path: straight into the world.
    if (!guest_id.empty() && !quick_start_ && scion_name.empty() && !pending_chronicles_) {
      pending_chronicles_=true;
      emit(Envelope{"chronicles:state",chronicles_state_payload("")});
      return;
    }
    // Plain login / session reuse keeps the live world (JS parity: a
    // re-login lands wherever the session already is). Fresh sessions start
    // in town via the WorldSimulation constructor; only a chronicles
    // set-out / successor path resets deliberately.
    emit_login(emit);
  }
}

struct WebSocketServer::Connection {
  socket_t socket = invalid_socket;
  std::string id;
  std::mutex send_mutex;
  bool closed = false;
  std::shared_ptr<ProtocolSession> session;
  void send_text(const std::string& text) {
    std::lock_guard lock(send_mutex); if (closed) return; std::vector<std::uint8_t> frame; frame.push_back(0x81); const auto size=text.size(); if(size<126) frame.push_back(static_cast<std::uint8_t>(size)); else if(size<=65535){frame.push_back(126);frame.push_back(static_cast<std::uint8_t>(size>>8));frame.push_back(static_cast<std::uint8_t>(size));} else {frame.push_back(127);for(int i=7;i>=0;--i)frame.push_back(static_cast<std::uint8_t>((size>>(i*8))&0xff));} frame.insert(frame.end(),text.begin(),text.end()); if(!send_all(socket,frame.data(),frame.size())) closed=true;
  }
  void close() { std::lock_guard lock(send_mutex); if (!closed) { closed=true; close_socket(socket); socket=invalid_socket; } }
  // Flush the send direction (FIN after buffered bytes) before a close. A
  // bare closesocket on Windows can race a blocked recv on this socket's own
  // thread and drop a just-sent frame (seen as a lost player:session-replaced).
  void shutdown_send() {
    std::lock_guard lock(send_mutex);
    if (closed || socket == invalid_socket) return;
#ifdef _WIN32
    ::shutdown(socket, SD_SEND);
#endif
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }
};

WebSocketServer::WebSocketServer(std::uint16_t port):port_(port) {}
WebSocketServer::~WebSocketServer(){ stop(); }
bool WebSocketServer::start(std::string* error) {
#ifdef _WIN32
  WSADATA data{}; if (WSAStartup(MAKEWORD(2,2),&data)!=0) { if(error)*error="WSAStartup failed"; return false; }
#endif
  const auto listener=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); if(listener==invalid_socket){if(error)*error="socket failed";return false;} int yes=1; setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<const char*>(&yes),sizeof(yes)); sockaddr_in address{}; address.sin_family=AF_INET; address.sin_addr.s_addr=inet_addr("127.0.0.1"); address.sin_port=htons(port_); if(bind(listener,reinterpret_cast<sockaddr*>(&address),sizeof(address))<0 || listen(listener,16)<0){close_socket(listener);if(error)*error="bind/listen failed";return false;} listen_socket_=static_cast<std::intptr_t>(listener); running_=true; accept_thread_=std::make_unique<std::thread>(&WebSocketServer::accept_loop,this); tick_thread_=std::make_unique<std::thread>([this]{ while(running_){ std::this_thread::sleep_for(std::chrono::milliseconds(150)); std::vector<std::shared_ptr<ProtocolSession>> ticking; { std::lock_guard lock(mutex_); for (auto& [key, session] : sessions_) ticking.push_back(session); } const auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); for (auto& session : ticking) session->tick(now); } }); return true;
}
void WebSocketServer::stop(){ if(!running_)return; running_=false; close_socket(static_cast<socket_t>(listen_socket_)); listen_socket_=-1; if(accept_thread_&&accept_thread_->joinable())accept_thread_->join(); if(tick_thread_&&tick_thread_->joinable())tick_thread_->join();
  // Close every connection socket first (unblocks each reader's recv), then
  // join the reader threads OUTSIDE the lock - handle_connection takes
  // mutex_ on its way out (remove_connection). Joining, not detaching, is
  // what makes `delete server` safe: a detached reader waking afterwards
  // would dereference this freed object.
  std::vector<std::thread> readers;
  { std::lock_guard lock(mutex_); for(auto& c:connections_)c->close(); readers.swap(connection_threads_); }
  for(auto& reader:readers) if(reader.joinable()) reader.join();
  std::lock_guard lock(mutex_); connections_.clear(); sessions_.clear();
#ifdef _WIN32
  WSACleanup();
#endif
}
void WebSocketServer::accept_loop(){ while(running_){ sockaddr_in address{}; socket_length_t length=sizeof(address); const auto client=::accept(static_cast<socket_t>(listen_socket_),reinterpret_cast<sockaddr*>(&address),&length); if(client==invalid_socket){if(running_)continue;break;} auto connection=std::make_shared<Connection>(); connection->socket=client; static std::atomic<std::uint64_t> serial{1}; connection->id="native-"+std::to_string(serial++); {std::lock_guard lock(mutex_);connections_.push_back(connection);connection_threads_.emplace_back(&WebSocketServer::handle_connection,this,connection);} } }
void WebSocketServer::handle_connection(std::shared_ptr<Connection> connection){ std::string headers; char buffer[1024]; while(headers.find("\r\n\r\n")==std::string::npos&&headers.size()<8192){ const auto got=recv(connection->socket,buffer,sizeof(buffer),0); if(got<=0){connection->close();remove_connection(connection);return;} headers.append(buffer,buffer+got); } const auto key_pos=headers.find("Sec-WebSocket-Key:"); if(key_pos==std::string::npos){connection->close();remove_connection(connection);return;} auto start=key_pos+18; while(start<headers.size()&&headers[start]==' ')++start; auto end=headers.find("\r\n",start); const auto key=headers.substr(start,end-start); const std::string response="HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: "+ws_accept_key(key)+"\r\n\r\n"; if(!send_all(connection->socket,response.data(),response.size())){connection->close();remove_connection(connection);return;} while(running_&&!connection->closed){ std::uint8_t header[2];if(!recv_all(connection->socket,header,2))break; const auto opcode=header[0]&0x0f; bool masked=(header[1]&0x80)!=0; std::uint64_t length=header[1]&0x7f; if(length==126){std::uint8_t ext[2];if(!recv_all(connection->socket,ext,2))break;length=(ext[0]<<8)|ext[1];}else if(length==127){std::uint8_t ext[8];if(!recv_all(connection->socket,ext,8))break;length=0;for(auto byte:ext)length=(length<<8)|byte;} if(length>16384||!masked)break; std::array<std::uint8_t,4> mask{};if(!recv_all(connection->socket,mask.data(),4))break;std::string payload(length,'\0');if(!recv_all(connection->socket,payload.data(),length))break;for(std::size_t i=0;i<length;++i)payload[i]^=mask[i%4]; if(opcode==8)break;if(opcode==9){std::vector<std::uint8_t> pong{0x8a,static_cast<std::uint8_t>(length)};pong.insert(pong.end(),payload.begin(),payload.end());send_all(connection->socket,pong.data(),pong.size());continue;}if(opcode==1)handle_message(connection,payload); } connection->close();remove_connection(connection); }
std::shared_ptr<ProtocolSession> WebSocketServer::session_by_username(const std::string& username) {
  std::lock_guard lock(mutex_);
  for (auto& [key, session] : sessions_) {
    if (session->matches_name(username)) return session;
  }
  return nullptr;
}

void WebSocketServer::send_to_identity(const std::string& identity, const Envelope& envelope) {
  std::vector<std::shared_ptr<Connection>> targets;
  { std::lock_guard lock(mutex_);
    for (const auto& candidate : connections_) {
      if (candidate->session && !candidate->closed && candidate->session->identity() == identity) targets.push_back(candidate);
    } }
  const auto wire = emit_envelope(envelope);
  for (auto& target : targets) target->send_text(wire);
}

void WebSocketServer::send_party_update(const ServerParty& party) {
  JsonValue::Array members;
  for (const auto& uuid : party.member_uuids) {
    std::shared_ptr<ProtocolSession> session;
    { std::lock_guard lock(mutex_); auto it = sessions_.find(uuid); if (it != sessions_.end()) session = it->second; }
    JsonValue::Object entry;
    put(entry, "uuid", uuid);
    put(entry, "username", session ? session->display_name() : uuid);
    auto ready_it = party.ready.find(uuid);
    put(entry, "ready", ready_it != party.ready.end() && ready_it->second);
    members.emplace_back(std::move(entry));
  }
  JsonValue::Object party_json;
  put(party_json, "id", party.id);
  put(party_json, "leaderId", party.leader_uuid);
  put(party_json, "state", party.state);
  put(party_json, "members", std::move(members));
  for (const auto& uuid : party.member_uuids) {
    send_to_identity(uuid, Envelope{"party:update", JsonValue::Object{{"party", JsonValue(party_json)}}});
  }
}

bool WebSocketServer::handle_party_event(const std::shared_ptr<Connection>& connection, const Envelope& envelope) {
  auto session = connection->session;
  if (!session) return false;
  const std::string uuid = session->identity();
  auto emit_to_self = [&](const Envelope& out) { connection->send_text(emit_envelope(out)); };
  if (envelope.event == "party:create") {
    static std::atomic<std::uint64_t> party_serial{1};
    ServerParty party;
    party.id = "party-" + std::to_string(party_serial++);
    party.leader_uuid = uuid;
    party.member_uuids.push_back(uuid);
    party.ready[uuid] = false;
    { std::lock_guard lock(mutex_); parties_[party.id] = party; party_by_uuid_[uuid] = party.id; }
    send_party_update(party);
    return true;
  }
  auto party_of = [&](const std::string& member) -> ServerParty* {
    auto id_it = party_by_uuid_.find(member);
    if (id_it == party_by_uuid_.end()) return nullptr;
    auto party_it = parties_.find(id_it->second);
    return party_it == parties_.end() ? nullptr : &party_it->second;
  };
  if (envelope.event == "party:invite") {
    const auto* name = envelope.data.get("username");
    if (!name || !name->string()) return true;
    auto target = session_by_username(*name->string());
    ServerParty* party = party_of(uuid);
    if (!target || !party) return true;
    JsonValue::Object invite;
    put(invite, "partyId", party->id);
    put(invite, "invitedBy", session->display_name());
    send_to_identity(target->identity(), Envelope{"party:invited", JsonValue::Object{{"invite", JsonValue(std::move(invite))}}});
    return true;
  }
  if (envelope.event == "party:invite:accept") {
    const auto* party_id = envelope.data.get("partyId");
    if (!party_id || !party_id->string()) return true;
    ServerParty snapshot;
    { std::lock_guard lock(mutex_);
      auto it = parties_.find(*party_id->string());
      if (it == parties_.end()) return true;
      it->second.member_uuids.push_back(uuid);
      it->second.ready[uuid] = false;
      party_by_uuid_[uuid] = it->second.id;
      snapshot = it->second; }
    send_party_update(snapshot);
    return true;
  }
  if (envelope.event == "party:ready") {
    ServerParty snapshot; bool found = false;
    { std::lock_guard lock(mutex_);
      ServerParty* party = party_of(uuid);
      if (party) { party->ready[uuid] = true; snapshot = *party; found = true; } }
    if (found) send_party_update(snapshot);
    return true;
  }
  if (envelope.event == "party:startInstance") {
    ServerParty snapshot; bool found = false;
    { std::lock_guard lock(mutex_);
      ServerParty* party = party_of(uuid);
      if (party) {
        party->state = "instance";
        for (auto& [member, ready] : party->ready) ready = false;
        snapshot = *party; found = true; } }
    if (!found) return true;
    const std::string scene_id = "instance-" + snapshot.id;
    std::shared_ptr<ProtocolSession> leader_session;
    { std::lock_guard lock(mutex_); auto it = sessions_.find(snapshot.leader_uuid); if (it != sessions_.end()) leader_session = it->second; }
    std::shared_ptr<WorldSimulation> shared_world;
    if (leader_session) {
      auto leader_id = snapshot.leader_uuid;
      leader_session->enter_shared_instance(scene_id, [this, leader_id](const Envelope& out) { send_to_identity(leader_id, out); });
      shared_world = leader_session->shared_world();
    }
    for (const auto& member : snapshot.member_uuids) {
      if (member == snapshot.leader_uuid) continue;
      std::shared_ptr<ProtocolSession> member_session;
      { std::lock_guard lock(mutex_); auto it = sessions_.find(member); if (it != sessions_.end()) member_session = it->second; }
      if (member_session && shared_world) {
        auto member_id = member;
        member_session->adopt_world(shared_world, scene_id, [this, member_id](const Envelope& out) { send_to_identity(member_id, out); });
      }
    }
    send_party_update(snapshot);
    return true;
  }
  if (envelope.event == "party:leave") {
    ServerParty snapshot; bool had_party = false;
    { std::lock_guard lock(mutex_);
      ServerParty* party = party_of(uuid);
      if (party) {
        had_party = true;
        party->member_uuids.erase(std::remove(party->member_uuids.begin(), party->member_uuids.end(), uuid), party->member_uuids.end());
        party->ready.erase(uuid);
        party_by_uuid_.erase(uuid);
        snapshot = *party; } }
    if (!had_party) return true;
    session->leave_to_town(emit_to_self);
    emit_to_self(Envelope{"party:update", JsonValue::Object{{"party", JsonValue(nullptr)}}});
    if (!snapshot.member_uuids.empty()) send_party_update(snapshot);
    return true;
  }
  if (envelope.event == "party:returnToTown") {
    ServerParty snapshot; bool in_party = false;
    { std::lock_guard lock(mutex_);
      ServerParty* party = party_of(uuid);
      if (party) { party->state = "lobby"; snapshot = *party; in_party = true; } }
    if (!in_party) return false;  // solo semantics fall through to the session handler
    session->leave_to_town(emit_to_self);
    session->handle(Envelope{"party:returnToTown:solo-complete", JsonValue::Object{}}, emit_to_self);
    send_party_update(snapshot);
    return true;
  }
  return false;
}
void WebSocketServer::handle_message(const std::shared_ptr<Connection>& connection,const std::string& text){ Envelope envelope; std::string error;if(!parse_envelope(text,envelope,&error))return; if(envelope.event=="player:login"){const auto* guest=envelope.data.get("guestId");std::string identity=(guest&&guest->string())?*guest->string():"default-guest";
// JS parity: the anonymous guest is ONE shared account. A second concurrent
// login replaces the earlier session (replaceExistingSession); multiplayer
// scenarios that need distinct players carry playtestGuestId/guestId.
const bool quick=as_bool(envelope.data.get("quickGuest"));const auto* playtest_guest=envelope.data.get("playtestGuestId");if(playtest_guest&&playtest_guest->string())identity=*playtest_guest->string();const auto* playtest_name=envelope.data.get("playtestGuestName");std::shared_ptr<ProtocolSession> session;std::shared_ptr<Connection> old;{std::lock_guard lock(mutex_);auto it=sessions_.find(identity);if(it!=sessions_.end()){for(const auto& candidate:connections_)if(candidate->session==it->second&&candidate!=connection&&!candidate->closed){old=candidate;break;}session=it->second;}if(!session){std::uint64_t seed=1469598103934665603ULL;for(unsigned char c:identity)seed=(seed^c)*1099511628211ULL;session=std::make_shared<ProtocolSession>(identity,connection->id,seed,quick);sessions_[identity]=session;}else { const bool adopted = connection->session != session; session->replace_socket(connection->id); if (adopted) session->reset_world_for_new_socket(); } connection->session=session;}session->set_broadcast([this](const Envelope& event){broadcast(event);});if(playtest_name&&playtest_name->string())session->set_username(*playtest_name->string());session->set_direct_emit([connection](const Envelope& event){connection->send_text(emit_envelope(event));});if(old){old->send_text(emit_envelope(Envelope{"player:session-replaced",JsonValue::Object{{"player",JsonValue::Object{{"socket_id",old->id}}}}}));old->shutdown_send();old->close();}session->handle(envelope,[connection](const Envelope& response){connection->send_text(emit_envelope(response));});return;} auto session=connection->session;if(!session)return;if(envelope.event.rfind("party:",0)==0&&handle_party_event(connection,envelope))return;session->handle(envelope,[connection](const Envelope& response){connection->send_text(emit_envelope(response));});}
void WebSocketServer::broadcast(const Envelope& envelope){ std::vector<std::shared_ptr<Connection>> targets; {std::lock_guard lock(mutex_);targets=connections_;} const auto wire=emit_envelope(envelope); for(const auto& candidate:targets) if(candidate->session&&!candidate->closed) candidate->send_text(wire); }
void WebSocketServer::remove_connection(const std::shared_ptr<Connection>& connection){std::lock_guard lock(mutex_);connections_.erase(std::remove(connections_.begin(),connections_.end(),connection),connections_.end());}

}  // namespace verdigris::networking
