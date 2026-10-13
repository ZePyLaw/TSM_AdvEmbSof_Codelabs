cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(
    BikeComputer
    VERSION 0.1
    DESCRIPTION "Bike computer application for Advanced Embedded Software Lesson"
    LANGUAGES C CXX
)

target_sources(
    app
    PRIVATE
    src/main.cpp
)