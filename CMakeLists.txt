cmake_minimum_required(VERSION 3.16)

project(telco LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For Termux/Android ARM64, configure with the Android NDK toolchain, for example:
# cmake -S . -B build-android \
#   -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
#   -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24
# The model uses only standard C++ and has no x86-specific dependencies.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)")
    message(STATUS "Configuring for ARM64 target: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

add_library(telemetry_payload INTERFACE)
target_include_directories(telemetry_payload INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(telemetry_payload_test tests/telemetry_payload_test.cpp)
    target_link_libraries(telemetry_payload_test
        PRIVATE telemetry_payload GTest::gtest_main
    )
    add_test(NAME telemetry_payload_test COMMAND telemetry_payload_test)
endif()
