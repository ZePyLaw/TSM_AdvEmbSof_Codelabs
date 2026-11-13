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
 * @file reset_device.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 * @author Alexandre Schmid <alexandre.schmid@master.hes-so.ch>
 *
 * @brief ResetDevice header file (static scheduling)
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

// zpp_lib
#include "zpp_include/interrupt_in.hpp"
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/registration_token.hpp"

namespace bike_computer::static_scheduling_with_wq {

// Polls the reset button, while an interrupt records the press timestamp.
class ResetDevice : private zpp_lib::NonCopyable {
public:
  ResetDevice();

  // In Part I, the reset is handled only while button 1 is still pressed.
  bool check_reset();

  // Timestamp captured by the GPIO callback (used for response-time logging).
  std::chrono::microseconds get_press_time();

private:
  // Called by zpp_lib when the button interrupt occurs (ISR context).
  void on_fall_button1();

  // Hardware input used to detect button presses.
  zpp_lib::InterruptIn _button1;

  // Button state flag
  std::atomic<bool> _button1_state{false};

  // RAII token unregisters the callback when the object is destroyed.
  zpp_lib::RegistrationToken _button1_token;
  std::chrono::microseconds _press_time{std::chrono::microseconds::zero()};
};

}  // namespace bike_computer::static_scheduling_with_wq
