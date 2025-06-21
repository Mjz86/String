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
#include "atomic_ref.hpp"
#ifndef MJZ_THREADS_mutex_ref_LIB_HPP_FILE_
#define MJZ_THREADS_mutex_ref_LIB_HPP_FILE_
namespace mjz::threads_ns {
template <std::integral T>
class mutex_ref_t : private atomic_ref_t<T> {
  template <class>
  friend class mjz_private_accessed_t;
  MJZ_CONSTANT(T) one { 1 };
  MJZ_CONSTANT(T) zero { 0 };

 private:
  MJZ_NCX_FN bool test_and_set_m() noexcept {
    return this->exchange(one, std::memory_order_acquire) != zero;
  }
  MJZ_NCX_FN void wait_m() noexcept {
    return this->try_wait(one, std::memory_order_acquire);
  }
  MJZ_NCX_FN void unlock_m() noexcept {
    this->store(false, std::memory_order_release);
    if (this->load(std::memory_order_acquire) == zero) MJZ_IS_LIKELY {
        this->try_notify_one();
      }
  }

  MJZ_CX_ND_RES_OBJ_FN
  success_t try_lock_ncx(const bool need_to_wait,
                         uint64_t timeout_count) noexcept {
    MJZ_IF_CONSTEVAL { return true; }
    uint64_t i{};
    while (i < timeout_count && this->test_and_set_m()) {
      i++;
    }
    if (i < timeout_count) {
      return true;
    }
    if (!need_to_wait) {
      return false;
    }
    while (this->test_and_set_m()) {
      this->wait_m();
    }
    return true;
  }
  MJZ_CX_FN void unlock_ncx() noexcept {
    MJZ_IF_CONSTEVAL { return; }
    this->unlock_m();
  }
  MJZ_CX_FN void unlock_cx() noexcept {
    MJZ_IFN_CONSTEVAL { return; }
    asserts(!!this->load(), " the mutex is not locked , why did you unlock??");
    this->store(zero);
  }
  MJZ_CX_ND_RES_OBJ_FN
  success_t try_lock_cx(const bool need_to_wait) noexcept {
    MJZ_IFN_CONSTEVAL { return false; }
    if (need_to_wait) {
      asserts(!this->load(),
              " the mutex is  already locked , why did you re-lock??");
      this->store(one);
      return true;
    }
    if (this->load()) {
      return false;
    }
    this->store(one);
    return true;
  }

 public:
  MJZ_CX_FN mutex_ref_t(atomic_ref_t<T> &&obj) noexcept
      : atomic_ref_t<T>{std::move(obj)} {};
  MJZ_CX_FN mutex_ref_t(const mutex_ref_t &) noexcept = default;
  MJZ_CX_FN mutex_ref_t(mutex_ref_t &&) noexcept = default;
  MJZ_CX_FN mutex_ref_t &operator=(const mutex_ref_t &) noexcept = delete;
  MJZ_CX_FN mutex_ref_t &operator=(mutex_ref_t &&) noexcept = delete;
  MJZ_CX_FN ~mutex_ref_t() noexcept = default;

  MJZ_CX_FN void unlock() noexcept {
    MJZ_IFN_CONSTEVAL { return this->unlock_ncx(); }
    return this->unlock_cx();
  }
  MJZ_CX_ND_RES_OBJ_FN
  success_t try_lock(const bool need_to_wait, uint64_t timeout_count) noexcept {
    MJZ_IFN_CONSTEVAL {
      return this->try_lock_ncx(need_to_wait, timeout_count);
    }
    return this->try_lock_cx(need_to_wait);
  }

