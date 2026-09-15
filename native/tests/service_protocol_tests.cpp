#include "verdigris/networking.hpp"
#include "verdigris/service_store.hpp"
#include "../client/service_transport.hpp"

#include <chrono>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

using verdigris::networking::Envelope;
using verdigris::networking::JsonValue;
using verdigris::networking::WebSocketServer;
using verdigris::service::Store;
using verdigris::client::ServiceTransport;
using namespace std::chrono_literals;
namespace {
int checks{};
void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
    ++checks;
}
std::string text(const JsonValue& value) { return value.string() ? *value.string() : ""; }
std::int64_t now_ms() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); }
#ifdef _WIN32
std::uint16_t free_port() {
    WSADATA startup{}; check(WSAStartup(MAKEWORD(2, 2), &startup) == 0, "Winsock fixture startup");
    SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in address{}; address.sin_family = AF_INET; address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    const bool bound = bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0;
    int size = sizeof(address); const bool queried = bound && getsockname(socket, reinterpret_cast<sockaddr*>(&address), &size) == 0;
    closesocket(socket); WSACleanup(); check(queried, "Find a disposable loopback port");
    return ntohs(address.sin_port);
}
struct Runtime {
    std::filesystem::path directory;
    std::shared_ptr<Store> store;
    std::unique_ptr<WebSocketServer> server;
    std::uint16_t port{};
    explicit Runtime(std::filesystem::path root) : directory(std::move(root)) { open(); }
    void open() {
        const auto u8 = directory.u8string(); store = std::make_shared<Store>(std::string(u8.begin(), u8.end()));
        std::string error; check(store->open(&error), "Open disposable production Store");
    }
    void start() {
        port = free_port(); server = std::make_unique<WebSocketServer>(port, std::filesystem::path{}, store);
        std::string error; check(server->start(&error), "Start real authenticated WebSocketServer");
    }
    void stop() { if (server) server->stop(); server.reset(); store.reset(); }
    ~Runtime() { stop(); }
};

// Read the real WinHTTP transport continuously, matching the production native
// receive path. A bounded queue and timeout prevent missing events hanging CI.
class Peer {
public:
    ServiceTransport transport;
    std::string account, token, epoch;
    std::uint64_t sequence{};
    explicit Peer(std::uint16_t port) {
        std::string error;
        check(transport.connect("ws://127.0.0.1:" + std::to_string(port) + "/game", &error), "Connect real WinHTTP service transport");
        reader = std::thread([this] {
            std::string message, error;
            while (transport.receive(message, &error)) {
                Envelope envelope;
                if (!verdigris::networking::parse_envelope(message, envelope)) break;
                std::lock_guard lock(mutex);
                if (queue.size() >= 256) { overflow = true; break; }
                queue.push_back(std::move(envelope)); changed.notify_all();
            }
            std::lock_guard lock(mutex); closed = true; changed.notify_all();
        });
    }
    ~Peer() { transport.close(); if (reader.joinable()) reader.join(); }
    void send(const Envelope& envelope) {
        std::string error;
        check(transport.send(verdigris::networking::emit_envelope(envelope), &error), "Send production protocol envelope");
    }
    JsonValue until(const std::string& event) {
        const auto deadline = std::chrono::steady_clock::now() + 5s;
        std::unique_lock lock(mutex);
        for (;;) {
            check(!overflow, "Bounded protocol test receive queue");
            while (!queue.empty()) {
                auto envelope = std::move(queue.front()); queue.pop_front();
                if (envelope.event == event) return std::move(envelope.data);
            }
            if (closed) throw std::runtime_error("Service connection closed while awaiting " + event);
            if (!changed.wait_until(lock, deadline, [&] { return closed || !queue.empty(); }))
                throw std::runtime_error("Production event timed out: " + event);
        }
    }
    JsonValue authenticate(const std::string& credential, bool enroll, int version = 1) {
        send({"service:authenticate", JsonValue::Object{{"credential", credential}, {"enroll", enroll}, {"protocolVersion", version}}});
        if (version != 1) return until("service:rejected");
        auto admission = until("service:authenticated");
        account = text(admission["accountId"]); token = text(admission["token"]); epoch = text(admission["commandEpoch"]); sequence = 0;
        check(!account.empty() && !token.empty() && !epoch.empty(), "Authenticated account and command epoch exist");
        return admission;
    }
    Envelope command(const std::string& event, JsonValue::Object data) {
        data["commandEpoch"] = epoch; data["commandSequence"] = static_cast<double>(++sequence);
        Envelope envelope{event, std::move(data)}; send(envelope); return envelope;
    }
    JsonValue snapshot() {
        // A ping barrier consumes old auth/transition snapshots before asking
        // for fresh owner state. No diagnostic event or fixture mutation used.
        send({"service:ping", JsonValue::Object{}}); until("service:pong");
        send({"world:snapshot", JsonValue::Object{{"includeMap", false}}});
        auto value = until("world:snapshot");
        check(value["protocolVersion"].number().value_or(0) == 1, "Production snapshot protocol version");
        return value["state"];
    }
private:
    std::thread reader;
    std::mutex mutex;
    std::condition_variable changed;
    std::deque<Envelope> queue;
    bool closed{}, overflow{};
};

