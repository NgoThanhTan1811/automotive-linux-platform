#include "ipc/telemetry_protocol.h"
#include "ipc/unix_domain_socket.h"

#include <chrono>
#include <exception>
#include <iostream>
#include <system_error>
#include <thread>
#include <unistd.h>

namespace {
constexpr const char* kSocketPath = "@automotive_linux_platform";
}

int main() {
    try {
        automotive::UnixDomainSocket connection;
        for (int attempt = 0; attempt < 50 && !connection.valid(); ++attempt) {
            try {
                connection = automotive::UnixDomainSocket::connect_to(kSocketPath);
            } catch (const std::system_error&) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        if (!connection.valid()) {
            std::cerr << "vehicle_middleware: could not connect to sensor_service\n";
            return 1;
        }

        std::cout << "vehicle_middleware connected in process " << getpid()
                  << '\n';
        std::size_t received_count = 0;
        std::string line;
        while (connection.receive_line(line)) {
            if (line == "END") {
                break;
            }

            automotive::TelemetryEvent event;
            if (!automotive::decode_telemetry_event(line, event)) {
                std::cerr << "vehicle_middleware: malformed event\n";
                return 1;
            }
            ++received_count;
            std::cout << "  received " << event.signal_name << " = "
                      << event.value << '\n';
        }

        std::cout << "vehicle_middleware received " << received_count
                  << " events\n";
        return received_count == 9 ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "vehicle_middleware: " << error.what() << '\n';
        return 1;
    }
}
