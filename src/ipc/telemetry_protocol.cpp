#include "ipc/telemetry_protocol.h"

#include <sstream>

namespace automotive {

std::string encode_telemetry_event(const TelemetryEvent& event) {
    std::ostringstream encoded;
    encoded << event.signal_name << '|' << event.value << '|' << event.sequence;
    return encoded.str();
}

bool decode_telemetry_event(const std::string& line, TelemetryEvent& event) {
    std::istringstream input(line);
    std::string value_text;
    std::string sequence_text;
    if (!std::getline(input, event.signal_name, '|') ||
        !std::getline(input, value_text, '|') ||
        !std::getline(input, sequence_text)) {
        return false;
    }

    try {
        event.value = std::stod(value_text);
        event.sequence = static_cast<std::size_t>(std::stoull(sequence_text));
    } catch (...) {
        return false;
    }
    return !event.signal_name.empty();
}

}  // namespace automotive
