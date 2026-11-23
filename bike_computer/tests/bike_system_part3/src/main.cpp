
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
static constexpr std::chrono::milliseconds kTestDuration(CONFIG_TEST_DURATION_IN_MS);

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
      bike_system.stop();

      // wait for thread to terminate
      res = thread.join();
      zpp_zassert_true(res, "Could not join thread");
    }

    // If this message appears, the Thread destructor has completed.
  }

  // If this message appears, the BikeSystem destructor has completed.
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

// Test the gear selection using the emulated buttons.
// Button 2 must be held while pressing button 3 or button 4.
ZPP_ZTEST(bike_system_part3, test_gear_device_with_wq) {
  using GearDevice = bike_computer::static_scheduling_with_wq::GearDevice;

  uint8_t current_gear      = GearDevice::kInitialGear;
  uint8_t current_gear_size = GearDevice::kInitialGearSize;
  unsigned callback_count   = 0U;

  GearDevice gear_device([&](uint8_t gear, uint8_t gear_size) {
    current_gear      = gear;
    current_gear_size = gear_size;
    ++callback_count;
  });

  auto& modifier_button  = gear_device.get_modifier_button();
  auto& gear_down_button = gear_device.get_gear_down_button();
  auto& gear_up_button   = gear_device.get_gear_up_button();

  // Simulate a complete button press and release.
  auto press_button = [](zpp_lib::InterruptIn& button) {
    button.write(!zpp_lib::kPolarityPressed);
    button.write(zpp_lib::kPolarityPressed);
    button.write(!zpp_lib::kPolarityPressed);
  };

  // Make sure all buttons start released.
  modifier_button.write(!zpp_lib::kPolarityPressed);
  gear_down_button.write(!zpp_lib::kPolarityPressed);
  gear_up_button.write(!zpp_lib::kPolarityPressed);

  // Without button 2, gear changes must be ignored.
  press_button(gear_up_button);
  press_button(gear_down_button);

  zpp_zassert_true(callback_count == 0U, "Gear changed without modifier button");

  // Hold button 2.
  modifier_button.write(zpp_lib::kPolarityPressed);

  // Increase the gear up to the maximum.
  uint8_t expected_gear = GearDevice::kInitialGear;

  while (expected_gear < bike_computer::kMaxGear) {
    press_button(gear_up_button);
    ++expected_gear;

    zpp_zassert_true(current_gear == expected_gear, "Gear did not increase correctly");

    zpp_zassert_true(current_gear_size == bike_computer::kMaxGearSize - expected_gear, "Gear size incorrect after increasing");
  }

  // Pressing UP at the maximum must not exceed the limit.
  press_button(gear_up_button);

  zpp_zassert_true(current_gear == bike_computer::kMaxGear, "Gear exceeded maximum value");

  // Decrease the gear down to the minimum.
  while (expected_gear > bike_computer::kMinGear) {
    press_button(gear_down_button);
    --expected_gear;

    zpp_zassert_true(current_gear == expected_gear, "Gear did not decrease correctly");

    zpp_zassert_true(current_gear_size == bike_computer::kMaxGearSize - expected_gear, "Gear size incorrect after decreasing");
  }

  // Pressing DOWN at the minimum must not exceed the limit.
  press_button(gear_down_button);

  zpp_zassert_true(current_gear == bike_computer::kMinGear, "Gear went below minimum value");

  // Releasing button 2 must disable gear adjustments again.
  modifier_button.write(!zpp_lib::kPolarityPressed);

  const unsigned callbacks_before_release = callback_count;

  press_button(gear_up_button);

  zpp_zassert_true(callback_count == callbacks_before_release, "Gear changed after releasing modifier button");
}

ZPP_ZTEST_SUITE(bike_system_part3, nullptr, nullptr, nullptr, nullptr, nullptr);
