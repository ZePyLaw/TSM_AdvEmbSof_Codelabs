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
 * @file main.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 # @author Robin Schaerrer <robin.schaerrer@master.hes-so.ch>
 *
 * @brief Main function of the BikeComputer program
 *
 * @date 2026-10-25
 * @version 1.0.1
 ***************************************************************************/

// zephyr

// stl

// zpp_lib
#include "zpp_include/zpp_log.hpp"

// Include BikeSystem implementations conditionally
#ifdef CONFIG_BIKE_COMPUTER_STATIC_SCHEDULING
#include "static_scheduling/bike_system.hpp"

#elif defined(CONFIG_BIKE_COMPUTER_STATIC_SCHEDULING_WITH_EVENT)
#include "static_scheduling_with_event/bike_system.hpp"

#elif defined(CONFIG_BIKE_COMPUTER_STATIC_SCHEDULING_WITH_WQ)
#include "static_scheduling_with_wq/bike_system.hpp"
#endif

ZPP_LOG_MODULE_REGISTER(bike_computer, CONFIG_APP_LOG_LEVEL);

int main() {
  ZPP_LOG_DBG("Running on board %s", CONFIG_BOARD_TARGET);

#ifdef CONFIG_BIKE_COMPUTER_STATIC_SCHEDULING
  // Start the BikeComputer static cyclic scheduler.
  bike_computer::static_scheduling::BikeSystem bike_system;

  auto result = bike_system.start();
  if (!result) {
    ZPP_LOG_ERR("BikeSystem failed: %d", static_cast<int>(result.error()));
    return -1;
  }

#elif defined(CONFIG_BIKE_COMPUTER_STATIC_SCHEDULING_WITH_EVENT)
  // Start the BikeComputer static scheduler with events.
  bike_computer::static_scheduling_with_event::BikeSystem bike_system;

  auto result = bike_system.start();
  if (!result) {
    ZPP_LOG_ERR("BikeSystem with events failed: %d", static_cast<int>(result.error()));
    return -1;
  }
#elif defined(CONFIG_BIKE_COMPUTER_STATIC_SCHEDULING_WITH_WQ)
  // Start the BikeComputer static scheduler with work queue.
  bike_computer::static_scheduling_with_wq::BikeSystem bike_system;

  auto result = bike_system.start();
  if (!result) {
    ZPP_LOG_ERR("BikeSystem with work queue failed: %d", static_cast<int>(result.error()));
    return -1;
  }
#else
  ZPP_LOG_WRN("No BikeComputer scheduling mode enabled");
#endif

  return 0;
}
