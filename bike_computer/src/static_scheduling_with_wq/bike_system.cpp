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

BikeSystem::BikeSystem()
    : _isr_work_queue("ISRWorkQueue", zpp_lib::prio_to_preemptable_thread_priority(CONFIG_WORK_QUEUE_THREAD_PRIORITY)),
      _reset_work(zpp_lib::Work<BikeSystem>(this, &BikeSystem::update_reset)),
      _gear_work(zpp_lib::Work<BikeSystem, uint8_t, uint8_t>(
          this, &BikeSystem::update_gear, GearDevice::kInitialGear, GearDevice::kInitialGearSize)),
      _pedal_work(zpp_lib::Work<BikeSystem, std::chrono::milliseconds>(this, &BikeSystem::update_pedal, PedalDevice::kInitialRotationTime)),
      _reset_device([this]() { this->on_reset(); }),
      _gear_device([this](uint8_t current_gear, uint8_t current_gear_size) { this->on_gear_changed(current_gear, current_gear_size); }),
      _pedal_device([this](std::chrono::milliseconds pedal_rotation_time) { this->on_pedal_rotation_time_changed(pedal_rotation_time); }) {}

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

  // minor cycle 1
  res = _ttce.add_task(1, [this]() { this->display_task1(); });
  if (!res) {
    return res;
  }

  // minor cycle 2

  // minor cycle 3
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

  // Stop the work queue dedicated to ISR events
  auto res = _isr_work_queue.stop();
  if (!res) {
    ZPP_LOG_ERR("Cannot stop ISR work queue: %d", static_cast<int>(res.error()));
  }
}

zpp_lib::ZephyrResult BikeSystem::initialize() {
  // Iinitialize the display
  auto res = _bike_display.initialize();
  if (!res) {
    ZPP_LOG_ERR("Cannot initialize display: %d", static_cast<int>(res.error()));
    return res;
  }

  // Initialize GearDevice and PedalDevice state
  _bike_display.display_gear(GearDevice::kInitialGear);
  _speedometer.set_gear_size(GearDevice::kInitialGearSize);
  _speedometer.set_current_pedal_rotation_time(PedalDevice::kInitialRotationTime);

  // Initialize the sensor device
  res = _sensor_device.initialize();
  if (!res) {
    ZPP_LOG_ERR("Sensor not present or initialization failed: %d", static_cast<int>(res.error()));
  }

  return {};
}

void BikeSystem::on_reset() {
  // Timestamp captured by the GPIO callback (used for response-time logging).
  _press_time = zpp_lib::Time::get_uptime();

  // Defer the reset work to the work queue
  auto res = _isr_work_queue.call(_reset_work);
  if (!res) {
    ZPP_LOG_ERR("Cannot call work in on_reset: %d", static_cast<int>(res.error()));
  }
}

void BikeSystem::on_gear_changed(uint8_t current_gear, uint8_t current_gear_size) {
  // Update the gear work parameters
  _gear_work.set_params(current_gear, current_gear_size);

  // Defer the gear work to the work queue
  auto res = _isr_work_queue.call(_gear_work);
  if (!res) {
    ZPP_LOG_ERR("Cannot call work in on_gear_changed: %d", static_cast<int>(res.error()));
  }
}

void BikeSystem::on_pedal_rotation_time_changed(std::chrono::milliseconds pedal_rotation_time) {
  // Update the pedal work parameter
  _pedal_work.set_params(pedal_rotation_time);

  // Defer the pedal work to the work queue
  auto res = _isr_work_queue.call(_pedal_work);
  if (!res) {
    ZPP_LOG_ERR("Cannot call work in on_pedal_rotation_time_changed: %d", static_cast<int>(res.error()));
  }
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void BikeSystem::update_gear(uint8_t current_gear, uint8_t current_gear_size) {
  // Update the components that needs current_gear and current_gear_size
  _bike_display.display_gear(current_gear);
  _speedometer.set_gear_size(current_gear_size);
}

void BikeSystem::update_pedal(std::chrono::milliseconds pedal_rotation_time) {
  // Update the component that need pedal_rotation_time
  _speedometer.set_current_pedal_rotation_time(pedal_rotation_time);
}

void BikeSystem::temperature_task() {
  // Temperature task
  _task_manager.register_task_start(TaskManager::TaskType::TemperatureTaskType);

  // no need to protect access to data members (single threaded)
  zpp_lib::ZephyrResult res = _sensor_device.read_temperature(_current_temperature);
  if (!res) {
    ZPP_LOG_ERR("Cannot read temperature: %d", static_cast<int>(res.error()));
  }

  // simulate task computation by waiting for the required task computation time
  _task_manager.simulate_computation_time(TaskManager::TaskType::TemperatureTaskType, kAllowSleep);
}

void BikeSystem::update_reset() {
  std::chrono::microseconds response_time = zpp_lib::Time::get_uptime() - _press_time;

  _speedometer.reset();

  ZPP_LOG_INF("Reset task: response time is %" PRIu64 " usecs", response_time.count());
}

void BikeSystem::display_task1() {
  _task_manager.register_task_start(TaskManager::TaskType::DisplayTask1Type);

  _bike_display.display_speed(_speedometer.get_current_speed());
  _bike_display.display_distance(_speedometer.get_traveled_distance());

  _task_manager.simulate_computation_time(TaskManager::TaskType::DisplayTask1Type, kAllowSleep);
}

void BikeSystem::display_task2() {
  _task_manager.register_task_start(TaskManager::TaskType::DisplayTask2Type);

  _bike_display.display_temperature(_current_temperature);

  _task_manager.simulate_computation_time(TaskManager::TaskType::DisplayTask2Type, kAllowSleep);
}

}  // namespace bike_computer::static_scheduling_with_wq
