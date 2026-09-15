#pragma once

#include <memory>
#include <string>

namespace verdigris::client {

// Windows WinHTTP WebSocket transport. Call connect/send/receive off the UI
// thread. One send and one receive may run concurrently; close cancels either
// and an in-progress handshake. Join callers before destroying the object.
// connect: 10 s total; send: 5 s; receive: 30 s total (including fragments).
// Any timeout fails/closes that connection; false returns an actionable error.
// wss:// is required except literal loopback or localhost ws:// for local QA.
// Endpoints contain no credentials, query strings, fragments or redirects.
class ServiceTransport {
public:
    ServiceTransport();
    ~ServiceTransport();
    ServiceTransport(const ServiceTransport&) = delete;
    ServiceTransport& operator=(const ServiceTransport&) = delete;

    bool connect(const std::string& endpoint, std::string* error = nullptr);
    bool send(const std::string& text, std::string* error = nullptr);
    bool receive(std::string& text, std::string* error = nullptr);
    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace verdigris::client