const JsonValue::Array& houses(const JsonValue& snapshot) {
    static const JsonValue::Array empty;
    const auto& record = snapshot["chroniclesRecord"];
    if (record.is_object() && !record["exists"].boolean().value_or(true) && record["state"].is_null()) return empty;
    const auto* rows = snapshot["chroniclesRecord"]["state"]["houses"].array();
    check(rows != nullptr, "Owner snapshot has House roster"); return *rows;
}
using Inventory = std::map<std::string, std::string>;
Inventory inventory(const JsonValue& snapshot) {
    const auto* rows = snapshot["inventoryDetails"].array(); check(rows != nullptr, "Owner snapshot has private inventory");
    Inventory result;
    for (const auto& item : *rows) {
        const auto id = text(item["uuid"]); check(!id.empty(), "Private item has stable UUID");
        check(result.emplace(id, item.stringify()).second, "No duplicate physical item UUID in owner inventory");
    }
    return result;
}
std::pair<std::string, std::string> found_and_select(Peer& peer, const char* house_name, bool replay) {
    const auto found = peer.command("chronicles:house:found", {{"name", house_name}});
    auto founded = peer.until("chronicles:state");
    const auto* owned = founded["chronicle"]["houses"].array();
    check(owned && owned->size() == 1, "Ordinary House founding creates exactly one owned House");
    const auto house = text(owned->front()["id"]); check(!house.empty(), "House has server-owned identity");
    if (replay) {
        peer.send(found);
        check(text(peer.until("service:result")["status"]) == "already-processed", "Same epoch/sequence replay is acknowledged without repeating mutation");
        check(houses(peer.snapshot()).size() == 1, "Replay did not create another House");
    }
    peer.command("chronicles:scion:create", {{"houseId", house}, {"name", "Protocol Scion"}, {"appearance", "male"}});
    auto created = peer.until("chronicles:state");
    const auto scion = text(created["createdScionId"]); check(!scion.empty(), "Ordinary Scion creation returns owned identity");
    peer.command("chronicles:scion:set-out", {{"scionId", scion}});
    peer.until("player:login");
    const auto snapshot = peer.snapshot();
    check(text(snapshot["lifecycle"]) == "alive", "Ordinary set-out admits a living Scion");
    check(text(snapshot["chronicles"]["houseId"]) == house && text(snapshot["chronicles"]["scionId"]) == scion, "Snapshot identifies selected owned House and Scion");
    return {house, scion};
}
#endif
} // namespace

