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
  return JsonValue(std::move(out));
}

// dev.js itemIdentity (server/shared item identity projection).
JsonValue item_identity_json(const GameItem& item) {
  JsonValue::Object out;
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
      simulation_(std::make_unique<Simulation>(seed, "House Verdigris")), world_(std::make_unique<WorldSimulation>(seed, identity_)) {
  // Fresh-scion admission (server/core treasuries/fresh profile): the purse
  // is the only starting inventory; the legacy starter blade is retired
  // vocabulary and intentionally absent (see the N4 report).
  CreateItemOptions purse;
  purse.quantity = 100;
  auto coins = create_game_item("coins", purse);
  if (coins) inventory_.add(std::move(*coins));
  sync_combat_mods();
}
void ProtocolSession::replace_socket(std::string socket_id) { std::lock_guard lock(mutex_); socket_id_=std::move(socket_id); }

void ProtocolSession::reset_world_for_new_socket() {
  // JS parity: a NEW socket gets a fresh Player position (town) while the
  // account state (stats, inventory, chronicle) persists. Same-socket
  // re-logins never pass through here, so hot dev re-logins keep the
  // instance (networking_tests: instance re-login snapshot).
  std::lock_guard lock(mutex_);
  world_->reset_to_town();
}
void ProtocolSession::set_broadcast(std::function<void(const Envelope&)> broadcast) { std::lock_guard lock(mutex_); broadcast_=std::move(broadcast); }
std::int64_t ProtocolSession::now_ms() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
std::string ProtocolSession::player_payload() const {
  JsonValue::Object player; const auto position=world_->position();
  put(player,"uuid",identity_); put(player,"username",active_scion_name_.empty()?identity_:active_scion_name_); put(player,"socket_id",socket_id_); put(player,"sceneId",world_->scene_id()); put(player,"x",position.x); put(player,"y",position.y); put(player,"facing",world_->facing());
  { const auto* actor=simulation_->actor(simulation_->scion().actor_id); put(player,"level",actor?actor->stats.level:1); }
  put(player,"passiveTree",passive_tree_json());
  put(player,"quests",quests_json());
  JsonValue::Array slots; for (const auto& item:inventory_.items()) { JsonValue::Object value; put(value,"id",item.id); put(value,"uuid",item.uuid); put(value,"name",item.name); if(item.slot>=0) put(value,"slot",item.slot); else put(value,"slot",nullptr); slots.emplace_back(std::move(value)); }
  JsonValue::Object inventory; put(inventory,"slots",std::move(slots)); put(player,"inventory",std::move(inventory));
  JsonValue::Object chronicles; put(chronicles,"mortal",mortal_oath_); put(chronicles,"scionId",active_scion_id_.empty()?JsonValue(nullptr):JsonValue(active_scion_id_)); put(chronicles,"houseId",active_house_id_.empty()?JsonValue(nullptr):JsonValue(active_house_id_)); put(player,"chronicles",std::move(chronicles));
  return JsonValue(std::move(player)).stringify();
}
std::string ProtocolSession::login_payload() const {
  std::lock_guard lock(mutex_); JsonValue::Object data; JsonValue player; parse_json(player_payload(),player); put(data,"player",std::move(player)); put(data,"scene",scene_payload()); put(data,"droppedItems",dropped_items_json()); if (quick_start_) put(data,"quickStart",true); return JsonValue(std::move(data)).stringify();
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
struct TownNpc { int id; const char* name; const char* examine; int x; int y; const char* actions[2]; int action_count; };
// server/core/data/npcs.js - the Crossroads roster.
const TownNpc kTownNpcs[] = {
    {1, "Aldwyn the Guide", "A weathered wayfinder who watches over the Crossroads' newest scions.", 34, 116, {"talk", "examine"}, 2},
    {2, "Mara, General Trader", "Keeps the general stall at the Crossroads bazaar. Buys most things, sells the rest.", 49, 103, {"trade", "examine"}, 2},
    {3, "Ludovicus, Weapons Trader", "Sells iron for the road. Claims every axe on his boards outlived its first three owners.", 19, 113, {"examine", "trade"}, 2},
    {4, "Rhea of the Countinghouse", "Keeps the countinghouse tent: personal storage, honest scales, no questions.", 31, 121, {"examine", "bank"}, 2},
};
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
    put(scene,"metadata",std::move(metadata)); }
  return JsonValue(std::move(scene));
}
JsonValue ProtocolSession::movement_step_payload() const {
  const auto& step=world_->last_step(); JsonValue::Object value; put(value,"sequence",static_cast<double>(step.sequence)); put(value,"startedAt",static_cast<double>(step.started_at_ms)); put(value,"duration",step.duration_ms); if(step.direction.empty()) put(value,"direction",nullptr); else put(value,"direction",step.direction); put(value,"blocked",step.blocked); return JsonValue(std::move(value));
}
JsonValue ProtocolSession::snapshot() const {
  JsonValue::Object state; const auto& scion=simulation_->scion(); const auto* actor=simulation_->actor(scion.actor_id); const auto position=world_->position();
  put(state,"uuid",identity_); put(state,"x",position.x); put(state,"y",position.y); put(state,"sceneId",world_->scene_id()); put(state,"sceneType",world_->scene_type()); put(state,"sceneName",world_->scene_name());
  put(state,"lifecycle",lifecycle_);
  put(state,"lifecycleMode",lifecycle_mode_);
  JsonValue::Object chronicles; put(chronicles,"mortal",mortal_oath_); put(chronicles,"scionId",active_scion_id_.empty()?JsonValue(nullptr):JsonValue(active_scion_id_)); put(chronicles,"houseId",active_house_id_.empty()?JsonValue(nullptr):JsonValue(active_house_id_)); put(state,"chronicles",std::move(chronicles));
  put(state,"bestDepth",best_depth_);
  put(state,"quests",quests_json());
  put(state,"questPoints",quest_points_);
  put(state,"passiveTree",passive_tree_json());
  { // stats-manager attributes: base 10s plus the tree path. STUB NOTE:
    // per-node attribute identity from the 271-node graph is approximated
    // as +2/attr per allocated node beyond the root until the geometric
    // tree engine is ported (successor task).
    int allocated = 0;
    if (passive_tree_saved_) {
      if (const auto* nodes = passive_tree_.get("nodes"); nodes && nodes->array())
        allocated = (std::max)(0, static_cast<int>(nodes->array()->size()) - 1);
    }
    JsonValue::Object attributes;
    put(attributes, "strength", 10 + allocated * 2);
    put(attributes, "dexterity", 10 + allocated * 2);
    put(attributes, "intelligence", 10 + allocated * 2);
    put(state, "attributes", std::move(attributes));
  }
  JsonValue::Array npcs;
  if (!world_->in_instance()) {
    for (const auto& npc : kTownNpcs) {
      JsonValue::Object entry;
      put(entry, "id", npc.id); put(entry, "name", npc.name);
      put(entry, "x", npc.x); put(entry, "y", npc.y);
      put(entry, "tileX", npc.x); put(entry, "tileY", npc.y);
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
    put(state,"sceneMetadata",std::move(metadata)); }
  else put(state,"sceneMetadata",JsonValue::Object{});
  // N4: the real item pipeline snapshot (dev.js buildStateSnapshot).
  put(state,"level",actor?actor->stats.level:1);
  JsonValue::Array items; for (const auto& item:inventory_.items()) items.emplace_back(snapshot_item_json(item)); put(state,"inventory",std::move(items));
  JsonValue::Array details; for (const auto& item:inventory_.items()) details.emplace_back(item_identity_json(item)); put(state,"inventoryDetails",std::move(details));
  put(state,"wear",wear_json()); put(state,"wearDetails",wear_details_json());
  put(state,"combat",combat_totals_json());
  put(state,"groundItems",dropped_items_json());
  put(state,"droppedItems",dropped_items_json());
  JsonValue::Array stored; for (const auto& item:house_store_) stored.emplace_back(item_identity_json(item)); put(state,"houseStoredItems",std::move(stored));
  put(state,"groundTrophies",JsonValue::Array{}); return JsonValue(std::move(state));
}
std::string ProtocolSession::state_payload(const std::string& request_id) const { std::lock_guard lock(mutex_); JsonValue::Object data; put(data,"player",JsonValue::Object{{"socket_id",socket_id_}}); put(data,"state",snapshot()); put(data,"requestId",request_id); return JsonValue(std::move(data)).stringify(); }
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
  std::vector<std::string> uuids;
  uuids.reserve(inventory_.items().size());
  for (const auto& item : inventory_.items()) uuids.push_back(item.uuid);
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
  emit_inventory_refresh(emit);
  emit_equip_state(emit);
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
  return JsonValue(std::move(quests));
}

