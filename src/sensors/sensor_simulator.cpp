#include "sensors/sensor_simulator.h"

#include "concurrency/event_queue.h"

#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace automotive {
namespace {

struct SharedTelemetryState {
    double vehicle_speed_kph = 0.0;
    int engine_rpm = 0;
    double engine_temperature_c = 0.0;
    int battery_level_percent = 100;
    std::string gear_position = "P";
    int update_count = 0;
};

void speed_sensor_unsafe(SharedTelemetryState& state,
                         std::size_t sample_count) {
    for (std::size_t sample = 0; sample < sample_count; ++sample) {
        state.vehicle_speed_kph = 40.0 + static_cast<double>(sample);
        ++state.update_count;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void rpm_sensor_unsafe(SharedTelemetryState& state,
                       std::size_t sample_count) {
    for (std::size_t sample = 0; sample < sample_count; ++sample) {
        state.engine_rpm = 1800 + static_cast<int>(sample * 50);
        ++state.update_count;
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}

void temperature_sensor_unsafe(SharedTelemetryState& state,
                               std::size_t sample_count) {
    for (std::size_t sample = 0; sample < sample_count; ++sample) {
        state.engine_temperature_c = 87.0 + static_cast<double>(sample) * 0.1;
        ++state.update_count;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void speed_sensor(SharedTelemetryState& state, std::mutex& state_mutex,
                  std::size_t sample_count) {
    for (std::size_t sample = 0; sample < sample_count; ++sample) {
        {
            std::lock_guard<std::mutex> lock(state_mutex);
            state.vehicle_speed_kph = 40.0 + static_cast<double>(sample);
            ++state.update_count;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void rpm_sensor(SharedTelemetryState& state, std::mutex& state_mutex,
                std::size_t sample_count) {
    for (std::size_t sample = 0; sample < sample_count; ++sample) {
        {
            std::lock_guard<std::mutex> lock(state_mutex);
            state.engine_rpm = 1800 + static_cast<int>(sample * 50);
            ++state.update_count;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}

void temperature_sensor(SharedTelemetryState& state, std::mutex& state_mutex,
                        std::size_t sample_count) {
    for (std::size_t sample = 0; sample < sample_count; ++sample) {
        {
            std::lock_guard<std::mutex> lock(state_mutex);
            state.engine_temperature_c =
                87.0 + static_cast<double>(sample) * 0.1;
            ++state.update_count;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

VehicleTelemetry to_vehicle_telemetry(const SharedTelemetryState& state) {
    return VehicleTelemetry{
        state.vehicle_speed_kph,
        state.engine_rpm,
        state.engine_temperature_c,
        state.battery_level_percent,
        state.gear_position,
    };
}

void produce_speed_events(TelemetryEventQueue& queue,
                          std::size_t sample_count) {
    for (std::size_t sample = 0; sample < sample_count; ++sample) {
        if (!queue.push(TelemetryEvent{"vehicle_speed_kph",
                                      40.0 + static_cast<double>(sample),
                                      sample})) {
            return;
        }
    }
}

void produce_rpm_events(TelemetryEventQueue& queue,
                        std::size_t sample_count) {
    for (std::size_t sample = 0; sample < sample_count; ++sample) {
        if (!queue.push(TelemetryEvent{"engine_rpm",
                                      1800.0 + static_cast<double>(sample * 50),
                                      sample})) {
            return;
        }
    }
}

void produce_temperature_events(TelemetryEventQueue& queue,
                                std::size_t sample_count) {
    for (std::size_t sample = 0; sample < sample_count; ++sample) {
        if (!queue.push(TelemetryEvent{"engine_temperature_c",
                                      87.0 + static_cast<double>(sample) * 0.1,
                                      sample})) {
            return;
        }
    }
}

}  // namespace

VehicleTelemetry run_sensor_threads(std::size_t sample_count) {
    SharedTelemetryState state;
    std::mutex state_mutex;

    std::thread speed_thread(speed_sensor, std::ref(state), std::ref(state_mutex),
                             sample_count);
    std::thread rpm_thread(rpm_sensor, std::ref(state), std::ref(state_mutex),
                           sample_count);
    std::thread temperature_thread(temperature_sensor, std::ref(state),
                                   std::ref(state_mutex), sample_count);

    speed_thread.join();
    rpm_thread.join();
    temperature_thread.join();

    return to_vehicle_telemetry(state);
}

VehicleTelemetry run_sensor_threads_unsafe(std::size_t sample_count) {
    SharedTelemetryState state;

    std::thread speed_thread(speed_sensor_unsafe, std::ref(state), sample_count);
    std::thread rpm_thread(rpm_sensor_unsafe, std::ref(state), sample_count);
    std::thread temperature_thread(temperature_sensor_unsafe, std::ref(state),
                                   sample_count);

    speed_thread.join();
    rpm_thread.join();
    temperature_thread.join();

    return to_vehicle_telemetry(state);
}

std::vector<TelemetryEvent> run_event_queue_demo(std::size_t sample_count) {
    TelemetryEventQueue queue(4);
    std::vector<TelemetryEvent> consumed_events;

    std::thread consumer([&queue, &consumed_events] {
        TelemetryEvent event;
        while (queue.pop(event)) {
            consumed_events.push_back(std::move(event));
        }
    });

    std::thread speed_producer(produce_speed_events, std::ref(queue),
                               sample_count);
    std::thread rpm_producer(produce_rpm_events, std::ref(queue), sample_count);
    std::thread temperature_producer(produce_temperature_events,
                                     std::ref(queue), sample_count);

    speed_producer.join();
    rpm_producer.join();
    temperature_producer.join();
    queue.close();
    consumer.join();

    return consumed_events;
}

}  // namespace automotive
