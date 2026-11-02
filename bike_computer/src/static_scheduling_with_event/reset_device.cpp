// Copyright 2026 Haute école d'ingénierie et d'architecture de Fribourg
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
 * @file reset_device.cpp
 * @author Alexandre Schmid <alexandre.schmid@master.hes-so.ch>
 *
 * @brief ResetDevice implementation (static scheduling)
 *
 * @date 2026-10-20
 * @version 1.0.1
 ***************************************************************************/

#include "reset_device.hpp"

// zpp_lib
#include "zpp_include/time.hpp"

namespace bike_computer::static_scheduling_with_event {

ResetDevice::ResetDevice()
    : _button1(zpp_lib::InterruptIn::PinName::BUTTON1), _button1_token(_button1.add_callback([this]() { on_fall_button1(); })) {}

bool ResetDevice::check_reset() {
  // Load flag with acquire to sync with ISR state write
  if (_button1_state.load(std::memory_order_acquire)) {
    // Clear flag with relaxed order (atomic write, no ordering constraints)
    _button1_state.store(false, std::memory_order_release);
    return true;
  }
  return false;
}

std::chrono::microseconds ResetDevice::get_press_time() {
  return _press_time;
}

void ResetDevice::on_fall_button1() {
  // Timestamp the physical button edge as early as possible in the ISR.
  // Do not trigger the reset here, BikeSystem handles it periodically.
  _press_time = zpp_lib::Time::get_uptime();

  // Set flag with release to safely publish the state change
  _button1_state.store(true, std::memory_order_release);
}

}  // namespace bike_computer::static_scheduling_with_event
