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

# C3 session seam (D-122): the only client code allowed to own a Simulation
# (local) or a socket (remote). Links verdigris_networking for the shared
# envelope codec — recorded interim coupling; a verdigris_protocol extraction
# is the successor cleanup if the codec needs to shed the server.
add_library(verdigris_client_session
  client/local_session.cpp
  client/remote_session.cpp
  client/presentation_state.cpp
)
target_include_directories(verdigris_client_session PUBLIC client)
target_link_libraries(verdigris_client_session PUBLIC verdigris_networking)
if (WIN32)
  target_link_libraries(verdigris_client_session PUBLIC ws2_32)
endif()

add_executable(verdigris_session_tests tests/session_tests.cpp)
target_link_libraries(verdigris_session_tests PRIVATE verdigris_client_session)

add_executable(verdigris_client client/main.cpp client/remote_play.cpp)
target_include_directories(verdigris_client PRIVATE client)
target_link_libraries(verdigris_client PRIVATE verdigris_core verdigris_client_session)
if (WIN32)
  target_compile_definitions(verdigris_client PRIVATE VERDIGRIS_NATIVE_WINDOWS=1)
  target_link_libraries(verdigris_client PRIVATE user32 gdi32)
endif()

enable_testing()
add_test(NAME verdigris_core_tests COMMAND verdigris_core_tests)
add_test(NAME verdigris_networking_tests COMMAND verdigris_networking_tests)
add_test(NAME verdigris_session_tests COMMAND verdigris_session_tests)
