# Automotive Embedded Linux Middleware Platform

This repository is being built incrementally to learn Embedded Linux and
Linux middleware concepts. The current milestone is **V0 complete**.

## V0 scope

The project currently contains one C++ executable with three sensor producer
threads and one consumer thread connected by a bounded blocking event queue.
The queue uses a mutex and condition variables. An explicitly unsafe function
is retained as a learning experiment for ThreadSanitizer. The project still
does not contain processes, IPC, SocketCAN, kernel code, or Buildroot
integration.

## Build and run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/vehicle_telemetry_simulator
```

## Debugging exercises

Run the executable under GDB:

```sh
gdb --args ./build/vehicle_telemetry_simulator
```

Useful first commands are `break main`, `run`, `next`, `print telemetry`, and
`quit`.

If Valgrind is installed:

```sh
valgrind --leak-check=full ./build/vehicle_telemetry_simulator
```

## Learning notes

- `VehicleTelemetry` is returned by value and owned by the caller.
- The local `telemetry` object in `main` has automatic lifetime and is cleaned
  up when `main` returns.
- Each sensor has its own thread stack, while the sensor functions receive a
  reference to shared process memory.
- `join()` establishes the lifecycle boundary before the main thread reads the
  final sensor state.
- The synchronized implementation uses `std::mutex` and
  `std::lock_guard<std::mutex>` around shared updates.
- `run_sensor_threads_unsafe()` intentionally contains a data race and is not
  part of the normal executable path.
- `run_event_queue_demo()` demonstrates producer-consumer flow. Producers block
  when the bounded queue is full, and the consumer blocks when it is empty.
- `close()` wakes blocked operations so the consumer can drain remaining events
  and exit cleanly.
- The project uses CMake to describe targets and dependencies; the compiler
  still performs compilation and the linker still creates the executable.
- The program is a file on disk until the shell launches it. The launched
  instance is a Linux process with its own address space and resources.

V0 is complete. The next version will introduce process isolation and IPC.
