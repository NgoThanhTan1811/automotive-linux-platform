# V0 DONE — Linux/C++ Core

V0 hoàn tất khi project có:

- CMake build cho C++17.
- Telemetry data model và ownership rõ ràng.
- Ba sensor thread chạy trong một process.
- `join()` lifecycle rõ ràng.
- Race-condition experiment và mutex-protected implementation.
- Bounded producer-consumer queue.
- Condition variables cho empty/full queue.
- Graceful queue shutdown.
- Automated CTest coverage cho telemetry, threads và queue.
- CMake sanitizer selection.
- Tài liệu GDB, Valgrind, ThreadSanitizer và `strace`.

## Giới hạn đã biết

V0 chưa có process isolation, IPC, Unix Domain Socket, shared memory, CAN,
SocketCAN, ARM64, QEMU, kernel, Device Tree hoặc Buildroot. Những nội dung đó
thuộc V1–V3.

## Kết quả học tập

Người học cần giải thích được:

```text
thread
→ shared state
→ race condition
→ mutex
→ condition variable
→ producer-consumer queue
```
