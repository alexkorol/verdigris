#include "verdigris/networking.hpp"
#include "verdigris/service_store.hpp"
#include "../client/build_identity.hpp"
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <sddl.h>
#include <bcrypt.h>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "bcrypt.lib")
#endif
namespace {
std::atomic<bool> stopping{};
void signal_stop(int) { stopping.store(true); }
std::int64_t now_ms() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
std::uint16_t parse_port(const std::string& text) {
    unsigned value{};
    if (text.empty()) throw std::runtime_error("Port is required.");
    for (char c : text) {
        if (c < '0' || c > '9' || value > 6553) throw std::runtime_error("Port must be between 1 and 65535.");
        value = value * 10 + static_cast<unsigned>(c - '0');
    }
    if (!value || value > 65535) throw std::runtime_error("Port must be between 1 and 65535.");
    return static_cast<std::uint16_t>(value);
}
std::filesystem::path path(const std::string& value) { return std::filesystem::path(std::u8string(value.begin(), value.end())); }
#ifdef _WIN32
std::string utf8(const std::wstring& value) {
    if (value.empty()) return {};
    const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    if (!size) throw std::runtime_error("Invalid Unicode argument.");
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), result.data(), size, nullptr, nullptr);
    return result;
}
struct Handle { HANDLE value{}; ~Handle() { if (value && value != INVALID_HANDLE_VALUE) CloseHandle(value); } };
HANDLE shutdown_completed{};
// Enrollment material and control handles never inherit a broad parent ACL.
struct PrivateSecurity {
    PSECURITY_DESCRIPTOR descriptor{};
    SECURITY_ATTRIBUTES attributes{sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE};
    PrivateSecurity() {
        Handle token;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token.value)) throw std::runtime_error("Cannot inspect operator identity.");
        DWORD size{}; GetTokenInformation(token.value, TokenUser, nullptr, 0, &size);
        std::vector<unsigned char> buffer(size);
        if (!GetTokenInformation(token.value, TokenUser, buffer.data(), size, &size)) throw std::runtime_error("Cannot inspect operator permissions.");
        LPWSTR sid{};
        if (!ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER*>(buffer.data())->User.Sid, &sid)) throw std::runtime_error("Cannot identify operator SID.");
        const auto sddl = std::wstring(L"D:P(A;;GA;;;SY)(A;;GA;;;") + sid + L")"; LocalFree(sid);
        if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl.c_str(), SDDL_REVISION_1, &descriptor, nullptr)) throw std::runtime_error("Cannot create private operator permissions.");
        attributes.lpSecurityDescriptor = descriptor;
    }
    ~PrivateSecurity() { if (descriptor) LocalFree(descriptor); }
};
std::wstring event_name(const std::filesystem::path& directory, const wchar_t* kind) {
    auto normalized = std::filesystem::weakly_canonical(std::filesystem::absolute(directory)).wstring();
    auto folded = normalized;
    if (!LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE, normalized.data(), static_cast<int>(normalized.size()),
        folded.data(), static_cast<int>(folded.size()), nullptr, nullptr, 0)) throw std::runtime_error("Cannot normalize service directory identity.");
    normalized = std::move(folded);
    BCRYPT_ALG_HANDLE algorithm{}; BCRYPT_HASH_HANDLE hash{}; unsigned char digest[32]{};
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0) throw std::runtime_error("Cannot initialize service control identity.");
    const auto created = BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0);
    const auto hashed = created >= 0 ? BCryptHashData(hash, reinterpret_cast<PUCHAR>(normalized.data()), static_cast<ULONG>(normalized.size() * sizeof(wchar_t)), 0) : created;
    const auto finished = hashed >= 0 ? BCryptFinishHash(hash, digest, sizeof(digest), 0) : hashed;
    if (hash) BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(algorithm, 0);
    if (finished < 0) throw std::runtime_error("Cannot create service control identity.");
    std::wstring result = std::wstring(L"Local\\Verdigris-") + kind + L"-";
    constexpr wchar_t hex[] = L"0123456789abcdef";
    for (auto byte : digest) { result += hex[byte >> 4]; result += hex[byte & 15]; }
    return result;
}
BOOL WINAPI console_stop(DWORD event) {
    if (event == CTRL_C_EVENT || event == CTRL_BREAK_EVENT || event == CTRL_CLOSE_EVENT || event == CTRL_SHUTDOWN_EVENT) {
        stopping.store(true);
        if ((event == CTRL_CLOSE_EVENT || event == CTRL_SHUTDOWN_EVENT) && shutdown_completed) WaitForSingleObject(shutdown_completed, 4000);
        return TRUE;
    }
    return FALSE;
}
void write_code(verdigris::service::Store& store, const std::filesystem::path& output, const std::string& account) {
    PrivateSecurity security; Handle file;
    file.value = CreateFileW(output.c_str(), GENERIC_WRITE, 0, &security.attributes, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file.value == INVALID_HANDLE_VALUE) throw std::runtime_error("Credential output must be a new writable private file.");
    std::string error;
    auto code = account.empty() ? store.issue_enrollment(now_ms(), 24LL * 60 * 60 * 1000, &error) : store.issue_recovery(account, now_ms(), 24LL * 60 * 60 * 1000, &error);
    if (code.empty()) throw std::runtime_error(error);
    code += "\n"; DWORD written{};
    if (!WriteFile(file.value, code.data(), static_cast<DWORD>(code.size()), &written, nullptr) || written != code.size() || !FlushFileBuffers(file.value)) throw std::runtime_error("Cannot durably write credential output. Issue a replacement to a new file.");
    SecureZeroMemory(code.data(), code.size());
}
#endif
std::string environment(const char* name) {
#ifdef _WIN32
    const std::string narrow(name); const std::wstring wide(narrow.begin(), narrow.end());
    const DWORD size = GetEnvironmentVariableW(wide.c_str(), nullptr, 0);
    if (!size) return {};
    std::wstring value(size, L'\0');
    const DWORD length = GetEnvironmentVariableW(wide.c_str(), value.data(), size);
    if (!length || length >= size) throw std::runtime_error("Environment changed while reading server configuration.");
    value.resize(length); return utf8(value);
#else
    const char* value = std::getenv(name); return value ? value : "";
#endif
}
int run(const std::vector<std::string>& args) {
    if (args.size() == 2 && args[1] == "--build-info") { std::cout << VERDIGRIS_BUILD_ID << " " << (VERDIGRIS_BUILD_DIRTY ? "dirty" : "clean") << "\n"; return 0; }
    std::uint16_t port = 6500;
    std::size_t party_capacity=4;
    bool service{}, enroll{}, health{}, stop{};
    std::string data, policy, output, backup, restore, recover;
    const auto configured_port = environment("VERDIGRIS_PORT");
    if (!configured_port.empty()) port = parse_port(configured_port);
    for (std::size_t i = 1; i < args.size(); ++i) {
        auto next = [&]() { if (++i >= args.size()) throw std::runtime_error("Missing server option value."); return args[i]; };
        const auto option = args[i];
        if (option == "--service") service = true;
        else if (option == "--enroll") enroll = true;
        else if (option == "--health") health = true;
        else if (option == "--stop") stop = true;
        else if (option == "--recover") recover = next();
        else if (option == "--data") data = next();
        else if (option == "--port") port = parse_port(next());
        else if (option == "--party-capacity") party_capacity=parse_port(next());
        else if (option == "--qa-policy") policy = next();
        else if (option == "--output") output = next();
        else if (option == "--backup") backup = next();
        else if (option == "--restore") restore = next();
        else if (i == 1 && option.find_first_not_of("0123456789") == std::string::npos) port = parse_port(option);
        else throw std::runtime_error("Unknown server option. Use --service --data <directory> --port <port> --qa-policy coop-v1, or --enroll/--recover/--backup/--restore/--health/--stop with --data.");
    }
    const int operations = int(service) + int(enroll) + int(health) + int(stop) + int(!backup.empty()) + int(!restore.empty()) + int(!recover.empty());
    if (operations > 1) throw std::runtime_error("Select exactly one server operation.");
    if (operations && data.empty()) throw std::runtime_error("Service operations require an explicit --data directory.");
    if (service && policy != "coop-v1") throw std::runtime_error("Service requires explicit --qa-policy coop-v1 acknowledgement; see SERVICE-OPERATIONS.md.");
    if (!service && !policy.empty()) throw std::runtime_error("QA policy applies only to --service.");
    if ((enroll || !recover.empty()) != !output.empty()) throw std::runtime_error("Enrollment/recovery requires --output <new private file>.");
    if (!operations && !data.empty()) throw std::runtime_error("Local review uses VERDIGRIS_SAVE_DIR; --data belongs to explicit service operations.");
#ifdef _WIN32
    if (health || stop) {
        Handle event;
        event.value = OpenEventW(health ? SYNCHRONIZE : EVENT_MODIFY_STATE, FALSE, event_name(path(data), health ? L"ready" : L"stop").c_str());
        if (!event.value) throw std::runtime_error("Service is not ready/running under this operator and Windows session.");
        if (stop) { if (!SetEvent(event.value)) throw std::runtime_error("Cannot request service shutdown."); std::cout << "Service shutdown requested.\n"; }
        else { if (WaitForSingleObject(event.value, 0) != WAIT_OBJECT_0) throw std::runtime_error("Service is not ready."); std::cout << "Service ready: storage open and listener accepting.\n"; }
        return 0;
    }
    std::string error;
    if (!restore.empty()) { if (!verdigris::service::Store::restore_backup(restore, data, &error)) throw std::runtime_error(error); std::cout << "Service backup restored to the fresh data directory.\n"; return 0; }
    std::shared_ptr<verdigris::service::Store> store;
    if (operations) { store = std::make_shared<verdigris::service::Store>(data); if (!store->open(&error)) throw std::runtime_error(error); }
    if (enroll || !recover.empty()) { write_code(*store, path(output), recover); std::cout << "Credential issued to the private output file; expires in 24 hours.\n"; return 0; }
    if (!backup.empty()) { if (!store->backup_to(backup, &error)) throw std::runtime_error(error); std::cout << "Consistent service backup completed.\n"; return 0; }
#else
    if (operations) throw std::runtime_error("Authenticated service operations require the verified Windows 10+ service build.");
    std::string error; std::shared_ptr<verdigris::service::Store> store;
#endif
    std::filesystem::path saves;
    if (!service) { const auto configured = environment("VERDIGRIS_SAVE_DIR"); if (!configured.empty()) saves = path(configured); else saves = std::filesystem::absolute(path(args[0])).parent_path() / "saves"; }
    verdigris::networking::WebSocketServer server(port, saves, store);
    if(!server.set_party_capacity(party_capacity))throw std::runtime_error("QA party capacity must be between 2 and 8.");
    if (!server.start(&error)) throw std::runtime_error(error);
    std::signal(SIGINT, signal_stop); std::signal(SIGTERM, signal_stop);
#ifdef _WIN32
    Handle completion;
    completion.value = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    shutdown_completed = completion.value;
    SetConsoleCtrlHandler(console_stop, TRUE);
    Handle stop_event, ready_event;
    if (service) {
        PrivateSecurity security;
        stop_event.value = CreateEventW(&security.attributes, TRUE, FALSE, event_name(path(data), L"stop").c_str());
        ready_event.value = CreateEventW(&security.attributes, TRUE, TRUE, event_name(path(data), L"ready").c_str());
        if (!stop_event.value || !ready_event.value) { server.stop(); throw std::runtime_error("Cannot establish private service control/readiness events."); }
    }
#endif
    std::cout << "verdigris_server listening on ws://127.0.0.1:" << server.port() << "\n";
    if (service) std::cout << "verdigris_service ready policy=coop-v1 source=" << VERDIGRIS_BUILD_ID << "\n";
    std::cout.flush(); bool failed{};
    if (service) {
        while (!stopping.load()) {
            if (!server.healthy()) { failed = true; break; }
#ifdef _WIN32
            if (WaitForSingleObject(stop_event.value, 100) == WAIT_OBJECT_0) break;
#else
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
        }
    } else {
        std::string line;
        while (std::getline(std::cin, line)) if (line == "quit" || line == "stop") break;
        if (std::cin.eof()) while (!stopping.load()) std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
#ifdef _WIN32
    if (ready_event.value) ResetEvent(ready_event.value);
#endif
    server.stop();
#ifdef _WIN32
    if (completion.value) SetEvent(completion.value);
    SetConsoleCtrlHandler(console_stop, FALSE);
    shutdown_completed = nullptr;
#endif
    if (failed) throw std::runtime_error("Service stopped after a storage or listener failure; inspect storage before restarting.");
    std::cout << "verdigris_server stopped cleanly\n"; return 0;
}
}
#ifdef _WIN32
int wmain(int argc, wchar_t** argv) {
    try { std::vector<std::string> args; for (int i = 0; i < argc; ++i) args.push_back(utf8(argv[i])); return run(args); }
    catch (const std::exception& error) { std::cerr << "verdigris_server: " << error.what() << "\n"; return 1; }
}
#else
int main(int argc, char** argv) {
    try { return run(std::vector<std::string>(argv, argv + argc)); }
    catch (const std::exception& error) { std::cerr << "verdigris_server: " << error.what() << "\n"; return 1; }
}
#endif
