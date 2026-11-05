#pragma once

#include <cstddef>
#include <string>

namespace automotive {

struct VehicleTelemetry {
    double vehicle_speed_kph;
    int engine_rpm;
    double engine_temperature_c;
    int battery_level_percent;
    std::string gear_position;
};

struct TelemetryEvent {
    std::string signal_name;
    double value;
    std::size_t sequence;
};

}  // namespace automotive
