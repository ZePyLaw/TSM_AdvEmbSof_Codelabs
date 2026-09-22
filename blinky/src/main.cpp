/**
******************************************************************************
* @file        : main.cpp
* @brief       : Blinky Application
* @author      : Alexandre Schmid <alexandre.schmid@master.hes-so.ch>
* @date        : 22. September 2026
******************************************************************************
* @copyright   : Copyright (c) 2026
*                HES-SO Master
* @attention   : SPDX-License-Identifier: MIT OR Apache-2.0
******************************************************************************
* @details
* Making LED1 on the board toggle between on and off repeatedly
******************************************************************************
*/

// stl
#include <chrono>

// zpp_lib
#include "zpp_include/digital_out.hpp"
#include "zpp_include/this_thread.hpp"
#include "zpp_include/thread.hpp"
#include "zpp_include/zpp_log.hpp"

ZPP_LOG_MODULE_REGISTER(blinky, CONFIG_APP_LOG_LEVEL);

void blink() {
  zpp_lib::DigitalOut led(zpp_lib::DigitalOut::PinName::LED0);
  using std::literals::chrono_literals::operator""ms;
  static constexpr std::chrono::milliseconds kBlinkInterval = 1000ms;

  while (true) {
    led = !led;
    zpp_lib::ThisThread::sleep_for(kBlinkInterval);
  }
}

// The complexity is increased by Zephyr logging macros.
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
int main() {
  ZPP_LOG_DBG("Running on board %s", CONFIG_BOARD_TARGET);
  ZPP_LOG_DBG("Starting thread");

  zpp_lib::Thread thread(zpp_lib::PreemptableThreadPriority::PriorityNormal, "BlinkyThread");

  auto res = thread.start(blink);
  if (!res) {
    ZPP_LOG_ERR("Could not start thread (%d)", static_cast<int>(res.error()));
    return -1;
  }

  res = thread.join();
  if (!res) {
    ZPP_LOG_ERR("Could not terminate thread (%d)", static_cast<int>(res.error()));
    return -1;
  }

  return 0;
}