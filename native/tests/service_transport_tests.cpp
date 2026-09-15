#include "../client/service_transport.hpp"

#include <chrono>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "crypt32.lib")
#endif

using verdigris::client::ServiceTransport;
using namespace std::chrono_literals;
namespace {
int checks{};
void check(bool condition, const std::string& label) {
    if (!condition) throw std::runtime_error(label);
    ++checks;
}
#ifdef _WIN32
// Deliberately small disposable wire fixture, not production transport. Uses
// Windows crypto for the RFC handshake; it never listens outside loopback.
std::string accept_key(const std::string& key) {
    const std::string input = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    HCRYPTPROV provider{};
    HCRYPTHASH hash{};
    if (!CryptAcquireContext(&provider, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) throw std::runtime_error("crypto provider");
    if (!CryptCreateHash(provider, CALG_SHA1, 0, 0, &hash)) throw std::runtime_error("crypto hash");
    CryptHashData(hash, reinterpret_cast<const BYTE*>(input.data()), static_cast<DWORD>(input.size()), 0);
    BYTE digest[20]; DWORD length = sizeof(digest);
    CryptGetHashParam(hash, HP_HASHVAL, digest, &length, 0);
    CryptDestroyHash(hash); CryptReleaseContext(provider, 0);
    DWORD encoded_length{};
    CryptBinaryToStringA(digest, length, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &encoded_length);
    std::string encoded(encoded_length, '\0');
    CryptBinaryToStringA(digest, length, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, encoded.data(), &encoded_length);
    encoded.resize(encoded_length);
    return encoded;
}
bool write_all(SOCKET socket, const std::string& value) {
    std::size_t at{};
    while (at < value.size()) {
        const int count = ::send(socket, value.data() + at, static_cast<int>(value.size() - at), 0);
        if (count <= 0) return false;
        at += count;
    }
    return true;
}
std::string frame(const std::string& data, unsigned char opcode = 0x81) {
    std::string output(1, static_cast<char>(opcode));
    const auto size = data.size();
    if (size < 126) output += static_cast<char>(size);
    else if (size <= 65535) { output += char(126); output += char(size >> 8); output += char(size); }
    else { output += char(127); for (int shift = 56; shift >= 0; shift -= 8) output += char(static_cast<std::uint64_t>(size) >> shift); }
    return output + data;
}
enum class Mode { echo, fragments, exact_limit, large, binary, stall_handshake, redirect, close_frame, idle, slow_reader };
struct Fixture {
    SOCKET listener = INVALID_SOCKET;
    unsigned short port{};
    std::thread thread;
    std::promise<void> accepted;
    std::future<void> accepted_future = accepted.get_future();
    std::string request_line;
    explicit Fixture(Mode mode) {
        listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in address{};
        address.sin_family = AF_INET; address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) || listen(listener, 1)) throw std::runtime_error("fixture bind");
        int length = sizeof(address); getsockname(listener, reinterpret_cast<sockaddr*>(&address), &length); port = ntohs(address.sin_port);
        thread = std::thread([this, mode] {
            SOCKET peer = accept(listener, nullptr, nullptr);
            if (peer == INVALID_SOCKET) return;
            accepted.set_value();
            DWORD timeout = mode == Mode::idle ? 35000 : 12000;
            setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));
            setsockopt(peer, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&timeout), sizeof(timeout));
            if (mode == Mode::slow_reader) {
                int window = 1024;
                setsockopt(peer, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&window), sizeof(window));
            }
            std::string request;
            char buffer[8192];
            while (request.find("\r\n\r\n") == std::string::npos && request.size() < 16384) {
                const int count = recv(peer, buffer, sizeof(buffer), 0);
                if (count <= 0) { closesocket(peer); return; }
                request.append(buffer, count);
            }
            request_line = request.substr(0, request.find("\r\n"));
            if (mode == Mode::stall_handshake) { recv(peer, buffer, sizeof(buffer), 0); closesocket(peer); return; }
            if (mode == Mode::redirect) {
                write_all(peer, "HTTP/1.1 302 Found\r\nLocation: ws://example.com/\r\nContent-Length: 0\r\n\r\n");
                closesocket(peer); return;
            }
            const auto key_start = request.find("Sec-WebSocket-Key: ");
            if (key_start == std::string::npos) { closesocket(peer); return; }
            const auto start = key_start + 19;
            const auto key = request.substr(start, request.find("\r\n", start) - start);
            write_all(peer, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " + accept_key(key) + "\r\n\r\n");
            if (mode == Mode::slow_reader) { std::this_thread::sleep_for(8s); closesocket(peer); return; }
            if (mode == Mode::fragments) { write_all(peer, frame("first-", 0x01)); write_all(peer, frame("second", 0x80)); }
            else if (mode == Mode::exact_limit) write_all(peer, frame(std::string(1024 * 1024, 'x')));
            else if (mode == Mode::large) write_all(peer, frame(std::string(1024 * 1024 + 1, 'x')));
            else if (mode == Mode::binary) write_all(peer, frame("binary", 0x82));
            else if (mode == Mode::close_frame) write_all(peer, frame(std::string("\x03\xe8", 2), 0x88));
            else if (mode == Mode::echo) {
                // A test command is short; accumulate one masked client frame.
                std::string input;
                while (input.size() < 6 || input.size() < 6 + (static_cast<unsigned char>(input[1]) & 127)) {
                    const int count = recv(peer, buffer, sizeof(buffer), 0);
                    if (count <= 0) { closesocket(peer); return; }
                    input.append(buffer, count);
                }
                const auto length = static_cast<unsigned char>(input[1]) & 127;
                std::string payload;
                for (int i = 0; i < length; ++i) payload += char(input[6 + i] ^ input[2 + i % 4]);
                write_all(peer, frame(payload));
            }
            // Keep the socket open until the test closes its transport.
            while (recv(peer, buffer, sizeof(buffer), 0) > 0) {}
            closesocket(peer);
        });
    }
    ~Fixture() { closesocket(listener); if (thread.joinable()) thread.join(); }
    std::string endpoint(const std::string& host = "127.0.0.1") const { return "ws://" + host + ":" + std::to_string(port) + "/game/socket"; }
};
#endif
}

