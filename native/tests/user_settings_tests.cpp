#include "../client/persist-audio-accessibility-controls.hpp"
#include <iostream>

namespace fs = std::filesystem;
using namespace verdigris::audio;
static int failures = 0;
static void check(bool value, const char* label) {
  std::cout << (value ? "PASS " : "FAIL ") << label << '\n';
  if (!value) ++failures;
}
static void fixture(const fs::path& path, const std::string& text) {
  std::ofstream file(path, std::ios::binary);
  file << text;
  if (!file) throw std::runtime_error("fixture write failed");
}
static std::string contents(const fs::path& path) {
  std::ifstream file(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}
int main(int argc, char** argv) {
  if (argc == 2 && std::string(argv[1]) == "--write") {
    check(save_user_settings({true, 300, 700}).ok, "fresh process saves user settings");
    std::cout << "settings=" << user_settings_path() << '\n';
    return failures;
  }
  if (argc == 2 && std::string(argv[1]) == "--reload") {
    const auto loaded = load_user_settings();
    check(loaded.status == SettingsStatus::Loaded && loaded.prefs.muted &&
          loaded.prefs.sfx_permille == 300 && loaded.prefs.music_permille == 700,
          "fresh process reloads mute, SFX 30, Music 70 from user path");
    std::cout << "settings=" << user_settings_path() << '\n';
    return failures;
  }
  // Never write the actual owner's file. Every fixture is under this unique
  // temporary root and kept on disk for inspection after a test failure.
  const auto root = settings_detail::unique_sibling(fs::temp_directory_path() / "verdigris-settings-tests", "-");
  fs::create_directories(root);
  std::cout << "fixtures=" << settings_detail::utf8(root) << '\n';
  const auto path = root / "settings.ini";
  const auto name = settings_detail::utf8(path);
  const auto missing = load_settings(name);
  check(missing.status == SettingsStatus::Missing && !missing.prefs.muted &&
        missing.prefs.sfx_permille == 1000 && missing.message.empty(), "missing file uses quiet defaults");
  check(save_settings(name, {false, 400, 700}).ok, "first save succeeds");
  check(contents(path).starts_with("version=1\n"), "saved file declares schema version 1");
  fixture(path, "version=1\nmuted=0\nsfx=400\nmusic=700\nmap.zoom=1.25\n# keep comment\nfuture.color=umber\n");
  check(save_settings(name, apply_mute_only(load_settings(name).prefs, true)).ok,
        "mute-only save succeeds");
  const auto muted = load_settings(name);
  check(muted.prefs.muted && muted.prefs.sfx_permille == 400 && muted.prefs.music_permille == 700,
        "mute retains both category values");
  check(contents(path).find("map.zoom=1.25\n# keep comment\nfuture.color=umber\n") != std::string::npos,
        "audio save preserves unrelated values and comments");
  const std::vector<std::string> corrupt = {
    "", "version=1\nmuted=0\nsfx=300garbage\nmusic=700\n",
    "version=1\nmuted=2\nsfx=300\nmusic=700\n",
    "version=1\nmuted=0\nsfx=-1\nmusic=700\n",
    "version=1\nmuted=0\nsfx=1001\nmusic=700\n",
    "version=1\nmuted=0\nsfx=300\nsfx=400\nmusic=700\n",
    "version=1\nmuted=0\nsfx=300\n", std::string(65537, 'x'),
    std::string("version=1\nmuted=0\nsfx=300\nmusic=700\n") + '\0' + "junk",
    "version=wat\nmuted=0\nsfx=300\nmusic=700\n"};
  for (const auto& text : corrupt) {
    fixture(path, text);
    const auto loaded = load_settings(name);
    check(loaded.status == SettingsStatus::Corrupt && loaded.prefs.sfx_permille == 1000 &&
          !loaded.message.empty(), "corrupt input returns safe defaults and visible warning");
  }
  fixture(path, "broken\nfuture.color=umber\nfuture.color=red\n");
  const auto damaged = contents(path);
  check(save_settings(name, {false, 200, 600}).ok, "corrupt file recovers on explicit save");
  bool backed_up = false;
  for (const auto& file : fs::directory_iterator(root))
    if (file.path().filename().string().starts_with("settings.ini.corrupt-") && contents(file.path()) == damaged)
      backed_up = true;
  check(backed_up && load_settings(name).status == SettingsStatus::Loaded,
        "recovery retains exact damaged bytes in sibling backup and writes valid settings");
  fixture(path, "version=2\nmuted=1\nsfx=400\nmusic=700\nfuture=keep\n");
  const auto future = contents(path);
  check(load_settings(name).status == SettingsStatus::UnsupportedVersion &&
        !save_settings(name, {}).ok && contents(path) == future,
        "future schema is never overwritten");
  fixture(path, "muted=1\nsfx=400\nmusic=700\n");
  check(load_settings(name).status == SettingsStatus::Loaded && save_settings(name, {true, 400, 700}).ok &&
        contents(path).starts_with("version=1\n"), "legacy audio document upgrades on save");
  const auto original = contents(path);
  check(!save_settings(name, {false, -1, 700}).ok && contents(path) == original,
        "invalid caller values cannot damage persisted settings");
  const auto folder = root / "folder";
  fs::create_directory(folder);
  check(load_settings(settings_detail::utf8(folder)).status == SettingsStatus::IoError &&
        !save_settings(settings_detail::utf8(folder), {}).ok, "unreadable target is reported without overwrite");
  check(!save_settings(settings_detail::utf8(path / "child.ini"), {}).ok && contents(path) == original,
        "blocked parent reports failure without changing the original");
#ifdef _WIN32
  HANDLE locked = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
  check(locked != INVALID_HANDLE_VALUE, "replacement failure fixture locks existing file");
  const auto failed_save = save_settings(name, {false, 900, 100});
  if (locked != INVALID_HANDLE_VALUE) CloseHandle(locked);
  check(!failed_save.ok && !failed_save.message.empty() && contents(path) == original,
        "atomic replacement failure reports error and preserves original bytes");
  bool temporary_left = false;
  for (const auto& file : fs::directory_iterator(root))
    temporary_left |= file.path().filename().string().find(".tmp-") != std::string::npos;
  check(!temporary_left, "failed save cleans its temporary file");
#endif
  const auto unicode_path = root / settings_detail::from_utf8("pr\xc3\xa9" "f\xc3\xa9rences") / "settings.ini";
  check(save_settings(settings_detail::utf8(unicode_path), {true, 100, 200}).ok &&
        load_settings(settings_detail::utf8(unicode_path)).prefs.music_permille == 200,
        "Unicode profile path saves and reloads");
  RecordingSink sink;
  AudioMixer mixer(sink);
  apply_audio_prefs(mixer, {false, 300, 700});
  check(mixer.bus_volume(Bus::Sfx) == 300 && mixer.bus_volume(Bus::Music) == 700,
        "controls reach independent mixer buses");
  mixer.set_bus_muted(Bus::Music, true);
  apply_audio_prefs(mixer, {true, 300, 700});
  check(mixer.bus_volume(Bus::Sfx) == 0 && mixer.bus_volume(Bus::Music) == 0 && mixer.bus_muted(Bus::Music),
        "master mute silences buses and preserves scene music gate");
  apply_audio_prefs(mixer, {false, 300, 700});
  check(mixer.bus_volume(Bus::Sfx) == 300 && mixer.bus_volume(Bus::Music) == 700 && mixer.bus_muted(Bus::Music),
        "unmute restores category volumes without starting unloaded music");
  CueSpec cue;
  cue.cue_id = "settings-volume-proof";
  cue.bus = Bus::Sfx;
  cue.params.gain_permille = 1000;
  mixer.submit(cue);
  const auto voiced = mixer.drain_scheduled();
  check(voiced.size() == 1 && voiced.front().effective_gain_permille == 300,
        "SFX preference scales actual scheduled audio gain");
  apply_audio_prefs(mixer, {false, 0, 700});
  mixer.submit(cue);
  check(mixer.drain_scheduled().empty(), "zero-volume effects schedule no audio");
  return failures == 0 ? 0 : 1;
}
