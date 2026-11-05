#include "ipc/telemetry_protocol.h"
#include "ipc/unix_domain_socket.h"
#include "sensors/sensor_simulator.h"

#include <exception>
#include <iostream>
#include <unistd.h>

namespace {
constexpr const char* kSocketPath = "@automotive_linux_platform";
}

int main() {
    try {
        const automotive::UnixDomainSocket server =
            automotive::UnixDomainSocket::listen_on(kSocketPath);
        std::cout << "sensor_service listening in process " << getpid()
                  << "\n";

        automotive::UnixDomainSocket client = server.accept_client();
        const std::vector<automotive::TelemetryEvent> events =
            automotive::run_event_queue_demo(3);

        for (const automotive::TelemetryEvent& event : events) {
            if (!client.send_line(automotive::encode_telemetry_event(event))) {
                std::cerr << "sensor_service: client disconnected\n";
                return 1;
            }
        }
        client.send_line("END");
        std::cout << "sensor_service sent " << events.size() << " events\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "sensor_service: " << error.what() << '\n';
        return 1;
    }
}
