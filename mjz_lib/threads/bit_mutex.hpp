/*MIT License

Copyright (c) 2025 Mjz86

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <atomic>

#include "base.hpp"

#ifndef MJZ_THREADS_bit_mutex_LIB_HPP_FILE_
#define MJZ_THREADS_bit_mutex_LIB_HPP_FILE_
namespace mjz::threads_ns {
MJZ_CONSTANT(bool)
uses_atomic_bool = std::atomic_bool::is_always_lock_free;
using flag_t =
    std::conditional_t<uses_atomic_bool, std::atomic_bool, std::atomic_flag>;
template <partial_same_as<totally_empty_type_t> = totally_empty_type_t>
class alignas(alignof(flag_t)) bit_mutex_t {
  template <class>
  friend class mjz_private_accessed_t;

 private:
  alignas(alignof(flag_t)) char buffer[sizeof(flag_t)]{};

  MJZ_NO_MV_NO_CPY(bit_mutex_t);
  MJZ_NODISCRAD MJZ_MAYBE_UNUSED inline flag_t *just_get() noexcept {
    return reinterpret_cast<flag_t *>(this);
  }
  MJZ_NODISCRAD MJZ_MAYBE_UNUSED inline flag_t *get() noexcept {
    return std::launder(just_get());
  }
  MJZ_NODISCRAD MJZ_MAYBE_UNUSED inline const flag_t *just_get()
      const noexcept {
    return reinterpret_cast<const flag_t *>(this);
  }
  MJZ_NODISCRAD MJZ_MAYBE_UNUSED inline const flag_t *get() const noexcept {
    return std::launder(just_get());
  }
  MJZ_NCX_FN bool test_and_set_m(auto &m) noexcept {
    if constexpr (uses_atomic_bool) {
      return m.exchange(true, std::memory_order_acquire) != 0;
    } else {
      return m.test_and_set(std::memory_order_acquire);
    }
  }
  MJZ_NCX_FN void wait_m(auto &m) noexcept {
#if !MJZ_SLEEP_WITH_WAIT_
    return m.wait(true, std::memory_order_acquire);
#else
    if constexpr (uses_atomic_bool) {
      while (m.load(std::memory_order_acquire));
      {
        std::this_thread::sleep_for(
            std::chrono_literals::operator""ns(MJZ_SLEEP_WITH_WAIT_));
      }
    } else {
      while (m.test(std::memory_order_acquire));
      {
        std::this_thread::sleep_for(
            std::chrono_literals::operator""ns(MJZ_SLEEP_WITH_WAIT_));
      }
    }
    return;
#endif
  }
  MJZ_NCX_FN void unlock_m(auto &m) noexcept {
    if constexpr (uses_atomic_bool) {
      m.store(false, std::memory_order_release);
      if (!m.load(std::memory_order_acquire) != 0) MJZ_IS_LIKELY {
#if !MJZ_SLEEP_WITH_WAIT_
          m.notify_one();
#endif
        }

    } else {
      m.clear(std::memory_order_release);
      if (!m.test(std::memory_order_acquire)) MJZ_IS_LIKELY {
#if !MJZ_SLEEP_WITH_WAIT_
          m.notify_one();
#endif
        }
    }
  }

  MJZ_CX_ND_RES_OBJ_FN
  success_t try_lock_ncx(const bool need_to_wait,
                         uint64_t timeout_count) noexcept {
    MJZ_IF_CONSTEVAL { return true; }
    auto &m = *get();
    uint64_t i{};
    while (i < timeout_count && test_and_set_m(m)) {
      i++;
    }
    if (i < timeout_count) {
      return true;
    }
    if (!need_to_wait) {
      return false;
    }
    while (test_and_set_m(m)) {
      wait_m(m);
    }
    return true;
  }
  MJZ_CX_FN void unlock_ncx() noexcept {
    MJZ_IF_CONSTEVAL { return; }
    unlock_m(*get());
  }
  MJZ_CX_FN void unlock_cx() noexcept {
    MJZ_IFN_CONSTEVAL { return; }
    asserts(!!buffer[0], " the mutex is not locked , why did you unlock??");
    buffer[0] = false;
  }
  MJZ_CX_ND_RES_OBJ_FN
  success_t try_lock_cx(const bool need_to_wait) noexcept {
    MJZ_IFN_CONSTEVAL { return false; }
    if (need_to_wait) {
      asserts(!buffer[0],
              " the mutex is  already locked , why did you re-lock??");
      buffer[0] = true;
      return true;
    }
    if (buffer[0]) {
      return false;
    }
    buffer[0] = true;
    return true;
  }

 public:
  MJZ_CX_FN bit_mutex_t() noexcept {
    MJZ_IF_CONSTEVAL { return; }
    std::construct_at(just_get());
  }
  MJZ_CX_FN ~bit_mutex_t() noexcept {
    MJZ_IF_CONSTEVAL {
      asserts(!*this, "you forgot to unlock this object!!");
      return;
    }
    std::destroy_at(get());
  }
  MJZ_CX_FN void unlock() noexcept {
    MJZ_IFN_CONSTEVAL { return unlock_ncx(); }
    return unlock_cx();
  }
  MJZ_CX_ND_RES_OBJ_FN
  success_t try_lock(const bool need_to_wait, uint64_t timeout_count) noexcept {
    MJZ_IFN_CONSTEVAL { return try_lock_ncx(need_to_wait, timeout_count); }
    return try_lock_cx(need_to_wait);
  }

  MJZ_CX_ND_RES_OBJ_FN success_t try_lock(uint64_t timeout_count) noexcept {
    return try_lock(false, timeout_count);
  }
  MJZ_CX_ND_RES_OBJ_FN success_t try_lock() noexcept {
    return try_lock(defult_timeout);
  }
  MJZ_CX_FN void lock() noexcept(!MJZ_IN_DEBUG_MODE) {
    if (try_lock(true, defult_timeout)) MJZ_IS_LIKELY {
        return;
      }
    asserts.panic(" what happened !? why did the un-fail-able fail?! ");
  }
  /*
   *returns true if its locked
   */
  MJZ_CX_ND_FN
  explicit operator bool() const noexcept {
    MJZ_IF_CONSTEVAL { return !!buffer[0]; }
    return [&](auto &&m) noexcept {
      if constexpr (uses_atomic_bool) {
        return m.load(std::memory_order_acquire);
      } else {
        return m.test(std::memory_order_acquire);
      }
    }(*just_get());
  }
};

};  // namespace mjz::threads_ns
#endif  // MJZ_THREADS_bit_mutex_LIB_HPP_FILE_
