#include "telemetry_payload.h"

#include <chrono>
#include <gtest/gtest.h>

TEST(TelemetryPayloadTest, PreservesEventTimeAndGeneratesIngestTime) {
    const auto before = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
    constexpr std::int64_t event_time = 1'234'567'890;

    const TelemetryPayload payload{"device-001", event_time, 47.6062, -122.3321, 42};

    const auto after = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
    EXPECT_EQ(payload.event_timestamp_ms, event_time);
    EXPECT_GE(payload.ingest_timestamp_ms, before);
    EXPECT_LE(payload.ingest_timestamp_ms, after);
    EXPECT_STREQ(payload.device_id.data(), "device-001");
}
