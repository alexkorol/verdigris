#pragma once

// User preferences are independent of game saves and the installation folder.
#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace verdigris::audio {

struct AudioPrefs {
  bool muted = false;
  int sfx_permille = 1000;
  int music_permille = 1000;
};

enum class SettingsStatus { Loaded, Missing, Corrupt, UnsupportedVersion, IoError };
struct SettingsLoadResult {
  AudioPrefs prefs;
  SettingsStatus status = SettingsStatus::Missing;
  std::string message;
};
struct SettingsSaveResult {
  bool ok = false;
  std::string message;
};

namespace settings_detail {
namespace fs = std::filesystem;
constexpr std::size_t kMaxBytes = 64 * 1024;
inline std::string utf8(const fs::path& path) {
  const auto value = path.u8string();
  return std::string(value.begin(), value.end());
}
inline fs::path from_utf8(const std::string& value) {
  return fs::path(std::u8string(value.begin(), value.end()));
}
inline fs::path environment_path(const char* key) {
#ifdef _WIN32
  const std::wstring wide(key, key + std::char_traits<char>::length(key));
  const DWORD size = GetEnvironmentVariableW(wide.c_str(), nullptr, 0);
  if (!size) return {};
  std::wstring value(size, L'\0');
  const DWORD copied = GetEnvironmentVariableW(wide.c_str(), value.data(), size);
  if (!copied || copied >= size) return {};
  value.resize(copied);
  return fs::path(value);
#else
  const char* value = std::getenv(key);
  return value ? fs::path(value) : fs::path{};
#endif
}
inline bool integer(const std::string& text, int& result) {
  const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
  return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size();
}
struct Document {
  SettingsLoadResult loaded;
  std::vector<std::string> unrelated;
};
inline Document read(const fs::path& path) {
  Document doc;
  auto fail = [&](SettingsStatus status, const char* message) {
    doc.loaded = {AudioPrefs{}, status, message};
    return doc;
  };
  if (path.empty()) return fail(SettingsStatus::IoError, "Settings location is unavailable.");
  std::error_code error;
  const auto status = fs::status(path, error);
  if (status.type() == fs::file_type::not_found) return doc;
  if (error || !fs::is_regular_file(status))
    return fail(SettingsStatus::IoError, "Settings could not be read; the original is unchanged.");
  const auto size = fs::file_size(path, error);
  if (error) return fail(SettingsStatus::IoError, "Settings could not be read; the original is unchanged.");
  if (size > kMaxBytes) return fail(SettingsStatus::Corrupt, "Settings file is too large; audio defaults are active.");
  std::ifstream stream(path, std::ios::binary);
  if (!stream) return fail(SettingsStatus::IoError, "Settings could not be read; the original is unchanged.");
  std::string text(static_cast<std::size_t>(size), '\0');
  if (!text.empty()) stream.read(text.data(), static_cast<std::streamsize>(size));
  if (!stream || stream.peek() != std::char_traits<char>::eof())
    return fail(SettingsStatus::IoError, "Settings changed or could not be read; try again.");
  std::map<std::string, std::string> values;
  bool malformed = text.empty() || text.find('\0') != std::string::npos;
  for (std::size_t offset = 0; offset < text.size();) {
    const auto end = text.find('\n', offset);
    std::string line = text.substr(offset, end == std::string::npos ? end : end - offset);
    offset = end == std::string::npos ? text.size() : end + 1;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.empty() || line.front() == '#' || line.front() == ';') {
      doc.unrelated.push_back(line);
      continue;
    }
    const auto equal = line.find('=');
    if (equal == std::string::npos || equal == 0) { malformed = true; continue; }
    const auto key = line.substr(0, equal);
    const auto value = line.substr(equal + 1);
    if (!values.emplace(key, value).second) malformed = true;
    if (key != "version" && key != "muted" && key != "sfx" && key != "music")
      doc.unrelated.push_back(line);
  }
  int version = 0;
  if (values.contains("version")) {
    if (!integer(values["version"], version) || version < 1) malformed = true;
    else if (version > 1)
      return fail(SettingsStatus::UnsupportedVersion, "Settings were written by a newer game; changes cannot be saved.");
  }
  // The former audio-only format is version 0. Read it, upgrade on next save.
  if (!values.contains("muted") || !values.contains("sfx") || !values.contains("music"))
    malformed = true;
  int muted = 0, sfx = 1000, music = 1000;
  if (!integer(values["muted"], muted) || (muted != 0 && muted != 1) ||
      !integer(values["sfx"], sfx) || sfx < 0 || sfx > 1000 ||
      !integer(values["music"], music) || music < 0 || music > 1000) malformed = true;
  if (malformed) return fail(SettingsStatus::Corrupt, "Settings are damaged; audio defaults are active. Saving keeps a backup.");
  doc.loaded = {{muted != 0, sfx, music}, SettingsStatus::Loaded, {}};
  return doc;
}
inline fs::path unique_sibling(const fs::path& path, const char* suffix) {
  static std::atomic<unsigned long long> counter{0};
  const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
#ifdef _WIN32
  const auto process = GetCurrentProcessId();
#else
  const auto process = getpid();
#endif
  fs::path next = path;
  next += std::string(suffix) + std::to_string(process) + "-" +
          std::to_string(tick) + "-" + std::to_string(counter++);
  return next;
}
// Commit through an exclusive sibling and atomic replacement. A failed write
// never truncates the previous preferences; the temporary file is removed.
inline bool atomic_write(const fs::path& path, const std::string& text) {
  const auto temporary = unique_sibling(path, ".tmp-");
  bool ok = false;
#ifdef _WIN32
  HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr,
                            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE) return false;
  DWORD written = 0;
  ok = WriteFile(file, text.data(), static_cast<DWORD>(text.size()), &written, nullptr) &&
       written == text.size() && FlushFileBuffers(file);
  if (!CloseHandle(file)) ok = false;
  if (ok) ok = MoveFileExW(temporary.c_str(), path.c_str(),
                           MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
  const int file = ::open(temporary.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
  if (file < 0) return false;
  const auto written = ::write(file, text.data(), text.size());
  ok = written >= 0 && static_cast<std::size_t>(written) == text.size() && ::fsync(file) == 0;
  if (::close(file) != 0) ok = false;
  if (ok) ok = ::rename(temporary.c_str(), path.c_str()) == 0;
#endif
  if (!ok) { std::error_code ignored; fs::remove(temporary, ignored); }
  return ok;
}
}  // namespace settings_detail

