#pragma once

#include "telemetry/telemetry_types.h"

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>

namespace automotive {

class TelemetryEventQueue {
public:
    explicit TelemetryEventQueue(std::size_t capacity);

    bool push(TelemetryEvent event);
    bool pop(TelemetryEvent& event);
    void close();

private:
    std::size_t capacity_;
    std::deque<TelemetryEvent> queue_;
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    bool closed_ = false;
};

}  // namespace automotive
