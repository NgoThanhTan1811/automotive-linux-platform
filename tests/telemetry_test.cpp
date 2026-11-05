#include "sensors/sensor_simulator.h"

#include "ipc/telemetry_protocol.h"
#include "telemetry/telemetry_generator.h"

#include <cmath>
#include <iostream>

namespace {

bool approximately_equal(double left, double right) {
    return std::abs(left - right) < 0.001;
}

}  // namespace

int main() {
    const automotive::VehicleTelemetry telemetry =
        automotive::generate_sample_telemetry();

    if (!approximately_equal(telemetry.vehicle_speed_kph, 42.5)) {
        std::cerr << "unexpected vehicle speed\n";
        return 1;
    }

    if (telemetry.engine_rpm != 1800 ||
        !approximately_equal(telemetry.engine_temperature_c, 87.0) ||
        telemetry.battery_level_percent != 94 ||
        telemetry.gear_position != "D") {
        std::cerr << "unexpected telemetry sample\n";
        return 1;
    }

    const automotive::VehicleTelemetry threaded_telemetry =
        automotive::run_sensor_threads(5);

    if (!approximately_equal(threaded_telemetry.vehicle_speed_kph, 44.0) ||
        threaded_telemetry.engine_rpm != 2000 ||
        !approximately_equal(threaded_telemetry.engine_temperature_c, 87.4)) {
        std::cerr << "unexpected synchronized threaded telemetry\n";
        return 1;
    }

    const std::vector<automotive::TelemetryEvent> events =
        automotive::run_event_queue_demo(100);
    if (events.size() != 300) {
        std::cerr << "unexpected number of consumed telemetry events\n";
        return 1;
    }

    if (!automotive::run_event_queue_demo(0).empty()) {
        std::cerr << "zero-sample queue should be empty\n";
        return 1;
    }

    const automotive::TelemetryEvent event{"engine_rpm", 1800.0, 7};
    automotive::TelemetryEvent decoded_event;
    if (!automotive::decode_telemetry_event(
            automotive::encode_telemetry_event(event), decoded_event) ||
        decoded_event.signal_name != event.signal_name ||
        decoded_event.value != event.value ||
        decoded_event.sequence != event.sequence ||
        automotive::decode_telemetry_event("invalid-event", decoded_event)) {
        std::cerr << "telemetry protocol test failed\n";
        return 1;
    }

    return 0;
}
