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
 * @file bike_system.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Bike System header file (static scheduling)
 *
 * @date 2025-07-01
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// stl
#include <atomic>
#include <functional>

// local
#include "gear_device.hpp"
#include "pedal_device.hpp"
#include "reset_device.hpp"

// zpp_lib
#include "zpp_include/display.hpp"
#include "zpp_include/work_queue.hpp"

// from common
#include "common/bike_display.hpp"
#include "common/sensor_device.hpp"
#include "common/speedometer.hpp"
#include "common/task_manager.hpp"
#include "common/ttce.hpp"

namespace bike_computer::static_scheduling_with_wq {

class BikeSystem {
public:
  // constructor
  BikeSystem();

  // destructor
  ~BikeSystem();

  /** Explicity prevent (move) copy and assignment
      rather than inheriting from NonCopyable. This avoids
      cppcoreguidelines-special-member-functions warning by clang-tidy.
  */
  BikeSystem(const BikeSystem&)            = delete;
  BikeSystem(BikeSystem&&)                 = delete;
  BikeSystem& operator=(const BikeSystem&) = delete;
  BikeSystem& operator=(BikeSystem&&)      = delete;

  // method called in main() for starting the system
  [[nodiscard]] zpp_lib::ZephyrResult start();

  // method called for stopping the system
  void stop();

private:
  // private methods
  [[nodiscard]] zpp_lib::ZephyrResult initialize();
  void speed_distance_task();
  void temperature_task();
  void reset_task();
  void display_task1();
  void display_task2();
  void on_reset();
  void on_gear_changed(uint8_t current_gear, uint8_t current_gear_size);
  void on_pedal_rotation_time_changed(std::chrono::milliseconds pedal_rotation_time);
  void gear_task(uint8_t current_gear, uint8_t current_gear_size);
  void speed_distance_task(std::chrono::milliseconds pedal_rotation_time);

  // define a work queue to serve the ISRs
  zpp_lib::WorkQueue _isr_work_queue;
  // define a work attribute for the reset
  zpp_lib::Work<BikeSystem> _reset_work;
  // define a work attribute for the gear
  zpp_lib::Work<BikeSystem, uint8_t, uint8_t> _gear_work;
  // define a work attribute for the pedal
  zpp_lib::Work<BikeSystem, std::chrono::milliseconds> _pedal_work;

  // flag stating whether sleep is allowed when simulating computation times
  static constexpr bool kAllowSleep = true;
  // data member that represents the device used for resetting
  ResetDevice _reset_device;
  // data member that represents the device for manipulating the gear
  GearDevice _gear_device;
  // data member that represents the device for manipulating the pedal rotation
  PedalDevice _pedal_device;
  // data member that represents the display
  BikeDisplay _bike_display;
  // data member that represents the device for counting wheel rotations
  Speedometer _speedometer;
  // data member that represents the sensor device
  SensorDevice _sensor_device;
  float _current_temperature = 0.0F;

  // used for managing tasks info
  TaskManager _task_manager;

  // define task type
  using VoidFunction = std::function<void()>;

  // define cycle parameters based on the Super Loop timing
  static constexpr uint16_t kNbrOfMinorCycles            = 4;
  static constexpr uint16_t kMaxNbrOfTasksInMinorCycles  = 5;  // 3 tasks max + 1 cpu log + 1 margin
  static constexpr std::chrono::milliseconds kMinorCycle = 400ms;

  // instance of the TTCE engine
  TTCE<VoidFunction, kNbrOfMinorCycles, kMaxNbrOfTasksInMinorCycles> _ttce{kMinorCycle};
};

}  // namespace bike_computer::static_scheduling_with_wq
