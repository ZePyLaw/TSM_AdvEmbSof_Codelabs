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
 * @file main.cpp
 * @author Schmid Alexandre <alexandre.schmid@master.hes-so.ch>
 *
 * @brief Basic ztest smoke test for the Zephyr testing infrastructure.
 *
 * @date 2026-10-08
 * @version 1.0.1
 ***************************************************************************/

// zpp_lib
#include "zpp_include/zpp_assert.hpp"
#include "zpp_include/zpp_test.hpp"

namespace {

constexpr int kOperand        = 2;
constexpr int kExpectedSquare = 4;

}  // namespace

// Smoke test for the ztest infrastructure.
// The arithmetic is intentionally trivial: the purpose is to verify that
// Twister can discover, execute, and evaluate a ztest assertion successfully.
ZPP_ZTEST(always_succeed, test_equality) {
  zpp_zassert_equal(kExpectedSquare, kOperand * kOperand);
}

// Register the test suite. No setup, teardown, or per-test hooks are required.
ZPP_ZTEST_SUITE(always_succeed, nullptr, nullptr, nullptr, nullptr, nullptr);
