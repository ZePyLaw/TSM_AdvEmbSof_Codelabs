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
 * @file pedal_device.cpp
 * @author Alexandre Schmid <alexandre.schmid@master.hes-so.ch>
 *
 * @brief PedalDevice implementation (static scheduling)
 *
 * @date 2026-10-20
 * @version 1.0.1
 ***************************************************************************/

#include "pedal_device.hpp"

// std
#include <chrono>

// from common
#include "common/task_manager.hpp"

// zpp_lib
#include "zpp_include/time.hpp"

namespace bike_computer::static_scheduling_with_event {

// Initialize the hardware button wrappers before entering the super-loop.
PedalDevice::PedalDevice()
    : _button2(zpp_lib::InterruptIn::PinName::BUTTON2), _button3(zpp_lib::InterruptIn::PinName::BUTTON3),
      _button4(zpp_lib::InterruptIn::PinName::BUTTON4) {}

std::chrono::milliseconds PedalDevice::get_current_rotation_time() {
  const auto initial_time = zpp_lib::Time::get_uptime();
  auto elapsed_time       = std::chrono::microseconds::zero();

  // One speed adjustment at most per task execution will prevents rapid changes
  // if the user keeps a button pressed during the polling loop for example
  bool has_changed = false;
  while (elapsed_time < TaskManager::get_task_computation_time(TaskManager::TaskType::SpeedTaskType)) {
    // Button 2 is the gear modifier, so we ignore pedal changes while it is held.
    if (!has_changed && _button2.read() != zpp_lib::kPolarityPressed) {
      if (_button3.read() == zpp_lib::kPolarityPressed) {
        decrease_rotation_speed();
        has_changed = true;
      } else if (_button4.read() == zpp_lib::kPolarityPressed) {
        increase_rotation_speed();
        has_changed = true;
      }
    }

    // Busy-wait for the prescribed duration of the speed task
    elapsed_time = zpp_lib::Time::get_uptime() - initial_time;
  }

  return _pedal_rotation_time;
}

void PedalDevice::increase_rotation_speed() {
  // Faster pedaling means less time per rotation. Saturate at the minimum.
  if (_pedal_rotation_time > bike_computer::kMinPedalRotationTime) {
    _pedal_rotation_time -= bike_computer::kDeltaPedalRotationTime;
    if (_pedal_rotation_time < bike_computer::kMinPedalRotationTime) {
      _pedal_rotation_time = bike_computer::kMinPedalRotationTime;
    }
  }
}

void PedalDevice::decrease_rotation_speed() {
  // Slower pedaling means more time per rotation. Saturate at the maximum.
  if (_pedal_rotation_time < bike_computer::kMaxPedalRotationTime) {
    _pedal_rotation_time += bike_computer::kDeltaPedalRotationTime;
    if (_pedal_rotation_time > bike_computer::kMaxPedalRotationTime) {
      _pedal_rotation_time = bike_computer::kMaxPedalRotationTime;
    }
  }
}

}  // namespace bike_computer::static_scheduling_with_event