int main(int argc, char** argv) {
    try {
        std::string error, output;
        ServiceTransport transport;
        check(!transport.send("x", &error) && !error.empty(), "send requires a connection");
        check(!transport.receive(output, &error) && !error.empty(), "receive requires a connection");
        transport.close(); transport.close();
#ifdef _WIN32
        if (argc == 3 && std::string(argv[1]) == "--reject-tls") {
            check(!transport.connect(argv[2], &error) && error.find("TLS certificate") != std::string::npos, "untrusted TLS certificate must fail: " + error);
            std::cout << "TLS validation rejected untrusted server certificate\n";
            return 0;
        }
        WSADATA startup{}; check(WSAStartup(MAKEWORD(2, 2), &startup) == 0, "Winsock fixture startup");
        const std::vector<std::string> invalid = {"", "https://example.com", "ws://example.com", "ws://127.0.0.2", "ws://127.1", "ws://2130706433", "ws://127.0.0.1.evil.test", "wss://u:p@example.com", "wss://example.com/#token", "wss://example.com/?token=secret", "wss://example.com/\r\nX:test", "wss://", "wss://example.com:", "wss://example.com:0", "wss://example.com:65536", "wss://example.com:9999999999999999999", "wss://example.com:-1", "wss://example.com:1foo", "ws://local%68ost", "wss://example.com\\evil", std::string(2049, 'x')};
        for (const auto& endpoint : invalid) check(!transport.connect(endpoint, &error) && !error.empty(), "reject invalid or insecure endpoint");
        check(!transport.send(std::string(1024 * 1024 + 1, 'x'), &error) && error.find("1 MiB") != std::string::npos, "outbound limit");
        {
            Fixture server(Mode::echo);
            check(transport.connect(server.endpoint("localhost"), &error), "hostname/path connect: " + error);
            std::string receive_error;
            auto receiving = std::async(std::launch::async, [&] { return transport.receive(output, &receive_error); });
            check(transport.send("hello service", &error), "send: " + error);
            check(receiving.get() && output == "hello service", "concurrent receive/send echo: " + receive_error);
            transport.close();
            server.thread.join();
            check(server.request_line == "GET /game/socket HTTP/1.1", "preserve endpoint path");
        }
        for (Mode mode : {Mode::fragments, Mode::exact_limit, Mode::large, Mode::binary, Mode::close_frame}) {
            Fixture server(mode);
            check(transport.connect(server.endpoint(), &error), "fixture connect: " + error);
            const bool received = transport.receive(output, &error);
            if (mode == Mode::fragments) check(received && output == "first-second", "fragment assembly");
            else if (mode == Mode::exact_limit) check(received && output == std::string(1024 * 1024, 'x'), "receive exact 1 MiB boundary");
            else check(!received && output.empty() && !error.empty(), "reject oversized/binary/closed messages without partial output");
            transport.close();
        }
        {
            Fixture server(Mode::redirect);
            check(!transport.connect(server.endpoint(), &error) && error.find("302") != std::string::npos, "redirect refused: " + error);
            transport.close();
        }
        for (int attempt = 0; attempt < 20; ++attempt) {
            Fixture server(Mode::stall_handshake);
            auto connecting = std::async(std::launch::async, [&] { return transport.connect(server.endpoint(), &error); });
            check(server.accepted_future.wait_for(2s) == std::future_status::ready, "handshake fixture accepted");
            const auto begin = std::chrono::steady_clock::now();
            transport.close();
            check(connecting.wait_for(1s) == std::future_status::ready && !connecting.get(), "cancel pending handshake");
            check(std::chrono::steady_clock::now() - begin < 1s, "bounded handshake close");
        }
        for (int attempt = 0; attempt < 20; ++attempt) {
            Fixture server(Mode::idle);
            check(transport.connect(server.endpoint(), &error), "idle fixture connect");
            auto receiving = std::async(std::launch::async, [&] { return transport.receive(output, &error); });
            std::this_thread::sleep_for(10ms);
            transport.close();
            check(receiving.wait_for(1s) == std::future_status::ready && !receiving.get(), "cancel pending receive");
        }
        {
            Fixture server(Mode::stall_handshake);
            const auto begin = std::chrono::steady_clock::now();
            check(!transport.connect(server.endpoint(), &error), "silent handshake times out");
            check(std::chrono::steady_clock::now() - begin < 11s, "handshake total deadline");
            transport.close();
        }
        {
            Fixture server(Mode::idle);
            check(transport.connect(server.endpoint(), &error), "receive deadline fixture connect");
            const auto begin = std::chrono::steady_clock::now();
            check(!transport.receive(output, &error) && error.find("timed out") != std::string::npos, "silent receive times out");
            const auto elapsed = std::chrono::steady_clock::now() - begin;
            check(elapsed >= 29s && elapsed < 32s, "receive total 30-second deadline");
            transport.close();
        }
        {
            Fixture server(Mode::slow_reader);
            check(transport.connect(server.endpoint(), &error), "slow reader fixture connect");
            const auto begin = std::chrono::steady_clock::now();
            bool rejected = false;
            for (int attempt = 0; attempt < 32 && !rejected; ++attempt)
                rejected = !transport.send(std::string(1024 * 1024, 'x'), &error);
            check(rejected && error.find("timed out") != std::string::npos, "slow reader send times out: " + error);
            check(std::chrono::steady_clock::now() - begin < 7s, "bounded pending send");
            transport.close();
        }
        WSACleanup();
#else
        (void)argc; (void)argv;
        check(!transport.connect("wss://example.com/game", &error) && error.find("Windows") != std::string::npos, "unsupported platform fails explicitly");
#endif
        std::cout << checks << " service transport checks passed\n";
        return 0;
    } catch (const std::exception& exception) {
        std::cerr << "FAIL: " << exception.what() << '\n';
        return 1;
    }
}
