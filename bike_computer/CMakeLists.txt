cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(
    BikeComputer
    VERSION 0.1
    DESCRIPTION "Bike computer application for Advanced Embedded Software Lesson"
    LANGUAGES C CXX
)

# Allow source files to include shared headers using "common/...".
target_include_directories(app PRIVATE src)

target_sources(
    app
    PRIVATE
    src/main.cpp

    # Common components and display resources
    src/common/bike_display.cpp
    src/common/resources/fonts_DejaVuSans_16.cpp
    src/common/resources/fonts_DejaVuSans_18.cpp

    # Hardware devices for static scheduling
    src/static_scheduling/gear_device.cpp
    src/static_scheduling/pedal_device.cpp
    src/static_scheduling/reset_device.cpp
    src/common/task_manager.cpp
    src/static_scheduling/bike_system.cpp
)