#pragma once

#include <filesystem>
#include <fstream>
#include <cstdint>
#include <stdexcept>
#include <system_error>
#include <string>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace verdigris::persistence {

// The core remains pure; this tiny adapter is the only v1 file boundary.  A
// completed temporary file is atomically renamed into place.  Callers supply
// bytes produced by verdigris::snapshot().
inline void write_atomic(const std::filesystem::path& target,
                         const std::vector<std::uint8_t>& bytes) {
  const std::filesystem::path temporary = target.string() + ".tmp";
  {
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output) throw std::runtime_error("unable to open snapshot temporary file");
    if (!bytes.empty()) {
      output.write(reinterpret_cast<const char*>(bytes.data()),
                   static_cast<std::streamsize>(bytes.size()));
    }
    output.flush();
    if (!output) throw std::runtime_error("unable to write snapshot temporary file");
  }

#if defined(_WIN32)
  // std::filesystem::rename does not replace an existing file on Windows.
  // MoveFileEx provides the same replacement/atomicity contract as POSIX
  // rename while keeping the adapter independent of the simulation core.
  if (!MoveFileExW(temporary.c_str(), target.c_str(),
                   MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    std::error_code error(static_cast<int>(GetLastError()), std::system_category());
    std::filesystem::remove(temporary);
    throw std::filesystem::filesystem_error("unable to atomically replace snapshot", temporary,
                                             target, error);
  }
#else
  std::error_code error;
  std::filesystem::rename(temporary, target, error);
  if (error) {
    std::filesystem::remove(temporary);
    throw std::filesystem::filesystem_error("unable to atomically replace snapshot", temporary,
                                             target, error);
  }
#endif
}

inline std::vector<std::uint8_t> read(const std::filesystem::path& source) {
  std::ifstream input(source, std::ios::binary);
  if (!input) throw std::runtime_error("unable to open snapshot file");
  input.seekg(0, std::ios::end);
  const std::streamoff length = input.tellg();
  if (length < 0) throw std::runtime_error("unable to size snapshot file");
  input.seekg(0, std::ios::beg);
  std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
  if (!bytes.empty()) {
    input.read(reinterpret_cast<char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
  }
  if (!input) throw std::runtime_error("unable to read snapshot file");
  return bytes;
}

// Descriptive aliases keep the seam convenient for platform callers without
// adding another implementation or changing the core boundary.
inline void write_snapshot(const std::filesystem::path& target,
                           const std::vector<std::uint8_t>& bytes) {
  write_atomic(target, bytes);
}

inline std::vector<std::uint8_t> read_snapshot(const std::filesystem::path& source) {
  return read(source);
}

}  // namespace verdigris::persistence