JsonValue ProtocolSession::passive_tree_json() const {
  // verdigris-authority.js: server owns the budget. earned =
  // min(140, min(max(2, level), 117) + min(questPoints, 23)).
  const auto* actor = simulation_->actor(simulation_->scion().actor_id);
  const int level = actor ? actor->stats.level : 1;
  const int earned = (std::min)(140, (std::min)((std::max)(2, level), 117) +
                                     (std::min)((std::max)(0, quest_points_), 23));
  JsonValue::Array nodes;
  JsonValue::Array conduits;
  std::string selected = "0,0";
  JsonValue::Array class_order;
  int spent = 1;
  if (passive_tree_saved_) {
    if (const auto* saved_nodes = passive_tree_.get("nodes"); saved_nodes && saved_nodes->array()) {
      nodes = *saved_nodes->array();
      spent = static_cast<int>(nodes.size());
    }
    if (const auto* saved_conduits = passive_tree_.get("conduits"); saved_conduits && saved_conduits->array())
      conduits = *saved_conduits->array();
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

void ProtocolSession::emit_bank_screen(const std::function<void(const Envelope&)>& emit) const {
  // chronicles.js sendBankState: open:screen bank with House treasury.
  JsonValue::Object house;
  put(house, "id", active_house_id_.empty() ? JsonValue(nullptr) : JsonValue(active_house_id_));
  put(house, "name", active_house_name_.empty() ? JsonValue("House Verdigris") : JsonValue(active_house_name_));
  put(house, "treasury", house_treasury_);
  JsonValue::Object payload;
  put(payload, "items", JsonValue::Array{});
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
  if (first_goal_stage_ == "clear-floor" && meta.theme == "dungeon" && meta.layout == "warren" && meta.depth == 1) {
    first_goal_stage_ = "return-to-town";
    emit_message(emit, "The floor is cleared. Return to Aldwyn at the Crossroads for your reward.");
    emit_quest_update(emit);
  }
}
void ProtocolSession::emit_quest_update(const std::function<void(const Envelope&)>& emit) const {
  // first-goal.js pushQuestState -> quest:update.
  JsonValue::Object data;
  put(data, "player", JsonValue::Object{{"socket_id", socket_id_}});
  put(data, "quests", quests_json());
  put(data, "questPoints", quest_points_);
  put(data, "passiveTree", passive_tree_json());
  emit(Envelope{"quest:update", JsonValue(std::move(data))});
}

void ProtocolSession::handle_npc_talk(const JsonValue& payload, const std::function<void(const Envelope&)>& emit) {
  // actions/index.js player:npc:talk - Aldwyn only, town only, chebyshev<=1.
  const auto* item = payload.get("item");
  const int npc_id = as_int(item ? item->get("id") : nullptr, -1);
  if (npc_id != 1 || world_->in_instance()) return;
  const Vec2 tile = tile_movement::occupied_tile(world_->position());
  if ((std::max)(std::abs(tile.x - 34), std::abs(tile.y - 116)) > 1) return;
  if (first_goal_stage_ == "available") {
    first_goal_stage_ = "clear-floor";
    first_goal_started_ms_ = now_ms();
    emit_message(emit, "No road holds past a living Warden. Take any gate out - the first stretch of every road is on your House's chart - put its Warden down, and come back to me.");
    emit_quest_update(emit);
    return;
  }
  if (first_goal_stage_ == "clear-floor") {
    emit_message(emit, "Your task remains: put down the Warden of any first stretch on your chart, then return to me.");
    return;
  }
  if (first_goal_stage_ == "return-to-town") {
    emit_message(emit, "The country lies still. Walk back through the gate and I will mark the deed.");
    return;
  }
  emit_message(emit, "The chart remembers your first Warden. Spend your Verdigris point wisely.");
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
  quest_points_ = (std::min)(quest_points_ + 1, 12);
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
  if (!relic_scion_id.empty()) mark_relic_recovered(relic_scion_id);
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
  if (clicked_has("gameMap")) {
    // World variant: Take per ground item on the clicked tile, newest first.
    const auto* tile=payload.get("tile");
    const auto* world_pos=tile?tile->get("world"):nullptr;
    const int wx=as_int(world_pos?world_pos->get("x"):nullptr);
    const int wy=as_int(world_pos?world_pos->get("y"):nullptr);
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
    // Inventory variant: the brand service entry for eligible vessel items.
    const int slot=as_int(misc?misc->get("slot"):nullptr,-1);
    const GameItem* item=nullptr;
    for (const auto& candidate:inventory_.items()) { if (candidate.slot==slot) { item=&candidate; break; } }
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
  const auto* item_ref=queue_item?queue_item->get("item"):nullptr;
  const std::string uuid=as_string(item_ref?item_ref->get("uuid"):nullptr);
  if (action_id=="player:take") { handle_take_ground(uuid,emit); return; }
  if (action_id=="player:screen:bank") { emit_bank_screen(emit); return; }
  if (action_id=="player:screen:shop-display" || action_id=="player:npc:trade" ||
      action_id=="player:shop-display:buy" || action_id=="player:shop-display:appraise") {
    Envelope forwarded{action_id, payload};
    handle(forwarded, emit);
    return;
  }
  if (action_id=="player:npc:talk") { if (queue_item) handle_npc_talk(*queue_item, emit); return; }
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
  put(data,"targetName",event.target_name); put(data,"targetType",event.target_id==identity_?"player":"monster"); put(data,"skillId",event.skill_id);
  put(data,"amount",event.amount); put(data,"died",event.died); put(data,"health",JsonValue::Object{{"current",event.health},{"max",event.health_max}});
  // combat/index.js hit parity fields.
  put(data,"baseAmount",event.base_amount); put(data,"beastbaneAmount",event.beastbane_amount);
  put(data,"beastbanePercent",event.beastbane_percent); put(data,"beastbane",event.beastbane);
  put(data,"critical",event.critical); put(data,"attackStyle",event.attack_style);
  emit_world(Envelope{"combat:hit",JsonValue(std::move(data))},emit);
}
void ProtocolSession::process_combat(std::int64_t now, const std::function<void(const Envelope&)>& emit) {
  auto* actor = simulation_->actor(simulation_->scion().actor_id); if (!actor) return;
  const int life_before = actor->stats.life;
  const auto combat_totals = wear_.totals();
  const int wear_attack = (std::max)(0, (std::max)((std::max)(combat_totals.attack.stab, combat_totals.attack.slash),
                                                   (std::max)(combat_totals.attack.crush, combat_totals.attack.range)));
  const auto events = world_->advance_combat(actor->stats.level, actor->stats.attack + wear_attack, actor->stats.life, actor->stats.life_max, now);
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
      // one queued House heirloom to the floor where it fell.
      if (!pending_relic_items_.empty()) {
        for (const auto& monster : world_->monsters()) {
          if (monster.uuid != event.target_id || monster.rarity != "elite") continue;
          GameItem relic = pending_relic_items_.front();
          pending_relic_items_.erase(pending_relic_items_.begin());
          static std::atomic<std::uint64_t> kill_relic_serial{1};
          const std::string relic_id = "relic-" + std::to_string(kill_relic_serial++);
          world_->add_relic_ground_item(std::move(relic), monster.x, monster.y, relic_id,
                                        relic_source_scion_id_, relic_source_scion_name_);
          emit_message(emit, "A relic of the fallen has surfaced.");
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
  maybe_complete_first_goal(emit);
  emit_transition(emit, "party:scene:transition");
}
void ProtocolSession::handle_final_death(const std::function<void(const Envelope&)>& emit) {
  auto* actor = simulation_->actor(simulation_->scion().actor_id);
  lifecycle_ = "permadead";
  lifecycle_mode_ = "hard";
  prepare_final_death_ = false;
  // D-106: carried value is never destroyed — capture it into circulation.
  pending_relic_items_.clear();
  for (const auto& item : inventory_.items()) if (item.id != "coins") pending_relic_items_.push_back(item);
  for (const auto& [seat, item] : wear_.slots()) (void)seat, pending_relic_items_.push_back(item);
  int relic_count = static_cast<int>(pending_relic_items_.size());
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
  const auto* payload=envelope.data.object()?&envelope.data:nullptr;
  if (envelope.event=="world:zone:enter") { const auto node=as_string(payload?payload->get("nodeId"):nullptr,"tin:1:0"); simulation_->dispatch(Command::enter(node.rfind("route:",0)==0?node:"route:"+node)); world_->enter_solo_instance("dungeon",""); emit_transition(emit,"world:scene:transition"); emit_ground_change(emit); return; }
  if (envelope.event=="instance:enterSolo") { world_->enter_solo_instance(as_string(payload?payload->get("template"):nullptr,"dungeon"),as_string(payload?payload->get("layout"):nullptr,"")); emit_transition(emit,"party:scene:transition"); emit_ground_change(emit); return; }
  if (envelope.event=="player:move") { const auto direction=as_string(payload?payload->get("direction"):nullptr); const bool was_instance=world_->in_instance(); const int depth_before=world_->metadata().depth; const std::string scene_before=world_->scene_id(); if (world_->apply_movement_sample(direction,now_ms())) { emit_movement(emit); auto_pickup_gold(emit); const bool depth_changed=world_->in_instance()&&world_->metadata().depth!=depth_before; const bool scene_changed=world_->scene_id()!=scene_before; if (depth_changed||scene_changed) { if (was_instance&&!world_->in_instance()) { emit_message(emit,"The party returns to the surface."); finish_extraction(emit); maybe_complete_first_goal(emit); } emit_transition(emit,"party:scene:transition"); if (world_->in_instance()) emit_ground_change(emit); } } return; }
  if (envelope.event=="dev:teleport") { if (!payload) return; const auto* x=payload->get("x"); const auto* y=payload->get("y"); if (!x||!x->number()||!y||!y->number()) return; const int tx=static_cast<int>(*x->number()); const int ty=static_cast<int>(*y->number()); const bool was_instance=world_->in_instance(); const int depth_before=world_->metadata().depth; const std::string scene_before=world_->scene_id(); world_->teleport(tx,ty,now_ms()); const bool returned=was_instance&&!world_->in_instance(); const bool depth_changed=world_->in_instance()&&world_->metadata().depth!=depth_before; const bool transitioned=returned||depth_changed||world_->scene_id()!=scene_before; emit_movement(emit); emit_message(emit,"Teleported to "+std::to_string(tx)+", "+std::to_string(ty)+(transitioned?" (portal followed).":".")); if (returned) { emit_message(emit,"The party returns to the surface."); finish_extraction(emit); maybe_complete_first_goal(emit); } if (transitioned) emit_transition(emit,"party:scene:transition"); if (world_->in_instance()) { if (depth_changed) emit_ground_change(emit); process_combat(now_ms(),emit); } return; }
  if (envelope.event=="dev:setlevel") { auto* actor=simulation_->actor(simulation_->scion().actor_id); const int level=as_int(payload?payload->get("level"):nullptr,1); if(actor){ actor->stats.level=(std::max)(1,level); actor->stats.attack=12+actor->stats.level*3; actor->stats.life_max=100+actor->stats.level*10; actor->stats.life=actor->stats.life_max; world_->set_level(actor->stats.level); } return; }
  if (envelope.event=="player:screen:shop-display" || envelope.event=="player:npc:trade") {
    // shops.js General Store pane.
    JsonValue::Array stock;
    { JsonValue::Object row; put(row,"id","bronze-sword"); put(row,"name","Bronze Sword"); put(row,"price",15); put(row,"qty",10); stock.emplace_back(std::move(row)); }
    JsonValue::Object payload_out;
    put(payload_out,"name","General Store");
    put(payload_out,"npcId",2);
    put(payload_out,"items",std::move(stock));
    put(payload_out,"carriedCoins",carried_gold());
    JsonValue::Object data;
    put(data,"player",JsonValue::Object{{"socket_id",socket_id_}});
    put(data,"screen","shop");
    put(data,"payload",std::move(payload_out));
    emit(Envelope{"open:screen",JsonValue(std::move(data))});
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
    // dev.js dev:release-relic: drop the next queued heirloom on the active
    // floor with its fallen-scion provenance.
    if (!pending_relic_items_.empty()) {
      GameItem item = pending_relic_items_.front();
      pending_relic_items_.erase(pending_relic_items_.begin());
      const auto position = world_->position();
      static std::atomic<std::uint64_t> relic_serial{1};
      const std::string relic_id = "relic-" + std::to_string(relic_serial++);
      world_->add_relic_ground_item(std::move(item), position.x, position.y, relic_id,
                                    relic_source_scion_id_, relic_source_scion_name_);
      emit_message(emit, "A relic of the fallen has surfaced.");
    }
    return;
  }
  if (envelope.event=="player:skill:trigger") { auto* actor=simulation_->actor(simulation_->scion().actor_id); if(actor&&world_->in_instance()){ if (respawn_protection_until_ms_ > 0) respawn_protection_until_ms_ = 0; const auto direction=as_string(payload?payload->get("direction"):nullptr,"down"); const auto wear_totals=wear_.totals(); const int wear_bonus=(std::max)((std::max)(wear_totals.attack.stab,wear_totals.attack.slash),(std::max)(wear_totals.attack.crush,wear_totals.attack.range)); world_->start_player_attack(actor->stats.level,actor->stats.attack+(std::max)(0,wear_bonus),now_ms(),direction); std::int64_t t=now_ms(); for(int i=0;i<25;++i){ process_combat(t,emit); t+=400; } } return; }
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
  if (envelope.event=="player:skilltree:save") { if (payload) handle_skilltree_save(*payload, emit); return; }
  if (envelope.event=="chronicles:house:deposit") { if (payload) handle_house_deposit(*payload, emit); return; }
  if (envelope.event=="dev:clear-floor") {
    // dev.js dev:clear-floor: kill every monster on the active floor.
    if (world_->in_instance()) {
      world_->kill_all_monsters();
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
      maybe_complete_first_goal(emit);
      emit_transition(emit, "party:scene:transition");
    }
    return;
  }
  if (envelope.event=="player:take:underfoot") { handle_take_underfoot(emit); return; }
  if (envelope.event=="player:context-menu:build") { if (payload) handle_menu_build(*payload,emit); return; }
  if (envelope.event=="player:context-menu:action") { if (payload) handle_menu_action(*payload,emit); return; }
  if (envelope.event=="player:inventory:commit") { if (payload) handle_inventory_commit(*payload,emit); return; }
  if (envelope.event=="dev:state") { maybe_respawn(now_ms()); if (world_->in_instance()) best_depth_=(std::max)(best_depth_,world_->metadata().depth); process_combat(now_ms(),emit); const auto id=as_string(payload?payload->get("requestId"):nullptr); JsonValue data; parse_json(state_payload(id),data); emit(Envelope{"dev:state",std::move(data)}); return; }
  // ── N5 Chronicles admission (server/player/handlers/chronicles.js) ──────
  if (envelope.event=="chronicles:house:found") {
    static std::atomic<std::uint64_t> house_serial{1};
    const std::string name=as_string(payload?payload->get("name"):nullptr,"House");
    const std::string house_id="house-"+std::to_string(house_serial++);
    ensure_chronicle_house(house_id,name);
    active_house_id_=house_id;
    active_house_name_=name;
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
    // chronicles.js starter kit: a clean bronze dagger and road gold.
    { bool has_dagger=false; int coins=0;
      for (const auto& item:inventory_.items()) { if (item.id=="bronze-dagger") has_dagger=true; if (item.id=="coins") coins+=item.qty; }
      if (!has_dagger) { CreateItemOptions o; auto dagger=create_game_item("bronze-dagger",o); if (dagger) inventory_.add(std::move(*dagger)); }
      if (coins<100) { CreateItemOptions o; o.quantity=100-coins; auto purse=create_game_item("coins",o); if (purse) inventory_.add(std::move(*purse)); }
    }
    world_->reset_to_town();
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
    // A new scion starts with the fresh-scion profile (purse only), never a
    // duplicate of the previous scion's equipment.
    wear_.clear(); inventory_.clear();
    CreateItemOptions purse; purse.quantity=100;
    auto coins=create_game_item("coins",purse); if (coins) inventory_.add(std::move(*coins));
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
  const auto listener=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); if(listener==invalid_socket){if(error)*error="socket failed";return false;} int yes=1; setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<const char*>(&yes),sizeof(yes)); sockaddr_in address{}; address.sin_family=AF_INET; address.sin_addr.s_addr=inet_addr("127.0.0.1"); address.sin_port=htons(port_); if(bind(listener,reinterpret_cast<sockaddr*>(&address),sizeof(address))<0 || listen(listener,16)<0){close_socket(listener);if(error)*error="bind/listen failed";return false;} listen_socket_=static_cast<std::intptr_t>(listener); running_=true; accept_thread_=std::make_unique<std::thread>(&WebSocketServer::accept_loop,this); return true;
}
void WebSocketServer::stop(){ if(!running_)return; running_=false; close_socket(static_cast<socket_t>(listen_socket_)); listen_socket_=-1; if(accept_thread_&&accept_thread_->joinable())accept_thread_->join(); std::lock_guard lock(mutex_); for(auto& c:connections_)c->close(); connections_.clear(); sessions_.clear();
#ifdef _WIN32
  WSACleanup();
#endif
}
void WebSocketServer::accept_loop(){ while(running_){ sockaddr_in address{}; socket_length_t length=sizeof(address); const auto client=::accept(static_cast<socket_t>(listen_socket_),reinterpret_cast<sockaddr*>(&address),&length); if(client==invalid_socket){if(running_)continue;break;} auto connection=std::make_shared<Connection>(); connection->socket=client; static std::atomic<std::uint64_t> serial{1}; connection->id="native-"+std::to_string(serial++); {std::lock_guard lock(mutex_);connections_.push_back(connection);} std::thread(&WebSocketServer::handle_connection,this,connection).detach(); } }
void WebSocketServer::handle_connection(std::shared_ptr<Connection> connection){ std::string headers; char buffer[1024]; while(headers.find("\r\n\r\n")==std::string::npos&&headers.size()<8192){ const auto got=recv(connection->socket,buffer,sizeof(buffer),0); if(got<=0){connection->close();remove_connection(connection);return;} headers.append(buffer,buffer+got); } const auto key_pos=headers.find("Sec-WebSocket-Key:"); if(key_pos==std::string::npos){connection->close();remove_connection(connection);return;} auto start=key_pos+18; while(start<headers.size()&&headers[start]==' ')++start; auto end=headers.find("\r\n",start); const auto key=headers.substr(start,end-start); const std::string response="HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: "+ws_accept_key(key)+"\r\n\r\n"; if(!send_all(connection->socket,response.data(),response.size())){connection->close();remove_connection(connection);return;} while(running_&&!connection->closed){ std::uint8_t header[2];if(!recv_all(connection->socket,header,2))break; const auto opcode=header[0]&0x0f; bool masked=(header[1]&0x80)!=0; std::uint64_t length=header[1]&0x7f; if(length==126){std::uint8_t ext[2];if(!recv_all(connection->socket,ext,2))break;length=(ext[0]<<8)|ext[1];}else if(length==127){std::uint8_t ext[8];if(!recv_all(connection->socket,ext,8))break;length=0;for(auto byte:ext)length=(length<<8)|byte;} if(length>16384||!masked)break; std::array<std::uint8_t,4> mask{};if(!recv_all(connection->socket,mask.data(),4))break;std::string payload(length,'\0');if(!recv_all(connection->socket,payload.data(),length))break;for(std::size_t i=0;i<length;++i)payload[i]^=mask[i%4]; if(opcode==8)break;if(opcode==9){std::vector<std::uint8_t> pong{0x8a,static_cast<std::uint8_t>(length)};pong.insert(pong.end(),payload.begin(),payload.end());send_all(connection->socket,pong.data(),pong.size());continue;}if(opcode==1)handle_message(connection,payload); } connection->close();remove_connection(connection); }
void WebSocketServer::handle_message(const std::shared_ptr<Connection>& connection,const std::string& text){ Envelope envelope; std::string error;if(!parse_envelope(text,envelope,&error))return; if(envelope.event=="player:login"){const auto* guest=envelope.data.get("guestId");const auto identity=(guest&&guest->string())?*guest->string():"default-guest";const bool quick=as_bool(envelope.data.get("quickGuest"));std::shared_ptr<ProtocolSession> session;std::shared_ptr<Connection> old;{std::lock_guard lock(mutex_);auto it=sessions_.find(identity);if(it!=sessions_.end()){for(const auto& candidate:connections_)if(candidate->session==it->second&&candidate!=connection&&!candidate->closed){old=candidate;break;}session=it->second;}if(!session){std::uint64_t seed=1469598103934665603ULL;for(unsigned char c:identity)seed=(seed^c)*1099511628211ULL;session=std::make_shared<ProtocolSession>(identity,connection->id,seed,quick);sessions_[identity]=session;}else { const bool adopted = connection->session != session; session->replace_socket(connection->id); if (adopted) session->reset_world_for_new_socket(); } connection->session=session;}session->set_broadcast([this](const Envelope& event){broadcast(event);});if(old){old->send_text(emit_envelope(Envelope{"player:session-replaced",JsonValue::Object{{"player",JsonValue::Object{{"socket_id",old->id}}}}}));old->shutdown_send();old->close();}session->handle(envelope,[connection](const Envelope& response){connection->send_text(emit_envelope(response));});return;} auto session=connection->session;if(!session)return;session->handle(envelope,[connection](const Envelope& response){connection->send_text(emit_envelope(response));});}
void WebSocketServer::broadcast(const Envelope& envelope){ std::vector<std::shared_ptr<Connection>> targets; {std::lock_guard lock(mutex_);targets=connections_;} const auto wire=emit_envelope(envelope); for(const auto& candidate:targets) if(candidate->session&&!candidate->closed) candidate->send_text(wire); }
void WebSocketServer::remove_connection(const std::shared_ptr<Connection>& connection){std::lock_guard lock(mutex_);connections_.erase(std::remove(connections_.begin(),connections_.end(),connection),connections_.end());}

}  // namespace verdigris::networking