inline std::string user_settings_path() {
  namespace sd = settings_detail;
  auto override_path = sd::environment_path("VERDIGRIS_SETTINGS_PATH");
  if (!override_path.empty()) return sd::utf8(override_path);
#ifdef _WIN32
  auto root = sd::environment_path("LOCALAPPDATA");
  return root.empty() ? std::string{} : sd::utf8(root / "Verdigris" / "settings.ini");
#else
  auto root = sd::environment_path("XDG_CONFIG_HOME");
  if (root.empty()) {
    root = sd::environment_path("HOME");
    if (root.empty()) return {};
    root /= ".config";
  }
  return sd::utf8(root / "verdigris" / "settings.ini");
#endif
}
inline SettingsLoadResult load_settings(const std::string& path) {
  return settings_detail::read(settings_detail::from_utf8(path)).loaded;
}
inline SettingsLoadResult load_user_settings() { return load_settings(user_settings_path()); }
inline SettingsSaveResult save_settings(const std::string& path, const AudioPrefs& prefs) {
  namespace sd = settings_detail;
  if (prefs.sfx_permille < 0 || prefs.sfx_permille > 1000 ||
      prefs.music_permille < 0 || prefs.music_permille > 1000)
    return {false, "Audio volume must be between 0 and 100 percent."};
  const auto target = sd::from_utf8(path);
  const auto doc = sd::read(target);
  if (doc.loaded.status == SettingsStatus::IoError ||
      doc.loaded.status == SettingsStatus::UnsupportedVersion)
    return {false, doc.loaded.message};
  std::error_code error;
  if (!target.parent_path().empty()) std::filesystem::create_directories(target.parent_path(), error);
  if (error) return {false, "Settings folder could not be created; changes are active for this session only."};
  if (doc.loaded.status == SettingsStatus::Corrupt) {
    const auto backup = sd::unique_sibling(target, ".corrupt-");
    if (!std::filesystem::copy_file(target, backup, std::filesystem::copy_options::none, error))
      return {false, "Damaged settings could not be backed up; the original is unchanged."};
  }
  std::string text = "version=1\nmuted=" + std::to_string(prefs.muted ? 1 : 0) +
      "\nsfx=" + std::to_string(prefs.sfx_permille) + "\nmusic=" +
      std::to_string(prefs.music_permille) + "\n";
  if (doc.loaded.status == SettingsStatus::Loaded)
    for (const auto& line : doc.unrelated) text += line + "\n";
  if (text.size() > sd::kMaxBytes || !sd::atomic_write(target, text))
    return {false, "Settings could not be saved; changes are active for this session only."};
  return {true, doc.loaded.status == SettingsStatus::Corrupt ?
      "Settings saved; the damaged file was kept as a backup." : "Settings saved."};
}
inline SettingsSaveResult save_user_settings(const AudioPrefs& prefs) {
  return save_settings(user_settings_path(), prefs);
}
inline AudioPrefs apply_mute_only(AudioPrefs prefs, bool muted) {
  prefs.muted = muted;
  return prefs;
}
inline bool save_audio_prefs(const std::string& path, const AudioPrefs& prefs) {
  return save_settings(path, prefs).ok;
}
inline AudioPrefs load_audio_prefs(const std::string& path) {
  return load_settings(path).prefs;
}

}  // namespace verdigris::audio