  MJZ_CX_ND_RES_OBJ_FN success_t try_lock(uint64_t timeout_count) noexcept {
    return try_lock(false, timeout_count);
  }
  MJZ_CONSTANT(uint64_t)
  defult_timeout = threads_ns::defult_timeout;
  MJZ_CX_ND_RES_OBJ_FN success_t try_lock() noexcept {
    return this->try_lock(defult_timeout);
  }
  MJZ_CX_FN void lock() noexcept(!MJZ_IN_DEBUG_MODE) {
    if (this->try_lock(true, defult_timeout)) MJZ_IS_LIKELY {
        return;
      }
    asserts.panic(" what happened !? why did the un-fail-able fail?! ");
  }
  /*
   *returns true if its locked
   */
  MJZ_CX_ND_FN
  explicit operator bool() const noexcept {
    return !!this->load(std::memory_order_acquire);
  }
};

template <std::unsigned_integral T>
struct multiread_singlewrite_mutex_ref_t {
  MJZ_NO_MV_NO_CPY(multiread_singlewrite_mutex_ref_t);
  MJZ_CONSTANT(T) const_rc_mask = T(-1) >> 1;
  MJZ_CONSTANT(T) mut_bit_mask = ~const_rc_mask;
  atomic_ref_t<T> ref;
  MJZ_CX_FN multiread_singlewrite_mutex_ref_t(T &r) noexcept : ref{r} {}
  MJZ_CX_FN void unlock(bool is_mut) noexcept { 
    if (is_mut) {
      ref.store(0, std::memory_order_release);
    } else { 
      if ((ref.fetch_sub(1, std::memory_order_release) & const_rc_mask) != 1)
        return;
    }
    ref.try_notify_all();
  }
  MJZ_CX_ND_RES_OBJ_FN success_t try_lock(bool is_mut) noexcept { 
    T expected =
        is_mut ? T{} : (ref.load(std::memory_order::relaxed) & const_rc_mask);
    return ref.compare_exchange_weak(
        expected, expected + (is_mut ? mut_bit_mask : 1),
                                     std::memory_order_acquire,
                                     std::memory_order_relaxed);
  }

  MJZ_CX_FN void lock_mut() noexcept {
    T expected{};
    while ((expected = ref.fetch_or(mut_bit_mask, std::memory_order_acquire)) &
           mut_bit_mask) {
      ref.try_wait(expected, std::memory_order_acquire);
    };
    while (expected & const_rc_mask) {
      ref.try_wait(expected, std::memory_order_acquire);
      expected = ref.load(std::memory_order_acquire);
    }; 
  }
  MJZ_CX_FN void lock_const() noexcept {
    T expected = ref.load(std::memory_order::relaxed);
    do {
      if (expected & mut_bit_mask) {
        ref.try_wait(expected, std::memory_order_acquire);
        expected = ref.load(std::memory_order::relaxed);
        expected &= const_rc_mask;
      }
    } while (!ref.compare_exchange_weak(expected, expected + 1,
                                        std::memory_order_acquire,
                                        std::memory_order_relaxed));
  }
  MJZ_CX_FN void lock(bool is_mut) noexcept {
    if (is_mut) {
      return lock_mut();
    }
    lock_const();
  }
};
template <std::unsigned_integral T>
struct ncx_multiread_singlewrite_lock_ref_t {
  MJZ_NO_MV_NO_CPY(ncx_multiread_singlewrite_lock_ref_t);
  MJZ_CX_FN ncx_multiread_singlewrite_lock_ref_t(T &ref_, bool is_mut_) noexcept
      : ref{ref_}, is_mut{is_mut_} {
    ref.lock(is_mut);
  }
  MJZ_CX_FN ~ncx_multiread_singlewrite_lock_ref_t() noexcept {
    ref.unlock(is_mut);
  }

 private:
  multiread_singlewrite_mutex_ref_t<T> ref;
  bool is_mut;
};

template <std::unsigned_integral T>
struct multiread_singlewrite_lock_ref_t {
  MJZ_NO_MV_NO_CPY(multiread_singlewrite_lock_ref_t);
  MJZ_CX_FN multiread_singlewrite_lock_ref_t(T &ref_, bool is_mut_) noexcept
      : is_mut{is_mut_}, ref{ref_} {}

  MJZ_CX_FN multiread_singlewrite_lock_ref_t(char *ptr, bool is_mut_) noexcept
      : is_mut{is_mut_},
        ref{MJZ_STD_is_constant_evaluated_FUNCTION_RET_
                ? (cx_val = cpy_bitcast<T>(cx_ptr = ptr))
                : *reinterpret_cast<T *>(
                      std::assume_aligned<alignof(T)>(ptr))} {
    ref.lock(is_mut);
  }
  MJZ_CX_FN ~multiread_singlewrite_lock_ref_t() noexcept {
    ref.unlock(is_mut);
    MJZ_IFN_CONSTEVAL_ return;
    if (cx_ptr) {
      cpy_bitcast(cx_ptr, cx_val);
    }
  }

 private:
  char *cx_ptr{};
  bool is_mut{};
  T cx_val{};
  multiread_singlewrite_mutex_ref_t<T> ref;
};
}  // namespace mjz::threads_ns
#endif  // MJZ_THREADS_mutex_ref_LIB_HPP_FILE_