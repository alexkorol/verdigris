# Platform seam

The first Windows client uses a tiny Win32 adapter in client/main.cpp. This
directory is the reserved seam for a macOS window/input adapter and later SDL3
or winit-style focused platform code. Platform code may translate input and
lifecycle events, but it must not own simulation rules.
