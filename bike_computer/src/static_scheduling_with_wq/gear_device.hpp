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
 * @file gear_device.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 * @author Alexandre Schmid <alexandre.schmid@master.hes-so.ch>
 *
 * @brief GearDevice header file (static scheduling)
 *
 * @date 2026-10-20
 * @version 1.0.1
 ***************************************************************************/

#pragma once

// std
#include <atomic>

// Ensure atomic bool operations are lock-free for ISR usage.
static_assert(std::atomic<bool>::is_always_lock_free, "Atomic bool must be lock-free for ISR usage");

// local
#include "common/constants.hpp"

// zpp_lib
#include "zpp_include/interrupt_in.hpp"
#include "zpp_include/non_copyable.hpp"

namespace bike_computer::static_scheduling_with_wq {

// Encapsulates gear selection using the physical board buttons.
// Button 2 must be held while pressing Button 3 or Button 4.
class GearDevice : private zpp_lib::NonCopyable {
public:
  // Define the reset callback function type
  using CallbackFunction = std::function<void(uint8_t, uint_8t)>;

  // GearDevice initial state
  static constexpr uint8_t kInitialGear = bike_computer::kMinGear;
  static constexpr uint8_t kInitialGearSize = bike_computer::kMaxGearSize - kInitialGear;

  GearDevice(CallbackFunction callback);

private:
  // Called by zpp_lib when the buttons interrupts occurs (ISR context).
  void on_fall_button3();
  void on_fall_button4();

  // GearDevice callback
  CallbackFunction _gear_callback;

  // Start with the minimum gear defined in the common constants.
  uint8_t _current_gear = kInitialGear;

  // Hardware inputs used to detect button presses.
  zpp_lib::InterruptIn _button2;
  zpp_lib::InterruptIn _button3;
  zpp_lib::InterruptIn _button4;

  // RAII token unregisters the callback when the object is destroyed.
  zpp_lib::RegistrationToken _button3_token;
  zpp_lib::RegistrationToken _button4_token;
};

}  // namespace bike_computer::static_scheduling_with_wq