int main() {
#ifdef _WIN32
    const auto root = std::filesystem::temp_directory_path() / ("verdigris-protocol-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    try {
        Runtime runtime(root);
        std::string error;
        const auto code_a = runtime.store->issue_enrollment(now_ms(), 600000, &error);
        const auto code_b = runtime.store->issue_enrollment(now_ms(), 600000, &error);
        check(!code_a.empty() && !code_b.empty(), "Issue disposable single-use enrollment codes through operator Store API");
        runtime.start();
        auto a = std::make_unique<Peer>(runtime.port);
        const auto mismatch = a->authenticate(code_a, true, 999);
        check(mismatch["versionMismatch"].boolean().value_or(false), "Version mismatch is explicitly rejected before account admission");
        a->authenticate(code_a, true);
        check(houses(a->snapshot()).empty(), "Rejected protocol version did not consume enrollment or create gameplay state");
        auto spoof = std::make_unique<Peer>(runtime.port);
        spoof->send({"player:login", JsonValue::Object{{"guestId", a->account}, {"playtestGuestId", a->account}, {"quickGuest", true}}});
        spoof->until("service:rejected");
        spoof->send({"chronicles:house:found", JsonValue::Object{{"name", "Unauthorized"}}}); spoof->until("service:rejected");
        check(houses(a->snapshot()).empty(), "Unauthenticated guest spoof cannot replace account or found a House");
        spoof->send({"service:authenticate", JsonValue::Object{{"credential", code_a}, {"enroll", true}, {"protocolVersion", 1}}});
        spoof->until("service:rejected");
        const auto owned_a = found_and_select(*a, "Protocol House A", true);
        const auto original = a->snapshot(); const auto items_a = inventory(original);
        check(!items_a.empty(), "Ordinary first set-out has real owned inventory to preserve");
        // Raw protocol fields are optional for compatibility, but supplied actor
        // or scene references must fence stale in-connection gameplay intents.
        a->command("player:move", {{"direction", "up"}, {"sequence", 1},
            {"sceneId", "instance:retired-protocol-fixture"}, {"actingActorId", text(original["uuid"])}});
        auto stale_scene = a->until("service:result");
        check(text(stale_scene["status"]) == "rejected" && text(stale_scene["reason"]) == "This command belongs to a retired actor or scene.", "Stale scene rejects raw gameplay intent");
        a->command("player:move", {{"direction", "down"}, {"sequence", 2},
            {"sceneId", text(original["sceneId"])}, {"actingActorId", "actor:foreign-protocol-fixture"}});
        auto stale_actor = a->until("service:result");
        check(text(stale_actor["status"]) == "rejected" && text(stale_actor["reason"]) == "This command belongs to a retired actor or scene.", "Foreign actingActorId rejects raw gameplay intent");
        const auto after_stale = a->snapshot();
        check(after_stale["x"].number() == original["x"].number() && after_stale["y"].number() == original["y"].number() && inventory(after_stale) == items_a,
            "Stale scene/actor commands leave position and possessions unchanged");
        auto b = std::make_unique<Peer>(runtime.port); b->authenticate(code_b, true);
        b->command("chronicles:scion:create", {{"houseId", owned_a.first}, {"name", "Foreign Scion"}});
        b->until("game:send:message");
        check(houses(b->snapshot()).empty(), "Foreign account cannot create a Scion in another House");
        const auto owned_b = found_and_select(*b, "Protocol House B", false);
        const auto b_state = b->snapshot(); const auto items_b = inventory(b_state);
        check(a->account != b->account && owned_a.first != owned_b.first && owned_a.second != owned_b.second, "Account, House and Scion identities remain independent");
        check(!items_b.empty(), "Second account owns a real private inventory");
        for (const auto& item : items_b) check(!items_a.count(item.first), "Physical item UUIDs are not copied across account inventories");
        const auto old_epoch = a->epoch, account_a = a->account, token_a = a->token, token_b = b->token;
        auto reconnect = std::make_unique<Peer>(runtime.port); reconnect->authenticate(token_a, false);
        check(reconnect->account == account_a && reconnect->epoch != old_epoch, "Reconnect preserves account and rotates command epoch");
        reconnect->send({"chronicles:house:found", JsonValue::Object{{"name", "Stale epoch"}, {"commandEpoch", old_epoch}, {"commandSequence", 1000}}});
        check(text(reconnect->until("service:result")["status"]) == "rejected", "Old connection command epoch is rejected");
        check(houses(reconnect->snapshot()).size() == 1 && inventory(reconnect->snapshot()) == items_a, "Stale epoch cannot mutate House roster or possessions");
        reconnect->command("player:login", {{"guestId", b->account}}); reconnect->until("service:rejected");
        check(text(reconnect->snapshot()["uuid"]) == text(original["uuid"]), "Authenticated guest spoof cannot switch the owned runtime actor");
        spoof.reset(); a.reset(); b.reset(); reconnect.reset();
        runtime.stop(); runtime.open(); runtime.start();
        auto restored_a = std::make_unique<Peer>(runtime.port); restored_a->authenticate(token_a, false);
        auto restored_b = std::make_unique<Peer>(runtime.port); restored_b->authenticate(token_b, false);
        const auto after_a = restored_a->snapshot(), after_b = restored_b->snapshot();
        check(restored_a->account == account_a, "Store restart preserves authenticated account identity");
        check(houses(after_a).size() == 1 && text(houses(after_a).front()["id"]) == owned_a.first, "Store restart preserves owned House");
        check(text(after_a["chronicles"]["scionId"]) == owned_a.second && text(after_a["lifecycle"]) == "alive", "Store restart preserves selected living Scion");
        const auto* living = houses(after_a).front()["scions"].array();
        check(living && living->size() == 1 && text(living->front()["id"]) == owned_a.second && after_a["hp"]["current"].number().value_or(0) > 0,
            "Restored selected Scion remains in the living roster with positive life");
        check(inventory(after_a) == items_a, "Store restart preserves exact private item UUIDs and item data");
        check(inventory(after_b) == items_b, "Store restart independently preserves second account possessions");
        check(text(houses(after_b).front()["id"]) == owned_b.first, "Store restart does not copy the first House into second account");
        restored_a.reset(); restored_b.reset(); runtime.stop();
        std::cout << checks << " production service protocol checks passed; no development commands\n";
        std::cout << "Disposable Store retained at " << root.string() << "\n";
        return 0;
    } catch (const std::exception& failure) {
        std::cerr << "FAIL: " << failure.what() << "\nDisposable Store retained at " << root.string() << "\n";
        return 1;
    }
#else
    std::cout << "SKIP: service protocol tests require the verified Windows Store/WinHTTP build\n";
    return 0;
#endif
}
