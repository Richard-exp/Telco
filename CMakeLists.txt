cmake_minimum_required(VERSION 3.16)
project(gateway_telemetry_shipper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(gateway_daemon
    src/main.cpp
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(gateway_daemon PRIVATE -Wall -Wextra -Wpedantic)
endif()
