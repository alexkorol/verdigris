#pragma once

// VG-MOVE-006: versioned keyboard bindings. Duplicate codes and unknown
// devices fail closed. Owner Documents paths are never the test profile.

#include <cstdio>
#include <cstring>
#include <string>

namespace verdigris::client::input {

inline constexpr int kBindingSchemaVersion = 1;

enum class Action : unsigned char {
  MoveN = 0,
  MoveW,
  MoveS,
  MoveE,
  Dash,
  Thrust,
  Sweep,
  WarCry,
  Count
};

enum class Device : unsigned char {
  Keyboard = 1,
  Mouse = 2,
  Pad = 3
};

enum class BindStatus : unsigned char {
  Ok = 0,
  Conflict,
  InvalidDevice,
  Inaccessible,
  OwnerProfile,
  IoError
};

struct Bindings {
  int version = kBindingSchemaVersion;
  int move_n = 'W';
  int move_w = 'A';
  int move_s = 'S';
  int move_e = 'D';
  int dash = 0x20;  // VK_SPACE
  int thrust = 'Q';
  int sweep = 'E';
  int warcry = 'R';
  Device device = Device::Keyboard;
};

inline Bindings default_bindings() { return {}; }

inline const char* bind_hud_label(BindStatus status) {
  switch (status) {
    case BindStatus::Ok:
      return "bind:ok";
    case BindStatus::Conflict:
      return "bind:conflict";
    case BindStatus::InvalidDevice:
      return "bind:invalid-device";
    case BindStatus::Inaccessible:
      return "bind:inaccessible";
    case BindStatus::OwnerProfile:
      return "bind:owner-profile";
    case BindStatus::IoError:
      return "bind:io";
  }
  return "bind:unknown";
}

inline bool path_is_owner_profile(const std::string& path) {
  std::string lower;
  lower.reserve(path.size());
  for (unsigned char c : path) {
    if (c >= 'A' && c <= 'Z') lower.push_back(static_cast<char>(c - 'A' + 'a'));
    else if (c == '/') lower.push_back('\\');
    else lower.push_back(static_cast<char>(c));
  }
  return lower.find("\\documents\\") != std::string::npos ||
         lower.find("\\my games\\") != std::string::npos;
}

inline int* slot(Bindings& bindings, Action action) {
  switch (action) {
    case Action::MoveN:
      return &bindings.move_n;
    case Action::MoveW:
      return &bindings.move_w;
    case Action::MoveS:
      return &bindings.move_s;
    case Action::MoveE:
      return &bindings.move_e;
    case Action::Dash:
      return &bindings.dash;
    case Action::Thrust:
      return &bindings.thrust;
    case Action::Sweep:
      return &bindings.sweep;
    case Action::WarCry:
      return &bindings.warcry;
    case Action::Count:
      break;
  }
  return nullptr;
}

inline const int* slot(const Bindings& bindings, Action action) {
  return slot(const_cast<Bindings&>(bindings), action);
}

inline bool codes_unique(const Bindings& bindings) {
  const int codes[] = {bindings.move_n, bindings.move_w, bindings.move_s,
                       bindings.move_e, bindings.dash,   bindings.thrust,
                       bindings.sweep,  bindings.warcry};
  for (int i = 0; i < 8; ++i) {
    if (codes[i] <= 0) return false;
    for (int j = i + 1; j < 8; ++j)
      if (codes[i] == codes[j]) return false;
  }
  return true;
}

inline BindStatus validate(const Bindings& bindings, Device device) {
  if (device != Device::Keyboard && device != Device::Mouse &&
      device != Device::Pad)
    return BindStatus::InvalidDevice;
  if (bindings.version != kBindingSchemaVersion) return BindStatus::Inaccessible;
  if (!codes_unique(bindings)) return BindStatus::Conflict;
  return BindStatus::Ok;
}

inline BindStatus remap(Bindings& bindings, Action action, Device device, int code) {
  if (device != Device::Keyboard && device != Device::Mouse &&
      device != Device::Pad)
    return BindStatus::InvalidDevice;
  if (code <= 0) return BindStatus::InvalidDevice;
  int* dest = slot(bindings, action);
  if (!dest) return BindStatus::Inaccessible;
  Bindings next = bindings;
  next.device = device;
  *slot(next, action) = code;
  const BindStatus status = validate(next, device);
  if (status != BindStatus::Ok) return status;
  bindings = next;
  return BindStatus::Ok;
}

inline BindStatus save_bindings(const std::string& path, const Bindings& bindings) {
  if (path.empty()) return BindStatus::IoError;
  if (path_is_owner_profile(path)) return BindStatus::OwnerProfile;
  if (validate(bindings, bindings.device) != BindStatus::Ok)
    return validate(bindings, bindings.device);
  FILE* file = nullptr;
#if defined(_MSC_VER)
  if (fopen_s(&file, path.c_str(), "wb") != 0 || !file) return BindStatus::IoError;
#else
  file = std::fopen(path.c_str(), "wb");
  if (!file) return BindStatus::IoError;
#endif
  std::fprintf(file,
               "version=%d\ndevice=%d\nmove_n=%d\nmove_w=%d\nmove_s=%d\n"
               "move_e=%d\ndash=%d\nthrust=%d\nsweep=%d\nwarcry=%d\n",
               bindings.version, static_cast<int>(bindings.device),
               bindings.move_n, bindings.move_w, bindings.move_s, bindings.move_e,
               bindings.dash, bindings.thrust, bindings.sweep, bindings.warcry);
  std::fclose(file);
  return BindStatus::Ok;
}

inline BindStatus load_bindings(const std::string& path, Bindings& out) {
  if (path.empty()) return BindStatus::IoError;
  if (path_is_owner_profile(path)) return BindStatus::OwnerProfile;
  FILE* file = nullptr;
#if defined(_MSC_VER)
  if (fopen_s(&file, path.c_str(), "rb") != 0 || !file) return BindStatus::IoError;
#else
  file = std::fopen(path.c_str(), "rb");
  if (!file) return BindStatus::IoError;
#endif
  char buf[512];
  const std::size_t n = std::fread(buf, 1, sizeof(buf) - 1, file);
  std::fclose(file);
  buf[n] = '\0';
  const std::string text(buf);
  auto take = [&](const char* key, int fallback) {
    const std::string needle = std::string(key) + "=";
    const auto at = text.find(needle);
    if (at == std::string::npos) return fallback;
    return std::atoi(text.c_str() + at + needle.size());
  };
  Bindings next = default_bindings();
  next.version = take("version", 0);
  next.device = static_cast<Device>(take("device", 1));
  next.move_n = take("move_n", next.move_n);
  next.move_w = take("move_w", next.move_w);
  next.move_s = take("move_s", next.move_s);
  next.move_e = take("move_e", next.move_e);
  next.dash = take("dash", next.dash);
  next.thrust = take("thrust", next.thrust);
  next.sweep = take("sweep", next.sweep);
  next.warcry = take("warcry", next.warcry);
  const BindStatus status = validate(next, next.device);
  if (status != BindStatus::Ok) return status;
  out = next;
  return BindStatus::Ok;
}

inline bool matches(const Bindings& bindings, Action action, int code) {
  const int* value = slot(bindings, action);
  return value && *value == code;
}

inline const char* owner_isolated_profile_label() { return "Isolated profile"; }
inline const char* owner_dash_remap_label() { return "Dash remap"; }

}  // namespace verdigris::client::input
