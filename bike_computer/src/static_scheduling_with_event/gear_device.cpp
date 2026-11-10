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
 * @file gear_device.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 * @author Alexandre Schmid <alexandre.schmid@master.hes-so.ch>
 *
 * @brief GearDevice implementation (static scheduling)
 *
 * @date 2026-10-20
 * @version 1.0.1
 ***************************************************************************/

#include "gear_device.hpp"

namespace bike_computer::static_scheduling_with_event {

GearDevice::GearDevice()
    : _button2(zpp_lib::InterruptIn::PinName::BUTTON2), _button3(zpp_lib::InterruptIn::PinName::BUTTON3),
      _button4(zpp_lib::InterruptIn::PinName::BUTTON4), _button3_token(_button3.add_callback([this]() { this->on_fall_button3(); })),
      _button4_token(_button4.add_callback([this]() { this->on_fall_button4(); })) {}

uint8_t GearDevice::get_current_gear() {
  // Consume events captured by the interrupt callbacks.
  // The TaskManager handles the simulated computation time.
  if (_button3_state.exchange(false, std::memory_order_acquire)) {
    if (_current_gear > bike_computer::kMinGear) {
      _current_gear--;
    }
  }

  if (_button4_state.exchange(false, std::memory_order_acquire)) {
    if (_current_gear < bike_computer::kMaxGear) {
      _current_gear++;
    }
  }

  return _current_gear;
}

uint8_t GearDevice::get_current_gear_size() const {
  return bike_computer::kMaxGearSize - _current_gear;
}

void GearDevice::on_fall_button3() {
  // Button 2 is a modifier: read its current state instead of registering
  // an interrupt, since InterruptIn only provides falling-edge callbacks.
  // Gear adjustments are allowed only while button 2 is pressed.
  if (_button2.read() == zpp_lib::kPolarityPressed) {
    _button3_state.store(true, std::memory_order_release);
  }
}

void GearDevice::on_fall_button4() {
  // Button 2 is a modifier: read its current state instead of registering
  // an interrupt, since InterruptIn only provides falling-edge callbacks.
  // Gear adjustments are allowed only while button 2 is pressed.
  if (_button2.read() == zpp_lib::kPolarityPressed) {
    _button4_state.store(true, std::memory_order_release);
  }
}

}  // namespace bike_computer::static_scheduling_with_event
