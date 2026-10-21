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
 * @file constants.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 * @author Alexandre Schmid <alexandre.schmid@master.hes-so.ch>
 *
 * @brief Constants used by the BikeComputer application
 *
 * @date 2026-10-20
 * @version 1.0.1
 ***************************************************************************/

#pragma once

// std
#include <chrono>
#include <cstdint>

namespace bike_computer {

// -------------------------------------------------------------------------
// Gear constants
// -------------------------------------------------------------------------

// Allowed gear range: gear 1 to gear 9.
static constexpr uint8_t kMinGear = 1;
static constexpr uint8_t kMaxGear = 9;

// Gear size represents the rear sprocket size.
// Higher gears correspond to smaller rear sprockets.
static constexpr uint8_t kMaxGearSize = 20;
static constexpr uint8_t kMinGearSize = kMaxGearSize - kMaxGear;

// -------------------------------------------------------------------------
// Pedal rotation constants
// -------------------------------------------------------------------------

// Enable the "ms" suffix for std::chrono::milliseconds literals.
using std::literals::chrono_literals::operator""ms;

// Initial pedal rotation time: 750 ms per turn (80 RPM).
static constexpr std::chrono::milliseconds kInitialPedalRotationTime = 750ms;

// Minimum rotation time: 375 ms per turn (160 RPM).
// A shorter rotation time corresponds to faster pedaling.
static constexpr std::chrono::milliseconds kMinPedalRotationTime = 375ms;

// Maximum rotation time: 1500 ms per turn (40 RPM).
// A longer rotation time corresponds to slower pedaling.
static constexpr std::chrono::milliseconds kMaxPedalRotationTime = 1500ms;

// Each button press changes the rotation time by 25 ms.
static constexpr std::chrono::milliseconds kDeltaPedalRotationTime = 25ms;

}  // namespace bike_computer
