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
 * @brief Tests C++ smart-pointer and raw-pointer ownership semantics.
 *
 * @date 2026-10-08
 * @version 1.0.1
 ***************************************************************************/

// stl
#include <atomic>
#include <cstdint>
#include <memory>
#include <utility>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zpp_assert.hpp"
#include "zpp_include/zpp_test.hpp"

namespace {

constexpr uint32_t kNoLiveInstances = 0U;
constexpr uint32_t kOneLiveInstance = 1U;

}  // namespace

// Structure used to observe object lifetime in the pointer tests
// Construction increments the number of live Test instances and destruction
// decrements it. This lets the tests verify when ownership actually ends
//
// We inherit from zpp_lib::NonCopyable to prevent accidental copies of the Test class
// cppcoreguidelines-special-member-functions is therefore a false positive
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct Test : public zpp_lib::NonCopyable {
  Test() : _value(kMagicNumber) {
    s_instance_count++;
  }

  ~Test() {
    s_instance_count--;
    _value = 0;
  }

  int _value                                           = 0;
  static constexpr uint32_t kMagicNumber               = 33;
  static inline std::atomic<uint32_t> s_instance_count = 0;
};

// A shared_ptr automatically destroys its object when the final owner
// disappears. Here there is only one owner, scoped inside the inner block
ZPP_ZTEST(ptr_test, test_single_sharedptr_lifetime) {
  zpp_zassert_equal(0, Test::s_instance_count);

  {
    std::shared_ptr<Test> shared_ptr = std::make_shared<Test>();

    zpp_zassert_equal(1, Test::s_instance_count);
    zpp_zassert_equal(Test::kMagicNumber, shared_ptr->_value);
  }

  // Leaving the scope destroys the only shared_ptr and therefore the Test
  zpp_zassert_equal(0, Test::s_instance_count);
}

// Two shared_ptr instances may share ownership of the same object
// Destroying one owner must not destroy the object while another owner remains
ZPP_ZTEST(ptr_test, test_instance_sharing) {
  std::shared_ptr<Test> shared_ptr1(nullptr);

  zpp_zassert_equal(0, Test::s_instance_count);

  {
    std::shared_ptr<Test> shared_ptr2 = std::make_shared<Test>();

    zpp_zassert_equal(1, Test::s_instance_count);

    shared_ptr1 = shared_ptr2;

    // Both smart pointers refer to the same single Test object
    zpp_zassert_equal(1, Test::s_instance_count);
    zpp_zassert_equal(Test::kMagicNumber, shared_ptr1->_value);
    zpp_zassert_true(shared_ptr1.get() == shared_ptr2.get());
  }

  // shared_ptr2 is gone, but shared_ptr1 still owns the Test
  zpp_zassert_equal(1, Test::s_instance_count);

  shared_ptr1 = nullptr;

  // The final owner has released the object, so its destructor has run
  zpp_zassert_equal(0, Test::s_instance_count);
}

// A unique_ptr has exactly one owner. std::move transfers that ownership
// without creating or destroying the underlying object
ZPP_ZTEST(ptr_test, test_unique_ptr) {
  zpp_zassert_equal(0, Test::s_instance_count);

  {
    std::unique_ptr<Test> unique_ptr1 = std::make_unique<Test>();

    zpp_zassert_equal(1, Test::s_instance_count);
    zpp_zassert_equal(Test::kMagicNumber, unique_ptr1->_value);

    std::unique_ptr<Test> unique_ptr2 = std::move(unique_ptr1);

    // Ownership moved to unique_ptr2; unique_ptr1 no longer owns anything
    zpp_zassert_true(unique_ptr1 == nullptr);
    zpp_zassert_equal(1, Test::s_instance_count);
    zpp_zassert_equal(Test::kMagicNumber, unique_ptr2->_value);
  }

  // The unique owner left scope, so the Test object was destroyed
  zpp_zassert_equal(0, Test::s_instance_count);
}

// Raw owning pointers do not manage lifetime automatically
// This exercise deliberately uses new/delete to verify that an explicit
// delete invokes the destructor and releases the Test object
ZPP_ZTEST(ptr_test, test_raw_pointers) {
  zpp_zassert_equal(0, Test::s_instance_count);

  // This exercise explicitly requires a raw owning pointer so that manual
  // allocation and deallocation can be tested
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  Test* raw_ptr = new Test();

  zpp_zassert_equal(1, Test::s_instance_count);
  zpp_zassert_equal(Test::kMagicNumber, raw_ptr->_value);

  // Explicit delete is required by the exercise to verify that destroying
  // the raw-owned object invokes Test::~Test()
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  delete raw_ptr;

  zpp_zassert_equal(0, Test::s_instance_count);
}

// Register the pointer test suite. No lifecycle hooks are needed here
ZPP_ZTEST_SUITE(ptr_test, nullptr, nullptr, nullptr, nullptr, nullptr);
