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
    : identity_(std::move(identity)), socket_id_(std::move(socket_id)), quick_start_(quick_start), simulation_(std::make_unique<Simulation>(seed, "House Verdigris")), world_(std::make_unique<WorldSimulation>(seed, identity_)) {}
void ProtocolSession::replace_socket(std::string socket_id) { std::lock_guard lock(mutex_); socket_id_=std::move(socket_id); }
void ProtocolSession::set_broadcast(std::function<void(const Envelope&)> broadcast) { std::lock_guard lock(mutex_); broadcast_=std::move(broadcast); }
std::int64_t ProtocolSession::now_ms() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
std::string ProtocolSession::player_payload() const {
  JsonValue::Object player; const auto position=world_->position();
  put(player,"uuid",identity_); put(player,"socket_id",socket_id_); put(player,"sceneId",world_->scene_id()); put(player,"x",position.x); put(player,"y",position.y); put(player,"facing",world_->facing());
  JsonValue::Array slots; for (const auto& item:inventory_) { JsonValue::Object value; put(value,"id",item.id); put(value,"uuid",item.id+"-"+identity_); put(value,"name",item.name); put(value,"slot",item.equipped?"weapon":"inventory"); slots.emplace_back(std::move(value)); }
  JsonValue::Object inventory; put(inventory,"slots",std::move(slots)); put(player,"inventory",std::move(inventory)); return JsonValue(std::move(player)).stringify();
}
std::string ProtocolSession::login_payload() const {
  std::lock_guard lock(mutex_); JsonValue::Object data; JsonValue player; parse_json(player_payload(),player); put(data,"player",std::move(player)); put(data,"scene",scene_payload()); if (quick_start_) put(data,"quickStart",true); return JsonValue(std::move(data)).stringify();
}
JsonValue ProtocolSession::scene_payload() const {
  JsonValue::Object scene; put(scene,"id",world_->scene_id()); put(scene,"type",world_->scene_type()); put(scene,"name",world_->scene_name());
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
  put(state,"lifecycle",actor && actor->alive && actor->stats.life > 0 ? "alive" : "dead");
  JsonValue::Object hp; put(hp,"current",actor?actor->stats.life:0); put(hp,"max",actor?actor->stats.life_max:0); put(state,"hp",std::move(hp));
  JsonValue::Array monsters; for (const auto& candidate:world_->monsters()) if (candidate.alive) {
    JsonValue::Object monster; put(monster,"uuid",candidate.uuid); put(monster,"id",candidate.id); put(monster,"name",candidate.name);
    put(monster,"x",candidate.x); put(monster,"y",candidate.y); put(monster,"level",candidate.level); put(monster,"rarity",candidate.rarity);
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
  JsonValue::Array items; for (const auto& item:inventory_) { JsonValue::Object value; put(value,"id",item.id); put(value,"uuid",item.id+"-"+identity_); put(value,"name",item.name); put(value,"slot",item.equipped?"weapon":"inventory"); items.emplace_back(std::move(value)); } put(state,"inventory",std::move(items));
  put(state,"groundItems",ground_items_); put(state,"groundTrophies",JsonValue::Array{}); return JsonValue(std::move(state));
}
std::string ProtocolSession::state_payload(const std::string& request_id) const { std::lock_guard lock(mutex_); JsonValue::Object data; put(data,"player",JsonValue::Object{{"socket_id",socket_id_}}); put(data,"state",snapshot()); put(data,"requestId",request_id); return JsonValue(std::move(data)).stringify(); }
void ProtocolSession::grant_item(const std::string& item_id, int quantity) { for (int i=0;i<(std::max)(1,quantity);++i) { Item item; item.id=item_id; item.name=item_id; item.owner_id=simulation_->scion().id; inventory_.push_back(std::move(item)); } }
void ProtocolSession::emit_combat_event(const WorldCombatEvent& event, const std::function<void(const Envelope&)>& emit) {
  if (event.type == "telegraph") {
    JsonValue::Object data; put(data,"attackerId",event.attacker_id); put(data,"attackerName",event.attacker_name); put(data,"skillId",event.skill_id);
    put(data,"x",event.x); put(data,"y",event.y); put(data,"radius",event.radius); put(data,"durationMs",event.duration_ms);
    emit_world(Envelope{"monster:telegraph",JsonValue(std::move(data))},emit); return;
  }
  if (event.type == "drop") {
    JsonValue::Object item; put(item,"id",event.item_id); put(item,"uuid",event.item_id+"-"+event.target_id); put(item,"name",event.item_id); put(item,"x",event.x); put(item,"y",event.y);
    ground_items_.push_back(JsonValue(std::move(item))); return;
  }
  JsonValue::Object data; put(data,"attackerId",event.attacker_id); put(data,"attackerName",event.attacker_name); put(data,"targetId",event.target_id);
  put(data,"targetName",event.target_name); put(data,"targetType",event.target_id==identity_?"player":"monster"); put(data,"skillId",event.skill_id);
  put(data,"amount",event.amount); put(data,"died",event.died); put(data,"health",JsonValue::Object{{"current",event.health},{"max",event.health_max}});
  emit_world(Envelope{"combat:hit",JsonValue(std::move(data))},emit);
}
void ProtocolSession::process_combat(std::int64_t now, const std::function<void(const Envelope&)>& emit) {
  auto* actor = simulation_->actor(simulation_->scion().actor_id); if (!actor) return;
  const auto events = world_->advance_combat(actor->stats.level, actor->stats.attack, actor->stats.life, actor->stats.life_max, now);
  for (const auto& event : events) emit_combat_event(event, emit);
}
void ProtocolSession::emit_login(const std::function<void(const Envelope&)>& emit) const { Envelope response{"player:login",JsonValue::Object{}}; parse_json(login_payload(),response.data); emit(response); }
void ProtocolSession::emit_world(const Envelope& envelope, const std::function<void(const Envelope&)>& emit) const { if (broadcast_) broadcast_(envelope); else emit(envelope); }
void ProtocolSession::emit_transition(const std::function<void(const Envelope&)>& emit, const char* event) const { JsonValue::Object data; put(data,"player",JsonValue::Object{{"socket_id",socket_id_}}); put(data,"scene",scene_payload()); JsonValue player_state; parse_json(player_payload(),player_state); JsonValue::Object state_fields; if (const auto* fields=player_state.object()) { for (const auto& key:{"uuid","x","y","sceneId"}) if (const auto* field=player_state.get(key)) put(state_fields,key,*field); } put(data,"playerState",std::move(state_fields)); emit_world(Envelope{event,JsonValue(std::move(data))},emit); }
void ProtocolSession::emit_movement(const std::function<void(const Envelope&)>& emit) const { JsonValue data; parse_json(player_payload(),data); Envelope movement{"player:movement",std::move(data)}; movement.meta=movement_step_payload(); emit_world(movement,emit); }
void ProtocolSession::emit_message(const std::function<void(const Envelope&)>& emit, const std::string& text) const { emit(Envelope{"game:send:message",JsonValue::Object{{"text",text}}}); }
void ProtocolSession::handle(const Envelope& envelope, const std::function<void(const Envelope&)>& emit) {
  const auto* payload=envelope.data.object()?&envelope.data:nullptr;
  if (envelope.event=="world:zone:enter") { const auto node=as_string(payload?payload->get("nodeId"):nullptr,"tin:1:0"); simulation_->dispatch(Command::enter(node.rfind("route:",0)==0?node:"route:"+node)); world_->enter_solo_instance("dungeon",""); emit_transition(emit,"world:scene:transition"); return; }
  if (envelope.event=="instance:enterSolo") { world_->enter_solo_instance(as_string(payload?payload->get("template"):nullptr,"dungeon"),as_string(payload?payload->get("layout"):nullptr,"")); emit_transition(emit,"party:scene:transition"); return; }
  if (envelope.event=="player:move") { const auto direction=as_string(payload?payload->get("direction"):nullptr); const bool was_instance=world_->in_instance(); if (world_->apply_movement_sample(direction,now_ms())) { emit_movement(emit); if (was_instance&&!world_->in_instance()) { emit_message(emit,"The party returns to the surface."); emit_transition(emit,"party:scene:transition"); } } return; }
  if (envelope.event=="dev:teleport") { if (!payload) return; const auto* x=payload->get("x"); const auto* y=payload->get("y"); if (!x||!x->number()||!y||!y->number()) return; const int tx=static_cast<int>(*x->number()); const int ty=static_cast<int>(*y->number()); const bool was_instance=world_->in_instance(); world_->teleport(tx,ty,now_ms()); const bool returned=was_instance&&!world_->in_instance(); emit_movement(emit); emit_message(emit,"Teleported to "+std::to_string(tx)+", "+std::to_string(ty)+(returned?" (portal followed).":".")); if (returned) { emit_message(emit,"The party returns to the surface."); emit_transition(emit,"party:scene:transition"); } if (!returned) process_combat(now_ms(),emit); return; }
  if (envelope.event=="dev:setlevel") { auto* actor=simulation_->actor(simulation_->scion().actor_id); const int level=as_int(payload?payload->get("level"):nullptr,1); if(actor){ actor->stats.level=(std::max)(1,level); actor->stats.attack=12+actor->stats.level*3; actor->stats.life_max=100+actor->stats.level*10; actor->stats.life=actor->stats.life_max; world_->set_level(actor->stats.level); } return; }
  if (envelope.event=="dev:heal") { auto* actor=simulation_->actor(simulation_->scion().actor_id); if(actor) world_->heal_player(actor->stats.life,actor->stats.life_max); return; }
  if (envelope.event=="player:skill:trigger") { auto* actor=simulation_->actor(simulation_->scion().actor_id); if(actor&&world_->in_instance()){ const auto direction=as_string(payload?payload->get("direction"):nullptr,"down"); world_->start_player_attack(actor->stats.level,actor->stats.attack,now_ms(),direction); process_combat(now_ms(),emit); } return; }
  if (envelope.event=="dev:give") { grant_item(as_string(payload?payload->get("itemId"):nullptr,"garnet-amulet"),as_int(payload?payload->get("qty"):nullptr,1)); return; }
  if (envelope.event=="dev:state") { process_combat(now_ms(),emit); const auto id=as_string(payload?payload->get("requestId"):nullptr); JsonValue data; parse_json(state_payload(id),data); emit(Envelope{"dev:state",std::move(data)}); return; }
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
void WebSocketServer::handle_message(const std::shared_ptr<Connection>& connection,const std::string& text){ Envelope envelope; std::string error;if(!parse_envelope(text,envelope,&error))return; if(envelope.event=="player:login"){const auto* guest=envelope.data.get("guestId");const auto identity=(guest&&guest->string())?*guest->string():"default-guest";const bool quick=as_bool(envelope.data.get("quickGuest"));std::shared_ptr<ProtocolSession> session;std::shared_ptr<Connection> old;{std::lock_guard lock(mutex_);auto it=sessions_.find(identity);if(it!=sessions_.end()){for(const auto& candidate:connections_)if(candidate->session==it->second&&candidate!=connection&&!candidate->closed){old=candidate;break;}if(old)session=it->second;}if(!session){std::uint64_t seed=1469598103934665603ULL;for(unsigned char c:identity)seed=(seed^c)*1099511628211ULL;session=std::make_shared<ProtocolSession>(identity,connection->id,seed,quick);sessions_[identity]=session;}else session->replace_socket(connection->id);connection->session=session;}session->set_broadcast([this](const Envelope& event){broadcast(event);});if(old){old->send_text(emit_envelope(Envelope{"player:session-replaced",JsonValue::Object{{"player",JsonValue::Object{{"socket_id",old->id}}}}}));old->close();}session->handle(envelope,[connection](const Envelope& response){connection->send_text(emit_envelope(response));});return;} auto session=connection->session;if(!session)return;session->handle(envelope,[connection](const Envelope& response){connection->send_text(emit_envelope(response));});}
void WebSocketServer::broadcast(const Envelope& envelope){ std::vector<std::shared_ptr<Connection>> targets; {std::lock_guard lock(mutex_);targets=connections_;} const auto wire=emit_envelope(envelope); for(const auto& candidate:targets) if(candidate->session&&!candidate->closed) candidate->send_text(wire); }
void WebSocketServer::remove_connection(const std::shared_ptr<Connection>& connection){std::lock_guard lock(mutex_);connections_.erase(std::remove(connections_.begin(),connections_.end(),connection),connections_.end());}

}  // namespace verdigris::networking
