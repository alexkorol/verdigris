cmake_minimum_required(VERSION 3.20)
project(verdigris_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(verdigris_core
  src/core.cpp
  src/seasonal.cpp
)
target_include_directories(verdigris_core PUBLIC include)

add_library(verdigris_networking
  src/networking.cpp
)
target_include_directories(verdigris_networking PUBLIC include)
target_link_libraries(verdigris_networking PUBLIC verdigris_core)
if (WIN32)
  target_link_libraries(verdigris_networking PRIVATE ws2_32)
endif()

add_executable(verdigris_core_tests tests/core_tests.cpp)
target_link_libraries(verdigris_core_tests PRIVATE verdigris_core)

add_executable(verdigris_networking_tests tests/networking_tests.cpp)
target_link_libraries(verdigris_networking_tests PRIVATE verdigris_networking)

add_executable(verdigris_server src/server_main.cpp)
target_link_libraries(verdigris_server PRIVATE verdigris_networking)

add_executable(verdigris_client client/main.cpp)
target_link_libraries(verdigris_client PRIVATE verdigris_core)
if (WIN32)
  target_compile_definitions(verdigris_client PRIVATE VERDIGRIS_NATIVE_WINDOWS=1)
endif()

enable_testing()
add_test(NAME verdigris_core_tests COMMAND verdigris_core_tests)
add_test(NAME verdigris_networking_tests COMMAND verdigris_networking_tests)
