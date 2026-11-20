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
 * @file reset_device.cpp
 * @author Alexandre Schmid <alexandre.schmid@master.hes-so.ch>
 *
 * @brief ResetDevice implementation (static scheduling)
 *
 * @date 2026-10-20
 * @version 1.0.1
 ***************************************************************************/

#include "reset_device.hpp"

namespace bike_computer::static_scheduling_with_wq {

ResetDevice::ResetDevice(CallbackFunction callback)
    : _button1(zpp_lib::InterruptIn::PinName::BUTTON1),
      _button1_token(_button1.add_callback(callback)) {}

}  // namespace bike_computer::static_scheduling_with_wq
