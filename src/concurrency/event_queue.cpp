#include "concurrency/event_queue.h"
#include <utility>

namespace automotive {

TelemetryEventQueue::TelemetryEventQueue(std::size_t capacity)
    : capacity_(capacity) {}

bool TelemetryEventQueue::push(TelemetryEvent event) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_full_.wait(lock, [this] {
        return queue_.size() < capacity_ || closed_;
    });

    if (closed_) {
        return false;
    }

    queue_.push_back(std::move(event));
    not_empty_.notify_one();
    return true;
}

bool TelemetryEventQueue::pop(TelemetryEvent& event) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_.wait(lock, [this] {
        return !queue_.empty() || closed_;
    });

    if (queue_.empty()) {
        return false;
    }

    event = std::move(queue_.front());
    queue_.pop_front();
    not_full_.notify_one();
    return true;
}

void TelemetryEventQueue::close() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
    }
    not_empty_.notify_all();
    not_full_.notify_all();
}

}  // namespace automotive
