#include "telemetry/telemetry_generator.h"

namespace automotive {

VehicleTelemetry generate_sample_telemetry() {
    return VehicleTelemetry{
        42.5,
        1800,
        87.0,
        94,
        "D",
    };
}

}  // namespace automotive
