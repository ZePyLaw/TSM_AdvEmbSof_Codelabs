
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
 * @file test_bike_system_part3.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 * @author Robin Schaerrer <robin.schaerrer@master.hes-so.ch>
 *
 * @brief Tests for BikeSystem Part III
 *
 * @date 2026-11-23
 * @version 1.0.0
 ***************************************************************************/
// std
#include <chrono>
#include <cstdio>
#include <cstdlib>

// zephyr
#include <zephyr/sys/printk.h>

// zpp_lib
#include "zpp_include/event.hpp"
#include "zpp_include/this_thread.hpp"
#include "zpp_include/thread.hpp"
#include "zpp_include/time.hpp"
#include "zpp_include/zpp_assert.hpp"
#include "zpp_include/zpp_log.hpp"
#include "zpp_include/zpp_test.hpp"

// bike computer
#include "static_scheduling_with_wq/bike_system.hpp"

ZPP_LOG_MODULE_REGISTER(bike_computer, CONFIG_APP_LOG_LEVEL);

// for ms or s literals
using std::literals::chrono_literals::operator""s;

// The value of TEST_DURATION_IN_MS is set in the test case configuration file (testcase.yaml) and
// can be overridden for each platform. The value should be set so that the test passes on your platform.
// It should not be below 20s for any platform.
// TEMPORARY DEBUG ONLY: restore CONFIG_TEST_DURATION_IN_MS before final validation.
static constexpr std::chrono::milliseconds kTestDuration(2000);

// test_bike_system_static_with_wq handler function
ZPP_ZTEST(bike_system_part3, test_bike_system_static_with_wq) {
  {
    // create the BikeSystem instance
    bike_computer::static_scheduling_with_wq::BikeSystem bike_system;

    {
      // run the bike system in a separate thread
      zpp_lib::Thread thread(zpp_lib::PreemptableThreadPriority::PriorityNormal, "Test BS WQ");
      ZPP_LOG_DBG("Starting thread");
      auto res = thread.start([&bike_system]() {
        auto res = bike_system.start();
        zpp_zassert_true(res, "BikeSystem start failed");
      });
      zpp_zassert_true(res, "Could not start thread");

      // let the bike system run for the test duration
      zpp_lib::ThisThread::sleep_for(kTestDuration);

      // stop the bike system
      printk("[DIAG] Sleep finished, calling stop()\n");
      bike_system.stop();
      printk("[DIAG] stop() returned\n");

      // wait for thread to terminate
      printk("[DIAG] Calling join()\n");
      res = thread.join();
      printk("[DIAG] join() returned\n");
      zpp_zassert_true(res, "Could not join thread");

      printk("[DIAG] Final assertion passed, leaving thread scope\n");
    }

    // If this message appears, the Thread destructor has completed.
    printk("[DIAG] Thread object destroyed\n");
  }

  // If this message appears, the BikeSystem destructor has completed.
  printk("[DIAG] BikeSystem object destroyed\n");
}

// test_reset_bike_system_with_wq handler function
// Using global static variables is acceptable for this test, since they
// are only used in this test and not in the BikeSystem class
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static std::chrono::microseconds s_reset_time = std::chrono::microseconds::zero();
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static zpp_lib::Event s_reset_event;
static constexpr uint32_t kResetEventFlag = (1UL << 0);
static void reset_callback() {
  s_reset_time = zpp_lib::Time::get_uptime();
  s_reset_event.set(kResetEventFlag);
}

ZPP_ZTEST(bike_system_part3, test_reset_bike_system_with_wq) {
  // create the BikeSystem instance
  bike_computer::static_scheduling_with_wq::BikeSystem bike_system;

  // run the bike system in a separate thread
  zpp_lib::Thread thread(zpp_lib::PreemptableThreadPriority::PriorityNormal, "Test BS MT");
  auto res = thread.start([&bike_system]() {
    auto res = bike_system.start();
    zpp_zassert_true(res, "Failed to start BikeSystem");
  });
  zpp_zassert_true(res, "Could not start thread");

  // let the bike system run for 10 secs
  // this will test whether scheduling is ok
  static constexpr auto kInitialTestDuration = 10s;
  zpp_lib::ThisThread::sleep_for(kInitialTestDuration);

  // test reset on BikeSystem
  bike_system.get_speedometer().set_on_reset_callback(reset_callback);

  // instantiate the reset button
  zpp_lib::InterruptIn& reset_button = bike_system.get_reset_device().get_reset_button();

  // check for reset response time
  constexpr uint8_t kNbrOfResets               = 10;
  std::chrono::microseconds last_response_time = std::chrono::microseconds::zero();
  for (uint8_t i = 0; i < kNbrOfResets; i++) {
    // reset the BikeSystem by simulating button press
    reset_button.write(zpp_lib::kPolarityPressed);

    // take time before waiting for reset
    auto start_time = zpp_lib::Time::get_uptime();

    // wait for resetCallback to be called
    s_reset_event.wait_any(kResetEventFlag);

    // get the response time and check it
    auto response_time = s_reset_time - start_time;

    // check that the response time is within the expected margin
    // cppcheck generates an internal error with 20us
    constexpr std::chrono::microseconds kMaxExpectedResponseTime(100);
    zpp_zassert_true(response_time <= kMaxExpectedResponseTime, "Response time is too long: %lld usecs", response_time.count());

    printf("Reset task: response time is %lld usecs\n", response_time.count());

    using std::literals::chrono_literals::operator""us;
    constexpr auto kMaxExpectedJitter = 31us;
    if (i > 0) {
      auto jitter = response_time - last_response_time;
      zpp_zassert_true(std::abs(jitter.count()) <= kMaxExpectedJitter.count(), "Response time jitter is too high");
    }
    last_response_time = response_time;

    // unpress the button
    reset_button.write(!zpp_lib::kPolarityPressed);

    // let the bike system run for 2 secs
    static constexpr auto kSleepDurationAfterReset = 2s;
    zpp_lib::ThisThread::sleep_for(kSleepDurationAfterReset);
  }

  // stop the bike system
  bike_system.stop();

  res = thread.join();
  zpp_zassert_true(res, "Could not join thread");
}

ZPP_ZTEST_SUITE(bike_system_part3, nullptr, nullptr, nullptr, nullptr, nullptr);
