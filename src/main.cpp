#include "sensors/sensor_simulator.h"

#include <iostream>

int main() {
    const std::vector<automotive::TelemetryEvent> events =
        automotive::run_event_queue_demo(5);

    std::cout << "Consumed telemetry events: " << events.size() << '\n';
    for (const automotive::TelemetryEvent& event : events) {
        std::cout << "  " << event.signal_name << " = " << event.value
                  << " (sample " << event.sequence << ")\n";
    }

    return 0;
}
