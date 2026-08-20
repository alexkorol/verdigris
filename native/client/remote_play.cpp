#include "remote_play.hpp"

#ifndef VERDIGRIS_NATIVE_WINDOWS
int run_remote_native_client(const char*, unsigned short, const char*) { return 1; }
#endif
