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
      _button4(zpp_lib::InterruptIn::PinName::BUTTON4), _button3_token(_button3.add_callback([this]() { on_fall_button3(); })),
      _button4_token(_button4.add_callback([this]() { on_fall_button4(); })) {}

std::chrono::milliseconds PedalDevice::get_current_rotation_time() {
  const auto initial_time = zpp_lib::Time::get_uptime();
  auto elapsed_time       = std::chrono::microseconds::zero();

  // One speed adjustment at most per task execution will prevents rapid changes
  // if the user keeps a button pressed during the polling loop for example
  bool has_changed = false;
  while (elapsed_time < TaskManager::get_task_computation_time(TaskManager::TaskType::SpeedTaskType)) {
    if (!has_changed) {
      // Atomically consume the button 3 event
      if (_button3_state.exchange(false, std::memory_order_acquire)) {
        decrease_rotation_speed();
        has_changed = true;
      }

      // Atomically consume the button 4 event
      if (_button4_state.exchange(false, std::memory_order_acquire)) {
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

void PedalDevice::on_fall_button3() {
  // Button 2 is a modifier: read its current state instead of registering
  // an interrupt, since InterruptIn only provides falling-edge callbacks.
  // Pedal adjustments are allowed only when button 2 is released.
  if (_button2.read() != zpp_lib::kPolarityPressed) {
    // Publish the button 3 event to the task
    _button3_state.store(true, std::memory_order_release);
  }
}

void PedalDevice::on_fall_button4() {
  // Button 2 is a modifier: read its current state instead of registering
  // an interrupt, since InterruptIn only provides falling-edge callbacks.
  // Pedal adjustments are allowed only when button 2 is released.
  if (_button2.read() != zpp_lib::kPolarityPressed) {
    // Publish the button 4 event to the task
    _button4_state.store(true, std::memory_order_release);
  }
}

}  // namespace bike_computer::static_scheduling_with_event
