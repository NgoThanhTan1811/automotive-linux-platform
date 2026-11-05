cmake_minimum_required(VERSION 3.16)

project(automotive_linux_platform VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_WARNINGS "Enable compiler warnings" ON)
set(SANITIZER "none" CACHE STRING "Sanitizer: none, address, undefined, or thread")
set_property(CACHE SANITIZER PROPERTY STRINGS none address undefined thread)

find_package(Threads REQUIRED)

add_library(telemetry
    src/concurrency/event_queue.cpp
    src/ipc/telemetry_protocol.cpp
    src/ipc/unix_domain_socket.cpp
    src/sensors/sensor_simulator.cpp
    src/telemetry/telemetry_generator.cpp
)

target_include_directories(telemetry
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(telemetry PUBLIC Threads::Threads)

if(ENABLE_WARNINGS)
    target_compile_options(telemetry PRIVATE -Wall -Wextra -Wpedantic)
endif()

function(enable_sanitizer target)
    if(SANITIZER STREQUAL "none")
        return()
    elseif(SANITIZER STREQUAL "address")
        set(SANITIZER_FLAGS -fsanitize=address -fno-omit-frame-pointer -g)
    elseif(SANITIZER STREQUAL "undefined")
        set(SANITIZER_FLAGS -fsanitize=undefined -fno-omit-frame-pointer -g)
    elseif(SANITIZER STREQUAL "thread")
        set(SANITIZER_FLAGS -fsanitize=thread -fno-omit-frame-pointer -g)
    else()
        message(FATAL_ERROR "Unsupported SANITIZER='${SANITIZER}'")
    endif()

    target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS})
    target_link_options(${target} PRIVATE ${SANITIZER_FLAGS})
endfunction()

add_executable(vehicle_telemetry_simulator
    src/main.cpp
)

target_link_libraries(vehicle_telemetry_simulator PRIVATE telemetry)
enable_sanitizer(vehicle_telemetry_simulator)

if(ENABLE_WARNINGS)
    target_compile_options(vehicle_telemetry_simulator PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(telemetry_test
    tests/telemetry_test.cpp
)

target_link_libraries(telemetry_test PRIVATE telemetry)
enable_sanitizer(telemetry_test)

enable_sanitizer(telemetry)

if(ENABLE_WARNINGS)
    target_compile_options(telemetry_test PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME telemetry_test COMMAND telemetry_test)

add_executable(sensor_service
    apps/sensor_service/main.cpp
)
target_link_libraries(sensor_service PRIVATE telemetry)
enable_sanitizer(sensor_service)

add_executable(vehicle_middleware
    apps/vehicle_middleware/main.cpp
)
target_link_libraries(vehicle_middleware PRIVATE telemetry)
enable_sanitizer(vehicle_middleware)

add_executable(v1_process_launcher
    apps/process_launcher/main.cpp
)
enable_sanitizer(v1_process_launcher)

add_test(
    NAME v1_sensor_middleware_ipc
    COMMAND v1_process_launcher
            $<TARGET_FILE:sensor_service>
            $<TARGET_FILE:vehicle_middleware>
)
