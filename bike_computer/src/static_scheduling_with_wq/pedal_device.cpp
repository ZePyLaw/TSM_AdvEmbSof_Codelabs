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

namespace bike_computer::static_scheduling_with_wq {

PedalDevice::PedalDevice(CallbackFunction callback)
    : _pedal_callback(std::move(callback)), _button2(zpp_lib::InterruptIn::PinName::BUTTON2),
      _button3(zpp_lib::InterruptIn::PinName::BUTTON3), _button4(zpp_lib::InterruptIn::PinName::BUTTON4),
      _button3_token(_button3.add_callback([this]() { this->on_fall_button3(); })),
      _button4_token(_button4.add_callback([this]() { this->on_fall_button4(); })) {}

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
    // Update the rotation speed
    decrease_rotation_speed();

    // Signal the BikeSystem using the registered callback
    _pedal_callback(_pedal_rotation_time);
  }
}

void PedalDevice::on_fall_button4() {
  // Button 2 is a modifier: read its current state instead of registering
  // an interrupt, since InterruptIn only provides falling-edge callbacks.
  // Pedal adjustments are allowed only when button 2 is released.
  if (_button2.read() != zpp_lib::kPolarityPressed) {
    // Update the rotation speed
    increase_rotation_speed();

    // Signal the BikeSystem using the registered callback
    _pedal_callback(_pedal_rotation_time);
  }
}

}  // namespace bike_computer::static_scheduling_with_wq
