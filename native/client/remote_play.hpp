#pragma once

// TASK-0061: native client remote mode. Presentation talks only to
// IClientSession; this entry point must never construct a Simulation.

int run_remote_native_client(const char* host, unsigned short port,
                             const char* guest_id);
