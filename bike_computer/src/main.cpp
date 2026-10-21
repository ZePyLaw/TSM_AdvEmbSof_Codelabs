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
 *
 * @brief Main function of the TemplateApplication program
 *
 * @date 2026-06-18
 * @version 1.0.0
 ***************************************************************************/

// zephyr

// stl

// zpp_lib
#include "zpp_include/zpp_log.hpp"

// Include BikeSystem only when static scheduling is enabled.
#if defined(CONFIG_BIKE_COMPUTER_STATIC_SCHEDULING)
#include "static_scheduling/bike_system.hpp"
#endif

ZPP_LOG_MODULE_REGISTER(bike_computer, CONFIG_APP_LOG_LEVEL);

int main() {
  ZPP_LOG_DBG("Running on board %s", CONFIG_BOARD_TARGET);

#if defined(CONFIG_BIKE_COMPUTER_STATIC_SCHEDULING)
  // Start the BikeComputer static cyclic scheduler.
  bike_computer::static_scheduling::BikeSystem bike_system;

  auto result = bike_system.start();
  if (!result) {
    ZPP_LOG_ERR("BikeSystem failed: %d", static_cast<int>(result.error()));
    return -1;
  }
#else
  // Avoid referencing BikeSystem when this feature is disabled.
  ZPP_LOG_WRN("BikeComputer static scheduling is disabled");
#endif

  return 0;
}
