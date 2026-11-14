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
 * @file pedal_device.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 * @author Alexandre Schmid <alexandre.schmid@master.hes-so.ch>
 *
 * @brief PedalDevice header file (static scheduling)
 *
 * @date 2026-10-20
 * @version 1.0.1
 ***************************************************************************/

#pragma once

// std
#include <atomic>
#include <chrono>

// Ensure atomic bool operations are lock-free for ISR usage.
static_assert(std::atomic<bool>::is_always_lock_free, "Atomic bool must be lock-free for ISR usage");

// local
#include "common/constants.hpp"

// zpp_lib
#include "zpp_include/interrupt_in.hpp"
#include "zpp_include/non_copyable.hpp"

namespace bike_computer::static_scheduling_with_wq {

// Simulates a pedal cadence adjusted with buttons 3 and 4.
// Button 2 must NOT be held: otherwise the buttons control the gear.
class PedalDevice : private zpp_lib::NonCopyable {
public:
  // Define the reset callback function type
  using CallbackFunction = std::function<void(std::chrono::milliseconds)>;

  // PedalDevice initial state
  static constexpr std::chrono::milliseconds kInitialRotationTime = bike_computer::kInitialPedalRotationTime;

  PedalDevice(CallbackFunction callback);

private:
  // Speed increases when rotation time decreases, and vice versa.
  void increase_rotation_speed();
  void decrease_rotation_speed();

  // Called by zpp_lib when the buttons interrupts occurs (ISR context).
  void on_fall_button3();
  void on_fall_button4();

  CallbackFunction _pedal_callback;

  // Initial cadence is configured in the common constants.
  std::chrono::milliseconds _pedal_rotation_time = bike_computer::kInitialPedalRotationTime;

  // Hardware inputs. NonCopyable prevents duplicating their ownership.
  zpp_lib::InterruptIn _button2;
  zpp_lib::InterruptIn _button3;
  zpp_lib::InterruptIn _button4;

  // RAII token unregisters the callback when the object is destroyed.
  zpp_lib::RegistrationToken _button3_token;
  zpp_lib::RegistrationToken _button4_token;
};

}  // namespace bike_computer::static_scheduling_with_wq
