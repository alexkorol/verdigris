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
void ProtocolSession::set_broadcast(std::function<void(const Envelope&)> broadcast) { std::lock_guard lock(mutex_); broadcast_=std::move(broadcast); }
std::int64_t ProtocolSession::now_ms() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
std::string ProtocolSession::player_payload() const {
  JsonValue::Object player; const auto position=world_->position();
  put(player,"uuid",identity_); put(player,"socket_id",socket_id_); put(player,"sceneId",world_->scene_id()); put(player,"x",position.x); put(player,"y",position.y); put(player,"facing",world_->facing());
  JsonValue::Array slots; for (const auto& item:inventory_.items()) { JsonValue::Object value; put(value,"id",item.id); put(value,"uuid",item.uuid); put(value,"name",item.name); if(item.slot>=0) put(value,"slot",item.slot); else put(value,"slot",nullptr); slots.emplace_back(std::move(value)); }
  JsonValue::Object inventory; put(inventory,"slots",std::move(slots)); put(player,"inventory",std::move(inventory)); return JsonValue(std::move(player)).stringify();
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
  GameItem item;
  if (!world_->take_ground_item(uuid,&item)) return;
  item.slot=-1;
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
  const auto events = world_->advance_combat(actor->stats.level, actor->stats.attack, actor->stats.life, actor->stats.life_max, now);
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
  }
  if (loot) emit_ground_change(emit);
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
  emit_transition(emit, "party:scene:transition");
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
void ProtocolSession::emit_login(const std::function<void(const Envelope&)>& emit) const { Envelope response{"player:login",JsonValue::Object{}}; parse_json(login_payload(),response.data); emit(response); }
void ProtocolSession::emit_world(const Envelope& envelope, const std::function<void(const Envelope&)>& emit) const { if (broadcast_) broadcast_(envelope); else emit(envelope); }
void ProtocolSession::emit_transition(const std::function<void(const Envelope&)>& emit, const char* event) const { JsonValue::Object data; put(data,"player",JsonValue::Object{{"socket_id",socket_id_}}); put(data,"scene",scene_payload()); JsonValue player_state; parse_json(player_payload(),player_state); JsonValue::Object state_fields; if (const auto* fields=player_state.object()) { for (const auto& key:{"uuid","x","y","sceneId"}) if (const auto* field=player_state.get(key)) put(state_fields,key,*field); } put(data,"playerState",std::move(state_fields)); emit_world(Envelope{event,JsonValue(std::move(data))},emit); }
void ProtocolSession::emit_movement(const std::function<void(const Envelope&)>& emit) const { JsonValue data; parse_json(player_payload(),data); Envelope movement{"player:movement",std::move(data)}; movement.meta=movement_step_payload(); emit_world(movement,emit); }
void ProtocolSession::emit_message(const std::function<void(const Envelope&)>& emit, const std::string& text) const { emit(Envelope{"game:send:message",JsonValue::Object{{"text",text}}}); }
void ProtocolSession::handle(const Envelope& envelope, const std::function<void(const Envelope&)>& emit) {
  const auto* payload=envelope.data.object()?&envelope.data:nullptr;
  if (envelope.event=="world:zone:enter") { const auto node=as_string(payload?payload->get("nodeId"):nullptr,"tin:1:0"); simulation_->dispatch(Command::enter(node.rfind("route:",0)==0?node:"route:"+node)); world_->enter_solo_instance("dungeon",""); emit_transition(emit,"world:scene:transition"); emit_ground_change(emit); return; }
  if (envelope.event=="instance:enterSolo") { world_->enter_solo_instance(as_string(payload?payload->get("template"):nullptr,"dungeon"),as_string(payload?payload->get("layout"):nullptr,"")); emit_transition(emit,"party:scene:transition"); emit_ground_change(emit); return; }
  if (envelope.event=="player:move") { const auto direction=as_string(payload?payload->get("direction"):nullptr); const bool was_instance=world_->in_instance(); const int depth_before=world_->metadata().depth; const std::string scene_before=world_->scene_id(); if (world_->apply_movement_sample(direction,now_ms())) { emit_movement(emit); const bool depth_changed=world_->in_instance()&&world_->metadata().depth!=depth_before; const bool scene_changed=world_->scene_id()!=scene_before; if (depth_changed||scene_changed) { if (was_instance&&!world_->in_instance()) { emit_message(emit,"The party returns to the surface."); finish_extraction(emit); } emit_transition(emit,"party:scene:transition"); if (world_->in_instance()) emit_ground_change(emit); } } return; }
  if (envelope.event=="dev:teleport") { if (!payload) return; const auto* x=payload->get("x"); const auto* y=payload->get("y"); if (!x||!x->number()||!y||!y->number()) return; const int tx=static_cast<int>(*x->number()); const int ty=static_cast<int>(*y->number()); const bool was_instance=world_->in_instance(); const int depth_before=world_->metadata().depth; const std::string scene_before=world_->scene_id(); world_->teleport(tx,ty,now_ms()); const bool returned=was_instance&&!world_->in_instance(); const bool depth_changed=world_->in_instance()&&world_->metadata().depth!=depth_before; const bool transitioned=returned||depth_changed||world_->scene_id()!=scene_before; emit_movement(emit); emit_message(emit,"Teleported to "+std::to_string(tx)+", "+std::to_string(ty)+(transitioned?" (portal followed).":".")); if (returned) { emit_message(emit,"The party returns to the surface."); finish_extraction(emit); } if (transitioned) emit_transition(emit,"party:scene:transition"); if (world_->in_instance()) { if (depth_changed) emit_ground_change(emit); process_combat(now_ms(),emit); } return; }
  if (envelope.event=="dev:setlevel") { auto* actor=simulation_->actor(simulation_->scion().actor_id); const int level=as_int(payload?payload->get("level"):nullptr,1); if(actor){ actor->stats.level=(std::max)(1,level); actor->stats.attack=12+actor->stats.level*3; actor->stats.life_max=100+actor->stats.level*10; actor->stats.life=actor->stats.life_max; world_->set_level(actor->stats.level); } return; }
  if (envelope.event=="dev:heal") { auto* actor=simulation_->actor(simulation_->scion().actor_id); if(actor) world_->heal_player(actor->stats.life,actor->stats.life_max); return; }
  if (envelope.event=="dev:kill") {
    // Soft death (lifecycle.js): first lethal hit enters awaiting-respawn;
    // further hits while already down neither count nor delay the respawn.
    auto* actor=simulation_->actor(simulation_->scion().actor_id);
    if (actor) actor->stats.life = 0;
    if (lifecycle_ == "alive") {
      lifecycle_ = "awaiting-respawn";
      lifecycle_deaths_ += 1;
      respawn_at_ms_ = now_ms() + 2000;
      respawn_protection_until_ms_ = 0;
    }
    emit_message(emit,"You have been slain.");
    return;
  }
  if (envelope.event=="player:skill:trigger") { auto* actor=simulation_->actor(simulation_->scion().actor_id); if(actor&&world_->in_instance()){ if (respawn_protection_until_ms_ > 0) respawn_protection_until_ms_ = 0; const auto direction=as_string(payload?payload->get("direction"):nullptr,"down"); world_->start_player_attack(actor->stats.level,actor->stats.attack,now_ms(),direction); process_combat(now_ms(),emit); } return; }
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
  if (envelope.event=="player:take:underfoot") { handle_take_underfoot(emit); return; }
  if (envelope.event=="player:context-menu:build") { if (payload) handle_menu_build(*payload,emit); return; }
  if (envelope.event=="player:context-menu:action") { if (payload) handle_menu_action(*payload,emit); return; }
  if (envelope.event=="player:inventory:commit") { if (payload) handle_inventory_commit(*payload,emit); return; }
  if (envelope.event=="dev:state") { maybe_respawn(now_ms()); process_combat(now_ms(),emit); const auto id=as_string(payload?payload->get("requestId"):nullptr); JsonValue data; parse_json(state_payload(id),data); emit(Envelope{"dev:state",std::move(data)}); return; }
  if (envelope.event=="player:login") emit_login(emit);
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
void WebSocketServer::handle_message(const std::shared_ptr<Connection>& connection,const std::string& text){ Envelope envelope; std::string error;if(!parse_envelope(text,envelope,&error))return; if(envelope.event=="player:login"){const auto* guest=envelope.data.get("guestId");const auto identity=(guest&&guest->string())?*guest->string():"default-guest";const bool quick=as_bool(envelope.data.get("quickGuest"));std::shared_ptr<ProtocolSession> session;std::shared_ptr<Connection> old;{std::lock_guard lock(mutex_);auto it=sessions_.find(identity);if(it!=sessions_.end()){for(const auto& candidate:connections_)if(candidate->session==it->second&&candidate!=connection&&!candidate->closed){old=candidate;break;}session=it->second;}if(!session){std::uint64_t seed=1469598103934665603ULL;for(unsigned char c:identity)seed=(seed^c)*1099511628211ULL;session=std::make_shared<ProtocolSession>(identity,connection->id,seed,quick);sessions_[identity]=session;}else session->replace_socket(connection->id);connection->session=session;}session->set_broadcast([this](const Envelope& event){broadcast(event);});if(old){old->send_text(emit_envelope(Envelope{"player:session-replaced",JsonValue::Object{{"player",JsonValue::Object{{"socket_id",old->id}}}}}));old->shutdown_send();old->close();}session->handle(envelope,[connection](const Envelope& response){connection->send_text(emit_envelope(response));});return;} auto session=connection->session;if(!session)return;session->handle(envelope,[connection](const Envelope& response){connection->send_text(emit_envelope(response));});}
void WebSocketServer::broadcast(const Envelope& envelope){ std::vector<std::shared_ptr<Connection>> targets; {std::lock_guard lock(mutex_);targets=connections_;} const auto wire=emit_envelope(envelope); for(const auto& candidate:targets) if(candidate->session&&!candidate->closed) candidate->send_text(wire); }
void WebSocketServer::remove_connection(const std::shared_ptr<Connection>& connection){std::lock_guard lock(mutex_);connections_.erase(std::remove(connections_.begin(),connections_.end(),connection),connections_.end());}

}  // namespace verdigris::networking
