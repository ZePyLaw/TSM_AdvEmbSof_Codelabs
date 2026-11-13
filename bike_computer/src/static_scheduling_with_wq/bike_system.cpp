// Copyright 2025 Haute école d'ingénierie et d'architecture de Fribourg
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/****************************************************************************
 * @file bike_system.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 * @author Robin Schaerrer
 *
 * @brief Bike System implementation (static scheduling)
 *
 * @date 2026-10-15
 * @version 1.0.1
 ***************************************************************************/

#include "bike_system.hpp"

// std
#include <chrono>

// zephyr

// zpp_lib
#include "zpp_include/this_thread.hpp"
#include "zpp_include/time.hpp"
#include "zpp_include/utils.hpp"
#include "zpp_include/work_queue.hpp"
#include "zpp_include/zpp_assert.hpp"
#include "zpp_include/zpp_log.hpp"

// common
#include "common/ttce.hpp"

ZPP_LOG_MODULE_DECLARE(bike_computer, CONFIG_APP_LOG_LEVEL);

namespace bike_computer::static_scheduling_with_wq {

// Define the destructor declared in bike_system.hpp.
// The compiler handles the destruction of all member objects.
BikeSystem::~BikeSystem() = default;

// The complexity is increased by zephyr macros
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
zpp_lib::ZephyrResult BikeSystem::start() {
  ZPP_LOG_INF("Starting TTCE Cyclic Scheduling");

  zpp_lib::Utils::log_threads_summary();

  _ttce.add_initial_task([this]() {
    auto res = this->initialize();
    if (!res) {
      ZPP_LOG_ERR("Init failed: %d", static_cast<int>(res.error()));
      return;
    }
    // initialize the task manager phase
    this->_task_manager.initialize_phase();
  });

  // Declare the variable res before assignation
  zpp_lib::ZephyrResult res;

  // minor cycle 0
  res = _ttce.add_task(0, [this]() { this->speed_distance_task(); });
  if (!res) {
    return res;
  }
  res = _ttce.add_task(0, [this]() { this->gear_task(); });
  if (!res) {
    return res;
  }
  res = _ttce.add_task(0, [this]() { this->reset_task(); });
  if (!res) {
    return res;
  }

  // minor cycle 1
  res = _ttce.add_task(1, [this]() { this->speed_distance_task(); });
  if (!res) {
    return res;
  }
  res = _ttce.add_task(1, [this]() { this->display_task1(); });
  if (!res) {
    return res;
  }

  // minor cycle 2
  res = _ttce.add_task(2, [this]() { this->speed_distance_task(); });
  if (!res) {
    return res;
  }
  res = _ttce.add_task(2, [this]() { this->gear_task(); });
  if (!res) {
    return res;
  }
  res = _ttce.add_task(2, [this]() { this->reset_task(); });
  if (!res) {
    return res;
  }

  // minor cycle 3
  res = _ttce.add_task(3, [this]() { this->speed_distance_task(); });
  if (!res) {
    return res;
  }
  res = _ttce.add_task(3, [this]() { this->display_task2(); });
  if (!res) {
    return res;
  }
  res = _ttce.add_task(3, [this]() { this->temperature_task(); });
  if (!res) {
    return res;
  }

  // log the cpu load
#ifdef CONFIG_CPU_LOAD
  res = _ttce.add_task(3, []() { zpp_lib::Utils::log_cpu_load(); });
  if (!res) {
    return res;
  }
#endif

  // Start the scheduler (this method will block as long as _ttce is running)
  _ttce.start();

  return {};
}

// Stop the TTCE scheduler
void BikeSystem::stop() {
  // _ttce handle his own stop
  _ttce.stop();
}

zpp_lib::ZephyrResult BikeSystem::initialize() {
  // initialize the display
  auto res = _bike_display.initialize();
  if (!res) {
    ZPP_LOG_ERR("Cannot initialize display: %d", static_cast<int>(res.error()));
    return res;
  }

  // initialize the sensor device
  res = _sensor_device.initialize();
  if (!res) {
    ZPP_LOG_ERR("Sensor not present or initialization failed: %d", static_cast<int>(res.error()));
  }

  return {};
}

void BikeSystem::gear_task() {
  // gear task
  _task_manager.register_task_start(TaskManager::TaskType::GearTaskType);

  // no need to protect access to data members (single threaded)
  _current_gear      = _gear_device.get_current_gear();
  _current_gear_size = _gear_device.get_current_gear_size();

  _task_manager.simulate_computation_time(TaskManager::TaskType::GearTaskType, kAllowSleep);
}

void BikeSystem::speed_distance_task() {
  // speed and distance task
  _task_manager.register_task_start(TaskManager::TaskType::SpeedTaskType);

  auto pedal_rotation_time = _pedal_device.get_current_rotation_time();
  _speedometer.set_current_pedal_rotation_time(pedal_rotation_time);
  _speedometer.set_gear_size(_current_gear_size);
  // no need to protect access to data members (single threaded)
  _current_speed     = _speedometer.get_current_speed();
  _traveled_distance = _speedometer.get_traveled_distance();

  _task_manager.simulate_computation_time(TaskManager::TaskType::SpeedTaskType, kAllowSleep);
}

void BikeSystem::temperature_task() {
  _task_manager.register_task_start(TaskManager::TaskType::TemperatureTaskType);

  // no need to protect access to data members (single threaded)
  zpp_lib::ZephyrResult res = _sensor_device.read_temperature(_current_temperature);
  if (!res) {
    ZPP_LOG_ERR("Cannot read temperature: %d", static_cast<int>(res.error()));
  }

  // simulate task computation by waiting for the required task computation time
  _task_manager.simulate_computation_time(TaskManager::TaskType::TemperatureTaskType, kAllowSleep);
}

void BikeSystem::reset_task() {
  _task_manager.register_task_start(TaskManager::TaskType::ResetTaskType);

  if (_reset_device.check_reset()) {
#if CONFIG_APP_LOG_LEVEL >= CONFIG_LOG_LEVEL_INFO
    std::chrono::microseconds response_time = zpp_lib::Time::get_uptime() - _reset_device.get_press_time();
    ZPP_LOG_INF("Reset task: response time is %" PRIu64 " usecs", response_time.count());
#endif  // CONFIG_APP_LOG_LEVEL >= CONFIG_LOG_LEVEL_INFO
    _speedometer.reset();
  }

  _task_manager.simulate_computation_time(TaskManager::TaskType::ResetTaskType, kAllowSleep);
}

void BikeSystem::display_task1() {
  _task_manager.register_task_start(TaskManager::TaskType::DisplayTask1Type);

  _bike_display.display_gear(_current_gear);
  _bike_display.display_speed(_current_speed);
  _bike_display.display_distance(_traveled_distance);

  _task_manager.simulate_computation_time(TaskManager::TaskType::DisplayTask1Type, kAllowSleep);
}

void BikeSystem::display_task2() {
  _task_manager.register_task_start(TaskManager::TaskType::DisplayTask2Type);

  _bike_display.display_temperature(_current_temperature);

  _task_manager.simulate_computation_time(TaskManager::TaskType::DisplayTask2Type, kAllowSleep);
}

}  // namespace bike_computer::static_scheduling_with_wq
