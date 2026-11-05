#pragma once

#include "telemetry/telemetry_types.h"

#include <cstddef>
#include <vector>

namespace automotive {

VehicleTelemetry run_sensor_threads(std::size_t sample_count);
VehicleTelemetry run_sensor_threads_unsafe(std::size_t sample_count);
std::vector<TelemetryEvent> run_event_queue_demo(std::size_t sample_count);

}  // namespace automotive
