#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <string_view>

struct TelemetryPayload {
    static constexpr std::size_t kDeviceIdCapacity = 64;

    std::array<char, kDeviceIdCapacity> device_id{};
    std::int64_t event_timestamp_ms{};
    double gps_lat{};
    double gps_lon{};
    std::int64_t ping_ms{};
    std::int64_t ingest_timestamp_ms{};

    TelemetryPayload(
        std::string_view device_id_value,
        std::int64_t event_timestamp_ms_value,
        double gps_lat_value,
        double gps_lon_value,
        std::int64_t ping_ms_value)
        : event_timestamp_ms(event_timestamp_ms_value),
          gps_lat(gps_lat_value),
          gps_lon(gps_lon_value),
          ping_ms(ping_ms_value),
          ingest_timestamp_ms(now_ms()) {
        const auto copy_length =
            device_id_value.size() < kDeviceIdCapacity - 1
                ? device_id_value.size()
                : kDeviceIdCapacity - 1;
        for (std::size_t index = 0; index < copy_length; ++index) {
            device_id[index] = device_id_value[index];
        }
        device_id[copy_length] = '\0';
    }

private:
    static std::int64_t now_ms() {
        const auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   now.time_since_epoch())
            .count();
    }
};
