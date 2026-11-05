#pragma once

#include "telemetry/telemetry_types.h"

#include <string>

namespace automotive {

std::string encode_telemetry_event(const TelemetryEvent& event);
bool decode_telemetry_event(const std::string& line, TelemetryEvent& event);

}  // namespace automotive
