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
#include "zpp_include/this_thread.hpp"
#include "zpp_include/zpp_log.hpp"

ZPP_LOG_MODULE_REGISTER(template_application, CONFIG_APP_LOG_LEVEL);

// The complexity is increased by zephyr macros
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
int main() {
  ZPP_LOG_DBG("Running on board %s", CONFIG_BOARD_TARGET);

  using std::literals::chrono_literals::operator""s;
  static constexpr auto kSleepDuration = 1s;
  while (true) {
    ZPP_LOG_DBG("TemplateApplication is running...");
    zpp_lib::ThisThread::sleep_for(kSleepDuration);
  }
  return 0;
}
