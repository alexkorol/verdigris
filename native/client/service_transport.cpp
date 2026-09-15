#include "service_transport.hpp"

#include <array>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#endif

namespace verdigris::client {
namespace {
constexpr std::size_t message_limit = 1024 * 1024;
bool fail(std::string* error, const std::string& reason) {
    if (error) *error = reason;
    return false;
}
#ifdef _WIN32
using Clock = std::chrono::steady_clock;

std::string system_error(const char* operation, DWORD code) {
    // Never include endpoint paths, credentials, or message content in errors.
    if (code == ERROR_WINHTTP_SECURE_FAILURE)
        return "The service TLS certificate or secure connection could not be validated.";
    if (code == ERROR_WINHTTP_TIMEOUT)
        return "Service connection timed out. Reconnect to continue.";
    return std::string(operation) + " failed (Windows error " + std::to_string(code) + ").";
}

struct Endpoint {
    std::wstring host;
    std::wstring path;
    INTERNET_PORT port{};
    bool secure{};
};

bool parse_endpoint(const std::string& input, Endpoint& result, std::string* error) {
    if (input.empty() || input.size() > 2048)
        return fail(error, "Service endpoint must contain 1 to 2048 characters.");
    for (unsigned char c : input)
        if (c <= 32 || c >= 127 || c == '\\' || c == '#' || c == '?' || c == '@')
            return fail(error, "Use an ASCII service URL without credentials, query, fragment, spaces or backslashes.");
    std::string url;
    if (input.rfind("wss://", 0) == 0) { result.secure = true; url = "https://" + input.substr(6); }
    else if (input.rfind("ws://", 0) == 0) { result.secure = false; url = "http://" + input.substr(5); }
    else return fail(error, "Service endpoint must start with wss:// (or ws:// for loopback QA).");
    // Reject ambiguous authority spellings before asking the OS URL parser.
    const auto authority_begin = url.find("://") + 3;
    const auto authority_end = url.find('/', authority_begin);
    const auto authority = url.substr(authority_begin, authority_end - authority_begin);
    if (authority.empty() || authority.find('%') != std::string::npos || authority.back() == ':')
        return fail(error, "Service endpoint has an invalid hostname or port.");
    const auto port_colon = authority.rfind(':');
    if (port_colon != std::string::npos && (authority.front() != '[' || port_colon > authority.find(']'))) {
        unsigned port = 0;
        for (std::size_t i = port_colon + 1; i < authority.size(); ++i) {
            if (authority[i] < '0' || authority[i] > '9')
                return fail(error, "Service endpoint has an invalid port.");
            port = port * 10 + static_cast<unsigned>(authority[i] - '0');
            if (port > 65535) return fail(error, "Service endpoint port must be between 1 and 65535.");
        }
        if (!port) return fail(error, "Service endpoint port must be between 1 and 65535.");
    }
    std::wstring wide(url.begin(), url.end());
    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof(parts);
    parts.dwHostNameLength = parts.dwUrlPathLength = parts.dwUserNameLength = parts.dwPasswordLength = static_cast<DWORD>(-1);
    if (!WinHttpCrackUrl(wide.c_str(), static_cast<DWORD>(wide.size()), 0, &parts) ||
        !parts.dwHostNameLength || parts.dwUserNameLength || parts.dwPasswordLength || !parts.nPort)
        return fail(error, "Service endpoint has an invalid hostname or port.");
    result.host.assign(parts.lpszHostName, parts.dwHostNameLength);
    result.path = parts.dwUrlPathLength ? std::wstring(parts.lpszUrlPath, parts.dwUrlPathLength) : L"/";
    result.port = parts.nPort;
    if (!result.secure) {
        for (auto& c : result.host) if (c >= L'A' && c <= L'Z') c += L'a' - L'A';
        if (result.host == L"localhost") result.host = L"127.0.0.1";
        if (result.host != L"127.0.0.1" && result.host != L"[::1]" && result.host != L"::1")
            return fail(error, "Unencrypted ws:// is restricted to loopback QA. Use wss:// for a service.");
    }
    return true;
}

// Callback-owned buffers outlive cancelled operations. HANDLE_CLOSING is the
// final callback, so releasing its heap context cannot race another callback.
struct Events {
    std::mutex mutex;
    std::condition_variable changed;
    bool cancelled{}, request_sent{}, headers{}, sent{}, received{};
    DWORD error{}, bytes{};
    WINHTTP_WEB_SOCKET_BUFFER_TYPE type{};
    std::array<char, 16384> input{};
    std::string output;
};
struct CallbackContext { std::shared_ptr<Events> events; };

void CALLBACK callback(HINTERNET, DWORD_PTR value, DWORD status, void* info, DWORD size) {
    auto* context = reinterpret_cast<CallbackContext*>(value);
    if (!context) return;
    if (status == WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING) { delete context; return; }
    auto& e = *context->events;
    std::lock_guard lock(e.mutex);
    if (status == WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE) e.request_sent = true;
    else if (status == WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE) e.headers = true;
    else if (status == WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE) e.sent = true;
    else if (status == WINHTTP_CALLBACK_STATUS_READ_COMPLETE && size >= sizeof(WINHTTP_WEB_SOCKET_STATUS)) {
        const auto* result = static_cast<WINHTTP_WEB_SOCKET_STATUS*>(info);
        e.bytes = result->dwBytesTransferred;
        e.type = result->eBufferType;
        e.received = true;
    } else if (status == WINHTTP_CALLBACK_STATUS_REQUEST_ERROR && size >= sizeof(WINHTTP_ASYNC_RESULT))
        e.error = static_cast<WINHTTP_ASYNC_RESULT*>(info)->dwError;
    e.changed.notify_all();
}

struct Connection {
    // Only short asynchronous WinHTTP initiation calls hold api_mutex. close
    // never closes a handle during a call, and does cancel pending operations.
    std::mutex api_mutex, send_mutex, receive_mutex;
    std::shared_ptr<Events> events = std::make_shared<Events>();
    HINTERNET session{}, connection{}, request{}, socket{};
    bool cancelled{};
    ~Connection() { close(); }
    void close() {
        std::lock_guard lock(api_mutex);
        cancelled = true;
        { std::lock_guard event_lock(events->mutex); events->cancelled = true; }
        events->changed.notify_all();
        for (auto* handle : {&socket, &request, &connection, &session})
            if (*handle) WinHttpCloseHandle(std::exchange(*handle, nullptr));
    }
    bool wait(bool Events::* completed, Clock::time_point deadline, std::string* error) {
        std::unique_lock lock(events->mutex);
        if (!events->changed.wait_until(lock, deadline, [&] { return events->cancelled || events->error || events.get()->*completed; }))
            return fail(error, "Service connection timed out. Reconnect to continue.");
        if (events->cancelled) return fail(error, "Service connection was closed.");
        if (events->error) return fail(error, system_error("Service operation", events->error));
        return true;
    }
};
#endif
} // namespace

struct ServiceTransport::Impl {
#ifdef _WIN32
    std::mutex mutex, connect_mutex;
    std::shared_ptr<Connection> current;
    std::shared_ptr<Connection> get() { std::lock_guard lock(mutex); return current; }
#endif
};

ServiceTransport::ServiceTransport() : impl_(std::make_unique<Impl>()) {}
ServiceTransport::~ServiceTransport() { close(); }

void ServiceTransport::close() {
#ifdef _WIN32
    std::shared_ptr<Connection> old;
    { std::lock_guard lock(impl_->mutex); old = std::exchange(impl_->current, nullptr); }
    if (old) old->close();
#endif
}

bool ServiceTransport::connect(const std::string& endpoint, std::string* error) {
    if (error) error->clear();
#ifdef _WIN32
    std::unique_lock serial(impl_->connect_mutex, std::try_to_lock);
    if (!serial.owns_lock()) return fail(error, "A service connection attempt is already in progress.");
    auto c = std::make_shared<Connection>();
    std::shared_ptr<Connection> old;
    { std::lock_guard lock(impl_->mutex); old = std::exchange(impl_->current, c); }
    if (old) old->close();
    const auto deadline = Clock::now() + std::chrono::seconds(10);
    auto abandon = [&] { c->close(); return false; };
    Endpoint target;
    if (!parse_endpoint(endpoint, target, error)) return abandon();
    DWORD initiation_error{};
    {
        std::lock_guard lock(c->api_mutex);
        auto initialize = [&]() -> bool {
            if (c->cancelled) return fail(error, "Service connection was closed.");
            c->session = WinHttpOpen(L"Verdigris/1", WINHTTP_ACCESS_TYPE_NO_PROXY,
                WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
            if (!c->session) return false;
            if (!WinHttpSetTimeouts(c->session, 5000, 5000, 5000, 5000)) return false;
            DWORD protocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
            if (!WinHttpSetOption(c->session, WINHTTP_OPTION_SECURE_PROTOCOLS, &protocols, sizeof(protocols))) return false;
            c->connection = WinHttpConnect(c->session, target.host.c_str(), target.port, 0);
            if (!c->connection) return false;
            c->request = WinHttpOpenRequest(c->connection, L"GET", target.path.c_str(), nullptr,
                WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, target.secure ? WINHTTP_FLAG_SECURE : 0);
            if (!c->request) return false;
            DWORD redirects = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
            DWORD disabled = WINHTTP_DISABLE_COOKIES | WINHTTP_DISABLE_AUTHENTICATION;
            if (!WinHttpSetOption(c->request, WINHTTP_OPTION_REDIRECT_POLICY, &redirects, sizeof(redirects)) ||
                !WinHttpSetOption(c->request, WINHTTP_OPTION_DISABLE_FEATURE, &disabled, sizeof(disabled)) ||
                !WinHttpSetOption(c->request, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr, 0)) return false;
            auto context = std::make_unique<CallbackContext>(CallbackContext{c->events});
            DWORD_PTR value = reinterpret_cast<DWORD_PTR>(context.get());
            if (!WinHttpSetOption(c->request, WINHTTP_OPTION_CONTEXT_VALUE, &value, sizeof(value))) return false;
            if (WinHttpSetStatusCallback(c->request, callback,
                WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_FLAG_HANDLES, 0) == WINHTTP_INVALID_STATUS_CALLBACK) return false;
            context.release(); // Final HANDLE_CLOSING owns deletion from here.
            return WinHttpSendRequest(c->request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                WINHTTP_NO_REQUEST_DATA, 0, 0, value) != FALSE;
        };
        if (!initialize()) initiation_error = GetLastError() ? GetLastError() : ERROR_OPERATION_ABORTED;
    }
    if (initiation_error) { if (!error || error->empty()) fail(error, system_error("Service connection", initiation_error)); return abandon(); }
    if (!c->wait(&Events::request_sent, deadline, error)) return abandon();
    {
        std::lock_guard lock(c->api_mutex);
        if (c->cancelled) initiation_error = ERROR_OPERATION_ABORTED;
        else if (!WinHttpReceiveResponse(c->request, nullptr)) initiation_error = GetLastError();
    }
    if (initiation_error) { fail(error, system_error("Service handshake", initiation_error)); return abandon(); }
    if (!c->wait(&Events::headers, deadline, error)) return abandon();
    bool upgraded = false;
    {
        std::lock_guard lock(c->api_mutex);
        if (!c->cancelled) {
            DWORD status{}, size = sizeof(status);
            if (!WinHttpQueryHeaders(c->request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX))
                fail(error, system_error("Service handshake status", GetLastError()));
            else if (status != 101) fail(error, "Service refused the WebSocket upgrade (HTTP " + std::to_string(status) + ").");
            else {
                auto context = std::make_unique<CallbackContext>(CallbackContext{c->events});
                c->socket = WinHttpWebSocketCompleteUpgrade(c->request, reinterpret_cast<DWORD_PTR>(context.get()));
                if (c->socket) { context.release(); upgraded = true; }
                else fail(error, system_error("Service WebSocket upgrade", GetLastError()));
            }
        } else fail(error, "Service connection was closed.");
    }
    return upgraded ? true : abandon();
#else
    (void)endpoint;
    return fail(error, "Service transport requires Windows 8 or later with WinHTTP WebSocket support.");
#endif
}

bool ServiceTransport::send(const std::string& text, std::string* error) {
    if (error) error->clear();
    if (text.size() > message_limit) return fail(error, "Service message exceeds the 1 MiB limit.");
#ifdef _WIN32
    auto c = impl_->get();
    if (!c) return fail(error, "Service is not connected.");
    std::unique_lock serial(c->send_mutex, std::try_to_lock);
    if (!serial.owns_lock()) return fail(error, "A service send is already in progress.");
    DWORD result{};
    {
        std::lock_guard lock(c->api_mutex);
        if (c->cancelled || !c->socket) return fail(error, "Service is not connected.");
        { std::lock_guard event_lock(c->events->mutex); c->events->sent = false; c->events->output = text; }
        result = WinHttpWebSocketSend(c->socket, WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
            c->events->output.data(), static_cast<DWORD>(text.size()));
    }
    if (result != ERROR_SUCCESS) { c->close(); return fail(error, system_error("Service send", result)); }
    if (!c->wait(&Events::sent, Clock::now() + std::chrono::seconds(5), error)) { c->close(); return false; }
    return true;
#else
    return fail(error, "Service transport is unsupported on this platform.");
#endif
}

bool ServiceTransport::receive(std::string& text, std::string* error) {
    text.clear();
    if (error) error->clear();
#ifdef _WIN32
    auto c = impl_->get();
    if (!c) return fail(error, "Service is not connected.");
    std::unique_lock serial(c->receive_mutex, std::try_to_lock);
    if (!serial.owns_lock()) return fail(error, "A service receive is already in progress.");
    const auto deadline = Clock::now() + std::chrono::seconds(30);
    std::string message;
    for (;;) {
        DWORD result{};
        {
            std::lock_guard lock(c->api_mutex);
            if (c->cancelled || !c->socket) return fail(error, "Service is not connected.");
            { std::lock_guard event_lock(c->events->mutex); c->events->received = false; }
            result = WinHttpWebSocketReceive(c->socket, c->events->input.data(),
                static_cast<DWORD>(c->events->input.size()), nullptr, nullptr);
        }
        if (result != ERROR_SUCCESS) { c->close(); return fail(error, system_error("Service receive", result)); }
        if (!c->wait(&Events::received, deadline, error)) { c->close(); return false; }
        DWORD bytes;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE type;
        { std::lock_guard lock(c->events->mutex); bytes = c->events->bytes; type = c->events->type; }
        if (type == WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE) { c->close(); return fail(error, "The service closed the connection."); }
        if (type != WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE && type != WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE) {
            c->close(); return fail(error, "The service sent an unsupported binary message.");
        }
        if (bytes > c->events->input.size() || bytes > message_limit - message.size()) {
            c->close(); return fail(error, "Service message exceeds the 1 MiB limit.");
        }
        message.append(c->events->input.data(), bytes);
        if (type == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE) { text = std::move(message); return true; }
        if (Clock::now() >= deadline) { c->close(); return fail(error, "Service message timed out."); }
    }
#else
    return fail(error, "Service transport is unsupported on this platform.");
#endif
}
} // namespace verdigris::client
