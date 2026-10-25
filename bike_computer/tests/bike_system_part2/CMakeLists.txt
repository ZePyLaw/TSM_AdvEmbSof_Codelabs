# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_bike_system_part2)

FILE(GLOB app_sources src/*.cpp ../../src/static_scheduling_with_event/*.cpp ../../src/common/*.cpp ../../src/common/resources/*.cpp)
target_sources(app PRIVATE ${app_sources})
target_include_directories(app PRIVATE ../../src/ )